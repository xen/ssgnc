#include "tools-common.h"

#include <algorithm>
#include <string>

namespace {

ssgnc::MemPool key_pool;
ssgnc::FreqHandler freq_handler;

class KeyFreqPair
{
public:
	KeyFreqPair(const ssgnc::String &key, long long freq)
		: index_(key_pool.append(key)),
		length_(static_cast<ssgnc::UInt16>(key.length())),
		freq_(freq_handler.encode(static_cast<ssgnc::Int64>(freq))) {}

	ssgnc::String key() const
	{ return ssgnc::String(key_pool.get(index_), length_); }
	ssgnc::Int16 freq() const { return freq_; }

private:
	ssgnc::UInt32 index_;
	ssgnc::UInt16 length_;
	ssgnc::Int16 freq_;
};

class FreqOrderComparer
{
public:
	bool operator()(const KeyFreqPair &lhs, const KeyFreqPair &rhs) const
	{
		if (lhs.freq() != rhs.freq())
			return lhs.freq() > rhs.freq();
		return lhs.key() < rhs.key();
	}
};

bool readKeyFreqPairs(std::vector<KeyFreqPair> *key_freq_pairs)
{
	std::cerr << "reading..." << std::endl;

	std::string line;
	while (std::getline(std::cin, line))
	{
		if (line.find_first_of(' ') != std::string::npos)
		{
			ERROR << "invalid format: " << line << std::endl;
			return false;
		}

		std::string::size_type pos = line.find_last_of('\t');
		if (pos == std::string::npos)
		{
			ERROR << "invalid format: " << line << std::endl;
			return false;
		}

		const char *freq_str = line.c_str() + pos + 1;
		char *end_of_freq;
		long long freq = std::strtoll(freq_str, &end_of_freq, 10);
		if (*end_of_freq != '\0')
		{
			ERROR << "invalid freq: " << line << std::endl;
			return false;
		}
		else if (freq <= 0 || freq > ssgnc::FreqHandler::MAX_FREQ)
		{
			ERROR << "out of range freq: " << freq << std::endl;
			return false;
		}

		ssgnc::String key(line.c_str(), pos);
		key_freq_pairs->push_back(KeyFreqPair(key, freq));

		if (key_freq_pairs->size() % 100000 == 0)
			std::cerr << "\rNo. keys: " << key_freq_pairs->size();
	}
	std::cerr << "\rNo. keys: " << key_freq_pairs->size() << std::endl;

	if (key_freq_pairs->size() == 0)
	{
		ERROR << "no input: " << std::endl;
		return false;
	}

	return true;
}

bool sortKeys(std::vector<KeyFreqPair> *key_freq_pairs,
	std::vector<ssgnc::String> *keys)
{
	std::cerr << "sorting..." << std::endl;

	std::sort(key_freq_pairs->begin(), key_freq_pairs->end(),
		FreqOrderComparer());

	keys->resize(key_freq_pairs->size());
	for (std::size_t i = 0; i < keys->size(); ++i)
		(*keys)[i] = (*key_freq_pairs)[i].key();

	std::vector<KeyFreqPair>().swap(*key_freq_pairs);

	return true;
}

bool buildVocabDic(const std::vector<ssgnc::String> &keys,
	ssgnc::VocabDic *vocab_dic)
{
	std::cerr << "building..." << std::endl;

	if (!vocab_dic->build(&keys[0], keys.size()))
	{
		ERROR << "failed to build dictionary" << std::endl;
		return false;
	}
	std::cerr << "Total size: " << vocab_dic->total_size() << std::endl;
	return true;
}

bool writeVocabDic(const ssgnc::VocabDic &vocab_dic)
{
	std::cerr << "writing..." << std::endl;

	if (!vocab_dic.write(&std::cout))
	{
		ERROR << "failed to write dictionary" << std::endl;
		return false;
	}
	else if (!std::cout.flush())
	{
		ERROR << "failed to flush output stream" << std::endl;
		return false;
	}

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::tools::initIO();

	if (argc != 1)
	{
		std::cerr << "Usage: " << argv[0] << std::endl;
		return 1;
	}

	std::vector<KeyFreqPair> key_freq_pairs;
	if (!readKeyFreqPairs(&key_freq_pairs))
		return 2;

	std::vector<ssgnc::String> keys;
	if (!sortKeys(&key_freq_pairs, &keys))
		return 3;

	ssgnc::VocabDic vocab_dic;
	if (!buildVocabDic(keys, &vocab_dic))
		return 4;

	if (!writeVocabDic(vocab_dic))
		return 5;

	return 0;
}
