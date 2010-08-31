#include "ssgnc/file-path.h"

namespace ssgnc {

FilePath::~FilePath()
{
	if (is_open())
		close();
}

bool FilePath::open(const String &format)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (!format.contains('%'))
	{
		SSGNC_ERROR << "Invalid path format: " << format << std::endl;
		return false;
	}

	if (!format_.append(format) || !format_.append())
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed: "
			<< format << std::endl;
		return false;
	}
	return true;
}

bool FilePath::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	format_.clear();
	file_id_ = 0;
	return true;
}

bool FilePath::seek(ssgnc::Int32 file_id)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (file_id < 0 || file_id > MAX_FILE_ID)
	{
		SSGNC_ERROR << "Out of range file ID: " << file_id << std::endl;
		return false;
	}

	file_id_ = file_id;
	return true;
}

bool FilePath::read(StringBuilder *path)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}
	else if (file_id_ > MAX_FILE_ID)
	{
		SSGNC_ERROR << "Too large file ID: " << file_id_ << std::endl;
		return false;
	}
	else if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	path->clear();
	if (!path->appendf(format_.ptr(), file_id_))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::appendf() failed: "
			<< format_ << ", " << file_id_ << std::endl;
		return false;
	}

	++file_id_;
	return true;
}

}  // namespace ssgnc
