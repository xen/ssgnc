#ifndef SSGNC_UNIFIED_READER_H
#define SSGNC_UNIFIED_READER_H

#include "db-filter.h"
#include "unified-db.h"

#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace ssgnc {

class UnifiedReader
{
public:
	UnifiedReader() : query_(), filters_(), ngrams_(), queue_() {}

	const Query &query() const { return query_; }

	// Clears a query and filters.
	void Clear()
	{
		query_.Clear();
		filters_.clear();
		ngrams_.clear();
		while (!queue_.empty())
			queue_.pop();
	}

	// Opens filters.
	bool Open(const UnifiedDb &db, const Query &query)
	{
		Clear();
		query_ = query;
		for (int i = 0; i < db.db_size(); ++i)
		{
			Ngram ngram;
			boost::shared_ptr<DbFilter> filter(new DbFilter);
			if (filter->Open(db.db(i), query))
			{
				if (filter->Read(query, &ngram))
					ngrams_.push_back(ngram);
				filters_.push_back(filter);
			}
		}
		for (std::size_t i = 0; i < ngrams_.size(); ++i)
			queue_.push(std::make_pair(&ngrams_[i], i));
		return !queue_.empty();
	}

	// Reads an n-gram in descending frequency order.
	bool Read(Ngram *ngram)
	{
		if (queue_.empty())
			return false;

		Pair pair = queue_.top();
		queue_.pop();

		*ngram = *pair.first;
		if (filters_[pair.second]->Read(query_, pair.first))
			queue_.push(pair);
		return true;
	}

private:
	typedef std::pair<Ngram *, int> Pair;

	class PairComparer
	{
	public:
		bool operator()(const Pair &lhs, const Pair &rhs) const
		{
			if (lhs.first->freq() == rhs.first->freq())
				return lhs.second > rhs.second;
			return lhs.first->freq() < rhs.first->freq();
		}
	};

	Query query_;
	std::vector<boost::shared_ptr<DbFilter> > filters_;
	std::vector<Ngram> ngrams_;
	std::priority_queue<Pair, std::vector<Pair>, PairComparer> queue_;

	// Disallows copies.
	UnifiedReader(const UnifiedReader &);
	UnifiedReader &operator=(const UnifiedReader &);
};

}  // namespace ssgnc

#endif  // SSGNC_UNIFIED_READER_H
