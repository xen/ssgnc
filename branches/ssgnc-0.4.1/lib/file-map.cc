#include "ssgnc/file-map.h"

#include <sys/types.h>
#include <sys/stat.h>

#if defined _WIN32 || defined _WIN64

#include <Windows.h>

#else  // defined _WIN32 || defined _WIN64

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#endif

namespace ssgnc {

class FileMap::Impl
{
public:
	Impl();
	~Impl() { close(); }

	bool open(const Int8 *path);
	void close();

	const void *ptr() const { return ptr_; }
	std::size_t size() const { return size_; }

private:
#if defined _WIN32 || defined _WIN64
	HANDLE *file_handle_;
	HANDLE *map_handle_;
#else
	int fd_;
#endif
	void *ptr_;
	std::size_t size_;

	// Disallows copies.
	Impl(const Impl &);
	Impl &operator=(const Impl &);
};

#if defined _WIN32 || defined _WIN64

FileMap::Impl::Impl() : file_handle_(NULL), map_handle_(NULL),
	ptr_(NULL), size_(0) {}

bool FileMap::Impl::open(const Int8 *path)
{
	struct __stat64 st;
	if (::_stat64(path, &st) != 0)
	{
		SSGNC_ERROR << "::_stat64() failed: " << path << std::endl;
		return false;
	}
	else if (st.st_size > FILE_MAX_SIZE)
	{
		SSGNC_ERROR << "Too large file: " << st.st_size << std::endl;
		return false;
	}
	size_ = static_cast<std::size_t>(st.st_size);

	file_handle_ = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle_ == NULL)
	{
		SSGNC_ERROR << "::CreateFile(): " << path << std::endl;
		return false;
	}

	map_handle_ = ::CreateFileMapping(file_handle_, NULL,
		PAGE_READONLY, 0, 0, NULL);
	if (map_handle_ == NULL)
	{
		SSGNC_ERROR << "::CreateFileMapping() failed" << std::endl;
		return false;
	}

	ptr_ = ::MapViewOfFile(map_handle_, FILE_MAP_READ, 0, 0, 0);
	if (ptr_ == NULL)
	{
		SSGNC_ERROR << "::MapViewOfFile() failed" << std::endl;
		return false;
	}

	return true;
}

void FileMap::Impl::close()
{
	if (ptr_ != NULL)
	{
		::UnmapViewOfFile(ptr_);
		ptr_ = NULL;
	}

	if (map_handle_ != NULL)
	{
		::CloseHandle(map_handle_);
		map_handle_ = NULL;
	}

	if (file_handle_ != NULL)
	{
		::CloseHandle(file_handle_);
		file_handle_ = NULL;
	}

	size_ = 0;
}

#else  // defined _WIN32 || defined _WIN64

FileMap::Impl::Impl() : fd_(-1), ptr_(MAP_FAILED), size_(0) {}

bool FileMap::Impl::open(const Int8 *path)
{
	struct stat st;
	if (::stat(path, &st) != 0)
	{
		SSGNC_ERROR << "::stat() failed: " << path << std::endl;
		return false;
	}
	else if (st.st_size > MAX_FILE_SIZE)
	{
		SSGNC_ERROR << "Too large file: " << st.st_size << std::endl;
		return false;
	}
	size_ = static_cast<std::size_t>(st.st_size);

	fd_ = ::open(path, O_RDONLY);
	if (fd_ == -1)
	{
		SSGNC_ERROR << "::open() failed: " << path << std::endl;
		return false;
	}

	ptr_ = ::mmap(NULL, size_, PROT_READ, MAP_SHARED, fd_, 0);
	if (ptr_ == MAP_FAILED)
	{
		SSGNC_ERROR << "::mmap() failed: "
			<< size_ << ", " << fd_ << std::endl;
		return false;
	}

	return true;
}

void FileMap::Impl::close()
{
	if (ptr_ != MAP_FAILED)
	{
		::munmap(ptr_, size_);
		ptr_ = MAP_FAILED;
	}

	if (fd_ != -1)
	{
		::close(fd_);
		fd_ = -1;
	}

	size_ = 0;
}

}  // namespace ssgnc

#endif  // defined _WIN32 || defined _WIN64

namespace ssgnc {

FileMap::~FileMap()
{
	if (is_open())
		close();
}

bool FileMap::open(const Int8 *path)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Impl *new_impl;
	try
	{
		new_impl = new Impl;
	}
	catch (...)
	{
		SSGNC_ERROR << "new ssgnc::FileMap::Impl failed" << std::endl;
		return false;
	}

	if (!new_impl->open(path))
	{
		SSGNC_ERROR << "ssgnc::FileMap::Impl::open() failed: "
			<< path << std::endl;
		delete new_impl;
		return false;
	}

	impl_ = new_impl;
	ptr_ = impl_->ptr();
	size_ = static_cast<UInt32>(impl_->size());
	return true;
}

bool FileMap::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	delete impl_;

	impl_ = NULL;
	ptr_ = NULL;
	size_ = 0;
	return true;
}

}  // namespace ssgnc
