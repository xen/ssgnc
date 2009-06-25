// This program encodes indices.

#include "ssgnc/encoder.h"
#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"

#include <iostream>

int main()
{
	ssgnc::Timer timer;

	ssgnc::Encoder encoder(&std::cout);
	if (!encoder.Init())
	{
		std::cerr << "error: failed to initialize encoder" << std::endl;
		return 1;
	}

	long long count = 0;
	const char *line;
	ssgnc::LineReader reader(&std::cin);
	while (reader.Read(&line))
	{
		if (!encoder.Encode(line))
		{
			std::cerr << "error: failed to encode line: "
				<< line << std::endl;
			return 1;
		}

		if ((++count % 1000000) == 0)
		{
			std::cerr << "count: " << count
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}

	if (!encoder.Finish())
	{
		std::cerr << "error: failed to finish encoding" << std::endl;
		return 1;
	}

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << std::endl;

	return 0;
}
