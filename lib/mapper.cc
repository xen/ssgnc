#include "ssgnc/mapper.h"

namespace ssgnc {

Mapper::~Mapper()
{
	if (is_open())
		close();
}

bool Mapper::open(const void *ptr, UInt32 size)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (ptr == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (size_ > MAX_SIZE)
	{
		SSGNC_ERROR << "Too large size: " << size_ << std::endl;
		return false;
	}
	ptr_ = ptr;
	size_ = size;
	return true;
}

bool Mapper::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	ptr_ = NULL;
	size_ = 0;
	total_ = 0;
	return true;
}

}  // namespace ssgnc
