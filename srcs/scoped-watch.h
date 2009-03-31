#ifndef GOOGLE_NGRAM_SCOPED_WATCH_H
#define GOOGLE_NGRAM_SCOPED_WATCH_H

#include <ctime>
#include <iostream>

namespace ngram
{

class scoped_watch
{
public:
	// Initializes a watch.
	explicit scoped_watch(std::ostream &out = std::cerr)
		: out_(out), cl_(std::clock()) {}
	// Shows passed time since initialization.
	~scoped_watch()
	{
		out_ << "time: " << (1.0 * (clock() - cl_) / CLOCKS_PER_SEC)
			<< 's' << std::endl;
	}

private:
	// Output stream.
	std::ostream &out_;
	// Clock counter.
	const std::clock_t cl_;

	// Copies are not allowed.
	scoped_watch(const scoped_watch &);
	scoped_watch &operator=(const scoped_watch &);
};

}  // ngram

#endif  // GOOGLE_NGRAM_SCOPED_WATCH_H
