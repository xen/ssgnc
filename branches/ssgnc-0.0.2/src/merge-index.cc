// This program encodes indices.

#include "ssgnc/merger.h"
#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"

#include <iostream>

int main()
{
	ssgnc::Timer timer;

	ssgnc::Merger merger(&std::cout);
	if (!merger.Init())
	{
		std::cerr << "error: failed to initialize encoder" << std::endl;
		return 1;
	}

	long long count = 0;
	const char *line;
	ssgnc::LineReader reader(&std::cin);
	while (reader.Read(&line))
	{
		if (!merger.Insert(line))
		{
			std::cerr << "error: failed to insert line: "
				<< line << std::endl;
			return 1;
		}

		if ((++count % 1000000) == 0)
		{
			std::cerr << "count: " << count
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}

	if (!merger.Finish())
	{
		std::cerr << "error: failed to finish merging" << std::endl;
		return 1;
	}

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << std::endl;

	return 0;
}
