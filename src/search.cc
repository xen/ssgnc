#include "ssgnc/client-options.h"
#include "ssgnc/unified-db.h"
#include "ssgnc/unified-reader.h"
#include "ssgnc/vocab-pair.h"

#include <getopt.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

namespace {


// Opens vocabulary files.
bool OpenVocab(const ssgnc::PathGenerator &path_gen, ssgnc::VocabPair *vocab)
{
	if (!vocab->Open(path_gen))
	{
		std::cerr << "error: failed to open vocabulary files" << std::endl;
		return false;
	}
	return true;
}

// Opens databases.
bool OpenDb(const ssgnc::ClientOptions &client_options,
	const ssgnc::PathGenerator &path_gen, ssgnc::UnifiedDb *db)
{
	if (!client_options.n_range().empty())
	{
		int min_n = 1;
		int max_n = INT_MAX - 1;

		const std::string &n_range = client_options.n_range();
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

bool InitQuery(const ssgnc::ClientOptions &client_options,
	ssgnc::Query *query)
{
	const std::string &order_name = client_options.order();
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

	const long long min_freq = client_options.freq();
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
	// Parses program options.
	ssgnc::ClientOptions client_options;
	if (!client_options.Parse(argc, argv))
	{
		ssgnc::ClientOptions::Usage(&std::cerr);
		return 1;
	}

	// Shows the help and exits.
	if (client_options.help())
	{
		ssgnc::ClientOptions::Usage(&std::cerr);
		return 0;
	}

	// Paths of vocabulary files and databases.
	ssgnc::PathGenerator path_gen;
	path_gen.set_dir_name(client_options.dir());

	// Opens vocabulary files.
	ssgnc::VocabPair vocab;
	if (!OpenVocab(path_gen, &vocab))
		return 1;

	// Opens databases.
	ssgnc::UnifiedDb db;
	if (!OpenDb(client_options, path_gen, &db))
		return 1;

	// Sets shared options of queries.
	ssgnc::Query query;
	if (!InitQuery(client_options, &query))
		return 1;

	const long long max_results = client_options.results();
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
