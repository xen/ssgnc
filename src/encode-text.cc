// This program encodes a text.

#include "ssgnc/encoder.h"
#include "ssgnc/line-reader.h"
#include "ssgnc/timer.h"
#include "ssgnc/varint-writer.h"
#include "ssgnc/vocab-dic.h"

#include <fstream>
#include <iostream>

namespace {

// Loads a dictionary.
bool LoadDic(const char *dic_file_name, ssgnc::VocabDic *dic)
{
	std::ifstream dic_file(dic_file_name, std::ios::binary);
	if (!dic_file)
	{
		std::cerr << "error: failed to open file: "
			<< dic_file_name << std::endl;
		return false;
	}
	return dic->ReadDic(&dic_file);
}

}  // namespace

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " DicFile" << std::endl;
		return 1;
	}

	const char *dic_file_name = argv[1];

	ssgnc::Timer timer;

	// Loads a dictionary.
	ssgnc::VocabDic dic;
	LoadDic(dic_file_name, &dic);
	std::cerr << "time: " << timer.Elapsed() << '\r';

	long long count = 0;
	const char *line;
	ssgnc::Encoder encoder(&std::cout);
	ssgnc::LineReader reader(&std::cin);
	while (reader.Read(&line))
	{
		if (!encoder.Encode(dic, line))
		{
			std::cerr << "error: failed to encode line: "
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
