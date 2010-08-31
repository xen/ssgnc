#include "ssgnc/agent.h"

namespace ssgnc {

Agent::Agent() : is_open_(false), bad_(false), query_(),
	ngram_readers_(), heap_queue_(), num_results_(0), total_(0) {}

Agent::~Agent()
{
	if (is_open())
		close();
}

bool Agent::open(const String &index_dir, const Query &query,
	const std::vector<Source> &sources)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}

	is_open_ = true;

	if (!query.clone(&query_))
	{
		SSGNC_ERROR << "ssgnc::Query::clone() failed" << std::endl;
		close();
		return false;
	}

	ngram_readers_.resize(sources.size(), NULL);
	for (std::size_t i = 0; i < sources.size(); ++i)
	{
		try
		{
			ngram_readers_[i] = new NgramReader;
		}
		catch (...)
		{
			SSGNC_ERROR << "new ssgnc::NgramReader failed" << std::endl;
			close();
			return false;
		}

		if (!ngram_readers_[i]->open(index_dir, sources[i].num_tokens(),
			sources[i].entry(), query.min_freq()))
		{
			SSGNC_ERROR << "ssgnc::NgramReader::open() failed" << std::endl;
			close();
			return false;
		}

		if (ngram_readers_[i]->good() && !heap_queue_.push(ngram_readers_[i]))
		{
			SSGNC_ERROR << "ssgnc::HeapQueue::push() failed" << std::endl;
			close();
			return false;
		}
	}

	return true;
}

bool Agent::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	for (std::size_t i = 0; i < ngram_readers_.size(); ++i)
		delete ngram_readers_[i];

	is_open_ = false;
	bad_ = false;
	query_.clear();
	ngram_readers_.clear();
	heap_queue_.clear();
	num_results_ = 0;
	total_ = 0;

	return true;
}

bool Agent::read(Int16 *encoded_freq, std::vector<Int32> *tokens)
{
	while (good())
	{
		NgramReader *ngram_reader;
		if (!heap_queue_.top(&ngram_reader))
		{
			SSGNC_ERROR << "ssgnc::HeapQueue<ssgnc::NgramReader *>::top() "
				"failed" << std::endl;
			return false;
		}

		total_ -= ngram_reader->tell();
		bool is_ok = ngram_reader->read(encoded_freq, tokens);
		total_ += ngram_reader->tell();

		if (is_ok)
		{
			heap_queue_.popPush(ngram_reader);
			if (filter(*tokens))
			{
				++num_results_;
				return true;
			}
		}
		else if (ngram_reader->bad())
		{
			SSGNC_ERROR << "ssgnc::NgramReader::read() failed" << std::endl;
			bad_ = true;
			return false;
		}
		else
			heap_queue_.pop();
	}

	return false;
}

bool Agent::filter(const std::vector<Int32> &tokens) const
{
	switch (query_.order())
	{
	case Query::UNORDERED:
		return filterUnordered(tokens);
	case Query::ORDERED:
		return filterOrdered(tokens);
	case Query::PHRASE:
		return filterPhrase(tokens);
	case Query::FIXED:
		return filterFixed(tokens);
	default:
		SSGNC_ERROR << "Undefined token order: " << std::endl;
		return false;
	}
}

bool Agent::filterUnordered(const std::vector<Int32> &tokens) const
{
	UInt32 mask = 0;
	for (Int32 i = 0; i < query_.num_tokens(); ++i)
	{
		Int32 token = query_.token(i);
		if (token == Query::META_TOKEN)
			continue;

		std::size_t j;
		for (j = 0; j < tokens.size(); ++j)
		{
			if ((mask & (1U << j)) != 0)
				continue;

			if (token == tokens[j])
			{
				mask |= 1U << j;
				break;
			}
		}
		if (j >= tokens.size())
			return false;
	}
	return true;
}

bool Agent::filterOrdered(const std::vector<Int32> &tokens) const
{
	for (Int32 i = 0, j = 0; i < query_.num_tokens(); ++i, ++j)
	{
		Int32 token = query_.token(i);
		while (j < static_cast<Int32>(tokens.size()))
		{
			if (token == Query::META_TOKEN || token == tokens[j])
				break;
			++j;
		}
		if (j >= static_cast<Int32>(tokens.size()))
			return false;
	}
	return true;
}

bool Agent::filterPhrase(const std::vector<Int32> &tokens) const
{
	Int32 max_i = static_cast<Int32>(tokens.size()) - query_.num_tokens();
	for (Int32 i = 0; i <= max_i; ++i)
	{
		Int32 j;
		for (j = 0; j < query_.num_tokens(); ++j)
		{
			Int32 token = query_.token(j);
			if (token == Query::META_TOKEN)
				continue;

			if (token != tokens[i + j])
				break;
		}
		if (j == query_.num_tokens())
			return true;
	}
	return false;
}

bool Agent::filterFixed(const std::vector<Int32> &tokens) const
{
	if (static_cast<Int32>(tokens.size()) != query_.num_tokens())
		return false;
	for (Int32 i = 0; i < query_.num_tokens(); ++i)
	{
		Int32 token = query_.token(i);
		if (token == Query::META_TOKEN)
			continue;

		if (token != tokens[i])
			return false;
	}
	return true;
}

}  // namespace ssgnc
