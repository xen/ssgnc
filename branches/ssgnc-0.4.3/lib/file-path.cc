#include "ssgnc/file-path.h"

namespace ssgnc {

FilePath::~FilePath()
{
	if (is_open())
		close();
}

bool FilePath::open(const String &dirname, const String &basename)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (!basename.contains('%'))
	{
		SSGNC_ERROR << "Invalid basename: " << basename << std::endl;
		return false;
	}

	if (!dirname_.append(dirname) || !dirname_.append())
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed: " << std::endl;
		dirname_.clear();
		return false;
	}
	else if (!basename_.append(basename) || !basename_.append())
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed: " << std::endl;
		dirname_.clear();
		basename_.clear();
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

	dirname_.clear();
	basename_.clear();
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

	StringBuilder basename;
	if (!basename.appendf(basename_.ptr(), file_id_))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::appendf() failed: "
			<< basename_ << ", " << file_id_ << std::endl;
		return false;
	}

	if (!join(dirname_.str(), basename.str(), path))
	{
		SSGNC_ERROR << "ssgnc::FilePath::join() failed: "
			<< dirname_ << ", " << basename << std::endl;
		return false;
	}

	++file_id_;
	return true;
}

bool FilePath::join(const String &dirname, const String &basename,
	StringBuilder *path)
{
	if (basename.empty())
	{
		SSGNC_ERROR << "Empty basename" << std::endl;
		return false;
	}
	else if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	path->clear();

	if (dirname.empty() ? !path->append('.') : !path->append(dirname))
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		return false;
	}
	else if (!path->str().endsWith('/'))
	{
		if (!path->append('/'))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::append() failed"
				<< std::endl;
			return false;
		}
	}

	if (!path->append(basename) || !path->append())
	{
		SSGNC_ERROR << "ssgnc::StringBuilder::append() failed" << std::endl;
		return false;
	}

	return true;
}

}  // namespace ssgnc
