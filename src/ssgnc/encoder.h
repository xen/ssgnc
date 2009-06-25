#ifndef SSGNC_ENCODER_H
#define SSGNC_ENCODER_H

#include "varint-writer.h"
#include "vocab-dic.h"

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace ssgnc {

class Encoder
{
public:
	explicit Encoder(std::ostream *output)
		: byte_writer_(output), writer_(&byte_writer_) {}

	// Encodes a line into a list of key IDs.
	bool Encode(const VocabDic &dic, const char *line)
	{
		bool is_end = false;
		while (!is_end)
		{
			// Extracts a key.
			const char *delim = std::strchr(line, ' ');
			if (delim == NULL)
			{
				delim = std::strchr(line, '\t');
				if (delim == NULL)
					return false;
				is_end = true;
			}

			// Lookups a dictionary and encodes a key ID.
			int key_id;
			std::size_t length = std::size_t(delim - line);
			if (!dic.Find(line, length, &key_id))
				return false;
			writer_.Write(key_id);

			line = delim + 1;
		}

		// Encodes a frequency.
		char *end;
		long long freq = std::strtoll(line, &end, 10);
		if (freq <= 0 || *end != '\0')
			return false;
		writer_.Write(freq);

		return true;
	}

private:
	ssgnc::ByteWriter byte_writer_;
	ssgnc::VarintWriter writer_;

	// Disallows copies.
	Encoder(const Encoder &);
	Encoder &operator=(const Encoder &);
};

}  // ssgnc

#endif  // SSGNC_ENCODER_H
