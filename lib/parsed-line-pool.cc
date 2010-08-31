#include <ssgnc/parsed-line-pool.h>

#include <ssgnc/exception.h>

#include <algorithm>

namespace ssgnc {
namespace {

class ValueComparer
{
public:
	bool operator()(const ParsedLinePool::KeyValuePair &lhs,
		const ParsedLinePool::KeyValuePair &rhs) const
	{
		return lhs.value() > rhs.value();
	}
};

}  // namespace

ParsedLinePool::ParsedLinePool(std::size_t chunk_size) :
	key_pool_(chunk_size), pairs_(), is_sorted_(false), num_keys_(0) {}

void ParsedLinePool::Append(const ParsedLine &line)
{
	if (line.num_keys() == 0)
		SSGNC_THROW("failed to append parsed line: no keys");
	else if (line.num_keys() > MAX_NUM_KEYS)
		SSGNC_THROW("failed to append parsed line: too many keys");
	else if (num_keys_ == 0)
		num_keys_ = line.num_keys();
	else if (line.num_keys() != num_keys_)
		SSGNC_THROW("failed to append parsed line: invalid number of keys");

	char buf[ENCODER_BUF_SIZE];
	std::size_t size = Encode(line, buf);
	const char *key = static_cast<const char *>(
		key_pool_.AppendBytes(buf, size));

	KeyValuePair pair;
	pair.set_key(key);
	pair.set_value(line.value());

	try
	{
		pairs_.push_back(pair);
	}
	catch (...)
	{
		SSGNC_THROW("failed to append parsed line: "
			"std::vector::push_back() failed");
	}
}

void ParsedLinePool::Sort()
{
	try
	{
		if (!is_sorted_)
			std::stable_sort(pairs_.begin(), pairs_.end(), ValueComparer());
		is_sorted_ = true;
	}
	catch (...)
	{
		SSGNC_THROW("failed to sort parsed lines: std::stable_sort() failed");
	}
}

void ParsedLinePool::Write(TempFile *file)
{
	for (std::size_t i = 0; i < pairs_.size(); ++i)
	{
		const char *key = pairs_[i].key();
		std::size_t count = 0;
		std::size_t length = 0;
		do
		{
			while (static_cast<unsigned char>(key[length]) >= 0x80)
				++length;
			++length;
		} while (count++ < num_keys_);
		file->Write(key, length);
	}
}

void ParsedLinePool::Clear()
{
	key_pool_.Clear();
	std::vector<KeyValuePair>().swap(pairs_);
	is_sorted_ = false;
	num_keys_ = 0;
}

void ParsedLinePool::Swap(ParsedLinePool *target)
{
	key_pool_.Swap(&target->key_pool_);
	pairs_.swap(target->pairs_);
	std::swap(is_sorted_, target->is_sorted_);
	std::swap(num_keys_, target->num_keys_);
}

std::size_t ParsedLinePool::Encode(const ParsedLine &line, char *buf)
{
	std::size_t pos = 0;

	unsigned long long value = line.value();
	unsigned char byte = static_cast<unsigned char>(value & 0x7F);
	while ((value >>= 7) > 0)
	{
		buf[pos++] = byte | 0x80;
		byte = static_cast<unsigned char>(value & 0x7F);
	}
	buf[pos++] = byte;

	for (std::size_t i = 0; i < line.num_keys(); ++i)
	{
		unsigned int key = line.key(i);
		byte = static_cast<unsigned char>(key & 0x7F);
		while ((key >>= 7) > 0)
		{
			buf[pos++] = byte | 0x80;
			byte = static_cast<unsigned char>(key & 0x7F);
		}
		buf[pos++] = byte;
	}

	return pos;
}

}  // namespace ssgnc
