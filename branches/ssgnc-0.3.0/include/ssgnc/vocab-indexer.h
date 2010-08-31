#ifndef SSGNC_VOCAB_INDEXER_H
#define SSGNC_VOCAB_INDEXER_H

#include <ssgnc/memory-pool.h>

#include <iostream>
#include <vector>

namespace ssgnc {

class VocabIndexer
{
public:
	VocabIndexer() : key_pool_(), pairs_() {}
	~VocabIndexer() { Clear(); }

	std::size_t num_keys() const { return pairs_.size(); }

	// Reads keys and values from an input stream.
	// This function must not be called more than once.
	void ReadVocab(std::istream *in);

	// Builds an index and a dictionary from keys and values.
	// Key-value pairs will be broken in this function.
	void Build(std::ostream *index, std::ostream *dic);

	void Clear();
	void Swap(VocabIndexer *target);

public:
	class KeyValuePair
	{
	public:
		KeyValuePair() : key_(NULL), value_(0) {}
		KeyValuePair(const KeyValuePair &pair) :
			key_(pair.key_), value_(pair.value_) {}
		KeyValuePair &operator=(const KeyValuePair &pair)
		{ key_ = pair.key_; value_ = pair.value_; return *this; }

		const char *key() const { return key_; }
		std::size_t value() const { return value_; }

		void set_key(const char *key) { key_ = key; }
		void set_value(std::size_t value) { value_ = value; }

	private:
		const char *key_;
		std::size_t value_;
	};

private:
	MemoryPool key_pool_;
	std::vector<KeyValuePair> pairs_;

	// Disallows copies.
	VocabIndexer(const VocabIndexer &);
	VocabIndexer &operator=(const VocabIndexer &);

	// Builds an index from keys and values.
	void BuildIndex(std::ostream *out);

	// Builds a dictionary from keys and values.
	void BuildDic(std::ostream *out);
};

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_INDEXER_H
