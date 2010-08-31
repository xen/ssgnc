#ifndef SSGNC_BACK_MERGER_H
#define SSGNC_BACK_MERGER_H

#include <ssgnc/parsed-line-reader.h>
#include <ssgnc/temp-file.h>

#include <memory>
#include <queue>

namespace ssgnc {

class BackMerger
{
public:
	BackMerger() : readers_(), queue_(), top_(NULL), bytes_(), length_(0) {}
	~BackMerger() { Clear(); }

	int key() const;
	long long value() const { return top_->parsed_line().value(); }

	const char *bytes() const { return bytes_; }
	std::size_t length() const { return length_; }

	void Open(TempFile *file, const std::vector<std::size_t> &points);
	void Close();

	bool Next();

	void Clear() { Close(); }
	void Swap(BackMerger *target);

public:
	class KeyComparer
	{
	public:
		bool operator()(const ParsedLineReader *lhs,
			const ParsedLineReader *rhs) const;
	};

private:
	typedef std::priority_queue<ParsedLineReader *,
		std::vector<ParsedLineReader *>, KeyComparer> PriorityQueue;

	enum { MAX_NUM_KEYS = 32 };
	enum { ENCODER_BUF_SIZE = MAX_NUM_KEYS * 8 };

	std::vector<ParsedLineReader *> readers_;
	std::auto_ptr<PriorityQueue> queue_;
	ParsedLineReader *top_;
	char bytes_[ENCODER_BUF_SIZE];
	std::size_t length_;

	// Disallows copies.
	BackMerger(const BackMerger &);
	BackMerger &operator=(const BackMerger &);

	void Encode(int last_key, long long last_value);
};

inline int BackMerger::key() const
{
	return top_->parsed_line().key(top_->parsed_line().num_keys() - 1);
}

inline bool BackMerger::KeyComparer::operator()(
	const ParsedLineReader *lhs, const ParsedLineReader *rhs) const
{
	int lhs_key = lhs->parsed_line().key(lhs->parsed_line().num_keys() - 1);
	int rhs_key = rhs->parsed_line().key(rhs->parsed_line().num_keys() - 1);
	if (lhs_key != rhs_key)
		return lhs_key > rhs_key;
	return lhs->file_cur() > rhs->file_cur();
}

}  // namespace ssgnc

#endif  // SSGNC_BACK_MERGER_H
