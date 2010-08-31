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

bool parseNumTokens(const Int8 *str, Int32 *num_tokens)
{
	enum { MIN_NUM_TOKENS = 1, MAX_NUM_TOKENS = 100 };

	Int8 *end_of_num_tokens;
	long temp_num_tokens = std::strtol(str, &end_of_num_tokens, 10);
	if (*end_of_num_tokens != '\0')
	{
		ERROR << "invalid NUM_TOKENS: " << str << std::endl;
		return false;
	}
	else if (temp_num_tokens < MIN_NUM_TOKENS ||
		temp_num_tokens > MAX_NUM_TOKENS)
	{
		ERROR << "out of range NUM_TOKENS: " << str << std::endl;
		return false;
	}

	*num_tokens = static_cast<int>(temp_num_tokens);
	std::cerr << "NUM_TOKENS: " << *num_tokens << std::endl;

	return true;
}

bool readVocabDic(const char *path, ssgnc::VocabDic *vocab_dic)
{
	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		ERROR << "failed to open file: " << path << std::endl;
		return false;
	}

	if (!vocab_dic->read(&file))
	{
		ERROR << "failed to read dictionary: " << path << std::endl;
		return false;
	}

	std::cerr << "VOCAB_DIC: " << path << std::endl;

	return true;
}

bool mmapVocabDic(const char *path, ssgnc::FileMap *file_map,
	ssgnc::VocabDic *vocab_dic)
{
	if (!file_map->open(path))
	{
		ERROR << "failed to open file: " << path << std::endl;
		return false;
	}

	if (!vocab_dic->map(file_map->ptr()))
	{
		ERROR << "failed to read dictionary: " << path << std::endl;
		return false;
	}

	return true;
}

bool initFilePath(const char *temp_dir, const char *file_ext,
	Int32 num_tokens, FilePath *file_path)
{
	char num_tokens_buf[16];
	std::sprintf(num_tokens_buf, "%d", num_tokens);

	StringBuilder format;
	format.append(temp_dir);
	if (format.length() > 0 && format[format.length() - 1] != '/')
		format.append('/');
	format.append(num_tokens_buf);
	format.append("gm-%04d.");
	format.append(file_ext);
	file_path->set_format(format.str());
	return true;
}

bool openFiles(const char *temp_dir, const char *file_ext,
	Int32 num_tokens, std::vector<std::ifstream *> *files)
{
	FilePath file_path;
	if (!initFilePath(temp_dir, file_ext, num_tokens, &file_path))
	{
		ERROR << "failed to initialize file path" << std::endl;
		return false;
	}

	for ( ; ; )
	{
		if (!file_path.next())
		{
			ERROR << "failed to generate file path" << std::endl;
			return false;
		}

		std::ifstream *file = new std::ifstream(
			file_path.path().ptr(), std::ios::binary);
		if (!*file)
		{
			delete file;
			break;
		}

		files->push_back(file);
	}

	if (files->empty())
	{
		ERROR << "no input" << std::endl;
		return false;
	}

	return true;
}

}  // namespace tools
}  // namespace ssgnc
