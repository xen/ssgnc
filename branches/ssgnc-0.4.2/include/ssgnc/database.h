#ifndef SSGNC_DATABASE_H
#define SSGNC_DATABASE_H

#include "agent.h"
#include "vocab-dic.h"

namespace ssgnc {

class Database
{
public:
	Database();
	~Database();

	bool open(const String &index_dir,
		FileMap::Mode mode = FileMap::DEFAULT_MODE) SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool parseQuery(const String &str, Query *query,
		const String &meta_token = "*") const SSGNC_WARN_UNUSED_RESULT;

	bool search(const Query &query, Agent *agent) const
		SSGNC_WARN_UNUSED_RESULT;

	bool decode(Int16 encoded_freq, const std::vector<Int32> &tokens,
		StringBuilder *ngram) const SSGNC_WARN_UNUSED_RESULT;
	bool decode(Int16 encoded_freq, const std::vector<Int32> &token_ids,
		Int64 *freq, std::vector<String> *token_strs) const 
		SSGNC_WARN_UNUSED_RESULT;

	bool decodeFreq(Int16 encoded_freq, Int64 *freq) const
		SSGNC_WARN_UNUSED_RESULT;

	bool decodeTokens(const std::vector<Int32> &token_ids,
		std::vector<String> *token_strs) const SSGNC_WARN_UNUSED_RESULT;
	bool decodeTokens(const std::vector<Int32> &tokens,
		StringBuilder *tokens_str) const SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return vocab_dic_.is_open(); }

	String index_dir() const { return index_dir_.str(); }
	const VocabDic &vocab_dic() const { return vocab_dic_; }
	const NgramIndex &ngram_index() const { return ngram_index_; }

	UInt32 num_keys() const { return vocab_dic_.num_keys(); }
	Int32 max_num_tokens() const { return ngram_index_.max_num_tokens(); }
	Int32 max_token_id() const { return ngram_index_.max_token_id(); }

private:
	StringBuilder index_dir_;
	VocabDic vocab_dic_;
	NgramIndex ngram_index_;
	FreqHandler freq_handler_;

	static String findDelim(const String &str);

	// Disallows copies.
	Database(const Database &);
	Database &operator=(const Database &);
};

}  // namespace ssgnc

#endif  // SSGNC_DATABASE_H
