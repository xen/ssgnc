#ifndef SSGNC_FILE_PATH_H
#define SSGNC_FILE_PATH_H

#include "string-builder.h"

#include <cstdio>

namespace ssgnc {

class FilePath
{
public:
	FilePath() : format_(), path_(), file_id_(-1) {}
	~FilePath() { clear(); }

	void clear();

	void set_format(const String &format);

	String format() const { return format_.str(); }
	String path() const { return path_.str(); }
	Int32 file_id() const { return file_id_; }

	bool next();

	enum { MAX_FILE_ID = 9999 };

private:
	StringBuilder format_;
	StringBuilder path_;
	Int32 file_id_;

	// Disallows copies.
	FilePath(const FilePath &);
	FilePath &operator=(const FilePath &);
};

inline void FilePath::clear()
{
	path_.clear();
	file_id_ = -1;
}

inline void FilePath::set_format(const String &format)
{
	clear();

	format_.assign(format);
	format_.append();
}

inline bool FilePath::next()
{
	if (file_id_ >= MAX_FILE_ID)
		return false;

	Int32 next_file_id = file_id_ + 1;

	path_.resize(path_.length());
	Int32 length = std::snprintf(path_.buf(), path_.length(),
		format_.ptr(), next_file_id);
	if (length < 0)
		return false;
	else if (static_cast<UInt32>(length) >= path_.length())
	{
		path_.resize(length + 1);
		if (std::snprintf(path_.buf(), path_.length(),
			format_.ptr(), next_file_id) != length)
			return false;
	}
	path_.resize(length);
	path_.append();
	file_id_ = next_file_id;
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_FILE_PATH_H
