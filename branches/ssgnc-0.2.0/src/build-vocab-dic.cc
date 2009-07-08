// This program builds a dictionary from a sorted vocabulary.

#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"

#include <cstdlib>
#include <iostream>

#include <dawgdic/dawg-builder.h>
#include <dawgdic/dictionary-builder.h>

int main()
{
	ssgnc::Timer timer;

	// Builds a dawg.
	int count = 0;
	const char *line;
	ssgnc::LineReader reader(&std::cin);
	dawgdic::DawgBuilder builder;
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

		// Checks the format of lines.
		char *end;
		int key_id = std::strtol(delim + 1, &end, 10);
		if (key_id < 0 || *end != '\0')
		{
			std::cerr << "error: invalid key ID: " << line << std::endl;
			return 1;
		}

		// Inserts keys in dictionary order.
		if (!builder.Insert(line, key_length, key_id))
		{
			std::cerr << "error: failed to insert key: " << line << std::endl;
			return 1;
		}

		if ((++count % 100000) == 0)
		{
			std::cerr << "count: " << count
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}
	dawgdic::Dawg dawg;
	builder.Finish(&dawg);

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << '\r';

	// Builds a dictionary.
	dawgdic::Dictionary dic;
	if (!dawgdic::DictionaryBuilder::Build(dawg, &dic))
	{
		std::cerr << "error: failed to build dictionary" << std::endl;
		return 1;
	}

	// Saves a dictionary.
	if (!std::cout.write(reinterpret_cast<const char *>(&count),
		sizeof(count)) || !dic.Write(&std::cout))
	{
		std::cerr << "error: failed to save dictionary" << std::endl;
		return 1;
	}

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << std::endl;

	return 0;
}
