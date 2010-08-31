#ifndef SSGNC_FD_STREAMBUF_H
#define SSGNC_FD_STREAMBUF_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <streambuf>
#include <vector>

namespace ssgnc {

class FdStreambuf : public std::streambuf
{
public:
	enum { DEFAULT_BUF_SIZE = 1 << 10 };

	FdStreambuf(int fd, std::size_t buf_size = DEFAULT_BUF_SIZE)
		: fd_(fd), buf_(buf_size)
	{
		setg(&buf_[0], &buf_[buf_.size()], &buf_[buf_.size()]);
		setp(&buf_[0], &buf_[buf_.size()]);
	}
	~FdStreambuf() { sync(); }

	// Seeks a file pointer.
	virtual std::streampos seekpos(
		std::streampos sp, std::ios_base::openmode which)
	{
		::off_t pos = ::lseek(fd_, static_cast< ::off_t>(sp), 0);
		return static_cast<pos_type>(pos);
	}

	// Processes an underflow.
	virtual int underflow()
	{
		::ssize_t count = ::read(fd_, &buf_[0], buf_.size());
		if (count <= 0)
			return EOF;
		setg(&buf_[0], &buf_[0], &buf_[count]);
		return static_cast<unsigned char>(buf_[0]);
	}

	// Processes an overflow.
	virtual int overflow(int c = EOF)
	{
		if (c == EOF)
			return c;
		if (sync() == -1)
			return EOF;
		setp(&buf_[0], &buf_[buf_.size()]);
		return sputc(static_cast<char>(c));
	}

	// Flushes an internal buffer.
	virtual int sync()
	{
		::size_t total_count = 0;
		::size_t size = static_cast< ::size_t>(pptr() - pbase());
		while (total_count < size)
		{
			::ssize_t count = ::write(
				fd_, &buf_[total_count], size - total_count);
			if (count <= 0)
				return -1;
			total_count += count;
		}
		return 0;
	}

private:
	int fd_;
	std::vector<char> buf_;

	// Disallows copies.
	FdStreambuf(const FdStreambuf &);
	FdStreambuf &operator=(const FdStreambuf &);
};

}  // namespace ssgnc

#endif  // SSGNC_FD_STREAMBUF_H
