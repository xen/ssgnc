#ifndef SSGNC_VOCAB_PAIR_H
#define SSGNC_VOCAB_PAIR_H

#include "path-generator.h"
#include "vocab-dic.h"
#include "vocab-index.h"

namespace ssgnc {

class VocabPair
{
public:
	VocabPair() : dic_file_(), index_file_(), dic_(), index_() {}

	// Closes vocabulary files.
	void Clear()
	{
		dic_.Clear();
		index_.Clear();
		dic_file_.Unmap();
		index_file_.Unmap();
	}

	// Opens vocabulary files.
	bool Open(const PathGenerator &path_gen)
	{
		Clear();
		if (!dic_file_.Map(path_gen.VocabDicPath().c_str()))
			return false;
		if (!index_file_.Map(path_gen.VocabIndexPath().c_str()))
		{
			dic_file_.Unmap();
			return false;
		}
		dic_.MapDic(dic_file_);
		index_.MapIndex(index_file_);
		return true;
	}

	// Fills key IDs in a query.
	bool FillQuery(Query *query) const { return dic_.FillQuery(query); }
	// Fills key strings in an n-gram.
	bool FillNgram(Ngram *ngram) const { return index_.FillNgram(ngram); }

private:
	FileMapper dic_file_;
	FileMapper index_file_;
	VocabDic dic_;
	VocabIndex index_;

	// Disallows copies.
	VocabPair(const VocabPair &);
	VocabPair &operator=(const VocabPair &);
};

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_PAIR_H
