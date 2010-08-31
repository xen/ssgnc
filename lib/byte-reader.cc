#include "ssgnc/byte-reader.h"

namespace ssgnc {

ByteReader::~ByteReader()
{
	if (is_open())
		close();
}

bool ByteReader::open(std::istream *stream, UInt32 buf_size)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (stream == NULL)
	{
		SSGNC_ERROR << "Null stream" << std::endl;
		return false;
	}

	if (buf_size == 0)
		buf_size = DEFAULT_BUF_SIZE;

	try
	{
		buf_.reserve(buf_size);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::Int8>::reserve() failed: "
			<< buf_size << std::endl;
		return false;
	}

	stream_ = stream;
	buf_size_ = buf_size;
	return true;
}

bool ByteReader::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	stream_ = NULL;
	std::vector<Int8>().swap(buf_);
	buf_size_ = 0;
	pos_ = 0;
	total_ = 0;
	return true;
}

bool ByteReader::fill()
{
	if (!*stream_)
		return false;

	buf_.resize(buf_size_);
	stream_->read(&buf_[0], buf_size_);
	buf_.resize(static_cast<std::size_t>(stream_->gcount()));

	pos_ = 0;
	return buf_.size() > 0;
}

}  // namespace ssgnc
