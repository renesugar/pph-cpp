#ifndef _XORSHIFT1024STAR_H
#define _XORSHIFT1024STAR_H

// http://xoroshiro.di.unimi.it

class SplitMix64 {
  /*  Written in 2015 by Sebastiano Vigna (vigna@acm.org)

   To the extent possible under law, the author has dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   See <http://creativecommons.org/publicdomain/zero/1.0/>. */

  /* This is a fixed-increment version of Java 8's SplittableRandom generator
   See http://dx.doi.org/10.1145/2714064.2660195 and
   http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html

   It is a very fast generator passing BigCrush, and it can be useful if
   for some reason you absolutely want 64 bits of state; otherwise, we
   rather suggest to use a xoroshiro128+ (for moderately parallel
   computations) or xorshift1024* (for massively parallel computations)
   generator. */

public:
  SplitMix64() {
    x_ = 0;
  }

  explicit SplitMix64(uint64_t seed): x_(seed) {
  }

  void seed(uint64_t x) {
    x_ = x;
  }

  uint64_t next() {
    uint64_t z = (x_ += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
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

private:
  uint64_t x_; /* The state can be seeded with any value. */
};

class XorShift1024Star {
  /*  Written in 2014 by Sebastiano Vigna (vigna@acm.org)

   To the extent possible under law, the author has dedicated all copyright
   and related and neighboring rights to this software to the public domain
   worldwide. This software is distributed without any warranty.

   See <http://creativecommons.org/publicdomain/zero/1.0/>. */

  /* This is a fast, top-quality generator. If 1024 bits of state are too
   much, try a xoroshiro128+ generator.

   Note that the three lowest bits of this generator are LSFRs, and thus
   they are slightly less random than the other bits. We suggest to use a
   sign test to extract a random Boolean value.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s. */
public:
  explicit XorShift1024Star(uint64_t x) {
    Init(x);
    p_ = 0;
  }

  XorShift1024Star() {
    p_ = 0;
  }

  void seed(uint64_t x) {
    Init(x);
  }

  uint64_t next() {
    const uint64_t s0 = s_[p_];
    uint64_t s1 = s_[p_ = ((p_ + 1) & 15)];
    s1 ^= s1 << 31;  // a
    s_[p_] = s1 ^ s0 ^ (s1 >> 11) ^ (s0 >> 30);  // b,c
    return s_[p_] * UINT64_C(1181783497276652981);
  }

  /* This is the jump function for the generator. It is equivalent
   to 2^512 calls to next(); it can be used to generate 2^512
   non-overlapping subsequences for parallel computations. */

  void jump() {
    static const uint64_t JUMP[] = { 0x84242f96eca9c41d,
      0xa3c65b8776f96855, 0x5b34a39f070b5837, 0x4489affce4f31a1e,
      0x2ffeeb0a48316f40, 0xdc2d9891fe68c022, 0x3659132bb12fea70,
      0xaac17d8efa43cab8, 0xc4cb815590989b13, 0x5ee975283d71c93b,
      0x691548c86c1bd540, 0x7910c41d10a1e6a5, 0x0b5fc64563b3e2a8,
      0x047f7684e9fc949d, 0xb99181f2d8f685ca, 0x284600e3f30e38c3
    };

    uint64_t t[16] = { 0 };
    for (int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
      for (int b = 0; b < 64; b++) {
        if (JUMP[i] & UINT64_C(1) << b) {
          for (int j = 0; j < 16; j++) {
            t[j] ^= s_[(j + p_) & 15];
          }
        }
        next();
      }

    for (int j = 0; j < 16; j++) {
      s_[(j + p_) & 15] = t[j];
    }
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

private:
  void Init(uint64_t x) {
    SplittableRandom_.seed(x);
    for (size_t i = 0; i < (sizeof(s_)/sizeof(s_[0])); i++) {
      s_[i] = SplittableRandom_.next();
    }
  }

  SplitMix64 SplittableRandom_;
  uint64_t s_[16];
  int p_;
};

#endif  // _XORSHIFT1024STAR_H
