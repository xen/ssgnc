#include "ssgnc.h"

#include <cassert>
#include <sstream>

int main()
{
	std::stringstream stream;

	ssgnc::String src = "This is a pen.";

	stream << src;

	ssgnc::ByteReader byte_reader;

	assert(byte_reader.is_open() == false);

	assert(byte_reader.bad() == true);
	assert(byte_reader.eof() == true);
	assert(byte_reader.fail() == true);
	assert(byte_reader.good() == false);

	assert(byte_reader.tell() == 0);

	assert(byte_reader.open(&stream));

	assert(byte_reader.is_open() == true);

	assert(byte_reader.bad() == false);
	assert(byte_reader.eof() == false);
	assert(byte_reader.fail() == false);
	assert(byte_reader.good() == true);

	assert(byte_reader.tell() == 0);

	for (ssgnc::UInt32 i = 0; i < src.length(); ++i)
	{
		ssgnc::Int8 byte;
		assert(byte_reader.read(&byte));
		assert(byte == src[i]);
	}

	assert(byte_reader.is_open() == true);

	assert(byte_reader.bad() == false);
	assert(byte_reader.eof() == true);
	assert(byte_reader.fail() == true);
	assert(byte_reader.good() == false);

	assert(byte_reader.tell() == src.length());

	ssgnc::Int8 byte;
	assert(!byte_reader.read(&byte));

	assert(byte_reader.is_open() == true);

	assert(byte_reader.bad() == false);
	assert(byte_reader.eof() == true);
	assert(byte_reader.fail() == true);
	assert(byte_reader.good() == false);

	assert(byte_reader.tell() == src.length());

	byte_reader.close();

	assert(byte_reader.is_open() == false);

	assert(byte_reader.bad() == true);
	assert(byte_reader.eof() == true);
	assert(byte_reader.fail() == true);
	assert(byte_reader.good() == false);

	assert(byte_reader.tell() == 0);

	return 0;
}
