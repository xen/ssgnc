#ifndef SSGNC_VOCAB_DIC_H
#define SSGNC_VOCAB_DIC_H

#include "file-mapper.h"

#include <iostream>

#include <dawgdic/dictionary.h>

namespace ssgnc {

class VocabDic
{
public:
	VocabDic() : size_(0), dic_() {}

	// Reads a dictionary.
	bool ReadDic(std::istream *input)
	{
		int lexicon_size;
		if (!input->read(reinterpret_cast<char *>(&lexicon_size),
			sizeof(lexicon_size)) || lexicon_size <= 0)
			return false;

		if (!dic_.Read(input))
			return false;

		size_ = lexicon_size;
		return true;
	}

	// Maps a dictionary.
	void MapDic(const FileMapper &file)
	{
		MapDic(file.address(), file.size());
	}
	void MapDic(const void *address, std::size_t)
	{
		size_ = *static_cast<const int *>(address);
		dic_.Map(static_cast<const int *>(address) + 1);
	}

	// Finds a key in a dictionary.
	bool Find(const char *key, int *key_id) const
	{
		return dic_.Find(key, key_id);
	}
	bool Find(const char *key, std::size_t length, int *key_id) const
	{
		return dic_.Find(key, length, key_id);
	}

	// Returns the number of keys.
	int size() const { return size_; }

private:
	int size_;
	dawgdic::Dictionary dic_;

	// Disallows copies.
	VocabDic(const VocabDic &);
	VocabDic &operator=(const VocabDic &);
};

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_DIC_H
