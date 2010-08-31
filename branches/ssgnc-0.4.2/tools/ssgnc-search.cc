#include "tools-common.h"

namespace {

void searchQueries(std::istream *in, const ssgnc::Database &database,
	ssgnc::Query *query)
{
	ssgnc::Int16 encoded_freq;
	std::vector<ssgnc::Int32> tokens;

	ssgnc::StringBuilder ngram;

	std::string line;
	while (ssgnc::tools::readLine(in, &line))
	{
		ssgnc::String query_str(line.c_str(), line.length());
		if (!database.parseQuery(query_str, query))
		{
			SSGNC_ERROR << "ssgnc::Database::parseQuery() failed" << std::endl;
			return;
		}

		ssgnc::Agent agent;
		if (!database.search(*query, &agent))
		{
			SSGNC_ERROR << "ssgnc::Database::search() failed" << std::endl;
			return;
		}

		ssgnc::Int32 count = 0;
		while (agent.read(&encoded_freq, &tokens))
		{
			if (!database.decode(encoded_freq, tokens, &ngram))
			{
				SSGNC_ERROR << "ssgnc::Database::decode() failed" << std::endl;
				return;
			}

			std::cout << ++count << ". " << ngram << '\n';
			if (count >= 10)
				break;
		}

		if (agent.bad())
		{
			SSGNC_ERROR << "ssgnc::Agent::read() failed" << std::endl;
			return;
		}
	}
}

}  // namespace

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0]
			<< " [OPTION]... [INDEX_DIR] [FILE]...\n\n";
		ssgnc::Query::showOptions(&std::cerr);
		return 1;
	}

	ssgnc::Query query;
	if (!query.parseOptions(&argc, argv))
		return 2;

	ssgnc::Database database;
	if (!database.open((argc > 1) ? argv[1] : ""))
		return 3;

	if (argc == 2)
		searchQueries(&std::cin, database, &query);

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
		searchQueries(&file, database, &query);
	}

	return 0;
}
