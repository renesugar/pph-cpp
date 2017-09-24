/*
 * Copyright 2017 Rene Sugar. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 */

/**
 * @file	pph.h
 * @author	Rene Sugar <rene.sugar@gmail.com>
 * @brief	Minimal order-preserving perfect hash function generator
 *
 * Copyright (c) 2017 Rene Sugar.  All rights reserved.
 **/

// References:
//
// https://goo.gl/e6fHT1
// Practical perfect hashing
// GV Cormack, RNS Horspool, M Kaiserswerth - The Computer Journal, 1985
//

#ifndef _PPH_H
#define _PPH_H

#include <boost/crc.hpp>

#include <deque>
#include <vector>
#include <list>
#include <memory>
#include <sstream>
#include <fstream>
#include <ctime>
#include <string>
#include <map>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <limits>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <random>       // for random_device

#include "SpookyV2.h"

#include "GcdBinary.h"

// http://reveng.sourceforge.net/crc-catalogue/17plus.htm
typedef boost::crc_optimal<64, 0x42f0e1eba9ea3693, 0xffffffffffffffff,
  0xffffffffffffffff, true, true> crc_64_type;

namespace pph {

static constexpr char     EMPTY_STR[] = "";
static constexpr uint64_t EMPTY_VAL   = ((uint64_t)((int64_t)-1));

// http://www.burtleburtle.net/bob/hash/doobs.html
static constexpr uint64_t HASH_MULTIPLIER        = UINT64_C(65);

static constexpr uint64_t KEY_ADJUSTMENT_FACTOR  = UINT64_C(10000000);

static constexpr double   DEFAULT_LOADING_FACTOR = 0.97;

static constexpr uint64_t DEFAULT_TIMEOUT        = UINT64_C(60000);

// Number of multipliers to try before increasing r
static constexpr uint64_t DEFAULT_ATTEMPTS       = UINT64_C(100);

inline uint64_t modulo(uint64_t x, uint64_t y) {
  if ((y & (y-1)) == 0) {
    // y is a power of 2
    return (x & (y-1));
  }
  else {
    return x % y;
  }
}

// http://xoroshiro.di.unimi.it
#include "XorShift1024Star.h"

// More random number generators:
//
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/
// http://www.deshawresearch.com/resources_random123.html
// https://github.com/cslarsen/mersenne-twister
// https://www.fourmilab.ch/hotbits/
// http://www.pcg-random.org
// http://www.pcg-random.org/using-pcg-cpp.html
// http://lemire.me/blog/2017/08/15/on-melissa-oneills-pcg-random-number-generator/amp/
// http://en.cppreference.com/w/cpp/concept/UniformRandomBitGenerator
//

// Floating point numbers:
//
// https://www.doc.ic.ac.uk/~eedwards/compsys/float/
//

#include "PrimeNumber.h"

#include "PowerOfTwo.h"

#include "StringUtil.h"

typedef struct _hdr {
  _hdr() : p_(0), r_(0), i_(0) {}
  // starting index for the group (p)
  uint64_t p_;
  // a parameter (i) to select the second hash function.
  uint32_t i_;
  // the size of the group (r)
  uint32_t r_;

  // if ((p_ == 0) && (r_ == 0) && (i_ == 0)) then slot is free
} hdr_t;

typedef struct _data {
  _data() : key_(EMPTY_STR), val_(0), idx_(0) {}
  _data(const char* k, uint64_t i, uint64_t h) {
    // if (key_ == EMPTY_STR) then slot is free
    key_ = k;
    val_ = i;
    idx_ = h;
  }
  const char* key_;
  uint64_t    val_;  // in empty slots, use for size of free range
  uint64_t    idx_;  // index into H_
} data_t;


// Hash functions:
//
// http://pub.gajendra.net/2012/09/notes_on_collisions_in_a_common_string_hashing_function
// "Note however that just setting the multiplier and table size to be
// relatively prime is not enough to ensure good performance."
// http://www.eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
// http://www.isthe.com/chongo/tech/comp/fnv/
// http://burtleburtle.net/bob/hash/integer.html
// "Full avalanche says that differences in any input bit can cause differences
// in any output bit."
// http://zimbry.blogspot.it/2011/09/better-bit-mixing-improving-on.html
// MurmurHash3Mixer
// https://github.com/aappleby/smhasher
// http://papa.bretmulvey.com/post/124027987928/hash-functions
// Reversible operations
// http://burtleburtle.net/bob/hash/avalanche.html
// http://burtleburtle.net/bob/hash/spooky.html
// http://www.burtleburtle.net/bob/hash/doobs.html
// "The mixing function ought to be a permutation."
// http://www.burtleburtle.net/bob/c/lookup3.c
// http://burtleburtle.net/bob/hash/evahash.html
// http://burtleburtle.net/bob/hash/evahash.html#funneling
// "A hash function is bad if it causes collisions when keys differ in only
// a few bits."
// http://burtleburtle.net/bob/hash/funnels.html
// http://burtleburtle.net/bob/hash/index.html
// https://github.com/google/cityhash
// http://iswsa.acm.org/mphf/index.html
//

typedef uint64_t (*keyfunc_t)(const std::string&, uint64_t, uint64_t);

// UUID: F80F007A-26C3-4BD0-A481-24EE9AE94D01
uint64_t crc64(const std::string& str, uint64_t multiplier, uint64_t adjustment) {
  crc_64_type crc;
  crc.process_bytes(str.data(), str.size());
  return crc.checksum() + adjustment;
}

// UUID: BCC54D42-34F0-43FF-88EB-59C7B47EE210
uint64_t djb_hash(const std::string& str, uint64_t multiplier, uint64_t adjustment) {
  uint64_t keyval = 0;

  for (int i = 0; i < str.size(); i++) {
    keyval = keyval * multiplier ^ static_cast<uint64_t>(str[i]);
  }

  return keyval + adjustment;
}

// UUID: 87333E59-7C1A-4613-9C6F-81F1BB1F6AED
#include "fnv64a_hash.h"

// UUID: A647F03D-A02E-477F-9635-420F3BCEB394
uint64_t spookyV2_hash(const std::string& str, uint64_t multiplier, uint64_t adjustment) {
  return SpookyHash::Hash64(str.c_str(), str.length(), multiplier) + adjustment;
}

// UUID: 3AC2A805-6771-4189-8C62-5F41297126FE
uint64_t oat_hash(const std::string& str, uint64_t multiplier, uint64_t adjustment) {
  uint64_t h = 0;

  for (int i = 0; i < str.size(); i++) {
    h += static_cast<uint64_t>(str[i]);
    h += (h << 10);
    h ^= (h >> 6);
  }

  h += (h << 3);
  h ^= (h >> 11);
  h += (h << 15);

  return h + adjustment;
}

keyfunc_t uuid_to_keyfunc(const std::string& uuid) {
  if (uuid == "F80F007A-26C3-4BD0-A481-24EE9AE94D01") {
    return crc64;
  } else if (uuid == "BCC54D42-34F0-43FF-88EB-59C7B47EE210") {
    return djb_hash;
  } else if (uuid == "87333E59-7C1A-4613-9C6F-81F1BB1F6AED") {
    return fnv64a_hash;
  } else if (uuid == "3AC2A805-6771-4189-8C62-5F41297126FE") {
    return oat_hash;
  } else if (uuid == "A647F03D-A02E-477F-9635-420F3BCEB394") {
    return spookyV2_hash;
  }

  // return djb_hash for unknown UUIDs
  return djb_hash;
}

typedef struct _func {
  _func() : key_(djb_hash), suggestion_(0) {
    add(0, 0, 0);
  }

  uint64_t h_internal(int64_t modulus, uint64_t multiplier,
                      uint64_t adjustment, const std::string& k, int64_t r) {
    return modulo(modulo(key_(k, multiplier, adjustment), modulus), r);
  }

  void reset_suggest_adjustment() {
    suggestion_ = 0;
  }

  // key should be much greater than modulus; if not, it needs to be adjusted
  uint64_t suggest_adjustment(int64_t modulus, uint64_t multiplier,
                              uint64_t adjustment, const char* k) {

    if (k == nullptr) {
      return 0;
    }

    uint64_t key        = key_(k, multiplier, 0);
    uint64_t suggestion = 0;

    if (key < (modulus*pph::KEY_ADJUSTMENT_FACTOR)) {
      suggestion = ((modulus*pph::KEY_ADJUSTMENT_FACTOR) - key);

      suggestion_ = std::max(suggestion_, suggestion);

      return suggestion_;
    }

    return 0;
  }

  void setup(keyfunc_t key) {
    key_ = key;
  }

  bool is_candidate(uint64_t i, uint64_t r) {
    if (gcd_binary(multiplier_[i], r) == 1) {
      return true;
    }

    return false;
  }

  uint64_t h(uint64_t i, const std::string& k, uint64_t r) {
    if (i >= h_.size())
      return 0;

    return h_internal(h_[i], multiplier_[i], adjustment_[i], k, r);
  }

  uint64_t add(uint64_t p, uint64_t m, uint64_t a) {
    h_.push_back(p);
    multiplier_.push_back(m);
    adjustment_.push_back(a);

    return (h_.size() - 1);
  }

  uint64_t modulus(uint64_t i) {
    return h_[i];
  }

  uint64_t multiplier(uint64_t i) {
    return multiplier_[i];
  }

  uint64_t adjustment(uint64_t i) {
    return adjustment_[i];
  }

  uint64_t size() {
    return h_.size();
  }

  // hash function parameter (2i + 100r + 1)
  // for h[i](k,r) = mod (mod (k, 2i + 100r + 1), r)
  //
  // It is implicitly assumed here that k >> 2i + 100r + 1.
  //
  // If it is not, the range of key values should be shifted by adding a large number to k.
  //
  std::vector<uint64_t> h_;
  std::vector<uint64_t> multiplier_;
  std::vector<uint64_t> adjustment_;
  uint64_t suggestion_;
  keyfunc_t key_;
} func_t;

class Table {
public:
  Table(): n_(0), p_(pph::DEFAULT_LOADING_FACTOR), multiplier_(pph::HASH_MULTIPLIER), adjustment_(0),
  uuid_("BCC54D42-34F0-43FF-88EB-59C7B47EE210"),
  timeout_(pph::DEFAULT_TIMEOUT) {
    empty_.key_ = EMPTY_STR;
    empty_.val_ = EMPTY_VAL;
    func_.key_  = djb_hash;
    key_ = djb_hash;
  }

  uint64_t s() {
    return s_;
  }

  uint64_t h(const std::string& k) {
    return modulo(key_(k, multiplier_, adjustment_), s());
  }

  // https://nedbatchelder.com/blog/201310/range_overlap_in_two_compares.html
  bool is_overlap(uint64_t start1, uint64_t end1, uint64_t start2, uint64_t end2) {
    return (end1 >= start2) && (end2 >= start1);
  }

  bool is_adjacent(uint64_t start1, uint64_t end1, uint64_t start2, uint64_t end2) {
    return (end1 + 1) == start2;
  }

  uint64_t find_r(uint64_t src, uint64_t size, uint64_t newsize) {
    uint64_t num_slots = D_.size();
    uint64_t count;

    // find non-overlapping free space

    // if src > newsize
    if (src > newsize) {
      for (int i = 0; i < src; i++) {
        if (D_[i].key_[0] == 0) {
          uint64_t upper = std::min(i+newsize, src);

          count = 0;

          for (int j = i; j < upper; j++) {
            if (D_[j].key_[0] == 0) {
              count++;
            }
          }

          if (count == newsize) {
            return i;
          }
        }
      }
    }

    // not enough free space; reallocate

    // INVARIANT: Free space of the size we're looking for at the end

    D_.resize(num_slots+newsize);
    num_slots = D_.size();

    for (int i = src+size; i < num_slots; i++) {
      if (D_[i].key_[0] == 0) {
        uint64_t upper = std::min(i+newsize, num_slots);

        count = 0;

        for (int j = i; j < upper; j++) {
          if (D_[j].key_[0] == 0) {
            count++;
          }
        }

        if (count == newsize) {
          return i;
        }
      }
    }

    // should never reach here

    return UINT64_MAX;
  }

  // NOTE: The number of items to be hashed (n) needs to be known
  //       when the table is created.
  void setup(uint64_t n, bool use_p, double p, uint64_t timeout = pph::DEFAULT_TIMEOUT,
             uint64_t seed = 0, uint64_t multiplier = pph::HASH_MULTIPLIER,
             uint64_t adjustment = 0, keyfunc_t key = djb_hash) {
    uint64_t s = 0;

    // Multiplier for hash function h()
    multiplier_ = multiplier;

    // Adjustment for hash function h()
    adjustment_ = adjustment;

    // The header size s is chosen such that n = ps, where p is a constant
    // with a value near one; p represents a loading factor for H.

    // Number of items to hash
    n_ = n;

    // Loading factor
    if (p > 1) {
      // default
      p_ = pph::DEFAULT_LOADING_FACTOR;
    } else {
      p_ = p;
    }

    s = (uint64_t(static_cast<double>(n_)/p_));

    // NOTE: Set use_p to true to create more compact hash tables:
    //

    if (use_p == false) {
      // Find next power of two after the calculated value of "s"
      power_.seed(s+1);
      s_ = power_();

      // Update the value of "p"
      p_ = (static_cast<double>(n_)/static_cast<double>(s_));
    }
    else {
      s_ = s;
    }

    // Find multiplier (multiplier_ and s_ should be relatively prime)

    while(gcd_binary(multiplier_, s_) != 1) {
      multiplier_++;
    }

    H_.resize(s_);
    D_.resize(n_);

    func_.setup(key);

    timeout_ = timeout;

    seed_ = seed;
    random_.seed(seed_);
  }

  hdr_t find_h(uint64_t p, uint64_t r, data_t& D, double timeout) {
    auto t_start        = std::chrono::high_resolution_clock::now();
    uint64_t idx        = 0;
    uint64_t modulus    = 0;
    uint64_t multiplier = pph::HASH_MULTIPLIER;
    uint64_t adjustment = 0;
    uint64_t attempts   = 0;
    hdr_t  hdr;

    uint64_t next_r     = r+1;

    hdr.p_ = 0;
    hdr.i_ = 0;
    hdr.r_ = r;

    // The size of r may have to be increased to find a hash function.
    //
    // Example: r = 2
    //
    // even * even = even
    // even * odd  = even
    // odd  * even = even
    // odd  * odd  = odd
    //
    // even + even = even
    // even + odd  = odd
    // odd  + even = even
    // odd  + odd  = even
    //
    // k mod r = k mod 2 = 0 or 1
    //

    // look for an existing hash function with no collisions

    for (uint64_t i = 1; i < func_.size(); i++) {
      if (!func_.is_candidate(i, next_r))
        continue;

      std::vector<bool> collisions(next_r);
      std::fill(collisions.begin(), collisions.end(), false);
      bool found = true;

      // add r+1 data
      idx = func_.h(i, D.key_, next_r);
      collisions[idx] = true;

      // check r data
      for (uint64_t j = 0; j < r; j++) {
        if (D_[p + j].key_[0] == 0)
          continue;

        idx = func_.h(i, D_[p + j].key_, next_r);

        if (collisions[idx] == true) {
          found = false;
          break;
        }

        collisions[idx] = true;
      }

      if (found == true) {
        // found a hash function with no collisions

        hdr.p_ = 0;
        hdr.i_ = i;
        hdr.r_ = next_r;

        return hdr;
      }
    }

    // look for a new hash function with no collisions

    std::vector<bool> collisions;

    attempts = 0;

    for (uint64_t i = 0;; i++) {

      collisions.clear();
      collisions.resize(next_r, false);

      std::fill(collisions.begin(), collisions.end(), false);
      bool found = true;

      // find modulus for h[i]

      // h[i](k,r) = mod (mod (k, 2i + 100r + 1), r)

      // Multiplier for the key function for h[i](k,r) should be
      // relatively prime to (2i + 100r + 1)

      modulus = (2*i + 100*(next_r) + 1);

      std::uniform_int_distribution<uint64_t> dist(modulus, UINT32_MAX);
      modulus = dist(random_);

      if ((modulus & UINT64_C(0x1)) == 0) {
        modulus++;
      }

      // find multiplier (multiplier and next_r should be relatively prime)

      // start with odd number
      multiplier = multiplier_;

      if ((multiplier & UINT64_C(0x1)) == 0) {
        multiplier++;
      }

      // check that the multiplier is relatively prime to the modulus

      while(gcd_binary(multiplier, next_r) != 1) {
        // add 2 to an odd number (want to eliminate powers of 2)
        multiplier++;
        multiplier++;
      }

      // check that the modulus is relatively prime to the multiplier

      while(gcd_binary(modulus, multiplier) != 1) {
        // add 2 to an odd number (want to eliminate powers of 2)
        modulus++;
        modulus++;
      }

      // check if a key adjustment is necessary

      func_.reset_suggest_adjustment();

      adjustment = func_.suggest_adjustment(modulus, multiplier, 0, D.key_);

      for (uint64_t j = 0; j < next_r; j++) {
        adjustment = func_.suggest_adjustment(modulus, multiplier, 0, D_[p + j].key_);
      }

      // add r+1 data
      idx = func_.h_internal(modulus, multiplier, adjustment, D.key_, next_r);
      collisions[idx] = true;

      // check r data
      for (uint64_t j = 0; j < r; j++) {
        if (D_[p + j].key_[0] == 0)
          continue;

        idx = func_.h_internal(modulus, multiplier, adjustment, D_[p + j].key_, next_r);

        if (collisions[idx] == true) {
          found = false;
          break;
        }

        collisions[idx] = true;
      }

      if (found == true) {
        // found a hash function with no collisions

        idx = func_.add(modulus, multiplier, adjustment);

        hdr.p_ = 0;
        hdr.i_ = idx;
        hdr.r_ = next_r;

        return hdr;
      }

      auto t_end = std::chrono::high_resolution_clock::now();
      double t_duration = std::chrono::duration<double, std::milli>(t_end-t_start).count();

      if (t_duration > timeout) {
        break;
      }

      attempts++;

      if (attempts >= DEFAULT_ATTEMPTS) {
        // Try a larger value of r
        next_r++;
        attempts = 0;
      }
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double t_duration = std::chrono::duration<double, std::milli>(t_end-t_start).count();

    // hdr.r_ is zero if a function has not been found

    hdr.p_ = 0;
    hdr.i_ = 0;
    hdr.r_ = 0;

    // not found
    return hdr;
  }

  void move_nonoverlap(uint64_t hidx, uint64_t src, uint64_t dst, uint64_t size,
                       uint64_t m, uint64_t r) {
    uint64_t num_slots = D_.size();
    uint64_t offset    = 0;

    if (src == dst) {
      // nothing to do
      return;
    }

    // Cases:
    //
    // 1) Non-overlapping after
    // src < src+size <= dst < dst+size
    //
    // 2) Non-overlapping before
    // dst < dst+size < src < src+size

    for (int i = 0; i < size; i++) {
      if (D_[src+i].key_[0] == 0)
        continue;

      // NOTE: May have to sort input file by hidx
      //       if a range value is used by another
      //       bucket before the next value is stored.
      //
      // ./pph -i file.txt --index > file_index.txt
      //
      // sort --numeric-sort --key=2 file_index.txt > file_sorted_index.txt
      //
      // awk -F' ' '{print $1}'  file_sorted_index.txt > file_sorted.txt
      //

      // Other ranges can be stored in the unused gaps
      if (D_[src+i].idx_ != hidx)
        continue;

      offset = func_.h(m, D_[src+i].key_, r);

      D_[dst+offset] = D_[src+i];

      // mark as free
      D_[src+i].key_ = EMPTY_STR;
      D_[src+i].val_ = 0;
    }
    return;

    // Since elements are moved into position using a hash function,
    // there is no guarantee that the hash function won't choose an
    // index where the element hasn't been copied yet.

    // In turn, the new position of the element that would be overwritten
    // could be positioned at an index where the element hasn't been copied.

    // And so on.

    // Moving to a non-overlapping location allows the hash function to be used
    // without having to deal with overwrites while copying.

    // 3) Overlapping start
    // src < dst <= src+size < dst+size

    // 4) Overlapping end
    // dst < src <= dst+size < src+size
  }

  bool insert(const char* k, uint64_t v) {
    auto t_start = std::chrono::high_resolution_clock::now();

    uint64_t hidx = h(k);
    hdr_t  hdr = H_[hidx];
    data_t dat;

    if (hdr.r_ == 0) {
      uint64_t y = find_r(0, 1, 1);

      hdr.p_ = y;
      hdr.i_ = 0;
      hdr.r_ = 1;

      H_[hidx] = hdr;

      dat.key_ = k;
      dat.val_ = v;
      dat.idx_ = hidx;

      D_[y] = dat;
    } else {
      // first index of existing r values
      uint64_t p = hdr.p_;
      uint64_t i = hdr.i_;
      uint64_t r = hdr.r_;

      dat.key_ = k;
      dat.val_ = v;
      dat.idx_ = hidx;

      // find a hash function with no collisions for r+1 values
      hdr = find_h(p, r, dat, timeout_);

      if (hdr.r_ == 0) {
        // hash function for r values was not found before timeout
        return false;
      }

      // find free space for next_r values
      uint64_t y = find_r(p, r, hdr.r_);

      if (y == UINT64_MAX) {
        return false;
      }

      // move existing values
      move_nonoverlap(hidx, p, y, r, hdr.i_, hdr.r_);

      // update header with new offset in D_
      hdr.p_ = y;
      // hdr.i_, hdr.r_ already set in call to find_h

      // add new value
      D_[y+func_.h(hdr.i_, dat.key_, hdr.r_)] = dat;

      // update header table
      H_[h(k)] = hdr;
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double t_duration = std::chrono::duration<double, std::milli>(t_end-t_start).count();

    return true;
  }

  uint64_t find_val(const std::string& k) {
    const data_t& dat = find_key(k);

    return dat.val_;
  }

  bool notfound_val(uint64_t v) {
    return (v == EMPTY_VAL);
  }

  void print_index(std::vector<std::string> keys) {
    for (uint64_t i = 0; i < keys.size(); i++) {
      printf("%s %llu\n", keys[i].c_str(), h(keys[i]));
    }
  }

  std::vector<std::string>& keys() {
    return keys_;
  }

  bool load(std::vector<std::string> keys, std::vector<uint64_t> values) {
    bool status = true;

    keys_.reserve(keys.size());

    for (uint64_t i = 0; i < keys.size(); i++) {
      keys_[i] = keys[i];
    }

    for (uint64_t i = 0; i < keys.size(); i++) {
      status = insert(keys_[i].c_str(), values[i]);

      if (status == false) {
        return false;
      }
    }

    return true;
  }

  bool serialize(std::ostream& ostr) {
    ostr << "pph version 1.0.0" << std::endl;

    ostr << std::endl;

    // Write UUID to identify the key function used

    ostr << escape_string(uuid_) << std::endl;

    ostr << std::endl;

    // Write seed

    ostr << seed_ << std::endl;

    ostr << std::endl;

    // Write h_ array size

    ostr << func_.size() << std::endl;

    ostr << std::endl;

    // Write h_ array

    for (int i = 0; i < func_.size(); i++) {
      ostr << i << " " << func_.modulus(i) << " " << func_.multiplier(i) << " " << func_.adjustment(i) << std::endl;
    }

    ostr << std::endl;

    // Write H_ array size, n, p, s, multiplier, adjustment, timeout

    ostr << H_.size() << " " << n_ << " " << p_  << " " << s_ << " " << multiplier_  << " " << adjustment_  << " " << timeout_ << std::endl;

    ostr << std::endl;

    // Write H_ array

    for (int i = 0; i < H_.size(); i++) {
      if (H_[i].r_ == 0)
        continue;

      ostr << i << " " << H_[i].p_ << " " << H_[i].i_ << " " << H_[i].r_ << std::endl;
    }

    ostr << std::endl;

    // Write D_ array size

    ostr << D_.size() << std::endl;

    ostr << std::endl;

    // Write D_ array

    for (int i = 0; i < D_.size(); i++) {
      if (D_[i].key_[0] == 0)
        continue;

      ostr << i << " " << escape_string(D_[i].key_) << " " << D_[i].val_ << " " << D_[i].idx_ << std::endl;
    }

    ostr << std::endl;

    return true;
  }

  bool unserialize(std::istream& istr) {
    std::string  line;
    std::vector<std::string> fields;
    std::vector<uint64_t> indices;
    std::vector<std::string> keys;
    uint64_t     idx;
    uint64_t     p;
    uint64_t     i;
    uint64_t     r;
    uint64_t     h;
    uint64_t     m;
    uint64_t     a;
    hdr_t        hdr;
    data_t       dat;
    std::string  key;
    uint64_t     val;
    uint64_t     s;
    uint64_t     count   = 0;
    uint64_t     size    = 0;
    uint64_t     hidx    = 0;
    std::string::size_type sz;

    // get header line
    std::getline(istr, line);

    line = trim(line);

    if (line != "pph version 1.0.0") {
      return false;
    }

    // empty line
    std::getline(istr, line);

    // Read UUID to identify the key function used

    std::getline(istr, line);

    uuid_ = unescape_string(trim(line));

    // Known UUID converted to key function pointer

    func_.key_ = uuid_to_keyfunc(uuid_);

    // Otherwise: custom key functions should be set by caller using
    //            the UUID read from the table file

    // empty line
    std::getline(istr, line);

    // get seed line
    std::getline(istr, line);

    seed_ = std::atoll(trim(line).c_str());

    // empty line
    std::getline(istr, line);

    // get h_ array size line
    std::getline(istr, line);

    size = std::atoll(trim(line).c_str());

    func_.h_.resize(size, 0);
    func_.multiplier_.resize(size, 0);
    func_.adjustment_.resize(size, 0);

    // empty line
    std::getline(istr, line);

    // read h_ array

    while ( std::getline(istr, line) ) {
      line = trim(line);

      if (line.empty())
        break;

      split(fields, line, " ");

      if (fields.size() != 4) {
        return false;
      }

      idx = std::atoll(fields[0].c_str());
      h   = std::atoll(fields[1].c_str());
      m   = std::atoll(fields[2].c_str());
      a   = std::atoll(fields[3].c_str());

      if (idx >= func_.h_.size()) {
        // size in hash function file is wrong
        return false;
      }

      func_.h_[idx]          = h;
      func_.multiplier_[idx] = m;
      func_.adjustment_[idx] = a;
    }

    // read H_ array size, n, p, s, multiplier, adjustment, timeout
    std::getline(istr, line);

    line = trim(line);

    split(fields, line, " ");

    // H_.size(), n, p, s, multiplier, adjustment, timeout
    if (fields.size() != 7) {
      return false;
    }

    size        = std::atoll(fields[0].c_str());
    n_          = std::atoll(fields[1].c_str());
    p_          = std::stod(fields[2], &sz);
    s_          = std::atoll(fields[3].c_str());
    multiplier_ = std::atoll(fields[4].c_str());
    adjustment_ = std::atoll(fields[5].c_str());
    timeout_    = std::atoll(fields[6].c_str());

    H_.resize(size);

    keys_.resize(n_);

    // empty line
    std::getline(istr, line);

    // read H_ array

    count   = 0;

    while ( std::getline(istr, line) ) {
      line = trim(line);

      if (line.empty())
        break;

      split(fields, line, " ");

      // index p i r
      if (fields.size() != 4) {
        return false;
      }

      idx = std::atoll(fields[0].c_str());
      p   = std::atoll(fields[1].c_str());
      i   = std::atoll(fields[2].c_str());
      r   = std::atoll(fields[3].c_str());

      if (idx >= H_.size()) {
        // size in hash function file is wrong
        return false;
      }

      hdr.p_ = p;
      hdr.i_ = i;
      hdr.r_ = r;

      H_[idx] = hdr;

      count++;
    }

    // get D_ array size line
    std::getline(istr, line);

    size = std::atoll(trim(line).c_str());

    D_.resize(size);

    // empty line
    std::getline(istr, line);

    // read D_ array

    while ( std::getline(istr, line) ) {
      line = trim(line);

      if (line.empty())
        break;

      split(fields, line, " ");

      // index key value
      if (fields.size() != 4) {
        return false;
      }

      idx  = std::atoll(fields[0].c_str());
      key  = unescape_string(fields[1].c_str());
      val  = std::atoll(fields[2].c_str());
      hidx = std::atoll(fields[3].c_str());

      if (idx >= D_.size()) {
        // size in hash function file is wrong
        return false;
      }

      keys.push_back(key);
      indices.push_back(idx);

      // Set dat.key_ after keys_ is built
      dat.val_ = val;
      dat.idx_ = hidx;

      D_[idx] = dat;
    }

    // Set D_ key values after memory allocations in keys_ are done

    for(uint64_t i = 0; i < indices.size(); i++) {
      keys_[i] = keys[i];

      D_[indices[i]].key_ = keys_[i].c_str();
    }

    return true;
  }

  std::string uuid() {
    return uuid_;
  }

  void set_uuid(std::string uuid) {
    uuid_ = uuid;
  }

  void set_keyfunc(keyfunc_t key) {
    func_.key_ = key;
    key_ = key;
  }

protected:
  const data_t& find_key(const std::string& k) {
    auto t_start = std::chrono::high_resolution_clock::now();

    hdr_t  hdr = H_[h(k)];

    if (hdr.r_ == 0) {
      // not found
      return empty_;
    }

    data_t& dat = D_[hdr.p_+func_.h(hdr.i_, k, hdr.r_)];

    if (dat.key_ == k) {
      return dat;
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    double t_duration = std::chrono::duration<double, std::milli>(t_end-t_start).count();

    // not found
    return empty_;
  }

private:
  uint64_t n_;
  double   p_;
  uint64_t s_;
  // Use std::deque instead of std::vector when inserting in D_
  std::vector<hdr_t> H_;
  std::deque<data_t> D_;
  std::vector<std::string> keys_;
  std::string uuid_;
  data_t      empty_;
  func_t      func_;
  keyfunc_t   key_;
  uint64_t    multiplier_;
  uint64_t    adjustment_;
  PrimeNumber prime_;
  PowerOfTwo  power_;
  uint64_t    timeout_;
  XorShift1024Star random_;
  uint64_t    seed_;
};

}  // namespace pph

#endif  // _PPH_H
