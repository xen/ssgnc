#ifndef SSGNC_DB_MERGER_H
#define SSGNC_DB_MERGER_H

#include "ssgnc/fd-streambuf.h"
#include "ssgnc/tempfile-manager.h"
#include "ssgnc/varint-reader.h"
#include "ssgnc/varint-writer.h"

#include <queue>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace ssgnc {

class DbMerger
{
public:
	DbMerger() : n_(0), last_key_(-1), last_freq_(0),
		readers_(), positions_(), queue_() {}

	// Initializes an object.
	void Init(int n, const TempfileManager &tempfile_manager)
	{
		n_ = n;
		readers_.resize(tempfile_manager.size());
		for (std::size_t i = 0; i < readers_.size(); ++i)
			readers_[i].reset(new Reader(tempfile_manager[i]));

		for (size_t i = 0; i < readers_.size(); ++i)
		{
			int key;
			if (readers_[i]->Read(&key))
				queue_.push(std::make_pair(key, static_cast<int>(i)));
		}
	}

	// Writes a next n-gram.
	bool Next(ByteWriter *byte_writer)
	{
		if (queue_.empty())
			return false;

		PairType pair = queue_.top();
		queue_.pop();

		for ( ; last_key_ < pair.first; ++last_key_)
		{
			std::size_t position = byte_writer->total();
			positions_.push_back(static_cast<long long>(position));
			last_freq_ = 0;
		}

		// Copies a frequency.
		long long freq;
		if (!readers_[pair.second]->Read(&freq))
			return false;
		if (last_freq_ != 0)
		{
			freq = last_freq_ - freq;
			last_freq_ -= freq;
		}
		else
			last_freq_ = freq;
		VarintWriter writer(byte_writer);
		writer.Write(freq);

		// Copies keys.
		for (int i = 0; i < n_; ++i)
		{
			int key;
			if (!readers_[pair.second]->Read(&key))
				return false;
			writer.Write(key);
		}

		// Reads a key of the next n-gram.
		if (readers_[pair.second]->Read(&pair.first))
			queue_.push(pair);

		return true;
	}

	// Finishes merging.
	void Finish(ByteWriter *byte_writer)
	{
		std::size_t position = byte_writer->total();
		positions_.push_back(static_cast<long long>(position));

		// Padding for alinment.
		while ((byte_writer->total() % sizeof(long long)) != 0)
			byte_writer->Write('\0');

		for (std::size_t i = 0; i < positions_.size(); ++i)
		{
			const char *p = reinterpret_cast<const char *>(&positions_[i]);
			for (std::size_t j = 0; j < sizeof(positions_[i]); ++j)
				byte_writer->Write(p[j]);
		}

		int size = static_cast<int>(positions_.size());
		const char *q = reinterpret_cast<const char *>(&size);
		for (std::size_t i = 0; i < sizeof(size); ++i)
			byte_writer->Write(q[i]);
	}

private:
	typedef std::pair<int, int> PairType;

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
			return lhs.first > rhs.first;
		}
	};

private:
	int n_;
	int last_key_;
	long long last_freq_;
	std::vector<boost::shared_ptr<Reader> > readers_;
	std::vector<long long> positions_;
	std::priority_queue<PairType, std::vector<PairType>, PairComparer> queue_;

	// Disallows copies.
	DbMerger(const DbMerger &);
	DbMerger &operator=(const DbMerger &);
};

}  // namespace ssgnc

#endif  // SSGNC_DB_MERGER_H
