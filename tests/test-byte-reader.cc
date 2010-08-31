#include "ssgnc.h"

#include <cassert>
#include <sstream>

int main()
{
	std::stringstream stream;

	ssgnc::String src = "This is a pen.";

	stream << src;

	ssgnc::ByteReader byte_reader;

	assert(byte_reader.total() == 0);

	assert(byte_reader.open(&stream));

	for (ssgnc::UInt32 i = 0; i < src.length(); ++i)
		assert(byte_reader.read() == src[i]);

	assert(byte_reader.total() == src.length());

	assert(byte_reader.read() == -1);

	assert(byte_reader.total() == src.length());

	byte_reader.close();

	assert(byte_reader.total() == 0);

	return 0;
}
