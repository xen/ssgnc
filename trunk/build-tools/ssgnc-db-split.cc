#include "tools-common.h"

namespace {

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

bool readNgram(ssgnc::ByteReader *byte_reader, ssgnc::Int16 *freq,
	ssgnc::StringBuilder *ngram_buf)
{
	if (!ssgnc::tools::readFreq(byte_reader, ngram_buf, freq))
	{
		if (byte_reader->bad())
			SSGNC_ERROR << "ssgnc::tools::readFreq() failed" << std::endl;
		return false;
	}

	if (*freq == 0)
		return true;

	if (!ssgnc::tools::readTokens(num_tokens, vocab_dic,
		byte_reader, ngram_buf, NULL))
	{
		SSGNC_ERROR << "ssgnc::tools::readTokens() failed" << std::endl;
		return false;
	}

	return true;
}

bool openNextFile(ssgnc::FilePath *file_path, std::ofstream *file)
{
	if (file_path->tell() != 0)
	{
		if (!file->flush())
		{
			SSGNC_ERROR << "std::ofstream::flush() failed" << std::endl;
			return false;
		}
		file->close();
	}

	ssgnc::StringBuilder path;
	if (!file_path->read(&path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::read() failed" << std::endl;
		return false;
	}

	if (file->is_open())
		file->close();

	file->open(path.ptr(), std::ios::binary);
	if (!*file)
	{
		SSGNC_ERROR << "std::ofstream::open() failed: "
			<< path.str() << std::endl;
		return false;
	}

	return true;
}

bool writeNgramOffset(ssgnc::Int32 file_id, ssgnc::UInt32 offset)
{
	ssgnc::NgramIndex::FileEntry entries;

	if (!entries.set_file_id(file_id))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::FileEntry::set_file_id() failed: "
			<< file_id << std::endl;
		return false;
	}
	if (!entries.set_offset(offset))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::FileEntry::set_offset() failed: "
			<< offset << std::endl;
		return false;
	}

	if (!ssgnc::Writer(&std::cout).write(entries))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed" << std::endl;
		return false;
	}

	return true;
}

bool splitDatabase(ssgnc::FilePath *file_path)
{
	static const ssgnc::UInt32 MAX_FILE_SIZE = 0x7FFFFFFFU;

	std::ofstream file;

	ssgnc::ByteReader byte_reader;
	if (!byte_reader.open(&std::cin))
	{
		SSGNC_ERROR << "ssgnc::ByteReader::open() failed" << std::endl;
		return false;
	}

	ssgnc::UInt64 num_ngrams = 0;
	ssgnc::UInt32 file_size = MAX_FILE_SIZE + 1;
	ssgnc::UInt64 total_size = 0;

	if (!writeNgramOffset(0, 0))
	{
		SSGNC_ERROR << "writeNgramOffset() failed" << std::endl;
		return false;
	}

	ssgnc::Int16 freq;
	ssgnc::StringBuilder ngram_buf;
	while (readNgram(&byte_reader, &freq, &ngram_buf))
	{
		if (file_size + ngram_buf.length() > MAX_FILE_SIZE)
		{
			if (file_path->tell() != 0)
			{
				std::cerr << "File ID: " << (file_path->tell() - 1)
					<< ", File size: " << file_size << std::endl;
			}

			if (!openNextFile(file_path, &file))
			{
				SSGNC_ERROR << "openNextFile() failed" << std::endl;
				return false;
			}
			file_size = 0;
		}

		if (freq == 0)
		{
			file.put('\0');
			++file_size;

			if (!writeNgramOffset(file_path->tell() - 1, file_size))
			{
				SSGNC_ERROR << "writeNgramOffset() failed" << std::endl;
				return false;
			}
			continue;
		}

		file << ngram_buf;
		if (!file)
		{
			SSGNC_ERROR << "std::ofstream::operator<<() failed" << std::endl;
		}

		file_size += ngram_buf.length();
		total_size += ngram_buf.length();

		++num_ngrams;
	}

	if (!file.flush())
	{
		SSGNC_ERROR << "std::ofstream::flush() failed" << std::endl;
		return false;
	}
	else if (!std::cout.flush())
	{
		SSGNC_ERROR << "std::ostream::flush() failed" << std::endl;
		return false;
	}

	if (byte_reader.bad())
	{
		SSGNC_ERROR << "readNgram() failed" << std::endl;
		return false;
	}
	else if (!byte_reader.eof())
	{
		SSGNC_ERROR << "Extra bytes" << std::endl;
		return false;
	}

	std::cerr << "File ID: " << (file_path->tell() - 1)
		<< ", File size: " << file_size << std::endl;

	std::cerr << "No. ngrams: " << num_ngrams
		<< ", Total size: " << total_size << std::endl;

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

	if (argc != 4)
	{
		std::cerr << "Usage: " << argv[0]
			<< " NUM_TOKENS VOCAB_DIC INDEX_DIR" << std::endl;
		return 1;
	}

	if (!ssgnc::tools::parseNumTokens(argv[1], &num_tokens))
		return 2;

	if (!vocab_dic.open(argv[2]))
		return 3;

	ssgnc::FilePath file_path;
	if (!ssgnc::tools::initFilePath(argv[3], "db", num_tokens, &file_path))
		return 4;

	if (!splitDatabase(&file_path))
		return 5;

	return 0;
}
