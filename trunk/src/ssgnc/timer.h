#ifndef SSGNC_TIMER_H
#define SSGNC_TIMER_H

#ifdef HAVE_CLOCK_GETTIME

#include <time.h>

namespace ssgnc {

class Timer
{
public:
	Timer() : tp_() { ::clock_gettime(CLOCK_REALTIME, &tp_); }

	// Returns how many seconds have passed.
	double Elapsed() const
	{
		struct ::timespec current_tp;
		::clock_gettime(CLOCK_REALTIME, &current_tp);
		return (current_tp.tv_sec - tp_.tv_sec)
			+ ((current_tp.tv_nsec - tp_.tv_nsec) / 1000000000.0);
	}

private:
	struct ::timespec tp_;

	// Disallows copies.
	Timer(const Timer &);
	Timer &operator=(const Timer &);
};

}  // namespace ssgnc

#elif defined HAVE_GETTIMEOFDAY

#include <cstddef>
#include <sys/time.h>

namespace ssgnc {

class Timer
{
public:
	Timer() : tv_() { ::gettimeofday(&tv_, NULL); }

	// Returns how many seconds have passed.
	double Elapsed() const
	{
		struct ::timeval current_tv;
		::gettimeofday(&current_tv, NULL);
		return (current_tv.tv_sec - tv_.tv_sec)
			+ ((current_tv.tv_usec - tv_.tv_usec) / 1000000.0);
	}

private:
	struct ::timeval tv_;

	// Disallows copies.
	Timer(const Timer &);
	Timer &operator=(const Timer &);
};

}  // namespace ssgnc

#else  // HAVE_GETTIMEOFDAY

#include <ctime>

namespace ssgnc {

class Timer
{
public:
	Timer() : tm_(std::time(NULL)) {}

	// Returns how many seconds have passed.
	double Elapsed() const { return 1.0 * (std::time(NULL) - tm_); }

private:
	std::time_t tm_;

	// Disallows copies.
	Timer(const Timer &);
	Timer &operator=(const Timer &);
};

}  // namespace ssgnc

#endif  // HAVE_GETTIMEOFDAY

#endif  // SSGNC_TIMER_H
