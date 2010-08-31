#ifndef SSGNC_PARSED_LINE_H
#define SSGNC_PARSED_LINE_H

#include <vector>

namespace ssgnc {

class ParsedLine
{
public:
	ParsedLine() : keys_(), value_(0) {}
	~ParsedLine() { Clear(); }

	int key(std::size_t id) const { return keys_[id]; }
	std::size_t num_keys() const { return keys_.size(); }

	void append_key(int key);

	long long value() const { return value_; }
	void set_value(long long value) { value_ = value; }

	void Clear();
	void Swap(ParsedLine *target);

private:
	std::vector<int> keys_;
	long long value_;

	// Disallows copies.
	ParsedLine(const ParsedLine &);
	ParsedLine &operator=(const ParsedLine &);
};

}  // namespace ssgnc

#endif  // SSGNC_PARSED_LINE_H
