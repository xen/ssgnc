#include <ssgnc/memory-pool.h>

#include <ssgnc/exception.h>

#include <algorithm>
#include <cstring>

namespace ssgnc {

MemoryPool::MemoryPool(std::size_t chunk_size) :
	chunk_size_((chunk_size != 0) ? chunk_size : DEFAULT_CHUNK_SIZE),
	chunks_(), ptr_(NULL), avail_(0), total_(0) {}

const char *MemoryPool::AppendString(const char *str)
{
	return static_cast<const char *>(AppendBytes(str, std::strlen(str) + 1));
}

const char *MemoryPool::AppendString(const char *str, std::size_t length)
{
	if (length >= avail_)
	{
		try
		{
			chunks_.push_back(NULL);
		}
		catch (...)
		{
			SSGNC_THROW("failed to copy bytes: "
				"std::vector::push_back() failed");
		}

		std::size_t chunk_size = std::max(length + 1, chunk_size_);
		try
		{
			chunks_.back() = new char[chunk_size];
		}
		catch (...)
		{
			chunks_.resize(chunks_.size() - 1);
			SSGNC_THROW("failed to copy bytes: std::vector::reserve() failed");
		}

		ptr_ = chunks_.back();
		avail_ = chunk_size;
		total_ += chunk_size;
	}

	char *ptr = ptr_;
	ptr_ += length + 1;
	avail_ -= length + 1;

	std::memcpy(ptr, str, length);
	ptr[length] = '\0';
	return ptr;
}

const void *MemoryPool::AppendBytes(const void *bytes, std::size_t size)
{
	if (size > avail_)
	{
		try
		{
			chunks_.push_back(NULL);
		}
		catch (...)
		{
			SSGNC_THROW("failed to copy bytes: "
				"std::vector::push_back() failed");
		}

		std::size_t chunk_size = std::max(size, chunk_size_);
		try
		{
			chunks_.back() = new char[chunk_size];
		}
		catch (...)
		{
			chunks_.resize(chunks_.size() - 1);
			SSGNC_THROW("failed to copy bytes: std::bad_alloc");
		}

		ptr_ = chunks_.back();
		avail_ = chunk_size;
		total_ += chunk_size;
	}

	char *ptr = ptr_;
	ptr_ += size;
	avail_ -= size;

	std::memcpy(ptr, bytes, size);
	return ptr;
}

void MemoryPool::Clear()
{
	for (std::size_t i = 0; i < chunks_.size(); ++i)
	{
		if (chunks_[i] != NULL)
			delete[] chunks_[i];
		chunks_[i] = NULL;
	}
	std::vector<char *>().swap(chunks_);

	chunk_size_ = chunk_size_;
	ptr_ = NULL;
	avail_ = 0;
	total_ = 0;
}

void MemoryPool::Swap(MemoryPool *target)
{
	std::swap(chunk_size_, target->chunk_size_);
	chunks_.swap(target->chunks_);
	std::swap(ptr_, target->ptr_);
	std::swap(avail_, target->avail_);
	std::swap(total_, target->total_);
}

}  // namespace ssgnc
