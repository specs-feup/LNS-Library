#include "bench_numerical.h"
#include "stats.h"
#include "csv.h"
#include "rng.h"
#include "lnssim.hpp"
#include "bfloatsim.hpp"

#include <math.h>

// ---------------------------------------------------------------------------
// Helper: scalar_compare using lns32 as reference (existing pattern)
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 1. Geometric progression:  a_n = a_0 * r^n
//    a_0=1, r=1.015, n=100 iterations.
// ---------------------------------------------------------------------------

static scalar_result geom_lns16(void) {
  lns16 acc     = lns16(1.0f);
  const lns16 r = lns16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc *= r;

  const lns32 expected = lns32((f32)pow(1.015, 100.0));
  return scalar_compare((f64)acc, (f64)expected);
}

static scalar_result geom_lns16_lns32acc(void) {
  lns32 acc     = lns32(1.0f);
  const lns16 r = lns16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc = (lns32)(lns16(acc) * r);

  const lns32 expected = lns32((f32)pow(1.015, 100.0));
  return scalar_compare((f64)lns16(acc), (f64)expected);
}

static scalar_result geom_lns16_f32acc(void) {
  f32 acc           = 1.0f;
  const f32 r       = (f32)lns16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc = acc * r;

  const lns32 expected = lns32((f32)pow(1.015, 100.0));
  return scalar_compare((f64)(f32)lns16(acc), (f64)expected);
}

static scalar_result geom_bf16(void) {
  bf16 acc     = bf16(1.0f);
  const bf16 r = bf16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc = bf16((f32)acc * (f32)r);

  const f32 expected = (f32)pow(1.015, 100.0);
  return scalar_compare((f64)(f32)acc, (f64)expected);
}

static scalar_result geom_bf16_f32acc(void) {
  f32 acc       = 1.0f;
  const f32 r   = (f32)bf16(1.015f);

  for (i32 i = 0; i < 100; i++)
    acc = acc * r;

  const f32 expected = (f32)pow(1.015, 100.0);
  return scalar_compare((f64)(f32)bf16(acc), (f64)expected);
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

// lns16 weights, lns32 accumulator
static scalar_result norm_lns16_lns32acc(void) {
  lns32 acc = lns32(0.0f);
  for (i32 i = 0; i < NORM_N; i++) {
    const lns16 v = lns16(g_norm_samples[i]);
    acc = acc + (lns32)(v * v);
  }

  f64 expected = 0.0;
  for (i32 i = 0; i < NORM_N; i++)
    expected += (f64)g_norm_samples[i] * (f64)g_norm_samples[i];

  expected = sqrt(expected);
  return scalar_compare((f64)(f32)lns16(acc).sqrt(), expected);
}

// lns16 weights, f32 accumulator
static scalar_result norm_lns16_f32acc(void) {
  f32 acc = 0.0f;
  for (i32 i = 0; i < NORM_N; i++) {
    const lns16 v = lns16(g_norm_samples[i]);
    acc = acc + (f32)(v * v);
  }

  f64 expected = 0.0;
  for (i32 i = 0; i < NORM_N; i++)
    expected += (f64)g_norm_samples[i] * (f64)g_norm_samples[i];

  expected = sqrt(expected);
  return scalar_compare((f64)(f32)lns16(sqrtf(acc)), expected);
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

// bf16 weights, f32 accumulator
static scalar_result norm_bf16_f32acc(void) {
  f32 acc = 0.0f;
  for (i32 i = 0; i < NORM_N; i++) {
    const f32 v = (f32)bf16(g_norm_samples[i]);
    acc = acc + v * v;
  }

  f64 expected = 0.0;
  for (i32 i = 0; i < NORM_N; i++)
    expected += (f64)g_norm_samples[i] * (f64)g_norm_samples[i];

  expected = sqrt(expected);
  return scalar_compare((f64)(f32)bf16(sqrtf(acc)), expected);
}

// ---------------------------------------------------------------------------
// 3. Alternating harmonic series (Leibniz pi)
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

static scalar_result leibniz_lns16_lns32acc(void) {
  lns32 acc = lns32(0.0f);
  for (i32 k = 0; k < 10000; k++) {
    const lns16 term = lns16(1.0f / (f32)(2 * k + 1));
    acc = (k & 1) ? acc - (lns32)term : acc + (lns32)term;
  }

  const lns16 result = lns16(4.0f) * lns16(acc);
  return scalar_compare((f64)result, (f64)lns32((f32)M_PI));
}

static scalar_result leibniz_lns16_f32acc(void) {
  f32 acc = 0.0f;
  for (i32 k = 0; k < 10000; k++) {
    const f32 term = (f32)lns16(1.0f / (f32)(2 * k + 1));
    acc = (k & 1) ? acc - term : acc + term;
  }

  const lns16 result = lns16(4.0f) * lns16(acc);
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

// bf16 weights, f32 accumulator
static scalar_result leibniz_bf16_f32acc(void) {
  f32 acc = 0.0f;
  for (i32 k = 0; k < 10000; k++) {
    const f32 term = (f32)bf16(1.0f / (f32)(2 * k + 1));
    acc = (k & 1) ? acc - term : acc + term;
  }

  const f32 result   = (f32)bf16(4.0f * acc);
  const f32 expected = (f32)M_PI;
  return scalar_compare((f64)result, (f64)expected);
}

// ---------------------------------------------------------------------------
// 4. Large-to-small accumulation: sum_{n=1}^{5000} 1/n^2 = pi^2/6
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

// lns16 values, lns32 accumulator — forward
static scalar_result accum_lns16_lns32acc_fwd(void) {
  lns32 acc = lns32(0.0f);
  for (i32 n = 1; n <= ACCUM_N; n++) {
    const lns16 v = lns16(1.0f / (f32)(n * n));
    acc = acc + (lns32)v;
  }
  return scalar_compare((f64)lns16(acc), (f64)lns32(PI2_OVER_6_F32));
}

// lns16 values, lns32 accumulator — backward
static scalar_result accum_lns16_lns32acc_bwd(void) {
  lns32 acc = lns32(0.0f);
  for (i32 n = ACCUM_N; n >= 1; n--) {
    const lns16 v = lns16(1.0f / (f32)(n * n));
    acc = acc + (lns32)v;
  }
  return scalar_compare((f64)lns16(acc), (f64)lns32(PI2_OVER_6_F32));
}

// lns16 values, f32 accumulator — forward
static scalar_result accum_lns16_f32acc_fwd(void) {
  f32 acc = 0.0f;
  for (i32 n = 1; n <= ACCUM_N; n++) {
    const f32 v = (f32)lns16(1.0f / (f32)(n * n));
    acc = acc + v;
  }
  return scalar_compare((f64)(f32)lns16(acc), (f64)lns32(PI2_OVER_6_F32));
}

// lns16 values, f32 accumulator — backward
static scalar_result accum_lns16_f32acc_bwd(void) {
  f32 acc = 0.0f;
  for (i32 n = ACCUM_N; n >= 1; n--) {
    const f32 v = (f32)lns16(1.0f / (f32)(n * n));
    acc = acc + v;
  }
  return scalar_compare((f64)(f32)lns16(acc), (f64)lns32(PI2_OVER_6_F32));
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

// bf16 values, f32 accumulator — forward
static scalar_result accum_bf16_f32acc_fwd(void) {
  f32 acc = 0.0f;
  for (i32 n = 1; n <= ACCUM_N; n++) {
    const f32 v = (f32)bf16(1.0f / (f32)(n * n));
    acc = acc + v;
  }
  return scalar_compare((f64)(f32)bf16(acc), (f64)PI2_OVER_6_F32);
}

// bf16 values, f32 accumulator — backward
static scalar_result accum_bf16_f32acc_bwd(void) {
  f32 acc = 0.0f;
  for (i32 n = ACCUM_N; n >= 1; n--) {
    const f32 v = (f32)bf16(1.0f / (f32)(n * n));
    acc = acc + v;
  }
  return scalar_compare((f64)(f32)bf16(acc), (f64)PI2_OVER_6_F32);
}

// ---------------------------------------------------------------------------
// 5. Sigmoid activation:  sigma(x) = 1 / (1 + exp(-x))
//    x swept from -10 to +10 in 1001 uniform steps.
// ---------------------------------------------------------------------------

#define SIGMOID_N 1001

enum class AccKind { LNS16, LNS32, F32 };

static scalar_result sigmoid_sweep(AccKind acc_kind) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < SIGMOID_N; i++) {
    const f32   x        = -10.0f + 20.0f * (f32)i / (f32)(SIGMOID_N - 1);
    const lns16 neg_x    = lns16(-x);
    const lns16 e_negx   = neg_x.exp();
    f64         got;

    if (acc_kind == AccKind::LNS32) {
      // accumulate denominator in lns32
      const lns32 denom = lns32(1.0f) + (lns32)e_negx;
      const lns16 sig   = lns16(1.0f) / lns16(denom);
      got = (f64)(f32)sig;
    } else if (acc_kind == AccKind::F32) {
      // accumulate denominator in f32
      const f32   denom = 1.0f + (f32)e_negx;
      const lns16 sig   = lns16(1.0f) / lns16(denom);
      got = (f64)(f32)sig;
    } else {
      const lns16 denom = lns16(1.0f) + e_negx;
      const lns16 sig   = lns16(1.0f) / denom;
      got = (f64)(f32)sig;
    }

    const f64 expected = (f64)(f32)lns32(1.0f / (1.0f + expf(-x)));
    const f64 abs      = fabs(got - expected);
    const f64 rel      = fabs(expected) > 1e-15
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

static scalar_result sigmoid_lns16(void)          { return sigmoid_sweep(AccKind::LNS16); }
static scalar_result sigmoid_lns16_lns32acc(void) { return sigmoid_sweep(AccKind::LNS32); }
static scalar_result sigmoid_lns16_f32acc(void)   { return sigmoid_sweep(AccKind::F32);   }

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

// bf16 inputs, f32 accumulator for denominator
static scalar_result sigmoid_bf16_f32acc(void) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < SIGMOID_N; i++) {
    const f32 x        = -10.0f + 20.0f * (f32)i / (f32)(SIGMOID_N - 1);
    const f32 neg_x    = (f32)bf16(-x);
    const f32 denom    = 1.0f + expf(neg_x);
    const f32 got_f    = (f32)bf16(1.0f / denom);
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
// 6. GELU activation (tanh approximation)
//    GELU(x) = 0.5*x*(1 + tanh(sqrt(2/pi)*(x + 0.044715*x^3)))
// ---------------------------------------------------------------------------

#define GELU_N 801
static const f64 GELU_C = 0.7978845608028654; // sqrt(2/pi)
static const f64 GELU_K = 0.044715;

static scalar_result gelu_sweep(AccKind acc_kind) {
  f64 sum_rel = 0.0;
  f64 sum_abs = 0.0;
  u32 cnt     = 0;

  for (i32 i = 0; i < GELU_N; i++) {
    const f32   x       = -2.0f + 4.0f * (f32)i / (f32)(GELU_N - 1);
    const lns16 lx      = lns16(x);
    const lns16 x3      = lx * lx * lx;
    const lns16 inner   = lns16((f32)GELU_C) * (lx + lns16((f32)GELU_K) * x3);
    const lns16 th      = inner.tanh();
    f64         got;

    if (acc_kind == AccKind::LNS32) {
      const lns32 one_plus_th = lns32(1.0f) + (lns32)th;
      const lns16 gelu        = lns16(0.5f) * lx * lns16(one_plus_th);
      got = (f64)(f32)gelu;
    } else if (acc_kind == AccKind::F32) {
      const f32   one_plus_th = 1.0f + (f32)th;
      const lns16 gelu        = lns16(0.5f) * lx * lns16(one_plus_th);
      got = (f64)(f32)gelu;
    } else {
      const lns16 gelu = lns16(0.5f) * lx * (lns16(1.0f) + th);
      got = (f64)(f32)gelu;
    }

    const f64 xd       = (f64)x;
    const f64 exp_f64  = 0.5 * xd * (1.0 + tanh(GELU_C * (xd + GELU_K * xd*xd*xd)));
    const f32 expected = (f32)lns32((f32)exp_f64);
    const f64 abs      = fabs(got - (f64)expected);
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

static scalar_result gelu_lns16(void)          { return gelu_sweep(AccKind::LNS16); }
static scalar_result gelu_lns16_lns32acc(void) { return gelu_sweep(AccKind::LNS32); }
static scalar_result gelu_lns16_f32acc(void)   { return gelu_sweep(AccKind::F32);   }

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

// bf16 inputs, f32 accumulator
static scalar_result gelu_bf16_f32acc(void) {
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
// 7. Softmax normalisation (model-representative)
//    sum_{i=0}^{N-1} exp(x_i - max_x)  for N=512 values in [-5, 5].
//    Represents the softmax denominator used in attention and token sampling.
//    lns16 baseline vs. lns32 accumulator vs. f32 (bf16 cast inputs).
// ---------------------------------------------------------------------------

#define SOFTMAX_N 512

static f32 g_softmax_samples[SOFTMAX_N];

static void softmax_init_samples(void) {
  for (i32 i = 0; i < SOFTMAX_N; i++)
    g_softmax_samples[i] = -5.0f + 10.0f * (f32)i / (f32)(SOFTMAX_N - 1);
}

static scalar_result softmax_sum_lns16(void) {
  // find max
  lns16 max_v = lns16(g_softmax_samples[0]);
  for (i32 i = 1; i < SOFTMAX_N; i++) {
    lns16 v = lns16(g_softmax_samples[i]);
    if (v > max_v)
      max_v = v;
  }

  lns16 acc = lns16(0.0f);
  for (i32 i = 0; i < SOFTMAX_N; i++) {
    lns16 e = lns16(expf(g_softmax_samples[i] - (f32)max_v));
    acc = acc + e;
  }
  // reference: f64 sum
  f64 max_d = (f64)(f32)max_v;
  f64 ref   = 0.0;
  for (i32 i = 0; i < SOFTMAX_N; i++)
    ref += exp((f64)g_softmax_samples[i] - max_d);

  return scalar_compare((f64)(f32)acc, ref);
}

static scalar_result softmax_sum_lns16_lns32acc(void) {
  lns16 max_v = lns16(g_softmax_samples[0]);
  for (i32 i = 1; i < SOFTMAX_N; i++) {
    lns16 v = lns16(g_softmax_samples[i]);
    if (v > max_v)
      max_v = v;
  }

  lns32 acc = lns32(0.0f);
  for (i32 i = 0; i < SOFTMAX_N; i++) {
    lns16 e = lns16(expf(g_softmax_samples[i] - (f32)max_v));
    acc = acc + (lns32)e;
  }

  f64 max_d = (f64)(f32)max_v;
  f64 ref   = 0.0;
  for (i32 i = 0; i < SOFTMAX_N; i++)
    ref += exp((f64)g_softmax_samples[i] - max_d);

  return scalar_compare((f64)(f32)lns16(acc), ref);
}

static scalar_result softmax_sum_lns16_f32acc(void) {
  lns16 max_v = lns16(g_softmax_samples[0]);
  for (i32 i = 1; i < SOFTMAX_N; i++) {
    lns16 v = lns16(g_softmax_samples[i]);
    if (v > max_v)
      max_v = v;
  }

  f32 acc = 0.0f;
  for (i32 i = 0; i < SOFTMAX_N; i++) {
    lns16 e = lns16(expf(g_softmax_samples[i] - (f32)max_v));
    acc = acc + (f32)e;
  }

  f64 max_d = (f64)(f32)max_v;
  f64 ref   = 0.0;
  for (i32 i = 0; i < SOFTMAX_N; i++)
    ref += exp((f64)g_softmax_samples[i] - max_d);

  return scalar_compare((f64)(f32)lns16(acc), ref);
}

static scalar_result softmax_sum_bf16(void) {
  f32 max_v = g_softmax_samples[0];
  for (i32 i = 1; i < SOFTMAX_N; i++)
    if (g_softmax_samples[i] > max_v)
      max_v = g_softmax_samples[i];

  f32 acc = 0.0f;
  for (i32 i = 0; i < SOFTMAX_N; i++)
    acc = (f32)bf16(acc + expf(g_softmax_samples[i] - max_v));

  f64 ref = 0.0;
  for (i32 i = 0; i < SOFTMAX_N; i++)
    ref += exp((f64)g_softmax_samples[i] - (f64)max_v);

  return scalar_compare((f64)(f32)bf16(acc), ref);
}

// bf16 inputs, f32 accumulator
static scalar_result softmax_sum_bf16_f32acc(void) {
  f32 max_v = g_softmax_samples[0];
  for (i32 i = 1; i < SOFTMAX_N; i++)
    if (g_softmax_samples[i] > max_v)
      max_v = g_softmax_samples[i];

  f32 acc = 0.0f;
  for (i32 i = 0; i < SOFTMAX_N; i++) {
    const f32 e = (f32)bf16(expf(g_softmax_samples[i] - max_v));
    acc = acc + e;
  }

  f64 ref = 0.0;
  for (i32 i = 0; i < SOFTMAX_N; i++)
    ref += exp((f64)g_softmax_samples[i] - (f64)max_v);

  return scalar_compare((f64)(f32)bf16(acc), ref);
}

// ---------------------------------------------------------------------------
// 8. RMSNorm denominator: sqrt(1/N * sum(x_i^2) + eps)
//    N=512 values in [0.01, 4], eps=1e-5.
//    Represents rmsnorm as used in the Llama 2 transformer blocks.
// ---------------------------------------------------------------------------

#define RMSNORM_N   512
#define RMSNORM_EPS 1e-5f

static f32 g_rmsnorm_samples[RMSNORM_N];

static void rmsnorm_init_samples(void) {
  for (i32 i = 0; i < RMSNORM_N; i++)
    g_rmsnorm_samples[i] = sample_band(0.01f, 4.0f, 0.01f);
}

static scalar_result rmsnorm_lns16(void) {
  lns16 acc = lns16(0.0f);
  for (i32 i = 0; i < RMSNORM_N; i++) {
    lns16 v = lns16(g_rmsnorm_samples[i]);
    acc = acc + v * v;
  }
  lns16 rms = lns16((f32)(1.0 / RMSNORM_N) * (f32)acc + RMSNORM_EPS);
  rms = rms.sqrt();

  f64 ref_ss = 0.0;
  for (i32 i = 0; i < RMSNORM_N; i++)
    ref_ss += (f64)g_rmsnorm_samples[i] * (f64)g_rmsnorm_samples[i];
  f64 ref = sqrt(ref_ss / RMSNORM_N + RMSNORM_EPS);

  return scalar_compare((f64)(f32)rms, ref);
}

static scalar_result rmsnorm_lns16_lns32acc(void) {
  lns32 acc = lns32(0.0f);
  for (i32 i = 0; i < RMSNORM_N; i++) {
    lns16 v = lns16(g_rmsnorm_samples[i]);
    acc = acc + (lns32)(v * v);
  }
  lns16 rms = lns16((f32)(1.0 / RMSNORM_N) * (f32)lns16(acc) + RMSNORM_EPS);
  rms = rms.sqrt();

  f64 ref_ss = 0.0;
  for (i32 i = 0; i < RMSNORM_N; i++)
    ref_ss += (f64)g_rmsnorm_samples[i] * (f64)g_rmsnorm_samples[i];
  f64 ref = sqrt(ref_ss / RMSNORM_N + RMSNORM_EPS);

  return scalar_compare((f64)(f32)rms, ref);
}

// lns16 inputs, f32 accumulator
static scalar_result rmsnorm_lns16_f32acc(void) {
  f32 acc = 0.0f;
  for (i32 i = 0; i < RMSNORM_N; i++) {
    lns16 v = lns16(g_rmsnorm_samples[i]);
    acc = acc + (f32)(v * v);
  }
  lns16 rms = lns16((f32)(1.0 / RMSNORM_N) * acc + RMSNORM_EPS);
  rms = rms.sqrt();

  f64 ref_ss = 0.0;
  for (i32 i = 0; i < RMSNORM_N; i++)
    ref_ss += (f64)g_rmsnorm_samples[i] * (f64)g_rmsnorm_samples[i];
  f64 ref = sqrt(ref_ss / RMSNORM_N + RMSNORM_EPS);

  return scalar_compare((f64)(f32)rms, ref);
}

static scalar_result rmsnorm_bf16(void) {
  f32 acc = 0.0f;
  for (i32 i = 0; i < RMSNORM_N; i++) {
    f32 v = (f32)bf16(g_rmsnorm_samples[i]);
    acc = (f32)bf16(acc + v * v);
  }
  f32 rms = sqrtf((f32)bf16(acc / (f32)RMSNORM_N) + RMSNORM_EPS);

  f64 ref_ss = 0.0;
  for (i32 i = 0; i < RMSNORM_N; i++)
    ref_ss += (f64)g_rmsnorm_samples[i] * (f64)g_rmsnorm_samples[i];
  f64 ref = sqrt(ref_ss / RMSNORM_N + RMSNORM_EPS);

  return scalar_compare((f64)(f32)bf16(rms), ref);
}

// bf16 inputs, f32 accumulator
static scalar_result rmsnorm_bf16_f32acc(void) {
  f32 acc = 0.0f;
  for (i32 i = 0; i < RMSNORM_N; i++) {
    f32 v = (f32)bf16(g_rmsnorm_samples[i]);
    acc = acc + v * v;
  }
  f32 rms = sqrtf(acc / (f32)RMSNORM_N + RMSNORM_EPS);

  f64 ref_ss = 0.0;
  for (i32 i = 0; i < RMSNORM_N; i++)
    ref_ss += (f64)g_rmsnorm_samples[i] * (f64)g_rmsnorm_samples[i];
  f64 ref = sqrt(ref_ss / RMSNORM_N + RMSNORM_EPS);

  return scalar_compare((f64)(f32)bf16(rms), ref);
}

// ---------------------------------------------------------------------------
// bench_numerical_run
// ---------------------------------------------------------------------------

void bench_numerical_run(const char* results_dir) {
  (void)results_dir;

  // -- 1. Geometric progression ------------------------------------------
  {
    const scalar_result l16          = geom_lns16();
    const scalar_result l16_lns32acc = geom_lns16_lns32acc();
    const scalar_result l16_f32acc   = geom_lns16_f32acc();
    const scalar_result b16          = geom_bf16();
    const scalar_result b16_f32      = geom_bf16_f32acc();
    const scalar_result l8           = geom_lns8();
    const scalar_result b8           = geom_bf8();
    csv_write_scalar("lns16",          "geometric_progression", "lns16",         &l16);
    csv_write_scalar("lns16_lns32acc", "geometric_progression", "lns16_lns32acc",&l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "geometric_progression", "lns16_f32acc",  &l16_f32acc);
    csv_write_scalar("bf16",           "geometric_progression", "bf16",          &b16);
    csv_write_scalar("bf16_f32acc",    "geometric_progression", "bf16_f32acc",   &b16_f32);
    csv_write_scalar("lns8",           "geometric_progression", "lns8",          &l8);
    csv_write_scalar("bf8",            "geometric_progression", "bf8",           &b8);
  }

  // -- 2. Euclidean norm -------------------------------------------------
  {
    norm_init_samples();
    const scalar_result l16          = norm_lns16();
    const scalar_result l16_lns32acc = norm_lns16_lns32acc();
    const scalar_result l16_f32acc   = norm_lns16_f32acc();
    const scalar_result b16          = norm_bf16();
    const scalar_result b16_f32      = norm_bf16_f32acc();
    csv_write_scalar("lns16",          "euclidean_norm", "lns16",          &l16);
    csv_write_scalar("lns16_lns32acc", "euclidean_norm", "lns16_lns32acc", &l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "euclidean_norm", "lns16_f32acc",   &l16_f32acc);
    csv_write_scalar("bf16",           "euclidean_norm", "bf16",           &b16);
    csv_write_scalar("bf16_f32acc",    "euclidean_norm", "bf16_f32acc",    &b16_f32);
  }

  // -- 3. Alternating harmonic (Leibniz pi) ------------------------------
  {
    const scalar_result l16          = leibniz_lns16();
    const scalar_result l16_lns32acc = leibniz_lns16_lns32acc();
    const scalar_result l16_f32acc   = leibniz_lns16_f32acc();
    const scalar_result b16          = leibniz_bf16();
    const scalar_result b16_f32      = leibniz_bf16_f32acc();
    csv_write_scalar("lns16",          "alternating_harmonic", "lns16",          &l16);
    csv_write_scalar("lns16_lns32acc", "alternating_harmonic", "lns16_lns32acc", &l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "alternating_harmonic", "lns16_f32acc",   &l16_f32acc);
    csv_write_scalar("bf16",           "alternating_harmonic", "bf16",           &b16);
    csv_write_scalar("bf16_f32acc",    "alternating_harmonic", "bf16_f32acc",    &b16_f32);
  }

  // -- 4. Large-to-small accumulation ------------------------------------
  {
    const scalar_result lf          = accum_lns16_fwd();
    const scalar_result lb          = accum_lns16_bwd();
    const scalar_result lf_lns32acc = accum_lns16_lns32acc_fwd();
    const scalar_result lb_lns32acc = accum_lns16_lns32acc_bwd();
    const scalar_result lf_f32acc   = accum_lns16_f32acc_fwd();
    const scalar_result lb_f32acc   = accum_lns16_f32acc_bwd();
    const scalar_result bf          = accum_bf16_fwd();
    const scalar_result bb          = accum_bf16_bwd();
    const scalar_result bf_f32      = accum_bf16_f32acc_fwd();
    const scalar_result bb_f32      = accum_bf16_f32acc_bwd();
    csv_write_scalar("lns16",          "pi2_over6_fwd", "lns16_fwd",          &lf);
    csv_write_scalar("lns16_lns32acc", "pi2_over6_fwd", "lns16_lns32acc_fwd", &lf_lns32acc);
    csv_write_scalar("lns16_f32acc",   "pi2_over6_fwd", "lns16_f32acc_fwd",   &lf_f32acc);
    csv_write_scalar("bf16",           "pi2_over6_fwd", "bf16_fwd",           &bf);
    csv_write_scalar("bf16_f32acc",    "pi2_over6_fwd", "bf16_f32acc_fwd",    &bf_f32);
    csv_write_scalar("lns16",          "pi2_over6_bwd", "lns16_bwd",          &lb);
    csv_write_scalar("lns16_lns32acc", "pi2_over6_bwd", "lns16_lns32acc_bwd", &lb_lns32acc);
    csv_write_scalar("lns16_f32acc",   "pi2_over6_bwd", "lns16_lns32acc_bwd", &lb_f32acc);
    csv_write_scalar("bf16",           "pi2_over6_bwd", "bf16_bwd",           &bb);
    csv_write_scalar("bf16_f32acc",    "pi2_over6_bwd", "bf16_f32acc_bwd",    &bb_f32);
  }

  // -- 5. Sigmoid --------------------------------------------------------
  {
    const scalar_result l16          = sigmoid_lns16();
    const scalar_result l16_lns32acc = sigmoid_lns16_lns32acc();
    const scalar_result l16_f32acc   = sigmoid_lns16_f32acc();
    const scalar_result b16          = sigmoid_bf16();
    const scalar_result b16_f32      = sigmoid_bf16_f32acc();
    csv_write_scalar("lns16",          "sigmoid", "lns16",          &l16);
    csv_write_scalar("lns16_lns32acc", "sigmoid", "lns16_lns32acc", &l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "sigmoid", "lns16_f32acc",   &l16_f32acc);
    csv_write_scalar("bf16",           "sigmoid", "bf16",           &b16);
    csv_write_scalar("bf16_f32acc",    "sigmoid", "bf16_f32acc",    &b16_f32);
  }

  // -- 6. GELU -----------------------------------------------------------
  {
    const scalar_result l16          = gelu_lns16();
    const scalar_result l16_lns32acc = gelu_lns16_lns32acc();
    const scalar_result l16_f32acc   = gelu_lns16_f32acc();
    const scalar_result b16          = gelu_bf16();
    const scalar_result b16_f32      = gelu_bf16_f32acc();
    csv_write_scalar("lns16",          "gelu", "lns16",          &l16);
    csv_write_scalar("lns16_lns32acc", "gelu", "lns16_lns32acc", &l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "gelu", "lns16_f32acc",   &l16_f32acc);
    csv_write_scalar("bf16",           "gelu", "bf16",           &b16);
    csv_write_scalar("bf16_f32acc",    "gelu", "bf16_f32acc",    &b16_f32);
  }

  // -- 7. Softmax sum ----------------------------------------------------
  {
    softmax_init_samples();
    const scalar_result l16          = softmax_sum_lns16();
    const scalar_result l16_lns32acc = softmax_sum_lns16_lns32acc();
    const scalar_result l16_f32acc   = softmax_sum_lns16_f32acc();
    const scalar_result b16          = softmax_sum_bf16();
    const scalar_result b16_f32      = softmax_sum_bf16_f32acc();
    csv_write_scalar("lns16",          "softmax_sum", "lns16",          &l16);
    csv_write_scalar("lns16_lns32acc", "softmax_sum", "lns16_lns32acc", &l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "softmax_sum", "lns16_f32acc",   &l16_f32acc);
    csv_write_scalar("bf16",           "softmax_sum", "bf16",           &b16);
    csv_write_scalar("bf16_f32acc",    "softmax_sum", "bf16_f32acc",    &b16_f32);
  }

  // -- 8. RMSNorm denominator --------------------------------------------
  {
    rmsnorm_init_samples();
    const scalar_result l16          = rmsnorm_lns16();
    const scalar_result l16_lns32acc = rmsnorm_lns16_lns32acc();
    const scalar_result l16_f32acc   = rmsnorm_lns16_f32acc();
    const scalar_result b16          = rmsnorm_bf16();
    const scalar_result b16_f32      = rmsnorm_bf16_f32acc();
    csv_write_scalar("lns16",          "rmsnorm_denom", "lns16",          &l16);
    csv_write_scalar("lns16_lns32acc", "rmsnorm_denom", "lns16_lns32acc", &l16_lns32acc);
    csv_write_scalar("lns16_f32acc",   "rmsnorm_denom", "lns16_f32acc",   &l16_f32acc);
    csv_write_scalar("bf16",           "rmsnorm_denom", "bf16",           &b16);
    csv_write_scalar("bf16_f32acc",    "rmsnorm_denom", "bf16_f32acc",    &b16_f32);
  }
}
