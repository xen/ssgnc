#ifndef SSGNC_VOCAB_DIC_H
#define SSGNC_VOCAB_DIC_H

//#include "query.h"

#include <ssgnc/darts.h>

#include <memory>

namespace ssgnc {

class VocabDic
{
public:
	VocabDic() : dic_(), num_keys_(0) {}
	~VocabDic() { Clear(); }

//	// Fills key IDs in a query.
//	bool FillQuery(Query *query) const
//	{
//		query->clear_key_id();
//		for (int i = 0; i < query->key_string_size(); ++i)
//		{
//			if (query->order() == Query::FIXED && query->key_string(i).empty())
//				query->add_key_id(-1);
//			else
//			{
//				int key_id = -1;
//				if (!Find(query->key_string(i).c_str(), &key_id))
//					return false;
//				query->add_key_id(key_id);
//			}
//		}
//		return true;
//	}

	std::size_t num_keys() const { return num_keys_; }
	std::size_t total_size() const { return dic_->total_size(); }

	bool Find(const char *key, int *key_ptr) const;
	bool Find(const char *key, std::size_t length, int *key_ptr) const;

	void Open(const char *dic_file_name);
	void Close();
	void Map(const void *address);

	void Clear() { Close(); }
	void Swap(VocabDic *target);

private:
	std::auto_ptr<Darts::DoubleArray> dic_;
	std::size_t num_keys_;

	// Disallows copies.
	VocabDic(const VocabDic &);
	VocabDic &operator=(const VocabDic &);
};

inline bool VocabDic::Find(const char *key, int *key_ptr) const
{
	dic_->exactMatchSearch(key, *key_ptr);
	return *key_ptr >= 0;
}

inline bool VocabDic::Find(const char *key, std::size_t length,
	int *key_ptr) const
{
	dic_->exactMatchSearch(key, *key_ptr, length);
	return *key_ptr >= 0;
}

}  // namespace ssgnc

#endif  // SSGNC_VOCAB_DIC_H
