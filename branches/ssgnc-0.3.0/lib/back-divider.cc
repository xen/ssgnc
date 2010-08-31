#include <ssgnc/back-divider.h>

#include <ssgnc/exception.h>

#include <algorithm>

namespace ssgnc {
namespace {

class KeyComparer
{
public:
	bool operator()(const BackDivider::KeyValuePair &lhs,
		const BackDivider::KeyValuePair &rhs) const
	{
		return lhs.key() < rhs.key();
	}
};

}  // namespace

BackDivider::BackDivider() : file_(NULL), points_(NULL),
	value_pool_(), values_(), pairs_() {}

void BackDivider::Open(TempFile *temp_file, std::vector<std::size_t> *points)
{
	BackDivider temp;

	temp.file_ = temp_file;
	temp.points_ = points;

	temp.points_->clear();
	try
	{
		temp.points_->push_back(temp.file_->Tell());
	}
	catch (...)
	{
		SSGNC_THROW("failed to open divider: std::vector::push_back() failed");
	}

	Swap(&temp);
}

void BackDivider::Close()
{
	Flush();

	file_ = NULL;
	points_ = NULL;
	value_pool_.Clear();
	std::vector<const char *>().swap(values_);
	std::vector<KeyValuePair>().swap(pairs_);
}

void BackDivider::Divide(const ParsedLine &parsed_line,
	const char *bytes, std::size_t length)
{
	const char *value = static_cast<const char *>(
		value_pool_.AppendBytes(bytes, length));
	try
	{
		values_.push_back(value);
	}
	catch (...)
	{
		SSGNC_THROW("failed to divide parsed line: "
			"std::vector::push_back() failed");
	}

	for (std::size_t i = 0; i < parsed_line.num_keys(); ++i)
	{
		KeyValuePair pair;
		pair.set_key(parsed_line.key(i));
		pair.set_value(static_cast<int>(values_.size() - 1));
		try
		{
			pairs_.push_back(pair);
		}
		catch (...)
		{
			SSGNC_THROW("failed to divide parsed line: "
				"std::vector::push_back() failed");
		}
	}

	if (pairs_.size() + parsed_line.num_keys() > MAX_NUM_PAIRS ||
		total_size() >= MAX_TOTAL_SIZE)
		Flush();
}

void BackDivider::Swap(BackDivider *target)
{
	std::swap(file_, target->file_);
	std::swap(points_, target->points_);
	value_pool_.Swap(&target->value_pool_);
	values_.swap(target->values_);
	pairs_.swap(target->pairs_);
}

void BackDivider::Flush()
{
	if (pairs_.empty())
		return;

	std::size_t num_keys = pairs_.size() / values_.size();

	try
	{
		std::stable_sort(pairs_.begin(), pairs_.end(), KeyComparer());
	}
	catch (...)
	{
		SSGNC_THROW("failed to flush divider: std::stable_sort() failed");
	}
	pairs_.erase(std::unique(pairs_.begin(), pairs_.end()), pairs_.end());

	file_->Write(num_keys + 1);
	file_->Write(pairs_.size());

	for (std::size_t i = 0; i < pairs_.size(); ++i)
		Write(pairs_[i], num_keys);

	try
	{
		points_->push_back(file_->Tell());
	}
	catch (...)
	{
		SSGNC_THROW("failed to flush divider: "
			"std::vector::push_back() failed");
	}

	value_pool_.Clear();
	std::vector<const char *>().swap(values_);
	std::vector<KeyValuePair>().swap(pairs_);
}

void BackDivider::Write(const KeyValuePair &pair, std::size_t num_keys)
{
	WriteValue(values_[pair.value()], num_keys);
	WriteKey(pair.key());
}

void BackDivider::WriteValue(const char *value, std::size_t num_keys)
{
	std::size_t count = 0;
	std::size_t length = 0;
	do
	{
		while (static_cast<unsigned char>(value[length]) >= 0x80)
			++length;
		++length;
	} while (count++ < num_keys);

	file_->Write(value, length);
}

void BackDivider::WriteKey(unsigned int key)
{
	char buf[16];
	std::size_t pos = 0;
	unsigned char byte = static_cast<unsigned char>(key & 0x7F);
	while ((key >>= 7) > 0)
	{
		buf[pos++] = byte | 0x80;
		byte = static_cast<unsigned char>(key & 0x7F);
	}
	buf[pos++] = byte;

	file_->Write(buf, pos);
}

}  // namespace ssgnc
