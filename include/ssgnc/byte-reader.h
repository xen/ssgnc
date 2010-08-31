#ifndef SSGNC_BYTE_READER_H
#define SSGNC_BYTE_READER_H

#include "common.h"

namespace ssgnc {

class ByteReader
{
public:
	ByteReader() : stream_(NULL), buf_(), buf_size_(0), pos_(0), total_(0) {}
	~ByteReader();

	bool open(std::istream *stream, UInt32 buf_size = 0)
		SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool read(Int32 *byte) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return stream_ != NULL; }

	UInt64 tell() const { return total_; }

	enum { DEFAULT_BUF_SIZE = 1 << 12 };

private:
	std::istream *stream_;
	std::vector<Int8> buf_;
	UInt32 buf_size_;
	UInt32 pos_;
	UInt64 total_;

	bool fill();

	// Disallows copies.
	ByteReader(const ByteReader &);
	ByteReader &operator=(const ByteReader &);
};

inline bool ByteReader::read(Int32 *byte)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Null stream" << std::endl;
		return false;
	}
	else if (byte == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (pos_ >= buf_.size() && !fill())
	{
		if (stream_->bad())
		{
			SSGNC_ERROR << "std::istream::bad(): true" << std::endl;
			return false;
		}
		*byte = EOF;
		return true;
	}

	++total_;
	*byte = static_cast<ssgnc::UInt8>(buf_[pos_++]);
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_BYTE_READER_H
