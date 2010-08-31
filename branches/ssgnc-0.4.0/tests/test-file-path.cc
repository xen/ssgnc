#include "ssgnc.h"

#include <cassert>

int main()
{
	ssgnc::FilePath file_path;

	assert(file_path.format() == "");
	assert(file_path.path() == "");
	assert(file_path.file_id() == -1);

	file_path.set_format("prefix-%04d.ext");

	assert(file_path.format() == "prefix-%04d.ext");
	assert(file_path.path() == "");
	assert(file_path.file_id() == -1);

	assert(file_path.next());

	assert(file_path.format() == "prefix-%04d.ext");

	assert(file_path.path() == "prefix-0000.ext");
	assert(file_path.file_id() == 0);

	assert(file_path.next());

	assert(file_path.format() == "prefix-%04d.ext");
	assert(file_path.path() == "prefix-0001.ext");
	assert(file_path.file_id() == 1);

	file_path.clear();

	assert(file_path.format() == "prefix-%04d.ext");
	assert(file_path.path() == "");
	assert(file_path.file_id() == -1);

	return 0;
}
