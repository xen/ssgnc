#ifndef SSGNC_VARINT_WRITER_H
#define SSGNC_VARINT_WRITER_H

#include "byte-writer.h"

namespace ssgnc {

class VarintWriter
{
public:
	explicit VarintWriter(ByteWriter *writer) : writer_(writer) {}

	// Encodes an unsigned integer and writes it to an output stream.
	template <typename ValueType>
	void Write(ValueType value)
	{
		unsigned char byte = static_cast<unsigned char>(value & 0x7F);
		while (value >>= 7)
		{
			writer_->Write(byte | 0x80);
			byte = static_cast<unsigned char>(value & 0x7F);
		}
		writer_->Write(byte);
	}

private:
	ByteWriter *writer_;

	// Disallows copies.
	VarintWriter(const VarintWriter &);
	VarintWriter &operator=(const VarintWriter &);
};

}  // namespace ssgnc

#endif  // SSGNC_VARINT_WRITER_H
