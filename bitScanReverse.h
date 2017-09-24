#ifndef _BITSCANREVERSE_H
#define _BITSCANREVERSE_H

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// https://chessprogramming.wikispaces.com/BitScan
// http://chessprogramming.wikispaces.com/De+Bruijn+Sequence+Generator
// http://aggregate.org/MAGIC/

int bitScanReverse(uint64_t bb);

#if defined(_MSC_VER) && defined(_WIN64)
#include <intrin.h>
#define CLZ(x) ([&] () {uint64_t _r = 0; _BitScanReverse64(&_r,(x)); return _r;} ())
#elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3)))
#define CLZ(x) __builtin_clzll((uint64_t)(x))
#else
#define CLZ(x) bitScanReverse((x))
#endif

#endif  // _BITSCANREVERSE_H
