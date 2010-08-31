#include "ssgnc.h"

#include <cassert>
#include <cstdlib>
#include <ctime>

bool openNextFile(ssgnc::FilePath *file_path, std::ofstream *file)
{
	ssgnc::StringBuilder path;
	if (!file_path->read(&path))
		return false;

	file->close();
	file->open(path.ptr(), std::ios::binary);
	assert(file->good());

	return true;
}

bool writeValue(ssgnc::Int32 value, std::ostream *out)
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
		out->put(temp_buf[num_bytes - i] | 0x80);
	out->put(temp_buf[0]);

	if (!*out)
		return false;
	return true;
}

int main()
{
	enum { NUM_TOKENS = 3, MAX_TOKEN_ID = 255, MAX_NUM_NGRAMS = 32 };
	enum { MAX_FREQ = 1000, FILE_SIZE = 1024 };

	std::srand(static_cast<unsigned>(std::time(NULL)));

	ssgnc::FilePath file_path;
	assert(file_path.open("3gm-%04d.db"));

	std::ofstream file;
	assert(openNextFile(&file_path, &file));

	std::vector<ssgnc::Int32> file_ids(1, 0);
	std::vector<ssgnc::Int32> offsets(1, 0);

	std::vector<ssgnc::Int16> src_freqs;
	std::vector<ssgnc::Int32> src_tokens;
	for (int i = 0; i < MAX_TOKEN_ID; ++i)
	{
		int num_ngrams = std::rand() % (MAX_NUM_NGRAMS + 1);
		for (int j = 0; j < num_ngrams; ++j)
		{
			src_freqs.push_back(static_cast<ssgnc::Int16>(
				1 + (std::rand() % MAX_FREQ)));
			assert(writeValue(src_freqs.back(), &file));
			for (int k = 0; k < NUM_TOKENS; ++k)
			{
				src_tokens.push_back(std::rand() % (MAX_TOKEN_ID + 1));
				assert(writeValue(src_tokens.back(), &file));
			}

			if (src_freqs.size() % 1024 == 0)
				assert(openNextFile(&file_path, &file));
		}
		assert(writeValue(0, &file));
		file_ids.push_back(file_path.tell() - 1);
		offsets.push_back(static_cast<ssgnc::Int32>(file.tellp()));
	}

	file.close();

	ssgnc::NgramReader ngram_reader;

	ssgnc::Int16 freq;
	std::vector<ssgnc::Int32> tokens;

	std::size_t src_id = 0;
	for (int i = 0; i < MAX_TOKEN_ID; ++i)
	{
		assert(ngram_reader.open(3, "3gm-%04d.db", file_ids[i], offsets[i]));
		while (ngram_reader.read(&freq, &tokens))
		{
			assert(freq == src_freqs[src_id]);
			for (int j = 0; j < NUM_TOKENS; ++j)
				assert(tokens[j] == src_tokens[(NUM_TOKENS * src_id) + j]);
			++src_id;
		}
	}
	assert(src_id == src_freqs.size());

	return 0;
}
