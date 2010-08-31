#include <ssgnc/line-reader.h>

#include <ssgnc/exception.h>

#include <cstring>

namespace ssgnc {

LineReader::LineReader(std::istream *stream, std::size_t buf_size)
	: stream_(stream), buf_size_(0), buf_(), pos_(0)
{
	if (buf_size == 0)
		buf_size = DEFAULT_BUF_SIZE;

	buf_size_ = buf_size;
	try
	{
		buf_.reserve(buf_size_);
	}
	catch (...)
	{
		SSGNC_THROW("failed to initialize LineReader: "
			"std::vector::reserve() failed");
	}
}

bool LineReader::Read(std::string *line)
{
	const char *start;
	std::size_t length;
	if (!Read(&start, &length))
		return false;

	try
	{
		line->assign(start, length);
	}
	catch (...)
	{
		SSGNC_THROW("failed to read line: std::string::assign() failed");
	}

	return true;
}

bool LineReader::Read(const char **line, std::size_t *length_ptr)
{
	std::size_t start = pos_;
	while (pos_ < buf_.size() && buf_[pos_] != '\n')
		++pos_;
	if (pos_ == buf_.size())
	{
		if (!Fill(&start))
			return false;

		while (pos_ < buf_.size() && buf_[pos_] != '\n')
			++pos_;

		if (pos_ == buf_.size())
		{
			if (buf_.size() == buf_size_)
				SSGNC_THROW("failed to read line: too long line");
			else
				SSGNC_THROW("failed to read line: missing end of line");
		}
	}

	*line = &buf_[start];
	if (length_ptr != NULL)
		*length_ptr = pos_ - start;
	buf_[pos_] = '\0';
	++pos_;
	return true;
}

bool LineReader::Fill(std::size_t *start)
{
	if (!*stream_)
		return false;

	std::size_t avail = (buf_.size() - *start);
	if (!buf_.empty() && avail > 0)
		std::memmove(&buf_[0], &buf_[*start], avail);

	pos_ -= *start;
	*start = 0;

	buf_.resize(buf_size_);
	stream_->read(&buf_[avail], buf_size_ - avail);
	buf_.resize(avail + stream_->gcount());
	return true;
}

}  // namespace ssgnc
