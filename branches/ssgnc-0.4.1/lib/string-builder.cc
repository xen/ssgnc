#include "ssgnc/string-builder.h"

#include <cstdarg>

namespace ssgnc {

bool StringBuilder::appendf(const Int8 *format, ...)
{
	std::va_list ap, ap_copy;
	va_start(ap, format);
	va_copy(ap_copy, ap);

	Int32 length = std::vsnprintf(buf_ + length_, size_ - length_,
		format, ap_copy);
	if (length < 0)
		SSGNC_ERROR << "std::vsnprintf() failed: " << format << std::endl;
	else if (length_ + length > size_)
	{
		if (!resizeBuf(length_ + length))
		{
			SSGNC_ERROR << "ssgnc::StringBuilder::resizeBuf() failed: "
				<< (length_ + length) << std::endl;
			length = -1;
		}
		else
			std::vsnprintf(buf_ + length_, size_ - length_, format, ap);
	}

	if (length > 0)
		length_ += length;

	va_end(ap_copy);
	va_end(ap);
	return length >= 0;
}

bool StringBuilder::resizeBuf(UInt32 size)
{
	UInt32 new_buf_size = size_;
	while (new_buf_size < size)
		new_buf_size *= 2;

	Int8 *new_buf;
	try
	{
		new_buf = new Int8[new_buf_size];
	}
	catch (...)
	{
		SSGNC_ERROR << "new ssgnc::Int8[] failed: "
			<< new_buf_size << std::endl;
		return false;
	}
	for (UInt32 i = 0; i < length_; ++i)
		new_buf[i] = buf_[i];

	if (buf_ != initial_buf_)
		delete [] buf_;

	buf_ = new_buf;
	size_ = new_buf_size;
	return true;
}

}  // namespace ssgnc
