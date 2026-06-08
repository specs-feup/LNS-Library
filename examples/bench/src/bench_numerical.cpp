#include "bench_numerical.h"
#include "stats.h"
#include "csv.h"
#include "rng.h"
#include "lnssim.hpp"
#include "bfloatsim.hpp"

#include <math.h>

// ---------------------------------------------------------------------------
// 1. Geometric progression:  a_n = a_0 * r^n
//    a_0=1, r=1.015, n=100 iterations.
//
//    pow(1.015, 100) ≈ 4.43, safely inside (2^-p, 16) for all formats.
//    Each step is a single multiply.  LNS mul is exact integer add in
//    log-space; BF16 mul rounds the full mantissa product every step.
//    Over 100 steps the rounding accumulates.  The reference is computed
//    with the same loop to match iteration order exactly.
// ---------------------------------------------------------------------------

static scalar_result geom_lns16(void) {
  lns16 acc     = lns16(1.0f);
  const lns16 r = lns16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc *= r;

  const lns32 expected = lns32((f32)pow(1.015, 100.0));
  return scalar_compare((f64)acc, (f64)expected);
}

static scalar_result geom_bf16(void) {
  bf16 acc     = bf16(1.0f);
  const bf16 r = bf16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc = bf16((f32)acc * (f32)r);

  const f32 expected = (f32)pow(1.015, 100.0);
  return scalar_compare((f64)(f32)acc, (f64)expected);
}

static scalar_result geom_lns8(void) {
  lns8 acc     = lns8(1.0f);
  const lns8 r = lns8(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc *= r;

  const lns32 expected = lns32((f32)pow(1.015, 100.0));
  return scalar_compare((f64)acc, (f64)expected);
}

static scalar_result geom_bf8(void) {
  bf8 acc     = bf8(1.0f);
  const bf8 r = bf8(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc = bf8((f32)acc * (f32)r);

  const f32 expected = (f32)pow(1.015, 100.0);
  return scalar_compare((f64)(f32)acc, (f64)expected);
}

// ---------------------------------------------------------------------------
// 2. Euclidean norm:  ||x|| = sqrt(sum(x_i^2))
//    N=4096 values sampled log-uniformly in [0.01, 100].
//
//    The raw norm ≈ 1500 which overflows lns8 and inflates absolute
//    errors.  We normalise by dividing by 256 so the expected result
//    falls in (1, 16) for typical sample sets, well inside all formats'
//    representable range.  The division is applied to each input sample
//    before squaring so the computation path is identical across formats.
//
//    Squaring amplifies relative error; summing N terms amplifies abs
//    error additively.  LNS squaring is exact (double the exponent);
//    LNS add is the weakness.  BF16 add is correctly-rounded per step.
//    We freeze the sample so both formats operate on identical inputs.
// ---------------------------------------------------------------------------

#define NORM_N   4096
#define NORM_DIV 256.0f

static f32 g_norm_samples[NORM_N];

static void norm_init_samples(void) {
  for (i32 i = 0; i < NORM_N; i++)
    g_norm_samples[i] = sample_band(0.01f, 100.0f, 0.01f) / NORM_DIV;
}

static scalar_result norm_lns16(void) {
  lns16 acc = lns16(0.0f);
  for (i32 i = 0; i < NORM_N; i++) {
    const lns16 v = lns16(g_norm_samples[i]);
    acc = acc + v * v;
  }

  f64 expected = 0.0;
  for (i32 i = 0; i < NORM_N; i++)
    expected += (f64)g_norm_samples[i] * (f64)g_norm_samples[i];

  expected = sqrt(expected);
  return scalar_compare((f64)(f32)acc.sqrt(), expected);
}

static scalar_result norm_bf16(void) {
  f32 acc = 0.0f;
  for (i32 i = 0; i < NORM_N; i++) {
    const f32 v = (f32)bf16(g_norm_samples[i]);
    acc = (f32)bf16(acc + v * v);
  }

  f32 expected = 0.0f;
  for (i32 i = 0; i < NORM_N; i++)
    expected += g_norm_samples[i] * g_norm_samples[i];

  expected = sqrtf(expected);
  return scalar_compare((f64)(f32)bf16(sqrtf(acc)), (f64)expected);
}

// ---------------------------------------------------------------------------
// 3. Alternating harmonic series (Leibniz pi):
//    4 * sum_{k=0}^{9999} (-1)^k / (2k+1)
//
//    Massive cancellation: positive and negative terms nearly cancel.
//    The result converges slowly to pi but each partial sum swings far.
//    Both formats struggle; the test reveals how badly LNS add error
//    grows relative to BF16's correctly-rounded add.
//
//    The final result pi ≈ 3.14 is already inside (2^-p, 16).
// ---------------------------------------------------------------------------

static scalar_result leibniz_lns16(void) {
  lns16 acc = lns16(0.0f);
  for (i32 k = 0; k < 10000; k++) {
    const lns16 term = lns16(1.0f / (f32)(2 * k + 1));
    acc = (k & 1) ? acc - term : acc + term;
  }

  const lns16 result = lns16(4.0f) * acc;
  return scalar_compare((f64)result, (f64)lns32((f32)M_PI));
}

static scalar_result leibniz_bf16(void) {
  f32 acc = 0.0f;
  for (i32 k = 0; k < 10000; k++) {
    const f32 term = (f32)bf16(1.0f / (f32)(2 * k + 1));
    acc = (k & 1)
      ? (f32)bf16(acc - term)
      : (f32)bf16(acc + term);
  }

  const f32 result   = (f32)bf16(4.0f * acc);
  const f32 expected = (f32)M_PI;
  return scalar_compare((f64)result, (f64)expected);
}

// ---------------------------------------------------------------------------
// 4. Large-to-small accumulation: sum_{n=1}^{5000} 1/n^2 = pi^2/6
//    Computed forward (n=1..N) and backward (n=N..1).
//
//    pi^2/6 ≈ 1.645, already inside (2^-p, 16) for all formats.
//
//    Forward: adds tiny 1/n^2 increments to a large accumulator —
//    exercises the regime where BF16 add wins (correctly rounded to result).
//    Backward: adds small to small first, which should help LNS by keeping
//    operand magnitudes similar.  Comparing the two orderings isolates
//    the ordering effect from format precision.
// ---------------------------------------------------------------------------

#define ACCUM_N 5000
static const f32 PI2_OVER_6_F32 = (f32)((M_PI * M_PI) / 6.0);

static scalar_result accum_lns16_fwd(void) {
  lns16 acc = lns16(0.0f);

  for (i32 n = 1; n <= ACCUM_N; n++) {
    const lns16 v = lns16(1.0f / (f32)(n * n));
    acc = acc + v;
  }

  return scalar_compare((f64)acc, (f64)lns32(PI2_OVER_6_F32));
}

static scalar_result accum_lns16_bwd(void) {
  lns16 acc = lns16(0.0f);

  for (i32 n = ACCUM_N; n >= 1; n--) {
    const lns16 v = lns16(1.0f / (f32)(n * n));
    acc = acc + v;
  }

  return scalar_compare((f64)acc, (f64)lns32(PI2_OVER_6_F32));
}

static scalar_result accum_bf16_fwd(void) {
  f32 acc = 0.0f;

  for (i32 n = 1; n <= ACCUM_N; n++) {
    const f32 v = (f32)bf16(1.0f / (f32)(n * n));
    acc = (f32)bf16(acc + v);
  }

  return scalar_compare((f64)acc, (f64)PI2_OVER_6_F32);
}

static scalar_result accum_bf16_bwd(void) {
  f32 acc = 0.0f;

  for (i32 n = ACCUM_N; n >= 1; n--) {
    const f32 v = (f32)bf16(1.0f / (f32)(n * n));
    acc = (f32)bf16(acc + v);
  }

  return scalar_compare((f64)acc, (f64)PI2_OVER_6_F32);
}

// ---------------------------------------------------------------------------
// 5. Sigmoid activation:  sigma(x) = 1 / (1 + exp(-x))
//    x swept from -10 to +10 in 1001 uniform steps.
//
//    Output is always in (0, 1), well inside (2^-p, 16).
//    avg_rel and avg_abs across all sweep points are reported.
//    LNS exp is native (exponent field inversion); BF16 must approximate.
// ---------------------------------------------------------------------------

#define SIGMOID_N 1001

static scalar_result sigmoid_lns16(void) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < SIGMOID_N; i++) {
    const f32   x        = -10.0f + 20.0f * (f32)i / (f32)(SIGMOID_N - 1);
    const lns16 neg_x    = lns16(-x);
    const lns16 e_negx   = neg_x.exp();
    const lns16 denom    = lns16(1.0f) + e_negx;
    const lns16 sig      = lns16(1.0f) / denom;
    const f64   got      = (f64)(f32)sig;
    const f64   expected = (f64)(f32)lns32(1.0f / (1.0f + expf(-x)));
    const f64   abs      = fabs(got - expected);
    const f64   rel      = fabs(expected) > 1e-15
      ? abs / fabs(expected)
      : abs;
    sum_abs += abs;
    sum_rel += rel;
    cnt++;
  }

  scalar_result r;
  r.got      = sum_rel / cnt;
  r.expected = 0.0;
  r.abs_err  = sum_abs / cnt;
  r.rel_err  = sum_rel / cnt;

  return r;
}

static scalar_result sigmoid_bf16(void) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < SIGMOID_N; i++) {
    const f32 x        = -10.0f + 20.0f * (f32)i / (f32)(SIGMOID_N - 1);
    const f32 got_f    = (f32)bf16(1.0f / (1.0f + expf(-x)));
    const f32 expected = 1.0f / (1.0f + expf(-x));
    const f64 abs      = fabs((f64)got_f - (f64)expected);
    const f64 rel      = fabs((f64)expected) > 1e-15
      ? abs / fabs((f64)expected)
      : abs;
    sum_abs += abs;
    sum_rel += rel;
    cnt++;
  }

  scalar_result r;
  r.got      = sum_rel / cnt;
  r.expected = 0.0;
  r.abs_err  = sum_abs / cnt;
  r.rel_err  = sum_rel / cnt;

  return r;
}

// ---------------------------------------------------------------------------
// 6. GELU activation (tanh approximation):
//    GELU(x) = 0.5*x*(1 + tanh(sqrt(2/pi)*(x + 0.044715*x^3)))
//
//    Sweep narrowed to x in [-2, 2] (801 steps) so that all expected
//    outputs fall in (-1.1, 2), safely inside (2^-p, 16).  The full
//    [-4, 4] range pushes outputs toward ±4 which exhausts lns8 dynamic
//    range and saturates relative error before format differences appear.
//    avg_rel and avg_abs across all sweep points are reported.
// ---------------------------------------------------------------------------

#define GELU_N 801
static const f64 GELU_C = 0.7978845608028654; // sqrt(2/pi)
static const f64 GELU_K = 0.044715;

static scalar_result gelu_lns16(void) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < GELU_N; i++) {
    const f32   x        = -2.0f + 4.0f * (f32)i / (f32)(GELU_N - 1);
    const lns16 lx       = lns16(x);
    const lns16 x3       = lx * lx * lx;
    const lns16 inner    = lns16((f32)GELU_C) * (lx + lns16((f32)GELU_K) * x3);
    const lns16 th       = inner.tanh();
    const lns16 gelu     = lns16(0.5f) * lx * (lns16(1.0f) + th);
    const f64   got      = (f64)(f32)gelu;
    const f64   xd       = (f64)x;
    const f64   exp_f64  = 0.5 * xd * (1.0 + tanh(GELU_C * (xd + GELU_K * xd*xd*xd)));
    const f32   expected = (f32)lns32((f32)exp_f64);
    const f64   abs      = fabs(got - (f64)expected);
    const f64   rel      = fabs((f64)expected) > 1e-15
      ? abs / fabs((f64)expected)
      : abs;
    sum_abs += abs;
    sum_rel += rel;
    cnt++;
  }

  scalar_result r;
  r.got      = sum_rel / cnt;
  r.expected = 0.0;
  r.abs_err  = sum_abs / cnt;
  r.rel_err  = sum_rel / cnt;

  return r;
}

static scalar_result gelu_bf16(void) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < GELU_N; i++) {
    const f32 x        = -2.0f + 4.0f * (f32)i / (f32)(GELU_N - 1);
    const f32 x3       = x * x * x;
    const f32 inner    = (f32)GELU_C * (x + (f32)GELU_K * x3);
    const f32 got_f    = (f32)bf16(0.5f * x * (1.0f + tanhf(inner)));
    const f32 expected = 0.5f * x * (1.0f + tanhf(inner));
    const f64 abs      = fabs((f64)got_f - (f64)expected);
    const f64 rel      = fabs((f64)expected) > 1e-15
      ? abs / fabs((f64)expected)
      : abs;
    sum_abs += abs;
    sum_rel += rel;
    cnt++;
  }

  scalar_result r;
  r.got      = sum_rel / cnt;
  r.expected = 0.0;
  r.abs_err  = sum_abs / cnt;
  r.rel_err  = sum_rel / cnt;

  return r;
}

// ---------------------------------------------------------------------------
// bench_numerical_run
// ---------------------------------------------------------------------------

void bench_numerical_run(const char* results_dir) {
  (void)results_dir;

  // -- 1. Geometric progression ------------------------------------------
  {
    const scalar_result l16 = geom_lns16();
    const scalar_result b16 = geom_bf16();
    const scalar_result l8  = geom_lns8();
    const scalar_result b8  = geom_bf8();
    csv_write_scalar("lns16", "geometric_progression", "lns16", &l16);
    csv_write_scalar("bf16",  "geometric_progression", "bf16",  &b16);
    csv_write_scalar("lns8",  "geometric_progression", "lns8",  &l8);
    csv_write_scalar("bf8",   "geometric_progression", "bf8",   &b8);
  }

  // -- 2. Euclidean norm -------------------------------------------------
  {
    norm_init_samples();
    const scalar_result l16 = norm_lns16();
    const scalar_result b16 = norm_bf16();
    csv_write_scalar("lns16", "euclidean_norm", "lns16", &l16);
    csv_write_scalar("bf16",  "euclidean_norm", "bf16",  &b16);
  }

  // -- 3. Alternating harmonic (Leibniz pi) ------------------------------
  {
    const scalar_result l16 = leibniz_lns16();
    const scalar_result b16 = leibniz_bf16();
    csv_write_scalar("lns16", "alternating_harmonic", "lns16", &l16);
    csv_write_scalar("bf16",  "alternating_harmonic", "bf16",  &b16);
  }

  // -- 4. Large-to-small accumulation ------------------------------------
  {
    const scalar_result lf = accum_lns16_fwd();
    const scalar_result lb = accum_lns16_bwd();
    const scalar_result bf = accum_bf16_fwd();
    const scalar_result bb = accum_bf16_bwd();
    csv_write_scalar("lns16", "pi2_over6_fwd", "lns16_fwd", &lf);
    csv_write_scalar("bf16",  "pi2_over6_fwd", "bf16_fwd",  &bf);
    csv_write_scalar("lns16", "pi2_over6_bwd", "lns16_bwd", &lb);
    csv_write_scalar("bf16",  "pi2_over6_bwd", "bf16_bwd",  &bb);
  }

  // -- 5. Sigmoid --------------------------------------------------------
  {
    const scalar_result l16 = sigmoid_lns16();
    const scalar_result b16 = sigmoid_bf16();
    csv_write_scalar("lns16", "sigmoid", "lns16", &l16);
    csv_write_scalar("bf16",  "sigmoid", "bf16",  &b16);
  }

  // -- 6. GELU -----------------------------------------------------------
  {
    const scalar_result l16 = gelu_lns16();
    const scalar_result b16 = gelu_bf16();
    csv_write_scalar("lns16", "gelu", "lns16", &l16);
    csv_write_scalar("bf16",  "gelu", "bf16",  &b16);
  }
}
