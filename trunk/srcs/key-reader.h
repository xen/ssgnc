#ifndef GOOGLE_NGRAM_KEY_READER_H
#define GOOGLE_NGRAM_KEY_READER_H

#include "key-type.h"
#include "vbe-reader.h"

#include <iostream>
#include <vector>

namespace ngram
{

class key_reader
{
public:
	key_reader(int n, std::istream &stream)
		: n_(n), byte_reader_(stream), vbe_reader_(byte_reader_), buf_() {}

	bool read(key_type *key)
	{
		buf_.clear();

		// Reads a frequency.
		long long freq;
		vbe_reader_ >> freq;
		if (!freq)
			return false;

		// Reads unigrams.
		for (int i = 0; i < n_; ++i)
		{
			int c;
			do
			{
				c = byte_reader_.get();
				if (c == EOF)
					return false;
				buf_.push_back(c);
			} while (c >= 0x80);
		}

		key->set_key(&buf_[0], buf_.size());
		key->set_freq(freq);

		return true;
	}

private:
	int n_;
	byte_reader byte_reader_;
	vbe_reader vbe_reader_;
	std::vector<char> buf_;

	// Copies are not allowed.
	key_reader(const key_reader &);
	key_reader &operator=(const key_reader &);
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_KEY_READER_H
