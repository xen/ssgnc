#ifndef SSGNC_INDEX_READER_H
#define SSGNC_INDEX_READER_H

#include "index-finder.h"
#include "mem-streambuf.h"
#include "varint-reader.h"

#include <iostream>

namespace ssgnc {

class IndexReader
{
public:
	typedef IndexFinder::RangeType RangeType;

	explicit IndexReader(const RangeType &range)
		: streambuf_(range.first, range.second), stream_(&streambuf_),
		byte_reader_(&stream_), reader_(&byte_reader_), position_(0) {}

	// Reads a position.
	bool Read(std::size_t *position)
	{
		std::size_t diff;
		if (!reader_.Read(&diff))
			return false;
		position_ += diff;
		*position = position_;
		return true;
	}

private:
	MemStreambuf streambuf_;
	std::istream stream_;
	ByteReader byte_reader_;
	VarintReader reader_;
	std::size_t position_;

	// Disallows copies.
	IndexReader(const IndexReader &);
	IndexReader &operator=(const IndexReader &);
};

}  // namespace ssgnc

#endif  // SSGNC_INDEX_READER_H
