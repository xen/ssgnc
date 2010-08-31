#include "ssgnc/vocab-dic.h"

namespace ssgnc {

VocabDic::VocabDic() : num_keys_(0), hash_table_size_(0), total_size_(0),
	hash_table_(NULL), pos_list_(NULL), keys_(NULL),
	hash_table_buf_(), pos_list_buf_(), keys_buf_() {}

bool VocabDic::build(const String *keys, UInt32 num_keys)
{
	clear();

	if (keys == NULL || num_keys == 0)
		return false;

	num_keys_ = num_keys;
	hash_table_size_ = num_keys + (num_keys / 4) + 1;

	UInt32 total_length = 0;
	for (UInt32 i = 0; i < num_keys; ++i)
		total_length += keys[i].length();

	hash_table_buf_.resize(hash_table_size_, INVALID_KEY_ID);
	pos_list_buf_.reserve(num_keys_ + 1);
	keys_buf_.reserve(total_length);

	total_size_ = sizeof(hash_table_buf_[0]) * hash_table_buf_.size()
		+ sizeof(pos_list_buf_[0]) * (num_keys_ + 1)
		+ total_length;

	hash_table_ = &hash_table_buf_[0];
	pos_list_ = &pos_list_buf_[0];
	keys_ = &keys_buf_[0];

	pos_list_buf_.push_back(0);
	for (UInt32 i = 0; i < num_keys; ++i)
	{
		if (keys[i].empty())
			return false;

		UInt32 hash_id = StringHash()(keys[i]) % hash_table_size_;
		for ( ; ; )
		{
			Int32 key_id = hash_table_buf_[hash_id];
			if (key_id == INVALID_KEY_ID)
				break;
			else if (keys[i] == find(key_id))
				return false;
			hash_id = (hash_id + 1) % hash_table_size_;
		}
		hash_table_buf_[hash_id] = static_cast<Int32>(i);

		for (UInt32 j = 0; j < keys[i].length(); ++j)
			keys_buf_.push_back(keys[i][j]);
		pos_list_buf_.push_back(keys_buf_.size());
	}

	return true;
}

bool VocabDic::map(const void *address)
{
	clear();

	if (address == NULL)
		return false;

	num_keys_ = *static_cast<const UInt32 *>(address);
	hash_table_size_ = *(static_cast<const UInt32 *>(address) + 1);
	total_size_ = *(static_cast<const UInt32 *>(address) + 2);

	if (num_keys_ == 0 || hash_table_size_ == 0 || total_size_ == 0)
		return false;

	hash_table_ = static_cast<const Int32 *>(address) + 3;
	pos_list_ = reinterpret_cast<const UInt32 *>(
		hash_table_ + hash_table_size_);
	keys_ = reinterpret_cast<const Int8 *>(pos_list_ + num_keys_ + 1);

	if (sizeof(hash_table_[0]) * hash_table_size_
		+ sizeof(pos_list_[0]) * (num_keys_ + 1)
		+ pos_list_[num_keys_] != total_size_)
		return false;

	return true;
}

void VocabDic::clear()
{
	num_keys_ = 0;
	hash_table_size_ = 0;
	total_size_ = 0;

	hash_table_ = NULL;
	pos_list_ = NULL;
	keys_ = NULL;

	std::vector<Int32>().swap(hash_table_buf_);
	std::vector<UInt32>().swap(pos_list_buf_);
	std::vector<Int8>().swap(keys_buf_);
}

}  // namespace ssgnc
