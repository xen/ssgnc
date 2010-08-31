#include "ssgnc.h"

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <vector>

int main()
{
	enum { NUM_OBJS = 1 << 16 };

	std::srand(static_cast<unsigned>(std::time(NULL)));

	std::vector<int> values;
	for (int i = 0; i < NUM_OBJS; ++i)
		values.push_back(std::rand());

	std::stringstream stream;

	ssgnc::Writer writer;
	assert(writer.open(&stream));
	for (int i = 0; i < NUM_OBJS; ++i)
		assert(writer.write(values[i]));
	assert(writer.tell() == stream.tellp());

	for (int i = 0; i < NUM_OBJS; ++i)
	{
		int value;
		stream.read(reinterpret_cast<char *>(&value), sizeof(value));
		assert(value == values[i]);
	}

	stream.seekg(0);
	stream.seekp(0);

	writer.close();
	assert(writer.open(&stream));
	assert(writer.write(&values[0], NUM_OBJS));
	assert(writer.tell() == stream.tellp());

	for (int i = 0; i < NUM_OBJS; ++i)
	{
		int value;
		stream.read(reinterpret_cast<char *>(&value), sizeof(value));
		assert(value == values[i]);
	}

	return 0;
}
