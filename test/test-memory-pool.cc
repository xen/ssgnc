#include <ssgnc/memory-pool.h>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void TestMemoryPool(const std::vector<std::string> &lines)
{
	ssgnc::MemoryPool pool(lines.size() / 2);
	std::vector<const char *> keys;
	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		const char *key = pool.AppendString(lines[i].c_str());
		assert(key != NULL);
		assert(lines[i] == key);
		keys.push_back(key);
	}
	assert(keys.size() == lines.size());

	for (std::size_t i = 0; i < lines.size(); ++i)
		assert(lines[i] == keys[i]);

	std::cerr << "ok" << std::endl;
}

int main()
{
	std::srand(static_cast<unsigned int>(std::time(NULL)));

	static const std::size_t NUM_LINES = (1 << 12) + 1;

	std::vector<char> line(NUM_LINES);
	std::vector<std::string> lines(NUM_LINES);
	for (std::size_t i = 0; i < NUM_LINES; ++i)
	{
		for (std::size_t j = 0; j < i; ++j)
			line[j] = 'A' + (std::rand() % 26);
		lines[i].assign(&line[0], i);
	}

	std::cerr << "MemoryPool(" << NUM_LINES / 2 << "): ";
	TestMemoryPool(lines);

	return 0;
}
