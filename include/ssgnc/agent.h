#ifndef SSGNC_AGENT_H
#define SSGNC_AGENT_H

#include "heap-queue.h"
#include "ngram-reader.h"
#include "query.h"

namespace ssgnc {

class Agent
{
public:
	class FreqComparer
	{
	public:
		bool operator()(const NgramReader *lhs,
			const NgramReader *rhs) const;
	};

	class Source
	{
	public:
		Source() : num_tokens_(0), entry_() {}
		Source(Int32 num_tokens, const NgramIndex::Entry &entry)
			: num_tokens_(num_tokens), entry_(entry) {}

		void set_num_tokens(Int32 num_tokens) { num_tokens_ = num_tokens; }
		void set_entry(const NgramIndex::Entry &entry) { entry_ = entry; }

		Int32 num_tokens() const { return num_tokens_; }
		const NgramIndex::Entry entry() const { return entry_; }

	private:
		Int32 num_tokens_;
		NgramIndex::Entry entry_;
	};

public:
	Agent();
	~Agent();

	bool open(const String &index_dir, const Query &query,
		const std::vector<Source> &sources) SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool read(Int16 *encoded_freq, std::vector<Int32> *tokens);

	bool is_open() const { return is_open_; }

	bool bad() const { return bad_; }
	bool eof() const;
	bool good() const { return !fail(); }
	bool fail() const { return bad() || eof(); }

	UInt64 num_results() const { return num_results_; }
	UInt64 tell() const { return total_; }

	const Query &query() const { return query_; }

private:
	bool is_open_;
	bool bad_;
	Query query_;
	std::vector<NgramReader *> ngram_readers_;
	HeapQueue<NgramReader *, FreqComparer> heap_queue_;
	UInt64 num_results_;
	UInt64 total_;

	bool filter(const std::vector<Int32> &tokens) const;
	bool filterUnordered(const std::vector<Int32> &tokens) const;
	bool filterOrdered(const std::vector<Int32> &tokens) const;
	bool filterPhrase(const std::vector<Int32> &tokens) const;
	bool filterFixed(const std::vector<Int32> &tokens) const;

	// Disallows copies.
	Agent(const Agent &);
	Agent &operator=(const Agent &);
};

inline bool Agent::FreqComparer::operator()(const NgramReader *lhs,
	const NgramReader *rhs) const
{
	if (lhs->encoded_freq() != rhs->encoded_freq())
		return lhs->encoded_freq() > rhs->encoded_freq();
	return lhs->num_tokens() < rhs->num_tokens();
}

inline bool Agent::eof() const
{
	if (heap_queue_.empty())
		return true;

	if (query_.max_num_results() != 0 &&
		num_results_ >= query_.max_num_results())
		return true;

	if (query_.io_limit() != 0 && total_ >= query_.io_limit())
		return true;

	return false;
}

}  // namespace ssgnc

#endif  // SSGNC_AGENT_H
