#ifndef SSGNC_MEM_POOL_H
#define SSGNC_MEM_POOL_H

#include "string.h"

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

	bool get(UInt32 index, const Int8 **ptr) const SSGNC_WARN_UNUSED_RESULT;
	bool get(UInt32 index, UInt32 length, String *str) const
		SSGNC_WARN_UNUSED_RESULT;

	bool append(const String &obj, String *clone) SSGNC_WARN_UNUSED_RESULT
	{ return append(obj, clone, NULL); }
	bool append(const String &obj, UInt32 *index) SSGNC_WARN_UNUSED_RESULT
	{ return append(obj, NULL, index); }
	bool append(const String &obj, String *clone, UInt32 *index)
		SSGNC_WARN_UNUSED_RESULT;

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

inline bool MemPool::get(UInt32 index, const Int8 **ptr) const
{
	UInt32 chunk_id = index / DEFAULT_CHUNK_SIZE;
	if (chunk_id >= static_cast<UInt32>(chunks_.size()))
	{
		SSGNC_ERROR << "Too large index: " << index << std::endl;
		return false;
	}
	else if (ptr == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	*ptr = chunks_[chunk_id] + (index % DEFAULT_CHUNK_SIZE);
	return true;
}

inline bool MemPool::get(UInt32 index, UInt32 length, String *str) const
{
	UInt32 chunk_id = index / DEFAULT_CHUNK_SIZE;
	if (chunk_id >= static_cast<UInt32>(chunks_.size()))
	{
		SSGNC_ERROR << "Too large index: " << index << std::endl;
		return false;
	}
	else if (str == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	*str = String(chunks_[chunk_id] + (index % DEFAULT_CHUNK_SIZE), length);
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_MEM_POOL_H
