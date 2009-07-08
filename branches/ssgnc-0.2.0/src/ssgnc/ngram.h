#ifndef SSGNC_NGRAM_H
#define SSGNC_NGRAM_H

#include <vector>

namespace ssgnc {

class Ngram
{
public:
	Ngram() : key_id_(), key_string_(), freq_(0) {}

	void Clear()
	{
		clear_key_id();
		clear_key_string();
		clear_freq();
	}

	void clear_key_id() { key_id_.clear(); }
	void add_key_id(int key_id) { key_id_.push_back(key_id); }
	int *mutable_key_id(int i) { return &key_id_[i]; }
	const int &key_id(int i) const { return key_id_[i]; }
	int key_id_size() const { return static_cast<int>(key_id_.size()); }

	void clear_key_string() { key_string_.clear(); }
	void add_key_string(const char *key_string)
	{
		key_string_.push_back(key_string);
	}
	const char **mutable_key_string(int i) { return &key_string_[i]; }
	const char * const &key_string(int i) const { return key_string_[i]; }
	int key_string_size() const
	{
		return static_cast<int>(key_string_.size());
	}

	void clear_freq() { freq_ = 0; }
	void set_freq(long long freq) { freq_ = freq; }
	long long *mutable_freq() { return &freq_; }
	long long freq() const { return freq_; }

private:
	std::vector<int> key_id_;
	std::vector<const char *> key_string_;
	long long freq_;

	// Copiable.
};

}  // namespace ssgnc

#endif  // SSGNC_NGRAM_H
