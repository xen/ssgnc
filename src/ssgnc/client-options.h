#ifndef SSGNC_CLIENT_OPTIONS_H
#define SSGNC_CLIENT_OPTIONS_H

#include <getopt.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace ssgnc {

class ClientOptions
{
public:
	ClientOptions() : help_(false), dir_(), order_("UNORDERED"),
		results_(20), freq_(0), n_range_() {}

	bool help() const { return help_; }
	const std::string &dir() const { return dir_; }
	const std::string &order() const { return order_; }
	long long results() const { return results_; }
	long long freq() const { return freq_; }
	const std::string &n_range() const { return n_range_; }

	// Parses program options.
	bool Parse(int argc, char *argv[])
	{
		for ( ; ; )
		{
			int option_index = 0;
			int getopt_result = ::getopt_long(argc, argv,
				"hd:o:r:f:n:", GetLongOptions(), &option_index);
			if (getopt_result == -1)
				break;

			switch (getopt_result)
			{
			case 'h':
				help_ = true;
				break;
			case 'd':
				dir_ = ::optarg;
				break;
			case 'o':
				order_ = ::optarg;
				break;
			case 'r':
				results_ = std::strtoll(::optarg, NULL, 10);
				break;
			case 'f':
				freq_ = std::strtoll(::optarg, NULL, 10);
				break;
			case 'n':
				n_range_ = ::optarg;
				break;
			}
		}

		// Checks the required option and extra arguments.
		if (dir_.empty() || ::optind < argc)
			return false;
		return true;
	}

	// Shows the usage information.
	static void Usage(std::ostream *output)
	{
		*output << "Options:\n"
			"  -h [ --help ]                   display this help and exit\n"
			"  -d [ --dir ] arg                index directory (required)\n"
			"  -o [ --order ] arg (=UNORDERED) query order\n"
			"                                  "
			"(unordered, ordered, phrase or fixed)\n"
			"  -r [ --results ] arg (=20)      maximum number of results\n"
			"  -f [ --freq ] arg (=0)          minimum frequency\n"
			"  -n [ --n-range ] arg            range of n (ex. 1-7)\n";
	}

private:
	bool help_;
	std::string dir_;
	std::string order_;
	long long results_;
	long long freq_;
	std::string n_range_;

	// Gets a pointer to the list of long options.
	static const struct ::option *GetLongOptions()
	{
		static const struct ::option long_options[] = {
			{ "help", 0, NULL, 'h' },
			{ "dir", 1, NULL, 'd' },
			{ "order", 1, NULL, 'o' },
			{ "results", 1, NULL, 'r' },
			{ "freq", 1, NULL, 'f' },
			{ "n-range", 1, NULL, 'n' },
			{ NULL, 0, NULL, 0 }
		};
		return long_options;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_CLIENT_OPTIONS
