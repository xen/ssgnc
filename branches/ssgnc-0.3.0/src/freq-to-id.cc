// This program replaces a frequency with an order (a key ID).

#include <cstring>
#include <iostream>

#include "ssgnc/line-reader.h"

int main()
{
	int key_id = 0;
	const char *line;
	ssgnc::LineReader reader(&std::cin);
	while (reader.Read(&line))
	{
		const char *delim = std::strchr(line, '\t');
		if (delim == NULL)
		{
			std::cerr << "error: failed to find delimitor: "
				<< line << std::endl;
			return 1;
		}
		std::size_t key_length = static_cast<std::size_t>(delim - line);
		std::cout.write(line, key_length) << '\t' << key_id++ << '\n';
	}

	return 0;
}
