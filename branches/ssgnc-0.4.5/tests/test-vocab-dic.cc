#include "ssgnc.h"

#include <algorithm>
#include <cassert>
#include <ctime>
#include <set>
#include <sstream>

int main()
{
	enum { KEY_LENGTH = 16, NUM_KEYS = 1 << 14 };

	std::srand(static_cast<unsigned>(std::time(NULL)));

	ssgnc::MemPool mem_pool;
	std::set<ssgnc::String> keyset;

	char key_buf[KEY_LENGTH + 1] = "";
	ssgnc::String key(key_buf, KEY_LENGTH);

	while (keyset.size() < NUM_KEYS)
	{
		for (int i = 0; i < KEY_LENGTH; ++i)
			key_buf[i] = 'A' + (std::rand() % 26);

		if (keyset.find(key) == keyset.end())
		{
			ssgnc::String key_clone;
			assert(mem_pool.append(key, &key_clone));
			keyset.insert(key_clone);
		}
	}

	std::vector<ssgnc::String> keys(keyset.begin(), keyset.end());
	std::random_shuffle(keys.begin(), keys.end());

	assert(ssgnc::VocabDic::build("VOCAB_DIC", keys));

	ssgnc::VocabDic vocab_dic;

	assert(vocab_dic.num_keys() == 0);
	assert(vocab_dic.table_size() == 0);
	assert(vocab_dic.total_size() == 0);

	assert(vocab_dic.open("VOCAB_DIC"));

	assert(vocab_dic.num_keys() == keys.size());
	assert(vocab_dic.table_size() > keys.size());
	assert(vocab_dic.total_size() ==
		sizeof(ssgnc::Int32) * vocab_dic.table_size()
		+ sizeof(ssgnc::UInt32) * (vocab_dic.num_keys() + 1)
		+ KEY_LENGTH * vocab_dic.num_keys());

	ssgnc::VocabDic vocab_dic_clone;

	assert(vocab_dic_clone.open("VOCAB_DIC", ssgnc::FileMap::READ_FILE));

	assert(vocab_dic.num_keys() == vocab_dic_clone.num_keys());
	assert(vocab_dic.table_size() == vocab_dic_clone.table_size());
	assert(vocab_dic.total_size() == vocab_dic_clone.total_size());

	for (ssgnc::UInt32 i = 0; i < vocab_dic.num_keys(); ++i)
	{
		ssgnc::Int32 key_id = static_cast<ssgnc::Int32>(i);
		assert(vocab_dic.find(key_id, &key));
		assert(key == keys[i]);

		ssgnc::Int32 value;
		assert(vocab_dic.find(key, &value));
		assert(value == key_id);

		key_id = static_cast<ssgnc::Int32>(i);
		assert(vocab_dic_clone.find(key_id, &key));
		assert(key == keys[i]);

		assert(vocab_dic_clone.find(key, &value));
		assert(value == key_id);
	}

	assert(vocab_dic.close());

	assert(vocab_dic.num_keys() == 0);
	assert(vocab_dic.table_size() == 0);
	assert(vocab_dic.total_size() == 0);

	assert(vocab_dic_clone.close());

	assert(vocab_dic_clone.num_keys() == 0);
	assert(vocab_dic_clone.table_size() == 0);
	assert(vocab_dic_clone.total_size() == 0);

	return 0;
}
