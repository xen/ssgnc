#ifndef SSGNC_COMMON_H
#define SSGNC_COMMON_H

#include <cstdio>
#include <functional>
#include <fstream>
#include <iostream>
#include <vector>

// The following enables warning to unused result on GCC 3.4 or newer.
#ifdef __GNUC__
#if __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
#define SSGNC_WARN_UNUSED_RESULT
#else  // __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
#define SSGNC_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#endif  // __GNUC__ < 3 || (__GNUC__ == 3 && __GNUC_MINOR__ < 4)
#endif  // __GNUC__

// These macros convert line number to constant string.
#define SSGNC_INT_TO_STR(value) #value
#define SSGNC_LINE_TO_STR(line) SSGNC_INT_TO_STR(line)
#define SSGNC_LINE_STR SSGNC_LINE_TO_STR(__LINE__)

// The format of error log is as follows:
// formatted time string: __FILE__:__LINE__: error:
//   In __FUNCTION__(): error message
#define SSGNC_ERROR (*ssgnc::error_stream() \
	<< (__FILE__ ":" SSGNC_LINE_STR ": error:\n  In ") \
	<< __FUNCTION__ << "(): ")

namespace ssgnc {

typedef char Int8;
typedef short Int16;
typedef int Int32;
typedef long long Int64;

typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned int UInt32;
typedef unsigned long long UInt64;

// Use SSGNC_ERROR for error logging.
// This function should not be called directly.
std::ostream *error_stream();

// Replaces an output stream for error logging. If a NULL pointer is given,
// error logging is disabled.
void set_error_stream(std::ostream *stream);

inline void disable_error_logging() { set_error_stream(NULL); }

}  // namespace ssgnc

#endif  // SSGNC_COMMON_H
