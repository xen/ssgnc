#ifndef SSGNC_INVERTED_DB_H
#define SSGNC_INVERTED_DB_H

#include "db-reader.h"
#include "file-mapper.h"

namespace ssgnc {

class InvertedDb
{
public:
	InvertedDb() : n_(0), address_(NULL), positions_(NULL), size_(0) {}

	// Maps a database.
	void MapDb(int n, const FileMapper &file)
	{
		MapDb(n, file.address(), file.size());
	}
	void MapDb(int n, const void *address, std::size_t size)
	{
		n_ = n;
		address_ = static_cast<const char *>(address);

		size_ = static_cast<int>(*reinterpret_cast<const std::size_t *>(
			address_ + size - sizeof(size_)));

		positions_ = static_cast<const std::size_t *>(address);
		positions_ += (size - sizeof(size_)) / sizeof(std::size_t);
		positions_ -= size_;
	}

	// Finds n-grams that contain a given key.
	// Returns false if there are no n-grams available.
	bool Find(int key_id, DbReader *reader) const
	{
		if (key_id < 0 || key_id >= size_ - 1)
			return false;
		reader->SetRange(n_, address_ + positions_[key_id],
			positions_[key_id + 1] - positions_[key_id]);
		return true;
	}

	// Returns the number of keys.
	int size() const { return size_; }

private:
	int n_;
	const char *address_;
	const std::size_t *positions_;
	int size_;

	// Disallows copies.
	InvertedDb(const InvertedDb &);
	InvertedDb &operator=(const InvertedDb &);
};

}  // namespace ssgnc

#endif  // SSGNC_INVERTED_DB_H
