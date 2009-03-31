// This program sorts a vocab in descending frequency order.

#include "line-reader.h"
#include "scoped-watch.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef long long int64;
typedef pair<int64, string> pair_type;

class vocab_comparer
{
public:
	bool operator()(const pair_type &lhs, const pair_type &rhs) const
	{
		if (lhs.first == rhs.first)
			return lhs.second < rhs.second;
		return lhs.first > rhs.first;
	}
};

int sort_vocab(istream &input, ostream &output)
{
	ngram::line_reader reader(input);
	vector<pair_type> vocab;

	string line;
	pair_type key_value;
	while (reader.read(&line))
	{
		string::size_type pos = line.find('\t');
		if (pos == string::npos)
		{
			cerr << "error: invalid format: " << line << endl;
			return 1;
		}

		key_value.first = std::atoll(line.c_str() + pos + 1);
		key_value.second.assign(line.c_str(), pos);
		vocab.push_back(key_value);

		if (vocab.size() % 10000 == 0)
			cerr << "size: " << vocab.size() << '\r';
	}
	cerr << "size: " << vocab.size() << '\n';

	cerr << "sorting...";
	sort(vocab.begin(), vocab.end(), vocab_comparer());
	cerr << " end" << endl;

	cerr << "writing...";
	for (size_t i = 0; i < vocab.size(); ++i)
		output << vocab[i].second << '\t' << vocab[i].first << '\n';
	cerr << " end" << endl;

	return 0;
}

int main(int argc, char *argv[])
{
	ngram::scoped_watch watch;
	return sort_vocab(cin, cout);
}
