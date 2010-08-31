#include "ssgnc.h"

#include <cassert>
#include <sstream>

int main()
{
	std::stringstream stream;

	ssgnc::String src = "This is a pen.";

	stream << src;

	ssgnc::ByteReader byte_reader;

	assert(byte_reader.tell() == 0);

	assert(byte_reader.open(&stream));

	for (ssgnc::UInt32 i = 0; i < src.length(); ++i)
	{
		ssgnc::Int32 byte;
		assert(byte_reader.read(&byte));
		assert(byte == src[i]);
	}

	assert(byte_reader.tell() == src.length());

	ssgnc::Int32 byte;
	assert(byte_reader.read(&byte));
	assert(byte == EOF);

	assert(byte_reader.tell() == src.length());

	byte_reader.close();

	assert(byte_reader.tell() == 0);

	return 0;
}
