#include <ssgnc/temp-file.h>

#include <ssgnc/exception.h>

#include <unistd.h>

#include <cstdlib>
#include <utility>

namespace ssgnc {

void TempFile::Open(const char *file_name, std::size_t buf_size)
{
	TempFile temp;

	if (file_name == NULL)
	{
		temp.file_ = std::tmpfile();
		if (temp.file_ == NULL)
		{
			SSGNC_THROW("failed to open temporary file: "
				"std::tmpfile() failed");
		}
	}
	else
	{
		temp.file_ = std::fopen(file_name, "wb+");
		if (temp.file_ == NULL)
			SSGNC_THROW("failed to open temporary file: std::fopen() failed");

		if (::unlink(file_name) != 0)
			SSGNC_THROW("failed to open temporary file: ::unlink() failed");
	}

	if (buf_size != 0)
	{
		if (std::setvbuf(temp.file_, NULL, _IOFBF,
			static_cast<std::size_t>(buf_size)) != 0)
		{
			SSGNC_THROW("failed to open temporary file: "
				"std::setvbuf() failed");
		}
	}

	Swap(&temp);
}

void TempFile::Close()
{
	if (is_open())
	{
		std::fclose(file_);
		file_ = NULL;
	}
}

void TempFile::Read(void *buf, std::size_t size)
{
	if (size == 0)
		return;

	if (std::fread(buf, static_cast<std::size_t>(size), 1, file_) != 1)
		SSGNC_THROW("failed to read data: std::fread() failed");
}

void TempFile::Write(const void *buf, std::size_t size)
{
	if (size == 0)
		return;

	if ( std::fwrite(buf, static_cast<std::size_t>(size), 1, file_) != 1)
		SSGNC_THROW("failed to write data: std::fwrite() failed");
}

void TempFile::Flush()
{
	if (std::fflush(file_) != 0)
		SSGNC_THROW("failed to flush file: std::fflush() failed");
}

std::size_t TempFile::Tell()
{
	long result = std::ftell(file_);
	if (result == -1)
		SSGNC_THROW("failed to get file position: std::ftell() failed");
	return static_cast<std::size_t>(result);
}

void TempFile::Seek(long offset, int whence)
{
	if (std::fseek(file_, offset, whence) != 0)
		SSGNC_THROW("failed to set file position: std::fseek() failed");
}

void TempFile::Swap(TempFile *target)
{
	std::swap(file_, target->file_);
}

}  // namespace ssgnc
