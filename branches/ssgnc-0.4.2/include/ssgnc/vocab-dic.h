#ifndef SSGNC_VOCAB_DIC_H
#define SSGNC_VOCAB_DIC_H

#include "string-hash.h"
#include "file-map.h"

namespace ssgnc {

class VocabDic
{
public:
	VocabDic();
	~VocabDic();

	bool open(const Int8 *path, FileMap::Mode mode = FileMap::DEFAULT_MODE)
		SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool find(const String &key, Int32 *key_id) const
		SSGNC_WARN_UNUSED_RESULT;
	bool find(Int32 key_id, String *key) const SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return file_map_.is_open(); }

	UInt32 num_keys() const { return num_keys_; }
	UInt32 table_size() const { return table_size_; }
	UInt32 total_size() const { return total_size_; }

	static bool build(const Int8 *path, const std::vector<String> &keys)
		SSGNC_WARN_UNUSED_RESULT;

	enum { INVALID_KEY_ID = -1 };

private:
	UInt32 num_keys_;
	UInt32 table_size_;
	UInt32 total_size_;
	const Int32 *table_;
	const UInt32 *offsets_;
	const Int8 *keys_;
	FileMap file_map_;

	String restoreKey(Int32 key_id) const;

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
