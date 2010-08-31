#ifndef SSGNC_BACK_DIVIDER_H
#define SSGNC_BACK_DIVIDER_H

#include <ssgnc/front-merger.h>
#include <ssgnc/memory-pool.h>
#include <ssgnc/temp-file.h>

#include <vector>

namespace ssgnc {

class BackDivider
{
public:
	BackDivider();
	~BackDivider() { Clear(); }

	std::size_t total_size() const;

	void Open(TempFile *temp_file, std::vector<std::size_t> *points);
	void Close();

	void Divide(const ParsedLine &parsed_line,
		const char *bytes, std::size_t length);

	void Clear() { Close(); }
	void Swap(BackDivider *target);

public:
	class KeyValuePair
	{
	public:
		KeyValuePair() : key_(NULL), value_(0) {}
		KeyValuePair(const KeyValuePair &pair) :
			key_(pair.key_), value_(pair.value_) {}
		KeyValuePair &operator=(const KeyValuePair &pair)
		{ key_ = pair.key_; value_ = pair.value_; return *this; }

		int key() const { return key_; }
		int value() const { return value_; }

		void set_key(int key) { key_ = key; }
		void set_value(int value) { value_ = value; }

		bool operator==(const KeyValuePair &rhs)
		{ return key_ == rhs.key_ && value_ == rhs.value_; }

	private:
		int key_;
		int value_;
	};

private:
	enum { MAX_NUM_PAIRS = 1 << 25 };
	enum { MAX_TOTAL_SIZE = MAX_NUM_PAIRS << 3 };

	TempFile *file_;
	std::vector<std::size_t> *points_;
	MemoryPool value_pool_;
	std::vector<const char *> values_;
	std::vector<KeyValuePair> pairs_;

	// Disallows copies.
	BackDivider(const BackDivider &);
	BackDivider &operator=(const BackDivider &);

	void Flush();

	void Write(const KeyValuePair &pair, std::size_t num_keys);
	void WriteValue(const char *value, std::size_t num_keys);
	void WriteKey(unsigned int key);
};

inline std::size_t BackDivider::total_size() const
{
	return value_pool_.total_size()
		+ sizeof(const char *) * values_.capacity()
		+ sizeof(KeyValuePair) * pairs_.capacity();
}

}  // namespace ssgnc

#endif  // SSGNC_BACK_DIVIDER_H
