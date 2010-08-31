#include "ssgnc.h"

#include <cassert>

int main()
{
	ssgnc::StringBuilder string_builder;

	assert(string_builder[0] == '\0');
	assert(string_builder.buf() != NULL);
	assert(string_builder.length() == 0);
	assert(string_builder.size() == sizeof(char *));

	string_builder.append('a');
	assert(string_builder.str() == "a");

	string_builder.clear();

	assert(string_builder[0] == '\0');
	assert(string_builder.buf() != NULL);
	assert(string_builder.length() == 0);
	assert(string_builder.size() == sizeof(char *));

	string_builder.append("aBc");
	assert(string_builder.str() == "aBc");

	assert(string_builder.length() == 3);
	assert(string_builder.size() == sizeof(char *));

	string_builder.append('Z');
	assert(string_builder[3] == 'Z');

	string_builder.resize(3);
	assert(string_builder[3] == 'Z');

	string_builder.append();
	assert(string_builder.length() == 3);
	assert(string_builder[3] == '\0');

	string_builder.resize(31);
	assert(string_builder.length() == 31);
	assert(string_builder.size() == 32);

	string_builder.resize(128);
	assert(string_builder.length() == 128);
	assert(string_builder.size() == 256);

	string_builder = "ABC";
	assert(string_builder.str() == "ABC");

	assert(string_builder.assign(ssgnc::String("012345", 3)).str() == "012");

	return 0;
}
