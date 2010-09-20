#ifndef SSGNC_CGI_CONFIG_H
#define SSGNC_CGI_CONFIG_H

#include <ssgnc.h>

namespace ssgnc {
namespace cgi {

class Config
{
public:
	enum TokenType { WORD_TOKEN, CHAR_TOKEN };

	static const Int8 *INDEX_DIR() { return "/usr/share/ssgnc"; }

	static TokenType TOKEN_TYPE() { return WORD_TOKEN; }

	static Int64 DEFAULT_MAX_NUM_RESULTS() { return 100LL; }
	static Int64 DEFAULT_IO_LIMIT() { return 256LL << 10; }

	static Int64 MAX_MAX_NUM_RESULTS() { return 0LL; }
	static Int64 MAX_IO_LIMIT() { return 1LL << 20; }

private:
	Config();
};

}  // namespace cgi
}  // namespace ssgnc

#endif  // SSGNC_CGI_CONFIG_H
