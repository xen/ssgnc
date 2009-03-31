#ifndef GOOGLE_NGRAM_VARIABLE_BYTE_ENCODING_WRITER_H
#define GOOGLE_NGRAM_VARIABLE_BYTE_ENCODING_WRITER_H

#include "byte-writer.h"

namespace ngram
{

class vbe_writer
{
public:
	explicit vbe_writer(byte_writer &writer) : writer_(writer) {}

	// Encodes an unsigned integer and writes it to a stream.
	template <typename ValueType>
	void put(ValueType value)
	{
		unsigned char byte = static_cast<unsigned char>(value & 0x7F);
		while (value >>= 7)
		{
			writer_.put(byte | 0x80);
			byte = static_cast<unsigned char>(value & 0x7F);
		}
		writer_.put(byte);
	}

	// Encodes an unsigned integer and writes it to a stream.
	template <typename ValueType>
	vbe_writer &operator<<(ValueType value)
	{
		put(value);
		return *this;
	}

private:
	byte_writer &writer_;

	// Copies are not allowed.
	vbe_writer(const vbe_writer &);
	vbe_writer &operator=(const vbe_writer &);
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_VARIABLE_BYTE_ENCODING_WRITER_H
