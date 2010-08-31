#ifndef SSGNC_FILE_PATH_H
#define SSGNC_FILE_PATH_H

#include "string-builder.h"

namespace ssgnc {

class FilePath
{
public:
	FilePath() : dirname_(), basename_(), file_id_(0) {}
	~FilePath();

	bool open(const String &dirname, const String &basename)
		SSGNC_WARN_UNUSED_RESULT;
	bool close();

	bool read(StringBuilder *path) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return !basename_.empty(); }

	bool seek(ssgnc::Int32 file_id) SSGNC_WARN_UNUSED_RESULT;
	Int32 tell() const { return file_id_; }

	String dirname() const { return dirname_.str(); }
	String basename() const { return basename_.str(); }

	static bool join(const String &dirname, const String &basename,
		StringBuilder *path) SSGNC_WARN_UNUSED_RESULT;

	enum { MAX_FILE_ID = 9999 };

private:
	StringBuilder dirname_;
	StringBuilder basename_;
	Int32 file_id_;

	// Disallows copies.
	FilePath(const FilePath &);
	FilePath &operator=(const FilePath &);
};

}  // namespace ssgnc

#endif  // SSGNC_FILE_PATH_H
