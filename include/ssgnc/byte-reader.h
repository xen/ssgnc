#ifndef SSGNC_BYTE_READER_H
#define SSGNC_BYTE_READER_H

#include "common.h"
#include "string-builder.h"

namespace ssgnc {

class ByteReader
{
public:
	ByteReader() : stream_(NULL), buf_(), buf_size_(0), pos_(0), total_(0) {}
	~ByteReader();

	bool open(std::istream *stream, UInt32 buf_size = 0)
		SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool read(Int8 *byte) SSGNC_WARN_UNUSED_RESULT;

	bool readFreq(Int16 *freq) SSGNC_WARN_UNUSED_RESULT;
	bool readFreq(StringBuilder *buf, Int16 *freq) SSGNC_WARN_UNUSED_RESULT;

	bool readToken(Int32 *token) SSGNC_WARN_UNUSED_RESULT;
	bool readToken(StringBuilder *buf, Int32 *token) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return stream_ != NULL; }

	bool bad() const
	{ return stream_ == NULL || stream_->bad(); }
	bool eof() const
	{ return stream_ == NULL || (pos_ >= buf_.size() && stream_->eof()); }
	bool good() const
	{ return !bad() && (pos_ < buf_.size() || stream_->good()); }
	bool fail() const
	{ return bad() || (pos_ >= buf_.size() && stream_->fail()); }

	UInt64 tell() const { return total_; }

	enum { DEFAULT_BUF_SIZE = 1 << 12 };
	enum { MAX_FREQ_LENGTH = 2, MAX_TOKEN_LENGTH = 5 };

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

inline bool ByteReader::read(Int8 *byte)
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
			SSGNC_ERROR << "ssgnc::ByteReader::fill() failed" << std::endl;
		return false;
	}

	++total_;
	*byte = buf_[pos_++];
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_BYTE_READER_H
