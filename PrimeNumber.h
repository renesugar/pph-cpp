/*
 * Copyright 2017 Rene Sugar
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
 * @file	PrimeNumber.h
 * @author	Rene Sugar <rene.sugar@gmail.com>
 * @brief	Class to find the next prime number after a given number
 *
 * Copyright (c) 2017 Rene Sugar.  All rights reserved.
 **/

#ifndef _PRIMENUMBER_H
#define _PRIMENUMBER_H

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <limits>
#include <numeric>

class PrimeNumber {
public:
  PrimeNumber() {
    x_ = 0;
  }

  explicit PrimeNumber(uint64_t seed): x_(seed) {
  }

  void seed(uint64_t x) {
    x_ = x;
  }

  uint64_t next() {
    uint64_t p = x_;

    // starting odd number
    if ((p & UINT64_C(0x1)) == 0) {
      p++;
    }

    do {
      if (isPrime(p)) {
        // return if prime number
        x_ = p+1;
        return p;
      }

      // next odd number
      p += 2;
    } while (1);
  }

  using result_type = uint64_t;
  static constexpr result_type min() {
    return std::numeric_limits<result_type>::min();
  }
  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }
  result_type operator()() {
    return next();
  }

protected:
  bool isPrime(uint64_t n) {
    uint64_t limit = floor(sqrt(n));
    for (uint64_t i = 2; i < limit; i++) {
      if (n % i == 0) return false;
    }
    return true;
  }

private:
  uint64_t x_;
};

#endif  // _PRIMENUMBER_H
