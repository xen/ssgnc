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

bool ByteReader::readFreq(Int16 *freq)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (freq == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int16 value = 0;
	Int8 byte;
	for (Int32 i = 0; i < MAX_FREQ_LENGTH; ++i)
	{
		if (!read(&byte))
		{
			if (value != 0)
			{
				stream_->setstate(std::ios::badbit);
				SSGNC_ERROR << "ssgnc::ByteReader::read() failed" << std::endl;
			}
			return false;
		}

		value = (value << 7) + (byte & 0x7F);
		if (static_cast<UInt8>(byte) < 0x80)
		{
			*freq = value;
			return true;
		}
	}
	stream_->setstate(std::ios::badbit);
	SSGNC_ERROR << "Too long freq sequence" << std::endl;
	return false;
}

bool ByteReader::readFreq(StringBuilder *buf, Int16 *freq)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (buf == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int16 value = 0;
	Int8 byte;
	for (Int32 i = 0; i < MAX_FREQ_LENGTH; ++i)
	{
		if (!read(&byte))
		{
			if (value != 0)
			{
				stream_->setstate(std::ios::badbit);
				SSGNC_ERROR << "ssgnc::ByteReader::read() failed" << std::endl;
			}
			return false;
		}
		else if (!buf->append(byte))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}

		value = (value << 7) + (byte & 0x7F);
		if (static_cast<UInt8>(byte) < 0x80)
		{
			if (freq != NULL)
				*freq = value;
			return true;
		}
	}
	stream_->setstate(std::ios::badbit);
	SSGNC_ERROR << "Too long freq sequence" << std::endl;
	return false;
}

bool ByteReader::readToken(Int32 *token)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (token == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int32 value = 0;
	Int8 byte;
	for (Int32 i = 0; i < MAX_TOKEN_LENGTH; ++i)
	{
		if (!read(&byte))
		{
			SSGNC_ERROR << "ssgnc::ByteReader::read() failed" << std::endl;
			return false;
		}

		value = (value << 7) + (byte & 0x7F);
		if (static_cast<UInt8>(byte) < 0x80)
		{
			*token = value;
			return true;
		}
	}
	stream_->setstate(std::ios::badbit);
	SSGNC_ERROR << "Too long token sequence" << std::endl;
	return false;
}

bool ByteReader::readToken(StringBuilder *buf, Int32 *token)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (buf == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int32 value = 0;
	Int8 byte;
	for (Int32 i = 0; i < MAX_TOKEN_LENGTH; ++i)
	{
		if (!read(&byte))
		{
			SSGNC_ERROR << "ssgnc::ByteReader::read() failed" << std::endl;
			return false;
		}
		else if (!buf->append(byte))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}

		value = (value << 7) + (byte & 0x7F);
		if (static_cast<UInt8>(byte) < 0x80)
		{
			if (token != NULL)
				*token = value;
			return true;
		}
	}
	stream_->setstate(std::ios::badbit);
	SSGNC_ERROR << "Too long token sequence" << std::endl;
	return false;
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
