#ifndef SSGNC_BYTE_WRITER_H
#define SSGNC_BYTE_WRITER_H

#include <iostream>
#include <vector>

namespace ssgnc {

class ByteWriter
{
public:
	enum { DEFAULT_BUF_SIZE = 1 << 12 };

	explicit ByteWriter(std::ostream *stream,
		std::size_t buf_size = DEFAULT_BUF_SIZE)
		: stream_(stream), buf_(buf_size), pos_(0), total_(0) {}
	~ByteWriter() { Flush(); }

	// Writes a byte.
	void Write(int c)
	{
		buf_[pos_] = static_cast<unsigned char>(c);
		if (++pos_ == buf_.size())
			Flush();
		++total_;
	}

	// Returns how many bytes this object has written.
	std::size_t total() const { return total_; }

private:
	std::ostream *stream_;
	std::vector<char> buf_;
	std::size_t pos_;
	std::size_t total_;

	// Disallows copies.
	ByteWriter(const ByteWriter &);
	ByteWriter &operator=(const ByteWriter &);

private:
	// Flushes bytes in a buffer to an output stream.
	void Flush()
	{
		if (pos_ != 0)
		{
			stream_->write(&buf_[0], pos_);
			pos_ = 0;
		}
	}
};

}  // namespace ssgnc

#endif  // SSGNC_BYTE_WRITER_H
