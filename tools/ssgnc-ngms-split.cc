#include "tools-common.h"

#include <algorithm>
#include <iomanip>

namespace {

struct Ngram
{
	ssgnc::UInt32 pos;
	ssgnc::UInt32 length;
};

// Token ID (first) and Ngram ID (second).
typedef std::pair<ssgnc::Int32, ssgnc::UInt32> IdPair;

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;
ssgnc::MemPool ngram_pool;
std::vector<Ngram> ngrams;
std::vector<IdPair> pairs;

bool testMemLimit(ssgnc::UInt64 *mem_limit)
{
	try
	{
		ngrams.reserve(*mem_limit / 64);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Ngram>::reserve() failed: "
			<< sizeof(Ngram) << " * " << (*mem_limit / 64) << std::endl;
		return false;
	}

	try
	{
		pairs.reserve(*mem_limit / 16);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<IdPair>::reserve() failed: "
			<< sizeof(IdPair) << " * " << (*mem_limit / 16) << std::endl;
		return false;
	}

	*mem_limit -= sizeof(ngrams[0]) * ngrams.capacity()
		+ sizeof(pairs[0]) * pairs.capacity();
	return true;
}

bool readNgram(ssgnc::ByteReader *byte_reader,
	std::vector<ssgnc::Int32> *tokens, Ngram *ngram)
{
	static ssgnc::StringBuilder ngram_buf;

	if (!ssgnc::tools::readFreq(byte_reader, &ngram_buf, NULL))
	{
		SSGNC_ERROR << "ssgnc::tools::readFreq() failed" << std::endl;
		return false;
	}
	else if (ngram_buf.empty())
	{
		ngram->pos = 0;
		ngram->length = 0;
		return true;
	}

	if (!ssgnc::tools::readTokens(num_tokens, vocab_dic,
		byte_reader, &ngram_buf, tokens))
	{
		SSGNC_ERROR << "ssgnc::tools::readTokens() failed" << std::endl;
		return false;
	}

	std::sort(tokens->begin(), tokens->end());
	tokens->erase(std::unique(tokens->begin(), tokens->end()), tokens->end());

	if (!ngram_pool.append(ngram_buf.str(), &ngram->pos))
	{
		SSGNC_ERROR << "ssgnc::MemPool::append() failed" << std::endl;
		return false;
	}
	ngram->length = ngram_buf.length();

	try
	{
		ngrams.push_back(*ngram);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Ngram>::push_back() failed: "
			<< ngrams.size() << std::endl;
		return false;
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

		ssgnc::String ngram_str;
		const Ngram &ngram = ngrams[pairs[i].second];
		if (!ngram_pool.get(ngram.pos, ngram.length, &ngram_str))
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
		SSGNC_ERROR << "std::ofstream::flush() failed" << std::endl;
		return false;
	}

	return true;
}

bool splitNgrams(ssgnc::FilePath *file_path, ssgnc::UInt64 mem_limit)
{
	const ssgnc::UInt64 FILE_SIZE_LIMIT = 0x7FFFFFFF - (256 * num_tokens);

	if (!testMemLimit(&mem_limit))
	{
		SSGNC_ERROR << "testMemLimit() failed" << std::endl;
		return false;
	}

	ssgnc::ByteReader byte_reader;
	if (!byte_reader.open(&std::cin))
	{
		SSGNC_ERROR << "ssgnc::ByteReader::open() failed" << std::endl;
		return false;
	}

	ssgnc::UInt64 num_pairs = 0;
	ssgnc::UInt64 file_size = 0;
	ssgnc::UInt64 total_size = 0;

	std::vector<ssgnc::Int32> tokens;
	Ngram ngram = { 0, 0 };
	while (readNgram(&byte_reader, &tokens, &ngram))
	{
		if (ngram.length == 0)
		{
			if (!ngrams.empty())
			{
				if (!flushNgrams(file_path))
				{
					SSGNC_ERROR << "flushNgrams() failed" << std::endl;
					return false;
				}
			}
			std::cerr << "No. pairs: " << num_pairs
				<< ", Total size: " << total_size << std::endl;
			return true;
		}

		try
		{
			for (std::size_t i = 0; i < tokens.size(); ++i)
				pairs.push_back(IdPair(tokens[i], ngram_pool.num_objs() - 1));
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<IdPair>::push_back() failed: "
				<< pairs.size() << std::endl;
			return false;
		}

		num_pairs += tokens.size();
		file_size += ngram.length * tokens.size();
		total_size += ngram.length * tokens.size();

		if (ngrams.size() >= ngrams.capacity() ||
			pairs.size() + num_tokens > pairs.capacity() ||
			ngram_pool.total_size() >= mem_limit ||
			file_size >= FILE_SIZE_LIMIT)
		{
			if (!flushNgrams(file_path))
			{
				SSGNC_ERROR << "flushNgrams() failed" << std::endl;
				return false;
			}
			file_size = 0;
		}
	}

	return false;
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

	if (!vocab_dic.mmap(argv[2]))
		return 3;

	ssgnc::FilePath file_path;
	if (!ssgnc::tools::initFilePath(argv[3], "part", num_tokens, &file_path))
		return 4;

	ssgnc::UInt64 mem_limit = 1024ULL << 20;
	if (argc > 4 && !ssgnc::tools::parseMemLimit(argv[4], &mem_limit))
		return 5;

	if (!splitNgrams(&file_path, mem_limit))
		return 6;

	return 0;
}
