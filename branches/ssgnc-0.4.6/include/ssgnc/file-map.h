#ifndef SSGNC_FILE_MAP_H
#define SSGNC_FILE_MAP_H

#include "common.h"

namespace ssgnc {

class FileMap
{
public:
	enum Mode { MMAP_FILE, READ_FILE, DEFAULT_MODE = MMAP_FILE };

	FileMap() : impl_(NULL), ptr_(NULL), size_(0) {}
	~FileMap();

	bool open(const Int8 *path, Mode mode = DEFAULT_MODE)
		SSGNC_WARN_UNUSED_RESULT;
	bool close();

	const void *ptr() const { return ptr_; }
	UInt32 size() const { return size_; }

	bool is_open() const { return impl_ != NULL; }

	enum { MAX_FILE_SIZE = 0x7FFFFFFFU };

private:
	class Impl;

	Impl *impl_;
	const void *ptr_;
	UInt32 size_;

	// Disallows copies.
	FileMap(const FileMap &);
	FileMap &operator=(const FileMap &);
};

}  // namespace ssgnc

#endif  // SSGNC_FILE_MAP_H
