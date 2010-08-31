#include "ssgnc.h"

#include <cassert>
#include <cstring>
#include <sstream>

#include <iostream>

int main()
{
	const char *src = "This is a pen.";
	ssgnc::String str = src;

	assert(str.ptr() == src);
	assert(str.length() == static_cast<ssgnc::UInt32>(std::strlen(src)));

	assert(str == "This is a pen.");
	assert(str != "This is a pen. EX");

	ssgnc::UInt32 pos;
	assert(str.first(' ', &pos));
	assert(pos == 4);
	assert(!str.first('X', &pos));

	assert(str.last(' ', &pos));
	assert(pos == 9);
	assert(!str.last('X', &pos));

	assert(str.substr(3) == "s is a pen.");
	assert(str.substr(5, 4) == "is a");

	assert(str < "X");
	assert(str <= src);
	assert(str > "A");
	assert(str >= src);

	return 0;
}
