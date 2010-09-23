#include "tools-common.h"

namespace {

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

bool readNgram(ssgnc::ByteReader *byte_reader, ssgnc::Int16 *freq,
	ssgnc::StringBuilder *ngram_buf)
{
	if (!ssgnc::tools::readFreq(byte_reader, ngram_buf, freq))
	{
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

bool mergeDatabase(std::vector<std::ifstream *> *files)
{
	enum { BYTE_READER_BUF_SIZE = 1 << 20 };

	ssgnc::ByteReader *readers;
	try
	{
		readers = new ssgnc::ByteReader[files->size()];
	}
	catch (...)
	{
		SSGNC_ERROR << "new ssgnc::ByteReader[] failed: "
			<< sizeof(ssgnc::ByteReader) << " * "
			<< files->size() << std::endl;
		return false;
	}

	for (std::size_t i = 0; i < files->size(); ++i)
	{
		if (!readers[i].open((*files)[i], BYTE_READER_BUF_SIZE))
		{
			SSGNC_ERROR << "ssgnc::ByteReader::open() failed: " << std::endl;
			delete [] readers;
			return false;
		}
	}

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
					SSGNC_ERROR << "readNgram() failed" << std::endl;
					delete [] readers;
					return false;
				}
				else if (freq == 0)
					break;

				std::cout << ngram_buf;
				if (!std::cout)
				{
					SSGNC_ERROR << "std::ostream::operator<<() failed"
						<< std::endl;
					return false;
				}

				++num_ngrams;
				total_size += ngram_buf.length();
			}
		}
		std::cout.put('\0');
	}

	for (std::size_t file_id = 0; file_id < files->size(); ++file_id)
	{
		if (!readers[file_id].eof())
		{
			SSGNC_ERROR << "Extra bytes" << std::endl;
			delete [] readers;
			return false;
		}
	}

	delete [] readers;

	if (!std::cout.flush())
	{
		SSGNC_ERROR << "std::ostream::flush() failed" << std::endl;
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

	if (!vocab_dic.open(argv[2]))
		return 3;

	std::vector<std::ifstream *> files;
	if (!ssgnc::tools::openFiles(argv[3], "part", num_tokens, &files))
		return 4;

	int ret = 0;
	if (!mergeDatabase(&files))
		ret = 5;

	ssgnc::tools::closeFiles(&files);

	return ret;
}
