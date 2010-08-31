#include <ssgnc/exception.h>
#include <ssgnc/front-encoder.h>
#include <ssgnc/front-merger.h>
#include <ssgnc/back-divider.h>
#include <ssgnc/back-merger.h>

#include <cstring>
#include <iostream>
#include <vector>

void FrontProcess(const char *dic_file_name, const char *front_file_name,
	const char *back_file_name, std::istream *in,
	ssgnc::TempFile *back_file, std::vector<std::size_t> *points)
{
	// This local temporary file stores encoded lines.
	ssgnc::TempFile front_file;
	if (std::strcmp(front_file_name, "-") != 0)
		front_file.Open(front_file_name);
	else
		front_file.Open();

	// This step encodes each line of the N-gram corpus.
	// The encoded lines are written into the local temporary file in which
	// lines are ordered in value order per block.
	ssgnc::FrontEncoder encoder;
	encoder.Open(dic_file_name);
	encoder.Encode(in, &front_file, points);
	encoder.Close();

	std::cerr << "no. front blocks: " << (points->size() - 1) << std::endl;

	// This instance is used to merge lines in different blocks.
	ssgnc::FrontMerger front_merger;
	front_merger.Open(&front_file, *points);

	// The given temporary file stores an intermediate of inverted database.
	if (std::strcmp(back_file_name, "-") != 0)
		back_file->Open(back_file_name);
	else
		back_file->Open();

	// The next step merges lines in the first temporary file, and builds an
	// intermediate of inverted database.
	// The result file consists of blocks and copied lines in each block are
	// ordered in order of the additional key.
	// The additional key is one of the keys in each line.
	ssgnc::BackDivider divider;
	divider.Open(back_file, points);

	long long count = 0;
	while (front_merger.Next())
	{
		divider.Divide(front_merger.parsed_line(),
			front_merger.bytes(), front_merger.length());

		if (++count % 100000 == 0)
			std::cerr << "\rcount: " << count;
	}
	std::cerr << "\rcount: " << count << std::endl;

	// Frees memory allocated for encoding and merging lines.
	front_merger.Close();
	front_file.Close();

	// Frees memory allocated for building the blocks of inverted database.
	divider.Close();

	std::cerr << "no. back blocks: " << (points->size() - 1) << std::endl;
}

void BackProcess(ssgnc::TempFile *back_file,
	std::vector<std::size_t> *points, std::ostream *out)
{
	// This instance merges the blocks of the intermediate.
	ssgnc::BackMerger back_merger;
	back_merger.Open(back_file, *points);

	int last_key = -1;
	std::size_t file_pos = 0;
	points->clear();

	long long count = 0;
	while (back_merger.Next())
	{
		// Keeps the start position of each key in order to generate a footer.
		for ( ; last_key < back_merger.key(); ++last_key)
		{
			try
			{
				points->push_back(file_pos);
			}
			catch (...)
			{
				SSGNC_THROW("failed to build database: "
					"std::vector::push_back() failed");
			}
		}

		out->write(back_merger.bytes(), back_merger.length());
		file_pos += back_merger.length();

		if (++count % 100000 == 0)
			std::cerr << "\rcount: " << count;
	}
	try
	{
		points->push_back(file_pos);
	}
	catch (...)
	{
		SSGNC_THROW("failed to build database: "
			"std::vector::push_back() failed");
	}
	std::cerr << "\rcount: " << count << std::endl;
	back_file->Close();

	// Creates a footer of the inverted database.
	while (file_pos % sizeof(std::size_t) != 0)
	{
		if (!out->put('\0'))
		{
			SSGNC_THROW("failed to build database: "
				"std::ostream::put() failed");
		}
		++file_pos;
	}

	if (!out->write(reinterpret_cast<const char *>(&(*points)[0]),
		sizeof((*points)[0]) * points->size()))
		SSGNC_THROW("failed to build database: std::ostream::write() failed");

	int num_points = static_cast<int>(points->size());
	if (!out->write(reinterpret_cast<const char *>(&num_points),
		sizeof(num_points)))
		SSGNC_THROW("failed to build database: std::ostream::write() failed");
}

int main(int argc, char *argv[])
{
	if (argc < 2 || argc > 4)
	{
		std::cerr << "Usage: " << argv[0]
			<< " DicFile [1stTempFIle] [2ndTempFile]" << std::endl;
		return 1;
	}

	const char *dic_file_name = argv[1];
	const char *front_file_name = (argc > 2) ? argv[2] : "-";
	const char *back_file_name = (argc > 3) ? argv[3] : "-";

	// Creates a vector to store block positions.
	std::vector<std::size_t> points;

	// This temporary file stores an intermediate of inverted database.
	ssgnc::TempFile back_file;
	FrontProcess(dic_file_name, front_file_name, back_file_name,
		&std::cin, &back_file, &points);

	BackProcess(&back_file, &points, &std::cout);

	return 0;
}
