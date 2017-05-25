// Copyright (c) 2009-2017 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Developers
// Copyright (c) 2014-2017 The Dash Core Developers
// Copyright (c) 2016-2017 Duality Blockchain Solutions Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DYNAMIC_RANDOM_H
#define DYNAMIC_RANDOM_H

#include "crypto/chacha20.h"
#include "crypto/common.h"
#include "uint256.h"

#include <stdint.h>

/**
 * Seed OpenSSL PRNG with additional entropy data
 */
void RandAddSeed();
void RandAddSeedPerfmon();

/**
 * Functions to gather random data via the OpenSSL PRNG
 */
void GetRandBytes(unsigned char* buf, int num);
uint64_t GetRand(uint64_t nMax);
int GetRandInt(int nMax);
uint256 GetRandHash();

/**
 * Fast randomness source. This is seeded once with secure random data, but
 * is completely deterministic and insecure after that.
 * This class is not thread-safe.
 */
class FastRandomContext {
private:
     bool requires_seed;
     ChaCha20 rng;
 
     unsigned char bytebuf[64];
     int bytebuf_size;
 
     uint64_t bitbuf;
     int bitbuf_size;
 
     void RandomSeed();
 
     void FillByteBuffer()
     {
         if (requires_seed) {
             RandomSeed();
         }
         rng.Output(bytebuf, sizeof(bytebuf));
         bytebuf_size = sizeof(bytebuf);
     }
 
     void FillBitBuffer()
     {
         bitbuf = rand64();
         bitbuf_size = 64;
	}
	
public:
    explicit FastRandomContext(bool fDeterministic=false);
    
    /** Initialize with explicit seed (only for testing) */
	explicit FastRandomContext(const uint256& seed);

	/** Generate a random 64-bit integer. */
     uint64_t rand64()
     {
         if (bytebuf_size < 8) FillByteBuffer();
         uint64_t ret = ReadLE64(bytebuf + 64 - bytebuf_size);
         bytebuf_size -= 8;
         return ret;
     }
 
     /** Generate a random (bits)-bit integer. */
     uint64_t randbits(int bits) {
         if (bits == 0) {
             return 0;
         } else if (bits > 32) {
             return rand64() >> (64 - bits);
         } else {
             if (bitbuf_size < bits) FillBitBuffer();
             uint64_t ret = bitbuf & (~(uint64_t)0 >> (64 - bits));
             bitbuf >>= bits;
             bitbuf_size -= bits;
             return ret;
         }
     }
     
    /** Generate a random integer in the range [0..range). */
    uint64_t randrange(uint64_t range)
    {
        --range;
        int bits = CountBits(range);
        while (true) {
            uint64_t ret = randbits(bits);
            if (ret <= range) return ret;
        }
    }

    /** Generate a random 32-bit integer. */
    uint32_t rand32() { return randbits(32); }
 
    /** Generate a random boolean. */
	bool randbool() { return randbits(1); }
};

/**
 * PRNG initialized from secure entropy based RNG
 */
class InsecureRand
{
private:
    uint32_t nRz;
    uint32_t nRw;
    bool fDeterministic;

public:
    InsecureRand(bool _fDeterministic = false);

    /**
     * MWC RNG of George Marsaglia
     * This is intended to be fast. It has a period of 2^59.3, though the
     * least significant 16 bits only have a period of about 2^30.1.
     *
     * @return random value < nMax
     */
    int64_t operator()(int64_t nMax)
    {
        nRz = 36969 * (nRz & 65535) + (nRz >> 16);
        nRw = 18000 * (nRw & 65535) + (nRw >> 16);
        return ((nRw << 16) + nRz) % nMax;
    }
};

#endif // DYNAMIC_RANDOM_H
