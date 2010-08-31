#ifndef SSGNC_FREQ_HANDLER_H
#define SSGNC_FREQ_HANDLER_H

#include "int-types.h"

namespace ssgnc {

class FreqHandler
{
public:
	FreqHandler() {}
	~FreqHandler() {}

	Int16 encode(Int64 freq) const;
	Int64 decode(Int16 encoded_freq) const;

	// Three 9s and fifteen 0s.
	static const Int64 MAX_FREQ = 999000000000000000LL;
	static const Int16 MAX_ENCODED_FREQ = (0x0F << 10) + 999;

private:
	enum { MAX_RAW_FREQ = 999 };

	// Lower 10 bits for freq base (0 - 999).
	enum { NUM_LOWER_BITS = 10,
		LOWER_MASK = (1 << NUM_LOWER_BITS) - 1 };

	// Upper 4 bits for freq multiplier (1e+0 - 1e+15).
	enum { NUM_UPPER_BITS = 4,
		UPPER_MASK = ((1 << NUM_UPPER_BITS) - 1) << NUM_LOWER_BITS };

	// Disallows copies.
	FreqHandler(const FreqHandler &);
	FreqHandler &operator=(const FreqHandler &);
};

inline Int16 FreqHandler::encode(Int64 freq) const
{
	Int16 encoded_freq = 0;
	while (freq > MAX_RAW_FREQ)
	{
		encoded_freq += 1 << NUM_LOWER_BITS;
		freq /= 10;
	}
	encoded_freq += static_cast<short>(freq);
	return encoded_freq;
}

inline Int64 FreqHandler::decode(Int16 encoded_freq) const
{
	static const Int64 TABLE[] = {
		1LL,
		10LL,
		100LL,
		1000LL,
		10000LL,
		100000LL,
		1000000LL,
		10000000LL,
		100000000LL,
		1000000000LL,
		10000000000LL,
		100000000000LL,
		1000000000000LL,
		10000000000000LL,
		100000000000000LL,
		1000000000000000LL
	};

	Int64 freq = encoded_freq & LOWER_MASK;
	return freq * TABLE[(encoded_freq & UPPER_MASK) >> NUM_LOWER_BITS];
}

}  // namespace ssgnc

#endif  // SSGNC_FREQ_HANDLER_H
