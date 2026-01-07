#ifndef PRNG_H
#define PRNG_H

#include "defs.hpp"
#include <cassert>
class PRNG {
public:
  PRNG(U64 seed) : seed(seed) { assert(seed); }
  /*
   * Generate random numbers using xorshift64* algorithm.
   * Reference: http://vigna.di.unimi.it/ftp/papers/xorshift.pdf
   * @return random 64bit number.
   */
  U64 rand64() {
    this->seed ^= this->seed >> 12;
    this->seed ^= this->seed << 25;
    this->seed ^= this->seed >> 27;
    return this->seed * 2685821657736338717ULL;
  }
  U64 sparse_rand() { return this->rand64() & this->rand64() & this->rand64(); }

private:
  U64 seed;
};

#endif
