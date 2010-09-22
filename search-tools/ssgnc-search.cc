#include <ssgnc.h>

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

bool searchNgrams(std::istream *in, const ssgnc::Database &database,
	ssgnc::Query *query)
{
	std::string line;
	while (readLine(in, &line))
	{
		// ssgnc::String represents a string as a pair of its start adress and
		// its length. So, if the source string is modified, the represented
		// string is modified too. In the worst-case scenario, the start
		// address will be invalid after the modification.
		ssgnc::String query_str(line.c_str(), line.length());
		if (!database.parseQuery(query_str, query))
		{
			SSGNC_ERROR << "ssgnc::Database::parseQuery() failed" << std::endl;
			return false;
		}

		// ssgnc::Agent opens .db files corresponding to the query. Then,
		// the agent's read() returns n-grams one by one.
		// If you want to reuse the agent for efficiency, please call close()
		// before() the next search. Otherwise, the next search fails.
		ssgnc::Agent agent;
		if (!database.search(*query, &agent))
		{
			SSGNC_ERROR << "ssgnc::Database::search() failed" << std::endl;
			return false;
		}

		ssgnc::Int16 encoded_freq;
		std::vector<ssgnc::Int32> tokens;
		ssgnc::StringBuilder ngram_str;

		// An n-gram consists of its encoded frequency and the IDs of its
		// tokens. And the n-gram can be decoded by decode() of the database.
		while (agent.read(&encoded_freq, &tokens))
		{
			// A string, formatted "1st_token ' ' 2nd_token ' ' ... '\t' freq",
			// is available by decode() of the database.
			if (!database.decode(encoded_freq, tokens, &ngram_str))
			{
				SSGNC_ERROR << "ssgnc::Database::decode() failed" << std::endl;
				return false;
			}

			std::cout << ngram_str << '\n';
			if (!std::cout)
			{
				SSGNC_ERROR << "std::ostream::operator<<() failed"
					<< std::endl;
				return false;
			}
		}

		// The bad bit of `agent' indicates if an error has occured or not.
		if (agent.bad())
		{
			SSGNC_ERROR << "ssgnc::Agent::read() failed" << std::endl;
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
		if (!searchNgrams(&std::cin, database, &query))
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

		if (!searchNgrams(&file, database, &query))
			return 4;
	}

	return 0;
}
