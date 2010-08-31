#include "tools-common.h"

namespace {

ssgnc::VocabDic vocab_dic;

bool initIndexFilePath(const ssgnc::String &index_dir,
	ssgnc::FilePath *file_path)
{
	ssgnc::StringBuilder basename;
	if (!basename.append("%dgms.idx"))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		return false;
	}

	if (!file_path->open(index_dir, basename.str()))
	{
		SSGNC_ERROR << "ssgnc::FilePath::open() failed" << std::endl;
		return false;
	}
	else if (!file_path->seek(1))
	{
		SSGNC_ERROR << "ssgnc::FilePath::seek() failed" << std::endl;
	}
	return true;
}

bool openIndexFiles(const ssgnc::String &index_dir,
	std::vector<std::ifstream *> *files)
{
	ssgnc::FilePath file_path;
	if (!initIndexFilePath(index_dir, &file_path))
	{
		SSGNC_ERROR << "initIndexFilePath() failed" << std::endl;
		return false;
	}

	ssgnc::StringBuilder path;
	for ( ; ; )
	{
		if (!file_path.read(&path))
		{
			SSGNC_ERROR << "ssgnc::FilePath::read() failed" << std::endl;
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
			ssgnc::tools::closeFiles(files);
			return false;
		}

		std::ifstream *file = new std::ifstream(
			path.ptr(), std::ios::binary);
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

bool writeHeader(std::size_t num_files)
{
	ssgnc::Int32 max_num_tokens = static_cast<ssgnc::UInt32>(num_files);
	ssgnc::Int32 max_token_id = vocab_dic.num_keys() - 1;

	if (!ssgnc::Writer(&std::cout).write(max_num_tokens) ||
		!ssgnc::Writer(&std::cout).write(max_token_id))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed" << std::endl;
		return false;
	}
	return true;
}

bool mergeIndices(std::vector<std::ifstream *> *files)
{
	enum { FILE_BUF_SIZE = 1 << 20 };

	if (!writeHeader(files->size()))
	{
		SSGNC_ERROR << "writeHeader() failed" << std::endl;
		return false;
	}

	std::vector<std::vector<char> > file_bufs;
	try
	{
		file_bufs.resize(files->size());
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<std::vector<char> >::resize() failed: "
			<< sizeof(std::vector<char>) << " * "
			<< files->size() << std::endl;
		return false;
	}

	for (std::size_t i = 0; i < files->size(); ++i)
	{
		try
		{
			file_bufs[i].resize(FILE_BUF_SIZE);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<char>::resize() failed: "
				<< FILE_BUF_SIZE << std::endl;
			return false;
		}
		(*files)[i]->rdbuf()->pubsetbuf(&file_bufs[i][0], file_bufs[i].size());
	}

	ssgnc::NgramIndex::FileEntry entries;
	for (ssgnc::UInt32 i = 0; i <= vocab_dic.num_keys(); ++i)
	{
		for (std::size_t j = 0; j < files->size(); ++j)
		{
			if (!ssgnc::Reader((*files)[j]).read(&entries))
			{
				SSGNC_ERROR << "ssgnc::Reader::read() failed" << std::endl;
				return false;
			}

			if (!ssgnc::Writer(&std::cout).write(entries))
			{
				SSGNC_ERROR << "ssgnc::Writer::write() failed" << std::endl;
				return false;
			}
		}
	}

	for (std::size_t i = 0; i < files->size(); ++i)
	{
		if ((*files)[i]->get() != EOF)
		{
			SSGNC_ERROR << "Extra bytes" << std::endl;
			return false;
		}
	}

	if (!std::cout.flush())
	{
		SSGNC_ERROR << "std::ofstream::flush() failed" << std::endl;
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
		std::cerr << "Usage: " << argv[0]
			<< " VOCAB_DIC TEMP_DIR" << std::endl;
		return 1;
	}

	if (!vocab_dic.open(argv[1]))
		return 2;

	std::vector<std::ifstream *> files;
	if (!openIndexFiles(argv[2], &files))
		return 3;

	int ret = 0;
	if (!mergeIndices(&files))
		ret = 4;

	ssgnc::tools::closeFiles(&files);

	return ret;
}
