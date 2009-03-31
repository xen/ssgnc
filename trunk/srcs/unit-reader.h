#ifndef GOOGLE_NGRAM_UNIT_READER_H
#define GOOGLE_NGRAM_UNIT_READER_H

#include "unit-type.h"
#include "mem-streambuf.h"
#include "vbe-reader.h"

#include "boost/tuple/tuple.hpp"

#include <iostream>
#include <vector>

namespace ngram
{

class unit_reader
{
public:
	explicit unit_reader(const boost::tuple<int, const char *, std::size_t> &t)
		: n_(t.get<0>()), size_(t.get<2>()),
		buf_(const_cast<char *>(t.get<1>()), t.get<2>()),
		stream_(&buf_), byte_reader_(stream_), vbe_reader_(byte_reader_) {}
	unit_reader(int n, const char *buf, std::size_t size)
		: n_(n), size_(size), buf_(const_cast<char *>(buf), size),
		stream_(&buf_), byte_reader_(stream_), vbe_reader_(byte_reader_) {}

	// Reads a frequency and unigrams.
	bool read(unit_type *unit)
	{
		unit->clear();

		// Reads a frequency.
		long long freq;
		vbe_reader_ >> freq;
		if (!freq)
			return false;
		unit->set_freq(freq);

		// Reads unigrams.
		for (int i = 0; i < n_; ++i)
		{
			int unigram;
			vbe_reader_ >> unigram;
			if (!unigram)
				return false;
			unit->add_unigram(unigram);
		}

		return true;
	}

	// Returns the buffer size.
	std::size_t size() const { return size_; }

private:
	const int n_;
	const std::size_t size_;
	mem_streambuf buf_;
	std::istream stream_;
	byte_reader byte_reader_;
	vbe_reader vbe_reader_;
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_UNIT_READER_H
