#ifndef GOOGLE_NGRAM_MEMORY_STREAM_BUF_H
#define GOOGLE_NGRAM_MEMORY_STREAM_BUF_H

#include <streambuf>

namespace ngram
{

// Stream buffer for file mapping.
class mem_streambuf : public std::streambuf
{
public:
	mem_streambuf(char *buf, std::size_t size)
	{
		char *mutable_buf = const_cast<char *>(buf);
		setp(mutable_buf, mutable_buf + size);
		setg(mutable_buf, mutable_buf, mutable_buf + size);
	}
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_MEMORY_STREAM_BUF_H
