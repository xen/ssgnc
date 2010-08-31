#ifndef SSGNC_FILE_PATH_H
#define SSGNC_FILE_PATH_H

#include "string-builder.h"

namespace ssgnc {

class FilePath
{
public:
	FilePath() : format_(), file_id_(0) {}
	~FilePath();

	bool open(const String &format) SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool read(StringBuilder *path) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return !format_.empty(); }

	bool seek(ssgnc::Int32 file_id) SSGNC_WARN_UNUSED_RESULT;
	Int32 tell() const { return file_id_; }

	String format() const { return format_.str(); }

	enum { MAX_FILE_ID = 9999 };

private:
	StringBuilder format_;
	Int32 file_id_;

	// Disallows copies.
	FilePath(const FilePath &);
	FilePath &operator=(const FilePath &);
};

}  // namespace ssgnc

#endif  // SSGNC_FILE_PATH_H
