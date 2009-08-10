#ifndef SSGNC_DATA_DIVIDER_H
#define SSGNC_DATA_DIVIDER_H

#include "ssgnc/fd-streambuf.h"
#include "ssgnc/tempfile-manager.h"
#include "ssgnc/varint-reader.h"
#include "ssgnc/varint-writer.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace ssgnc {

class DataDivider
{
public:
	DataDivider() : n_(0), max_num_pairs_(0), keys_(), pairs_(),
		total_count_(0), tempfile_manager_(NULL) {}

	// Calculates maximum number of pairs and allocates memory.
	void Init(int n, int memory_size, TempfileManager *tempfile_manager)
	{
		n_ = n;
		max_num_pairs_ = static_cast<int>(
			(static_cast<long long>(memory_size) << 20)
			/ (sizeof(int) * n + sizeof(PairType)));
		keys_.reserve(max_num_pairs_ * n);
		pairs_.reserve(max_num_pairs_);
		total_count_ = 0;

		tempfile_manager_ = tempfile_manager;
	}

	// Clears buffers.
	void Clear()
	{
		std::vector<int>(0).swap(keys_);
		std::vector<PairType>(0).swap(pairs_);
	}

	// Reads an n-gram, sorts it and outputs the result.
	bool Next(VarintReader *reader)
	{
		if (Read(reader))
		{
			if (pairs_.size() < max_num_pairs_)
				return true;
		}
		else if (pairs_.empty())
			return false;

		std::stable_sort(pairs_.begin(), pairs_.end(), PairComparer());

		bool result = Flush();
		keys_.clear();
		pairs_.clear();
		return result;
	}

	// Returns the number of n-grams processed.
	std::size_t total_count() const { return total_count_; }

private:
	typedef std::pair<long long, int> PairType;

	class PairComparer
	{
	public:
		bool operator()(const PairType &lhs, const PairType &rhs) const
		{
			return lhs.first > rhs.first;
		}
	};

private:
	int n_;
	std::size_t max_num_pairs_;
	std::vector<int> keys_;
	std::vector<PairType> pairs_;
	std::size_t total_count_;
	TempfileManager *tempfile_manager_;

	// Disallows copies.
	DataDivider(const DataDivider &);
	DataDivider &operator=(const DataDivider &);

	// Reads an n-gram.
	bool Read(VarintReader *reader)
	{	
		// Reads keys.
		for (int i = 0; i < n_; ++i)
		{
			int key;
			if (!reader->Read(&key))
				break;
			keys_.push_back(key);
		}
		if (keys_.size() != (pairs_.size() + 1) * n_)
			return false;

		// Reads a frequency.
		PairType pair;
		if (!reader->Read(&pair.first))
			return false;
		pair.second = pairs_.size();
		pairs_.push_back(pair);
		++total_count_;

		return true;
	}

	// Flushes a buffer.
	bool Flush()
	{
		int fd = tempfile_manager_->Add();
		if (fd == -1)
			return false;

		FdStreambuf temp_file(fd);
		std::ostream output(&temp_file);
		ByteWriter byte_writer(&output);
		VarintWriter writer(&byte_writer);
		for (std::size_t i = 0; i < pairs_.size(); ++i)
		{
			writer.Write(pairs_[i].first);
			for (int j = 0; j < n_; ++j)
				writer.Write(keys_[pairs_[i].second * n_ + j]);
		}
		return true;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_DATA_DIVIDER_H
