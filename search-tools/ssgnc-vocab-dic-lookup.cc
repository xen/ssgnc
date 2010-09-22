#include <ssgnc.h>

#include <iostream>
#include <string>

namespace {

// This function reads a line from `in' and stores it into `line'.
// If the stream reaches its end or an unexpected error occurs,
// this function returns false. And in the latter case,
// the bad bit of `in' is set to true.
bool readLine(std::istream *in, std::string *line)
{
	try
	{
		if (!std::getline(*in, *line))
			return false;
		return true;
	}
	catch (...)
	{
		in->setstate(std::ios::badbit);
		return false;
	}
}

bool lookupKeys(std::istream *in, const ssgnc::VocabDic &vocab_dic)
{
	std::string line;
	while (readLine(in, &line))
	{
		// ssgnc::String represents a string as a pair of its start adress and
		// its length. So, if the source string is modified, the represented
		// string is modified too. In the worst-case scenario, the start
		// address will be invalid after the modification.
		ssgnc::String key(line.c_str(),
			static_cast<ssgnc::UInt32>(line.length()));
		std::cout << "vocab_dic[\"" << key << "\"]: ";

		// A dictionary lookup for conversion from a token to its ID.
		ssgnc::Int32 key_id;
		if (vocab_dic.find(key, &key_id))
			std::cout << key_id;
		else
			std::cout << "not found";

		// If the given key seems to be a token ID, a dictionary lookup for
		// conversion from a token ID to its entry is tested.
		char *end_of_key_id;
		ssgnc::Int64 key_id_64 = std::strtoll(key.ptr(), &end_of_key_id, 10);
		if (*end_of_key_id == '\0')
		{
			// The token ID-like integer might cause an over flow becase it
			// is ssgnc::Int64 and find() takes ssgnc::Int32.
			std::cout << ", vocab_dic[" << key_id << "]: ";
			if (key_id_64 < 0 || key_id_64 >=
				static_cast<ssgnc::Int64>(vocab_dic.num_keys()))
				std::cout << "out of range";
			else
			{
				// If the token ID is valid, its body is given.
				key_id = static_cast<ssgnc::Int32>(key_id_64);
				ssgnc::String match;
				if (vocab_dic.find(key_id, &match))
					std::cout << match;
				else
					std::cout << "out of range";
			}
		}

		std::cout << '\n';
		if (!std::cout)
		{
			SSGNC_ERROR << "std::ostream::operator<<() failed" << std::endl;
			return false;
		}
	}

	// The bad bit of `in' indicates whether an error has occured or not.
	if (in->bad())
	{
		SSGNC_ERROR << "::readLine() failed" << std::endl;
		return false;
	}

	if (!std::cout.flush())
	{
		SSGNC_ERROR << "std::ostream::flush() failed" << std::endl;
		return false;
	}
	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0]
			<< " VOCAB_DIC [FILE]..." << std::endl;
		return 1;
	}

	// A dictionary file is opened as a memory-mapped file.
	// If open() takes ssgnc::FileMap::READ_FILE as the 2nd argument,
	// the entire file is loaded in this function.
	ssgnc::VocabDic vocab_dic;
	if (!vocab_dic.open(argv[1]))
		return 2;

	// If there are no more arguments, keys are read from the standard input.
	if (argc == 2)
	{
		if (!lookupKeys(&std::cin, vocab_dic))
			return 3;
	}

	// If there are arguments other than the dictionary path,
	// keys are read from the files specified as the remaining arguments.
	for (int i = 2; i < argc; ++i)
	{
		std::cerr << "> " << argv[i] << std::endl;
		std::ifstream file(argv[i], std::ios::binary);
		if (!file)
		{
			SSGNC_ERROR << "ssgnc::ifstream::open() failed: "
				<< argv[i] << std::endl;
			continue;
		}

		if (!lookupKeys(&file, vocab_dic))
			return 3;
	}

	return 0;
}
