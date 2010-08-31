#include "ssgnc/database.h"

#include <cctype>

namespace ssgnc {

Database::Database() : index_dir_(), vocab_dic_(), ngram_index_(),
	freq_handler_() {}

Database::~Database()
{
	if (is_open())
		close();
}

bool Database::open(const String &index_dir, FileMap::Mode mode)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}

	StringBuilder path;
	if (!FilePath::join(index_dir, "vocab.dic", &path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::join() failed" << std::endl;
		return false;
	}
	else if (!vocab_dic_.open(path.ptr(), mode))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::open() failed"
			<< path << std::endl;
		return false;
	}

	path.clear();
	if (!FilePath::join(index_dir, "ngms.idx", &path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::join() failed" << std::endl;
		close();
		return false;
	}
	else if (!ngram_index_.open(path.ptr(), mode))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::open() failed"
			<< path << std::endl;
		close();
		return false;
	}

	if (vocab_dic_.num_keys() !=
		static_cast<UInt32>(ngram_index_.max_token_id() + 1))
	{
		SSGNC_ERROR << "Wrong pair: " << vocab_dic_.num_keys()
			<< ", " << ngram_index_.max_token_id() << std::endl;
		close();
		return false;
	}

	if (!index_dir_.append(index_dir))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		close();
		return false;
	}

	return true;
}

bool Database::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	index_dir_.clear();
	vocab_dic_.close();
	if (ngram_index_.is_open())
		ngram_index_.close();
	return true;
}

bool Database::parseQuery(const String &str, Query *query,
	const String &meta_token) const
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (query == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	query->clearTokens();

	String avail = str;
	while (!avail.empty())
	{
		String delim = findDelim(avail);
		String token(avail.begin(), delim.begin());
		avail = String(delim.end(), avail.end());

		if (token.empty())
			continue;

		Int32 token_id;
		if (token == meta_token)
			token_id = Query::META_TOKEN;
		else if (!vocab_dic_.find(token, &token_id))
		{
			SSGNC_ERROR << "ssgnc::VocabDic::find() failed: "
				<< token << std::endl;
			return false;
		}

		if (!query->appendToken(token_id))
		{
			SSGNC_ERROR << "ssgnc::Query::appendToken() failed: "
				<< token_id << std::endl;
			return false;
		}
	}

	return true;
}

bool Database::search(const Query &query, Agent *agent) const
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (agent == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int32 min_num_tokens = 1;
	Int32 max_num_tokens = ngram_index_.max_num_tokens();

	if (query.min_num_tokens() != 0 && query.min_num_tokens() > min_num_tokens)
		min_num_tokens = query.min_num_tokens();
	if (query.max_num_tokens() != 0 && query.max_num_tokens() < max_num_tokens)
		max_num_tokens = query.max_num_tokens();

	if (query.num_tokens() > min_num_tokens)
		min_num_tokens = query.num_tokens();
	if (query.order() == Query::FIXED && query.num_tokens() < max_num_tokens)
		max_num_tokens = query.num_tokens();

	std::vector<Agent::Source> sources;
	for (Int32 i = min_num_tokens; i <= max_num_tokens; ++i)
	{
		NgramIndex::Entry min_entry;
		for (Int32 j = 0; j < query.num_tokens(); ++j)
		{
			Int32 token;
			if (!query.token(j, &token))
			{
				SSGNC_ERROR << "ssgnc::Query::token() failed" << std::endl;
				return false;
			}
			else if (token == Query::META_TOKEN)
				continue;

			NgramIndex::Entry entry;
			if (!ngram_index_.get(i, token, &entry))
			{
				SSGNC_ERROR << "ssgnc::NgramIndex::get() failed" << std::endl;
				return false;
			}

			if (min_entry.approx_size() == 0 ||
				entry.approx_size() < min_entry.approx_size())
				min_entry = entry;
		}

		if (min_entry.approx_size() > 1)
		{
			try
			{
				sources.push_back(Agent::Source(i, min_entry));
			}
			catch (...)
			{
				SSGNC_ERROR << "std::vector<ssgnc::Agent::Source>::"
					"push_back(): " << sources.size() << std::endl;
				return false;
			}
		}
	}

	if (!agent->open(index_dir_.str(), query, sources))
	{
		SSGNC_ERROR << "ssgnc::Agent::open() failed" << std::endl;
		return false;
	}

	return true;
}

bool Database::decode(Int16 encoded_freq, const std::vector<Int32> &tokens,
	StringBuilder *ngram) const
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (ngram == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Int64 freq;
	if (!decodeFreq(encoded_freq, &freq))
	{
		SSGNC_ERROR << "ssgnc::Database::decodeFreq() failed: "
			<< encoded_freq << std::endl;
		return false;
	}

	if (!decodeTokens(tokens, ngram))
	{
		SSGNC_ERROR << "ssgnc::Database::decodeTokens() failed: " << std::endl;
		return false;
	}

	if (!ngram->appendf("\t%lld", freq))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::appendf() failed: " << std::endl;
		return false;
	}

	return true;
}

bool Database::decode(Int16 encoded_freq, const std::vector<Int32> &token_ids,
	Int64 *freq, std::vector<String> *token_strs) const
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (freq == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (token_strs == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!decodeFreq(encoded_freq, freq))
	{
		SSGNC_ERROR << "ssgnc::Database::decodeFreq() failed: "
			<< encoded_freq << std::endl;
		return false;
	}

	if (!decodeTokens(token_ids, token_strs))
	{
		SSGNC_ERROR << "ssgnc::Database::decodeTokens() failed: " << std::endl;
		return false;
	}

	return true;
}

bool Database::decodeFreq(Int16 encoded_freq, Int64 *freq) const
{
	if (freq == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!freq_handler_.decode(encoded_freq, freq))
	{
		SSGNC_ERROR << "ssgnc::FreqHandler::decode() failed: "
			<< encoded_freq << std::endl;
		return false;
	}

	return true;
}

bool Database::decodeTokens(const std::vector<Int32> &token_ids,
	std::vector<String> *token_strs) const
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (token_strs == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	token_strs->clear();

	for (std::size_t i = 0; i < token_ids.size(); ++i)
	{
		String token;
		if (!vocab_dic_.find(token_ids[i], &token))
		{
			SSGNC_ERROR << "ssgnc::VocabDic::find() failed: "
				<< token_ids[i] << std::endl;
			return false;
		}

		try
		{
			token_strs->push_back(token);
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<ssgnc::String>::push_back() failed: "
				<< token_strs->size() << std::endl;
			return false;
		}
	}

	return true;
}

bool Database::decodeTokens(const std::vector<Int32> &tokens,
	StringBuilder *tokens_str) const
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (tokens_str == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	tokens_str->clear();

	for (std::size_t i = 0; i < tokens.size(); ++i)
	{
		String token;
		if (!vocab_dic_.find(tokens[i], &token))
		{
			SSGNC_ERROR << "ssgnc::VocabDic::find() failed: "
				<< tokens[i] << std::endl;
			return false;
		}

		if (!tokens_str->empty() && !tokens_str->append(' '))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}

		if (!tokens_str->append(token))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}
	}

	if (!tokens_str->append())
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		return false;
	}

	return true;
}

String Database::findDelim(const String &str)
{
	for (std::size_t i = 0; i < str.length(); ++i)
	{
		if (std::isspace(str[i]))
			return str.substr(i, 1);
		else if (static_cast<UInt8>(str[i]) == 0xE3)
		{
			if (str.length() > i + 2 &&
				static_cast<UInt8>(str[i + 1]) == 0x80 &&
				static_cast<UInt8>(str[i + 2]) == 0x80)
				return str.substr(i, 3);
		}
	}

	return str.substr(str.length());
}

}  // namespace ssgnc
