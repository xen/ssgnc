#include "ssgnc/unified-db.h"
#include "ssgnc/unified-reader.h"
#include "ssgnc/vocab-pair.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

namespace {

// Parses command line options.
bool ParseOptions(int argc, char *argv[],
	boost::program_options::variables_map *vmap)
{
	namespace opts = boost::program_options;

	opts::options_description options("Options");
	options.add_options()
		("help,h", "display this help and exit")
		("dir,d", opts::value<std::string>(), "index directory (required)")
		("order,o", opts::value<std::string>()->default_value("UNORDERED"),
			"query order (unordered, ordered, phrase or fixed)")
		("results,r", opts::value<long long>()->default_value(20),
			"maximum number of results")
		("freq,f", opts::value<long long>()->default_value(0),
			"minimum frequency")
		("n-range,n", opts::value<std::string>(), "range of n (ex. 1-7)");

	opts::store(opts::parse_command_line(argc, argv, options), *vmap);
	opts::notify(*vmap);

	if (vmap->count("help") || !vmap->count("dir"))
	{
		std::cerr << options << std::endl;
		return false;
	}

	return true;
}

// Opens vocabulary files.
bool OpenVocab(const boost::program_options::variables_map &vmap,
	const ssgnc::PathGenerator &path_gen, ssgnc::VocabPair *vocab)
{
	if (!vocab->Open(path_gen))
	{
		std::cerr << "error: failed to open vocabulary files" << std::endl;
		return false;
	}
	return true;
}

// Opens databases.
bool OpenDb(const boost::program_options::variables_map &vmap,
	const ssgnc::PathGenerator &path_gen, ssgnc::UnifiedDb *db)
{
	if (vmap.count("n-range"))
	{
		int min_n = 1;
		int max_n = INT_MAX - 1;

		const std::string &n_range = vmap["n-range"].as<std::string>();
		std::string::size_type delim_pos = n_range.find('-');

		if (delim_pos != std::string::npos)
		{
			if (!n_range.empty() && delim_pos != 0)
				min_n = std::atoi(n_range.c_str());
			if (delim_pos != n_range.length() - 1)
				max_n = std::atoi(n_range.c_str() + delim_pos + 1);
		}
		else if (!n_range.empty())
			max_n = min_n = std::atoi(n_range.c_str());
		
		if (min_n <= 0 || min_n > max_n)
		{
			std::cerr << "error: invalid n range" << std::endl;
			return false;
		}

		if (!db->Open(path_gen, min_n, max_n))
		{
			std::cerr << "error: failed to open databases" << std::endl;
			return false;
		}
	}
	else if (!db->Open(path_gen))
	{
		std::cerr << "error: failed to open databases" << std::endl;
		return false;
	}
	return true;
}

bool InitQuery(const boost::program_options::variables_map &vmap,
	ssgnc::Query *query)
{
	const std::string &order_name = vmap["order"].as<std::string>();
	if (boost::istarts_with("UNORDERED", order_name))
		query->set_order(ssgnc::Query::UNORDERED);
	else if (boost::istarts_with("ORDERED", order_name))
		query->set_order(ssgnc::Query::ORDERED);
	else if (boost::istarts_with("PHRASE", order_name))
		query->set_order(ssgnc::Query::PHRASE);
	else if (boost::istarts_with("FIXED", order_name))
		query->set_order(ssgnc::Query::FIXED);
	else
	{
		std::cerr << "error: invalid order: " << order_name << std::endl;
		return false;
	}

	const long long min_freq = vmap["freq"].as<long long>();
	if (min_freq < 0)
	{
		std::cerr << "error: invalid min_freq: " << min_freq << std::endl;
		return false;
	}
	query->set_min_freq(min_freq);

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	boost::program_options::variables_map vmap;
	if (!ParseOptions(argc, argv, &vmap))
		return 1;

	// Paths of vocabulary files and databases.
	const std::string &dir_name = vmap["dir"].as<std::string>();
	ssgnc::PathGenerator path_gen;
	path_gen.set_dir_name(dir_name);

	// Opens vocabulary files.
	ssgnc::VocabPair vocab;
	if (!OpenVocab(vmap, path_gen, &vocab))
		return 1;

	// Opens databases.
	ssgnc::UnifiedDb db;
	if (!OpenDb(vmap, path_gen, &db))
		return 1;

	// Sets shared options of queries.
	ssgnc::Query query;
	if (!InitQuery(vmap, &query))
		return 1;

	const long long max_results = vmap["results"].as<long long>();
	std::string line;
	while (std::getline(std::cin, line))
	{
		// Parses a query into a list of keys.
		boost::replace_all(line, "\xE3\x80\x80", " ");
		boost::trim(line);
		std::vector<std::string> key_strings;
		boost::split(key_strings, line, boost::is_space(),
			boost::token_compress_on);

		query.clear_key_string();
		for (std::size_t i = 0; i < key_strings.size(); ++i)
		{
			if (query.order() == ssgnc::Query::FIXED && key_strings[i] == "*")
				query.add_key_string("");
			else
				query.add_key_string(key_strings[i]);
		}

		if (!vocab.FillQuery(&query))
		{
			std::cerr << "warning: failed to fill query" << std::endl;
			continue;
		}

		ssgnc::UnifiedReader reader;
		if (!reader.Open(db, query))
		{
			std::cerr << "warning: no matching n-grams" << std::endl;
			continue;
		}

		// Reads n-grams one by one.
		ssgnc::Ngram ngram;
		for (long long count = 0; count < max_results &&
			reader.Read(&ngram); ++count)
		{
			if (!vocab.FillNgram(&ngram))
			{
				std::cerr << "error: failed to restore n-gram" << std::endl;
				continue;
			}

			std::cout << ngram.freq() << ' ';
			std::cout << ngram.key_string(0);
			for (int i = 1; i < ngram.key_string_size(); ++i)
				std::cout << ' ' << ngram.key_string(i);
			std::cout << '\n';
		}
	}

	return 0;
}
