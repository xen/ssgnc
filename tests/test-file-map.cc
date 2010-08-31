#include "ssgnc.h"

#include <cassert>

int main()
{
	ssgnc::String src = "This is a pen.";

	std::ofstream file("FILE_MAP");
	assert(file.good());

	static_cast<std::ostream &>(file) << src;
	assert(file.good());

	file.close();

	ssgnc::FileMap file_map;

	assert(file_map.ptr() == NULL);
	assert(file_map.size() == 0);

	assert(file_map.open("FILE_MAP"));

	assert(ssgnc::String(static_cast<const ssgnc::Int8 *>(file_map.ptr()),
		file_map.size()) == src);

	file_map.close();

	return 0;
}
