#include "tools-common.h"

namespace {

ssgnc::VocabDic vocab_dic;

bool initIndexFilePath(const char *index_dir, ssgnc::FilePath *file_path)
{
	ssgnc::StringBuilder format;
	format.append(index_dir);
	if (format.length() > 0 && format[format.length() - 1] != '/')
		format.append('/');
	format.append("%dgms.idx");
	file_path->set_format(format.str());
	return file_path->next();
}

bool openIndexFiles(const char *index_dir,
	std::vector<std::ifstream *> *files)
{
	ssgnc::FilePath file_path;
	if (!initIndexFilePath(index_dir, &file_path))
	{
		ERROR << "failed to initialize file path" << std::endl;
		return false;
	}

	for ( ; ; )
	{
		if (!file_path.next())
		{
			ERROR << "failed to generate path" << std::endl;
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

bool writeHeader(std::size_t num_files)
{
	ssgnc::Int32 max_num_tokens = static_cast<ssgnc::UInt32>(num_files);
	ssgnc::Int32 max_token_id = vocab_dic.num_keys();

	std::cout.write(reinterpret_cast<const char *>(&max_num_tokens),
		sizeof(max_num_tokens));
	std::cout.write(reinterpret_cast<const char *>(&max_token_id),
		sizeof(max_token_id));

	if (!std::cout)
		return false;
	return true;
}

bool mergeIndices(std::vector<std::ifstream *> *files)
{
	enum { FILE_BUF_SIZE = 1 << 20 };

	if (!writeHeader(files->size()))
		return false;

	std::vector<std::vector<char> > file_bufs(files->size());
	for (std::size_t i = 0; i < files->size(); ++i)
	{
		file_bufs[i].resize(FILE_BUF_SIZE);
		(*files)[i]->rdbuf()->pubsetbuf(&file_bufs[i][0], file_bufs[i].size());
	}

	char file_pos_buf[sizeof(ssgnc::Int16) * 3];
	for (ssgnc::UInt32 i = 0; i <= vocab_dic.num_keys(); ++i)
	{
		for (std::size_t j = 0; j < files->size(); ++j)
		{
			if (!(*files)[j]->read(file_pos_buf, sizeof(file_pos_buf)))
			{
				ERROR << "failed to read index" << std::endl;
				return false;
			}
			std::cout.write(file_pos_buf, sizeof(file_pos_buf));
		}
	}

	for (std::size_t i = 0; i < files->size(); ++i)
	{
		if (!(*files)[i]->ignore().eof())
		{
			ERROR << "extra bytes" << std::endl;
			return false;
		}
	}

	if (!std::cout.flush())
	{
		ERROR << "failed to write index" << std::endl;
		return false;
	}

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

	if (argc != 3)
	{
		std::cerr << "Usage: " << argv[0] << " VOCAB_DIC IO_DIR" << std::endl;
		return 1;
	}

	ssgnc::FileMap file_map;
	if (!ssgnc::tools::mmapVocabDic(argv[1], &file_map, &vocab_dic))
		return 2;

	std::vector<std::ifstream *> files;
	if (!openIndexFiles(argv[2], &files))
		return 3;

	int ret = 0;
	if (!mergeIndices(&files))
		ret = 4;

	for (std::size_t i = 0; i < files.size(); ++i)
	{
		if (files[i] != NULL)
		{
			delete files[i];
			files[i] = NULL;
		}
	}

	return ret;
}
