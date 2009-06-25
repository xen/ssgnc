#ifndef SSGNC_NGRAM_H
#define SSGNC_NGRAM_H

#include <vector>

namespace ssgnc {

class Ngram
{
public:
	Ngram() : unigram_(), freq_(0) {}

	void Clear() { clear_unigram(); clear_freq(); }

	void clear_unigram() { unigram_.clear(); }
	void clear_freq() { freq_ = 0; }

	void add_unigram(int unigram) { unigram_.push_back(unigram); }
	void set_freq(long long freq) { freq_ = freq; }

	int *mutable_unigram(int i) { return &unigram_[i]; }
	long long *mutable_freq() { return &freq_; }

	const int &unigram(int i) const { return unigram_[i]; }
	long long freq() const { return freq_; }

	const int unigram_size() const
	{
		return static_cast<int>(unigram_.size());
	}

private:
	std::vector<int> unigram_;
	long long freq_;

	// Copiable.
};

}  // namespace ssgnc

#endif  // SSGNC_NGRAM_H
