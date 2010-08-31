#ifndef SSGNC_EXCEPTION_H
#define SSGNC_EXCEPTION_H

#include <cstddef>
#include <exception>

namespace ssgnc {

class Exception : public std::exception
{
public:
	explicit Exception(const char *msg = NULL) throw() : msg_(msg) {}
	Exception(const Exception &rhs) throw() : msg_(rhs.msg_) {}
	virtual ~Exception() throw() {}

	virtual const char *what() const throw()
	{ return (msg_ != NULL) ? msg_ : ""; }

private:
	const char *msg_;

	// Disallows operator=.
	Exception &operator=(const Exception &);
};

}  // namespace ssgnc

#define SSGNC_INT_TO_STR(value) #value
#define SSGNC_LINE_TO_STR(line) SSGNC_INT_TO_STR(line)
#define SSGNC_LINE_STR SSGNC_LINE_TO_STR(__LINE__)

#define SSGNC_THROW(msg) throw ssgnc::Exception( \
	__FILE__ ":" SSGNC_LINE_STR ": throw: " msg)

#ifdef SSGNC_DEBUG
#define SSGNC_ASSERT(cond) do { \
	if (!(cond)) \
	{ \
		throw ssgnc::Exception( \
			__FILE__ ":" SSGNC_LINE_STR ": assert: " #cond); \
	} \
} while (false)
#else  // SSGNC_DEBUG
#define SSGNC_ASSERT(cond)
#endif  // SSGNC_DEBUG

#endif  // SSGNC_EXCEPTION_H
