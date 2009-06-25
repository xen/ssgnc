#ifndef SSGNC_TEXT_FINDER_H
#define SSGNC_TEXT_FINDER_H

#include "file-mapper.h"
#include "ngram.h"

#include <cstring>
#include <string>

namespace ssgnc {

class TextFinder
{
public:
	TextFinder(const FileMapper &file, int n)
		: address_(file.Pointer<char>()), size_(file.Size()), n_(n) {}
	TextFinder(const void *address, std::size_t size, int n)
		: address_(static_cast<const char *>(address)), size_(size), n_(n) {}

	// Finds an n-gram.
	bool Find(std::size_t position, Ngram *ngram) const
	{
		if (position >= size_)
			return false;
		ngram->Clear();

		// Reads unigrams.
		for (int i = 0; i < n_; ++i)
		{
			int key_id;
			if (!Read(&position, &key_id))
				return false;
			ngram->add_unigram(key_id);
		}

		// Reads a frequency.
		long long freq;
		if (!Read(&position, &freq))
			return false;
		ngram->set_freq(freq);

		return true;
	}

private:
	const char *address_;
	const std::size_t size_;
	const int n_;

	// Disallows copies.
	TextFinder(const TextFinder &);
	TextFinder &operator=(const TextFinder &);

private:
	// Reads an unsigned integer.
	template <typename ValueType>
	bool Read(std::size_t *position, ValueType *value) const
	{
		int count = 0;
		int c = static_cast<unsigned char>(address_[(*position)++]);
		*value = static_cast<ValueType>(c) & 0x7F;
		while (c & 0x80)
		{
			c = static_cast<unsigned char>(address_[(*position)++]);
			*value |= static_cast<ValueType>(c & 0x7F) << (7 * ++count);
		}
		return true;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_TEXT_FINDER_H
