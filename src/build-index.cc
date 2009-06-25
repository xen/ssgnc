// This program builds indices from sorted n-grams.

#include "ssgnc/indexer.h"
#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"

#include <iostream>

int main()
{
	ssgnc::Timer timer;

	ssgnc::Indexer indexer;
	if (!indexer.ReadDic(&std::cin))
	{
		std::cerr << "error: failed to restore dictionary" << std::endl;
		return 1;
	}
	std::cerr << "time: " << timer.Elapsed() << '\r';

	long long count = 0;
	const char *line;
	std::size_t length;
	ssgnc::LineReader reader(&std::cin);
	while (reader.Read(&line, &length))
	{
		if (!indexer.Index(line, length, &std::cout))
		{
			std::cerr << "error: failed to index line: "
				<< line << std::endl;
			return 1;
		}

		if ((++count % 100000) == 0)
		{
			std::cerr << "count: " << count
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}

	std::cerr << "count: " << count
		<< ", time: " << timer.Elapsed() << std::endl;

	return 0;
}
