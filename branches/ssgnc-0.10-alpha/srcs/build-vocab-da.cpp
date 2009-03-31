// This program builds a double-array from a sorted vocab.

#include "line-reader.h"
#include "scoped-watch.h"

#include "darts/darts-clone.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef pair<string, int> pair_type;

int build_da(const vector<pair_type> &vocab, const std::string &da_name)
{
	// Creates lists of keys and values.
	vector<const char *> keys(vocab.size());
	vector<int> values(vocab.size());
	for (size_t i = 0; i < keys.size(); ++i)
	{
		keys[i] = vocab[i].first.c_str();
		values[i] = vocab[i].second;
	}

	// Builds a double-array and writes it to a file.
	try
	{
		Darts::DoubleArray da;

		cerr << "building...";
		if (da.build(keys.size(), &keys[0], 0, &values[0]) == -1)
		{
			cerr << "error: failed to build double-array" << endl;
			return 1;
		}
		cerr << " end" << endl;

		cerr << "writing...";
		if (da.save(da_name.c_str()) == -1)
		{
			cerr << "error: failed to save double-array: " << da_name << endl;
			return 1;
		}
		cerr << " end" << endl;
	}
	catch (const std::exception &ex)
	{
		cerr << "error: " << ex.what() << endl;
		return 1;
	}

	return 0;
}

int build_vocab_da(istream &input, ostream &output, const std::string &da_name)
{
	ngram::line_reader reader(input);
	vector<pair_type> vocab;

	// Reads a sorted vocab.
	int key_id = 0;
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

		key_value.first.assign(line.c_str(), pos);
		key_value.second = ++key_id;
		vocab.push_back(key_value);

		if (vocab.size() % 10000 == 0)
			cerr << "size: " << vocab.size() << '\r';
	}
	cerr << "size: " << vocab.size() << endl;

	// Sorts a vocab in dictionary order.
	cerr << "sorting...";
	sort(vocab.begin(), vocab.end());
	cerr << " end" << endl;

	return build_da(vocab, da_name);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " DAFile" << endl;
		return 1;
	}

	const string da_name = argv[1];

	ngram::scoped_watch watch;
	return build_vocab_da(cin, cout, da_name);
}
