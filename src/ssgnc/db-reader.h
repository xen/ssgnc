#ifndef SSGNC_DB_READER_H
#define SSGNC_DB_READER_H

#include "mem-streambuf.h"
#include "ngram.h"
#include "varint-reader.h"

#include <iostream>

#include <boost/shared_ptr.hpp>

namespace ssgnc {

class DbReader
{
public:
	DbReader() : n_(0), size_(0), freq_(0),
		streambuf_(), stream_(), byte_reader_(), reader_() {}

	// Sets a range of n-grams.
	void SetRange(int n, const char *address, std::size_t size)
	{
		n_ = n;
		size_ = size;
		freq_ = 0;
		streambuf_.reset(new MemStreambuf(address, size));
		stream_.reset(new std::istream(streambuf_.get()));
		byte_reader_.reset(new ByteReader(stream_.get()));
		reader_.reset(new VarintReader(byte_reader_.get()));
	}

	// Reads an n-gram.
	bool Read(Ngram *ngram)
	{
		ngram->Clear();

		long long freq_diff;
		if (!reader_->Read(&freq_diff))
			return false;
		freq_ = (freq_ != 0) ? (freq_ - freq_diff) : freq_diff;
		ngram->set_freq(freq_);

		for (int i = 0; i < n_; ++i)
		{
			int key;
			if (!reader_->Read(&key))
				return false;
			ngram->add_key_id(key);
		}
		return true;
	}

	// Returns a parameter n.
	int n() const { return n_; }
	// Returns the number of bytes in a range.
	std::size_t size() const { return size_; }
	// Returns the number of bytes read.
	std::size_t total() const { return byte_reader_->total(); }

private:
	int n_;
	std::size_t size_;
	long long freq_;
	boost::shared_ptr<MemStreambuf> streambuf_;
	boost::shared_ptr<std::istream> stream_;
	boost::shared_ptr<ByteReader> byte_reader_;
	boost::shared_ptr<VarintReader> reader_;

	// Disallows copies.
	DbReader(const DbReader &);
	DbReader &operator=(const DbReader &);
};

}  // namespace ssgnc

#endif  // SSGNC_DB_READER_H
