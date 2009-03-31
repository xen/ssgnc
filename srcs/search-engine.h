#ifndef GOOGLE_NGRAM_SEARCH_ENGINE_H
#define GOOGLE_NGRAM_SEARCH_ENGINE_H

#include "file-mapper.h"
#include "unit-manager.h"
#include "unit-reader.h"
#include "vocab-table.h"

#include "darts/darts-clone.h"

#include <climits>
#include <queue>
#include <set>
#include <string>

namespace ngram
{

// For storing data required for searching n-grams.
class search_engine
{
public:
	class query_type;
	class result_type;
	class reader_type;

	search_engine(const std::string &input_dir) : manager_(input_dir),
		table_(input_dir + "/vocab"), da_map_(input_dir + "/vocab.da"), da_()
	{
		if (da_map_.is_open())
			da_.set_array(da_map_.pointer<void>(), da_map_.size());
	}

	// Checks if an object is available or not.
	bool is_open() const
	{
		return manager_.is_open() && table_.is_open()
			&& da_map_.is_open() && (da_.size() > 0);
	}

private:
	const unit_manager manager_;
	const vocab_table table_;
	const file_mapper da_map_;
	Darts::DoubleArray da_;

	// Copies are not allowed.
	search_engine(const search_engine &);
	search_engine &operator=(const search_engine &);
};

// Query.
class search_engine::query_type
{
public:
	typedef std::vector<std::string>::const_iterator unigram_iterator;
	typedef std::set<int>::const_iterator n_iterator;

	enum order_type { UNORDERED, ORDERED, PHRASE, FIXED };

	query_type() : min_freq_(0), unigrams_(), order_(UNORDERED), ns_() {}

	void clear_unigram() { unigrams_.clear(); }
	void clear_n() { ns_.clear(); }

	void set_min_freq(long long min_freq)
	{ min_freq_ = (min_freq > 0) ? min_freq : 1; }
	void add_unigram(const std::string &unigram)
	{ unigrams_.push_back(unigram); }
	void set_order(order_type order) { order_ = order; }
	void add_n(int n) { if (n > 0) ns_.insert(n); }

	long long min_freq() const { return min_freq_; }
	const std::string &unigram(int id) const { return unigrams_[id]; }
	order_type order() const { return order_; }

	int unigram_size() const { return static_cast<int>(unigrams_.size()); }
	int n_size() const { return static_cast<int>(ns_.size()); }

	unigram_iterator unigram_begin() const { return unigrams_.begin(); }
	unigram_iterator unigram_end() const { return unigrams_.end(); }

	n_iterator n_begin() const { return ns_.begin(); }
	n_iterator n_end() const { return ns_.end(); }

private:
	long long min_freq_;
	std::vector<std::string> unigrams_;
	order_type order_;
	std::set<int> ns_;
};

// One search result.
class search_engine::result_type
{
public:
	result_type() : freq_(0), unigrams_() {}

	void clear() { unigrams_.clear(); }

	void set_freq(long long freq) { freq_ = freq; }
	void add_unigram(const char *unigram) { unigrams_.push_back(unigram); }

	// Returns a unigram.
	const char * const &operator[](int index) const
	{ return unigrams_[index]; }
	// Returns the number of unigrams.
	int size() const { return unigrams_.size(); }

	// Returns a frequency.
	long long freq() const { return freq_; }
	// Returns a list of unigrams.
	const std::vector<const char *> &unigrams() const { return unigrams_; }

private:
	long long freq_;
	std::vector<const char *> unigrams_;
};

class search_engine::reader_type
{
public:
	// For sorting units.
	class unit_comparer
	{
	public:
		bool operator()(const unit_type *lhs, const unit_type *rhs) const
		{
			if (lhs->freq() == rhs->freq())
				return lhs->size() > rhs->size();
			return lhs->freq() < rhs->freq();
		}
	};
	typedef std::priority_queue<unit_type *,
		std::vector<unit_type *>, unit_comparer> queue_type;

	// Creates an object to read results.
	reader_type(const search_engine &engine, const query_type &query)
		: table_(&engine.table_), query_(query), unigrams_(),
		readers_(), units_(), queue_()
	{
		convert_unigrams(engine, query);
		init_readers(engine, query);

		// Reads the first unit for each n.
		units_.resize(readers_.size());
		for (size_t i = 0; i < readers_.size(); ++i)
		{
			if (readers_[i]->read(&units_[i])
				&& units_[i].freq() >= query_.min_freq())
				queue_.push(&units_[i]);
		}
	}

	// Reads the next result.
	bool read(result_type *result)
	{
		while (!queue_.empty())
		{
			// Pops a unit which has the largest frequency.
			unit_type *unit = queue_.top();
			queue_.pop();

			// Compares a query and a unit.
			bool is_ok = match(*unit);
			if (is_ok)
			{
				result->clear();
				result->set_freq(unit->freq());
				for (int i = 0; i < unit->size(); ++i)
					result->add_unigram((*table_)[(*unit)[i]]);
			}

			// Reads the next unit.
			if (readers_[unit - &units_[0]]->read(unit)
				&& unit->freq() >= query_.min_freq())
				queue_.push(unit);

			if (is_ok)
				return true;
		}
		return false;
	}

private:
	const vocab_table *table_;
	query_type query_;
	std::vector<int> unigrams_;
	std::vector<boost::shared_ptr<unit_reader> > readers_;
	std::vector<unit_type> units_;
	queue_type queue_;

	// Converts from unigram strings to unigram IDs.
	void convert_unigrams(const search_engine &engine, const query_type &query)
	{
		const Darts::DoubleArray &da = engine.da_;

		for (int i = 0; i < query.unigram_size(); ++i)
		{
			// Skips an empty unigram.
			if (query.unigram(i).empty())
			{
				if (query.order() == search_engine::query_type::FIXED)
					unigrams_.push_back(0);
				continue;
			}

			int unigram_id;
			da.exactMatchSearch(query.unigram(i).c_str(), unigram_id);
			unigrams_.push_back(unigram_id);
		}
	}

	// Initializes readers.
	void init_readers(const search_engine &engine, const query_type &query)
	{
		const unit_manager &manager = engine.manager_;

		// Checks if a query contains an available unigram.
		bool contains_non_empty = false;
		for (int i = 0; i < query.unigram_size(); ++i)
		{
			if (!query.unigram(i).empty())
			{
				contains_non_empty = true;
				break;
			}
		}
		if (!contains_non_empty)
			return;

		if (query.order() == search_engine::query_type::FIXED)
			add_reader(engine, query, unigrams_.size());
		else if (!query.n_size())
		{
			// Uses all n-grams.
			for (int i = manager.min_n(); i <= manager.max_n(); ++i)
				add_reader(engine, query, i);
		}
		else
		{
			// Uses specified n-grams.
			for (query_type::n_iterator it = query.n_begin();
				it != query.n_end(); ++it)
				add_reader(engine, query, *it);
		}
	}

	// Adds a reader.
	void add_reader(const search_engine &engine,
		const query_type &query, int n)
	{
		const unit_manager &manager = engine.manager_;

		// Avoids no use readers.
		if (static_cast<std::size_t>(n) < unigrams_.size())
			return;

		// Chooses the uniquest unigram.
		unit_manager::tuple_type tuple(0, 0, INT_MAX);
		for (std::size_t i = 0; i < unigrams_.size(); ++i)
		{
			// Skips an empty unigram.
			if (!unigrams_[i])
				continue;

			unit_manager::tuple_type temp_tuple(manager.find(unigrams_[i], n));
			if (temp_tuple.get<2>() < tuple.get<2>())
				tuple = temp_tuple;
		}

		boost::shared_ptr<ngram::unit_reader> reader(
			new ngram::unit_reader(tuple));
		readers_.push_back(reader);
	}

	// Checks if a unit matches a query or not.
	bool match(const unit_type &unit)
	{
		switch (query_.order())
		{
		case search_engine::query_type::UNORDERED:
			return match_unordered_query(unit);
		case search_engine::query_type::ORDERED:
			return match_ordered_query(unit);
		case search_engine::query_type::PHRASE:
			return match_phrase_query(unit);
		case search_engine::query_type::FIXED:
			return match_fixed_query(unit);
		}
		return false;
	}

	// Checks if a unit matches a query or not.
	bool match_unordered_query(const unit_type &unit)
	{
		unsigned flags = (1U << unigrams_.size()) - 1;
		for (int i = 0; flags && i < unit.size(); ++i)
		{
			for (size_t j = 0; j < unigrams_.size(); ++j)
			{
				if (!(flags & (1U << j)))
					continue;

				if (unit[i] == unigrams_[j])
				{
					flags &= ~(1U << j);
					break;
				}
			}
		}
		return flags == 0;
	}

	// Checks if a unit matches a query or not.
	bool match_ordered_query(const unit_type &unit)
	{
		int unit_id = 0;
		for (std::size_t i = 0; i < unigrams_.size(); ++i, ++unit_id)
		{
			for ( ; unit_id < unit.size(); ++unit_id)
			{
				if (unit[unit_id] == unigrams_[i])
					break;
			}
			if (unit_id >= unit.size())
				return false;
		}
		return true;
	}

	// Checks if a unit matches a query or not.
	bool match_phrase_query(const unit_type &unit)
	{
		int max = unit.size() - unigrams_.size();
		for (int i = 0; i <= max; ++i)
		{
			if (unit[i] != unigrams_[0])
				continue;

			bool not_equal = false;
			for (std::size_t j = 1; j < unigrams_.size(); ++j)
			{
				not_equal = (unit[i + j] != unigrams_[j]);
				if (not_equal)
					break;
			}
			if (!not_equal)
				return true;
		}
		return false;
	}

	// Checks if a unit matches a query or not.
	bool match_fixed_query(const unit_type &unit)
	{
		for (std::size_t i = 0; i < unigrams_.size(); ++i)
		{
			if (unigrams_[i] > 0 && unigrams_[i] != unit[i])
				return false;
		}
		return true;
	}
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_SEARCH_ENGINE_H
