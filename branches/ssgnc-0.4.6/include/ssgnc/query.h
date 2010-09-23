#ifndef SSGNC_QUERY_H
#define SSGNC_QUERY_H

#include "freq-handler.h"
#include "mem-pool.h"
#include "string-builder.h"

namespace ssgnc {

class Query
{
public:
	enum TokenOrder
	{
		UNORDERED, ORDERED, PHRASE, FIXED,
		DEFAULT_ORDER = UNORDERED
	};

	enum { META_TOKEN = -1, UNKNOWN_TOKEN = -2 };

public:
	Query();
	~Query() { clear(); }

	void clear();

	bool clone(Query *dest) const;

	void clearTokens() { tokens_.clear(); }
	bool appendToken(Int64 value);
	bool set_min_freq(Int64 value);
	bool set_min_encoded_freq(Int64 value);
	bool set_min_num_tokens(Int64 value);
	bool set_max_num_tokens(Int64 value);
	bool set_max_num_results(Int64 value);
	bool set_io_limit(Int64 value);
	bool set_order(TokenOrder value);

	Int32 token(Int32 index) const;
	Int32 num_tokens() const { return static_cast<Int32>(tokens_.size()); }
	std::vector<Int32> *mutable_tokens() { return &tokens_; }
	const std::vector<Int32> &tokens() { return tokens_; }

	Int64 min_freq() const { return min_freq_; }
	Int16 min_encoded_freq() const { return min_encoded_freq_; }
	Int32 min_num_tokens() const { return min_num_tokens_; }
	Int32 max_num_tokens() const { return max_num_tokens_; }
	UInt64 max_num_results() const { return max_num_results_; }
	UInt64 io_limit() const { return io_limit_; }
	TokenOrder order() const { return order_; }

	bool parseOptions(Int32 *argc, Int8 *argv[]);
	bool parseQueryString(const String &str, MemPool *mem_pool = NULL,
		std::vector<std::pair<String, String> > *pairs = NULL);
	bool parseKeyValue(const String &key, const String &value);

	bool parseMinFreq(const String &str);
	bool parseNumTokens(const String &str);
	bool parseMinNumTokens(const String &str);
	bool parseMaxNumTokens(const String &str);
	bool parseMaxNumResults(const String &str);
	bool parseIOLimit(const String &str);
	bool parseOrder(const String &str);

	static bool showOptions(std::ostream *out);

	static const Int64 MIN_TOKEN = 0;
	static const Int64 MIN_FREQ = 1;
	static const Int64 MIN_ENCODED_FREQ = 1;
	static const Int64 MIN_NUM_TOKENS = 0;
	static const Int64 MIN_NUM_RESULTS = 0;
	static const Int64 MIN_IO_LIMIT = 0;

	static const Int64 MAX_TOKEN = 0x7FFFFFFF;
	static const Int64 MAX_FREQ = FreqHandler::MAX_FREQ;
	static const Int64 MAX_ENCODED_FREQ = FreqHandler::MAX_ENCODED_FREQ;
	static const Int64 MAX_NUM_TOKENS = 30;
	static const Int64 MAX_NUM_RESULTS = 0xFFFFFFFFFFLL;
	static const Int64 MAX_IO_LIMIT = 0xFFFFFFFFFFLL;

private:
	std::vector<Int32> tokens_;
	Int64 min_freq_;
	Int16 min_encoded_freq_;
	Int32 min_num_tokens_;
	Int32 max_num_tokens_;
	UInt64 max_num_results_;
	UInt64 io_limit_;
	TokenOrder order_;
	FreqHandler freq_handler_;

	static bool percentDecode(const String &src, StringBuilder *dest);
	static Int8 percentDecode(Int8 hi_byte, Int8 lo_byte);

	static bool parseInt(const String &str, Int64 *value);

	// Disallows copies.
	Query(const Query &);
	Query &operator=(const Query &);
};

inline Int32 Query::token(Int32 index) const
{
	if (index < 0 || index > num_tokens())
	{
		SSGNC_ERROR << "Out of range index: " << index << std::endl;
		return UNKNOWN_TOKEN;
	}
	return tokens_[index];
}

}  // namespace ssgnc

#endif  // SSGNC_QUERY_H
