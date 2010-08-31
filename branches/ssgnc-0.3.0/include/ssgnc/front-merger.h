#ifndef SSGNC_FRONT_MERGER_H
#define SSGNC_FRONT_MERGER_H

#include <ssgnc/parsed-line-reader.h>
#include <ssgnc/temp-file.h>

#include <memory>
#include <queue>

namespace ssgnc {

class FrontMerger
{
public:
	FrontMerger() : readers_(), queue_(), top_(NULL) {}
	~FrontMerger() { Clear(); }

	const ParsedLine &parsed_line() const { return top_->parsed_line(); }
	const char *bytes() const { return top_->bytes(); }
	std::size_t length() const { return top_->length(); }

	void Open(TempFile *file, const std::vector<std::size_t> &points);
	void Close();

	bool Next();

	void Clear() { Close(); }
	void Swap(FrontMerger *target);

public:
	class ValueComparer
	{
	public:
		bool operator()(const ParsedLineReader *lhs,
			const ParsedLineReader *rhs) const;
	};

private:
	typedef std::priority_queue<ParsedLineReader *,
		std::vector<ParsedLineReader *>, ValueComparer> PriorityQueue;

	std::vector<ParsedLineReader *> readers_;
	std::auto_ptr<PriorityQueue> queue_;
	ParsedLineReader *top_;

	// Disallows copies.
	FrontMerger(const FrontMerger &);
	FrontMerger &operator=(const FrontMerger &);
};

inline bool FrontMerger::ValueComparer::operator()(
	const ParsedLineReader *lhs, const ParsedLineReader *rhs) const
{
	if (lhs->parsed_line().value() != rhs->parsed_line().value())
		return lhs->parsed_line().value() < rhs->parsed_line().value();
	return lhs->file_cur() > rhs->file_cur();
}

}  // namespace ssgnc

#endif  // SSGNC_FRONT_MERGER_H
