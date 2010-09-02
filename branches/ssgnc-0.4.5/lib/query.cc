#include "ssgnc/query.h"

namespace ssgnc {

Query::Query() : tokens_(), min_freq_(MIN_FREQ),
	min_encoded_freq_(MIN_ENCODED_FREQ), min_num_tokens_(0),
	max_num_tokens_(0), max_num_results_(0), io_limit_(0),
	order_(DEFAULT_ORDER), freq_handler_() {}

void Query::clear()
{
	tokens_.clear();
	min_freq_ = MIN_FREQ;
	min_encoded_freq_ = MIN_ENCODED_FREQ;
	min_num_tokens_ = 0;
	max_num_tokens_ = 0;
	max_num_results_ = 0;
	io_limit_ = 0;
	order_ = DEFAULT_ORDER;
}

bool Query::clone(Query *dest) const
{
	if (dest == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	try
	{
		dest->tokens_ = tokens_;
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int32>::operator=()" << std::endl;
		return false;
	}

	dest->min_freq_ = min_freq_;
	dest->min_encoded_freq_ = min_encoded_freq_;
	dest->min_num_tokens_ = min_num_tokens_;
	dest->max_num_tokens_ = max_num_tokens_;
	dest->max_num_results_ = max_num_results_;
	dest->io_limit_ = io_limit_;
	dest->order_ = order_;

	return true;
}

bool Query::appendToken(Int64 value)
{
	if (value != META_TOKEN && value != UNKNOWN_TOKEN &&
		(value < MIN_TOKEN || value > MAX_TOKEN))
	{
		SSGNC_ERROR << "Out of range token ID: " << value << std::endl;
		return false;
	}

	try
	{
		tokens_.push_back(static_cast<Int32>(value));
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<ssgnc::Int32>::push_back() failed: "
			<< tokens_.size() << std::endl;
		return false;
	}
	return true;
}

bool Query::set_min_freq(Int64 value)
{
	if (value < MIN_FREQ || value > MAX_FREQ)
	{
		SSGNC_ERROR << "Out of range freq: " << value << std::endl;
		return false;
	}

	Int16 encoded_freq;
	if (!freq_handler_.encode(value, &encoded_freq))
	{
		SSGNC_ERROR << "ssgnc::FreqHandler::encode() failed: "
			<< value << std::endl;
		return false;
	}

	if (!set_min_encoded_freq(encoded_freq))
	{
		SSGNC_ERROR << "ssgnc::Query::set_min_encoded_freq() failed: "
			<< encoded_freq << std::endl;
		return false;
	}
	return true;
}

bool Query::set_min_encoded_freq(Int64 value)
{
	if (value < MIN_ENCODED_FREQ && value > MIN_ENCODED_FREQ)
	{
		SSGNC_ERROR << "Out of range encoded freq: " << value << std::endl;
		return false;
	}

	Int64 freq;
	if (!freq_handler_.decode(static_cast<Int16>(value), &freq))
	{
		SSGNC_ERROR << "ssgnc::FreqHandler::decode() failed: "
			<< value << std::endl;
		return false;
	}
	min_freq_ = freq;
	min_encoded_freq_ = static_cast<Int16>(value);
	return true;
}

bool Query::set_min_num_tokens(Int64 value)
{
	if (value < MIN_NUM_TOKENS || value > MAX_NUM_TOKENS)
	{
		SSGNC_ERROR << "Out of range #tokens: " << value << std::endl;
		return false;
	}

	min_num_tokens_ = static_cast<Int32>(value);
	return true;
}

bool Query::set_max_num_tokens(Int64 value)
{
	if (value < MIN_NUM_TOKENS || value > MAX_NUM_TOKENS)
	{
		SSGNC_ERROR << "Out of range #tokens: " << value << std::endl;
		return false;
	}

	max_num_tokens_ = static_cast<Int32>(value);
	return true;
}

bool Query::set_max_num_results(Int64 value)
{
	if (value < MIN_NUM_RESULTS || value > MAX_NUM_RESULTS)
	{
		SSGNC_ERROR << "Out of range #results: " << value << std::endl;
		return false;
	}

	max_num_results_ = static_cast<UInt64>(value);
	return true;
}

bool Query::set_io_limit(Int64 value)
{
	if (value < MIN_IO_LIMIT || value > MAX_IO_LIMIT)
	{
		SSGNC_ERROR << "Out of range IO limit: " << value << std::endl;
		return false;
	}

	io_limit_ = static_cast<UInt64>(value);
	return true;
}

bool Query::set_order(TokenOrder value)
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

bool Query::parseOptions(Int32 *argc, Int8 *argv[])
{
	static const String OPTION_PREFIX = "--ssgnc-";

	if (argc == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (argv == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	bool is_ok = true;
	Int32 new_argc = 1;
	for (Int32 i = 1; i < *argc; ++i)
	{
		String opt = argv[i];
		if (!opt.startsWith(OPTION_PREFIX))
		{
			argv[new_argc++] = argv[i];
			continue;
		}

		String key, value;
		UInt32 delim_pos;
		if (opt.first('=', &delim_pos))
		{
			key = opt.substr(0, delim_pos);
			value = opt.substr(delim_pos + 1);
		}
		else if (i + 1 < *argc)
		{
			key = opt;
			value = argv[++i];
		}
		else
		{
			SSGNC_ERROR << "No option value: " << opt << std::endl;
			return false;
		}

		if (!parseKeyValue(key, value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseKeyValue() failed: "
				<< key << ", " << value << std::endl;
			is_ok = false;
		}
	}
	*argc = new_argc;
	return is_ok;
}

bool Query::parseQueryString(const String &str, MemPool *mem_pool,
	std::vector<std::pair<String, String> > *pairs)
{
	if ((mem_pool == NULL && pairs != NULL) ||
		(mem_pool != NULL && pairs == NULL))
	{
		SSGNC_ERROR << "Inconsistent pointers" << std::endl;
		return false;
	}

	StringBuilder key, value;

	for (String avail = str; !avail.empty(); )
	{
		String param;
		UInt32 delim_pos;
		if (avail.first('&', &delim_pos))
		{
			param = avail.substr(0, delim_pos);
			avail = avail.substr(delim_pos + 1);
		}
		else
		{
			param = avail;
			avail = avail.substr(avail.length());
		}

		if (!param.first('=', &delim_pos))
		{
			SSGNC_ERROR << "No value: " << param << std::endl;
			return false;
		}

		if (!percentDecode(param.substr(0, delim_pos), &key))
		{
			SSGNC_ERROR << "ssgnc::Query::percentDecode() failed: "
				<< param.substr(0, delim_pos) << std::endl;
			return false;
		}
		else if (!percentDecode(param.substr(delim_pos + 1), &value))
		{
			SSGNC_ERROR << "ssgnc::Query::percentDecode() failed: "
				<< param.substr(delim_pos + 1) << std::endl;
			return false;
		}

		if (key.str() == "f" || key.str() == "t" || key.str() == "r" ||
			key.str() == "i" || key.str() == "o")
		{
			if (!parseKeyValue(key.str(), value.str()))
			{
				SSGNC_ERROR << "ssgnc::Query::parseKeyValue() failed: "
					<< key << ", " << value << std::endl;
				return false;
			}
			continue;
		}

		if (mem_pool == NULL)
			continue;

		String key_copy, value_copy;
		if (!mem_pool->append(key.str(), &key_copy) ||
			!mem_pool->append(value.str(), &value_copy))
		{
			SSGNC_ERROR << "ssgnc::MemPool::append() failed"
				<< std::endl;
			return false;
		}

		try
		{
			pairs->push_back(std::make_pair(key_copy, value_copy));
		}
		catch (...)
		{
			SSGNC_ERROR << "std::vector<std::pair<ssgnc::String, "
				"ssgnc::String> >::push_back() failed: "
				<< pairs->size() << std::endl;
			return false;
		}
	}
	return true;
}

bool Query::parseKeyValue(const String &key, const String &value)
{
	static const String MIN_FREQ_OPTION_KEY = "--ssgnc-min-freq";
	static const String NUM_TOKENS_OPTION_KEY = "--ssgnc-num-tokens";
	static const String MIN_NUM_TOKENS_OPTION_KEY = "--ssgnc-min-num-tokens";
	static const String MAX_NUM_TOKENS_OPTION_KEY = "--ssgnc-max-num-tokens";
	static const String MAX_NUM_RESULTS_OPTION_KEY =
		"--ssgnc-max-num-results";
	static const String IO_LIMIT_OPTION_KEY = "--ssgnc-io-limit";
	static const String ORDER_OPTION_KEY = "--ssgnc-order";

	if (key == "f" || key == MIN_FREQ_OPTION_KEY)
	{
		if (!parseMinFreq(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseMinFreq() failed: "
				<< value << std::endl;
			return false;
		}
	}
	else if (key == "t" || key == NUM_TOKENS_OPTION_KEY)
	{
		if (!parseNumTokens(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseNumTokens() failed: "
				<< value << std::endl;
			return false;
		}
	}
	else if (key == MIN_NUM_TOKENS_OPTION_KEY)
	{
		if (!parseMinNumTokens(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseMinNumTokens() failed: "
				<< value << std::endl;
			return false;
		}
	}
	else if (key == MAX_NUM_TOKENS_OPTION_KEY)
	{
		if (!parseMaxNumTokens(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseMaxNumTokens() failed: "
				<< value << std::endl;
			return false;
		}
	}
	else if (key == "r" || key == MAX_NUM_RESULTS_OPTION_KEY)
	{
		if (!parseMaxNumResults(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseMaxNumResults() failed: "
				<< value << std::endl;
			return false;
		}
	}
	else if (key == "i" || key == IO_LIMIT_OPTION_KEY)
	{
		if (!parseIOLimit(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseIOLimit() failed: "
				<< value << std::endl;
			return false;
		}
	}
	else if (key == "o" || key == ORDER_OPTION_KEY)
	{
		if (!parseOrder(value))
		{
			SSGNC_ERROR << "ssgnc::Query::parseOrder() failed: "
				<< value << std::endl;
			return false;
		}
	}

	return true;
}

bool Query::parseMinFreq(const String &str)
{

	Int64 value;
	if (!parseInt(str, &value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: " << str << std::endl;
		return false;
	}
	else if (!set_min_freq(value))
	{
		SSGNC_ERROR << "ssgnc::Query::set_min_freq: " << value << std::endl;
		return false;
	}
	return true;
}

bool Query::parseNumTokens(const String &str)
{
	String min_str;
	String max_str;

	UInt32 delim_pos;
	if (str.first('-', &delim_pos))
	{
		min_str = str.substr(0, delim_pos);
		max_str = str.substr(delim_pos + 1);
	}
	else
	{
		min_str = str;
		max_str = str;
	}

	Int64 min_value;
	if (!parseInt(min_str, &min_value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: "
			<< min_str << std::endl;
		return false;
	}

	Int64 max_value;
	if (!parseInt(max_str, &max_value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: "
			<< max_str << std::endl;
		return false;
	}

	if (!set_min_num_tokens(min_value))
	{
		SSGNC_ERROR << "ssgnc::set_min_num_tokens() failed: "
			<< min_value << std::endl;
		return false;
	}
	else if (!set_max_num_tokens(max_value))
	{
		SSGNC_ERROR << "ssgnc::set_max_num_tokens() failed: "
			<< max_value << std::endl;
		return false;
	}
	return true;
}

bool Query::parseMinNumTokens(const String &str)
{
	Int64 value;
	if (!parseInt(str, &value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: " << str << std::endl;
		return false;
	}
	else if (!set_min_num_tokens(value))
	{
		SSGNC_ERROR << "ssgnc::Query::set_min_num_tokens: "
			<< value << std::endl;
		return false;
	}
	return true;
}

bool Query::parseMaxNumTokens(const String &str)
{
	Int64 value;
	if (!parseInt(str, &value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: " << str << std::endl;
		return false;
	}
	else if (!set_max_num_tokens(value))
	{
		SSGNC_ERROR << "ssgnc::Query::set_max_num_tokens: "
			<< value << std::endl;
		return false;
	}
	return true;
}

bool Query::parseMaxNumResults(const String &str)
{
	Int64 value;
	if (!parseInt(str, &value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: " << str << std::endl;
		return false;
	}
	else if (!set_max_num_results(value))
	{
		SSGNC_ERROR << "ssgnc::Query::set_max_num_results: "
			<< value << std::endl;
		return false;
	}
	return true;
}

bool Query::parseIOLimit(const String &str)
{
	Int64 value;
	if (!parseInt(str, &value))
	{
		SSGNC_ERROR << "ssgnc::Query::parseInt() failed: " << str << std::endl;
		return false;
	}
	else if (!set_io_limit(value))
	{
		SSGNC_ERROR << "ssgnc::Query::set_io_limit: " << value << std::endl;
		return false;
	}
	return true;
}

bool Query::parseOrder(const String &str)
{
	if (str.empty())
	{
		SSGNC_ERROR << "Empty string" << std::endl;
		return false;
	}

	if (String("unordered").lowerStartsWith(str))
		order_ = UNORDERED;
	else if (String("ordered").lowerStartsWith(str))
		order_ = ORDERED;
	else if (String("phrase").lowerStartsWith(str))
		order_ = PHRASE;
	else if (String("fixed").lowerStartsWith(str))
		order_ = FIXED;
	else
	{
		SSGNC_ERROR << "Unknown token order: " << str << std::endl;
		return false;
	}
	return true;
}

bool Query::showOptions(std::ostream *out)
{
	static const String MIN_FREQ_OPTION_KEY = "--ssgnc-min-freq";
	static const String NUM_TOKENS_OPTION_KEY = "--ssgnc-num-tokens";
	static const String MIN_NUM_TOKENS_OPTION_KEY = "--ssgnc-min-num-tokens";
	static const String MAX_NUM_TOKENS_OPTION_KEY = "--ssgnc-max-num-tokens";
	static const String IO_LIMIT_OPTION_KEY = "--ssgnc-io-limit";
	static const String ORDER_OPTION_KEY = "--ssgnc-order";

	if (out == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	*out << "Options:\n"
		<< "  --ssgnc-min-freq="
		<< '[' << MIN_FREQ << '-' << MAX_FREQ << "]\n"
		<< "  --ssgnc-num-tokens="
		<< '[' << MIN_NUM_TOKENS << '-' << MAX_NUM_TOKENS << ']'
		<< "[-][" << MIN_NUM_TOKENS << '-' << MAX_NUM_TOKENS << "]\n"
		<< "  --ssgnc-min-num-tokens="
		<< '[' << MIN_NUM_TOKENS << '-' << MAX_NUM_TOKENS << "]\n"
		<< "  --ssgnc-max-num-tokens="
		<< '[' << MIN_NUM_TOKENS << '-' << MAX_NUM_TOKENS << "]\n"
		<< "  --ssgnc-max-num-results="
		<< '[' << MIN_NUM_RESULTS << '-' << MAX_NUM_RESULTS << "]\n"
		<< "  --ssgnc-io-limit="
		<< '[' << MIN_IO_LIMIT << '-' << MAX_IO_LIMIT << "]\n"
		<< "  --ssgnc-order="
		<< "[UNORDERED, ORDERED, PHRASE, FIXED]\n";
	if (!*out)
	{
		SSGNC_ERROR << "std::ostream::operator<<() failed" << std::endl;
		return false;
	}

	return true;
}

bool Query::percentDecode(const String &src, StringBuilder *dest)
{
	if (dest == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	dest->clear();

	for (UInt32 i = 0; i < src.length(); ++i)
	{
		Int8 byte = src[i];
		if (src[i] == '%')
		{
			if (i + 2 < src.length() &&
				std::isxdigit(static_cast<UInt8>(src[i + 1])) &&
				std::isxdigit(static_cast<UInt8>(src[i + 2])))
			{
				byte = percentDecode(src[i + 1], src[i + 2]);
				i += 2;
			}
		}
		else if (src[i] == '+')
			byte = ' ';

		if (!dest->append(byte))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}
	}

	if (!dest->append())
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		return false;
	}
	return true;
}

Int8 Query::percentDecode(Int8 hi_byte, Int8 lo_byte)
{
	if (!std::isxdigit(static_cast<UInt8>(hi_byte)))
	{
		SSGNC_ERROR << "Non hexadecimal byte: " << hi_byte << std::endl;
		return false;
	}
	else if (!std::isxdigit(static_cast<UInt8>(lo_byte)))
	{
		SSGNC_ERROR << "Non hexadecimal byte: " << lo_byte << std::endl;
		return false;
	}

	Int32 value = 0;

	Int32 upper = static_cast<UInt8>(std::toupper(hi_byte));
	if (std::isdigit(upper))
		value += upper - '0';
	else
		value += upper - 'A' + 10;
	value *= 16;

	upper = static_cast<UInt8>(std::toupper(lo_byte));
	if (std::isdigit(upper))
		value += upper - '0';
	else
		value += upper - 'A' + 10;

	return static_cast<Int8>(value);
}

bool Query::parseInt(const String &str, Int64 *value)
{
	if (str.length() >= 20)
	{
		SSGNC_ERROR << "Too long string: " << str << std::endl;
		return false;
	}
	else if (value == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	*value = 0;
	for (UInt32 i = 0; i < str.length(); ++i)
	{
		if (!std::isdigit(static_cast<UInt8>(str[i])))
		{
			SSGNC_ERROR << "Non digit character: " << str << std::endl;
			return false;
		}
		*value = (*value * 10) + str[i] - '0';
	}
	return true;
}

}  // namespace ssgnc
