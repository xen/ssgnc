#ifndef GOOGLE_NGRAM_UNIT_TYPE_H
#define GOOGLE_NGRAM_UNIT_TYPE_H

#include <vector>

namespace ngram
{

class unit_type
{
public:
	unit_type() : freq_(0), unigrams_() {}

	void clear() { unigrams_.clear(); }

	void set_freq(long long freq) { freq_ = freq; }
	void add_unigram(int unigram) { unigrams_.push_back(unigram); }

	const int &operator[](int index) const { return unigrams_[index]; }
	int size() const { return unigrams_.size(); }

	long long freq() const { return freq_; }
	const std::vector<int> &unigrams() const { return unigrams_; }

private:
	long long freq_;
	std::vector<int> unigrams_;
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_UNIT_TYPE_H
