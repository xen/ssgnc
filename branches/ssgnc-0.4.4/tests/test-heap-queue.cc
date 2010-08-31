#include "ssgnc.h"

#include <algorithm>
#include <cassert>
#include <ctime>

int main()
{
	enum { NUM_VALUES = 1 << 16 };
	enum { MAX_VALUE = 1 << 16 };

	std::srand(static_cast<unsigned>(std::time(NULL)));

	std::vector<int> values;
	for (std::size_t i = 0; i < NUM_VALUES; ++i)
	{
		int value = std::rand() % MAX_VALUE;
		values.push_back(value);
	}

	ssgnc::HeapQueue<int> queue;
	for (std::size_t i = 0; i < values.size(); ++i)
		assert(queue.push(values[i]));

	std::vector<int> results;
	while (!queue.empty())
	{
		int value;
		assert(queue.top(&value));
		results.push_back(value);
		assert(queue.pop());
	}

	assert(results.size() == values.size());

	std::sort(values.begin(), values.end());
	for (std::size_t i = 0; i < values.size(); ++i)
		assert(results[i] == values[i]);

	return 0;
}
