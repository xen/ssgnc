#include "ssgnc.h"

#include <cassert>
#include <sstream>

int main()
{
	ssgnc::Query query;

	assert(query.num_tokens() == 0);
	assert(query.min_freq() == 1);
	assert(query.min_num_tokens() == 0);
	assert(query.max_num_tokens() == 0);
	assert(query.max_num_results() == 0);
	assert(query.io_limit() == 0);
	assert(query.order() == ssgnc::Query::DEFAULT_ORDER);

	assert(query.appendToken(ssgnc::Query::MIN_TOKEN));
	assert(query.set_min_freq(ssgnc::Query::MIN_FREQ));
	assert(query.set_min_num_tokens(ssgnc::Query::MIN_NUM_TOKENS));
	assert(query.set_max_num_tokens(ssgnc::Query::MIN_NUM_TOKENS));
	assert(query.set_max_num_results(ssgnc::Query::MIN_NUM_RESULTS));
	assert(query.set_io_limit(ssgnc::Query::MIN_IO_LIMIT));
	assert(query.set_order(ssgnc::Query::UNORDERED));

	assert(query.num_tokens() == 1);

	assert(query.token(0) == ssgnc::Query::MIN_TOKEN);

	assert(query.min_freq() == ssgnc::Query::MIN_FREQ);
	assert(query.min_num_tokens() == ssgnc::Query::MIN_NUM_TOKENS);
	assert(query.max_num_tokens() == ssgnc::Query::MIN_NUM_TOKENS);
	assert(query.max_num_results() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MIN_NUM_RESULTS));
	assert(query.io_limit() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MIN_IO_LIMIT));
	assert(query.order() == ssgnc::Query::UNORDERED);

	assert(query.appendToken(ssgnc::Query::MAX_TOKEN));
	assert(query.set_min_freq(ssgnc::Query::MAX_FREQ));
	assert(query.set_min_num_tokens(ssgnc::Query::MAX_NUM_TOKENS));
	assert(query.set_max_num_tokens(ssgnc::Query::MAX_NUM_TOKENS));
	assert(query.set_max_num_results(ssgnc::Query::MAX_NUM_RESULTS));
	assert(query.set_io_limit(ssgnc::Query::MAX_IO_LIMIT));
	assert(query.set_order(ssgnc::Query::ORDERED));

	assert(query.num_tokens() == 2);

	assert(query.token(0) == ssgnc::Query::MIN_TOKEN);
	assert(query.token(1) == ssgnc::Query::MAX_TOKEN);

	assert(query.min_freq() == ssgnc::FreqHandler::MAX_ENCODED_FREQ);
	assert(query.min_num_tokens() == ssgnc::Query::MAX_NUM_TOKENS);
	assert(query.max_num_tokens() == ssgnc::Query::MAX_NUM_TOKENS);
	assert(query.max_num_results() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MAX_NUM_RESULTS));
	assert(query.io_limit() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MAX_IO_LIMIT));
	assert(query.order() == ssgnc::Query::ORDERED);

	query.clearTokens();

	assert(query.num_tokens() == 0);

	std::stringstream stream;

	stream << ssgnc::Query::MIN_FREQ;
	assert(query.parseMinFreq(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MIN_NUM_TOKENS;
	assert(query.parseMinNumTokens(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MIN_NUM_TOKENS;
	assert(query.parseMaxNumTokens(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MIN_NUM_RESULTS;
	assert(query.parseMaxNumResults(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MIN_IO_LIMIT;
	assert(query.parseIOLimit(stream.str().c_str()));
	assert(query.parseOrder("PHRASE"));

	assert(query.min_freq() == ssgnc::Query::MIN_FREQ);
	assert(query.min_num_tokens() == ssgnc::Query::MIN_NUM_TOKENS);
	assert(query.max_num_tokens() == ssgnc::Query::MIN_NUM_TOKENS);
	assert(query.max_num_results() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MIN_NUM_RESULTS));
	assert(query.io_limit() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MIN_IO_LIMIT));
	assert(query.order() == ssgnc::Query::PHRASE);

	stream.str("");
	stream << ssgnc::Query::MAX_FREQ;
	assert(query.parseMinFreq(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MAX_NUM_TOKENS;
	assert(query.parseMinNumTokens(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MAX_NUM_TOKENS;
	assert(query.parseMaxNumTokens(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MAX_NUM_RESULTS;
	assert(query.parseMaxNumResults(stream.str().c_str()));
	stream.str("");
	stream << ssgnc::Query::MAX_IO_LIMIT;
	assert(query.parseIOLimit(stream.str().c_str()));
	assert(query.parseOrder("Fix"));

	assert(query.min_freq() == ssgnc::FreqHandler::MAX_ENCODED_FREQ);
	assert(query.min_num_tokens() == ssgnc::Query::MAX_NUM_TOKENS);
	assert(query.max_num_tokens() == ssgnc::Query::MAX_NUM_TOKENS);
	assert(query.max_num_results() == 
		static_cast<ssgnc::UInt64>(ssgnc::Query::MAX_NUM_RESULTS));
	assert(query.io_limit() ==
		static_cast<ssgnc::UInt64>(ssgnc::Query::MAX_IO_LIMIT));
	assert(query.order() == ssgnc::Query::FIXED);

	assert(query.parseNumTokens("1"));

	assert(query.min_num_tokens() == 1);
	assert(query.max_num_tokens() == 1);

	assert(query.parseNumTokens("2-4"));

	assert(query.min_num_tokens() == 2);
	assert(query.max_num_tokens() == 4);

	assert(query.parseNumTokens("3-"));

	assert(query.min_num_tokens() == 3);
	assert(query.max_num_tokens() == 0);

	assert(query.parseNumTokens("-5"));

	assert(query.min_num_tokens() == 0);
	assert(query.max_num_tokens() == 5);

	char args[][32] = {
		"argv[0]",
		"argv[1]",
		"--ssgnc-min-freq", "12345",
		"argv[2]",
		"--ssgnc-min-num-tokens=3",
		"--ssgnc-num-tokens", "2-4",
		"--ssgnc-max-num-tokens", "5",
		"argv[3]",
		"--ssgnc-max-num-results", "100",
		"--ssgnc-io-limit=1000000000000",
		"--ssgnc-order", "unordered" };

	int argc = static_cast<int>(sizeof(args) / sizeof(args[0]));
	char *argv[sizeof(args) / sizeof(args[0])];
	for (int i = 0; i < argc; ++i)
		argv[i] = args[i];

	assert(query.parseOptions(&argc, argv));
	assert(argc == 4);
	assert(ssgnc::String(argv[0]) == "argv[0]");
	assert(ssgnc::String(argv[1]) == "argv[1]");
	assert(ssgnc::String(argv[2]) == "argv[2]");
	assert(ssgnc::String(argv[3]) == "argv[3]");

	assert(query.min_freq() == (2 << 10) + 123);
	assert(query.min_num_tokens() == 2);
	assert(query.max_num_tokens() == 5);
	assert(query.max_num_results() == 100);
	assert(query.io_limit() == 1000000000000LL);
	assert(query.order() == ssgnc::Query::UNORDERED);

	assert(query.parseQueryString(
		"q=C%2B%2b+Q%26A%20FAQ"
		"&f=1%300"
		"&t=1-3"
		"&r=200"
		"&i=10%32%34"
		"&o=p"));

	assert(query.min_freq() == 100);
	assert(query.min_num_tokens() == 1);
	assert(query.max_num_tokens() == 3);
	assert(query.max_num_results() == 200);
	assert(query.io_limit() == 1024);
	assert(query.order() == ssgnc::Query::PHRASE);

	ssgnc::MemPool mem_pool;
	std::vector<std::pair<ssgnc::String, ssgnc::String> > key_value_pairs;

	assert(query.parseQueryString(
		"q=C%2B%2b+Q%26A%20FAQ"
		"&f=1%300"
		"&t=1-3"
		"&r=200"
		"&i=10%32%34"
		"&o=p", &mem_pool, &key_value_pairs));

	assert(key_value_pairs.size() == 1);
	assert(key_value_pairs[0].first == "q");
	assert(key_value_pairs[0].second == "C++ Q&A FAQ");

	return 0;
}
