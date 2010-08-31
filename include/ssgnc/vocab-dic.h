#ifndef SSGNC_VOCAB_DIC_H
#define SSGNC_VOCAB_DIC_H

#include "string-hash.h"
#include "file-map.h"

namespace ssgnc {

class VocabDic
{
public:
	VocabDic();
	~VocabDic() { clear(); }

	void clear();

	UInt32 num_keys() const { return num_keys_; }
	UInt32 table_size() const { return table_size_; }
	UInt32 total_size() const { return total_size_; }

	bool find(const String &key, Int32 *key_id) const
		SSGNC_WARN_UNUSED_RESULT;
	bool find(Int32 key_id, String *key) const SSGNC_WARN_UNUSED_RESULT;

	bool build(const String *keys, UInt32 num_keys) SSGNC_WARN_UNUSED_RESULT;

	bool load(const Int8 *path) SSGNC_WARN_UNUSED_RESULT;
	bool read(std::istream *in) SSGNC_WARN_UNUSED_RESULT;

	bool mmap(const Int8 *path) SSGNC_WARN_UNUSED_RESULT;
	bool map(const void *ptr, UInt32 size) SSGNC_WARN_UNUSED_RESULT;

	bool save(const Int8 *path) const SSGNC_WARN_UNUSED_RESULT;
	bool write(std::ostream *out) const SSGNC_WARN_UNUSED_RESULT;

	enum { INVALID_KEY_ID = -1 };

private:
	UInt32 num_keys_;
	UInt32 table_size_;
	UInt32 total_size_;
	const Int32 *table_;
	const UInt32 *offsets_;
	const Int8 *keys_;
	std::vector<Int32> table_buf_;
	std::vector<UInt32> offsets_buf_;
	std::vector<Int8> keys_buf_;
	FileMap file_map_;

	String restoreKey(Int32 key_id) const;

	bool readData(std::istream *in) SSGNC_WARN_UNUSED_RESULT;
	bool mapData(const void *ptr, UInt32 size) SSGNC_WARN_UNUSED_RESULT;

	// Disallows copies.
	VocabDic(const VocabDic &);
	VocabDic &operator=(const VocabDic &);
};

inline bool VocabDic::find(Int32 key_id, String *key) const
{
	if (static_cast<UInt32>(key_id) >= num_keys_)
	{
		SSGNC_ERROR << "Out of range key ID: " << key_id << std::endl;
		return false;
	}
	else if (key == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	*key = restoreKey(key_id);
	return true;
}

inline String VocabDic::restoreKey(Int32 key_id) const
{
	return String(keys_ + offsets_[key_id],
		offsets_[key_id + 1] - offsets_[key_id]);
}

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_DIC_H
