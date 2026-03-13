///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016-2021, Intel Corporation
//
// SPDX-License-Identifier: MIT
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Author(s):  Filip Strugar (filip.strugar@intel.com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "HCoreTypes.h"
#include "HAssert.h"

namespace Horizon
{
	// Written in 2014 by Sebastiano Vigna (vigna@acm.org)
	// (Murmurhash seed bit added by Filip Strugar and it's maybe somehow wrong)
	//
	// To the extent possible under law, the author has dedicated all copyright
	// and related and neighboring rights to this software to the public domain
	// worldwide. This software is distributed without any warranty.
	//
	// See <http://creativecommons.org/publicdomain/zero/1.0/>.
	//
	// This is the fastest generator passing BigCrush without systematic
	// errors, but due to the relatively short period it is acceptable only
	// for applications with a very mild amount of parallelism; otherwise, use
	// a xorshift1024* generator.
	//
	// The state must be seeded so that it is not everywhere zero. If you have
	// a 64-bit seed, we suggest to pass it twice through MurmurHash3's
	// avalanching function.

	class HRandom
	{
		uint64 s[2];

	public:
		// This class can be instantiated, but a singleton is provided as a convenience for global use, when Seed/order is unimportant.
		static HRandom  Singleton;

	public:
		HRandom() { Seed(0); }
		explicit HRandom(int seed) { Seed(seed); }
		~HRandom() = default;

		inline void         Seed(int seed)
		{
			// A modification of https://en.wikipedia.org/wiki/MurmurHash https://sites.google.com/site/murmurhash/ by Austin Appleby in 2008,  under MIT license

			seed = seed ^ ((seed ^ 0x85ebca6b) >> 13) * 0xc2b2ae3;
			s[0] = static_cast<uint64>(seed) * 0xff51afd7ed558ccd;
			s[1] = static_cast<uint64>(seed ^ 0xe6546b64) * 0xc4ceb9fe1a85ec53;
			NextUINT64();
		}

		uint64              NextUINT64() { return next(); }
		int64               NextINT64() { return static_cast<int64>(next()); }

		uint32              NextUINT32() { return static_cast<uint32>(next()); }
		int32               NextINT32() { return static_cast<int32>(next()); }

		// range [0, 1)
		float               NextFloat() { return NextUINT32() / static_cast<float>(1ull << 32); }
		float               NextFloatRange(float rmin, float upbound) { return rmin + (upbound - rmin) * NextFloat(); }

		// for a faster range look at https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
		int32               NextIntRange(int32 range) { uint64 nextVal = next(); return (range == 0) ? (0) : static_cast<int32>(nextVal % uint64(range)); }
		int32               NextIntRange(int32 rmin, int32 rmax) { uint64 nextVal = next(); uint64 delta = rmax - rmin; assert((delta) >= 0); return rmin + ((delta == 0) ? (0) : static_cast<int32>(nextVal % uint64(delta))); }

	private:
		uint64 next(void)
		{
			uint64 s1 = s[0];
			const uint64 s0 = s[1];
			s[0] = s0;
			s1 ^= s1 << 23; // a
			return (s[1] = (s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26))) + s0; // b, c
		}
	};
}