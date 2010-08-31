#include "ssgnc/freq-handler.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <vector>

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
		freqs[i] = std::abs((static_cast<ssgnc::Int64>(std::rand()) << 32)
			| (std::rand() & 0xFFFFFFFF)) % ssgnc::FreqHandler::MAX_FREQ;
	}

	std::sort(freqs.begin(), freqs.end());

	for (std::size_t i = 0; i < encoded_freqs.size(); ++i)
		encoded_freqs[i] = handler.encode(freqs[i]);

	for (std::size_t i = 1; i < encoded_freqs.size(); ++i)
		assert(encoded_freqs[i - 1] <= encoded_freqs[i]);

	for (std::size_t i = 1; i < encoded_freqs.size(); ++i)
	{
		ssgnc::Int64 decoded_freq = handler.decode(encoded_freqs[i]);
		assert(decoded_freq <= freqs[i]);
		assert(compareFreqs(decoded_freq, freqs[i]));
	}

	return 0;
}
