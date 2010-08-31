#include "tools-common.h"

#include <algorithm>

namespace {

ssgnc::MemPool key_pool;
ssgnc::FreqHandler freq_handler;

class KeyFreqPair
{
public:
	KeyFreqPair(ssgnc::UInt32 index, ssgnc::UInt32 length, ssgnc::Int16 freq)
		: index_(index), length_(static_cast<ssgnc::UInt16>(length)),
		freq_(freq) {}

	ssgnc::String key() const
	{
		ssgnc::String str;
		if (!key_pool.get(index_, length_, &str))
		{
			SSGNC_ERROR << "ssgnc::MemPool::get() failed: "
				<< index_ << ", " << length_ << std::endl;
			exit(10);
		}
		return str;
	}
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
	std::string line;
	while (ssgnc::tools::readLine(&std::cin, &line))
	{
		if (line.find_first_of(' ') != std::string::npos)
		{
			SSGNC_ERROR << "Wrong delimitor: " << line << std::endl;
			return false;
		}

		std::string::size_type delim_pos = line.find_last_of('\t');
		if (delim_pos == std::string::npos)
		{
			SSGNC_ERROR << "No delimitor: " << line << std::endl;
			return false;
		}

		const ssgnc::Int8 *freq_str = line.c_str() + delim_pos + 1;
		ssgnc::Int64 freq;
		if (!ssgnc::tools::parseInt64(freq_str, &freq))
		{
			SSGNC_ERROR << "ssgnc::tools::parseInt64() failed: "
				<< freq_str << std::endl;
			return false;
		}

		ssgnc::String key(line.c_str(), delim_pos);
		ssgnc::UInt32 index;
		if (!key_pool.append(key, &index))
		{
			SSGNC_ERROR << "ssgnc::MemPool::append() failed: "
				<< key << std::endl;
			return false;
		}

		ssgnc::Int16 encoded_freq;
		if (!freq_handler.encode(freq, &encoded_freq))
		{
			SSGNC_ERROR << "ssgnc::FreqHandler::encode() failed: "
				<< freq << std::endl;
			return false;
		}

		try
		{
			key_freq_pairs->push_back(KeyFreqPair(
				index, key.length(), encoded_freq));
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<KeyFreqPair>::push_back() failed: "
				<< key_freq_pairs->size() << std::endl;
			return false;
		}

		if (key_freq_pairs->size() % 100000 == 0)
			std::cerr << "\rNo. keys: " << key_freq_pairs->size();
	}
	if (std::cin.bad())
		return false;
	std::cerr << "\rNo. keys: " << key_freq_pairs->size() << std::endl;

	if (key_freq_pairs->empty())
	{
		SSGNC_ERROR << "No input" << std::endl;
		return false;
	}

	return true;
}

bool sortKeys(std::vector<KeyFreqPair> *key_freq_pairs,
	std::vector<ssgnc::String> *keys)
{
	std::sort(key_freq_pairs->begin(), key_freq_pairs->end(),
		FreqOrderComparer());
	try
	{
		keys->resize(key_freq_pairs->size());
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::String>::resize() failed: "
			<< key_freq_pairs->size() << std::endl;
		return false;
	}

	for (std::size_t i = 0; i < keys->size(); ++i)
		(*keys)[i] = (*key_freq_pairs)[i].key();

	std::vector<KeyFreqPair>().swap(*key_freq_pairs);
	return true;
}

bool buildVocabDic(const std::vector<ssgnc::String> &keys,
	ssgnc::VocabDic *vocab_dic)
{
	if (!vocab_dic->build(&keys[0], static_cast<ssgnc::UInt32>(keys.size())))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::build() failed: "
			<< keys.size() << std::endl;
		return false;
	}
	std::cerr << "Total size: " << vocab_dic->total_size() << std::endl;
	return true;
}

bool writeVocabDic(const ssgnc::VocabDic &vocab_dic)
{
	if (!vocab_dic.write(&std::cout))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::write() failed" << std::endl;
		return false;
	}
	else if (!std::cout.flush())
	{
		SSGNC_ERROR << "std::ostream::flush() failed" << std::endl;
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
