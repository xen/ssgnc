#include "ssgnc.h"

#include <cassert>
#include <ctime>
#include <sstream>

int main()
{
	enum { NUM_OBJS = 1 << 16 };

	std::srand(static_cast<unsigned>(std::time(NULL)));

	std::vector<int> values;
	for (int i = 0; i < NUM_OBJS; ++i)
		values.push_back(std::rand());

	std::stringstream stream;
	for (int i = 0; i < NUM_OBJS; ++i)
	{
		stream.write(reinterpret_cast<const char *>(&values[i]),
			sizeof(values[i]));
	}

	ssgnc::Reader reader;

	assert(reader.is_open() == false);

	assert(reader.bad() == true);
	assert(reader.eof() == true);
	assert(reader.fail() == true);
	assert(reader.good() == false);

	assert(reader.tell() == 0);

	assert(reader.open(&stream));

	assert(reader.is_open() == true);

	assert(reader.bad() == false);
	assert(reader.eof() == false);
	assert(reader.fail() == false);
	assert(reader.good() == true);

	assert(reader.tell() == 0);

	for (int i = 0; i < NUM_OBJS; ++i)
	{
		int value;
		assert(reader.read(&value));
		assert(value == values[i]);
	}
	assert(reader.tell() == stream.tellg());

	std::vector<int> values_clone(NUM_OBJS);

	stream.seekg(0);

	reader.close();

	assert(reader.is_open() == false);

	assert(reader.bad() == true);
	assert(reader.eof() == true);
	assert(reader.fail() == true);
	assert(reader.good() == false);

	assert(reader.tell() == 0);

	assert(reader.open(&stream));
	assert(reader.read(&values_clone[0], NUM_OBJS));
	assert(reader.tell() == stream.tellg());

	for (int i = 0; i < NUM_OBJS; ++i)
		assert(values_clone[i] == values[i]);

	return 0;
}
