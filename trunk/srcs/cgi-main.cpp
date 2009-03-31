#include "cgi-config.h"
#include "cgi-request.h"
#include "search-engine.h"

#include <boost/algorithm/string.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace boost;
using namespace boost::algorithm;
using namespace std;

namespace
{

// Gets an order type.
ngram::search_engine::query_type::order_type get_order(
	const ngram::cgi_request &request)
{
	ngram::cgi_request::iterator it = request.find("t");
	if (it != request.end())
	{
		if (it->second == "ordered")
			return ngram::search_engine::query_type::ORDERED;
		else if (it->second == "phrase")
			return ngram::search_engine::query_type::PHRASE;
		else if (it->second == "fixed")
			return ngram::search_engine::query_type::FIXED;
	}

	// Default setting (unordered).
	return ngram::search_engine::query_type::UNORDERED;
}

// Gets the minimum frequency.
long long get_min_freq(const ngram::cgi_request &request)
{
	ngram::cgi_request::iterator it = request.find("f");
	if (it != request.end())
		return atoll(it->second.c_str());

	// Default setting (100).
	return 100;
}

// Gets the maximum number of results from a request.
long long get_max_results(const ngram::cgi_request &request)
{
	ngram::cgi_request::iterator it = request.find("m");
	if (it != request.end())
	{
		// Ignores an invalid value.
		long long max_results = atoll(it->second.c_str());
		if (max_results > 0)
			return max_results;
	}

	// Default setting (100).
	return 100;
}

// Analyzes a query.
bool analyze_query(const ngram::cgi_request &request,
	ngram::search_engine::query_type *query)
{
	// Splits a query into unigrams.
	ngram::cgi_request::iterator it = request.find("q");
	if (it == request.end())
		return false;

	vector<string> unigrams;
	split(unigrams, it->second, is_any_of(" "), token_compress_on);
	if (unigrams.empty())
		return false;

	for (size_t i = 0; i < unigrams.size(); ++i)
	{
		if (query->order() == ngram::search_engine::query_type::FIXED)
			query->add_unigram((unigrams[i] != "*") ? unigrams[i] : "");
		else
			query->add_unigram(unigrams[i]);
	}

	return true;
}

// Analyzes n values.
void analyze_n_values(const ngram::cgi_request &request,
	ngram::search_engine::query_type *query)
{
	// Splits a query into unigrams.
	ngram::cgi_request::iterator it = request.find("n");
	if (it == request.end())
		return;

	vector<string> values;
	split(values, it->second, is_any_of(","), token_compress_on);
	for (size_t i = 0; i < values.size(); ++i)
	{
		int n = atoi(values[i].c_str());
		string::size_type pos = values[i].find('-');
		if (pos != string::npos)
		{
			int max_n = atoi(values[i].c_str() + pos + 1);
			for (int j = n; j <= max_n; ++j)
				query->add_n(j);
		}
		else
			query->add_n(n);
	}
}

// Converts from a cgi request to an internal query.
bool create_query_from_request(const ngram::cgi_request &request,
	ngram::search_engine::query_type *query)
{
	query->set_order(get_order(request));
	query->set_min_freq(get_min_freq(request));

	if (!analyze_query(request, query))
		return false;

	analyze_n_values(request, query);

	return true;
}

// Selects a requested language.
string select_input_dir(const ngram::cgi_request &request)
{
	const string english_dir = ngram::cgi_config::english_dir();
	const string japanese_dir = ngram::cgi_config::japanese_dir();

	ngram::cgi_request::iterator it = request.find("l");
	if (it != request.end())
	{
		// Specifies English.
		if (it->second == "en")
			return english_dir;
	}

	// Default setting (Japanese).
	return japanese_dir;
}

// Prints a content type.
void output_content_type(const ngram::cgi_request &request)
{
	ngram::cgi_request::iterator it = request.find("c");
	if (it != request.end())
	{
		// Shows a dialog to save as a file.
		if (it->second == "save")
		{
			cout << "Content-Type: text/download; name=\"ngram.txt\"\n"
				"Content-Disposition: attachment; filename=\"ngram.txt\"\n\n";
			return;
		}
	}

	// Default setting (plain text).
	cout << "Content-Type: text/plain; charset=utf-8\n\n";
}

}  // namespace

int main()
{
	ngram::cgi_request request = ngram::cgi_request::analyze();

	ngram::search_engine::query_type query;
	if (!create_query_from_request(request, &query))
		return 1;

	long long max_results = get_max_results(request);

	ngram::search_engine engine(select_input_dir(request));
	if (!engine.is_open())
		return 1;

	output_content_type(request);

	ngram::search_engine::reader_type reader(engine, query);
	ngram::search_engine::result_type result;
	for (long long i = 0; i < max_results && reader.read(&result); ++i)
	{
		cout << result.freq();
		for (int j = 0; j < result.size(); ++j)
			cout << ' ' << result[j];
		cout << '\n';
	}

	return 0;
}
