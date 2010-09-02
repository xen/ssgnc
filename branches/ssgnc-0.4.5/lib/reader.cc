#include "ssgnc/reader.h"

namespace ssgnc {

Reader::~Reader()
{
	if (is_open())
		close();
}

bool Reader::open(std::istream *stream)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (stream == NULL)
	{
		SSGNC_ERROR << "Null stream" << std::endl;
		return false;
	}
	stream_ = stream;
	return true;
}

bool Reader::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	stream_ = NULL;
	total_ = 0;
	return true;
}

}  // namespace ssgnc
