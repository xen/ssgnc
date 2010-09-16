#include "tools-common.h"

#include <algorithm>

namespace {

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

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;
ssgnc::MemPool freq_tokens_pool;
std::vector<Ngram> ngrams;
ssgnc::FreqHandler freq_handler;

bool testMemLimit(ssgnc::UInt64 *mem_limit)
{
	if (vocab_dic.total_size() >= *mem_limit ||
		*mem_limit - vocab_dic.total_size() < (256ULL << 20))
	{
		SSGNC_ERROR << "Not enough memory: " << *mem_limit << std::endl;
		return false;
	}

	*mem_limit -= vocab_dic.total_size();
	try
	{
		ngrams.reserve(*mem_limit / 16);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Ngram>::reserve() failed: "
			<< sizeof(Ngram) << " * " << (*mem_limit / 16) << std::endl;
		return false;
	}
	*mem_limit -= sizeof(ngrams[0]) * ngrams.capacity();
	return true;
}

bool parseLine(const ssgnc::String &line,
	ssgnc::Int16 *encoded_freq, ssgnc::String *avail)
{
	ssgnc::UInt32 delim_pos;
	if (!line.last('\t', &delim_pos))
	{
		SSGNC_ERROR << "No delimitor: " << line << std::endl;
		return false;
	}

	const ssgnc::Int8 *freq_str = line.ptr() + delim_pos + 1;
	ssgnc::Int64 freq;
	if (!ssgnc::tools::parseInt64(freq_str, &freq))
	{
		SSGNC_ERROR << "ssgnc::tools::parseInt64() failed: "
			<< freq_str << std::endl;
		return false;
	}

	if (!freq_handler.encode(freq, encoded_freq))
	{
		SSGNC_ERROR << "ssgnc::FreqHandler::encode() failed: "
			<< freq << std::endl;
		return false;
	}

	*avail = line.substr(0, delim_pos);
	return true;
}

bool parseTokens(ssgnc::String avail, std::vector<ssgnc::Int32> *tokens)
{
	tokens->clear();
	for (ssgnc::Int32 i = 0; i < num_tokens; ++i)
	{
		if (avail.empty())
		{
			SSGNC_ERROR << "Too few tokens: " << avail << std::endl;
			return false;
		}

		ssgnc::String token;
		ssgnc::UInt32 delim_pos;
		if (avail.first(' ', &delim_pos))
		{
			token = avail.substr(0, delim_pos);
			avail = avail.substr(delim_pos + 1);
		}
		else
		{
			token = avail;
			avail = avail.substr(avail.length());
		}

		ssgnc::Int32 token_id;
		if (!vocab_dic.find(token, &token_id))
		{
			SSGNC_ERROR << "Unknown token: " << token << std::endl;
			return false;
		}

		try
		{
			tokens->push_back(token_id);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<ssgnc::Int32>::push_back() "
				"failed: " << tokens->size() << std::endl;
			return false;
		}
	}

	if (!avail.empty())
	{
		SSGNC_ERROR << "Too many tokens: " << avail << std::endl;
		return false;
	}
	return true;
}

bool encodeValue(ssgnc::Int32 value, ssgnc::StringBuilder *builder)
{
	if (value < 0)
	{
		SSGNC_ERROR << "Negative value: " << value << std::endl;
		return false;
	}

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
		if (!builder->append(static_cast<ssgnc::Int8>(
			temp_buf[num_bytes - i] | 0x80)))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}
	}

	if (!builder->append(static_cast<ssgnc::Int8>(temp_buf[0])))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		return false;
	}

	return true;
}

bool encodeFreqTokens(ssgnc::Int16 freq,
	const std::vector<ssgnc::Int32> &tokens,
	ssgnc::StringBuilder *freq_tokens)
{
	freq_tokens->clear();

	if (!encodeValue(freq, freq_tokens))
	{
		SSGNC_ERROR << "encodeValue() failed: " << freq << std::endl;
		return false;
	}

	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		if (!encodeValue(tokens[i], freq_tokens))
		{
			SSGNC_ERROR << "encodeValue() failed: "
				<< i << ", " << tokens[i] << std::endl;
			return false;
		}
	}

	return true;
}

bool flushNgrams(ssgnc::FilePath *file_path)
{
	ssgnc::StringBuilder path;
	if (!file_path->read(&path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::read() failed: " << std::endl;
		return false;
	}

	std::ofstream file(path.ptr(), std::ios::binary);
	if (!file)
	{
		SSGNC_ERROR << "std::ofstream::open() failed: "
			<< path.str() << std::endl;
		return false;
	}

	ssgnc::UInt64 mem_usage = vocab_dic.total_size()
		+ freq_tokens_pool.total_size()
		+ sizeof(ngrams[0]) * ngrams.capacity();
	std::cerr << "No. ngrams: " << ngrams.size()
		<< ", Total length: " << freq_tokens_pool.total_length()
		<< ", Memory usage: " << mem_usage << std::endl;

	std::stable_sort(ngrams.begin(), ngrams.end(), NgramComparer());

	for (std::size_t i = 0; i < ngrams.size(); ++i)
	{
		const Ngram &ngram = ngrams[i];
		ssgnc::String ngram_str;
		if (!freq_tokens_pool.get(ngram.pos, ngram.length, &ngram_str))
		{
			SSGNC_ERROR << "ssgnc::MemPool::get() failed: "
				<< ngram.pos << ", " << ngram.length << std::endl;
			return false;
		}

		file << ngram_str;
		if (!file)
		{
			SSGNC_ERROR << "std::ofstream::operator<<() failed" << std::endl;
			return false;
		}
	}

	freq_tokens_pool.clear();
	ngrams.clear();

	if (!file.flush())
	{
		SSGNC_ERROR << "std::ofstream::flush() failed" << std::endl;
		return false;
	}

	return true;
}

bool encodeNgrams(ssgnc::FilePath *file_path, ssgnc::UInt64 mem_limit)
{
	if (!testMemLimit(&mem_limit))
	{
		SSGNC_ERROR << "testMemLimit() failed" << std::endl;
		return false;
	}

	std::string line;
	std::vector<ssgnc::Int32> tokens;
	ssgnc::StringBuilder freq_tokens;
	while (ssgnc::tools::readLine(&std::cin, &line))
	{
		Ngram ngram;
		ssgnc::String avail;
		if (!parseLine(ssgnc::String(line.c_str(), line.length()),
			&ngram.freq, &avail))
		{
			SSGNC_ERROR << "parseLine() failed: " << line << std::endl;
			return false;
		}

		if (!parseTokens(avail, &tokens))
		{
			SSGNC_ERROR << "parseTokens() failed: " << avail << std::endl;
			return false;
		}

		if (!encodeFreqTokens(ngram.freq, tokens, &freq_tokens))
		{
			SSGNC_ERROR << "encodeFreqTokens() failed" << std::endl;
			return false;
		}

		if (!freq_tokens_pool.append(freq_tokens.str(), &ngram.pos))
		{
			SSGNC_ERROR << "ssgnc::MemPool::append() failed" << std::endl;
			return false;
		}
		ngram.length = freq_tokens.length();

		try
		{
			ngrams.push_back(ngram);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<Ngram>::push_back() failed: "
				<< ngrams.size() << std::endl;
			return false;
		}

		if (ngrams.size() >= ngrams.capacity() ||
			freq_tokens_pool.total_size() >= mem_limit)
		{
			if (!flushNgrams(file_path))
			{
				SSGNC_ERROR << "flushNgrams() failed" << std::endl;
				return false;
			}
		}
	}
	if (std::cin.bad())
		return false;

	if (!ngrams.empty())
	{
		if (!flushNgrams(file_path))
		{
			SSGNC_ERROR << "flushNgrams() failed" << std::endl;
			return false;
		}
	}

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

	if (argc < 4 || argc > 5)
	{
		std::cerr << "Usage: " << argv[0]
			<< " NUM_TOKENS VOCAB_DIC TEMP_DIR [MEM_LIMIT]" << std::endl;
		return 1;
	}

	if (!ssgnc::tools::parseNumTokens(argv[1], &num_tokens))
		return 2;

	if (!vocab_dic.open(argv[2], ssgnc::FileMap::READ_FILE))
		return 3;

	ssgnc::FilePath file_path;
	if (!ssgnc::tools::initFilePath(argv[3], "bin", num_tokens, &file_path))
		return 4;

	ssgnc::UInt64 mem_limit = 1024ULL << 20;
	if (argc > 4 && !ssgnc::tools::parseMemLimit(argv[4], &mem_limit))
		return 5;

	if (!encodeNgrams(&file_path, mem_limit))
		return 6;

	return 0;
}
