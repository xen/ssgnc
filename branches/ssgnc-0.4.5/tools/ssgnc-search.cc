#include "tools-common.h"

namespace {

void searchQueries(std::istream *in, const ssgnc::Database &database,
	ssgnc::Query *query)
{
	ssgnc::Int16 encoded_freq;
	std::vector<ssgnc::Int32> tokens;

	ssgnc::StringBuilder ngram_str;

	std::string line;
	while (ssgnc::tools::readLine(in, &line))
	{
		ssgnc::String query_str(line.c_str(), line.length());
		if (!database.parseQuery(query_str, query))
		{
			SSGNC_ERROR << "ssgnc::Database::parseQuery() failed" << std::endl;
			continue;
		}

		ssgnc::Agent agent;
		if (!database.search(*query, &agent))
		{
			SSGNC_ERROR << "ssgnc::Database::search() failed" << std::endl;
			continue;
		}

		while (agent.read(&encoded_freq, &tokens))
		{
			if (!database.decode(encoded_freq, tokens, &ngram_str))
			{
				SSGNC_ERROR << "ssgnc::Database::decode() failed" << std::endl;
				return;
			}

			std::cout << ngram_str << '\n';
			if (!std::cout)
			{
				SSGNC_ERROR << "std::ostream::operator<<() failed"
					<< std::endl;
				return;
			}
		}

		if (agent.bad())
		{
			SSGNC_ERROR << "ssgnc::Agent::read() failed" << std::endl;
			return;
		}

		std::cout << '\n';
		if (!std::cout)
		{
			SSGNC_ERROR << "std::ostream::operator<<() failed" << std::endl;
			return;
		}
	}

	if (in->bad())
		SSGNC_ERROR << "ssgnc::tools::readLine() failed" << std::endl;
}

}  // namespace

int main(int argc, char *argv[])
{
	ssgnc::Query query;
	query.set_max_num_results(10);
	query.set_io_limit(1 << 20);

	if (!query.parseOptions(&argc, argv))
		return 1;

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0]
			<< " [OPTION]... INDEX_DIR [FILE]...\n\n";
		ssgnc::Query::showOptions(&std::cerr);
		return 2;
	}

	ssgnc::Database database;
	if (!database.open(argv[1]))
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
