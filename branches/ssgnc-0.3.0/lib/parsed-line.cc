#include <ssgnc/parsed-line.h>

#include <ssgnc/exception.h>

namespace ssgnc {

void ParsedLine::append_key(int key)
{
	try
	{
		keys_.push_back(key);
	}
	catch (...)
	{
		SSGNC_THROW("failed to append key: std::vector::push_back() failed");
	}
}

void ParsedLine::Clear()
{
	keys_.clear();
	value_ = 0;
}

void ParsedLine::Swap(ParsedLine *target)
{
	keys_.swap(target->keys_);
	std::swap(value_, target->value_);
}

}  // namespace ssgnc
