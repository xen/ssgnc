#include "ssgnc.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <vector>

int main()
{
	enum { CHUNK_SIZE = ssgnc::MemPool::DEFAULT_CHUNK_SIZE };
	enum { BUF_SIZE = CHUNK_SIZE * 2 };
	enum { POOL_SIZE = 1024 };

	std::srand(static_cast<unsigned>(std::time(NULL)));

	ssgnc::MemPool mem_pool;

	assert(mem_pool.num_objs() == 0);
	assert(mem_pool.total_length() == 0);
	assert(mem_pool.total_size() == 0);

	char buf[BUF_SIZE];
	for (int i = 0; i < BUF_SIZE; ++i)
		buf[i] = 'A' + (std::rand() % 26);

	ssgnc::UInt32 num_objs = 0;
	ssgnc::UInt32 total_length = 0;
	ssgnc::UInt32 total_size = 0;
	ssgnc::UInt32 avail = 0;

	std::vector<ssgnc::String> originals;
	std::vector<ssgnc::String> copies;

	for (int i = 0; i < POOL_SIZE; ++i)
	{
		int pos = std::rand() % BUF_SIZE;
		ssgnc::UInt32 length = std::rand() % (BUF_SIZE - pos);

		ssgnc::String str(buf + pos, length);
		ssgnc::String str_copy = mem_pool.clone(str);

		assert(str == str_copy);
		assert(str.ptr() != str_copy.ptr());

		originals.push_back(str);
		copies.push_back(str_copy);

		++num_objs;
		assert(mem_pool.num_objs() == num_objs);

		total_length += length;
		assert(mem_pool.total_length() == total_length);

		if (length > avail)
		{
			ssgnc::UInt32 new_chunk_size = (length <= CHUNK_SIZE)
				? CHUNK_SIZE : length;
			avail = new_chunk_size;
			total_size += new_chunk_size;
		}
		avail -= length;

		assert(mem_pool.total_size() == total_size);
	}

	for (std::size_t i = 0; i < POOL_SIZE; ++i)
		assert(originals[i] == copies[i]);

	mem_pool.clear();
	assert(mem_pool.num_objs() == 0);
	assert(mem_pool.total_length() == 0);
	assert(mem_pool.total_size() == 0);

	return 0;
}
