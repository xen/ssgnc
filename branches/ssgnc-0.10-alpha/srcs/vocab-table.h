#ifndef GOOGLE_NGRAM_VOCAB_TABLE_H_
#define GOOGLE_NGRAM_VOCAB_TABLE_H_

#include "file-mapper.h"

#include <string>

#include <iostream>

namespace ngram
{

class vocab_table
{
public:
	// Opens files for a vocabulary table.
	explicit vocab_table(const std::string &vocab_prefix)
		: index_map_(vocab_prefix + ".idx"),
		data_map_(vocab_prefix + ".dat") {}
	// Opens files for a vocabulary table.
	vocab_table(const std::string &index_file_name,
		const std::string &data_file_name)
		: index_map_(index_file_name), data_map_(data_file_name) {}

	// Gets a unigram whose id equals to a given id.
	const char *operator[](int id) const
	{
		return data_map_.pointer<char>()
			+ index_map_.pointer<int>()[id - 1];
	}

	// Returns if mappings are valid or not.
	bool is_open() const
	{ return index_map_.is_open() && data_map_.is_open(); }

	// Returns the number of unigrams.
	int size() const { return index_map_.size() / sizeof(int) - 1; }
	int min_id() const { return 1; }
	int max_id() const { return size(); }

private:
	// Mapping for an index file.
	const file_mapper index_map_;
	// Mapping for a data file.
	const file_mapper data_map_;

	// Copies are not allowed.
	vocab_table(const vocab_table &);
	vocab_table &operator=(const vocab_table &);
};

}  // namespace ngram

#endif // GOOGLE_NGRAM_VOCAB_TABLE_H_
