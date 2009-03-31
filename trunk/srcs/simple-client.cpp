#include "search-engine.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <vector>

using namespace boost::algorithm;
using namespace boost::program_options;
using namespace std;

// Parses command line options.
bool analyze_options(int argc, char *argv[], variables_map *vmap)
{
	options_description options("Options");
	options.add_options()
		("help,h", "display this help and exit")
		("dir,d", value<string>(), "index directory (required)")
		("query,q", value<string>(), "white-space separated query (required)")
		("order,o", value<string>()->default_value("unordered"),
			"query order (unordered, ordered, phrase or fixed)")
		("results,r", value<long long>()->default_value(20),
			"maximum number of results")
		("freq,f", value<long long>()->default_value(0), "minimum frequency");

	store(parse_command_line(argc, argv, options), *vmap);
	notify(*vmap);

	if (vmap->count("help") || !vmap->count("dir") || !vmap->count("query"))
	{
		cerr << options << endl;
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{
	variables_map vmap;
	if (!analyze_options(argc, argv, &vmap))
		return 1;

	const string input_dir = vmap["dir"].as<string>();
	ngram::search_engine engine(input_dir);
	if (!engine.is_open())
	{
		cerr << "error: failed to open search engine: " << input_dir << endl;
		return 1;
	}

	ngram::search_engine::query_type query;

	const string query_order = vmap["order"].as<string>();
	if (query_order == "unordered")
		query.set_order(ngram::search_engine::query_type::UNORDERED);
	else if (query_order == "ordered")
		query.set_order(ngram::search_engine::query_type::ORDERED);
	else if (query_order == "phrase")
		query.set_order(ngram::search_engine::query_type::PHRASE);
	else if (query_order == "fixed")
		query.set_order(ngram::search_engine::query_type::FIXED);
	else
	{
		cerr << "error: unknown query order: " << query_order << endl;
		return 1;
	}

	long long max_results = vmap["results"].as<long long>();
	query.set_min_freq(vmap["freq"].as<long long>());

	vector<string> unigrams;
	split(unigrams, vmap["query"].as<string>(), is_any_of(" "));
	for (size_t i = 0; i < unigrams.size(); ++i)
	{
		if (query.order() == ngram::search_engine::query_type::FIXED)
			query.add_unigram(unigrams[i] != "*" ? unigrams[i] : "");
		else
			query.add_unigram(unigrams[i]);
	}

	ngram::search_engine::reader_type reader(engine, query);
	ngram::search_engine::result_type result;
	for (long long i = 0; i < max_results && reader.read(&result); ++i)
	{
		cout << result.freq() << '\t';
		for (int i = 0; i < result.size(); ++i)
			cout << ' ' << result[i];
		cout << endl;
	}

	return 0;
}
