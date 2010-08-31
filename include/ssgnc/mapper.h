#ifndef SSGNC_MAPPER_H
#define SSGNC_MAPPER_H

#include "common.h"

namespace ssgnc {

class Mapper
{
public:
	Mapper() : ptr_(NULL), size_(0), total_(0) {}
	Mapper(const void *ptr, UInt32 size)
		: ptr_(ptr), size_(size), total_(0) {}
	~Mapper();

	bool open(const void *ptr, UInt32 size) SSGNC_WARN_UNUSED_RESULT;
	bool close();

	template <typename T>
	bool map(const T **ptr) SSGNC_WARN_UNUSED_RESULT;
	template <typename T>
	bool map(const T **ptr, UInt32 num_objs) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return ptr_ != NULL; }

	UInt32 tell() const { return total_; }

	enum { MAX_SIZE = 0x7FFFFFFF };

private:
	const void *ptr_;
	UInt32 size_;
	UInt32 total_;

	// Disallows copies.
	Mapper(const Mapper &);
	Mapper &operator=(const Mapper &);
};

template <typename T>
bool Mapper::map(const T **ptr)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (ptr == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (total_ + sizeof(T) > size_)
	{
		SSGNC_ERROR << "No more input: " << total_
			<< " + " << sizeof(T) << std::endl;
		return false;
	}

	*ptr = static_cast<const T *>(ptr_);

	ptr_ = static_cast<const T *>(ptr_) + 1;
	total_ += static_cast<UInt32>(sizeof(T));
	return true;
}

template <typename T>
bool Mapper::map(const T **ptr, UInt32 num_objs)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (ptr == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	UInt64 num_bytes = static_cast<UInt64>(sizeof(T)) * num_objs;
	if (total_ + num_bytes > size_)
	{
		SSGNC_ERROR << "No more input: " << total_
			<< " + " << sizeof(T) << " * " << num_objs << std::endl;
		return false;
	}

	*ptr = static_cast<const T *>(ptr_);

	ptr_ = static_cast<const T *>(ptr_) + num_objs;
	total_ += static_cast<UInt32>(num_bytes);
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_MAPPER_H
