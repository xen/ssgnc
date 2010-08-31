#ifndef SSGNC_TEMPFILE_MANAGER
#define SSGNC_TEMPFILE_MANAGER

#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace ssgnc {

// A manager class for temporary files.
class TempfileManager
{
public:
	explicit TempfileManager(const char *temp_dir)
		: temp_dir_(temp_dir), fd_list_(), path_list_()
	{
		if (temp_dir_.empty())
			temp_dir_ = "/tmp/";
		else if (temp_dir_[temp_dir_.length() - 1] != '/')
			temp_dir_ += '/';
	}
	~TempfileManager()
	{
		// Closes all temporary files.
		for (std::size_t i =  0; i < fd_list_.size(); ++i)
		{
			::close(fd_list_[i]);
			::unlink(path_list_[i].c_str());
		}
	}

	// Number of temporary files.
	std::size_t size() const { return fd_list_.size(); }

	// Descriptors of temporary files.
	const int &operator[](std::size_t id) const { return fd_list_[id]; }

	// Creates a new temporary file.
	int Add()
	{
		std::string temp_path = temp_dir_ + "ssgnc-tempfile-XXXXXX";
		std::vector<char> temp_path_buf(temp_path.length() + 1);
		temp_path.copy(&temp_path_buf[0], temp_path.length());
		temp_path_buf.back() = '\0';

		int fd = ::mkstemp(&temp_path_buf[0]);
		if (fd != -1)
		{
			fd_list_.push_back(fd);
			path_list_.push_back(&temp_path_buf[0]);
		}
		return fd;
	}

private:
	std::string temp_dir_;
	std::vector<int> fd_list_;
	std::vector<std::string> path_list_;

	// Disallows copies.
	TempfileManager(const TempfileManager &);
	TempfileManager &operator=(const TempfileManager &);
};

}  // namespace ssgnc

#endif  // SSGNC_TEMPFILE_MANAGER
