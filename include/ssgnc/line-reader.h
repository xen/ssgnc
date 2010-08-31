#ifndef SSGNC_LINE_READER_H
#define SSGNC_LINE_READER_H

#include <iostream>
#include <string>
#include <vector>

namespace ssgnc {

// To read lines from an input stream.
// For fast reading, this class uses an internal buffer.
class LineReader
{
public:
	explicit LineReader(std::istream *stream, std::size_t buf_size = 0);

	// The size of internal buffer.
	std::size_t buf_size() const { return buf_size_; }

	// Reads a line and assigns it to the given std::string.
	bool Read(std::string *line);

	// Reads a line (the buffer will be broken by the next call).
	bool Read(const char **line, std::size_t *length_ptr = NULL);

private:
	// Lines must be shorter than this size.
	enum { DEFAULT_BUF_SIZE = 1 << 12 };

	std::istream *stream_;
	std::size_t buf_size_;
	std::vector<char> buf_;
	std::size_t pos_;

	// Disallows copies.
	LineReader(const LineReader &);
	LineReader &operator=(const LineReader &);

	// Fills the internal buffer.
	bool Fill(std::size_t *start);
};

}  // namespace ssgnc

#endif  // SSGNC_LINE_READER_H
