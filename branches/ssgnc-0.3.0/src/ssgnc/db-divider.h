#ifndef SSGNC_DB_DIVIDER_H
#define SSGNC_DB_DIVIDER_H

#include "ssgnc/fd-streambuf.h"
#include "ssgnc/tempfile-manager.h"
#include "ssgnc/varint-reader.h"
#include "ssgnc/varint-writer.h"

#include <algorithm>
#include <iostream>
#include <vector>

namespace ssgnc {

class DbDivider
{
public:
	DbDivider() : n_(0), max_num_freqs_(0), freqs_(), keys_(),
		pairs_(), total_count_(0), tempfile_manager_(NULL) {}

	// Calculates maximum number of freqs and allocates memory.
	void Init(int n, int memory_size, TempfileManager *tempfile_manager)
	{
		n_ = n;
		max_num_freqs_ = static_cast<int>(
			(static_cast<long long>(memory_size) << 20)
				/ (sizeof(long long) + sizeof(int) * n
					+ sizeof(PairType) * n));
		freqs_.reserve(max_num_freqs_);
		keys_.reserve(max_num_freqs_ * n);
		pairs_.reserve(max_num_freqs_ * n);

		tempfile_manager_ = tempfile_manager;
	}

	// Clears buffers.
	void Clear()
	{
		std::vector<long long>(0).swap(freqs_);
		std::vector<int>(0).swap(keys_);
		std::vector<PairType>(0).swap(pairs_);
	}

	// Reads an n-gram, indexes it and outputs the result.
	bool Next(VarintReader *reader)
	{
		if (Read(reader))
		{
			if (freqs_.size() < max_num_freqs_)
				return true;
		}
		else if (freqs_.empty())
			return false;

		std::stable_sort(pairs_.begin(), pairs_.end(), PairComparer());
		pairs_.erase(std::unique(pairs_.begin(), pairs_.end()), pairs_.end());

		bool result = Flush();
		freqs_.clear();
		keys_.clear();
		pairs_.clear();
		return result;
	}

	// Returns the number of n-grams processed.
	std::size_t total_count() const { return total_count_; }

private:
	typedef std::pair<int, int> PairType;

	class PairComparer
	{
	public:
		bool operator()(const PairType &lhs, const PairType &rhs) const
		{
			return lhs.first < rhs.first;
		}
	};

private:
	int n_;
	std::size_t max_num_freqs_;
	std::vector<long long> freqs_;
	std::vector<int> keys_;
	std::vector<PairType> pairs_;
	std::size_t total_count_;
	TempfileManager *tempfile_manager_;

	// Disallows copies.
	DbDivider(const DbDivider &);
	DbDivider &operator=(const DbDivider &);

	// Reads an n-gram.
	bool Read(VarintReader *reader)
	{
		// Reads a frequency.
		long long freq;
		if (!reader->Read(&freq))
			return false;
		freqs_.push_back(freq);

		// Reads keys.
		for (int i = 0; i < n_; ++i)
		{
			int key;
			if (!reader->Read(&key))
				break;
			keys_.push_back(key);
			pairs_.push_back(std::make_pair(key,
				static_cast<int>(freqs_.size() - 1)));
		}
		if (keys_.size() != freqs_.size() * n_)
			return false;

		++total_count_;
		return true;
	}

	// Flushes an internal buffer.
	bool Flush()
	{
		int fd = tempfile_manager_->Add();
		if (fd == -1)
			return false;

		FdStreambuf temp_file(fd);
		std::ostream output(&temp_file);
		ByteWriter byte_writer(&output);
		VarintWriter writer(&byte_writer);
		for (std::vector<PairType>::const_iterator it = pairs_.begin();
			it != pairs_.end(); ++it)
		{
			writer.Write(it->first);
			writer.Write(freqs_[it->second]);
			for (int i = 0; i < n_; ++i)
				writer.Write(keys_[it->second * n_ + i]);
		}
		return true;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_DB_DIVIDER_H
