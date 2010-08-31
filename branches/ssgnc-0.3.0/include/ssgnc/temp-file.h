#ifndef SSGNC_TEMP_FILE_H
#define SSGNC_TEMP_FILE_H

#include <cstdio>

namespace ssgnc {

class TempFile
{
public:
	TempFile() : file_(NULL) {}
	~TempFile() { Clear(); }

	bool is_open() const { return file_ != NULL; }

	void Open(const char *file_name = NULL, std::size_t buf_size = 0);
	void Close();

	void Read(void *buf, std::size_t size = 1);
	template <typename T>
	void Read(T *buf, std::size_t size = 1)
	{ Read(static_cast<void *>(buf), sizeof(T) * size); }

	void Write(const void *buf, std::size_t size);
	template <typename T>
	void Write(const T *buf, std::size_t size)
	{ Write(static_cast<const void *>(buf), sizeof(T) * size); }

	template <typename T>
	void Write(const T &buf) { Write(&buf, 1); }

	// Disallows writing a pointer.
	template <typename T>
	void Write(const T *buf);

	void Flush();

	std::size_t Tell();
	void Seek(long offset = 0, int whence = SEEK_SET);

	void Clear() { Close(); }
	void Swap(TempFile *target);

private:
	std::FILE *file_;

	// Disallows copies.
	TempFile(const TempFile &);
	TempFile &operator=(const TempFile &);
};

}  // namespace ssgnc

#endif  // SSGNC_TEMP_FILE_H
