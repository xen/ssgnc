#ifndef SSGNC_PATH_GENERATOR_H
#define SSGNC_PATH_GENERATOR_H

#include <sstream>
#include <string>

namespace ssgnc {

class PathGenerator
{
public:
	PathGenerator() : dir_name_(".") {}

	void set_dir_name(const std::string &dir_name)
	{
		dir_name_ = dir_name;
		while (!dir_name_.empty())
		{
			if (dir_name_[dir_name_.length() - 1] != '/')
				break;
			dir_name_.resize(dir_name_.length() - 1);
		}
	}
	const std::string &dir_name() const { return dir_name_; }

	// Generates a default dictionary path.
	std::string VocabDicPath() const
	{
		return dir_name_ + "/vocab.dic";
	}

	// Generates a default vocabulary path.
	std::string VocabIndexPath() const
	{
		return dir_name_ + "/vocab.idx";
	}

	// Generates a default text path.
	std::string DbPath(int n) const
	{
		std::ostringstream stream;
		stream << dir_name_ << '/' << n << "gms.db";
		return stream.str();
	}

private:
	std::string dir_name_;

	// Disallows copies.
	PathGenerator(const PathGenerator &);
	PathGenerator &operator=(const PathGenerator &);
};

}  // namespace ssgnc

#endif  // SSGNC_PATH_GENERATOR_H
