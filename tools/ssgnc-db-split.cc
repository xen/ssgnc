#include "tools-common.h"

namespace {

struct FilePos
{
	ssgnc::Int16 file_id;
	ssgnc::UInt16 offset_lo;
	ssgnc::UInt16 offset_hi;
};

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

bool readNgram(ssgnc::ByteReader *byte_reader, ssgnc::Int16 *freq,
	ssgnc::StringBuilder *ngram_buf)
{
	ngram_buf->clear();

	ssgnc::Int32 byte;
	ssgnc::Int32 temp_freq = 0;
	while ((byte = byte_reader->read()) >= 0)
	{
		ngram_buf->append(static_cast<ssgnc::Int8>(byte));

		temp_freq = (temp_freq << 7) + (byte & 0x7F);
		if (temp_freq > ssgnc::FreqHandler::MAX_ENCODED_FREQ)
		{
			ERROR << "out of range freq: " << temp_freq << std::endl;
			return false;
		}
		else if (byte < 0x80)
			break;
	}
	*freq = static_cast<ssgnc::Int16>(temp_freq);

	if (byte < 0)
		return false;
	else if (*freq == 0)
		return true;

	ssgnc::Int32 token = 0;
	ssgnc::Int32 token_count = 0;
	while ((byte = byte_reader->read()) >= 0)
	{
		ngram_buf->append(static_cast<ssgnc::Int8>(byte));

		token = (token << 7) + (byte & 0x7F);
		if (byte < 0x80)
		{
			if (static_cast<ssgnc::UInt32>(token) >= vocab_dic.num_keys())
			{
				ERROR << "unknown token: " << token << std::endl;
				return false;
			}
			token = 0;

			if (++token_count == num_tokens)
				return true;
		}
	}

	return false;
}

bool openNextFile(ssgnc::FilePath *file_path, std::ofstream *file)
{
	if (file_path->file_id() != -1)
	{
		if (!file->flush())
		{
			ERROR << "failed to write ngrams" << std::endl;
			return false;
		}
		file->close();
	}

	if (!file_path->next())
	{
		ERROR << "failed to generate file path" << std::endl;
		return false;
	}

	file->open(file_path->path().ptr(), std::ios::binary);
	if (!*file)
	{
		ERROR << "failed to open file: " << file_path->path() << std::endl;
		return false;
	}

	return true;
}

bool writeFilePos(ssgnc::Int32 file_id, ssgnc::UInt32 offset)
{
	FilePos file_pos;

	file_pos.file_id = static_cast<ssgnc::Int16>(file_id);
	file_pos.offset_lo = static_cast<ssgnc::UInt16>(offset & 0xFFFF);
	file_pos.offset_hi = static_cast<ssgnc::UInt16>(offset >> 16);

	std::cout.write(reinterpret_cast<const char *>(&file_pos),
		sizeof(file_pos));
	if (!std::cout)
	{
		ERROR << "failed to write index" << std::endl;
		return false;
	}

	return true;
}

bool splitDatabase(ssgnc::FilePath *file_path)
{
	static const ssgnc::UInt32 MAX_FILE_SIZE = 0x7FFFFFFFU;

	std::ofstream file;

	ssgnc::ByteReader byte_reader;
	byte_reader.open(&std::cin);

	ssgnc::UInt64 num_ngrams = 0;
	ssgnc::UInt32 file_size = MAX_FILE_SIZE + 1;
	ssgnc::UInt64 total_size = 0;

	if (!writeFilePos(0, 0))
		return false;

	ssgnc::Int16 freq;
	ssgnc::StringBuilder ngram_buf;
	while (readNgram(&byte_reader, &freq, &ngram_buf))
	{
		if (file_size + ngram_buf.length() > MAX_FILE_SIZE)
		{
			if (file_path->file_id() != -1)
			{
				std::cerr << "File ID: " << file_path->file_id()
					<< ", File size: " << file_size << std::endl;
			}

			if (!openNextFile(file_path, &file))
				return false;
			file_size = 0;
		}

		if (freq == 0)
		{
			file.put('\0');
			++file_size;

			if (!writeFilePos(file_path->file_id(), file_size))
				return false;
			continue;
		}

		file << ngram_buf.str();
		file_size += ngram_buf.length();
		total_size += ngram_buf.length();

		++num_ngrams;
	}

	if (!file.flush())
	{
		ERROR << "failed to write ngrams" << std::endl;
		return false;
	}
	else if (!std::cout.flush())
	{
		ERROR << "failed to write index" << std::endl;
		return false;
	}

	if (byte_reader.read() >= 0 || !byte_reader.eof())
	{
		ERROR << "extra bytes" << std::endl;
		return false;
	}

	std::cerr << "File ID: " << file_path->file_id()
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
			<< " NUM_TOKENS VOCAB_DIC OUTPUT_DIR" << std::endl;
		return 1;
	}

	if (!ssgnc::tools::parseNumTokens(argv[1], &num_tokens))
		return 2;

	ssgnc::FileMap file_map;
	if (!ssgnc::tools::mmapVocabDic(argv[2], &file_map, &vocab_dic))
		return 3;

	ssgnc::FilePath file_path;
	if (!ssgnc::tools::initFilePath(argv[3], "db", num_tokens, &file_path))
		return 4;

	if (!splitDatabase(&file_path))
		return 5;

	return 0;
}
