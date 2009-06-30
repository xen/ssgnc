// This program builds a dictionary from sorted unigrams.

#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"

#include <iostream>
#include <string>

#include <dawgdic/dawg-builder.h>
#include <dawgdic/dictionary-builder.h>

int main()
{
	ssgnc::Timer timer;

	// Builds a dawg.
	int key_id = 0;
	const char *line;
	ssgnc::LineReader reader(&std::cin);
	dawgdic::DawgBuilder builder;
	while (reader.Read(&line))
	{
		const char *delim = strchr(line, '\t');
		if (delim == NULL)
		{
			std::cerr << "error: failed to find delimitor: "
				<< line << std::endl;
			return 1;
		}

		std::size_t length = static_cast<std::size_t>(delim - line);
		if (!builder.Insert(line, length, key_id++))
		{
			std::cerr << "error: failed to insert key: " << line << std::endl;
			return 1;
		}

		if ((key_id % 100000) == 0)
		{
			std::cerr << "count: " << key_id
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}
	dawgdic::Dawg dawg;
	builder.Finish(&dawg);

	std::cerr << "count: " << key_id
		<< ", time: " << timer.Elapsed() << '\r';

	// Builds a dictionary.
	dawgdic::Dictionary dic;
	if (!dawgdic::DictionaryBuilder::Build(dawg, &dic))
	{
		std::cerr << "error: failed to build dictionary" << std::endl;
		return 1;
	}

	// Saves a dictionary.
	if (!std::cout.write(reinterpret_cast<const char *>(&key_id),
		sizeof(key_id)) || !dic.Write(&std::cout))
	{
		std::cerr << "error: failed to save dictionary" << std::endl;
		return 1;
	}

	std::cerr << "count: " << key_id
		<< ", time: " << timer.Elapsed() << std::endl;

	return 0;
}
