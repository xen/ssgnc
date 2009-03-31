// This program encodes text n-gram data to binary n-gram data.

#include "line-reader.h"
#include "scoped-watch.h"
#include "vbe-reader.h"
#include "vbe-writer.h"

#include "darts/darts-clone.h"

#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

int encode_ngram(istream &input, ostream &output, const Darts::DoubleArray &da)
{
	ngram::line_reader reader(input);
	ngram::byte_writer byte_writer(output);
	ngram::vbe_writer vbe_writer(byte_writer);

	long long count = 0;
	vector<int> keys;
	string line;
	while (reader.read(&line))
	{
		keys.clear();

		string::size_type start = 0;
		for (string::size_type pos; ; start = pos + 1)
		{
			// Reads unigrams.
			pos = line.find_first_of(" \t", start);
			if (pos == string::npos)
			{
				cerr << "error: invalid format: " << line << endl;
				return 1;
			}

			int key_id;
			da.exactMatchSearch(line.c_str() + start, key_id, pos - start);
			if (key_id == -1)
			{
				cerr << "error: unknown unigram: " << line << endl;
				return 1;
			}
			keys.push_back(key_id);

			if (line[pos] == '\t')
			{
				// Writes a frequency and unigrams.
				vbe_writer << std::atoll(line.c_str() + pos + 1);
				for (size_t i = 0; i < keys.size(); ++i)
					vbe_writer << keys[i];
				break;
			}
		}

		if (++count % 100000 == 0)
			cerr << "size: " << count << '\r';
	}
	cerr << "size: " << count << endl;

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " DAFile" << endl;
		return 1;
	}

	const string da_name = argv[1];

	Darts::DoubleArray da;
	if (da.open(da_name.c_str()) == -1)
	{
		cerr << "error: failed to open double-array: " << da_name << endl;
		return 1;
	}

	ngram::scoped_watch watch;
	return encode_ngram(cin, cout, da);
}
