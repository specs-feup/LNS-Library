#ifndef __BENCH_NUMERICAL_H__
#define __BENCH_NUMERICAL_H__

#include "utils.h"

// ---------------------------------------------------------------------------
// bench_numerical — six algorithm-level numerical tests comparing
// lns16 vs bf16 (and where sensible, lns8 vs bf8).
//
// Each test computes a well-known numerical result using the low-precision
// format and compares to a f64 ground-truth reference.  A scalar_result is
// emitted to the CSV and a one-line summary is printed to stdout.
//
// Tests
// -----
//  1. Geometric progression
//       a_n = a_0 * r^n,  a_0=1, r=1.015, n=1000
//       Stress-tests repeated multiplication: errors accumulate
//       multiplicatively in IEEE, additively (in log-space) in LNS.
//       LNS should accumulate less rounding per step.
//
//  2. Euclidean norm
//       ||x|| = sqrt(sum(x_i^2)),  N=4096, x_i in [0.01, 100]
//       Exercises squared accumulation then a sqrt. The accumulator
//       suffers n additions; LNS's add error grows the total, while BF
//       add is correctly-rounded per step.  Shows BF16 advantage on add.
//
//  3. Alternating harmonic series (Leibniz pi)
//       sum_{k=0}^{9999} (-1)^k / (2k+1) * 4  ->  pi
//       Severe cancellation: terms grow smaller, alternating sign.
//       Both formats lose precision; relative error reveals catastrophic
//       cancellation for LNS whose add/sub is anchored to input scale.
//
//  4. Kahan / large-to-small accumulation (pi^2/6)
//       sum_{n=1}^{5000} 1/n^2  in two directions: forward (1->N) and
//       backward (N->1).  The backward sum aligns better with BF16's
//       error model (adding small to small then to large); forward sum
//       adds small increments to a large accumulator, which exercises
//       LNS's weakness in add.
//
//  5. Sigmoid activation
//       sigma(x) = 1 / (1 + exp(-x)),  x swept over [-10, 10], 1001 pts.
//       Exercises exp (implemented as exp2 via log2 lookup in LNS) and
//       reciprocal (division).  avg_rel over all sweep points is reported.
//
//  6. GELU activation
//       GELU(x) = 0.5*x*(1+tanh(sqrt(2/pi)*(x+0.044715*x^3))),
//       x swept over [-4, 4], 801 pts.
//       Exercises mul, add, and a tanh approximation chain.
// ---------------------------------------------------------------------------

// Run all six numerical tests, printing to stdout and writing to the
// already-open CSV.  lns_table_path is informational only (tables already
// loaded by caller).
void bench_numerical_run(const char* results_dir);

#endif // !__BENCH_NUMERICAL_H__
