#ifndef GOOGLE_NGRAM_CGI_REQUEST_H
#define GOOGLE_NGRAM_CGI_REQUEST_H

#include <boost/algorithm/string.hpp>

#include <cctype>
#include <cstdlib>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace ngram
{

// For receiving GET and POST http requests.
class cgi_request
{
public:
	// Iterators for parameters.
	typedef std::map<std::string, std::string>::const_iterator iterator;

public:
	// Analyzes a http request string.
	explicit cgi_request(const std::string &request) : params_()
	{
		std::vector<std::string> params, key_value;
		boost::split(params, request, boost::is_any_of("&"));
		for (size_t i = 0; i < params.size(); ++i)
		{
			boost::split(key_value, params[i], boost::is_any_of("="));
			for (size_t j = 0; j < key_value.size(); ++j)
				decode_string(&key_value[j]);

			if (key_value.size() == 2)
				params_[key_value[0]] = key_value[1];
			else
				params_[key_value[0]] = "";
		}
	}

	// Gets and analyzes a http request.
	static cgi_request analyze(std::size_t max_length = 1024)
	{
		const char *env;
		if (!(env = std::getenv("REQUEST_METHOD")))
			return cgi_request("");

		if (std::string(env) == "GET")
		{
			// GET requests.
			if (!(env = std::getenv("QUERY_STRING")))
				return cgi_request("");
			return cgi_request(env);
		}
		else
		{
			// POST requests.
			if (!(env = std::getenv("CONTENT_LENGTH")))
				return cgi_request("");

			std::istringstream parser(env);
			std::size_t length;
			parser >> length;

			if (length > max_length)
				return cgi_request("");

			std::vector<char> buf(length);
			std::cin.read(&buf[0], length);

			return cgi_request(std::string(&buf[0], length));
		}

		return cgi_request("");
	}

	// Empty or not.
	bool empty() const { return params_.empty(); }

	// Returns an iterator to the beginning of parameters.
	iterator begin() const { return params_.begin(); }
	// Returns an iterator to the end of parameters.
	iterator end() const { return params_.end(); }

	// Finds a parameter or returns the end iterator.
	iterator find(const std::string &key) const { return params_.find(key); }
	// Finds a parameter or returns the end iterator.
	iterator operator[](const std::string &key) const { return find(key); }

private:
	// Parameters.
	std::map<std::string, std::string> params_;

	// Decodes a percent encoded string.
	static void decode_string(std::string *value)
	{
		const char *c_str = value->c_str();
		std::vector<char> buf;
		for (size_t i = 0; i < value->length(); ++i)
		{
			switch (c_str[i])
			{
			case '+':
				buf.push_back(' ');
				break;
			case '%':
				if (i + 2 >= value->length())
					break;
				++i;
				buf.push_back(to_int(c_str[i]) * 16 + to_int(c_str[i + 1]));
				++i;
				break;
			default:
				buf.push_back(c_str[i]);
				break;
			}
		}

		if (!buf.empty())
			value->assign(&buf[0], buf.size());

		boost::algorithm::replace_all(*value, "\xE3\x80\x80", " ");
		boost::algorithm::trim(*value);
	}

	// Transforms a character to an integer.
	static int to_int(int c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';
		else if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		else if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		return 0;
	}
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_CGI_REQUEST_H
