#ifndef GOOGLE_NGRAM_KEY_WRITER_H
#define GOOGLE_NGRAM_KEY_WRITER_H

#include "key-type.h"
#include "vbe-writer.h"

#include <iostream>

namespace ngram
{

class key_writer
{
public:
	explicit key_writer(std::ostream &stream)
		: byte_writer_(stream), vbe_writer_(byte_writer_) {}

	void write(const key_type &key)
	{
		vbe_writer_ << key.freq();
		for (std::size_t i = 0; i < key.size(); ++i)
			byte_writer_.put(key[i]);
	}

protected:
	byte_writer byte_writer_;
	vbe_writer vbe_writer_;

private:
	// Copies are not allowed.
	key_writer(const key_writer &);
	key_writer &operator=(const key_writer &);
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_KEY_WRITER_H
