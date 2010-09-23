#include "ssgnc.h"

#include <algorithm>
#include <cassert>
#include <ctime>

bool compareFreqs(ssgnc::Int64 lhs, ssgnc::Int64 rhs)
{
	while (lhs >= 1000 && rhs >= 1000)
	{
		lhs /= 10;
		rhs /= 10;
	}

	return lhs == rhs;
}

int main()
{
	enum { NUM_FREQS = 1 << 16 };

	ssgnc::FreqHandler handler;

	std::srand(static_cast<unsigned>(std::time(NULL)));

	std::vector<ssgnc::Int64> freqs(NUM_FREQS);
	std::vector<ssgnc::Int16> encoded_freqs(NUM_FREQS);

	for (std::size_t i = 0; i < freqs.size(); ++i)
	{
		freqs[i] = static_cast<ssgnc::Int64>(std::rand())
			* (static_cast<ssgnc::Int64>(RAND_MAX) + 1);
		freqs[i] |= std::rand();
		freqs[i] %= ssgnc::FreqHandler::MAX_FREQ - 1;
		freqs[i] += 1;
	}

	std::sort(freqs.begin(), freqs.end());

	for (std::size_t i = 0; i < encoded_freqs.size(); ++i)
		assert(handler.encode(freqs[i], &encoded_freqs[i]));

	for (std::size_t i = 1; i < encoded_freqs.size(); ++i)
		assert(encoded_freqs[i - 1] <= encoded_freqs[i]);

	for (std::size_t i = 1; i < encoded_freqs.size(); ++i)
	{
		ssgnc::Int64 decoded_freq;
		assert(handler.decode(encoded_freqs[i], &decoded_freq));
		assert(decoded_freq <= freqs[i]);
		assert(compareFreqs(decoded_freq, freqs[i]));
	}

	for (ssgnc::Int64 freq = 1; freq < 1000; ++freq)
	{
		ssgnc::Int16 encoded_freq;
		assert(handler.encode(freq, &encoded_freq));
		assert(encoded_freq == freq);

		ssgnc::Int64 decoded_freq;
		assert(handler.decode(encoded_freq, &decoded_freq));
		assert(decoded_freq == freq);
	}

	ssgnc::Int16 temp_encoded_freq;
	assert(handler.encode(ssgnc::FreqHandler::MAX_FREQ, &temp_encoded_freq));
	assert(temp_encoded_freq == ssgnc::FreqHandler::MAX_ENCODED_FREQ);

	ssgnc::Int64 temp_freq;
	assert(handler.decode(ssgnc::FreqHandler::MAX_ENCODED_FREQ, &temp_freq));
	assert(temp_freq == ssgnc::FreqHandler::MAX_FREQ);

	return 0;
}
