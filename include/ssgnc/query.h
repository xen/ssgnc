#ifndef SSGNC_QUERY_H
#define SSGNC_QUERY_H

#include "freq-handler.h"
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

	enum { META_TOKEN = -1 };

public:
	Query() : tokens_(), min_freq_(1), min_num_tokens_(0), max_num_tokens_(0),
		io_limit_(0), order_(DEFAULT_ORDER) {}
	~Query() { clear(); }

	void clear();

	bool clone(Query *dest) const;

	void clearTokens() { tokens_.clear(); }
	bool appendToken(Int64 value);
	bool set_min_freq(Int64 value);
	bool set_min_num_tokens(Int64 value);
	bool set_max_num_tokens(Int64 value);
	bool set_io_limit(Int64 value);
	bool set_order(TokenOrder value);

	bool token(Int32 index, Int32 *token) const;
	Int32 num_tokens() const { return static_cast<Int32>(tokens_.size()); }
	Int16 min_freq() const { return min_freq_; }
	Int32 min_num_tokens() const { return min_num_tokens_; }
	Int32 max_num_tokens() const { return max_num_tokens_; }
	UInt64 io_limit() const { return io_limit_; }
	TokenOrder order() const { return order_; }

	bool parseOptions(Int32 *argc, Int8 *argv[]);
	bool parseQueryString(const String &str, StringBuilder *query);
	bool parseKeyValue(const String &key, const String &value);

	bool parseMinFreq(const String &str);
	bool parseNumTokens(const String &str);
	bool parseMinNumTokens(const String &str);
	bool parseMaxNumTokens(const String &str);
	bool parseIOLimit(const String &str);
	bool parseOrder(const String &str);

	static bool showOptions(std::ostream *out);

	static const Int64 MIN_TOKEN = 0;
	static const Int64 MIN_FREQ = 1;
	static const Int64 MIN_NUM_TOKENS = 0;
	static const Int64 MIN_IO_LIMIT = 0;

	static const Int64 MAX_TOKEN = 0x7FFFFFFF;
	static const Int64 MAX_FREQ = FreqHandler::MAX_FREQ;
	static const Int64 MAX_NUM_TOKENS = 30;
	static const Int64 MAX_IO_LIMIT = 0xFFFFFFFFFFLL;

private:
	std::vector<Int32> tokens_;
	Int16 min_freq_;
	Int32 min_num_tokens_;
	Int32 max_num_tokens_;
	UInt64 io_limit_;
	TokenOrder order_;

	static bool percentDecode(const String &src, StringBuilder *dest);
	static Int8 percentDecode(Int8 hi_byte, Int8 lo_byte);
	static bool parseInt(const String &str, Int64 *value);

	// Disallows copies.
	Query(const Query &);
	Query &operator=(const Query &);
};

inline bool Query::token(Int32 index, Int32 *token) const
{
	if (index < 0 || index > num_tokens())
	{
		SSGNC_ERROR << "Out of range index: " << index << std::endl;
		return false;
	}
	*token = tokens_[index];
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_QUERY_H
