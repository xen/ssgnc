#include <ssgnc/line-reader.h>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void TestLineReader(const std::vector<std::string> &lines)
{
	std::stringstream stream;
	for (std::size_t i = 0; i < lines.size(); ++i)
		stream << lines[i] << '\n';

	ssgnc::LineReader reader(&stream, lines.size() - 1);
	bool catches_exception = false;
	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		const char *line;
		std::size_t length;

		try
		{
			assert(reader.Read(&line, &length));
			assert(std::string(line, length) == lines[i]);
		}
		catch (const std::exception &ex)
		{
			if (i < reader.buf_size())
				throw ex;

			if (catches_exception)
				throw ex;
			catches_exception = true;
		}
	}
	assert(catches_exception);

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

	std::cerr << "LineReader(" << (NUM_LINES - 1) << "): ";
	TestLineReader(lines);

	return 0;
}
