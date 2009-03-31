// This program sorts n-grams in lexical order.

#include "key-reader.h"
#include "key-writer.h"
#include "scoped-watch.h"

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
class key_divider
{
public:
	key_divider(int n, int memory_size, const string &output_prefix)
		: n_(n), output_prefix_(output_prefix),
		max_id_(0), keys_(), total_count_(0), file_id_(0)
	{
		// Allocates memory to buffers.
		// Note: this size was calculated experimentally.
		size_t key_string_size =
			sizeof(void *) * 3 + sizeof(string::size_type) * 3;
		max_id_ = (int)(((long long)memory_size << 20)
			/ (sizeof(ngram::key_type) + key_string_size));
		keys_.resize(max_id_);
	}

	bool divide(istream &input, vector<string> *output_file_names)
	{
		ngram::key_reader key_reader(n_, input);

		for ( ; ; )
		{
			if (!fill(key_reader))
				return false;
			if (keys_.empty())
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
	vector<ngram::key_type> keys_;

	int total_count_;
	int file_id_;

	// Copies are not allowed.
	key_divider(const key_divider &);
	key_divider &operator=(const key_divider &);

private:
	bool fill(ngram::key_reader &key_reader)
	{
		keys_.clear();

		for (int id = 0; id < max_id_; ++id)
		{
			ngram::key_type key;
			if (!key_reader.read(&key))
				break;
			keys_.push_back(key);

			if (!(++total_count_ % 100000))
				cerr << "dividing... " << total_count_ << '/'
					<< file_id_ << '\r';
		}
		return true;
	}

	bool flush(vector<string> *output_file_names)
	{
		sort(keys_.begin(), keys_.end());

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
		ngram::key_writer key_writer(output);
		for (size_t i = 0; i < keys_.size(); ++i)
			key_writer.write(keys_[i]);

		return true;
	}
};

// Merges ordered n-gram files into one ordered n-gram file.
class key_merger
{
public:
	// Key and file ID.
	typedef pair<ngram::key_type, int> pair_type;

	key_merger(int n) : n_(n), input_files_(), key_readers_(), queue_() {}

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
	vector<shared_ptr<ngram::key_reader> > key_readers_;

	priority_queue<pair_type, vector<pair_type>, greater<pair_type> > queue_;

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

			shared_ptr<ngram::key_reader> key_reader(
				new ngram::key_reader(n_, *input_file));
			key_readers_.push_back(key_reader);
		}
		return true;
	}

	// Reads first frequencies from files.
	bool init_queue()
	{
		ngram::key_type key;
		for (size_t i = 0; i < input_files_.size(); ++i)
		{
			if (!key_readers_[i]->read(&key))
			{
				cerr << "error: empty file" << endl;
				return false;
			}
			queue_.push(make_pair(key, static_cast<int>(i)));
		}
		return true;
	}

	// Merges input files.
	bool merge_files(ostream &output)
	{
		ngram::key_writer key_writer(output);

		long long count = 0;
		while (!queue_.empty())
		{
			pair_type key_and_id = queue_.top();
			queue_.pop();

			key_writer.write(key_and_id.first);

			if (!(++count % 100000))
				cerr << "merging... " << count << '\r';

			if (key_readers_[key_and_id.second]->read(&key_and_id.first))
				queue_.push(key_and_id);
		}
		cerr << "merging... " << count << " end" << endl;
		return true;
	}
};

bool sort_keys(int n, int memory_size, const string &temp_file_prefix)
{
	vector<string> temp_file_names;

	{
		// Divides one unordered n-gram file to ordered n-gram files.
		key_divider divider(n, memory_size, temp_file_prefix);
		if (!divider.divide(cin, &temp_file_names))
			return false;
	}

	{
		// Merges ordered n-gram files into one ordered n-gram file.
		key_merger merger(n);
		if (!merger.merge(cout, temp_file_names))
			return false;
	}

	// Removes temporary files.
	for (size_t i = 0; i < temp_file_names.size(); ++i)
		remove(temp_file_names[i].c_str());

	return true;
}

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

	if (!sort_keys(n, memory_size, temp_file_prefix))
		return 1;

	return 0;
}
