#include <ssgnc/front-encoder.h>

#include <ssgnc/exception.h>
#include <ssgnc/line-reader.h>
#include <ssgnc/line-parser.h>
#include <ssgnc/parsed-line-pool.h>

namespace ssgnc {
namespace {

void Flush(ParsedLinePool *pool, TempFile *temp_file,
	std::vector<std::size_t> *points)
{
	pool->Sort();

	temp_file->Write(pool->num_keys());
	temp_file->Write(pool->num_lines());
	pool->Write(temp_file);

	try
	{
		points->push_back(temp_file->Tell());
	}
	catch (...)
	{
		SSGNC_THROW("failed to flush pool: std::vector::push_back() failed");
	}

	pool->Clear();
}

}  // namespace

void FrontEncoder::Open(const char *dic_file_name)
{
	FrontEncoder temp;

	temp.dic_.Open(dic_file_name);

	Swap(&temp);
}

void FrontEncoder::Close()
{
	dic_.Clear();
}

void FrontEncoder::Encode(std::istream *in, TempFile *temp_file,
	std::vector<std::size_t> *points)
{
	LineParser parser(dic_);
	ParsedLine parsed_line;
	ParsedLinePool parsed_line_pool;

	points->clear();
	try
	{
		points->push_back(temp_file->Tell());
	}
	catch (...)
	{
		SSGNC_THROW("failed to encode lines: std::vector::push_back() failed");
	}

	LineReader reader(in);
	const char *line;
	long long count = 0;
	while (reader.Read(&line))
	{
		parser.Parse(line, &parsed_line);
		parsed_line_pool.Append(parsed_line);
		if (parsed_line_pool.num_lines() >= MAX_NUM_LINES ||
			parsed_line_pool.total_size() >= MAX_TOTAL_SIZE)
			Flush(&parsed_line_pool, temp_file, points);

		if (++count % 100000 == 0)
			std::cerr << "\rcount: " << count;
	}
	if (parsed_line_pool.num_lines() > 0)
		Flush(&parsed_line_pool, temp_file, points);
	std::cerr << "\rcount: " << count << std::endl;
}

void FrontEncoder::Swap(FrontEncoder *target)
{
	dic_.Swap(&target->dic_);
}

}  // namespace ssgnc
