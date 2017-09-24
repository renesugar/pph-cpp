#include "bitScanReverse.h"

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

// https://chessprogramming.wikispaces.com/BitScan
// http://chessprogramming.wikispaces.com/De+Bruijn+Sequence+Generator
// http://aggregate.org/MAGIC/

/**
 * bitScanReverse
 * @authors Kim Walisch, Mark Dickinson
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of most significant one bit
 */
int bitScanReverse(uint64_t bb) {
  // NOTE: Array on web page returns bit positions in wrong order
  static const int index64[64] = {
    63, 16, 62,  7, 15, 36, 61,  3,
    6, 14, 22, 26, 35, 47, 60,  2,
    9,  5, 28, 11, 13, 21, 42, 19,
    25, 31, 34, 40, 46, 52, 59,  1,
    17,  8, 37,  4, 23, 27, 48, 10,
    29, 12, 43, 20, 32, 41, 53, 18,
    38, 24, 49, 30, 44, 33, 54, 39,
    50, 45, 55, 51, 56, 57, 58,  0
  };

  const uint64_t debruijn64 = UINT64_C(0x03f79d71b4cb0a89);
  bb |= bb >> 1;
  bb |= bb >> 2;
  bb |= bb >> 4;
  bb |= bb >> 8;
  bb |= bb >> 16;
  bb |= bb >> 32;
  return index64[(bb * debruijn64) >> 58];
}
