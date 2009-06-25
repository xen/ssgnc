// This program builds indices from encoded n-grams.

#include "ssgnc/indexer.h"
#include "ssgnc/timer.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " N" << std::endl;
		return 1;
	}

	int n = std::atoi(argv[1]);
	if (n <= 0)
	{
		std::cerr << "error: invalid argument: " << argv[1] << std::endl;
		return 1;
	}

	ssgnc::Timer timer;

	long long count = 0;
	ssgnc::Indexer indexer(&std::cin);
	while (indexer.Index(n, &std::cout))
	{
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
