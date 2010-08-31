#ifndef SSGNC_QUERY_H
#define SSGNC_QUERY_H

#include <string>
#include <vector>

namespace ssgnc {

class Query
{
public:
	enum Order { UNORDERED, ORDERED, PHRASE, FIXED };

	Query() : key_string_(), key_id_(), min_freq_(0), order_(UNORDERED) {}

	void Clear()
	{
		clear_key_string();
		clear_key_id();
	}

	void clear_key_string() { key_string_.clear(); }
	void add_key_string(const char *key_string)
	{
		key_string_.push_back(key_string);
	}
	void add_key_string(const char *key_string, std::size_t length)
	{
		key_string_.resize(key_string_.size() + 1);
		key_string_.back().assign(key_string, length);
	}
	void add_key_string(const std::string &key_string)
	{
		key_string_.push_back(key_string);
	}
	const std::string *mutable_key_string(int i) { return &key_string_[i]; }
	const std::string &key_string(int i) const { return key_string_[i]; }
	int key_string_size() const
	{
		return static_cast<int>(key_string_.size());
	}

	void clear_key_id() { key_id_.clear(); }
	void add_key_id(int key_id) { key_id_.push_back(key_id); }
	int *mutable_key_id(int i) { return &key_id_[i]; }
	const int &key_id(int i) const { return key_id_[i]; }
	int key_id_size() const { return static_cast<int>(key_id_.size()); }

	void clear_min_freq() { min_freq_ = 0; }
	long long min_freq() const { return min_freq_; }
	void set_min_freq(long long min_freq) { min_freq_ = min_freq; }

	void clear_order() { order_ = UNORDERED; }
	Order order() const { return order_; }
	void set_order(Order order) { order_ = order; }

private:
	std::vector<std::string> key_string_;
	std::vector<int> key_id_;
	long long min_freq_;
	Order order_;

	// Copiable.
};

}  // namespace ssgnc

#endif  // SSGNC_QUERY_H
