#include "ssgnc.h"

#include <cassert>

int main()
{
	ssgnc::StringBuilder string_builder;

	assert(string_builder[0] == '\0');
	assert(string_builder.buf() != NULL);
	assert(string_builder.length() == 0);
	assert(string_builder.size() == sizeof(char *));

	assert(string_builder.append('a'));
	assert(string_builder.str() == "a");

	string_builder.clear();

	assert(string_builder.buf() != NULL);
	assert(string_builder.length() == 0);
	assert(string_builder.size() == sizeof(char *));

	assert(string_builder.append("aBc"));
	assert(string_builder.str() == "aBc");

	assert(string_builder.length() == 3);
	assert(string_builder.size() == sizeof(char *));

	assert(string_builder.append('Z'));
	assert(string_builder[3] == 'Z');

	assert(string_builder.resize(3));
	assert(string_builder[3] == 'Z');

	assert(string_builder.append());
	assert(string_builder.length() == 3);
	assert(string_builder[3] == '\0');

	assert(string_builder.resize(31));
	assert(string_builder.length() == 31);
	assert(string_builder.size() == 32);

	assert(string_builder.resize(33));
	assert(string_builder.length() == 33);
	assert(string_builder.size() == 64);

	assert(string_builder.resize(128));
	assert(string_builder.length() == 128);
	assert(string_builder.size() == 128);

	ssgnc::StringBuilder format_builder;

	assert(format_builder.appendf("%s/%dgms-%04d", "/tmp", 3, 5));
	assert(format_builder.str() == "/tmp/3gms-0005");

	assert(format_builder.appendf("%s", "+alpha"));
	assert(format_builder.str() == "/tmp/3gms-0005+alpha");

	return 0;
}
