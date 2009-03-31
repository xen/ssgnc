#ifndef GOOGLE_NGRAM_UNIT_MANAGER_H
#define GOOGLE_NGRAM_UNIT_MANAGER_H

#include "file-mapper.h"

#include "boost/shared_ptr.hpp"
#include "boost/tuple/tuple.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace ngram
{

class unit_manager
{
public:
	typedef boost::tuple<int, const char *, std::size_t> tuple_type;

	unit_manager(const std::string &input_dir)
		: index_map_(input_dir + "/ngms.idx"), data_maps_(), max_id_(0)
	{
		for (int i = 1; ; ++i)
		{
			std::ostringstream s;
			s << input_dir << '/' << i << "gms.dat";
			boost::shared_ptr<file_mapper> map(new file_mapper(s.str()));
			if (!map->is_open())
				break;
			data_maps_.push_back(map);
		}

		if (max_n())
			max_id_ = index_map_.size() / max_n() - 1;
	}

	// Returns a buffer.
	tuple_type find(int unigram_id, int n) const
	{
		if (unigram_id < min_id() || unigram_id > max_id()
			|| n < min_n() || n > max_n())
			return tuple_type(0, 0, 0);

		const long long *index_pointer = index_map_.pointer<long long>();
		long long begin = index_pointer[unigram_id * max_n() + n - 1];
		long long end = index_pointer[(unigram_id + 1) * max_n() + n - 1];

		const char *data_pointer = data_maps_[n - 1]->pointer<char>();
		return tuple_type(n, data_pointer + begin, end - begin);
	}

	// Returns the range of unigram ID.
	int min_id() const { return 1; }
	int max_id() const { return max_id_; }

	// Returns the range of n.
	int min_n() const { return 1; }
	int max_n() const { return static_cast<int>(data_maps_.size()); }

	// Returns if an object is valid or not.
	bool is_open() const
	{ return index_map_.is_open() && !data_maps_.empty(); }

private:
	const file_mapper index_map_;
	std::vector<boost::shared_ptr<file_mapper> > data_maps_;
	int max_id_;
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_UNIT_MANAGER_H
