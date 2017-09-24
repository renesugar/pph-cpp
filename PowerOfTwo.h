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
 * @file	PowerOfTwo.h
 * @author	Rene Sugar <rene.sugar@gmail.com>
 * @brief	Class to round up to the nearest power of two given a number
 *
 * Copyright (c) 2017 Rene Sugar.  All rights reserved.
 **/

#ifndef _POWEROFTWO_H
#define _POWEROFTWO_H

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <limits>
#include <numeric>

// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

class PowerOfTwo {
public:
  PowerOfTwo() {
    x_ = 0;
  }

  explicit PowerOfTwo(uint64_t seed): x_(seed) {
  }

  void seed(uint64_t x) {
    x_ = x;
  }

  uint64_t next() {
    uint64_t p = RoundUpPowerOf2(x_);
    x_ = p+1;
    return p;
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
  uint64_t RoundUpPowerOf2(uint64_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v |= v >> 32;
    v++;

    return v;
  }

private:
  uint64_t x_;
};

#endif  // _POWEROFTWO_H
