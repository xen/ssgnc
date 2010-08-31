#include "tools-common.h"

namespace {

void lookupKeys(std::istream *in, const ssgnc::VocabDic &vocab_dic)
{
	enum { MAX_KEY_ID = 0x7FFFFFFF };

	std::string line;
	while (ssgnc::tools::readLine(in, &line))
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
		long key_id_l = std::strtol(key.ptr(), &end_of_key_id, 10);
		if (*end_of_key_id == '\0')
		{
			std::cout << ", vocab_dic[" << key_id << "]: ";
			if (key_id_l < 0 || key_id_l > MAX_KEY_ID)
				std::cout << "out of range";
			else
			{
				key_id = static_cast<ssgnc::Int32>(key_id_l);
				ssgnc::String match;
				if (vocab_dic.find(key_id, &match))
					std::cout << match;
				else
					std::cout << "out of range";
			}
		}
		std::cout << '\n';
	}
	std::cout.flush();
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

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
		lookupKeys(&std::cin, vocab_dic);

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
		lookupKeys(&file, vocab_dic);
	}

	return 0;
}
