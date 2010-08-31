#ifndef SSGNC_BYTE_READER_H
#define SSGNC_BYTE_READER_H

#include "int-types.h"

#include <iostream>
#include <vector>

namespace ssgnc {

class ByteReader
{
public:
	enum { DEFAULT_BUF_SIZE = 1 << 12 };
	enum { EOS_VALUE = -1 };

	ByteReader() : in_(NULL), buf_(), buf_size_(0), pos_(0), total_(0) {}
	~ByteReader() { close(); }

	bool open(std::istream *in, UInt32 buf_size = 0);
	void close();

	Int32 read();

	bool eof() const { return (pos_ >= buf_.size()) && in_->eof(); }

	UInt64 total() const { return total_; }

private:
	std::istream *in_;
	std::vector<Int8> buf_;
	UInt32 buf_size_;
	UInt32 pos_;
	UInt64 total_;

	bool fill();

	// Disallows copies.
	ByteReader(const ByteReader &);
	ByteReader &operator=(const ByteReader &);
};

inline bool ByteReader::open(std::istream *in, UInt32 buf_size)
{
	close();

	if (buf_size == 0)
		buf_size = DEFAULT_BUF_SIZE;

	in_ = in;
	buf_.reserve(buf_size);
	buf_.clear();
	buf_size_ = buf_size;
	return true;
}

inline void ByteReader::close()
{
	in_ = NULL;
	buf_size_ = 0;
	pos_ = 0;
	total_ = 0;
}

inline Int32 ByteReader::read()
{
	if (pos_ >= buf_.size() && !fill())
		return EOS_VALUE;

	++total_;
	return static_cast<UInt8>(buf_[pos_++]);
}

inline bool ByteReader::fill()
{
	if (!(*in_))
		return false;

	buf_.resize(buf_size_);
	in_->read(&buf_[0], buf_size_);
	buf_.resize(static_cast<std::size_t>(in_->gcount()));

	pos_ = 0;
	return buf_.size() > 0;
}

}  // namespace ssgnc

#endif  // SSGNC_BYTE_READER_H
