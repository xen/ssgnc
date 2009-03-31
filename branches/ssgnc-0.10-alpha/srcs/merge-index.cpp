// This program builds indices from sorted n-grams.

#include "scoped-watch.h"
#include "vbe-reader.h"
#include "vbe-writer.h"

#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace boost;
using namespace std;

// Merges index files into one index file.
class index_merger
{
public:
	typedef long long int64;
	// Unigram ID and address.
	typedef pair<int, int64> pair_type;

	index_merger() : input_files_(), byte_readers_(), readers_(),
		addresses_() {}

	bool merge(const string &input_dir, ostream &output)
	{
		if (!open_input_files(input_dir))
			return false;

		if (!init_addresses())
			return false;

		for (int id = 0; ; ++id)
		{
			if (!next_addresses(output, id))
				break;
		}

		return true;
	}

private:
	vector<shared_ptr<ifstream> > input_files_;
	vector<shared_ptr<ngram::byte_reader> > byte_readers_;
	vector<shared_ptr<ngram::vbe_reader> > readers_;

	vector<int64> addresses_;
	vector<pair_type> pairs_;

	// Opens all input files.
	bool open_input_files(const string &input_dir)
	{
		for (int i = 1; ; ++i)
		{
			ostringstream s;
			s << input_dir << '/' << i << "gms.idx";
			const string input_file_name = s.str();

			shared_ptr<ifstream> input_file(
				new ifstream(input_file_name.c_str(), ios::binary));
			if (!input_file->is_open())
				break;
			input_files_.push_back(input_file);

			shared_ptr<ngram::byte_reader> byte_reader(
				new ngram::byte_reader(*input_file));
			byte_readers_.push_back(byte_reader);

			shared_ptr<ngram::vbe_reader> reader(
				new ngram::vbe_reader(*byte_reader));
			readers_.push_back(reader);
		}

		if (input_files_.empty())
		{
			cerr << "error: no input file" << endl;
			return false;
		}
		return true;
	}

	// Reads first addresses.
	bool init_addresses()
	{
		for (size_t i = 0; i < input_files_.size(); ++i)
		{
			pair_type pair;
			*readers_[i] >> pair.first >> pair.second;
			if (!pair.first || !pair.second)
			{
				cerr << "error: empty input file" << endl;
				return false;
			}

			pairs_.push_back(pair);
			addresses_.push_back(pair.second);
		}
		return true;
	}

	// Reads and writes next addresses.
	bool next_addresses(ostream &output, int id)
	{
		for (size_t i = 0; i < pairs_.size(); ++i)
		{
			if (id >= pairs_[i].first)
			{
				addresses_[i] = pairs_[i].second;

				pair_type diff;
				*readers_[i] >> diff.first >> diff.second;

				pairs_[i].first += diff.first;
				pairs_[i].second += diff.second;
			}
		}

		output.write(reinterpret_cast<const char *>(&addresses_[0]),
			addresses_.size() * sizeof(addresses_[0]));
		return id <= pairs_[0].first;
	}
};

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " InputDir" << endl;
		return 1;
	}

	const string input_dir = argv[1];

	ngram::scoped_watch watch;

	index_merger merger;
	if (!merger.merge(input_dir, cout))
		return 1;

	return 0;
}
