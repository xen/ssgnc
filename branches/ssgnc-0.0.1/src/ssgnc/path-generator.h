#ifndef SSGNC_PATH_GENERATOR_H
#define SSGNC_PATH_GENERATOR_H

#include <sstream>
#include <string>

namespace ssgnc {

class PathGenerator
{
public:
	explicit PathGenerator(const std::string &data_dir) : data_dir_(data_dir)
	{
		while (!data_dir_.empty())
		{
			if (data_dir_[data_dir_.length() - 1] != '/')
				break;
			data_dir_.resize(data_dir_.length() - 1);
		}
	}

	// Generates a default dictionary path.
	std::string DicPath() const
	{
		return data_dir_ + "/vocab.dic";
	}

	// Generates a default index path.
	std::string IndexPath(int n) const
	{
		std::ostringstream stream;
		stream << data_dir_ << '/' << n << "gms.idx";
		return stream.str();
	}

	// Generates a default text path.
	std::string TextPath(int n) const
	{
		std::ostringstream stream;
		stream << data_dir_ << '/' << n << "gms.txt";
		return stream.str();
	}

private:
	std::string data_dir_;

	// Disallows copies.
	PathGenerator(const PathGenerator &);
	PathGenerator &operator=(const PathGenerator &);
};

}  // namespace ssgnc

#endif  // SSGNC_PATH_GENERATOR_H
