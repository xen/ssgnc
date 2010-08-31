#include <ssgnc/vocab-indexer.h>

#include <ssgnc/darts.h>
#include <ssgnc/exception.h>
#include <ssgnc/line-reader.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace ssgnc {
namespace {

class KeyComparer
{
public:
	bool operator()(const VocabIndexer::KeyValuePair &lhs,
		const VocabIndexer::KeyValuePair &rhs) const
	{
		const char *lhs_key = lhs.key();
		const char *rhs_key = rhs.key();
		while (*lhs_key != '\0' && *lhs_key == *rhs_key)
			++lhs_key, ++rhs_key;
		return static_cast<unsigned char>(*lhs_key)
			< static_cast<unsigned char>(*rhs_key);
	}
};

class ValueComparer
{
public:
	bool operator()(const VocabIndexer::KeyValuePair &lhs,
		const VocabIndexer::KeyValuePair &rhs) const
	{
		return lhs.value() > rhs.value();
	}
};

}  // namespace

void VocabIndexer::ReadVocab(std::istream *in)
{
	VocabIndexer temp;

	LineReader reader(in);
	const char *line;
	std::size_t length;
	while (reader.Read(&line, &length))
	{
		std::size_t delim_pos = length;
		for ( ; delim_pos > 0; --delim_pos)
		{
			if (line[delim_pos - 1] == '\t')
				break;
		}
		if (delim_pos == 0)
			SSGNC_THROW("failed to read vocab: no value");

		if (--delim_pos == 0)
			SSGNC_THROW("failed to read vocab: no key");

		char *value_end;
		long long value = std::strtoll(line + delim_pos + 1, &value_end, 10);

		if (*value_end != '\0')
			SSGNC_THROW("failed to read vocab: invalid character in value");
		else if (value < 0LL)
			SSGNC_THROW("failed to read vocab: negative value");
		else if (value >= std::numeric_limits<long long>::max())
			SSGNC_THROW("failed to read vocab: too large value");

		KeyValuePair pair;
		pair.set_key(temp.key_pool_.AppendString(line, delim_pos));
		pair.set_value(static_cast<std::size_t>(value));

		try
		{
			temp.pairs_.push_back(pair);
		}
		catch (...)
		{
			SSGNC_THROW("failed to read vocab: "
				"std::vector::push_back() failed");
		}
	}

	try
	{
		std::vector<KeyValuePair>(temp.pairs_).swap(temp.pairs_);
	}
	catch (...)
	{
		SSGNC_THROW("failed to read vocab: failed to shrink std::vector");
	}

	Swap(&temp);
}

void VocabIndexer::Build(std::ostream *index, std::ostream *dic)
{
	BuildIndex(index);
	BuildDic(dic);
	Clear();
}

void VocabIndexer::BuildIndex(std::ostream *out)
{
	try
	{
		std::stable_sort(pairs_.begin(), pairs_.end(), ValueComparer());
	}
	catch (...)
	{
		SSGNC_THROW("failed to build index: std::statble_sort() failed");
	}

	std::vector<std::size_t> positions;
	try
	{
		positions.resize(pairs_.size() + 1);
	}
	catch (...)
	{
		SSGNC_THROW("failed to build index: std::vector::resize() failed");
	}

	positions[0] = 0;
	for (std::size_t i = 0; i < pairs_.size(); ++i)
	{
		std::size_t length = std::strlen(pairs_[i].key());
		if (!out->write(pairs_[i].key(), length + 1))
			SSGNC_THROW("failed to build index: std::ostream::write() failed");
		positions[i + 1] = positions[i] + length + 1;
	}

	std::size_t pos = positions.back();
	while (pos % sizeof(positions[0]) != 0)
	{
		if (!out->put('\0'))
			SSGNC_THROW("failed to build index: std::ostream::put() failed");
		++pos;
	}

	if (!out->write(reinterpret_cast<const char *>(&positions[0]),
		sizeof(positions[0]) * positions.size()))
		SSGNC_THROW("failed to build index: std::ostream::write() failed");

	std::size_t num_keys = static_cast<unsigned int>(pairs_.size());
	if (!out->write(reinterpret_cast<const char *>(&num_keys),
		sizeof(num_keys)))
		SSGNC_THROW("failed to build index: std::ostream::write() failed");
}

void VocabIndexer::BuildDic(std::ostream *out)
{
	for (std::size_t i = 0; i < pairs_.size(); ++i)
		pairs_[i].set_value(i);

	try
	{
		std::stable_sort(pairs_.begin(), pairs_.end(), KeyComparer());
	}
	catch (...)
	{
		SSGNC_THROW("failed to build dictionary: std::statble_sort() failed");
	}

	std::vector<const char *> keys;
	std::vector<int> values;
	try
	{
		keys.resize(pairs_.size());
		values.resize(pairs_.size());
	}
	catch (...)
	{
		SSGNC_THROW("failed to build dictionary: "
			"std::vector::resize() failed");
	}

	for (std::size_t i = 0; i < pairs_.size(); ++i)
	{
		keys[i] = pairs_[i].key();
		values[i] = static_cast<int>(pairs_[i].value());
	}
	std::vector<KeyValuePair>().swap(pairs_);

	Darts::DoubleArray dic;
	try
	{
		if (dic.build(keys.size(), &keys[0], NULL, &values[0]) == -1)
		{
			SSGNC_THROW("failed to build dictionary: "
				"Darts::DoubleArray::build() failed");
		}
	}
	catch (...)
	{
		SSGNC_THROW("failed to build dictionary: "
			"Darts::DoubleArray::build() failed");
	}

	unsigned int num_keys = static_cast<unsigned int>(keys.size());
	if (!out->write(reinterpret_cast<const char *>(&num_keys),
		sizeof(num_keys)))
	{
		SSGNC_THROW("failed to build dictionary: "
			"std::ostream::write() failed");
	}

	unsigned int num_units = static_cast<unsigned int>(dic.size());
	if (!out->write(reinterpret_cast<const char *>(&num_units),
		sizeof(num_units)))
	{
		SSGNC_THROW("failed to build dictionary: "
			"std::ostream::write() failed");
	}

	if (!out->write(static_cast<const char *>(dic.array()), dic.total_size()))
	{
		SSGNC_THROW("failed to build dictionary: "
			"std::ostream::write() failed");
	}
}

void VocabIndexer::Clear()
{
	key_pool_.Clear();
	std::vector<KeyValuePair>().swap(pairs_);
}

void VocabIndexer::Swap(VocabIndexer *target)
{
	key_pool_.Swap(&target->key_pool_);
	pairs_.swap(target->pairs_);
}

}  // namespace ssgnc
