#include "ssgnc.h"

namespace ssgnc {

bool NgramReader::open(Int32 num_tokens, const String &format,
	Int32 file_id, UInt32 offset, Int16 min_freq)
{
	close();

	num_tokens_ = num_tokens;

	if (min_freq <= 0)
	{
		SSGNC_ERROR << "Invalid minimum freq: " << min_freq << std::endl;
		return false;
	}
	min_freq_ = min_freq;

	if (!file_path_.open(format))
	{
		SSGNC_ERROR << "ssgnc::FilePath::open() failed: "
			<< format << std::endl;
		return false;
	}

	if (!file_path_.seek(file_id))
	{
		SSGNC_ERROR << "ssgnc::FilePath::seek() failed: "
			<< file_id << std::endl;
		return false;
	}

	if (!openNextFile())
	{
		SSGNC_ERROR << "ssgnc::NgramReader::openNextFile() failed"
			<< std::endl;
		return false;
	}

	if (offset != 0 && !file_.seekg(offset))
	{
		SSGNC_ERROR << "ssgnc::ifstream::seek() failed: "
			<< offset << std::endl;
		return false;
	}

	if (!readFreq())
	{
		SSGNC_ERROR << "ssgnc::NgramReader::readFreq() failed: " << std::endl;
		return false;
	}

	return true;
}

void NgramReader::close()
{
	if (file_path_.is_open())
		file_path_.close();
	if (file_.is_open())
		file_.close();
	if (byte_reader_.is_open())
		byte_reader_.close();
	min_freq_ = 1;
	freq_ = 0;
	total_ = 0;
}

bool NgramReader::read(Int16 *freq, std::vector<Int32> *tokens)
{
	*freq = freq_;
	if (freq_ < min_freq_)
		return false;

	if (!readTokens(tokens))
	{
		SSGNC_ERROR << "ssgnc::NgramReader::readTokens() failed" << std::endl;
		return false;
	}

	if (!readFreq())
	{
		if (freq_ < 0)
		{
			SSGNC_ERROR << "ssgnc::NgramReader::readFreq() failed"
				<< std::endl;
			return false;
		}

		if (!openNextFile())
		{
			SSGNC_ERROR << "ssgnc::NgramReader::openNextFile() failed"
				<< std::endl;
			return false;
		}

		if (!readFreq())
		{
			freq_ = -1;
			SSGNC_ERROR << "ssgnc::NgramReader::readFreq() failed"
				<< std::endl;
			return false;
		}
	}

	return true;
}

bool NgramReader::openNextFile()
{
	StringBuilder path;
	if (!file_path_.read(&path))
	{
		freq_ = -1;
		SSGNC_ERROR << "ssgnc::FilePath::read() failed" << std::endl;
		return false;
	}

	if (file_.is_open())
		file_.close();
	file_.open(path.ptr(), std::ios::binary);
	if (!file_)
	{
		freq_ = -1;
		SSGNC_ERROR << "std::ifstream::open() failed: "
			<< path.str() << std::endl;
		return false;
	}

	total_ += byte_reader_.tell();
	if (byte_reader_.is_open())
		byte_reader_.close();
	if (!byte_reader_.open(&file_, BYTE_READER_BUF_SIZE))
	{
		freq_ = -1;
		SSGNC_ERROR << "ssgnc::ByteReader::open() failed" << std::endl;
		return false;
	}

	return true;
}

bool NgramReader::readFreq()
{
	Int32 byte;
	Int32 freq = 0;
	while (byte_reader_.read(&byte) && byte >= 0)
	{
		freq = (freq << 7) + (byte & 0x7F);
		if (freq > FreqHandler::MAX_ENCODED_FREQ)
		{
			freq_ = -1;
			SSGNC_ERROR << "Out of range freq: " << freq << std::endl;
			return false;
		}

		if (byte < 0x80)
		{
			freq_ = static_cast<Int16>(freq);
			return true;
		}
	}
	if (freq > 0)
	{
		freq_ = -1;
		SSGNC_ERROR << "Unexpected EOF: " << freq_ << std::endl;
	}
	return false;
}

bool NgramReader::readTokens(std::vector<Int32> *tokens)
{
	tokens->clear();

	Int32 token = 0;
	Int32 token_count = 0;

	Int32 byte;
	while (byte_reader_.read(&byte) && byte >= 0)
	{
		token = (token << 7) + (byte & 0x7F);
		if (byte < 0x80)
		{
			try
			{
				tokens->push_back(token);
			}
			catch (...)
			{
				freq_ = -1;
				SSGNC_ERROR << "std::vector<ssgnc::Int32>::push_back() "
					"failed: " << tokens->size() << std::endl;
				return false;
			}

			if (++token_count == num_tokens_)
				return true;
			token = 0;
		}
	}

	freq_ = -1;
	SSGNC_ERROR << "Unexpected EOF" << std::endl;
	return false;
}

}  // namespace ssgnc
