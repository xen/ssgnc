#ifndef SSGNC_INDEXER_H
#define SSGNC_INDEXER_H

#include "varint-reader.h"

#include <iostream>

namespace ssgnc {

class Indexer
{
public:
	Indexer(std::istream *input)
		: byte_reader_(input), reader_(&byte_reader_) {}

	// Builds an index for a line.
	bool Index(int n, std::ostream *output)
	{
		std::size_t last_address = byte_reader_.Total();
		for (int i = 0; i < n; ++i)
		{
			int key_id;
			if (!reader_.Read(&key_id))
				return false;

			*output << key_id << ' ' << last_address << '\n';
		}

		long long freq;
		return reader_.Read(&freq);
	}

private:
	ssgnc::ByteReader byte_reader_;
	ssgnc::VarintReader reader_;

	// Disallows copies.
	Indexer(const Indexer &);
	Indexer &operator=(const Indexer &);
};

}  // ssgnc

#endif  // SSGNC_INDEXER_H
