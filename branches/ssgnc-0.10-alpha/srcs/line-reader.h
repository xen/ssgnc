#ifndef GOOGLE_NGRAM_LINE_READER_H
#define GOOGLE_NGRAM_LINE_READER_H

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace ngram
{

class line_reader
{
public:
	enum { BLOCK_SIZE = 1 << 12 };

	explicit line_reader(std::istream &stream)
		: stream_(stream), buf_(), pos_(0) { fill(); }

	// Reads a line.
	bool read(std::string *line)
	{
		size_t start = pos_;
		while (pos_ < buf_.size() && buf_[pos_] != '\n')
			++pos_;
		if (pos_ == buf_.size())
		{
			if (!fill(&start))
				return false;

			while (pos_ < buf_.size() && buf_[pos_] != '\n')
				++pos_;
		}
		if (buf_[pos_] != '\n')
			return false;

		line->assign(&buf_[start], pos_ - start);
		++pos_;
		return true;
	}

private:
	std::istream &stream_;
	std::vector<char> buf_;
	std::size_t pos_;

	// Copies are not allowed.
	line_reader(const line_reader &);
	line_reader &operator=(const line_reader &);

private:
	// Reads bytes and fills an internal buffer.
	bool fill(std::size_t *start = 0)
	{
		if (!stream_)
			return false;

		std::size_t avail = (buf_.size() - (start ? *start : 0));
		if (!buf_.empty() && avail)
			std::memmove(&buf_[0], &buf_[(start ? *start : 0)], avail);

		if (start)
		{
			pos_ -= *start;
			*start = 0;
		}

		buf_.resize(BLOCK_SIZE);
		stream_.read(&buf_[avail], BLOCK_SIZE - avail);
		buf_.resize(avail + stream_.gcount());
		return true;
	}
};

}  // ngram

#endif  // GOOGLE_NGRAM_LINE_READER_H
