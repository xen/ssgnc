#ifndef SSGNC_MEM_POOL_H
#define SSGNC_MEM_POOL_H

#include "string.h"

#include <vector>

namespace ssgnc {

class MemPool
{
public:
	MemPool();
	~MemPool() { clear(); }

	UInt32 num_objs() const { return num_objs_; }
	UInt32 total_length() const { return total_length_; }
	UInt32 total_size() const { return total_size_; }

	void clear();

	const Int8 *operator[](UInt32 index) const { return get(index); }
	const Int8 *get(UInt32 index) const;
	const String get(UInt32 index, UInt32 length) const
	{ return String(get(index), length); }

	UInt32 append(const String &obj);
	String clone(const String &obj)
	{ return String(get(append(obj)), obj.length()); }

	enum { DEFAULT_CHUNK_SIZE = 4096 };

private:
	std::vector<Int8 *> chunks_;
	Int8 *ptr_;
	UInt32 avail_;
	UInt32 num_objs_;
	UInt32 total_length_;
	UInt32 total_size_;

	// Disallows copies.
	MemPool(const MemPool &);
	MemPool &operator=(const MemPool &);
};

inline MemPool::MemPool() : chunks_(), ptr_(NULL), avail_(0),
	num_objs_(0), total_length_(0), total_size_(0) {}

inline void MemPool::clear()
{
	for (std::size_t i = 0; i < chunks_.size(); ++i)
	{
		if (chunks_[i] != NULL)
		{
			delete [] chunks_[i];
			chunks_[i] = NULL;
		}
	}
	chunks_.clear();
	ptr_ = NULL;
	avail_ = 0;
	num_objs_ = 0;
	total_length_ = 0;
	total_size_ = 0;
}

inline const Int8 *MemPool::get(UInt32 index) const
{
	return chunks_[index / DEFAULT_CHUNK_SIZE]
		+ (index % DEFAULT_CHUNK_SIZE);
}

inline UInt32 MemPool::append(const String &obj)
{
	if (obj.length() > avail_)
	{
		UInt32 new_chunk_size = (obj.length() <= DEFAULT_CHUNK_SIZE)
			? DEFAULT_CHUNK_SIZE : obj.length();

		chunks_.resize(chunks_.size() + 1);
		Int8 *new_chunk = new Int8[new_chunk_size];
		chunks_.back() = new_chunk;

		ptr_ = new_chunk;
		avail_ = new_chunk_size;
		total_size_ += new_chunk_size;
	}

	UInt32 index = DEFAULT_CHUNK_SIZE * (chunks_.size() - 1);
	if (avail_ < DEFAULT_CHUNK_SIZE)
		index += DEFAULT_CHUNK_SIZE - avail_;

	for (UInt32 i = 0; i < obj.length(); ++i)
		*ptr_++ = obj[i];
	avail_ -= obj.length();

	++num_objs_;
	total_length_ += obj.length();
	return index;
}

}  // namespace ssgnc

#endif  // SSGNC_MEM_POOL_H
