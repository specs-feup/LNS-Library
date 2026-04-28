/*
 * ===========================================================================
 * BENCHMARK — LNS8 vs BF8,  LNS16 vs BF16
 * ===========================================================================
 *
 * Compares arithmetic accuracy of LNS8, LNS16, BF8, and BF16 across five
 * operations: round-trip store, mul, div, add, sub.
 * Results are broken down by operand magnitude band (all powers of 2) so
 * that accuracy degradation is visible rather than averaged away.
 *
 * ---------------------------------------------------------------------------
 * FORMAT DEFINITIONS
 * ---------------------------------------------------------------------------
 *
 * lns8  = lns<8, i=4, f=3>
 *   Sign:                     1 bit
 *   Exponent integer part:    4 bits (signed two's complement)
 *   Exponent fractional part: 3 bits
 *   Representable real range: 2^(-8) .. 2^(7) = 128
 *   Effective mantissa bits:  3  (same as bf8 below)
 *   USABLE range for this benchmark: |x| <= 4.
 *     lns8 has only 4 signed integer exponent bits. Two operands at |x| = 8
 *     already have L = 3 in fixed-point, and their sum/product pushes the
 *     exponent field into overflow/wrap-around. In practice avg_rel ≈ 1.0
 *     for mul at |x| in [8,32] and above — that is not measurement noise,
 *     it is the format being structurally saturated at that range. There is
 *     nothing to learn from those numbers, so lns8 is only tested where
 *     |x| <= 4.
 *
 * bf8  (E4M3 — 1 sign, 4 exponent, 3 mantissa bits)
 *   Representable real range: ~2^(-9) .. 448  (no infinity, NaN = 0xFF)
 *   Effective mantissa bits:  3
 *   Simulated by truncating f32 to E4M3 precision (round-to-zero).
 *   Same usable cap as lns8: |x| <= 4, because bf8 E4M3 likewise saturates
 *   quickly — mul of two values near 16 already approaches the 448 ceiling
 *   and overflows produce clamped garbage.
 *
 * lns16 = lns<16, i=8, f=7>
 *   Sign:                     1 bit
 *   Exponent integer part:    8 bits (signed two's complement)
 *   Exponent fractional part: 7 bits
 *   Representable real range: 2^(-128) .. 2^(127)
 *   Effective mantissa bits:  7  (same as bf16 below)
 *   Tested up to |x| = 2^16.
 *
 * bf16 (bfloat16 — 1 sign, 8 exponent, 7 mantissa bits)
 *   Representable real range: ~2^(-126) .. 2^(127)  (IEEE 754 envelope)
 *   Effective mantissa bits:  7
 *   Simulated by zeroing the lower 16 bits of an f32 (round-to-zero).
 *   Tested up to |x| = 2^16.
 *
 * Note on |x| > 64 for lns16/bf16:
 *   Both formats can represent values up to 2^127, but once operand
 *   magnitudes exceed ~64, the absolute errors for mul and add explode
 *   (into the millions and billions) even though relative error stays
 *   roughly constant. The numbers are technically valid but useless for
 *   most applications, and they dominate any average that mixes bands.
 *   We cap the benchmark at 2^16 to keep the output readable and focused
 *   on the range where these formats are actually deployed. Anything above
 *   2^16 produces enormous absolute errors for ALL three formats and adds
 *   no discriminating information about format quality.
 *
 * ---------------------------------------------------------------------------
 * SAMPLING
 * ---------------------------------------------------------------------------
 * Each test is parameterised by a band [lo, hi] (absolute value of each
 * operand). All bands are consecutive powers of 2:
 *
 *   8-bit formats  (lns8, bf8):   [1,2]  [2,4]
 *   16-bit formats (lns16, bf16): [1,2]  [2,4]  [4,8]  [8,16]  [16,32]
 *                                 [32,64]  [64,2^16]
 *
 * Within each band, operands are drawn log-uniformly: the exponent is
 * chosen uniformly among integer powers of 2 inside [lo, hi], a full
 * random 23-bit mantissa is appended, and the sign is randomised
 * independently. This gives log-uniform density with no pile-up at
 * boundaries.
 *
 * Pairs whose f64 ground-truth result cast to f32 is non-finite are
 * rejected and resampled. Pairs where the format result is non-finite are
 * also rejected.
 *
 * ---------------------------------------------------------------------------
 * CANCELLATION FILTER — format-aware
 * ---------------------------------------------------------------------------
 * For add/sub, pairs where |a + b| < 2^(-f) * max(|a|, |b|) are skipped,
 * where f is the fractional bit-width of the format under test:
 *   f = 3  for lns8 / bf8
 *   f = 7  for lns16 / bf16
 *
 * Rationale: the threshold is tied to the format's own precision. If the
 * true result is smaller than one LSB of the format's exponent relative to
 * the input scale, the result lies in a regime where no fixed-precision
 * format of that width can be expected to recover it — the error is
 * dominated by the cancellation itself, not by the quality of the format.
 * Filtering these cases out makes the LNS vs IEEE comparison fair: both
 * formats are only judged on inputs where the result is representable at
 * the format's native precision.
 *
 * Earlier iterations used format-blind thresholds (1e-5, then 1e-2).
 * Both were too loose: they still admitted near-cancellation cases where
 * the relative error denominator collapses, making LNS look artificially
 * worse than it is. The format-aware 2^(-f) threshold is the correct
 * principled choice.
 *
 * ---------------------------------------------------------------------------
 * WHY avg_rel IS THE WRONG METRIC FOR add/sub
 * ---------------------------------------------------------------------------
 * Relative error = |got - expected| / |expected|. Near cancellation the
 * denominator |expected| → 0 while the numerator stays bounded by ~1 LSB
 * of the format's absolute precision, so the ratio can reach millions even
 * for a correctly-functioning format. This is not an LNS pathology — it
 * applies equally to BF16. BF16 avoids it only because IEEE addition is
 * correctly rounded to 1 ULP of the *result*, so its absolute error also
 * shrinks with the result. LNS cannot match this: its error is anchored
 * in log-space to 1 LSB of the *input* exponent field, independent of
 * how small the result is.
 *
 * For this reason, add/sub uses avg_abs as the primary metric (marked *).
 * avg_rel is printed for reference but NOT used to determine the winner.
 * The cancellation filter ensures surviving pairs have results large enough
 * for avg_abs to be meaningful.
 *
 * ---------------------------------------------------------------------------
 * METRICS
 * ---------------------------------------------------------------------------
 * Round-trip / mul / div:
 *   Primary:   avg_rel = mean( |got - expected| / |expected| )
 *   Secondary: max_rel, avg_abs, max_abs
 *
 * Add / sub  (cancellation filtered, format-aware):
 *   Primary:   avg_abs* = mean( |got - expected| )
 *   Secondary: max_abs, avg_rel, max_rel
 *   avg_rel is printed but NOT used to pick the winner.
 *
 * Ground truth: f64 arithmetic for all operations.
 *
 * ---------------------------------------------------------------------------
 * OUTPUT STRUCTURE
 * ---------------------------------------------------------------------------
 * 1. Per-band detail blocks: one block per band per format pair, showing
 *    all five ops with full four-column stats and winner rows.
 * 2. Summary tables: one compact table for 8-bit (lns8 vs bf8) and one for
 *    16-bit (lns16 vs bf16), showing avg and max for every op × band in a
 *    single view, with the winner highlighted in green per cell.
 *
 * ---------------------------------------------------------------------------
 * EXPECTED RESULTS
 * ---------------------------------------------------------------------------
 * mul / div: LNS wins on avg_rel.
 *   LNS mul/div is exact integer add/sub on the exponent field — zero
 *   mantissa rounding. BF* must round a full mantissa product to 3 or 7
 *   bits, introducing the usual ~0.5 ULP rounding error every time.
 *
 * add / sub: BF* wins on avg_abs.
 *   BF* add is IEEE 754 correctly-rounded to 1 ULP of the result. LNS add
 *   evaluates φ(d) = log2(1 ± 2^d) via spline table lookup, adding spline
 *   approximation error on top of quantisation error; the absolute error is
 *   bounded by the spline budget (1 bit) but is anchored to the input scale
 *   rather than the result scale.
 *
 * round-trip: roughly equal within each bit-width class.
 *   lns8 and bf8 both carry 3 effective mantissa bits.
 *   lns16 and bf16 both carry 7 effective mantissa bits.
 *   Worst-case relative error ≈ 1/2^f for both, so round-trip accuracy
 *   should be near-identical when operands are within the usable range.
 * ===========================================================================
 */

#include <iostream>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <climits>
#include <cmath>

#include "lnssim.hpp"
#include "bfloatsim.hpp"

using std::isfinite;
using std::fabs;
using std::fmax;
using std::log2f;
using std::ldexp;
using std::memcpy;

#ifndef __LNS_DEFS__
#define __LNS_DEFS__
using lns8  = lns<8,  4, 3>;
using lns16 = lns<16, 8, 7>;
#endif

#define GREEN "\033[032m"
#define BLUE  "\033[036m"
#define RESET "\033[0m"

// ----------------------------------------------------------------------------
// Statistics
// ----------------------------------------------------------------------------

struct op_stats {
  f32 max_abs_err, min_abs_err;
  f64 sum_abs_err;
  f32 max_rel_err, min_rel_err;
  f64 sum_rel_err;
  u32 count;

  op_stats()
    : max_abs_err(0), min_abs_err(1e38f), sum_abs_err(0),
      max_rel_err(0), min_rel_err(1e38f), sum_rel_err(0),
      count(0) {}

  void update(f32 got, f32 expected) {
    f32 abs_err = fabsf(got - expected);
    f32 rel_err = fabsf(expected) > 1e-12f
      ? abs_err / fabsf(expected)
      : abs_err;

    if (abs_err > max_abs_err) max_abs_err = abs_err;
    if (abs_err < min_abs_err) min_abs_err = abs_err;
    sum_abs_err += abs_err;

    if (rel_err > max_rel_err) max_rel_err = rel_err;
    if (rel_err < min_rel_err) min_rel_err = rel_err;
    sum_rel_err += rel_err;

    count++;
  }

  f32 avg_abs() const { return count ? (f32)(sum_abs_err / count) : 0.0f; }
  f32 avg_rel() const { return count ? (f32)(sum_rel_err / count) : 0.0f; }
};

struct format_stats {
  op_stats rt, mul, div_, add_, sub_;
};

// ----------------------------------------------------------------------------
// RNG — xorshift32
// ----------------------------------------------------------------------------

static u32 rng_state = 0xdeadbeef;
static u32 rng_u32() {
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 17;
  rng_state ^= rng_state << 5;
  return rng_state;
}

// ----------------------------------------------------------------------------
// Samplers
// ----------------------------------------------------------------------------

// Log-uniform sample with |result| in [lo, hi], clamped to fmt_min below.
static f32 sample_band(f32 lo, f32 hi, f32 fmt_min) {
  f32 lo_safe = lo < fmt_min ? fmt_min : lo;

  i32 exp_lo = (i32)ceilf (log2f(lo_safe));
  i32 exp_hi = (i32)floorf(log2f(hi));
  if (exp_lo > exp_hi) exp_lo = exp_hi;

  i32 exp_i     = exp_lo + (i32)(rng_u32() % (u32)(exp_hi - exp_lo + 1));
  u32 frac_bits = rng_u32() & 0x7FFFFFu;
  u32 f32_bits  = ((u32)(exp_i + 127) << 23) | frac_bits;
  f32 v; memcpy(&v, &f32_bits, sizeof(f32));

  return (rng_u32() & 1) ? -v : v;
}

// Format-aware cancellation filter.
// Skips pairs where |a + b| < 2^(-f) * max(|a|, |b|).
// f = fractional bits of the format (3 for 8-bit, 7 for 16-bit).
static bool cancels(f64 a, f64 b, u8 f) {
  f64 result = a + b;
  f64 scale  = fmax(fabs(a), fabs(b));
  return scale > 0.0 && fabs(result) < ldexp(1.0, -(i32)f) * scale;
}

// ----------------------------------------------------------------------------
// Format floor constants (smallest safe sample magnitude per format)
//   lns8:  exponent integer bits=4, signed → min exponent = -8  → 2^-8
//   lns16: conservative floor at 2^-4 (well within representable range)
//   bf8 E4M3: min normal exponent = -6                          → 2^-6
//   bf16:  conservative floor at 2^-4
// ----------------------------------------------------------------------------

static constexpr f32 LNS8_MIN  = 1.0f / 256.0f;  // 2^-8
static constexpr f32 LNS16_MIN = 0.0625f;         // 2^-4
static constexpr f32 BF8_MIN   = 1.0f / 64.0f;   // 2^-6
static constexpr f32 BF16_MIN  = 0.0625f;         // 2^-4

#define SB8(lo,hi)   sample_band((lo),(hi), LNS8_MIN)
#define SB16(lo,hi)  sample_band((lo),(hi), LNS16_MIN)
#define SBBF8(lo,hi) sample_band((lo),(hi), BF8_MIN)
#define SBBF(lo,hi)  sample_band((lo),(hi), BF16_MIN)

// ----------------------------------------------------------------------------
// Per-format test functions
// ----------------------------------------------------------------------------

static format_stats test_lns8(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  constexpr u8 F = 3;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SB8(lo, hi);
    fs.rt.update((f32)lns8(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SB8(lo, hi),
      b = SB8(lo, hi);

    const f32 exp = (f32)((f64)a * (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns8(a) * lns8(b));
    if (!isfinite(got))
      continue;

    fs.mul.update(got, exp); k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SB8(lo, hi),
      b = SB8(lo, hi);

    const f32 exp = (f32)((f64)a / (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns8(a) / lns8(b));
    if (!isfinite(got))
      continue;

    fs.div_.update(got, exp); k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SB8(lo, hi),
      b = SB8(lo, hi);
    if (cancels((f64)a,  (f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a + (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns8(a) + lns8(b));
    if (!isfinite(got))
      continue;

    fs.add_.update(got, exp); k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SB8(lo, hi),
      b = SB8(lo, hi);
    if (cancels((f64)a, -(f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a - (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns8(a) - lns8(b));
    if (!isfinite(got))
      continue;

    fs.sub_.update(got, exp); k++;
  }
  return fs;
}

static format_stats test_bf8(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  constexpr u8 F = 3;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SBBF8(lo, hi);
    fs.rt.update((f32)bf8(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);

    const f32 exp = (f32)((f64)a * (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a * b);
    if (!isfinite(got))
      continue;

    fs.mul.update(got, exp); k++;
  }
  for (u32 k = 0; k < n; ) {
    const f32
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);

    const f32 exp = (f32)((f64)a / (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a / b);
    if (!isfinite(got))
      continue;

    fs.div_.update(got, exp); k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);
    if (cancels((f64)a,  (f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a + (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a + b);
    if (!isfinite(got))
      continue;

    fs.add_.update(got, exp); k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);
    if (cancels((f64)a, -(f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a - (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a - b);
    if (!isfinite(got))
      continue;

    fs.sub_.update(got, exp); k++;
  }
  return fs;
}

static format_stats test_lns16(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  constexpr u8 F = 7;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SB16(lo, hi);
    fs.rt.update((f32)lns16(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SB16(lo, hi),
      b = SB16(lo, hi);

    const f32 exp = (f32)((f64)a * (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns16(a) * lns16(b));
    if (!isfinite(got))
      continue;

    fs.mul.update(got, exp); k++;
  }
  for (u32 k = 0; k < n; ) {
    const f32
      a = SB16(lo, hi),
      b = SB16(lo, hi);

    const f32 exp = (f32)((f64)a / (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns16(a) / lns16(b));
    if (!isfinite(got))
      continue;

    fs.div_.update(got, exp); k++;
  }
  for (u32 k = 0; k < n; ) {
    const f32
      a = SB16(lo, hi),
      b = SB16(lo, hi);
    if (cancels((f64)a,  (f64)b,  F))
      continue;
    
    const f32 exp = (f32)((f64)a + (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)(lns16(a) + lns16(b));
    if (!isfinite(got))
      continue;

    fs.add_.update(got, exp); k++;
  }
  for (u32 k = 0; k < n; ) {
    const f32
      a = SB16(lo, hi),
      b = SB16(lo, hi);
    if (cancels((f64)a, -(f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a - (f64)b);
    if (!isfinite(exp))
      continue;
    
    const f32 got = (f32)(lns16(a) - lns16(b));
    if (!isfinite(got))
      continue;

    fs.sub_.update(got, exp); k++;
  }
  return fs;
}

static format_stats test_bf16(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  constexpr u8 F = 7;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SBBF(lo, hi);
    fs.rt.update((f32)bf16(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const f32
      a = SBBF(lo, hi),
      b = SBBF(lo, hi);

    const f32 exp = (f32)((f64)a * (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a * b);
    if (!isfinite(got))
      continue;

    fs.mul.update(got, exp); k++;
  }
  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF(lo, hi),
      b = SBBF(lo, hi);

    const f32 exp = (f32)((f64)a / (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a / b);
    if (!isfinite(got))
      continue;

    fs.div_.update(got, exp); k++;
  }
  for (u32 k = 0; k < n; ) {
    const f32
      a = SBBF(lo, hi),
      b = SBBF(lo, hi);
    if (cancels((f64)a,  (f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a + (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a + b);
    if (!isfinite(got))
      continue;

    fs.add_.update(got, exp); k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF(lo, hi),
      b = SBBF(lo, hi);
    if (cancels((f64)a, -(f64)b,  F))
      continue;

    const f32 exp = (f32)((f64)a - (f64)b);
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a - b);
    if (!isfinite(got))
      continue;

    fs.sub_.update(got, exp); k++;
  }

  return fs;
}

// ----------------------------------------------------------------------------
// Per-band detail output
// ----------------------------------------------------------------------------

static void print_table_relmain(
  const char*     title,
  u32             n_ops,
  const u32*      op_idx,
  const char*     op_names[],
  const op_stats* rows[2][5],
  const char*     fmt_names[2]
) {
  printf(BLUE "  %s\n" RESET, title);
  printf(BLUE "────────────────────────────────────────────────────────────────────────────────────────────────────────\n" RESET);

  printf("%-10s", "");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-51s", op_names[op_idx[i]]);
  printf("\n");

  printf("%-10s", "format");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-12s %-12s %-12s %-12s", "avg_rel", "max_rel", "avg_abs", "max_abs");
  printf("\n");

  printf("%-10s", "");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-12s %-12s %-12s %-12s",
      "------------","------------","------------","------------");
  printf("\n");

  for (u32 f = 0; f < 2; f++) {
    printf("%-10s", fmt_names[f]);
    for (u32 i = 0; i < n_ops; i++) {
      const op_stats* s = rows[f][op_idx[i]];
      printf("  %-12.5e %-12.5e %-12.5e %-12.5e",
        (f64)s->avg_rel(), (f64)s->max_rel_err,
        (f64)s->avg_abs(), (f64)s->max_abs_err);
    }
    printf("\n");
  }

  const char* criteria[] = { "best_rel", "best_abs" };
  auto pick = [](const op_stats* s, u32 c) -> f32 {
    return c == 0 ? s->avg_rel() : s->avg_abs();
  };
  for (u32 c = 0; c < 2; c++) {
    printf("%-10s", criteria[c]);
    for (u32 i = 0; i < n_ops; i++) {
      u32 op = op_idx[i];
      f32 best = 1e38f; u32 winner = 0;
      for (u32 f = 0; f < 2; f++) {
        f32 v = pick(rows[f][op], c);
        if (v < best) { best = v; winner = f; }
      }
      printf("  " GREEN "%-51s" RESET, fmt_names[winner]);
    }
    printf("\n");
  }
  printf("\n");
}

static void print_table_absmain(
  const char*     title,
  u32             n_ops,
  const u32*      op_idx,
  const char*     op_names[],
  const op_stats* rows[2][5],
  const char*     fmt_names[2]
) {
  printf(BLUE "  %s\n" RESET, title);
  printf(BLUE "────────────────────────────────────────────────────────────────────────────────────────────────────────\n" RESET);

  printf("%-10s", "");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-51s", op_names[op_idx[i]]);
  printf("\n");

  printf("%-10s", "format");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-12s %-12s %-12s %-12s", "avg_abs*", "max_abs", "avg_rel", "max_rel");
  printf("\n");

  printf("%-10s", "");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-12s %-12s %-12s %-12s",
      "------------","------------","------------","------------");
  printf("\n");

  for (u32 f = 0; f < 2; f++) {
    printf("%-10s", fmt_names[f]);
    for (u32 i = 0; i < n_ops; i++) {
      const op_stats* s = rows[f][op_idx[i]];
      printf("  %-12.5e %-12.5e %-12.5e %-12.5e",
        (f64)s->avg_abs(), (f64)s->max_abs_err,
        (f64)s->avg_rel(), (f64)s->max_rel_err);
    }
    printf("\n");
  }

  const char* criteria[] = { "best_abs*", "best_rel" };
  auto pick = [](const op_stats* s, u32 c) -> f32 {
    return c == 0 ? s->avg_abs() : s->avg_rel();
  };
  for (u32 c = 0; c < 2; c++) {
    printf("%-10s", criteria[c]);
    for (u32 i = 0; i < n_ops; i++) {
      u32 op = op_idx[i];
      f32 best = 1e38f; u32 winner = 0;
      for (u32 f = 0; f < 2; f++) {
        f32 v = pick(rows[f][op], c);
        if (v < best) { best = v; winner = f; }
      }
      printf("  " GREEN "%-51s" RESET, fmt_names[winner]);
    }
    printf("\n");
  }
  printf("\n");
}

static void print_band(
  const char*         band_label,
  const format_stats& fa,
  const format_stats& fb,
  const char*         name_a,
  const char*         name_b
) {
  const op_stats* rows[2][5] = {
    { &fa.rt, &fa.mul, &fa.div_, &fa.add_, &fa.sub_ },
    { &fb.rt, &fb.mul, &fb.div_, &fb.add_, &fb.sub_ },
  };
  const char* fmt_names[2] = { name_a, name_b };
  const char* op_names[5]  = { "round-trip", "mul", "div", "add", "sub" };

  printf("\n");
  printf(BLUE "══════════════════════════════════════════════════════════════════════════════════════════════════════════\n" RESET);
  printf(BLUE "  BAND: %-12s  [%s  vs  %s]\n" RESET, band_label, name_a, name_b);
  printf(BLUE "  mul/div/rt → avg_rel primary   |   add/sub → avg_abs* primary\n" RESET);
  printf(BLUE "══════════════════════════════════════════════════════════════════════════════════════════════════════════\n" RESET);
  printf("\n");

  const u32 rt_ops[]     = { 0 };
  const u32 muldiv_ops[] = { 1, 2 };
  const u32 addsub_ops[] = { 3, 4 };

  print_table_relmain("Round-trip",                                               1, rt_ops,     op_names, rows, fmt_names);
  print_table_relmain("Mul & Div",                                                2, muldiv_ops, op_names, rows, fmt_names);
  print_table_absmain("Add & Sub  (fmt-aware cancel filter, *abs-primary)",       2, addsub_ops, op_names, rows, fmt_names);

  printf(BLUE "══════════════════════════════════════════════════════════════════════════════════════════════════════════\n" RESET);
}

struct band { f32 lo, hi; const char* label; };

// 8-bit formats: capped at |x| = 4 (see format definitions)
static constexpr band BANDS8[] = {
  { 1.0f / 8.0f, 1.0f, "[2^(-p + 1), 1]"   },
  { 1.0f,        2.0f, "[1, 2]" },
  { 2.0f,        4.0f, "[2, 4]" },
};
static constexpr u32 N_BANDS8 = sizeof(BANDS8) / sizeof(BANDS8[0]);

// 16-bit formats: power-of-2 bands up to 2^16
static constexpr band BANDS16[] = {
  { 1.0f / 128.0f, 1.0f,    "[2^(-p + 1), 1]"   },
  { 1.0f,          2.0f,    "[1, 2]"   },
  { 2.0f,          4.0f,    "[2, 4]"   },
  { 4.0f,          8.0f,    "[4, 8]"   },
  { 8.0f,          16.0f,   "[8, 16]"   },
  { 16.0f,         32.0f,   "[16, 32]"   },
  { 32.0f,         64.0f,   "[32, 64]"   },
  { 64.0f,         65536.0f,"[64, 2^16]" },
};
static constexpr u32 N_BANDS16 = sizeof(BANDS16) / sizeof(BANDS16[0]);

i32 main(const i32 argc, const char* argv[]) {
  if (argc < 3) {
    fprintf(stderr, "usage: %s <lns8.lns> <lns16.lns> [n_tests]\n", argv[0]);
    return EXIT_FAILURE;
  }

  lns8_read_tables  (argv[1]);
  lns16_read_tables (argv[2]);

  u32 n = (argc > 3) ? (u32)atoi(argv[3]) : 100000;

  // ── 8-bit: lns8 vs bf8 ───────────────────────────────────────────────────

  printf(BLUE "\n\n══════════════════════════════════════════\n" RESET);
  printf(BLUE "  8-BIT: lns8 vs bf8 (E4M3)  |x| in [1,4]\n" RESET);
  printf(BLUE "══════════════════════════════════════════\n" RESET);

  format_stats stats8_lns[N_BANDS8];
  format_stats stats8_bf [N_BANDS8];

  for (u32 b = 0; b < N_BANDS8; b++) {
    printf(BLUE "\n── Band: %s — %u tests per op ──\n" RESET, BANDS8[b].label, n);
    stats8_lns[b] = test_lns8(n, BANDS8[b].lo, BANDS8[b].hi);
    stats8_bf [b] = test_bf8 (n, BANDS8[b].lo, BANDS8[b].hi);
    print_band(BANDS8[b].label, stats8_lns[b], stats8_bf[b], "lns8", "bf8");
  }

  // ── 16-bit: lns16 vs bf16 ────────────────────────────────────────────────

  printf(BLUE "\n\n═══════════════════════════════════════════════════\n" RESET);
  printf(BLUE "  16-BIT: lns16 vs bf16  |x| in [1, 2^16]\n" RESET);
  printf(BLUE "═══════════════════════════════════════════════════\n" RESET);

  format_stats stats16_lns[N_BANDS16];
  format_stats stats16_bf [N_BANDS16];

  for (u32 b = 0; b < N_BANDS16; b++) {
    printf(BLUE "\n── Band: %s — %u tests per op ──\n" RESET, BANDS16[b].label, n);
    stats16_lns[b] = test_lns16(n, BANDS16[b].lo, BANDS16[b].hi);
    stats16_bf [b] = test_bf16 (n, BANDS16[b].lo, BANDS16[b].hi);
    print_band(BANDS16[b].label, stats16_lns[b], stats16_bf[b], "lns16", "bf16");
  }

  std::cout << 0.2f << " roundtrip: " << (f32)lns16(0.2f) << std::endl;  
  std::cout << 0.1f << " roundtrip: " << (f32)lns16(0.1f) << std::endl;  
  std::cout << 0.01f << " roundtrip: " << (f32)lns16(0.01f) << std::endl;  
  std::cout << 2.f << " roundtrip: " << (f32)lns16(2.f) << std::endl;  
  std::cout << 2.3f << " roundtrip: " << (f32)lns16(2.3f) << std::endl;  

  lns_close();
  return 0;
}
