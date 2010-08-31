#ifndef SSGNC_FILE_MAP_H
#define SSGNC_FILE_MAP_H

#include "string.h"

namespace ssgnc {

class FileMap
{
public:
	FileMap() : impl_(NULL), ptr_(NULL), size_(0) {}
	~FileMap() { close(); }

	bool open(const Int8 *path);
	void close();

	const void *ptr() const { return ptr_; }
	UInt32 size() const { return size_; }

	String str() const
	{ return String(static_cast<const Int8 *>(ptr_), size_); }

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
