#include "tools-common.h"

#if defined _WIN32 || defined _WIN64
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#endif  // defined _WIN32 || defined _WIN64

namespace ssgnc {
namespace tools {

void initIO()
{
	std::ios::sync_with_stdio(false);

#if defined _WIN32 || defined _WIN64
	::_setmode(::_fileno(::stdin), _O_BINARY);
	::_setmode(::_fileno(::stdout), _O_BINARY);
	::_setmode(::_fileno(::stderr), _O_BINARY);
#endif  // defined _WIN32 || defined _WIN64
}

bool parseInt64(const Int8 *str, Int64 *value)
{
	if (str == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (value == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	char *end_of_value;
	*value = std::strtoll(str, &end_of_value, 10);
	if (*end_of_value != '\0')
	{
		SSGNC_ERROR << "std::strtoll() failed: " << str << std::endl;
		return false;
	}
	return true;
}

bool parseNumTokens(const Int8 *str, Int32 *num_tokens)
{
	if (str == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (num_tokens == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int64 value;
	if (!parseInt64(str, &value))
	{
		SSGNC_ERROR << "ssgnc::tools::parseInt64() failed: "
			<< str << std::endl;
		return false;
	}
	else if (value < MIN_NUM_TOKENS || value > MAX_NUM_TOKENS)
	{
		SSGNC_ERROR << "Out of range: " << value << std::endl;
		return false;
	}

	*num_tokens = static_cast<Int32>(value);
	return true;
}

bool parseMemLimit(const Int8 *str, UInt64 *mem_limit)
{
	if (str == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (mem_limit == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int64 value;
	if (!parseInt64(str, &value))
	{
		SSGNC_ERROR << "ssgnc::tools::parseInt64() failed: "
			<< str << std::endl;
		return false;
	}
	else if (value < MIN_MEM_LIMIT || value > MAX_MEM_LIMIT)
	{
		SSGNC_ERROR << "Out of range: " << value << std::endl;
		return false;
	}

	*mem_limit = value << 32;
	return true;
}

bool readLine(std::istream *stream, std::string *line)
{
	if (stream == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (line == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	try
	{
		if (!std::getline(*stream, *line))
		{
			if (stream->bad())
				SSGNC_ERROR << "std::istream::getline() failed" << std::endl;
			return false;
		}
	}
	catch (...)
	{
		stream->setstate(std::ios::badbit);
		SSGNC_ERROR << "std::istream::getline() failed" << std::endl;
		return false;
	}
	return true;
}

bool readFreq(ByteReader *byte_reader, StringBuilder *buf, Int16 *freq)
{
	if (byte_reader == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (buf == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	buf->clear();
	if (freq != NULL)
		*freq = 0;

	if (!byte_reader->readFreq(buf, freq))
	{
		if (byte_reader->bad())
			SSGNC_ERROR << "ssgnc::ByteReader::readFreq() failed" << std::endl;
		return false;
	}
	return true;
}

bool readTokens(Int32 num_tokens, const VocabDic &vocab_dic,
	ByteReader *byte_reader, StringBuilder *buf, std::vector<Int32> *tokens)
{
	if (byte_reader == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (buf == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (tokens != NULL)
	{
		try
		{
			tokens->resize(num_tokens);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<ssgnc::Int32>::resize() failed: "
				<< sizeof(Int32) << " * " << num_tokens << std::endl;
			return false;
		}
	}

	for (Int32 i = 0; i < num_tokens; ++i)
	{
		Int32 token;
		if (!byte_reader->readToken(buf, &token))
		{
			SSGNC_ERROR << "ssgnc::ByteReader::readToken() failed"
				<< std::endl;
			return false;
		}

		if (static_cast<UInt32>(token) >= vocab_dic.num_keys())
		{
			SSGNC_ERROR << "Unknown token: " << token << std::endl;
			return false;
		}

		if (tokens != NULL)
			(*tokens)[i] = token;
	}
	return true;
}

bool initFilePath(const String &dirname, const String &file_ext,
	Int32 num_tokens, FilePath *file_path)
{
	if (num_tokens < MIN_NUM_TOKENS || num_tokens > MAX_NUM_TOKENS)
	{
		SSGNC_ERROR << "Out of range #tokens: " << num_tokens << std::endl;
		return false;
	}
	else if (file_path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	StringBuilder basename;
	if (!basename.appendf("%dgm-%%04d.%.*s", num_tokens,
		file_ext.length(), file_ext.ptr()))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::appendf() failed"
			<< std::endl;
		return false;
	}

	if (!file_path->open(dirname, basename.str()))
	{
		SSGNC_ERROR << "ssgnc::FilePath::open() failed: "
			<< dirname << ", " << basename << std::endl;
		return false;
	}
	return true;
}

bool openFiles(const String &dirname, const String &file_ext,
	Int32 num_tokens, std::vector<std::ifstream *> *files)
{
	if (num_tokens < MIN_NUM_TOKENS || num_tokens > MAX_NUM_TOKENS)
	{
		SSGNC_ERROR << "Out of range #tokens: " << num_tokens << std::endl;
		return false;
	}
	else if (files == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	closeFiles(files);

	FilePath file_path;
	if (!initFilePath(dirname, file_ext, num_tokens, &file_path))
	{
		SSGNC_ERROR << "ssgnc::tools::initFilePath() failed: "
			<< dirname << ", " << file_ext << ", " << num_tokens << std::endl;
		return false;
	}

	StringBuilder path;
	for ( ; ; )
	{
		if (!file_path.read(&path))
		{
			SSGNC_ERROR << "ssgnc::FilePath::read() failed" << std::endl;
			closeFiles(files);
			return false;
		}

		try
		{
			files->reserve(files->size() + 1);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<std::ifstream *>::reserve() failed: "
				<< sizeof(std::ifstream *) << " * "
				<< (files->size() + 1) << std::endl;
			closeFiles(files);
			return false;
		}

		std::ifstream *file = new std::ifstream(path.ptr(), std::ios::binary);
		if (!*file)
		{
			delete file;
			break;
		}

		files->push_back(file);
	}

	if (files->empty())
	{
		SSGNC_ERROR << "No files" << std::endl;
		return false;
	}
	return true;
}

bool closeFiles(std::vector<std::ifstream *> *files)
{
	if (files == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	for (std::size_t i = 0; i < files->size(); ++i)
	{
		if ((*files)[i] != NULL)
			delete (*files)[i];
	}
	files->clear();
	return true;
}

}  // namespace tools
}  // namespace ssgnc
