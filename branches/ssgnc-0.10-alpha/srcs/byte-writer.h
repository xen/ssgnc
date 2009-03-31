#ifndef GOOGLE_NGRAM_BYTE_WRITER_H
#define GOOGLE_NGRAM_BYTE_WRITER_H

#include <iostream>
#include <vector>

namespace ngram
{

class byte_writer
{
public:
	enum { BUF_SIZE = 1 << 12 };

	explicit byte_writer(std::ostream &stream)
		: stream_(stream), buf_(BUF_SIZE), pos_(0), count_(0) {}
	~byte_writer() { flush(); }

	// Writes a byte.
	void put(int c)
	{
		buf_[pos_] = static_cast<unsigned char>(c);
		if (++pos_ == BUF_SIZE)
			flush();
		++count_;
	}

	// Returns how many bytes this object has written.
	long long count() const { return count_; }

private:
	std::ostream &stream_;
	std::vector<char> buf_;
	std::size_t pos_;
	long long count_;

	// Copies are not allowed.
	byte_writer(const byte_writer &);
	byte_writer &operator=(const byte_writer &);

private:
	// Writes bytes in a buffer to a stream.
	void flush()
	{
		stream_.write(&buf_[0], pos_);
		pos_ = 0;
	}
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_BYTE_WRITER_H
