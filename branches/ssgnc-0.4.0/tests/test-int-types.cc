#include "ssgnc.h"

#include <cassert>

int main()
{
	assert(sizeof(ssgnc::Int8) == 1);
	assert(sizeof(ssgnc::Int16) == 2);
	assert(sizeof(ssgnc::Int32) == 4);
	assert(sizeof(ssgnc::Int64) == 8);

	assert(sizeof(ssgnc::UInt8) == 1);
	assert(sizeof(ssgnc::UInt16) == 2);
	assert(sizeof(ssgnc::UInt32) == 4);
	assert(sizeof(ssgnc::UInt64) == 8);

	return 0;
}
