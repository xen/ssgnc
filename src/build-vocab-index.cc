// This program builds an index for a vocabulary.

#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"

#include <cstdlib>
#include <iostream>
#include <vector>

int main()
{
	ssgnc::Timer timer;

	int count = 0;
	const char *line;
	std::size_t last_index = 0;
	ssgnc::LineReader reader(&std::cin);
	std::vector<std::size_t> index_vector;
	while (reader.Read(&line))
	{
		const char *delim = strchr(line, '\t');
		if (delim == NULL)
		{
			std::cerr << "error: failed to find delimitor: "
				<< line << std::endl;
			return 1;
		}
		std::size_t key_length = static_cast<std::size_t>(delim - line);

		// Keeps a start position of each key.
		index_vector.push_back(last_index);
		last_index += key_length + 1;

		// Writes a key with a delimitor.
		if (!(std::cout.write(line, key_length) << '\0'))
		{
			std::cerr << "error: failed to write key: " << line << std::endl;
			return 1;
		}

		if ((++count % 100000) == 0)
		{
			std::cerr << "count: " << count
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}
	index_vector.push_back(last_index);

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << '\r';

	// Writes paddings for alignment.
	for (std::size_t i = last_index; i % sizeof(last_index); ++i)
		std::cout << '\0';

	// Writes a list of start positions and its size.
	std::size_t index_size = index_vector.size() - 1;
	std::cout.write(reinterpret_cast<const char *>(&index_vector[0]),
		sizeof(index_vector[0]) * (index_size + 1));
	std::cout.write(reinterpret_cast<const char *>(&index_size),
		sizeof(index_size));
	if (!std::cout)
	{
		std::cerr << "error: failed to write index" << std::endl;
		return 1;
	}

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << std::endl;

	return 0;
}
