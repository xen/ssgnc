#include "ssgnc.h"

#include <cassert>

int main()
{
	ssgnc::Query query;

	assert(query.num_tokens() == 0);
	assert(query.min_freq() == 1);
	assert(query.min_num_tokens() == 0);
	assert(query.max_num_tokens() == 0);
	assert(query.order() == ssgnc::Query::DEFAULT);

	assert(query.append_token(0));
	assert(query.set_min_freq(10));
	assert(query.set_min_num_tokens(2));
	assert(query.set_max_num_tokens(4));
	assert(query.set_order(ssgnc::Query::UNORDERED));

	ssgnc::Int32 token;
	assert(query.token(0, &token));
	assert(token == 0);
	assert(query.num_tokens() == 1);

	assert(query.min_freq() == 10);
	assert(query.min_num_tokens() == 2);
	assert(query.max_num_tokens() == 4);
	assert(query.order() == ssgnc::Query::DEFAULT);

	assert(query.append_token(2));
	assert(query.token(0, &token));
	assert(token == 0);
	assert(query.token(1, &token));
	assert(token == 2);
	assert(query.num_tokens() == 2);

	return 0;
}
