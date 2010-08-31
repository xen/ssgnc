#include "ssgnc/freq-handler.h"
#include "ssgnc/ngram-reader.h"

namespace ssgnc {

NgramReader::~NgramReader()
{
	if (is_open())
		close();
}

bool NgramReader::open(const String &index_dir, Int32 num_tokens,
	const NgramIndex::Entry &entry, Int16 min_freq)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (min_freq <= 0)
	{
		SSGNC_ERROR << "Invalid minimum freq: " << min_freq << std::endl;
		return false;
	}

	StringBuilder basename;
	if (!basename.appendf("%dgm-%%04d.db", num_tokens))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::appendf() failed" << std::endl;
		return false;
	}

	if (!file_path_.open(index_dir, basename.str()))
	{
		SSGNC_ERROR << "ssgnc::FilePath::open() failed: "
			<< index_dir << ", " << basename << std::endl;
		return false;
	}
	else if (!file_path_.seek(entry.file_id()))
	{
		SSGNC_ERROR << "ssgnc::FilePath::seek() failed: "
			<< entry.file_id() << std::endl;
		close();
		return false;
	}

	if (!openNextFile())
	{
		SSGNC_ERROR << "ssgnc::NgramReader::openNextFile() failed"
			<< std::endl;
		close();
		return false;
	}

	if (entry.offset() != 0 && !file_.seekg(entry.offset()))
	{
		SSGNC_ERROR << "ssgnc::ifstream::seek() failed: "
			<< entry.offset() << std::endl;
		close();
		return false;
	}

	if (!readFreq())
	{
		SSGNC_ERROR << "ssgnc::NgramReader::readFreq() failed: " << std::endl;
		close();
		return false;
	}

	num_tokens_ = num_tokens;
	min_freq_ = min_freq;

	return true;
}

bool NgramReader::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	num_tokens_ = 0;
	file_path_.close();
	if (file_.is_open())
		file_.close();
	if (byte_reader_.is_open())
		byte_reader_.close();
	min_freq_ = 1;
	freq_ = -1;
	total_ = 0;
	return true;
}

bool NgramReader::read(Int16 *freq, std::vector<Int32> *tokens)
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
	else if (tokens == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	*freq = freq_;
	tokens->clear();

	if (fail())
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

	if (byte_reader_.is_open())
	{
		total_ += byte_reader_.tell();
		byte_reader_.close();
	}
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
	if (!byte_reader_.readFreq(&freq_))
	{
		if (byte_reader_.bad())
		{
			freq_ = -1;
			SSGNC_ERROR << "ssgnc::ByteReader::readFreq() failed" << std::endl;
		}
		return false;
	}
	return true;
}

bool NgramReader::readTokens(std::vector<Int32> *tokens)
{
	try
	{
		tokens->resize(num_tokens_);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::Int32>::resize() failed: "
			<< sizeof(Int32) << " * " << num_tokens_ << std::endl;
		return false;
	}

	for (Int32 i = 0; i < num_tokens_; ++i)
	{
		if (!byte_reader_.readToken(&(*tokens)[i]))
		{
			freq_ = -1;
			SSGNC_ERROR << "ssgnc::ByteReader::readToken() failed"
				<< std::endl;
			return false;
		}
	}
	return true;
}

}  // namespace ssgnc
