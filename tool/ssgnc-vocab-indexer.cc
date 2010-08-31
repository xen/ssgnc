#include <ssgnc/vocab-indexer.h>

#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0]
			<< " DicFile IndexFile" << std::endl;
		return 1;
	}

	const char *dic_file_name = argv[1];
	const char *index_file_name = argv[2];

	try
	{
		ssgnc::VocabIndexer indexer;
		indexer.ReadVocab(&std::cin);
		std::cerr << "no. keys: " << indexer.num_keys() << std::endl;

		std::ofstream dic_file(dic_file_name, std::ios::binary);
		if (!dic_file)
		{
			std::cerr << "error: failed to create dictionary file: "
				<< dic_file_name << std::endl;
			return 1;
		}

		std::ofstream index_file(index_file_name, std::ios::binary);
		if (!index_file)
		{
			std::cerr << "error: failed to create index file: "
				<< dic_file_name << std::endl;
			return 1;
		}

		indexer.Build(&index_file, &dic_file);
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
