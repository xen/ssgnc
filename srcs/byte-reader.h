#ifndef GOOGLE_NGRAM_BYTE_READER_H
#define GOOGLE_NGRAM_BYTE_READER_H

#include <iostream>
#include <vector>

namespace ngram
{

class byte_reader
{
public:
	enum { BUF_SIZE = 1 << 12 };

	explicit byte_reader(std::istream &stream)
		: stream_(stream), buf_(), pos_(0), count_(0) {}

	// Reads a byte.
	int get()
	{
		if (pos_ >= buf_.size() && !fill())
			return EOF;
		++count_;
		return static_cast<unsigned char>(buf_[pos_++]);
	}

	// Returns how many bytes this object has read.
	long long count() const { return count_; }

private:
	std::istream &stream_;
	std::vector<char> buf_;
	std::size_t pos_;
	long long count_;

	// Copies are not allowed.
	byte_reader(const byte_reader &);
	byte_reader &operator=(const byte_reader &);

private:
	// Reads bytes from a stream to a buffer.
	bool fill()
	{
		if (!stream_)
			return false;

		buf_.resize(BUF_SIZE);
		stream_.read(&buf_[0], BUF_SIZE);
		buf_.resize(stream_.gcount());

		pos_ = 0;
		return buf_.size() > 0;
	}
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_BYTE_READER_H
