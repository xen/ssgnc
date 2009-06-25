#ifndef SSGNC_BYTE_READER_H
#define SSGNC_BYTE_READER_H

#include <iostream>
#include <vector>

namespace ssgnc {

class ByteReader
{
public:
	enum { BUF_SIZE = 1 << 12 };

	explicit ByteReader(std::istream *stream)
		: stream_(stream), buf_(), pos_(0), total_(0) {}

	// Reads a byte as a char.
	bool Read(char *c)
	{
		if (pos_ >= buf_.size() && !Fill())
			return false;
		++total_;
		*c = buf_[pos_++];
		return true;
	}

	// Reads a byte as an unsigned char.
	bool Read(unsigned char *c)
	{
		if (pos_ >= buf_.size() && !Fill())
			return false;
		++total_;
		*c = static_cast<unsigned char>(buf_[pos_++]);
		return true;
	}

	// Returns how many bytes this object has read.
	std::size_t Total() const { return total_; }

private:
	std::istream *stream_;
	std::vector<char> buf_;
	std::size_t pos_;
	std::size_t total_;

	// Disallows copies.
	ByteReader(const ByteReader &);
	ByteReader &operator=(const ByteReader &);

private:
	// Reads bytes from an input stream to an internal buffer.
	bool Fill()
	{
		if (!(*stream_))
			return false;

		buf_.resize(BUF_SIZE);
		stream_->read(&buf_[0], BUF_SIZE);
		buf_.resize(stream_->gcount());

		pos_ = 0;
		return buf_.size() > 0;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_BYTE_READER_H
