#ifndef SSGNC_DATA_MERGER_H
#define SSGNC_DATA_MERGER_H

#include "ssgnc/fd-streambuf.h"
#include "ssgnc/tempfile-manager.h"
#include "ssgnc/varint-reader.h"
#include "ssgnc/varint-writer.h"

#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace ssgnc {

class DataMerger
{
public:
	DataMerger() : n_(0), readers_(), queue_() {}

	// Initializes an object.
	void Init(int n, const TempfileManager &tempfile_manager)
	{
		n_ = n;
		readers_.resize(tempfile_manager.size());
		for (std::size_t i = 0; i < readers_.size(); ++i)
			readers_[i].reset(new Reader(tempfile_manager[i]));

		for (size_t i = 0; i < readers_.size(); ++i)
		{
			long long freq;
			if (readers_[i]->Read(&freq))
				queue_.push(std::make_pair(freq, static_cast<int>(i)));
		}
	}

	// Writes a next n-gram.
	bool Next(VarintWriter *writer)
	{
		if (queue_.empty())
			return false;

		PairType pair = queue_.top();
		queue_.pop();

		// Writes a frequency and unigrams.
		writer->Write(pair.first);
		for (int i = 0; i < n_; ++i)
		{
			int key;
			if (!readers_[pair.second]->Read(&key))
				return false;
			writer->Write(key);
		}

		// Reads frequency of the next n-gram.
		if (readers_[pair.second]->Read(&pair.first))
			queue_.push(pair);

		return true;
	}

private:
	typedef std::pair<long long, int> PairType;

	class Reader
	{
	public:
		Reader(int fd) : streambuf_(fd, BUF_SIZE), stream_(&streambuf_),
			byte_reader_(&stream_, BUF_SIZE), reader_(&byte_reader_) {}

		// Reads bytes and decodes those bytes to an unsigned integer.
		template <typename ValueType>
		bool Read(ValueType *value) { return reader_.Read(value); }

	private:
		enum { BUF_SIZE = 1 << 20 };

		FdStreambuf streambuf_;
		std::istream stream_;
		ByteReader byte_reader_;
		VarintReader reader_;

		// Disallows copies.
		Reader(const Reader &);
		Reader &operator=(const Reader &);
	};

	class PairComparer
	{
	public:
		bool operator()(const PairType &lhs, const PairType &rhs) const
		{
			if (lhs.first == rhs.first)
				return lhs.second > rhs.second;
			return lhs.first < rhs.first;
		}
	};

private:
	int n_;
	std::vector<boost::shared_ptr<Reader> > readers_;
	std::priority_queue<PairType, std::vector<PairType>, PairComparer> queue_;

	// Disallows copies.
	DataMerger(const DataMerger &);
	DataMerger &operator=(const DataMerger &);
};

}  // namespace ssgnc

#endif  // SSGNC_DATA_MERGER_H
