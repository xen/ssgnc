#include "tools-common.h"

namespace {

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

bool mergeDatabase(std::vector<std::ifstream *> *files)
{
	enum { BYTE_READER_BUF_SIZE = 1 << 20 };

	ssgnc::ByteReader *readers = new ssgnc::ByteReader[files->size()];
	for (std::size_t i = 0; i < files->size(); ++i)
		readers[i].open((*files)[i], BYTE_READER_BUF_SIZE);

	ssgnc::UInt64 num_ngrams = 0;
	ssgnc::UInt64 total_size = 0;

	ssgnc::StringBuilder ngram_buf;
	for (ssgnc::UInt32 token_id = 0; token_id < vocab_dic.num_keys();
		++token_id)
	{
		for (std::size_t file_id = 0; file_id < files->size(); ++file_id)
		{
			for ( ; ; )
			{
				ssgnc::Int16 freq;
				if (!readNgram(&readers[file_id], &freq, &ngram_buf))
				{
					ERROR << "failed to read ngram" << std::endl;
					delete [] readers;
					return false;
				}
				else if (freq == 0)
					break;

				std::cout << ngram_buf.str();

				++num_ngrams;
				total_size += ngram_buf.length();
			}
		}
		std::cout.put('\0');
	}

	for (std::size_t file_id = 0; file_id < files->size(); ++file_id)
	{
		if (readers[file_id].read() >= 0)
		{
			ERROR << "extra bytes" << std::endl;
			delete [] readers;
			return false;
		}
	}

	delete [] readers;

	if (!std::cout.flush())
	{
		ERROR << "failed to flush standard output" << std::endl;
		return false;
	}

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
			<< " NUM_TOKENS VOCAB_DIC TEMP_DIR" << std::endl;
		return 1;
	}

	if (!ssgnc::tools::parseNumTokens(argv[1], &num_tokens))
		return 2;

	ssgnc::FileMap file_map;
	if (!ssgnc::tools::mmapVocabDic(argv[2], &file_map, &vocab_dic))
		return 3;

	std::vector<std::ifstream *> files;
	if (!ssgnc::tools::openFiles(argv[3], "part", num_tokens, &files))
		return 4;

	int ret = 0;
	if (!mergeDatabase(&files))
		ret = 5;

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
