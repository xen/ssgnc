#ifndef SSGNC_NGRAM_READER_H
#define SSGNC_NGRAM_READER_H

#include "byte-reader.h"
#include "file-path.h"
#include "ngram-index.h"

namespace ssgnc {

class NgramReader
{
public:
	NgramReader() : num_tokens_(0), file_path_(), file_(), byte_reader_(),
		min_freq_(1), freq_(-1), total_(0) {}
	~NgramReader();

	bool open(const String &index_dir, Int32 num_tokens,
		const NgramIndex::Entry &entry, Int16 min_freq = 1)
		SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool read(Int16 *freq, std::vector<Int32> *tokens)
		SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return file_path_.is_open(); }

	bool bad() const { return freq_ < 0; }
	bool eof() const { return freq_ >= 0 && freq_ < min_freq_; }
	bool good() const { return freq_ >= min_freq_; }
	bool fail() const { return freq_ < min_freq_; }

	UInt64 tell() const { return total_ + byte_reader_.tell(); }

	Int32 num_tokens() const { return num_tokens_; }
	Int16 min_freq() const { return min_freq_; }
	Int16 freq() const { return freq_; }

private:
	Int32 num_tokens_;
	FilePath file_path_;
	std::ifstream file_;
	ByteReader byte_reader_;
	Int16 min_freq_;
	Int16 freq_;
	UInt64 total_;

	enum { BYTE_READER_BUF_SIZE = 16 << 10 };

	bool openNextFile();

	bool readFreq();
	bool readTokens(std::vector<Int32> *tokens);

	// Disallows copies.
	NgramReader(const NgramReader &);
	NgramReader &operator=(const NgramReader &);
};

}  // namespace ssgnc

#endif  // SSGNC_NGRAM_READER_H
