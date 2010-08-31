#ifndef SSGNC_VOCAB_DIC_H
#define SSGNC_VOCAB_DIC_H

#include "string.h"
#include "string-hash.h"

#include <vector>

namespace ssgnc {

class VocabDic
{
public:
	VocabDic();
	~VocabDic() { clear(); }

	UInt32 num_keys() const { return num_keys_; }
	UInt32 hash_table_size() const { return hash_table_size_; }
	UInt32 total_size() const { return total_size_; }

	Int32 operator[](const String &key) const { return find(key); }
	String operator[](Int32 key_id) const { return find(key_id); }

	Int32 find(const String &key) const;
	String find(Int32 key_id) const;

	bool build(const String *keys, UInt32 num_keys);

	template <typename InputStream>
	bool read(InputStream *in);
	template <typename OutputStream>
	bool write(OutputStream *out) const;

	bool map(const void *address);

	void clear();

private:
	UInt32 num_keys_;
	UInt32 hash_table_size_;
	UInt32 total_size_;
	const Int32 *hash_table_;
	const UInt32 *pos_list_;
	const Int8 *keys_;
	std::vector<Int32> hash_table_buf_;
	std::vector<UInt32> pos_list_buf_;
	std::vector<Int8> keys_buf_;

	enum { INVALID_KEY_ID = -1 };

	// Disallows copies.
	VocabDic(const VocabDic &);
	VocabDic &operator=(const VocabDic &);
};

inline Int32 VocabDic::find(const String &key) const
{
	UInt32 hash_id = StringHash()(key) % hash_table_size_;
	for ( ; ; )
	{
		Int32 key_id = hash_table_[hash_id];
		if (hash_table_[hash_id] == INVALID_KEY_ID)
			return -1;
		else if (key == find(key_id))
			return key_id;

		hash_id = (hash_id + 1) % hash_table_size_;
	}
}

inline String VocabDic::find(Int32 key_id) const
{
	if (static_cast<UInt32>(key_id) >= num_keys_)
		return String();

	return String(keys_ + pos_list_[key_id],
		pos_list_[key_id + 1] - pos_list_[key_id]);
}

template <typename InputStream>
inline bool VocabDic::read(InputStream *in)
{
	clear();

	if (in == NULL)
		return false;

	in->read(reinterpret_cast<Int8 *>(&num_keys_), sizeof(num_keys_));
	in->read(reinterpret_cast<Int8 *>(&hash_table_size_),
		sizeof(hash_table_size_));
	in->read(reinterpret_cast<Int8 *>(&total_size_), sizeof(total_size_));

	if (!*in)
		return false;
	else if (num_keys_ == 0 || hash_table_size_ == 0 || total_size_ == 0)
		return false;

	hash_table_buf_.resize(hash_table_size_);
	if (!in->read(reinterpret_cast<Int8 *>(&hash_table_buf_[0]),
		sizeof(hash_table_buf_[0]) * hash_table_buf_.size()))
		return false;

	pos_list_buf_.resize(num_keys_ + 1);
	if (!in->read(reinterpret_cast<Int8 *>(&pos_list_buf_[0]),
		sizeof(pos_list_buf_[0]) * pos_list_buf_.size()))
		return false;

	keys_buf_.resize(pos_list_buf_.back());
	if (!in->read(&keys_buf_[0], sizeof(keys_buf_[0]) * keys_buf_.size()))
		return false;

	hash_table_ = &hash_table_buf_[0];
	pos_list_ = &pos_list_buf_[0];
	keys_ = &keys_buf_[0];

	return true;
}

template <typename OutputStream>
inline bool VocabDic::write(OutputStream *out) const
{
	if (!out->write(reinterpret_cast<const Int8 *>(&num_keys_),
		sizeof(num_keys_)))
		return false;

	if (!out->write(reinterpret_cast<const Int8 *>(&hash_table_size_),
		sizeof(hash_table_size_)))
		return false;

	if (!out->write(reinterpret_cast<const Int8 *>(&total_size_),
		sizeof(total_size_)))
		return false;

	if (!out->write(reinterpret_cast<const Int8 *>(hash_table_),
		sizeof(hash_table_[0]) * hash_table_size_))
		return false;

	if (!out->write(reinterpret_cast<const Int8 *>(pos_list_),
		sizeof(pos_list_[0]) * (num_keys_ + 1)))
		return false;

	if (!out->write(keys_, sizeof(keys_[0]) * pos_list_[num_keys_]))
		return false;

	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_DIC_H
