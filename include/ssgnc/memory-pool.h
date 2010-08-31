#ifndef SSGNC_MEMORY_POOL_H
#define SSGNC_MEMORY_POOL_H

#include <vector>

namespace ssgnc {

class MemoryPool
{
public:
	explicit MemoryPool(std::size_t chunk_size = 0);
	~MemoryPool() { Clear(); }

	std::size_t total_size() const { return total_; }

	// Makes a copy of a zero-terminated string and returns its address.
	const char *AppendString(const char *str);
	const char *AppendString(const char *str, std::size_t length);

	// Makes a copy of bytes and returns its address.
	// To copy a zero-terminated string, the `size' must be its length + 1.
	const void *AppendBytes(const void *bytes, std::size_t size);

	void Clear();
	void Swap(MemoryPool *target);

private:
	enum { DEFAULT_CHUNK_SIZE = 1 << 12 };

	std::size_t chunk_size_;
	std::vector<char *> chunks_;
	char *ptr_;
	std::size_t avail_;
	std::size_t total_;

	// Disallows copies.
	MemoryPool(const MemoryPool &);
	MemoryPool &operator=(const MemoryPool &);
};

}  // namespace ssgnc

#endif  // SSGNC_MEMORY_POOL_H
