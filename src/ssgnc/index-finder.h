#ifndef SSGNC_INDEX_FINDER_H
#define SSGNC_INDEX_FINDER_H

#include "file-mapper.h"

#include <utility>

#include <iostream>

namespace ssgnc {

class IndexFinder
{
public:
	typedef std::pair<const char *, std::size_t> RangeType;

	explicit IndexFinder(const FileMapper &file)
		: address_(NULL), positions_(NULL), max_key_id_(0)
	{
		Init(file.Address(), file.Size());
	}
	IndexFinder(const void *address, std::size_t size)
		: address_(NULL), positions_(NULL), max_key_id_(0)
	{
		Init(address, size);
	}

	// Finds a range of addresses.
	RangeType Find(int key_id) const
	{
		RangeType range(address_, 0);
		if (key_id < 0 || key_id > max_key_id_)
			return range;

		range.first = address_ + positions_[key_id];
		range.second = positions_[key_id + 1] - positions_[key_id];
		return range;
	}

private:
	const char *address_;
	const std::size_t *positions_;
	int max_key_id_;

	// Disallows copies.
	IndexFinder(const IndexFinder &);
	IndexFinder &operator=(const IndexFinder &);

private:
	// Initializes an object.
	void Init(const void *address, std::size_t size)
	{
		address_ = static_cast<const char *>(address);

		max_key_id_ = static_cast<int>(*reinterpret_cast<const long *>(
			address_ + size - sizeof(max_key_id_)));

		positions_ = static_cast<const std::size_t *>(address);
		positions_ += (size - sizeof(max_key_id_)) / sizeof(std::size_t);
		positions_ -= max_key_id_ + 2;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_INDEX_FINDER_H
