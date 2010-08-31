#ifndef SSGNC_DB_FILTER_H
#define SSGNC_DB_FILTER_H

#include "db-reader.h"
#include "inverted-db.h"
#include "query.h"

#include <limits>

namespace ssgnc {

class DbFilter
{
public:
	DbFilter() : reader_() {}

	// Prepares a filter to read n-grams.
	bool Open(const InvertedDb &db, const Query &query)
	{
		// Checks the number of keys in a query.
		if (!IsPossiblePair(db, query))
			return false;

		if (!FindMinimumReader(db, query))
			return false;

		return true;
	}

	// Reads an n-gram that satisfies a query.
	bool Read(const Query &query, Ngram *ngram)
	{
		while (reader_.Read(ngram))
		{
			if (ngram->freq() < query.min_freq())
				break;

			if (Filter(query, *ngram))
				return true;
		}
		return false;
	}

private:
	DbReader reader_;

	// Disallows copies.
	DbFilter(const DbFilter &);
	DbFilter &operator=(const DbFilter &);

	// Finds the best reader which has the minimum size.
	bool FindMinimumReader(const InvertedDb &db, const Query &query)
	{
		int min_key_id = 0;
		std::size_t min_size = std::numeric_limits<std::size_t>::max();
		for (int i = 0; i < query.key_id_size(); ++i)
		{
			if (query.key_id(i) < 0)
				continue;

			std::size_t size;
			if (db.FindSize(query.key_id(i), &size))
			{
				if (size < min_size)
				{
					min_key_id = query.key_id(i);
					min_size = size;
				}
			}
		}
		if (min_size == 0)
			return false;

		return db.Find(min_key_id, &reader_);
	}

	// Checks if a database may have n-grams which satisfy a query.
	static bool IsPossiblePair(const InvertedDb &db, const Query &query)
	{
		if (query.key_id_size() <= 0)
			return false;
		else if (query.order() == Query::FIXED)
		{
			if (query.key_id_size() != db.n())
				return false;
		}
		else if (query.key_id_size() > db.n())
			return false;

		return true;
	}

	// Filters an n-gram by using a query.
	// Returns true if an n-gram satisfies a query.
	static bool Filter(const Query &query, const Ngram &ngram)
	{
		switch (query.order())
		{
		case Query::UNORDERED:
			return FilterUnordered(query, ngram);
		case Query::ORDERED:
			return FilterOrdered(query, ngram);
		case Query::PHRASE:
			return FilterPhrase(query, ngram);
		case Query::FIXED:
			return FilterFixed(query, ngram);
		}
		return false;
	}
	static bool FilterUnordered(const Query &query, const Ngram &ngram)
	{
		unsigned mask = (1U << query.key_id_size()) - 1;
		for (int i = 0; mask != 0 && i < ngram.key_id_size(); ++i)
		{
			for (int j = 0; j < query.key_id_size(); ++j)
			{
				if (!(mask & (1U << j)))
					continue;

				if (ngram.key_id(i) == query.key_id(j))
				{
					mask &= ~(1U << j);
					break;
				}
			}
		}
		return mask == 0;
	}
	static bool FilterOrdered(const Query &query, const Ngram &ngram)
	{
		for (int query_id = 0, ngram_id = 0; query_id < query.key_id_size();
			++query_id, ++ngram_id)
		{
			for ( ; ngram_id < ngram.key_id_size(); ++ngram_id)
			{
				if (ngram.key_id(ngram_id) == query.key_id(query_id))
					break;
			}
			if (ngram_id >= ngram.key_id_size())
				return false;
		}
		return true;
	}
	static bool FilterPhrase(const Query &query, const Ngram &ngram)
	{
		int max_key_id = ngram.key_id_size() - query.key_id_size();
		for (int i = 0; i <= max_key_id; ++i)
		{
			bool contains_mismatch = false;
			for (int j = 0; j < query.key_id_size(); ++j)
			{
				contains_mismatch = (ngram.key_id(i + j) != query.key_id(j));
				if (contains_mismatch)
					break;
			}
			if (!contains_mismatch)
				return true;
		}
		return false;
	}
	static bool FilterFixed(const Query &query, const Ngram &ngram)
	{
		if (query.key_id_size() != ngram.key_id_size())
			return false;
		for (int i = 0; i < query.key_id_size(); ++i)
		{
			if (query.key_id(i) >= 0 && query.key_id(i) != ngram.key_id(i))
				return false;
		}
		return true;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_DB_FILTER_H
