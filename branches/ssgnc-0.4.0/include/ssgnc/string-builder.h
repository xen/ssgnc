#ifndef SSGNC_STRING_BUILDER_H
#define SSGNC_STRING_BUILDER_H

#include "string.h"

namespace ssgnc {

class StringBuilder
{
public:
	StringBuilder();
	~StringBuilder();

	void clear();

	Int8 &operator[](UInt32 index) { return buf_[index]; }
	const Int8 &operator[](UInt32 index) const { return buf_[index]; }

	Int8 *buf() { return buf_; }
	UInt32 length() const { return length_; }
	UInt32 size() const { return size_; }

	const Int8 *ptr() const { return buf_; }
	String str() const { return String(buf_, length_); }

	StringBuilder &operator=(const String &str) { return assign(str); }

	StringBuilder &assign(const String &str);

	void append() { buf_[length_] = '\0'; }
	void append(Int8 byte);
	void append(const String &str);

	void resize(UInt32 length);

private:
	enum { INITIAL_BUF_SIZE = sizeof(Int8 *) };

	Int8 initial_buf_[INITIAL_BUF_SIZE];
	Int8 *buf_;
	UInt32 length_;
	UInt32 size_;

	void resizeBuf(UInt32 size);

	// Disallows copies.
	StringBuilder(const StringBuilder &);
	StringBuilder &operator=(const StringBuilder &);
};

inline StringBuilder::StringBuilder() : initial_buf_(), buf_(initial_buf_),
	length_(0), size_(sizeof(initial_buf_))
{
	buf_[0] = '\0';
}

inline StringBuilder::~StringBuilder()
{
	if (buf_ != initial_buf_)
		delete [] buf_;
}

inline void StringBuilder::clear()
{
	buf_[0] = '\0';
	length_ = 0;
}

inline StringBuilder &StringBuilder::assign(const String &str)
{
	clear();

	append(str);
	return *this;
}

inline void StringBuilder::append(Int8 byte)
{
	if (length_ + 1 >= size_)
		resizeBuf(length_ + 2);

	buf_[length_++] = byte;
}

inline void StringBuilder::append(const String &str)
{
	if (length_ + str.length() >= size_)
		resizeBuf(length_ + str.length() + 1);

	for (std::size_t i = 0; i < str.length(); ++i)
		buf_[length_++] = str[i];
}

inline void StringBuilder::resize(UInt32 length)
{
	if (length >= size_)
		resizeBuf(length + 1);
	length_ = length;
}

inline void StringBuilder::resizeBuf(UInt32 size)
{
	UInt32 new_buf_size = size_;
	while (new_buf_size < size)
		new_buf_size *= 2;

	Int8 *new_buf = new Int8[new_buf_size];
	for (UInt32 i = 0; i < length_; ++i)
		new_buf[i] = buf_[i];
	if (buf_ != initial_buf_)
		delete [] buf_;
	buf_ = new_buf;
	size_ = new_buf_size;
}

}  // namespace ssgnc

#endif  // SSGNC_STRING_BUILDER_H
