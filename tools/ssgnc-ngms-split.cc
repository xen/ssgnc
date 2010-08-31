#include "tools-common.h"

#include <algorithm>
#include <iomanip>

namespace {

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

struct Ngram
{
	ssgnc::UInt32 pos;
	ssgnc::UInt32 length;
};

// Token ID (first) and Ngram ID (second).
typedef std::pair<ssgnc::Int32, ssgnc::UInt32> IdPair;

ssgnc::MemPool ngram_pool;
std::vector<Ngram> ngrams;
std::vector<IdPair> pairs;

bool readNgram(ssgnc::ByteReader *byte_reader,
	std::vector<ssgnc::Int32> *tokens, Ngram *ngram)
{
	static ssgnc::StringBuilder ngram_buf;

	ngram_buf.clear();
	tokens->clear();

	ssgnc::Int32 byte;
	ssgnc::Int32 freq = 0;
	while ((byte = byte_reader->read()) >= 0)
	{
		ngram_buf.append(static_cast<ssgnc::Int8>(byte));

		freq = (freq << 7) + (byte & 0x7F);
		if (freq > ssgnc::FreqHandler::MAX_ENCODED_FREQ)
		{
			ERROR << "out of range freq: " << freq << std::endl;
			return false;
		}
		else if (byte < 0x80)
			break;
	}

	if (byte < 0)
		return false;

	ssgnc::Int32 token = 0;
	ssgnc::Int32 token_count = 0;
	while ((byte = byte_reader->read()) >= 0)
	{
		ngram_buf.append(static_cast<ssgnc::Int8>(byte));

		token = (token << 7) + (byte & 0x7F);
		if (byte < 0x80)
		{
			if (static_cast<ssgnc::UInt32>(token) >= vocab_dic.num_keys())
			{
				ERROR << "unknown token: " << token << std::endl;
				return false;
			}
			else if (std::find(tokens->begin(), tokens->end(), token)
				== tokens->end())
				tokens->push_back(token);
			token = 0;

			if (++token_count == num_tokens)
			{
				ngram->pos = ngram_pool.append(ngram_buf.str());
				ngram->length = ngram_buf.length();
				ngrams.push_back(*ngram);
				return true;
			}
		}
	}

	return false;
}

bool flushNgrams(ssgnc::FilePath *file_path)
{
	if (!file_path->next())
	{
		ERROR << "failed to generate file path" << std::endl;
		return false;
	}

	std::ofstream file(file_path->path().ptr(), std::ios::binary);
	if (!file)
	{
		ERROR << "failed to open file: " << file_path->path() << std::endl;
		return false;
	}

	ssgnc::UInt32 mem_usage = ngram_pool.total_size()
		+ static_cast<ssgnc::UInt32>(sizeof(ngrams[0]) * ngrams.capacity())
		+ static_cast<ssgnc::UInt32>(sizeof(pairs[0]) * pairs.capacity());
	std::cerr << "No. ngrams: " << ngrams.size()
		<< ", No. pairs: " << pairs.size()
		<< ", Mem usage: " << mem_usage << std::endl;

	std::sort(pairs.begin(), pairs.end());

	ssgnc::Int32 token = 0;
	for (std::size_t i = 0; i < pairs.size(); ++i)
	{
		while (token < pairs[i].first)
		{
			file.put('\0');
			++token;
		}
		ssgnc::String ngram = ngram_pool.get(
			ngrams[pairs[i].second].pos, ngrams[pairs[i].second].length);
		file << ngram;
	}

	while (static_cast<ssgnc::UInt32>(token) < vocab_dic.num_keys())
	{
		file.put('\0');
		++token;
	}

	ngram_pool.clear();
	ngrams.clear();
	pairs.clear();

	if (!file.flush())
	{
		ERROR << "failed to write ngrams" << std::endl;
		return false;
	}

	return true;
}

bool splitNgrams(ssgnc::FilePath *file_path)
{
	static const ssgnc::UInt32 NUM_NGRAMS_THRESHOLD = 1 << 24;
	static const ssgnc::UInt32 NUM_PAIRS_THRESHOLD = (1 << 26) - (1 << 16);
	static const ssgnc::UInt32 MEM_USAGE_THRESHOLD = 1 << 30;
	static const ssgnc::UInt32 FILE_SIZE_THRESHOLD = (1U << 31) - (1 << 26);

	ssgnc::ByteReader byte_reader;
	byte_reader.open(&std::cin);

	std::vector<ssgnc::Int32> tokens;

	ssgnc::UInt64 num_pairs = 0;
	ssgnc::UInt32 file_size = 0;
	ssgnc::UInt64 total_size = 0;

	Ngram ngram = { 0, 0 };
	while (readNgram(&byte_reader, &tokens, &ngram))
	{
		for (std::size_t i = 0; i < tokens.size(); ++i)
			pairs.push_back(IdPair(tokens[i], ngram_pool.num_objs() - 1));

		num_pairs += tokens.size();
		file_size += static_cast<ssgnc::UInt32>(ngram.length * tokens.size());
		total_size += ngram.length * tokens.size();

		ssgnc::UInt32 mem_usage = ngram_pool.total_size()
			+ static_cast<ssgnc::UInt32>(sizeof(ngrams[0]) * ngrams.capacity())
			+ static_cast<ssgnc::UInt32>(sizeof(pairs[0]) * pairs.capacity());
		if (ngrams.size() >= NUM_NGRAMS_THRESHOLD ||
			pairs.size() >= NUM_PAIRS_THRESHOLD ||
			mem_usage >= MEM_USAGE_THRESHOLD ||
			file_size >= FILE_SIZE_THRESHOLD)
		{
			if (!flushNgrams(file_path))
				return false;
			file_size = 0;
		}
	}

	if (!byte_reader.eof())
		return false;

	if (!ngrams.empty())
	{
		if (!flushNgrams(file_path))
			return false;
	}

	std::cerr << "No. pairs: " << num_pairs
		<< ", Total size: " << total_size << std::endl;

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

	ssgnc::FileMap file_map;
	if (!ssgnc::tools::mmapVocabDic(argv[2], &file_map, &vocab_dic))
		return 3;

	ssgnc::FilePath file_path;
	if (!ssgnc::tools::initFilePath(argv[3], "part", num_tokens, &file_path))
		return 4;

	if (!splitNgrams(&file_path))
		return 5;

	return 0;
}
