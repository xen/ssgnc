#ifndef SSGNC_TEXT_FINDER_H
#define SSGNC_TEXT_FINDER_H

#include "file-mapper.h"

#include <cstring>
#include <string>

namespace ssgnc {

class TextFinder
{
public:
	explicit TextFinder(const FileMapper &file)
		: address_(file.Pointer<char>()), size_(file.Size()) {}
	TextFinder(const void *address, std::size_t size)
		: address_(static_cast<const char *>(address)), size_(size) {}

	// Finds an n-gram.
	bool Find(std::size_t position, const char **line) const
	{
		if (position >= size_)
			return false;

		*line = address_ + position;
		return true;
	}

	// Finds an n-gram and copies it to a string.
	bool Find(std::size_t position, std::string *line) const
	{
		const char *pointer;
		if (!Find(position, &pointer))
			return false;

		const char *delim = std::strchr(pointer, '\n');
		std::size_t length = static_cast<std::size_t>(delim - pointer);

		line->assign(pointer, length);
		return true;
	}

private:
	const char *address_;
	std::size_t size_;

	// Disallows copies.
	TextFinder(const TextFinder &);
	TextFinder &operator=(const TextFinder &);
};

}  // namespace ssgnc

#endif  // SSGNC_TEXT_FINDER_H
