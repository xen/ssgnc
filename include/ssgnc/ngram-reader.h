#ifndef SSGNC_NGRAM_READER_H
#define SSGNC_NGRAM_READER_H

#include "file-path.h"

namespace ssgnc {

class NgramReader
{
public:
	NgramReader() : num_tokens_(0), file_path_(), file_(), byte_reader_(),
		min_freq_(1), freq_(0), total_(0) {}
	~NgramReader() { close(); }

	bool open(Int32 num_tokens, const String &format, Int32 file_id,
		UInt32 offset, Int16 min_freq = 1) SSGNC_WARN_UNUSED_RESULT;
	void close();

	bool read(Int16 *freq, std::vector<Int32> *tokens)
		SSGNC_WARN_UNUSED_RESULT;

	UInt64 total() const { return total_ + byte_reader_.tell(); }

private:
	Int32 num_tokens_;
	FilePath file_path_;
	std::ifstream file_;
	ByteReader byte_reader_;
	Int16 min_freq_;
	Int16 freq_;
	UInt64 total_;

	enum { BYTE_READER_BUF_SIZE = 1 << 16 };

	bool openNextFile();

	bool readFreq();
	bool readTokens(std::vector<Int32> *tokens);

	// Disallows copies.
	NgramReader(const NgramReader &);
	NgramReader &operator=(const NgramReader &);
};

}  // namespace ssgnc

#endif  // SSGNC_NGRAM_READER_H
