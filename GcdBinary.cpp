#include <cstddef>
#include <cstdint>
#include <cstdlib>


#if defined(_MSC_VER) && defined(_WIN64)
#include <intrin.h>
#define CTZ(x) ([&] () {uint64_t _r = 0; _BitScanForward64(&_r,(x)); return _r;} ())
#elif (defined(__clang__) || (defined(__GNUC__) && (__GNUC__>=3)))
#define CTZ(x) __builtin_ctzll((uint64_t)(x))
#else
#include "bitScanForward.h"
#define CTZ(x) bitScanForward((x))
#endif

// https://lemire.me/blog/2013/12/26/fastest-way-to-compute-the-greatest-common-divisor/
//
// References:
//
// https://gist.github.com/cslarsen/1635213
// https://chessprogramming.wikispaces.com/BitScan
// http://chessprogramming.wikispaces.com/De+Bruijn+Sequence+Generator
// https://en.wikipedia.org/wiki/Find_first_set#CTZ
// https://en.wikipedia.org/wiki/De_Bruijn_sequence
// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
//
uint64_t gcd_binary(uint64_t u, uint64_t v) {
  uint64_t shift;
  if (u == 0) return v;
  if (v == 0) return u;
  shift = CTZ(u | v);
  u >>= CTZ(u);
  do {
    v >>= CTZ(v);
    if (u > v) {
      uint64_t t = v;
      v = u;
      u = t;
    }
    v = v - u;
  } while (v != 0);
  return u << shift;
}
