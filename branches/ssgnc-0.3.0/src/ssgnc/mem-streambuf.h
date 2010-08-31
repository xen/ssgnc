#ifndef SSGNC_MEM_STREAMBUF_H
#define SSGNC_MEM_STREAMBUF_H

#include <streambuf>

namespace ssgnc {

class MemStreambuf : public std::streambuf
{
public:
	MemStreambuf(const char *buf, std::size_t size)
	{
		char *mutable_buf = const_cast<char *>(buf);
		setg(mutable_buf, mutable_buf, mutable_buf + size);
	}
};

}  // namespace ssgnc

#endif  // SSGNC_MEM_STREAMBUF_H
