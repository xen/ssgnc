#include "ssgnc/unified-db.h"
#include "ssgnc/unified-reader.h"
#include "ssgnc/vocab-pair.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

namespace {

bool AnalyzeOptions(int argc, char *argv[],
	boost::program_options::variables_map *vmap)
{
	namespace opts = boost::program_options;

	opts::options_description options("Options");
	options.add_options()
		("help,h", "display this help and exit")
		("dir,d", opts::value<std::string>(), "index directory (required)")
		("order,o", opts::value<std::string>()->default_value("unordered"),
			"query order (unordered, ordered, phrase or fixed)")
		("results,r", opts::value<long long>()->default_value(20),
			"maximum number of results")
		("freq,f", opts::value<long long>()->default_value(0),
			"minimum frequency");

	opts::store(opts::parse_command_line(argc, argv, options), *vmap);
	opts::notify(*vmap);

	if (vmap->count("help") || !vmap->count("dir"))
	{
		std::cerr << options << std::endl;
		return false;
	}

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	boost::program_options::variables_map vmap;
	if (!AnalyzeOptions(argc, argv, &vmap))
		return 1;

	// Parameters.
	const std::string &dir_name = vmap["dir"].as<std::string>();
	const std::string &order_name = vmap["order"].as<std::string>();
	const long long max_results = vmap["results"].as<long long>();
	const long long min_freq = vmap["freq"].as<long long>();

	// Paths of vocabulary files and databases.
	ssgnc::PathGenerator path_gen;
	path_gen.set_dir_name(dir_name);

	// Opens vocabulary files.
	ssgnc::VocabPair vocab;
	if (!vocab.Open(path_gen))
	{
		std::cerr << "error: failed to open vocabulary files" << std::endl;
		return 1;
	}

	// Opens databases.
	ssgnc::UnifiedDb db;
	if (!db.Open(path_gen))
	{
		std::cerr << "error: failed to open databases" << std::endl;
		return 1;
	}

	// Sets shared options of queries.
	ssgnc::Query query;
	if (boost::istarts_with("UNORDERED", order_name))
		query.set_order(ssgnc::Query::UNORDERED);
	else if (boost::istarts_with("ORDERED", order_name))
		query.set_order(ssgnc::Query::ORDERED);
	else if (boost::istarts_with("PHRASE", order_name))
		query.set_order(ssgnc::Query::PHRASE);
	else if (boost::istarts_with("FIXED", order_name))
		query.set_order(ssgnc::Query::FIXED);
	query.set_min_freq(min_freq);

	std::string line;
	while (std::getline(std::cin, line))
	{
		// Parses a query into a list of keys.
		boost::replace_all(line, "0x810x40", " ");
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

		ssgnc::Ngram ngram;
		for (long long count = 0; count < max_results &&
			reader.Read(&ngram); ++count)
		{
			if (!vocab.FillNgram(&ngram))
			{
				std::cerr << "error: failed to restore n-gram" << std::endl;
				continue;
			}

			std::cout << ngram.freq() << '\t';
			std::cout << ngram.key_string(0);
			for (int i = 1; i < ngram.key_string_size(); ++i)
				std::cout << ' ' << ngram.key_string(i);
			std::cout << '\n';
		}
	}

	return 0;
}
