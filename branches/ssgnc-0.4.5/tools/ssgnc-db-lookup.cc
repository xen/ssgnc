#include "tools-common.h"

namespace {

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;
ssgnc::NgramIndex ngram_index;
ssgnc::Int64 max_count = 3;
ssgnc::FreqHandler freq_handler;

bool mmapVocabDic(const ssgnc::String &index_dir)
{
	ssgnc::StringBuilder path;
	if (!ssgnc::FilePath::join(index_dir, "vocab.dic", &path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::join() failed" << std::endl;
		return false;
	}

	if (!vocab_dic.open(path.ptr()))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::mmap() failed: " << path << std::endl;
		return false;
	}

	return true;
}

bool mmapNgramIndex(const ssgnc::String &index_dir)
{
	ssgnc::StringBuilder path;
	if (!ssgnc::FilePath::join(index_dir, "ngms.idx", &path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::join() failed" << std::endl;
		return false;
	}

	if (!ngram_index.open(path.ptr()))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::mmap() failed: "
			<< path << std::endl;
		return false;
	}

	return true;
}

void lookupToken(const ssgnc::String &index_dir, ssgnc::Int32 token_id)
{
	ssgnc::NgramIndex::Entry entry;
	if (!ngram_index.get(num_tokens, token_id, &entry))
		return;

	std::cout << "File id: " << entry.file_id()
		<< ", Offset: " << entry.offset()
		<< ", Approx size: " << entry.approx_size() << '\n';

	ssgnc::NgramReader reader;
	if (!reader.open(index_dir, num_tokens, entry))
		return;

	ssgnc::Int64 count = 0;
	ssgnc::Int16 encoded_freq;
	std::vector<ssgnc::Int32> tokens;
	while (reader.read(&encoded_freq, &tokens))
	{
		ssgnc::Int64 freq;
		if (!freq_handler.decode(encoded_freq, &freq))
			return;

		std::cout << "- Token IDs:";
		for (std::size_t i = 0; i < tokens.size(); ++i)
			std::cout << ' ' << tokens[i];
		std::cout << ", Freq: " << freq << '\n';

		std::cout << ' ';
		for (std::size_t i = 0; i < tokens.size(); ++i)
		{
			ssgnc::String token;
			if (!vocab_dic.find(tokens[i], &token))
				return;

			std::cout << ' ' << token;
		}
		std::cout << '\t' << freq << '\n';

		if (++count >= max_count)
			break;
	}
}

void lookupTokens(const ssgnc::String &path_format, std::istream *in)
{
	enum { MAX_TOKEN_ID = 0x7FFFFFFF };

	std::string line;
	while (ssgnc::tools::readLine(in, &line))
	{
		ssgnc::String token(line.c_str(),
			static_cast<ssgnc::UInt32>(line.length()));
		std::cout << "vocab_dic[\"" << token << "\"]: ";

		ssgnc::Int32 token_id;
		if (vocab_dic.find(token, &token_id))
		{
			std::cout << token_id << '\n';
			lookupToken(path_format, token_id);
		}
		else
			std::cout << "not found\n";

		char *end_of_token_id;
		long token_id_l = std::strtol(token.ptr(), &end_of_token_id, 10);
		if (*end_of_token_id == '\0')
		{
			std::cout << "vocab_dic[" << token_id_l << "]: ";
			if (token_id_l < 0 || token_id_l > MAX_TOKEN_ID)
				std::cout << "out of range";
			else
			{
				token_id = static_cast<ssgnc::Int32>(token_id_l);
				ssgnc::String match;
				if (vocab_dic.find(token_id, &match))
				{
					std::cout << match << '\n';
					lookupToken(path_format, token_id);
				}
				else
					std::cout << "out of range\n";
			}
		}
	}
	std::cout.flush();
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

	if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0]
			<< " INDEX_DIR NUM_TOKENS [MAX_COUNT] [FILE]..." << std::endl;
		return 1;
	}

	ssgnc::String index_dir = argv[1];

	if (!ssgnc::tools::parseNumTokens(argv[2], &num_tokens))
		return 3;

	if (!mmapVocabDic(index_dir) || !mmapNgramIndex(index_dir))
		return 4;

	if (argc > 3 && !ssgnc::tools::parseInt64(argv[3], &max_count))
		return 5;

	if (max_count <= 0)
	{
		SSGNC_ERROR << "Out of range max count: " << max_count << std::endl;
		return 6;
	}

	if (argc < 5)
		lookupTokens(index_dir, &std::cin);

	for (int i = 4; i < argc; ++i)
	{
		std::cerr << "> " << argv[i] << std::endl;
		std::ifstream file(argv[i], std::ios::binary);
		if (!file)
		{
			SSGNC_ERROR << "ssgnc::ifstream::open() failed: "
				<< argv[i] << std::endl;
			continue;
		}
		lookupTokens(index_dir, &file);
	}

	return 0;
}
