#ifndef SSGNC_STRING_BUILDER_H
#define SSGNC_STRING_BUILDER_H

#include "string.h"

namespace ssgnc {

class StringBuilder
{
public:
	StringBuilder() : initial_buf_(), buf_(initial_buf_),
		length_(0), size_(sizeof(initial_buf_)) {}
	~StringBuilder();

	void clear() { length_ = 0; }

	Int8 &operator[](UInt32 index) { return buf_[index]; }
	const Int8 &operator[](UInt32 index) const { return buf_[index]; }

	Int8 *buf() { return buf_; }
	UInt32 length() const { return length_; }
	UInt32 size() const { return size_; }

	const Int8 *ptr() const { return buf_; }
	String str() const { return String(buf_, length_); }

	bool empty() const { return length_ == 0; }

	bool append() SSGNC_WARN_UNUSED_RESULT;
	bool append(Int8 byte) SSGNC_WARN_UNUSED_RESULT;
	bool append(const String &str) SSGNC_WARN_UNUSED_RESULT;

	bool appendf(const Int8 *format, ...) SSGNC_WARN_UNUSED_RESULT;

	bool resize(UInt32 length) SSGNC_WARN_UNUSED_RESULT;

private:
	enum { INITIAL_BUF_SIZE = sizeof(Int8 *) };

	Int8 initial_buf_[INITIAL_BUF_SIZE];
	Int8 *buf_;
	UInt32 length_;
	UInt32 size_;

	bool resizeBuf(UInt32 size);

	// Disallows copies.
	StringBuilder(const StringBuilder &);
	StringBuilder &operator=(const StringBuilder &);
};

inline StringBuilder::~StringBuilder()
{
	if (buf_ != initial_buf_)
		delete [] buf_;
}

inline bool StringBuilder::append()
{
	if (length_ >= size_)
	{
		if (!resizeBuf(length_ + 1))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::resizeBuf() failed: "
				<< (length_ + 1) << std::endl;
			return false;
		}
	}
	buf_[length_] = '\0';
	return true;
}

inline bool StringBuilder::append(Int8 byte)
{
	if (length_ >= size_)
	{
		if (!resizeBuf(length_ + 1))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::resizeBuf() failed: "
				<< (length_ + 1) << std::endl;
			return false;
		}
	}
	buf_[length_++] = byte;
	return true;
}

inline bool StringBuilder::append(const String &str)
{
	if (length_ + str.length() >= size_)
	{
		if (!resizeBuf(length_ + str.length()))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::resizeBuf() failed: "
				<< (length_ + str.length()) << std::endl;
			return false;
		}
	}
	for (std::size_t i = 0; i < str.length(); ++i)
		buf_[length_++] = str[i];
	return true;
}

inline bool StringBuilder::resize(UInt32 length)
{
	if (length >= size_)
	{
		if (!resizeBuf(length))
			return false;
	}
	length_ = length;
	return true;
}

}  // namespace ssgnc

inline std::ostream &operator<<(std::ostream &out,
	const ssgnc::StringBuilder &builder)
{
	return out << builder.str();
}

#endif  // SSGNC_STRING_BUILDER_H
