#ifndef GOOGLE_NGRAM_KEY_TYPE_H
#define GOOGLE_NGRAM_KEY_TYPE_H

#include <string>

namespace ngram
{

class key_type
{
public:
	key_type() : key_(), freq_(0) {}
	explicit key_type(const std::string &key, long long freq = 0)
		: key_(key), freq_(freq) {}
	explicit key_type(const char *key, long long freq = 0)
		: key_(key), freq_(freq) {}
	key_type(const char *key, std::size_t length, long long freq = 0)
		: key_(key, length), freq_(freq) {}

	void set_key(const std::string &key) { key_.assign(key); }
	void set_key(const char *key) { key_.assign(key); }
	void set_key(const char *key, std::size_t length)
	{ key_.assign(key, length); }

	void set_freq(long long freq) { freq_ = freq; }

	const char *c_str() const { return key_.c_str(); }
	char &operator[](std::size_t index) { return key_[index]; }
	const char &operator[](std::size_t index) const { return key_[index]; }
	std::size_t length() const { return key_.length(); }
	std::size_t size() const { return key_.size(); }

	const std::string &key() const { return key_; }
	long long freq() const { return freq_; }

	void swap(key_type &key)
	{
		key_.swap(key.key_);
		std::swap(freq_, key.freq_);
	}

private:
	std::string key_;
	long long freq_;
};

inline bool operator==(const key_type &lhs, const key_type &rhs)
{ return lhs.key() == rhs.key(); }
inline bool operator!=(const key_type &lhs, const key_type &rhs)
{ return !(lhs == rhs); }
inline bool operator<(const key_type &lhs, const key_type &rhs)
{ return lhs.key() < rhs.key(); }
inline bool operator>(const key_type &lhs, const key_type &rhs)
{ return rhs < lhs; }

}  // namespace ngram

namespace std
{

inline void swap(ngram::key_type &lhs, ngram::key_type &rhs) { lhs.swap(rhs); }

}  // namespace std

#endif  // GOOGLE_NGRAM_KEY_TYPE_H
