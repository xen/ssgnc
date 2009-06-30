#ifndef SSGNC_NGRAM_H
#define SSGNC_NGRAM_H

#include <vector>

namespace ssgnc {

class Ngram
{
public:
	Ngram() : key_(), freq_(0) {}

	void Clear() { clear_key(); clear_freq(); }

	void clear_key() { key_.clear(); }
	void add_key(int key) { key_.push_back(key); }
	int *mutable_key(int i) { return &key_[i]; }
	const int &key(int i) const { return key_[i]; }
	int key_size() const { return static_cast<int>(key_.size()); }

	void clear_freq() { freq_ = 0; }
	void set_freq(long long freq) { freq_ = freq; }
	long long *mutable_freq() { return &freq_; }
	long long freq() const { return freq_; }

private:
	std::vector<int> key_;
	long long freq_;

	// Copiable.
};

}  // namespace ssgnc

#endif  // SSGNC_NGRAM_H
