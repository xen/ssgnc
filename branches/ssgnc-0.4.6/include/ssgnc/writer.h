#ifndef SSGNC_WRITER_H
#define SSGNC_WRITER_H

#include "common.h"

namespace ssgnc {

class Writer
{
public:
	explicit Writer(std::ostream *stream = NULL)
		: stream_(stream), total_(0) {}
	~Writer();

	bool open(std::ostream *stream) SSGNC_WARN_UNUSED_RESULT;
	bool close();

	template <typename T>
	bool write(const T &obj) SSGNC_WARN_UNUSED_RESULT;
	template <typename T>
	bool write(const T *objs, UInt32 num_objs) SSGNC_WARN_UNUSED_RESULT;

	bool is_open() const { return stream_ != NULL; }

	bool bad() const { return stream_ == NULL || stream_->bad(); }
	bool eof() const { return stream_ == NULL || stream_->eof(); }
	bool good() const { return stream_ != NULL && stream_->good(); }
	bool fail() const { return stream_ == NULL || stream_->fail(); }

	UInt32 tell() const { return total_; }

	enum { MAX_TOTAL = 0x7FFFFFFF };

private:
	std::ostream *stream_;
	UInt32 total_;

	// Disallows copies.
	Writer(const Writer &);
	Writer &operator=(const Writer &);
};

template <typename T>
bool Writer::write(const T &obj)
{
	if (!is_open())
	{
		SSGNC_ERROR << "Null stream" << std::endl;
		return false;
	}

	if (total_ + sizeof(T) > MAX_TOTAL)
	{
		SSGNC_ERROR << "Total overflow: " << total_
			<< " + " << sizeof(T) << std::endl;
		return false;
	}

	if (!stream_->write(reinterpret_cast<const Int8 *>(&obj), sizeof(T)))
	{
		SSGNC_ERROR << "std::iostream::write() failed: " << total_
			<< " + " << sizeof(T) << std::endl;
		return false;
	}

	total_ += static_cast<UInt32>(sizeof(T));
	return true;
}

template <typename T>
bool Writer::write(const T *objs, UInt32 num_objs)
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

	if (!stream_->write(reinterpret_cast<const Int8 *>(objs),
		sizeof(T) * num_objs))
	{
		SSGNC_ERROR << "std::iostream::write() failed: " << total_
			<< " + " << sizeof(T) << " * " << num_objs << std::endl;
		return false;
	}

	total_ += static_cast<UInt32>(num_bytes);
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_WRITER_H
