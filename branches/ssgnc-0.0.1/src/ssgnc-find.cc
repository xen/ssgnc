#include "ssgnc/index-reader.h"
#include "ssgnc/path-generator.h"
#include "ssgnc/text-finder.h"
#include "ssgnc/timer.h"
#include "ssgnc/vocab-dic.h"

#include <algorithm>
#include <climits>
#include <iostream>
#include <iterator>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>

namespace {

// Opens a file.
bool OpenFile(const std::string &path, ssgnc::FileMapper *file)
{
	if (!file->Map(path.c_str()))
	{
		std::cerr << "error: failed to map file: " << path << std::endl;
		return false;
	}
	return true;
}

// Filters out.
bool Filter(const std::string &line, const std::vector<std::string> &filters)
{
	if (filters.empty())
		return true;

	unsigned mask = 0;
	std::size_t start_pos = 0;
	do
	{
		std::size_t delim_pos = line.find_first_of(" \t", start_pos);
		for (std::size_t i = 0; i < filters.size(); ++i)
		{
			if ((mask >> i) & 1)
				continue;

			std::size_t length = delim_pos - start_pos;
			if (!line.compare(start_pos, length, filters[i]))
			{
				mask |= 1 << i;
				break;
			}
		}
		start_pos = delim_pos + 1;
	} while (line[start_pos - 1] != '\t');

	for (std::size_t i = 0; i < filters.size(); ++i)
	{
		if (!((mask >> i) & 1))
			return false;
	}
	return true;
}

// Tuple.
struct Tuple
{
	typedef ssgnc::IndexFinder::RangeType RangeType;

	std::string key_string;
	int key_id;
	RangeType range;

	Tuple() : key_string(), key_id(), range() {}

	bool operator<(const Tuple &target) const
	{
		if (range.second != target.range.second)
			return range.second < target.range.second;
		return range.first < target.range.first;
	}
};

}  // namespace

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " DataDir" << std::endl;
		return 1;
	}

	const std::string data_dir = argv[1];
	ssgnc::PathGenerator path_generator(data_dir);

	// Opens a dictionary.
	ssgnc::FileMapper dic_file;
	if (!OpenFile(path_generator.DicPath(), &dic_file))
		return 1;
	ssgnc::VocabDic dic(dic_file);

	// Opens indices.
	std::vector<boost::shared_ptr<ssgnc::FileMapper> > files;
	std::vector<boost::shared_ptr<ssgnc::IndexFinder> > index_finders;
	std::vector<boost::shared_ptr<ssgnc::TextFinder> > text_finders;
	for (int i = 1; ; ++i)
	{
		boost::shared_ptr<ssgnc::FileMapper>
			index_file(new ssgnc::FileMapper());
		if (!OpenFile(path_generator.IndexPath(i), index_file.get()))
			break;
		boost::shared_ptr<ssgnc::IndexFinder>
			index_finder(new ssgnc::IndexFinder(*index_file));

		boost::shared_ptr<ssgnc::FileMapper>
			text_file(new ssgnc::FileMapper());
		if (!OpenFile(path_generator.TextPath(i), text_file.get()))
			return 1;
		boost::shared_ptr<ssgnc::TextFinder>
			text_finder(new ssgnc::TextFinder(*text_file));

		files.push_back(index_file);
		files.push_back(text_file);
		index_finders.push_back(index_finder);
		text_finders.push_back(text_finder);
	}

	std::string query_string;
	while (std::getline(std::cin, query_string))
	{
		ssgnc::Timer timer;

		// Parses an input string.
		std::string::size_type delim_pos = query_string.find('\t');
		if (delim_pos == std::string::npos)
		{
			std::cerr << "error: invalid format: "
				<< query_string << std::endl;
			return 1;
		}
		query_string = query_string.substr(0, delim_pos);

		std::vector<std::string> key_strings;
		boost::split(key_strings, query_string, boost::is_any_of(" "));
		std::vector<Tuple> tuples(key_strings.size());
		for (std::size_t i = 0; i < tuples.size(); ++i)
			tuples[i].key_string = key_strings[i];

		// Gets key IDs.
		for (std::size_t i = 0; i < tuples.size(); ++i)
		{
			if (!dic.Find(tuples[i].key_string.c_str(), &tuples[i].key_id))
			{
				std::cerr << "error: failed to find key: "
					<< tuples[i].key_string << std::endl;
				return 1;
			}
		}

		long long count = 0;
		for (std::size_t n = tuples.size(); n <= index_finders.size(); ++n)
		{
			for (std::size_t i = 0; i < tuples.size(); ++i)
			{
				tuples[i].range = index_finders[n - 1]->Find(
					tuples[i].key_id);
			}
			std::sort(tuples.begin(), tuples.end());

			// Finds keys and outputs n-grams.
			std::vector<std::size_t> positions;
			{
				std::size_t position;
				ssgnc::IndexReader index_reader(tuples[0].range);
				while (index_reader.Read(&position))
					positions.push_back(position);
			}

			std::vector<std::string> filters;
			for (std::size_t i = 1; i < tuples.size(); ++i)
			{
				if (tuples[i].key_id == tuples[i - 1].key_id)
				{
					if (filters.empty() ||
						filters.back() != tuples[i].key_string)
						filters.push_back(tuples[i].key_string);
					filters.push_back(tuples[i].key_string);
					continue;
				}
				else if (tuples[i].range.second >= positions.size() << 16)
				{
					filters.push_back(tuples[i].key_string);
					continue;
				}

				std::size_t position;
				std::vector<std::size_t> candidate_positions;
				ssgnc::IndexReader index_reader(tuples[i].range);
				while (index_reader.Read(&position))
					candidate_positions.push_back(position);

				std::vector<std::size_t> intersections;
				std::set_intersection(positions.begin(), positions.end(),
					candidate_positions.begin(), candidate_positions.end(),
					std::back_inserter(intersections));

				positions.swap(intersections);
			}

			std::string line;
			for (std::size_t i = 0; i < positions.size(); ++i)
			{
				if (!text_finders[n - 1]->Find(positions[i], &line))
				{
					std::cerr << "error: failed to find n-gram: "
						<< positions[i] << std::endl;
					return 1;
				}

				if (!Filter(line, filters))
					continue;

//				std::cout << line << '\n';
				++count;
			}
		}
		std::cerr << count << ' ' << timer.Elapsed() << '\n';
		std::cout << count << ' ' << timer.Elapsed() << '\n';
	}

	return 0;
}
