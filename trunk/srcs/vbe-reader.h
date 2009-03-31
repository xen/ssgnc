#ifndef GOOGLE_NGRAM_VARIABLE_BYTE_ENCODING_READER_H
#define GOOGLE_NGRAM_VARIABLE_BYTE_ENCODING_READER_H

#include "byte-reader.h"

namespace ngram
{

class vbe_reader
{
public:
	explicit vbe_reader(byte_reader &reader) : reader_(reader) {}

	// Reads bytes and decodes those bytes to an unsigned integer.
	template <typename ValueType>
	ValueType get()
	{
		int count = 0;
		int c = reader_.get();
		ValueType value = c & 0x7F;
		while ((c != EOF) && (c & 0x80))
		{
			c = reader_.get();
			value |= static_cast<ValueType>(c & 0x7F) << (7 * ++count);
		}
		return (c != EOF) ? value : 0;
	}

	// Reads bytes and decodes those bytes to an unsigned integer.
	template <typename ValueType>
	vbe_reader &operator>>(ValueType &value)
	{
		value = get<ValueType>();
		return *this;
	}

private:
	byte_reader &reader_;

	// Copies are not allowed.
	vbe_reader(const vbe_reader &);
	vbe_reader &operator=(const vbe_reader &);
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_VARIABLE_BYTE_ENCODING_READER_H
