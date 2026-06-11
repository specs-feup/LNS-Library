#include "bench_ops.h"
#include "rng.h"
#include "csv.h"
#include "lnssim.hpp"
#include "bfloatsim.hpp"

#include <math.h>

// ---------------------------------------------------------------------------
// Band tables
// ---------------------------------------------------------------------------

const band_t BANDS8[] = {
  { 1.0f / 8.0f, 1.0f,     "[2^(-p+1), 1]" },
  { 1.0f,        2.0f,     "[1, 2]"         },
  { 2.0f,        4.0f,     "[2, 4]"         },
};
const u32 N_BANDS8  = sizeof(BANDS8)  / sizeof(BANDS8[0]);

const band_t BANDS16[] = {
  { 1.0f / 128.0f, 1.0f,      "[2^(-p+1), 1]" },
  { 1.0f,          2.0f,      "[1, 2]"         },
  { 2.0f,          4.0f,      "[2, 4]"         },
  { 4.0f,          8.0f,      "[4, 8]"         },
  { 8.0f,          16.0f,     "[8, 16]"        },
  { 16.0f,         32.0f,     "[16, 32]"       },
  { 32.0f,         64.0f,     "[32, 64]"       },
  { 64.0f,         65536.0f,  "[64, 2^16]"     },
};
const u32 N_BANDS16 = sizeof(BANDS16) / sizeof(BANDS16[0]);

// ---------------------------------------------------------------------------
// Format-aware cancellation filter.
// Rejects pairs where |a+b| < 2^(-f) * max(|a|,|b|).
// f = fractional bit-width: 3 for 8-bit formats, 7 for 16-bit formats.
// ---------------------------------------------------------------------------

static int cancels(f64 a, f64 b, u8 f) {
  const f64 result = a + b;
  const f64 scale  = fmax(fabs(a), fabs(b));
  return scale > 0.0 && fabs(result) < ldexp(1.0, -(i32)f) * scale;
}

// ---------------------------------------------------------------------------
// Per-format test runners
// ---------------------------------------------------------------------------

static format_stats run_lns8(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  fstats_init(&fs);
  const u8 F = 3;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SB8(lo, hi);
    stats_update(&fs.rt, (f32)(lns32)lns8(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const lns32 
      a = SB8(lo, hi),
      b = SB8(lo, hi);

    const lns32 exp = a * b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns8(a) * lns8(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.mul, (f32)got, (f32)exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const lns32 
      a = SB8(lo, hi),
      b = SB8(lo, hi);

    const lns32 exp = a / b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns8(a) / lns8(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.div_, (f32)got, (f32)exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const lns32
      a = SB8(lo, hi),
      b = SB8(lo, hi);

    if (cancels((f32)a, (f32)b, F))
      continue;

    const lns32 exp = a + b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns8(a) + lns8(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.add_, (f32)got, (f32)exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const lns32 
      a = SB8(lo, hi),
      b = SB8(lo, hi);

    if (cancels((f32)a, (f32)-b, F))
      continue;

    const lns32 exp = a - b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns8(a) - lns8(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.sub_, (f32)got, (f32)exp);
    k++;
  }

  return fs;
}

static format_stats run_bf8(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  fstats_init(&fs);
  const u8 F = 3;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SBBF8(lo, hi);
    stats_update(&fs.rt, (f32)bf8(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);

    const f32 exp = a * b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a * b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.mul, got, exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);

    const f32 exp = a / b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a / b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.div_, got, exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);

    if (cancels(a, b, F))
      continue;

    const f32 exp = a + b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a + b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.add_, got, exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF8(lo, hi),
      b = SBBF8(lo, hi);

    if (cancels(a, -b, F))
      continue;

    const f32 exp = a - b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf8(a - b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.sub_, got, exp);
    k++;
  }

  return fs;
}

static format_stats run_lns16(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  fstats_init(&fs);
  const u8 F = 7;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SB16(lo, hi);
    stats_update(&fs.rt, (f32)lns16(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const lns32 
      a = SB16(lo, hi),
      b = SB16(lo, hi);

    const lns32 exp = a * b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns16(a) * lns16(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.mul, (f32)got, (f32)exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const lns32 
      a = SB16(lo, hi),
      b = SB16(lo, hi);

    const lns32 exp = a / b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns16(a) / lns16(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.div_, (f32)got, (f32)exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const lns32 
      a = SB16(lo, hi),
      b = SB16(lo, hi);

    if (cancels((f32)a, (f32)b, F))
      continue;

    const lns32 exp = a + b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns16(a) + lns16(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.add_, (f32)got, (f32)exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SB16(lo, hi),
      b = SB16(lo, hi);

    if (cancels((f32)a, (f32)-b, F))
      continue;

    const lns32 exp = a - b;
    if (!isfinite((f32)exp))
      continue;

    const lns32 got = (lns32)(lns16(a) - lns16(b));
    if (!isfinite((f32)got))
      continue;

    stats_update(&fs.sub_, (f32)got, (f32)exp);
    k++;
  }

  return fs;
}

static format_stats run_bf16(u32 n, f32 lo, f32 hi) {
  format_stats fs;
  fstats_init(&fs);
  const u8 F = 7;

  for (u32 k = 0; k < n; k++) {
    const f32 a = SBBF16(lo, hi);
    stats_update(&fs.rt, (f32)bf16(a), a);
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF16(lo, hi),
      b = SBBF16(lo, hi);

    const f32 exp = a * b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a * b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.mul, got, exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF16(lo, hi),
      b = SBBF16(lo, hi);

    const f32 exp = a / b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a / b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.div_, got, exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF16(lo, hi),
      b = SBBF16(lo, hi);

    if (cancels(a, b, F))
      continue;

    const f32 exp = a + b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a + b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.add_, got, exp);
    k++;
  }

  for (u32 k = 0; k < n; ) {
    const f32 
      a = SBBF16(lo, hi),
      b = SBBF16(lo, hi);

    if (cancels(a, -b, F))
      continue;

    const f32 exp = a - b;
    if (!isfinite(exp))
      continue;

    const f32 got = (f32)bf16(a - b);
    if (!isfinite(got))
      continue;

    stats_update(&fs.sub_, got, exp);
    k++;
  }

  return fs;
}

// ---------------------------------------------------------------------------
// CSV flush helper — emits all five ops for one format in one band.
// ---------------------------------------------------------------------------

static void flush_csv(const char* fmt, const char* band, const format_stats* fs) {
  csv_write_ops(fmt, band, "rt",  &fs->rt);
  csv_write_ops(fmt, band, "mul", &fs->mul);
  csv_write_ops(fmt, band, "div", &fs->div_);
  csv_write_ops(fmt, band, "add", &fs->add_);
  csv_write_ops(fmt, band, "sub", &fs->sub_);
}

// ---------------------------------------------------------------------------
// bench_ops_run
// ---------------------------------------------------------------------------

void bench_ops_run(u32 n_samples, const char* results_dir) {
  (void)results_dir; // csv_out is already open; here for API symmetry.

  for (u32 b = 0; b < N_BANDS8; b++) {
    format_stats fs_lns = run_lns8(n_samples, BANDS8[b].lo, BANDS8[b].hi);
    format_stats fs_bf  = run_bf8 (n_samples, BANDS8[b].lo, BANDS8[b].hi);
    flush_csv("lns8", BANDS8[b].label, &fs_lns);
    flush_csv("bf8",  BANDS8[b].label, &fs_bf);
    stats_free(&fs_lns.rt);  stats_free(&fs_lns.mul); stats_free(&fs_lns.div_);
    stats_free(&fs_lns.add_); stats_free(&fs_lns.sub_);
    stats_free(&fs_bf.rt);   stats_free(&fs_bf.mul);  stats_free(&fs_bf.div_);
    stats_free(&fs_bf.add_);  stats_free(&fs_bf.sub_);
  }

  for (u32 b = 0; b < N_BANDS16; b++) {
    format_stats fs_lns = run_lns16(n_samples, BANDS16[b].lo, BANDS16[b].hi);
    format_stats fs_bf  = run_bf16 (n_samples, BANDS16[b].lo, BANDS16[b].hi);
    flush_csv("lns16", BANDS16[b].label, &fs_lns);
    flush_csv("bf16",  BANDS16[b].label, &fs_bf);
    stats_free(&fs_lns.rt);  stats_free(&fs_lns.mul); stats_free(&fs_lns.div_);
    stats_free(&fs_lns.add_); stats_free(&fs_lns.sub_);
    stats_free(&fs_bf.rt);   stats_free(&fs_bf.mul);  stats_free(&fs_bf.div_);
    stats_free(&fs_bf.add_);  stats_free(&fs_bf.sub_);
  }
}
