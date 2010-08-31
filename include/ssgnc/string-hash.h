#ifndef SSGNC_STRING_HASH_H
#define SSGNC_STRING_HASH_H

#include "string.h"

namespace ssgnc {

class StringHash
{
public:
	UInt32 operator()(const String &str) const;

private:
	static void mix(UInt32 &x, UInt32 &y, UInt32 &z);
	static UInt32 toUInt32(Int8 c) { return static_cast<UInt8>(c); }
};

// http://burtleburtle.net/bob/hash/doobs.html
inline UInt32 StringHash::operator()(const String &str) const
{
	UInt32 x = 0x9E3779B9U;
	UInt32 y = 0x9E3779B9U;
	UInt32 z = 0;
	String avail = str;

	while (avail.length() >= 12)
	{
		x += toUInt32(avail[0]) + (toUInt32(avail[1]) << 8)
			+ (toUInt32(avail[2]) << 16) + (toUInt32(avail[3]) << 24);
		y += toUInt32(avail[4]) + (toUInt32(avail[5]) << 8)
			+ (toUInt32(avail[6]) << 16) + (toUInt32(avail[7]) << 24);
		z += toUInt32(avail[8]) + (toUInt32(avail[9]) << 8)
			+ (toUInt32(avail[10]) << 16) + (toUInt32(avail[11]) << 24);
		mix(x, y, z);
		avail = avail.substr(12);
	}

	z += str.length();
	switch (avail.length())
	{
	case 11:
		z += toUInt32(avail[10]) << 24;
	case 10:
		z += toUInt32(avail[9]) << 16;
	case 9:
		z += toUInt32(avail[8]) << 8;
	case 8:
		y += toUInt32(avail[7]) << 24;
	case 7:
		y += toUInt32(avail[6]) << 16;
	case 6:
		y += toUInt32(avail[5]) << 8;
	case 5:
		y += toUInt32(avail[4]);
	case 4:
		x += toUInt32(avail[3]) << 24;
	case 3:
		x += toUInt32(avail[2]) << 16;
	case 2:
		x += toUInt32(avail[1]) << 8;
	case 1:
		x += toUInt32(avail[0]);
	}

	mix(x, y, z);
	return z;
}

inline void StringHash::mix(UInt32 &x, UInt32 &y, UInt32 &z)
{
	x -= y; x -= z; x ^= (z >> 13);
	y -= z; y -= x; y ^= (x << 8);
	z -= x; z -= y; z ^= (y >> 13);
	x -= y; x -= z; x ^= (z >> 12);
	y -= z; y -= x; y ^= (x << 16);
	z -= x; z -= y; z ^= (y >> 5);
	x -= y; x -= z; x ^= (z >> 3);
	y -= z; y -= x; y ^= (x << 10);
	z -= x; z -= y; z ^= (y >> 15);
}

}  // namespace ssgnc

#endif  // SSGNC_STRING_HASH_H
