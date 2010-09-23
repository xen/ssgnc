#ifndef SSGNC_STRING_H
#define SSGNC_STRING_H

#include "common.h"

namespace ssgnc {

class String
{
public:
	String() : ptr_(NULL), length_(0) {}
	String(const Int8 *str) : ptr_(str), length_(lengthOf(str)) {}
	String(const Int8 *str, UInt32 length) : ptr_(str), length_(length) {}
	String(const Int8 *begin, const Int8 *end)
		: ptr_(begin), length_(end - begin) {}
	String(const String &str) : ptr_(str.ptr_), length_(str.length_) {}
	~String() {}

	String &operator=(const String &str);
	Int8 operator[](UInt32 index) const { return ptr_[index]; }

	bool operator==(const String &str) const;
	bool operator!=(const String &str) const { return !(*this == str); }

	bool operator<(const String &str) const { return compare(str) < 0; }
	bool operator<=(const String &str) const { return compare(str) <= 0; }
	bool operator>(const String &str) const { return compare(str) > 0; }
	bool operator>=(const String &str) const { return compare(str) >= 0; }

	const Int8 *ptr() const { return ptr_; }
	UInt32 length() const { return length_; }

	const Int8 *begin() const { return ptr_; }
	const Int8 *end() const { return ptr_ + length_; }

	bool empty() const { return length_ == 0; }

	bool contains(Int8 c) const;

	bool startsWith(const String &str) const;
	bool lowerStartsWith(const String &str) const;

	bool endsWith(Int8 c) const
	{ return !empty() && (ptr_[length_ - 1] == c); }

	bool first(Int8 c, UInt32 *pos) const;
	bool last(Int8 c, UInt32 *pos) const;

	int compare(const String &str) const;

	String substr(UInt32 pos) const
	{ return String(ptr_ + pos, length_ - pos); }
	String substr(UInt32 pos, UInt32 length) const
	{ return String(ptr_ + pos, length); }

private:
	const Int8 *ptr_;
	UInt32 length_;

	static UInt32 lengthOf(const Int8 *str);
};

inline String &String::operator=(const String &str)
{
	ptr_ = str.ptr_;
	length_ = str.length_;
	return *this;
}

inline bool String::operator==(const String &str) const
{
	if (length_ != str.length_)
		return false;
	for (UInt32 i = 0; i < length_; ++i)
	{
		if (ptr_[i] != str[i])
			return false;
	}
	return true;
}

inline bool String::contains(Int8 c) const
{
	for (UInt32 i = 0; i < length_; ++i)
	{
		if (ptr_[i] == c)
			return true;
	}
	return false;
}

inline bool String::startsWith(const String &str) const
{
	if (length_ < str.length())
		return false;
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		if (ptr_[i] != str[i])
			return false;
	}
	return true;
}

inline bool String::lowerStartsWith(const String &str) const
{
	if (length_ < str.length())
		return false;
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		if (std::tolower(static_cast<UInt8>(ptr_[i])) !=
			std::tolower(static_cast<UInt8>(str[i])))
			return false;
	}
	return true;
}

inline bool String::first(Int8 c, UInt32 *pos) const
{
	for (UInt32 i = 0; i < length_; ++i)
	{
		if (ptr_[i] == c)
		{
			*pos = i;
			return true;
		}
	}
	return false;
}

inline bool String::last(Int8 c, UInt32 *pos) const
{
	for (UInt32 i = 1; i <= length_; ++i)
	{
		if (ptr_[length_ - i] == c)
		{
			*pos = length_ - i;
			return true;
		}
	}
	return false;
}

inline UInt32 String::lengthOf(const Int8 *str)
{
	if (str == NULL)
		return 0;

	UInt32 length = 0;
	while (str[length] != '\0')
		++length;
	return length;
}

inline int String::compare(const String &str) const
{
	std::size_t i = 0;
	while (i < length_)
	{
		if (i == str.length())
			return 1;

		UInt8 lc = static_cast<UInt8>(ptr_[i]);
		UInt8 rc = static_cast<UInt8>(str[i]);
		if (lc != rc)
			return lc - rc;
		++i;
	}
	return (i == str.length()) ? 0 : -1;
}

}  // namespace ssgnc

inline std::ostream &operator<<(std::ostream &out, const ssgnc::String &str)
{
	return out.write(str.ptr(), str.length());
}

#endif  // SSGNC_STRING_H
