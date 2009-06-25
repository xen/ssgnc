#ifndef SSGNC_FILE_MAPPER_H
#define SSGNC_FILE_MAPPER_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstddef>

namespace ssgnc
{

class FileMapper
{
public:
	FileMapper() : fd_(-1), address_(MAP_FAILED), size_(0) {}
	~FileMapper() { Unmap(); }

	// Unmaps a file and then maps a file.
	bool Map(const char *file_name)
	{
		Unmap();

		// Gets the file size.
		struct ::stat st;
		if (::stat(file_name, &st))
			return false;

		// Opens a file.
		fd_ = ::open(file_name, O_RDONLY);
		if (fd_ == -1)
			return false;
		size_ = static_cast<std::size_t>(st.st_size);

		// Maps a file.
		address_ = ::mmap(NULL, size_, PROT_READ, MAP_SHARED, fd_, 0);
		if (address_ == MAP_FAILED)
		{
			Unmap();
			return false;
		}
		return true;
	}

	// Unmaps a file.
	void Unmap()
	{
		if (address_ != MAP_FAILED)
		{
			::munmap(address_, static_cast<std::size_t>(size_));
			address_ = MAP_FAILED;
		}

		if (fd_ != -1)
		{
			::close(fd_);
			fd_ = -1;
		}

		size_ = 0;
	}

	template <typename ValueType>
	const ValueType *Pointer() const
	{
		return static_cast<const ValueType *>(address_);
	}
	const void *Address() const { return address_; }
	std::size_t Size() const { return size_; }

private:
	int fd_;
	void *address_;
	std::size_t size_;

	// Disallows copies.
	FileMapper(const FileMapper &);
	FileMapper &operator=(const FileMapper &);
};

}  // namespace ssgnc

#endif  // SSGNC_FILE_MAPPER_H
