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

	bool open(const Int8 *path, Mode mode);
	void close();

#if defined _WIN32 || defined _WIN64
	const void *ptr() const { return (ptr_ != NULL) ? ptr_ : buf_; }
#else  // defined _WIN32 || defined _WIN64
	const void *ptr() const { return (ptr_ != MAP_FAILED) ? ptr_ : buf_; }
#endif  // defined _WIN32 || defined _WIN64

	std::size_t size() const { return size_; }

private:
#if defined _WIN32 || defined _WIN64
	HANDLE *file_handle_;
	HANDLE *map_handle_;
#else  // defined _WIN32 || defined _WIN64
	int fd_;
#endif  // defined _WIN32 || defined _WIN64

	void *ptr_;
	char *buf_;
	std::size_t size_;

	bool getFileSize(const Int8 *path, std::size_t *file_size);

	bool mmap(const Int8 *path, std::size_t file_size);
	bool read(const Int8 *path, std::size_t file_size);

	// Disallows copies.
	Impl(const Impl &);
	Impl &operator=(const Impl &);
};

#if defined _WIN32 || defined _WIN64

FileMap::Impl::Impl() : file_handle_(NULL), map_handle_(NULL),
	ptr_(NULL), buf_(NULL), size_(0) {}

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

	if (buf_ != NULL)
	{
		delete [] buf_;
		buf_ = NULL;
	}

	size_ = 0;
}

bool FileMap::Impl::getFileSize(const Int8 *path, std::size_t *file_size)
{
	struct __stat64 st;
	if (::_stat64(path, &st) != 0)
	{
		SSGNC_ERROR << "::_stat64() failed: " << path << std::endl;
		return false;
	}
	else if (st.st_size > MAX_FILE_SIZE)
	{
		SSGNC_ERROR << "Too large file: " << st.st_size << std::endl;
		return false;
	}

	*file_size = static_cast<std::size_t>(st.st_size);
	return true;
}

bool FileMap::Impl::mmap(const Int8 *path, std::size_t file_size)
{
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

	size_ = file_size;
	return true;
}

#else  // defined _WIN32 || defined _WIN64

FileMap::Impl::Impl() : fd_(-1), ptr_(MAP_FAILED), buf_(NULL), size_(0) {}

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

	if (buf_ != NULL)
	{
		delete [] buf_;
		buf_ = NULL;
	}

	size_ = 0;
}

bool FileMap::Impl::getFileSize(const Int8 *path, std::size_t *file_size)
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

	*file_size = static_cast<std::size_t>(st.st_size);
	return true;
}

bool FileMap::Impl::mmap(const Int8 *path, std::size_t file_size)
{
	fd_ = ::open(path, O_RDONLY);
	if (fd_ == -1)
	{
		SSGNC_ERROR << "::open() failed: " << path << std::endl;
		return false;
	}

	ptr_ = ::mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd_, 0);
	if (ptr_ == MAP_FAILED)
	{
		SSGNC_ERROR << "::mmap() failed: " << file_size << std::endl;
		return false;
	}

	size_ = file_size;
	return true;
}

#endif  // defined _WIN32 || defined _WIN64

bool FileMap::Impl::open(const Int8 *path, Mode mode)
{
	if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	std::size_t file_size;
	if (!getFileSize(path, &file_size))
	{
		SSGNC_ERROR << "ssgnc::FileMap::Impl::getFileSize() failed: "
			<< path << std::endl;
		return false;
	}

	switch (mode)
	{
	case MMAP_FILE:
		if (!mmap(path, file_size))
		{
			SSGNC_ERROR << "ssgnc::FileMap::Impl::mmap() failed: "
				<< path << std::endl;
			return false;
		}
		break;
	case READ_FILE:
		if (!read(path, file_size))
		{
			SSGNC_ERROR << "ssgnc::FileMap::Impl::read() failed: "
				<< path << std::endl;
			return false;
		}
		break;
	default:
		SSGNC_ERROR << "Unknown mode: " << mode << std::endl;
		return false;
	}

	return true;
}

bool FileMap::Impl::read(const Int8 *path, std::size_t file_size)
{
	if (buf_ != NULL)
	{
		SSGNC_ERROR << "Not null pointer" << std::endl;
		return false;
	}

	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		SSGNC_ERROR << "std::ifstream::open() failed: " << path << std::endl;
		return false;
	}

	try
	{
		buf_ = new Int8[file_size];
	}
	catch (...)
	{
		SSGNC_ERROR << "new Int8[] failed: " << file_size << std::endl;
		return false;
	}

	if (!file.read(buf_, file_size))
	{
		SSGNC_ERROR << "std::ifstream::read() failed: "
			<< file_size << std::endl;
		return false;
	}

	size_ = file_size;
	return true;
}

FileMap::~FileMap()
{
	if (is_open())
		close();
}

bool FileMap::open(const Int8 *path, Mode mode)
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

	if (!new_impl->open(path, mode))
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
