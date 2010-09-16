#include <ssgnc.h>

#include <iostream>
#include <string>

namespace {

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
	enum { MAX_KEY_ID = 0x7FFFFFFF };

	std::string line;
	while (readLine(in, &line))
	{
		ssgnc::String key(line.c_str(),
			static_cast<ssgnc::UInt32>(line.length()));
		std::cout << "vocab_dic[\"" << key << "\"]: ";

		ssgnc::Int32 key_id;
		if (vocab_dic.find(key, &key_id))
			std::cout << key_id;
		else
			std::cout << "not found";

		char *end_of_key_id;
		ssgnc::Int64 key_id_64 = std::strtoll(key.ptr(), &end_of_key_id, 10);
		if (*end_of_key_id == '\0')
		{
			std::cout << ", vocab_dic[" << key_id << "]: ";
			if (key_id_64 < 0 || key_id_64 >=
				static_cast<ssgnc::Int64>(vocab_dic.num_keys()))
				std::cout << "out of range";
			else
			{
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

	ssgnc::VocabDic vocab_dic;
	if (!vocab_dic.open(argv[1]))
		return 2;

	if (argc == 2)
	{
		if (!lookupKeys(&std::cin, vocab_dic))
			return 3;
	}

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
