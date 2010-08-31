#ifndef SSGNC_FRONT_ENCODER_H
#define SSGNC_FRONT_ENCODER_H

#include <ssgnc/temp-file.h>
#include <ssgnc/vocab-dic.h>

#include <iostream>
#include <vector>

namespace ssgnc {

class FrontEncoder
{
public:
	FrontEncoder() : dic_() {}
	~FrontEncoder() { Clear(); }

	void Open(const char *dic_file_name);
	void Close();

	void Encode(std::istream *in, TempFile *temp_file,
		std::vector<std::size_t> *points);

	void Clear() { Close(); }
	void Swap(FrontEncoder *target);

private:
	enum { MAX_NUM_LINES = 1 << 23 };
	enum { MAX_TOTAL_SIZE = MAX_NUM_LINES << 5 };

	VocabDic dic_;

	// Disallows copies.
	FrontEncoder(const FrontEncoder &);
	FrontEncoder &operator=(const FrontEncoder &);
};

}  // namespace ssgnc

#endif  // SSGNC_FRONT_ENCODER_H
