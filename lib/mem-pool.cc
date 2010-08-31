#include "ssgnc.h"

namespace ssgnc {

MemPool::MemPool() : chunks_(), ptr_(NULL), avail_(0),
	num_objs_(0), total_length_(0), total_size_(0) {}

void MemPool::clear()
{
	for (std::size_t i = 0; i < chunks_.size(); ++i)
	{
		if (chunks_[i] != NULL)
			delete [] chunks_[i];
	}
	chunks_.clear();
	ptr_ = NULL;
	avail_ = 0;
	num_objs_ = 0;
	total_length_ = 0;
	total_size_ = 0;
}

bool MemPool::append(const String &obj, String *clone, UInt32 *index)
{
	if (clone == NULL && index == NULL)
	{
		SSGNC_ERROR << "Null pointers" << std::endl;
		return false;
	}

	if (obj.length() > avail_)
	{
		try
		{
			chunks_.reserve(chunks_.size() + 1);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<ssgnc::Int8 *>::reserve() failed: "
				<< sizeof(Int8 *) << " * "
				<< (chunks_.size() + 1) << std::endl;
			return false;
		}

		UInt32 new_chunk_size = (obj.length() <= DEFAULT_CHUNK_SIZE)
			? DEFAULT_CHUNK_SIZE : obj.length();

		try
		{
			chunks_.push_back(new Int8[new_chunk_size]);
		}
		catch (...)
		{
			SSGNC_ERROR << "new ssgnc::Int8[] failed: "
				<< new_chunk_size << std::endl;
			return false;
		}

		ptr_ = chunks_.back();
		avail_ = new_chunk_size;
		total_size_ += new_chunk_size;
	}

	if (clone != NULL)
		*clone = String(ptr_, obj.length());

	if (index != NULL)
	{
		*index = DEFAULT_CHUNK_SIZE * (chunks_.size() - 1);
		if (avail_ < DEFAULT_CHUNK_SIZE)
			*index += DEFAULT_CHUNK_SIZE - avail_;
	}

	for (UInt32 i = 0; i < obj.length(); ++i)
		ptr_[i] = obj[i];
	ptr_ += obj.length();
	avail_ -= obj.length();

	++num_objs_;
	total_length_ += obj.length();
	return true;
}

}  // namespace ssgnc
