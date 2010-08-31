#ifndef SSGNC_QUERY_H
#define SSGNC_QUERY_H

#include "common.h"

namespace ssgnc {

class Query
{
public:
	enum TokenOrder
	{
		UNORDERED,
		ORDERED,
		PHRASE,
		FIXED,
		DEFAULT = UNORDERED
	};

	enum { META_ANY_TOKEN = -1 };

public:
	Query() : tokens_(), min_freq_(1), min_num_tokens_(0),
		max_num_tokens_(0), order_(DEFAULT) {}
	~Query() {}

	void clear_tokens() { tokens_.clear(); }
	bool append_token(Int32 value);
	bool set_min_freq(Int32 value);
	bool set_min_num_tokens(Int32 value);
	bool set_max_num_tokens(Int32 value);
	bool set_order(TokenOrder value);

	bool token(Int32 index, Int32 *token) const;
	Int32 num_tokens() const { return static_cast<Int32>(tokens_.size()); }
	Int16 min_freq() const { return min_freq_; }
	Int32 min_num_tokens() const { return min_num_tokens_; }
	Int32 max_num_tokens() const { return max_num_tokens_; }
	TokenOrder order() const { return order_; }

private:
	std::vector<Int32> tokens_;
	Int16 min_freq_;
	Int32 min_num_tokens_;
	Int32 max_num_tokens_;
	TokenOrder order_;

	// Disallows copies.
	Query(const Query &);
	Query &operator=(const Query &);
};

inline bool Query::append_token(Int32 value)
{
	try
	{
		tokens_.push_back(value);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::Int32>::push_back() failed: "
			<< tokens_.size() << std::endl;
		return false;
	}
	return true;
}

inline bool Query::set_min_freq(Int32 value)
{
	if (value <= 0)
	{
		SSGNC_ERROR << "Invalid min_freq: " << value << std::endl;
		return false;
	}
	min_freq_ = value;
	return true;
}

inline bool Query::set_min_num_tokens(Int32 value)
{
	if (value < 0)
	{
		SSGNC_ERROR << "Invalid min_num_tokens: " << value << std::endl;
		return false;
	}
	min_num_tokens_ = value;
	return true;
}

inline bool Query::set_max_num_tokens(Int32 value)
{
	if (value < 0)
	{
		SSGNC_ERROR << "Invalid max_num_tokens: " << value << std::endl;
		return false;
	}
	max_num_tokens_ = value;
	return true;
}

inline bool Query::set_order(TokenOrder value)
{
	switch (value)
	{
	case UNORDERED:
	case ORDERED:
	case PHRASE:
	case FIXED:
		break;
	default:
		SSGNC_ERROR << "Undefined token order: " << value << std::endl;
		return false;
	}
	order_ = value;
	return true;
}

inline bool Query::token(Int32 index, Int32 *token) const
{
	if (static_cast<UInt32>(index) >= static_cast<UInt32>(num_tokens()))
	{
		SSGNC_ERROR << "Out of range index: " << index << std::endl;
		return false;
	}
	*token = tokens_[index];
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_QUERY_H
