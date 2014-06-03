#ifndef __STD_RANDOM_H__
#define __STD_RANDOM_H__

#include "std/types.h"

/* Don't pass zero or else... it doesn't work. */
static inline int32_t RandomInt32(int32_t *i) {
  int a = *i;
  a ^= (a << 13);
  a ^= (a >> 17);
  a ^= (a << 5);
  *i = a;
  return a;
}

typedef struct Random {
  uint32_t value;
  uint32_t seed;
} RandomT;

__regargs int NextRandom(RandomT *random);
__regargs float NextRandomFloat(RandomT *random, float lo, float hi);

#endif
