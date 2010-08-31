#ifndef SSGNC_PARSED_LINE_READER_H
#define SSGNC_PARSED_LINE_READER_H

#include <ssgnc/parsed-line.h>
#include <ssgnc/temp-file.h>

#include <vector>

namespace ssgnc {

class ParsedLineReader
{
public:
	explicit ParsedLineReader(std::size_t buf_size = 0);
	~ParsedLineReader() { Clear(); }

	std::size_t file_cur() const { return file_cur_; }
	std::size_t file_end() const { return file_end_; }
	std::size_t buf_size() const { return buf_size_; }

	const ParsedLine &parsed_line() const { return parsed_line_; }
	const char *bytes() const { return &buf_[pos_ - length_]; }
	std::size_t length() const { return length_; }

	void Open(TempFile *file, std::size_t file_begin, std::size_t file_end);
	void Close();

	bool Next();

	void Clear() { Close(); }
	void Swap(ParsedLineReader *target);

private:
	enum { DEFAULT_BUF_SIZE = 1 << 20 };

	TempFile *file_;
	std::size_t file_cur_;
	std::size_t file_end_;
	std::size_t buf_size_;
	std::vector<char> buf_;
	std::size_t pos_;
	std::size_t length_;
	ParsedLine parsed_line_;
	std::size_t num_keys_;
	std::size_t num_lines_;
	std::size_t count_;

	// Disallows copies.
	ParsedLineReader(const ParsedLineReader &);
	ParsedLineReader &operator=(const ParsedLineReader &);

	void Fill();
	bool Parse();
};

}  // namespace ssgnc

#endif  // SSGNC_PARSED_LINE_READER_H
