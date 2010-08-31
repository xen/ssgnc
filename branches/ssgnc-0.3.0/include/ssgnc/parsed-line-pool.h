#ifndef SSGNC_PARSED_LINE_POOL_H
#define SSGNC_PARSED_LINE_POOL_H

#include <ssgnc/memory-pool.h>
#include <ssgnc/parsed-line.h>
#include <ssgnc/temp-file.h>

#include <vector>

namespace ssgnc {

class ParsedLinePool
{
public:
	explicit ParsedLinePool(std::size_t chunk_size = 0);
	~ParsedLinePool() { Clear(); }

	bool is_sorted() const { return is_sorted_; }

	std::size_t num_keys() const { return num_keys_; }
	std::size_t num_lines() const { return pairs_.size(); }
	std::size_t total_size() const;

	// Encodes a parsed line and keeps it as a pair of key and value.
	void Append(const ParsedLine &line);

	// Sorts pairs by values.
	void Sort();

	// Writes keys into a temporary file.
	void Write(TempFile *file);

	void Clear();
	void Swap(ParsedLinePool *target);

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
		long long value() const { return value_; }

		void set_key(const char *key) { key_ = key; }
		void set_value(long long value) { value_ = value; }

	private:
		const char *key_;
		long long value_;
	};

private:
	enum { MAX_NUM_KEYS = 32 };
	enum { ENCODER_BUF_SIZE = MAX_NUM_KEYS * 8 };

	MemoryPool key_pool_;
	std::vector<KeyValuePair> pairs_;
	bool is_sorted_;
	std::size_t num_keys_;

	// Disallows copies.
	ParsedLinePool(const ParsedLinePool &);
	ParsedLinePool &operator=(const ParsedLinePool &);

	static std::size_t Encode(const ParsedLine &line, char *buf);
};

inline std::size_t ParsedLinePool::total_size() const
{
	return key_pool_.total_size() + sizeof(KeyValuePair) * pairs_.capacity();
}

}  // namespace ssgnc

#endif  // SSGNC_PARSED_LINE_POOL_H
