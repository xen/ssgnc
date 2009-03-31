// This program sorts encoded n-grams in descending frequency order.

#include "scoped-watch.h"
#include "vbe-reader.h"
#include "vbe-writer.h"

#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <cstdlib>
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

// Divides one unordered n-gram file to ordered n-gram files.
class ngram_divider
{
public:
	typedef long long int64;
	// Frequency and ID.
	typedef pair<int64, int> pair_type;

	class pair_comparer
	{
	public:
		bool operator()(const pair_type &lhs, const pair_type &rhs)
		{
			if (lhs.first == rhs.first)
				return lhs.second < rhs.second;
			return lhs.first > rhs.first;
		}
	};

	ngram_divider(int n, int memory_size, const string &output_prefix)
		: n_(n), output_prefix_(output_prefix),
		max_id_(0), unigrams_(), pairs_(), total_count_(0), file_id_(0)
	{
		// Allocates memory to buffers.
		max_id_ = (int)(((int64)memory_size << 20)
			/ (sizeof(int) * n_ + sizeof(pair_type)));
		unigrams_.resize(max_id_ * n_);
		pairs_.resize(max_id_);
	}

	bool divide(istream &input, vector<string> *output_file_names)
	{
		ngram::byte_reader byte_reader(input);
		ngram::vbe_reader reader(byte_reader);

		for ( ; ; )
		{
			if (!fill(reader))
				return false;
			if (!pairs_.size())
				break;
			if (!flush(output_file_names))
				return false;
		}

		cerr << "dividing... " << total_count_ << '/'
			<< file_id_ << " end" << endl;
		return true;
	}

private:
	const int n_;
	const string output_prefix_;

	int max_id_;
	vector<int> unigrams_;
	vector<pair_type> pairs_;

	int total_count_;
	int file_id_;

	// Copies are not allowed.
	ngram_divider(const ngram_divider &);
	ngram_divider &operator=(const ngram_divider &);

private:
	bool fill(ngram::vbe_reader &reader)
	{
		unigrams_.clear();
		pairs_.clear();

		for (int id = 0; id < max_id_; ++id)
		{
			// Reads a frequency.
			int64 freq;
			reader >> freq;
			if (!freq)
				break;
			pairs_.push_back(make_pair(freq, pairs_.size()));

			// Reads unigrams.
			for (int i = 0; i < n_; ++i)
			{
				int unigram;
				reader >> unigram;
				if (!unigram)
				{
					cerr << "error: invalid format" << endl;
					return false;
				}
				unigrams_.push_back(unigram);
			}

			if (!(++total_count_ % 100000))
				cerr << "dividing... " << total_count_ << '/'
					<< file_id_ << '\r';
		}
		return true;
	}

	bool flush(vector<string> *output_file_names)
	{
		sort(pairs_.begin(), pairs_.end(), pair_comparer());

		// Generates an output file name.
		ostringstream s;
		s << output_prefix_ << '.';
		s << setw(4) << setfill('0') << file_id_++;
		const string output_file_name = s.str();

		ofstream output(output_file_name.c_str(), ios::binary);
		if (!output.is_open())
		{
			cerr << "error: failed to open file: " << output_file_name << endl;
			return false;
		}
		output_file_names->push_back(output_file_name);

		// Writes n-grams to an output file.
		ngram::byte_writer byte_writer(output);
		ngram::vbe_writer writer(byte_writer);
		for (size_t i = 0; i < pairs_.size(); ++i)
		{
			int id = pairs_[i].second;
			writer << pairs_[i].first;
			for (int j = 0; j < n_; ++j)
				writer << unigrams_[id * n_ + j];
		}

		return true;
	}
};

// Merges ordered n-gram files into one ordered n-gram file.
class ngram_merger
{
public:
	typedef long long int64;
	// Frequency and file ID.
	typedef pair<int64, int> pair_type;

	class pair_comparer
	{
	public:
		bool operator()(const pair_type &lhs, const pair_type &rhs)
		{
			if (lhs.first == rhs.first)
				return lhs.second > rhs.second;
			return lhs.first < rhs.first;
		}
	};

	ngram_merger(int n)
		: n_(n), input_files_(), byte_readers_(), readers_(), queue_() {}

	bool merge(ostream &output, const vector<string> &input_file_names)
	{
		if (!open_input_files(input_file_names))
			return false;

		if (!init_queue())
			return false;

		if (!merge_files(output))
			return false;

		return true;
	}

private:
	const int n_;

	vector<shared_ptr<ifstream> > input_files_;
	vector<shared_ptr<ngram::byte_reader> > byte_readers_;
	vector<shared_ptr<ngram::vbe_reader> > readers_;

	priority_queue<pair_type, vector<pair_type>, pair_comparer> queue_;

	// Opens all input files.
	bool open_input_files(const vector<string> &input_file_names)
	{
		for (size_t i = 0; i < input_file_names.size(); ++i)
		{
			shared_ptr<ifstream> input_file(
				new ifstream(input_file_names[i].c_str(), ios::binary));
			if (!input_file->is_open())
			{
				cerr << "error: failed to open file: "
					<< input_file_names[i] << endl;
				return false;
			}
			input_files_.push_back(input_file);

			shared_ptr<ngram::byte_reader> byte_reader(
				new ngram::byte_reader(*input_file));
			byte_readers_.push_back(byte_reader);

			shared_ptr<ngram::vbe_reader> reader(
				new ngram::vbe_reader(*byte_reader));
			readers_.push_back(reader);
		}
		return true;
	}

	// Reads first frequencies from files.
	bool init_queue()
	{
		for (size_t i = 0; i < input_files_.size(); ++i)
		{
			int64 freq;
			*readers_[i] >> freq;
			if (!freq)
			{
				cerr << "error: empty file" << endl;
				return false;
			}
			queue_.push(make_pair(freq, static_cast<int>(i)));
		}
		return true;
	}

	// Merges input files.
	bool merge_files(ostream &output)
	{
		ngram::byte_writer byte_writer(output);
		ngram::vbe_writer writer(byte_writer);

		int64 count = 0;
		while (!queue_.empty())
		{
			pair_type pair = queue_.top();
			queue_.pop();

			int64 freq = pair.first;
			int file_id = pair.second;

			// Writes a frequency and unigrams.
			writer << freq;
			for (int i = 0; i < n_; ++i)
			{
				int unigram;
				*readers_[file_id] >> unigram;
				if (!unigram)
				{
					cerr << "error: invalid format" << endl;
					return false;
				}
				writer << unigram;
			}

			// Reads frequency of the next n-gram.
			*readers_[file_id] >> freq;
			if (freq)
				queue_.push(make_pair(freq, file_id));

			if (!(++count % 100000))
				cerr << "merging... " << count << '\r';
		}
		cerr << "merging... " << count << " end" << endl;
		return true;
	}
};

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cerr << "Usage: " << argv[0]
			<< " N MemorySize TempFilePrefix" << endl;
		return 1;
	}

	int n = atoi(argv[1]);
	if (n <= 0)
	{
		cerr << "error: invalid N value: " << argv[1] << endl;
		return 1;
	}

	int memory_size = atoi(argv[2]);
	if (memory_size < 256)
	{
		cerr << "error: invalid memory size: " << argv[2] << endl;
		return 1;
	}

	const string temp_file_prefix = argv[3];

	ngram::scoped_watch watch;

	vector<string> temp_file_names;

	{
		// Divides one unordered n-gram file to ordered n-gram files.
		ngram_divider divider(n, memory_size, temp_file_prefix);
		if (!divider.divide(cin, &temp_file_names))
			return 1;
	}

	{
		// Merges ordered n-gram files into one ordered n-gram file.
		ngram_merger merger(n);
		if (!merger.merge(cout, temp_file_names))
			return 1;
	}

	// Removes temporary files.
	for (size_t i = 0; i < temp_file_names.size(); ++i)
		remove(temp_file_names[i].c_str());

	return 0;
}
