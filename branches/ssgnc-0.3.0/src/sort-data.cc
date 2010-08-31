// This program sorts encoded n-grams in descending frequency order.

#include "ssgnc/data-divider.h"
#include "ssgnc/data-merger.h"
#include "ssgnc/tempfile-manager.h"
#include "ssgnc/timer.h"
#include "ssgnc/varint-writer.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
			<< " N MemorySize TempDir" << std::endl;
		return 1;
	}

	const char *n_string = argv[1];
	const char *memory_size_string = argv[2];
	const char *temp_dir = argv[3];

	char *end;
	long value = std::strtol(n_string, &end, 10);
	if (value <= 0L || !end != '\0')
	{
		std::cerr << "error: invalid N value: " << n_string << std::endl;
		return 1;
	}
	int n = static_cast<int>(value);

	value = std::strtol(memory_size_string, &end, 10);
	if (value <= 0L || !end != '\0')
	{
		std::cerr << "error: invalid memory size: "
			<< memory_size_string << std::endl;
		return 1;
	}
	int memory_size = static_cast<int>(value);

	ssgnc::Timer timer;

	// Divides n-gram data into separated sorted chunks.
	ssgnc::TempfileManager tempfile_manager(temp_dir);
	ssgnc::DataDivider divider;
	divider.Init(n, memory_size, &tempfile_manager);
	ssgnc::ByteReader byte_reader(&std::cin);
	ssgnc::VarintReader reader(&byte_reader);
	while (divider.Next(&reader))
	{
		if ((divider.total_count() % 100000) == 0)
		{
			std::cerr << "count: " << divider.total_count()
				<< ", time: " << timer.Elapsed() << '\r';
		}
	}
	std::cerr << "count: " << divider.total_count()
		<< ", time: " << timer.Elapsed() << std::endl;
	divider.Clear();

	for (std::size_t i = 0; i < tempfile_manager.size(); ++i)
		::lseek(tempfile_manager[i], 0, SEEK_SET);

	// Merges sorted chunks.
	long long output_count = 0;
	ssgnc::DataMerger merger;
	merger.Init(n, tempfile_manager);
	ssgnc::ByteWriter byte_writer(&std::cout, 1 << 20);
	ssgnc::VarintWriter writer(&byte_writer);
	while (merger.Next(&writer))
	{
		if ((++output_count % 100000) == 0)
		{
			std::cerr << "count: " << output_count <<
				", time: " << timer.Elapsed() << '\r';
		}
	}
	std::cerr << "count: " << output_count <<
		", time: " << timer.Elapsed() << std::endl;

	return 0;
}
