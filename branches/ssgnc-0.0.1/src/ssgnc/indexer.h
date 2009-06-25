#ifndef SSGNC_INDEXER_H
#define SSGNC_INDEXER_H

#include "vocab-dic.h"

#include <cstring>
#include <iostream>

namespace ssgnc {

class Indexer
{
public:
	Indexer() : dic_(), address_(0) {}

	// Reads a dictionary.
	bool ReadDic(std::istream *input)
	{
		return dic_.ReadDic(input);
	}

	// Builds an index for a line.
	bool Index(const char *line, std::size_t length, std::ostream *output)
	{
		bool is_ok = IndexLine(line, output);
		address_ += length + 1;
		return is_ok;
	}

private:
	VocabDic dic_;
	std::size_t address_;

	// Disallows copies.
	Indexer(const Indexer &);
	Indexer &operator=(const Indexer &);

private:
	// Builds an index for a line.
	bool IndexLine(const char *line, std::ostream *output) const
	{
		bool is_end = false;
		while (!is_end)
		{
			// Extracts a key.
			const char *delim = std::strchr(line, ' ');
			if (delim == NULL)
			{
				delim = std::strchr(line, '\t');
				if (delim == NULL)
					return false;
				is_end = true;
			}

			// Lookups a dictionary.
			int key_id;
			std::size_t length = static_cast<std::size_t>(delim - line);
			if (!dic_.Find(line, length, &key_id))
				return false;
			*output << key_id << ' ' << address_ << '\n';

			line = delim + 1;
		}
		return true;
	}
};

}  // ssgnc

#endif  // SSGNC_INDEXER_H
