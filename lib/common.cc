#include "ssgnc/common.h"

#include <ctime>

namespace ssgnc {
namespace {

// This streambuf works like /dev/null.
// Error logging can be disabled by using this streambuf.
class NullStreambuf : public std::streambuf
{
public:
	NullStreambuf() : std::streambuf() {}
	~NullStreambuf() {}

	int_type overflow(int_type c) { return c; }

private:
	// Disallows copies.
	NullStreambuf(const NullStreambuf &);
	NullStreambuf &operator=(const NullStreambuf &);
};

// In default, messages are written to the standard error through std::clog
// which is a buffered output stream. This means that the stream should be
// flushed by std::endl or std::flush.
std::ostream *error_stream_ = &std::clog;

}  // namespace

std::ostream *error_stream()
{
	std::time_t epoch_time = std::time(NULL);
	struct tm current_time;

// The thread-safe versions are used instead of ::localtime().
#ifdef _MSC_VER
	::localtime_s(&current_time, &epoch_time);
#else  // _MSC_VER
	::localtime_r(&epoch_time, &current_time);
#endif  // _MSC_VER

	Int8 time_buf[32];
	std::strftime(time_buf, sizeof(time_buf),
		"%Y-%m-%d %H-%M-%S: ", &current_time);
	*error_stream_ << time_buf;
	return error_stream_;
}

void set_error_stream(std::ostream *stream)
{
	if (stream == NULL)
	{
		static NullStreambuf null_streambuf;
		static std::ostream null_stream(&null_streambuf);
		stream = &null_stream;
	}
	error_stream_ = stream;
}

}  // namespace ssgnc
