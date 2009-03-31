// This program builds indices from a sorted vocab.

#include "line-reader.h"
#include "scoped-watch.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

int sort_vocab(istream &input, ostream &index_output, ostream &data_output)
{
	ngram::line_reader reader(input);

	int count = 0;
	int address = 0;
	string line;

	// Writes the first address.
	index_output.write(
		reinterpret_cast<const char *>(&address), sizeof(address));

	while (reader.read(&line))
	{
		string::size_type pos = line.find('\t');
		if (pos == string::npos)
		{
			cerr << "error: invalid format: " << line << endl;
			return 1;
		}

		// Write a unigram.
		data_output.write(line.c_str(), pos) << '\0';

		// Writes the address of the next unigram.
		address += static_cast<int>(pos) + 1;
		index_output.write(
			reinterpret_cast<const char *>(&address), sizeof(address));

		if (++count % 10000 == 0)
			cerr << "count: " << count << '\r';
	}
	cerr << "count: " << count << endl;

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " IndexFile" << endl;
		return 1;
	}

	const string index_file_name = argv[1];
	ofstream index_file(index_file_name.c_str(), ios::binary);
	if (!index_file.is_open())
	{
		cerr << "error: failed to open file: " << index_file_name << endl;
		return 1;
	}

	ngram::scoped_watch watch;
	return sort_vocab(cin, index_file, cout);
}
