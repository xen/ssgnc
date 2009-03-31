#include "scoped-watch.h"
#include "search-engine.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " InputDir" << endl;
		return 1;
	}

	const string input_dir = argv[1];

	ngram::scoped_watch watch;

	ngram::search_engine engine(input_dir);
	if (!engine.is_open())
	{
		cerr << "error: failed to open search engine" << endl;
		return 1;
	}

	ngram::search_engine::query_type query;
	query.add_unigram("");
	query.add_unigram("");
	query.add_unigram("の");
	query.add_unigram("よう");
	query.add_unigram("な");
	query.add_unigram("");
	query.add_unigram("");
	query.set_min_freq(100);
	query.set_order(ngram::search_engine::query_type::FIXED);

	ngram::search_engine::reader_type reader(engine, query);
	ngram::search_engine::result_type result;
	for (int i = 0; i < 30 && reader.read(&result); ++i)
	{
		cout << result.freq() << ':';
		for (int i = 0; i < result.size(); ++i)
			cout << ' ' << result[i];
		cout << endl;
	}

	return 0;
}
