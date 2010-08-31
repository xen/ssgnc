#include "tools-common.h"

#include <algorithm>
#include <string>

namespace {

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

struct Ngram
{
	ssgnc::UInt32 pos;
	ssgnc::UInt16 length;
	ssgnc::Int16 freq;
};

class NgramComparer
{
public:
	bool operator()(const Ngram &lhs, const Ngram &rhs) const
	{ return lhs.freq > rhs.freq; }
};

void encodeValue(ssgnc::Int32 value, ssgnc::StringBuilder *builder)
{
	ssgnc::UInt8 temp_buf[8];
	ssgnc::Int32 num_bytes = 0;

	while (value >= 0x80)
	{
		temp_buf[num_bytes++] = static_cast<ssgnc::UInt8>(value & 0x7F);
		value >>= 7;
	}
	temp_buf[num_bytes++] = static_cast<ssgnc::UInt8>(value & 0x7F);

	for (ssgnc::Int32 i = 1; i < num_bytes; ++i)
	{
		builder->append(static_cast<ssgnc::Int8>(
			temp_buf[num_bytes - i] | 0x80));
	}
	builder->append(static_cast<ssgnc::Int8>(temp_buf[0]));
}

void encodeFreqTokens(ssgnc::Int16 freq,
	const std::vector<ssgnc::Int32> &tokens,
	ssgnc::StringBuilder *freq_tokens)
{
	freq_tokens->clear();

	encodeValue(freq, freq_tokens);
	for (std::size_t i = 0; i < tokens.size(); ++i)
		encodeValue(tokens[i], freq_tokens);
}

bool flushNgrams(ssgnc::MemPool *freq_tokens_pool,
	std::vector<Ngram> *ngrams, ssgnc::FilePath *file_path)
{
	if (!file_path->next())
	{
		ERROR << "failed to generate path: "
			<< file_path->format() << std::endl;
		return false;
	}

	std::ofstream file(file_path->path().ptr(), std::ios::binary);
	if (!file)
	{
		ERROR << "failed to open file: " << file_path->path() << std::endl;
		return false;
	}

	ssgnc::UInt32 total_size = freq_tokens_pool->total_size()
		+ static_cast<ssgnc::UInt32>(sizeof((*ngrams)[0]) * ngrams->size());
	std::cerr << "No. ngrams: " << ngrams->size()
		<< ", Memory usage: " << total_size
		<< ", Total length: " << freq_tokens_pool->total_length() << std::endl;

	std::stable_sort(ngrams->begin(), ngrams->end(), NgramComparer());

	for (std::size_t i = 0; i < ngrams->size(); ++i)
	{
		const Ngram &ngram = (*ngrams)[i];
		file << freq_tokens_pool->get(ngram.pos, ngram.length);
	}

	freq_tokens_pool->clear();
	ngrams->clear();

	if (!file.flush())
	{
		ERROR << "failed to write ngrams" << std::endl;
		return false;
	}

	return true;
}

bool encodeNgrams(ssgnc::FilePath *file_path)
{
	static const ssgnc::UInt32 NUM_NGRAMS_THRESHOLD = 1 << 25;
	static const ssgnc::UInt32 TOTAL_SIZE_THRESHOLD = 1 << 29;

	ssgnc::FreqHandler freq_handler;

	ssgnc::MemPool freq_tokens_pool;
	std::vector<Ngram> ngrams;

	std::string line;
	std::vector<ssgnc::Int32> tokens;
	ssgnc::StringBuilder freq_tokens;
	while (std::getline(std::cin, line))
	{
		tokens.clear();

		Ngram ngram;

		std::string::size_type delim_pos = line.find_last_of('\t');
		if (delim_pos == std::string::npos)
		{
			ERROR << "no delimitor: " << line << std::endl;
			return false;
		}

		const char *freq_str = line.c_str() + delim_pos + 1;
		char *end_of_freq;
		long long temp_freq = std::strtoll(freq_str, &end_of_freq, 10);
		if (*end_of_freq != '\0')
		{
			ERROR << "invalid freq: " << freq_str << std::endl;
			return false;
		}
		else if (temp_freq <= 0)
		{
			ERROR << "out of range freq: " << temp_freq << std::endl;
			return false;
		}
		ngram.freq = freq_handler.encode(temp_freq);

		ssgnc::String avail(line.c_str(), delim_pos);
		for (ssgnc::Int32 i = 0; i < num_tokens; ++i)
		{
			ssgnc::String delim = avail.first(' ');
			ssgnc::String token(avail.begin(), delim.begin());
			if (token.empty())
			{
				ERROR << "too few tokens: " << line << std::endl;
				return false;
			}

			ssgnc::Int32 token_id = vocab_dic[token];
			if (token_id < 0)
			{
				ERROR << "unknown token: " << token << std::endl;
				return false;
			}
			tokens.push_back(token_id);

			avail = ssgnc::String(delim.end(), avail.end());
		}

		if (!avail.empty())
		{
			ERROR << "too many tokens: " << line << std::endl;
			return false;
		}

		encodeFreqTokens(ngram.freq, tokens, &freq_tokens);

		ngram.pos = freq_tokens_pool.append(freq_tokens.str());
		ngram.length = freq_tokens.length();

		ngrams.push_back(ngram);

		ssgnc::UInt32 total_size = freq_tokens_pool.total_size()
			+ static_cast<ssgnc::UInt32>(sizeof(ngrams[0]) * ngrams.capacity());
		if (ngrams.size() >= NUM_NGRAMS_THRESHOLD ||
			total_size >= TOTAL_SIZE_THRESHOLD)
		{
			if (!flushNgrams(&freq_tokens_pool, &ngrams, file_path))
				return false;
		}
	}

	if (!ngrams.empty())
	{
		if (!flushNgrams(&freq_tokens_pool, &ngrams, file_path))
			return false;
	}

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
			<< " NUM_TOKENS VOCAB_DIC TEMP_DIR" << std::endl;
		return 1;
	}

	if (!ssgnc::tools::parseNumTokens(argv[1], &num_tokens))
		return 2;

	if (!ssgnc::tools::readVocabDic(argv[2], &vocab_dic))
		return 3;

	ssgnc::FilePath file_path;
	if (!ssgnc::tools::initFilePath(argv[3], "bin", num_tokens, &file_path))
		return 4;

	if (!encodeNgrams(&file_path))
		return 5;

	return 0;
}
