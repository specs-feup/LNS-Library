#ifndef __RNG_H__
#define __RNG_H__

#include "utils.h"
#include <math.h>
#include <string.h>

// ---------------------------------------------------------------------------
// xorshift32 — fast, reproducible, seeded at startup.
// Not cryptographically safe; fine for numerical benchmarks.
// ---------------------------------------------------------------------------

u32 rng_state = 0xdeadbeef;

static inline u32 rng_u32(void) {
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 17;
  rng_state ^= rng_state << 5;
  return rng_state;
}

// Uniform float in [0, 1).
static inline f32 rng_f32(void) {
  return (f32)(rng_u32() & 0x00FFFFFFu) / (f32)0x01000000u;
}

// ---------------------------------------------------------------------------
// sample_interval — log-uniform sample with |result| in [lo, hi].
//
// Strategy: pick an integer exponent uniformly in [ceil(log2(lo)), floor(log2(hi))],
// attach a full random 23-bit significand, randomise the sign.
// This gives log-uniform density with no pile-up at power-of-2 boundaries.
//
// fmt_min clamps lo to the smallest safely-representable value for the format
// under test, preventing denormal/underflow territory where both formats are
// ill-defined.
// ---------------------------------------------------------------------------

static inline f32 sample_interval(f32 lo, f32 hi, f32 fmt_min) {
  f32 lo_safe = lo < fmt_min ? fmt_min : lo;

  i32 exp_lo = (i32)ceilf (log2f(lo_safe));
  i32 exp_hi = (i32)floorf(log2f(hi));
  if (exp_lo > exp_hi)
    exp_lo = exp_hi;

  i32 range  = exp_hi - exp_lo + 1;
  i32 exp_i  = exp_lo + (i32)(rng_u32() % (u32)range);
  u32 frac   = rng_u32() & 0x7FFFFFu;
  u32 bits   = ((u32)(exp_i + 127) << 23) | frac;

  f32 v;
  memcpy(&v, &bits, sizeof(f32));
  return (rng_u32() & 1) ? -v : v;
}

// ---------------------------------------------------------------------------
// Format floor constants — smallest magnitude sampled per format.
// Keeps sampling away from denormal/overflow territory where both formats
// saturate and produce no discriminating information.
//
//   lns8:  4-bit signed integer exponent → min exponent = -8  → 2^-8
//   lns16: conservative floor at 2^-4  (well within representable range)
//   bf8 E4M3: min normal exp = -6                             → 2^-6
//   bf16:  conservative floor at 2^-4
// ---------------------------------------------------------------------------

#define LNS8_MIN   (1.0f / 256.0f)
#define LNS16_MIN  (0.0625f)
#define BF8_MIN    (1.0f / 64.0f)
#define BF16_MIN   (0.0625f)

#define SB8(lo,hi)    sample_interval((lo), (hi), LNS8_MIN)
#define SB16(lo,hi)   sample_interval((lo), (hi), LNS16_MIN)
#define SBBF8(lo,hi)  sample_interval((lo), (hi), BF8_MIN)
#define SBBF16(lo,hi) sample_interval((lo), (hi), BF16_MIN)

#endif // !__RNG_H__
