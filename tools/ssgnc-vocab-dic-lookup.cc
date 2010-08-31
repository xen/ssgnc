#include "tools-common.h"

#include <string>

namespace {

void lookupKeys(std::istream *in, const ssgnc::VocabDic &vocab_dic)
{
	enum { MAX_KEY_ID = 0x7FFFFFFF };

	std::string line;
	while (std::getline(*in, line))
	{
		ssgnc::String key(line.c_str(),
			static_cast<ssgnc::UInt32>(line.length()));
		std::cout << "vocab_dic[\"" << key << "\"]: " << vocab_dic[key];

		char *end_of_key_id;
		long key_id = std::strtol(key.ptr(), &end_of_key_id, 10);
		if (*end_of_key_id == '\0')
		{
			std::cout << ", vocab_dic[" << key_id << "]: ";
			if (key_id > MAX_KEY_ID)
				std::cout << "out of range";
			else
			{
				ssgnc::String match = vocab_dic[key_id];
				if (key_id > MAX_KEY_ID || match.empty())
					std::cout << "out of range";
				else
					std::cout << match;
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

	ssgnc::FileMap file_map;
	ssgnc::VocabDic vocab_dic;
	if (!ssgnc::tools::mmapVocabDic(argv[1], &file_map, &vocab_dic))
		return 2;

	if (argc == 2)
		lookupKeys(&std::cin, vocab_dic);

	for (int i = 2; i < argc; ++i)
	{
		std::cerr << "> " << argv[i] << std::endl;
		std::ifstream file(argv[i], std::ios::binary);
		if (!file)
		{
			ERROR << "failed to open file: " << argv[i] << std::endl;
			continue;
		}
		lookupKeys(&file, vocab_dic);
	}

	return 0;
}
