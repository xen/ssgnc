#ifndef GOOGLE_NGRAM_FILE_MAPPER_H_
#define GOOGLE_NGRAM_FILE_MAPPER_H_

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>

namespace ngram
{

class file_mapper
{
public:
	// Creates a file mapping.
	explicit file_mapper(const std::string &file_name)
		: fd_(-1), address_(0), size_(0)
	{
		// Gets the sizes of files.
		struct stat st;
		if (stat(file_name.c_str(), &st))
			return;
		size_ = st.st_size;

		fd_ = open(file_name.c_str(), O_RDONLY);
		if (fd_ == -1)
			return;

		address_ = mmap(0, size_, PROT_READ, MAP_SHARED, fd_, 0);
	}
	// Unmaps a file.
	~file_mapper()
	{
		if (address_)
			munmap(address_, size_);

		if (fd_ != -1)
			close(fd_);
	}

	// Returns if an object is valid or not.
	bool is_open() const { return address_ != 0; }
	// Returns a pointer to a mapped file.
	template <typename Type>
	const Type *pointer() const { return static_cast<const Type *>(address_); }
	// Size of a mapped file.
	long long size() const { return size_; }

private:
	// File descriptor.
	int fd_;
	// Address of a mapped file.
	void *address_;
	// Size of a mapped file.
	long long size_;

	// Copies are not allowed.
	file_mapper(const file_mapper &);
	file_mapper &operator=(const file_mapper &);
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_FILE_MAPPER_H_
