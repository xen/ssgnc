#include <ssgnc.h>

#include <cstdio>
#include <iostream>
#include <string>

namespace {

// This function reads a line from `in' and stores it into `line'.
// If the stream reaches its end or an unexpected error occurs,
// this function returns false. And in the latter case,
// the bad bit of `in' is set to true.
bool readLine(std::istream *in, std::string *line)
{
	try
	{
		if (!std::getline(*in, *line))
			return false;
		return true;
	}
	catch (...)
	{
		in->setstate(std::ios::badbit);
		return false;
	}
}

bool findCore(const ssgnc::Database &database,
	const ssgnc::Query &query, ssgnc::Int64 *core_freq)
{
	*core_freq = 0;

	// ssgnc::Agent opens .db files corresponding to the query. Then,
	// the agent's read() returns n-grams one by one.
	ssgnc::Agent agent;
	if (!database.search(query, &agent))
	{
		SSGNC_ERROR << "ssgnc::Database::search() failed" << std::endl;
		return false;
	}

	// An n-gram consists of its encoded frequency and the IDs of its
	// tokens. And the n-gram can be decoded by decode() of the database.
	ssgnc::Int16 encoded_freq;
	std::vector<ssgnc::Int32> tokens;
	if (!agent.read(&encoded_freq, &tokens))
	{
		if (agent.bad())
		{
			SSGNC_ERROR << "ssgnc::Agent::read() failed" << std::endl;
			return false;
		}
		return true;
	}

	ssgnc::StringBuilder ngram_str;
	if (!database.decode(encoded_freq, tokens, &ngram_str))
	{
		SSGNC_ERROR << "ssgnc::Database::decode() failed" << std::endl;
		return false;
	}

	if (!database.decodeFreq(encoded_freq, core_freq))
	{
		SSGNC_ERROR << "ssgnc::Database::decodeFreq() failed: "
			<< encoded_freq << std::endl;
		return false;
	}

	std::cout << ngram_str << '\n';
	if (!std::cout)
	{
		SSGNC_ERROR << "std::ostream::operator<<() failed"
			<< std::endl;
		return false;
	}

	return true;
}

bool predictMargin(const ssgnc::Database &database, const ssgnc::Query &query,
	ssgnc::Int64 core_freq)
{
	// ssgnc::Agent opens .db files corresponding to the query. Then,
	// the agent's read() returns n-grams one by one.
	ssgnc::Agent agent;
	if (!database.search(query, &agent))
	{
		SSGNC_ERROR << "ssgnc::Database::search() failed" << std::endl;
		return false;
	}

	ssgnc::Int16 encoded_freq;
	std::vector<ssgnc::Int32> tokens;
	ssgnc::StringBuilder ngram_str;

	ssgnc::Int64 total_freq = 0;

	// An n-gram consists of its encoded frequency and the IDs of its
	// tokens. And the n-gram can be decoded by decode() of the database.
	while (agent.read(&encoded_freq, &tokens))
	{
		if (!database.decode(encoded_freq, tokens, &ngram_str))
		{
			SSGNC_ERROR << "ssgnc::Database::decode() failed" << std::endl;
			return false;
		}

		ssgnc::Int64 freq;
		if (!database.decodeFreq(encoded_freq, &freq))
		{
			SSGNC_ERROR << "ssgnc::Database::decodeFreq() failed: "
				<< encoded_freq << std::endl;
			return false;
		}
		total_freq += freq;

		// This std::printf() outputs the rate and the accumulated rate.
		std::printf("%5.2f%%\t%5.2f%%\t", 100.0 * freq / core_freq,
			100.0 * total_freq / core_freq);
		std::cout << ngram_str << '\n';
		if (!std::cout)
		{
			SSGNC_ERROR << "std::ostream::operator<<() failed"
				<< std::endl;
			return false;
		}

		if ((100.0 * freq / core_freq) < 0.1)
			break;
	}

	if (agent.bad())
	{
		SSGNC_ERROR << "ssgnc::Agent::read() failed" << std::endl;
		return false;
	}

	return true;
}

bool predictFront(const ssgnc::Database &database, ssgnc::Query *query,
	ssgnc::Int64 core_freq)
{
	// A meta token is added to the head of the query.
	std::vector<ssgnc::Int32> *query_tokens = query->mutable_tokens();
	try
	{
		query_tokens->push_back(ssgnc::Query::META_TOKEN);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::Int32>::push_back() failed: "
			<< query_tokens->size() << std::endl;
		return false;
	}

	if (!predictMargin(database, *query, core_freq))
	{
		SSGNC_ERROR << "::predictMargin() failed" << std::endl;
		return false;
	}

	// The given query is restored.
	query_tokens->pop_back();
	return true;
}

bool predictBack(const ssgnc::Database &database, ssgnc::Query *query,
	ssgnc::Int64 core_freq)
{
	// A meta token is added to the end of the query.
	std::vector<ssgnc::Int32> *query_tokens = query->mutable_tokens();
	try
	{
		query_tokens->insert(query_tokens->begin(),
			ssgnc::Query::META_TOKEN);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::Int32>::insert() failed: "
			<< query_tokens->size() << std::endl;
		return false;
	}

	if (!predictMargin(database, *query, core_freq))
	{
		SSGNC_ERROR << "::predictMargin() failed" << std::endl;
		return false;
	}

	// The given query is restored.
	query_tokens->erase(query_tokens->begin());
	return true;
}

bool predictMargins(std::istream *in, const ssgnc::Database &database,
	ssgnc::Query *query)
{
	ssgnc::FreqHandler freq_handler;

	std::string line;
	while (readLine(in, &line))
	{
		// ssgnc::String represents a string as a pair of its start adress and
		// its length. So, if the source string is modified, the represented
		// string is modified too. In the worst-case scenario, the start
		// address will be invalid after the modification.
		ssgnc::String query_str(line.c_str(), line.length());

		// The 3rd argument of parseQuery() specifies a meta token. In this
		// case, given an empty string, meta tokens are not allowed.
		// Actually, "*" is handled as a regular token.
		if (!database.parseQuery(query_str, query, ""))
		{
			SSGNC_ERROR << "ssgnc::Database::parseQuery() failed" << std::endl;
			return false;
		}

		// Whether the n-gram which consists of the given tokens exists or not
		// is confirmed. If the n-gram does not exist, there is no output.
		ssgnc::Int64 core_freq;
		if (!findCore(database, *query, &core_freq))
		{
			SSGNC_ERROR << "::findCore() failed" << std::endl;
			return false;
		}
		else if (core_freq == 0)
			continue;

		// The top n-grams ending with the given tokens are listed.
		if (!predictFront(database, query, core_freq))
		{
			SSGNC_ERROR << "::predictFront() failed" << std::endl;
			return false;
		}

		// The top n-grams starting with the given tokens are listed.
		if (!predictBack(database, query, core_freq))
		{
			SSGNC_ERROR << "::predictBack() failed" << std::endl;
			return false;
		}

		std::cout << '\n';
		if (!std::cout)
		{
			SSGNC_ERROR << "std::ostream::operator<<() failed" << std::endl;
			return false;
		}
	}

	// The bad bit of `in' indicates whether an error has occured or not.
	if (in->bad())
	{
		SSGNC_ERROR << "::readLine() failed" << std::endl;
		return false;
	}

	if (!std::cout.flush())
	{
		SSGNC_ERROR << "std::ostream::flush() failed" << std::endl;
		return false;
	}
	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::Query query;

	// Here limits the number of results as the default setting.
	query.set_max_num_results(10);

	// Command line options are easily parsed by using parseOptions().
	// In this function, options starting with "--ssgnc-" and the arguments of
	// them are removed from the `argc' and `argv'.
	if (!query.parseOptions(&argc, argv))
		return 1;

	// This program always use ssgnc::Query::FIXED.
	query.set_order(ssgnc::Query::FIXED);

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0]
			<< " [OPTION]... INDEX_DIR [FILE]...\n\n";

		// You can see the list of available options by executing this command
		// without arguments.
		ssgnc::Query::showOptions(&std::cerr);
		return 2;
	}

	// A dictionary file and an index file are opend as memory-mapped files.
	// If open() takes ssgnc::FileMap::READ_FILE as the 2nd argument,
	// the entire files are loaded in this function.
	// Database files containing n-grams are opened when a query is given.
	ssgnc::Database database;
	if (!database.open(argv[1]))
		return 3;

	// If there are no more arguments,
	// queries are read from the standard input.
	if (argc == 2)
	{
		if (!predictMargins(&std::cin, database, &query))
			return 4;
	}

	// If there are arguments other than the dictionary path,
	// queries are read from the files specified as the remaining arguments.
	for (int i = 2; i < argc; ++i)
	{
		std::cerr << "> " << argv[i] << std::endl;
		std::ifstream file(argv[i], std::ios::binary);
		if (!file)
		{
			SSGNC_ERROR << "ssgnc::ifstream::open() failed: "
				<< argv[i] << std::endl;
			continue;
		}

		if (!predictMargins(&file, database, &query))
			return 4;
	}

	return 0;
}
