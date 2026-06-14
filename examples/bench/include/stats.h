#ifndef __STATS_H__
#define __STATS_H__

#include "utils.h"
#include <lnssim.hpp>
#include <math.h>

// ---------------------------------------------------------------------------
// op_stats — accumulates absolute and relative error for one operation.
//
// avg_abs is the primary metric for add/sub (cancellation-dominated ops).
// avg_rel is the primary metric for mul/div/round-trip.
//
// Both are kept for all ops so the CSV can include the full picture.
//
// Per-sample errors are stored in dynamically-grown arrays:
//   abs_samples[i], rel_samples[i]  — one entry per stats_update() call.
// These are written to samples.bin by csv_write_ops for Wilcoxon testing
// in plot.py.  Call stats_free() after flushing to release the arrays.
// ---------------------------------------------------------------------------

typedef struct {
  f32 
    max_abs_err, min_abs_err,
    max_rel_err, min_rel_err,
    sum_abs_err, sum_rel_err;

  u32 count;

  // Per-sample storage.
  f32
    *abs_samples,
    *rel_samples;

  u32 capacity;
} op_stats;

static inline void stats_init(op_stats* s) {
  *s = (op_stats){
    .max_abs_err = 0.0f,
    .min_abs_err = 1e38f,
    .max_rel_err = 0.0f,
    .min_rel_err = 1e38f,
    .sum_abs_err = 0.0,
    .sum_rel_err = 0.0,
    .count       = 0,
    .abs_samples = NULL,
    .rel_samples = NULL,
    .capacity    = 0
  };
}

static inline void stats_update(op_stats* s, const f32 got, const f32 expected) {
  const f32 
    abs_err = fabsf(got - expected),
    rel_err = fabsf(expected) > 1e-12f ? abs_err / fabsf(expected) : abs_err;

  if (abs_err > s->max_abs_err)
    s->max_abs_err = abs_err;
  if (abs_err < s->min_abs_err)
    s->min_abs_err = abs_err;
  s->sum_abs_err += abs_err;

  if (rel_err > s->max_rel_err)
    s->max_rel_err = rel_err;
  if (rel_err < s->min_rel_err)
    s->min_rel_err = rel_err;
  s->sum_rel_err += rel_err;

  // Grow sample arrays if needed (double capacity, starting at 4096).
  if (s->count >= s->capacity) {
    const u32 new_cap = s->capacity ? s->capacity * 2 : 4096;
    s->abs_samples = (f32*)realloc(s->abs_samples, new_cap * sizeof(f32));
    s->rel_samples = (f32*)realloc(s->rel_samples, new_cap * sizeof(f32));
    s->capacity    = new_cap;
  }

  s->abs_samples[s->count] = abs_err;
  s->rel_samples[s->count] = rel_err;
  s->count++;
}

static inline f32 stats_avg_abs(const op_stats* s) {
  return s->count ? (f32)(s->sum_abs_err / s->count) : 0.0f;
}

static inline f32 stats_avg_rel(const op_stats* s) {
  return s->count ? (f32)(s->sum_rel_err / s->count) : 0.0f;
}

static inline void stats_free(op_stats* s) {
  free(s->abs_samples);
  free(s->rel_samples);
  s->abs_samples = NULL;
  s->rel_samples = NULL;
  s->capacity    = 0;
}

// ---------------------------------------------------------------------------
// format_stats — one op_stats per arithmetic operation tested.
// ---------------------------------------------------------------------------

typedef struct {
  op_stats 
    rt,
    mul,  div_,
    add_, sub_;
} format_stats;

static inline void fstats_init(format_stats* fs) {
  stats_init(&fs->rt);
  stats_init(&fs->mul);
  stats_init(&fs->div_);
  stats_init(&fs->add_);
  stats_init(&fs->sub_);
}

// ---------------------------------------------------------------------------
// scalar_stats — for single-value numerical tests (geometric series, norms…)
// Tracks the final scalar result vs. f64 ground truth.
// ---------------------------------------------------------------------------

typedef struct {
  f32 
    got,
    abs_err,
    rel_err;
} scalar_result;

static inline scalar_result scalar_compare(const f32 got, const f32 expected) {
  return (scalar_result){
    .got     = got,
    .abs_err = fabs(got - expected),
    .rel_err = fabs(expected) > 1e-15 ? fabs(got - expected) / fabs(expected) : fabs(got - expected)
  };
}

#endif // !__STATS_H__
