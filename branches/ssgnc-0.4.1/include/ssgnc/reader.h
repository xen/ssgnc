#ifndef SSGNC_READER_H
#define SSGNC_READER_H

#include "common.h"

namespace ssgnc {

class Reader
{
public:
	explicit Reader(std::istream *stream = NULL)
		: stream_(stream), total_(0) {}
	~Reader();

	bool open(std::istream *stream) SSGNC_WARN_UNUSED_RESULT;
	bool close();

	template <typename T>
	bool read(T *obj) SSGNC_WARN_UNUSED_RESULT;
	template <typename T>
	bool read(T *objs, UInt32 num_objs) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return stream_ != NULL; }

	UInt32 tell() const { return total_; }

	enum { MAX_TOTAL = 0x7FFFFFFF };

private:
	std::istream *stream_;
	UInt32 total_;

	// Disallows copies.
	Reader(const Reader &);
	Reader &operator=(const Reader &);
};

template <typename T>
bool Reader::read(T *obj)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Null stream" << std::endl;
		return false;
	}
	else if (obj == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (total_ + sizeof(T) > MAX_TOTAL)
	{
		SSGNC_ERROR << "Total overflow: " << total_
			<< " + " << sizeof(T) << std::endl;
		return false;
	}

	if (!stream_->read(reinterpret_cast<Int8 *>(obj), sizeof(T)))
	{
		SSGNC_ERROR << "std::iostream::read() failed: " << total_
			<< " + " << sizeof(T) << std::endl;
		return false;
	}

	total_ += static_cast<UInt32>(sizeof(T));
	return true;
}

template <typename T>
bool Reader::read(T *objs, UInt32 num_objs)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Null stream" << std::endl;
		return false;
	}
	else if (objs == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	UInt64 num_bytes = static_cast<UInt64>(sizeof(T)) * num_objs;
	if (total_ + num_bytes > MAX_TOTAL)
	{
		SSGNC_ERROR << "Total overflow: " << total_
			<< " + " << sizeof(T) << " * " << num_objs << std::endl;
		return false;
	}

	if (!stream_->read(reinterpret_cast<Int8 *>(objs),
		sizeof(T) * num_objs))
	{
		SSGNC_ERROR << "std::iostream::read() failed: " << total_
			<< " + " << sizeof(T) << " * " << num_objs << std::endl;
		return false;
	}

	total_ += static_cast<UInt32>(num_bytes);
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_READER_H
