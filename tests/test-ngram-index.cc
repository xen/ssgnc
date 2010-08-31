#include "ssgnc.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <sstream>

int main()
{
	static const ssgnc::Int32 MAX_NUM_TOKENS = 4;
	static const ssgnc::Int32 MAX_TOKEN_ID = 1023;

	std::stringstream stream;

	assert(ssgnc::Writer(&stream).write(MAX_NUM_TOKENS));
	assert(ssgnc::Writer(&stream).write(MAX_TOKEN_ID));

	std::vector<ssgnc::NgramIndex::FileEntry> entries;
	for (ssgnc::Int32 i = 1; i <= MAX_NUM_TOKENS; ++i)
	{
		ssgnc::NgramIndex::FileEntry ngram_offset;
		assert(ngram_offset.set_file_id(0));
		assert(ngram_offset.set_offset(0));
		entries.push_back(ngram_offset);

		assert(ssgnc::Writer(&stream).write(ngram_offset));
	}
	std::size_t index = 0;
	for (ssgnc::Int32 i = 0; i <= MAX_TOKEN_ID; ++i)
	{
		for (ssgnc::Int32 j = 1; j <= MAX_NUM_TOKENS; ++j)
		{
			ssgnc::Int64 total = (static_cast<ssgnc::Int64>(
				entries[index].file_id()) << 31) + entries[index].offset();
			if (std::rand() % 10 == 0)
			{
				total += std::rand();
				if (std::rand() % 10 == 0)
					total += std::rand() * 4ULL;
			}
			else
				total += std::rand() % 65536;
			++index;

			ssgnc::NgramIndex::FileEntry ngram_offset;
			assert(ngram_offset.set_file_id(
				static_cast<ssgnc::Int32>(total >> 31)));
			assert(ngram_offset.set_offset(
				static_cast<ssgnc::UInt32>(total & 0x7FFFFFFF)));
			entries.push_back(ngram_offset);

			assert(ssgnc::Writer(&stream).write(ngram_offset));
		}
	}

	ssgnc::NgramIndex ngram_index;

	assert(ngram_index.max_num_tokens() == 0);
	assert(ngram_index.max_token_id() == 0);

	assert(ngram_index.read(&stream));

	assert(ngram_index.max_num_tokens() == MAX_NUM_TOKENS);
	assert(ngram_index.max_token_id() == MAX_TOKEN_ID);

	for (ssgnc::Int32 i = 1; i <= MAX_NUM_TOKENS; ++i)
	{
		for (ssgnc::Int32 j = 0; j <= MAX_TOKEN_ID; ++j)
		{
			ssgnc::NgramIndex::Entry entry;
			assert(ngram_index.get(i, j, &entry));

			ssgnc::UInt32 id = (MAX_NUM_TOKENS * j) + (i - 1);
			assert(entry.file_id() == entries[id].file_id());
			assert(entry.offset() == entries[id].offset());
			assert(entry.approx_size() >= 0);
			assert(entry.approx_size() == entries[id + MAX_NUM_TOKENS]
				- entries[id]);
		}
	}

	std::string str = stream.str();
	assert(ngram_index.map(str.data(), str.size()));

	assert(ngram_index.max_num_tokens() == MAX_NUM_TOKENS);
	assert(ngram_index.max_token_id() == MAX_TOKEN_ID);

	for (ssgnc::Int32 i = 1; i <= MAX_NUM_TOKENS; ++i)
	{
		for (ssgnc::Int32 j = 0; j <= MAX_TOKEN_ID; ++j)
		{
			ssgnc::NgramIndex::Entry entry;
			assert(ngram_index.get(i, j, &entry));

			ssgnc::UInt32 id = (MAX_NUM_TOKENS * j) + (i - 1);
			assert(entry.file_id() == entries[id].file_id());
			assert(entry.offset() == entries[id].offset());
			assert(entry.approx_size() >= 0);
			assert(entry.approx_size() == entries[id + MAX_NUM_TOKENS]
				- entries[id]);
		}
	}

	ngram_index.clear();

	assert(ngram_index.max_num_tokens() == 0);
	assert(ngram_index.max_token_id() == 0);

	return 0;
}
