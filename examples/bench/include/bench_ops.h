#ifndef __BENCH_OPS_H__
#define __BENCH_OPS_H__

#include "utils.h"
#include "stats.h"

// ---------------------------------------------------------------------------
// bench_ops — LNS vs BF arithmetic accuracy across magnitude intervals.
//
// Tests five operations (round-trip, mul, div, add, sub) at each interval,
// comparing lns8 vs bf8 and lns16 vs bf16.
//
// Exhaustive tables for lns8 and lns16 are read from .lns files once at
// startup (via lns8_read_tables / lns16_read_tables from lnssim.hpp).
// Because lns8 has only 256 representable values and lns16 has 65536,
// these tables fit comfortably in memory and make per-sample arithmetic
// a pure table lookup.
//
// Band layout
// -----------
//  8-bit  (lns8, bf8):   |x| in {[2^(-p+1),1], [1,2], [2,4]}
//  16-bit (lns16, bf16): |x| in {[2^(-p+1),1], [1,2], [2,4], [4,8],
//                                [8,16], [16,32], [32,64], [64,2^16]}
//
// The 8-bit cap at |x| = 4 prevents the format's 4-bit signed exponent
// from wrapping on mul/add results.
//
// Cancellation filter (add/sub)
// ------------------------------
// Pairs where |a+b| < 2^(-f) * max(|a|,|b|) are skipped (f = frac bits).
// This is format-aware: lns8/bf8 use f=3, lns16/bf16 use f=7.
// See the README for justification.
//
// Primary metrics
// ---------------
//  mul / div / round-trip : avg_rel
//  add / sub              : avg_abs   (avg_rel printed but not used to rank)
// ---------------------------------------------------------------------------

typedef struct {
  f32 
    lo, hi;
  const char* label;
} interval_t;

// Exported interval tables so bench_numerical and main can iterate them
// if needed for range context.
extern const interval_t BANDS8[];
extern const interval_t BANDS16[];
extern const u32        N_BANDS8;
extern const u32        N_BANDS16;

// Run the full ops benchmark for both format pairs.
// results_dir is passed so this module can call csv_write_ops directly.
void bench_ops_run(u32 n_samples, const char* results_dir);

#endif // !__BENCH_OPS_H__
