#include "tools-common.h"

namespace {

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

class HeapUnit
{
public:
	HeapUnit() : freq(0), ngram(), byte_reader() {}
	~HeapUnit() {}

	ssgnc::Int16 freq;
	ssgnc::StringBuilder ngram;
	ssgnc::ByteReader byte_reader;

	bool operator<(const HeapUnit &rhs) const;
};

class HeapUnitComparer
{
public:
	bool operator()(const HeapUnit *lhs, const HeapUnit *rhs) const
	{
		if (lhs->freq != rhs->freq)
			return lhs->freq > rhs->freq;
		return lhs < rhs;
	}
};

bool readNgram(HeapUnit *heap_unit)
{
	heap_unit->freq = 0;
	heap_unit->ngram.clear();

	ssgnc::Int32 byte;
	ssgnc::Int32 freq = 0;
	while ((byte = heap_unit->byte_reader.read()) >= 0)
	{
		heap_unit->ngram.append(static_cast<ssgnc::Int8>(byte));

		freq = (freq << 7) + (byte & 0x7F);
		if (freq > ssgnc::FreqHandler::MAX_ENCODED_FREQ)
		{
			ERROR << "out of range freq: " << freq << std::endl;
			return false;
		}
		else if (byte < 0x80)
			break;
	}
	heap_unit->freq = static_cast<ssgnc::Int16>(freq);

	if (byte < 0)
		return false;

	ssgnc::Int32 token = 0;
	ssgnc::Int32 token_count = 0;
	while ((byte = heap_unit->byte_reader.read()) >= 0)
	{
		heap_unit->ngram.append(static_cast<ssgnc::Int8>(byte));

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

bool mergeFiles(std::vector<std::ifstream *> *files)
{
	enum { BYTE_READER_BUF_SIZE = 1 << 20 };

	ssgnc::HeapQueue<HeapUnit *, HeapUnitComparer> heap_queue;
	HeapUnit *heap_units = new HeapUnit[files->size()];

	ssgnc::UInt64 num_ngrams = 0;
	ssgnc::UInt64 total_size = 0;

	for (std::size_t i = 0; i < files->size(); ++i)
	{
		heap_units[i].byte_reader.open((*files)[i], BYTE_READER_BUF_SIZE);
		if (!readNgram(&heap_units[i]))
		{
			ERROR << "no ngrams in file" << std::endl;
			delete [] heap_units;
			return false;
		}
		heap_queue.push(&heap_units[i]);
	}

	while (!heap_queue.empty())
	{
		HeapUnit *heap_unit = heap_queue.top();

		std::cout << heap_unit->ngram.str();
		if (!std::cout)
		{
			ERROR << "failed to write ngram" << std::endl;
			delete [] heap_units;
			return false;
		}
		++num_ngrams;
		total_size += heap_unit->ngram.length();

		if (readNgram(heap_unit))
			heap_queue.popPush(heap_unit);
		else if (!heap_unit->byte_reader.eof())
		{
			delete [] heap_units;
			return false;
		}
		else
			heap_queue.pop();
	}

	delete [] heap_units;

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
	if (!ssgnc::tools::openFiles(argv[3], "bin", num_tokens, &files))
		return 4;

	int ret = 0;
	if (!mergeFiles(&files))
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
