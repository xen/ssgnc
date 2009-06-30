#ifndef SSGNC_VOCAB_INDEX_H
#define SSGNC_VOCAB_INDEX_H

#include "file-mapper.h"

#include <string>

namespace ssgnc {

class VocabIndex
{
public:
	VocabIndex() : keys_(NULL), positions_(NULL), size_(0) {}

	// Maps an index.
	void MapIndex(const FileMapper &file)
	{
		MapIndex(file.address(), file.size());
	}
	void MapIndex(const void *address, std::size_t size)
	{
		keys_ = static_cast<const char *>(address);

		positions_ = static_cast<const std::size_t *>(address);
		positions_ += size / sizeof(std::size_t) - 1;

		size_ = static_cast<int>(*positions_);
		positions_ -= size_ + 1;
	}

	// Finds a key in an index
	bool Find(int id, const char **key) const
	{
		if (id < 0 || id >= size_)
			return false;
		*key = keys_ + positions_[id];
		return true;
	}
	bool Find(int id, const char **key, std::size_t *length) const
	{
		if (id < 0 || id >= size_)
			return false;
		*key = keys_ + positions_[id];
		*length = static_cast<std::size_t>(
			positions_[id + 1] - positions_[id]);
		return true;
	}
	bool Find(int id, std::string *key) const
	{
		const char *key_begin;
		std::size_t length;
		if (!Find(id, &key_begin, &length))
			return false;
		key->assign(key_begin, length);
		return true;
	}

	// Returns the number of keys.
	int size() const { return size_; }

private:
	const char *keys_;
	const std::size_t *positions_;
	int size_;

	// Disallows copies.
	VocabIndex(const VocabIndex &);
	VocabIndex &operator=(const VocabIndex &);
};

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_INDEX_H
