#include "ssgnc.h"

#include <cassert>

int main()
{
	ssgnc::FilePath file_path;

	assert(file_path.dirname() == "");
	assert(file_path.basename() == "");
	assert(file_path.tell() == 0);

	assert(file_path.open(".", "prefix-%04d.ext"));

	assert(file_path.dirname() == ".");
	assert(file_path.basename() == "prefix-%04d.ext");
	assert(file_path.tell() == 0);

	ssgnc::StringBuilder path;

	assert(file_path.read(&path));
	assert(path.str() == "./prefix-0000.ext");

	assert(file_path.dirname() == ".");
	assert(file_path.basename() == "prefix-%04d.ext");
	assert(file_path.tell() == 1);

	assert(file_path.read(&path));
	assert(path.str() == "./prefix-0001.ext");

	assert(file_path.dirname() == ".");
	assert(file_path.basename() == "prefix-%04d.ext");
	assert(file_path.tell() == 2);

	file_path.close();

	assert(file_path.dirname() == "");
	assert(file_path.basename() == "");
	assert(file_path.tell() == 0);

	return 0;
}
