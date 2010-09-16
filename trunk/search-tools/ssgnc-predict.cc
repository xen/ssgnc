#include <ssgnc.h>

#include <cstdio>
#include <iostream>
#include <string>

namespace {

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

	ssgnc::Agent agent;
	if (!database.search(query, &agent))
	{
		SSGNC_ERROR << "ssgnc::Database::search() failed" << std::endl;
		return false;
	}

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

	query_tokens->pop_back();
	return true;
}

bool predictBack(const ssgnc::Database &database, ssgnc::Query *query,
	ssgnc::Int64 core_freq)
{
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
		ssgnc::String query_str(line.c_str(), line.length());
		if (!database.parseQuery(query_str, query, ""))
		{
			SSGNC_ERROR << "ssgnc::Database::parseQuery() failed" << std::endl;
			return false;
		}

		ssgnc::Int64 core_freq;
		if (!findCore(database, *query, &core_freq))
		{
			SSGNC_ERROR << "::findCore() failed" << std::endl;
			return false;
		}
		else if (core_freq == 0)
			continue;

		if (!predictFront(database, query, core_freq))
		{
			SSGNC_ERROR << "::predictFront() failed" << std::endl;
			return false;
		}

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
	query.set_max_num_results(10);

	if (!query.parseOptions(&argc, argv))
		return 1;

	query.set_order(ssgnc::Query::FIXED);

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
	{
		if (!predictMargins(&std::cin, database, &query))
			return 4;
	}

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
