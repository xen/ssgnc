#ifndef SSGNC_VARINT_READER_H
#define SSGNC_VARINT_READER_H

#include "byte-reader.h"

namespace ssgnc {

class VarintReader
{
public:
	explicit VarintReader(ByteReader *reader) : reader_(reader) {}

	// Reads bytes and decodes those bytes to an unsigned integer.
	template <typename ValueType>
	bool Read(ValueType *value)
	{
		int count = 0;
		unsigned char c;
		if (!reader_->Read(&c))
			return false;
		*value = static_cast<ValueType>(c & 0x7F);
		while (c & 0x80)
		{
			if (!reader_->Read(&c))
				return false;
			*value |= static_cast<ValueType>(c & 0x7F) << (7 * ++count);
		}
		return true;
	}

private:
	ByteReader *reader_;

	// Disallows copies.
	VarintReader(const VarintReader &);
	VarintReader &operator=(const VarintReader &);
};

}  // namespace ssgnc

#endif  // SSGNC_VARINT_READER_H
