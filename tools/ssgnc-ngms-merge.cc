#include "tools-common.h"

namespace {

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

ssgnc::Int32 num_tokens;
ssgnc::VocabDic vocab_dic;

bool readNgram(HeapUnit *heap_unit)
{
	if (!ssgnc::tools::readFreq(&heap_unit->byte_reader, &heap_unit->ngram,
		&heap_unit->freq))
	{
		SSGNC_ERROR << "ssgnc::tools::readFreq() failed" << std::endl;
		return false;
	}
	else if (heap_unit->ngram.empty())
		return true;

	if (!ssgnc::tools::readTokens(num_tokens, vocab_dic,
		&heap_unit->byte_reader, &heap_unit->ngram, NULL))
	{
		SSGNC_ERROR << "ssgnc::tools::readTokens() failed" << std::endl;
		return false;
	}

	return true;
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
		if (!heap_units[i].byte_reader.open((*files)[i], BYTE_READER_BUF_SIZE))
		{
			SSGNC_ERROR << "ssngc::ByteReader::open() failed: " << std::endl;
			return false;
		}

		if (!readNgram(&heap_units[i]) || heap_units[i].ngram.empty())
		{
			SSGNC_ERROR << "readNgrams() failed: " << i << std::endl;
			delete [] heap_units;
			return false;
		}

		if (!heap_queue.push(&heap_units[i]))
		{
			SSGNC_ERROR << "ssngc::HeapQueue::push() failed: "
				<< heap_queue.size() << std::endl;
			return false;
		}
	}

	while (!heap_queue.empty())
	{
		HeapUnit *heap_unit;
		if (!heap_queue.top(&heap_unit))
		{
			SSGNC_ERROR << "ssgnc::HeapQueue::top() failed" << std::endl;
			return false;
		}

		std::cout << heap_unit->ngram;
		if (!std::cout)
		{
			SSGNC_ERROR << "std::ostream::operator<<() failed" << std::endl;
			delete [] heap_units;
			return false;
		}
		++num_ngrams;
		total_size += heap_unit->ngram.length();

		if (!readNgram(heap_unit))
		{
			SSGNC_ERROR << "readNgrams() failed" << std::endl;
			delete [] heap_units;
			return false;
		}

		if (heap_unit->ngram.empty())
			heap_queue.pop();
		else
			heap_queue.popPush(heap_unit);
	}

	delete [] heap_units;

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

	if (!vocab_dic.mmap(argv[2]))
		return 3;

	std::vector<std::ifstream *> files;
	if (!ssgnc::tools::openFiles(argv[3], "bin", num_tokens, &files))
		return 4;

	int ret = 0;
	if (!mergeFiles(&files))
		ret = 5;

	ssgnc::tools::closeFiles(&files);

	return ret;
}
