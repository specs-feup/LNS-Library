#include <cstdio>
#include <cstdint>
#include <cstring>
#include <climits>
#include <cmath>

#include "lns_sim.hpp"

#ifndef __LNS_DEFS__
#define __LNS_DEFS__

using lns8  = lns<8,  4, 3>;
using lns16 = lns<16, 8, 7>;

#endif // !__LNS_DEFS__

#define RED    "\033[031m"
#define GREEN  "\033[032m"
#define BLUE   "\033[036m"
#define YELLOW "\033[033m"
#define RESET  "\033[0m"

struct op_stats {
  f32 max_abs_err;
  f32 min_abs_err;
  f64 sum_abs_err;
  f32 max_rel_err;
  f32 min_rel_err;
  f64 sum_rel_err;
  f32 worst_abs_a, worst_abs_b, worst_abs_exp, worst_abs_got;
  f32 worst_rel_a, worst_rel_b, worst_rel_exp, worst_rel_got;
  u32 count;

  op_stats() : max_abs_err(0), min_abs_err(1e38f), sum_abs_err(0),
               max_rel_err(0), min_rel_err(1e38f), sum_rel_err(0),
               worst_abs_a(0), worst_abs_b(0), worst_abs_exp(0), worst_abs_got(0),
               worst_rel_a(0), worst_rel_b(0), worst_rel_exp(0), worst_rel_got(0),
               count(0) {}

  void update(f32 got, f32 expected, f32 a, f32 b = 0.0f) {
    f32 abs_err = fabsf(got - expected);
    f32 rel_err = (fabsf(expected) > 1e-10f) ? abs_err / fabsf(expected) : abs_err;

    if (abs_err > max_abs_err) {
      max_abs_err = abs_err;
      worst_abs_a = a; worst_abs_b = b;
      worst_abs_exp = expected; worst_abs_got = got;
    }

    if (abs_err < min_abs_err)
      min_abs_err = abs_err;

    sum_abs_err += abs_err;

    if (rel_err > max_rel_err) {
      max_rel_err = rel_err;
      worst_rel_a = a; worst_rel_b = b;
      worst_rel_exp = expected; worst_rel_got = got;
    }

    if (rel_err < min_rel_err)
      min_rel_err = rel_err;

    sum_rel_err += rel_err;

    count++;
  }

  f32 avg_abs() const {
    return count ? (f32)(sum_abs_err / count) : 0.0f;
  }
  f32 avg_rel() const {
    return count ? (f32)(sum_rel_err / count) : 0.0f;
  }

  void print(const char* label) const {
    printf(BLUE "  %-8s" RESET " over %u tests\n", label, count);
    printf("    abs: min=" YELLOW "%.6f" RESET " avg=" YELLOW "%.6f" RESET " max=" YELLOW "%.6f" RESET "\n",
           (double)min_abs_err, (double)avg_abs(), (double)max_abs_err);
    printf("    rel: min=" YELLOW "%.6f" RESET " avg=" YELLOW "%.6f" RESET " max=" YELLOW "%.6f" RESET "\n",
           (double)min_rel_err, (double)avg_rel(), (double)max_rel_err);
    printf("    worst_abs: a=%.6f b=%.6f exp=%.6f got=%.6f\n",
           (double)worst_abs_a, (double)worst_abs_b, (double)worst_abs_exp, (double)worst_abs_got);
    printf("    worst_rel: a=%.6f b=%.6f exp=%.6f got=%.6f\n",
           (double)worst_rel_a, (double)worst_rel_b, (double)worst_rel_exp, (double)worst_rel_got);
  }

  void print_row(const char* label) const {
    printf("  %-8s"
           " abs[min=" YELLOW "%.5f" RESET " avg=" YELLOW "%.5f" RESET " max=" YELLOW "%.5f" RESET "]"
           " rel[min=" YELLOW "%.5f" RESET " avg=" YELLOW "%.5f" RESET " max=" YELLOW "%.5f" RESET "]\n",
           label,
           (double)min_abs_err, (double)avg_abs(), (double)max_abs_err,
           (double)min_rel_err, (double)avg_rel(), (double)max_rel_err);
  }
};

struct format_stats {
  op_stats 
    rt, mul, div_,
    stats_add, stats_sub;
};

static u32 rng_state = 0xdeadbeef;
static u32 rng_u32() {
  rng_state ^= rng_state << 13;
  rng_state ^= rng_state >> 17;
  rng_state ^= rng_state << 5;
  return rng_state;
}

static f32 rng_lns8_val() {
  u8 exp_bits = rng_u32() & 0x7F;
  i8 signed_exp = (exp_bits & 0x40) ? (i8)(exp_bits | 0x80) : (i8)exp_bits;

  f32 exp = (f32)signed_exp / 8.0f;  // range [-8, +7.875]
  
  if (exp > 3.0f)
    exp = 3.0f; // cap at 2^3 = 8
  
  if (exp < -3.0f)
    exp = -3.0f; // cap at 2^-3 = 0.125

  return powf(2.0f, exp);
}

static f32 rng_lns16_val() {
  u16 exp_bits = rng_u32() & 0x7FFF;
  i16 signed_exp = (exp_bits & 0x4000) ? (i16)(exp_bits | 0x8000) : (i16)exp_bits;

  f32 exp = (f32)signed_exp / 128.0f;  // range [-128, +127.992]
  
  if (exp > 6.0f)
    exp = 6.0f;         // cap at 2^6 = 64

  if (exp < -6.0f)
    exp = -6.0f;        // cap at 2^-6 = 0.015625
    
  return powf(2.0f, exp);
}

static format_stats test_lns8(u32 n_tests) {
  printf(BLUE "lns8 (8-bit, i=4, f=3) — %u tests per op\n" RESET, n_tests);

  format_stats fs;

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns8_val();
    fs.rt.update(lns8(a).convert(), a, a);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns8_val(), b = rng_lns8_val();
    fs.mul.update((lns8(a) * lns8(b)).convert(), a * b, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns8_val(), b = rng_lns8_val();
    fs.div_.update((lns8(a) / lns8(b)).convert(), a / b, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns8_val(), b = rng_lns8_val();
    f32 res = a + b;
    if (res > 0.0f)
      fs.stats_add.update((lns8(a) + lns8(b)).convert(), res, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns8_val(), b = rng_lns8_val();
    if (a > b) {
      f32 res = a - b;
      if (res > 0.0f)
        fs.stats_sub.update((lns8(a) - lns8(b)).convert(), res, a, b);
    }
  }

  return fs;
}

static format_stats test_lns16(u32 n_tests) {
  printf(BLUE "lns16 (16-bit, i=8, f=7) — %u tests per op\n" RESET, n_tests);

  format_stats fs;

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val();
    fs.rt.update(lns16(a).convert(), a, a);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    fs.mul.update((lns16(a) * lns16(b)).convert(), a * b, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    fs.div_.update((lns16(a) / lns16(b)).convert(), a / b, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    f32 res = a + b;
    if (res > 0.0f)
      fs.stats_add.update((lns16(a) + lns16(b)).convert(), res, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    if (a > b) {
      f32 res = a - b;
      if (res > 0.0f)
        fs.stats_sub.update((lns16(a) - lns16(b)).convert(), res, a, b);
    }
  }

  return fs;
}

static f32 to_bf16(f32 x) {
  u32 bits;
  memcpy(&bits, &x, sizeof(u32));

  bits &= 0xFFFF0000u;

  f32 result;
  memcpy(&result, &bits, sizeof(f32));

  return result;
}

static format_stats test_bf16(u32 n_tests) {
  printf(BLUE "bfloat16 — %u tests per op\n" RESET, n_tests);

  format_stats fs;

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val();
    fs.rt.update(to_bf16(a), a, a);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    fs.mul.update(to_bf16(to_bf16(a) * to_bf16(b)), a * b, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    fs.div_.update(to_bf16(to_bf16(a) / to_bf16(b)), a / b, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    f32 res = a + b;
    if (res > 0.0f)
      fs.stats_add.update(to_bf16(to_bf16(a) + to_bf16(b)), res, a, b);
  }

  for (u32 k = 0; k < n_tests; k++) {
    f32 a = rng_lns16_val(), b = rng_lns16_val();
    if (a > b) {
      f32 res = a - b;
      if (res > 0.0f)
        fs.stats_sub.update(to_bf16(to_bf16(a) - to_bf16(b)), res, a, b);
    }
  }

  return fs;
}

static void print_comparison_table(
  const char* title,
  const u32 n_ops, const u32* op_indices,
  const char* op_names[5],
  const op_stats* rows[3][5],
  const char* fmt_names[3]
) {
  printf(BLUE "  %s\n" RESET, title);
  printf(BLUE "──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────\n" RESET);

  printf("%-10s", "");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-46s", op_names[op_indices[i]]);
  printf("\n");

  printf("%-10s", "format");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-10s %-10s %-10s %-10s", "min_abs", "avg_abs", "avg_rel", "max_abs");
  printf("\n");

  printf("%-10s", "");
  for (u32 i = 0; i < n_ops; i++)
    printf("  %-10s %-10s %-10s %-10s", "----------", "----------", "----------", "----------");
  printf("\n");

  for (u32 fmt = 0; fmt < 3; fmt++) {
    printf("%-10s", fmt_names[fmt]);
    for (u32 i = 0; i < n_ops; i++) {
      const op_stats* s = rows[fmt][op_indices[i]];
      printf("  %-10.6f %-10.6f %-10.6f %-10.6f",
             (double)s->min_abs_err,
             (double)s->avg_abs(),
             (double)s->avg_rel(),
             (double)s->max_abs_err);
    }
    printf("\n");
  }

  const char* criteria[] = { "avg_abs", "avg_rel", "max_abs" };

  auto get_val = [](const op_stats* s, u32 crit) -> f32 {
    switch (crit) {
      case 0: return s->avg_abs();
      case 1: return s->avg_rel();
      case 2: return s->max_abs_err;
      default: return 1e38f;
    }
  };

  for (u32 crit = 0; crit < 3; crit++) {
    printf("%-10s", criteria[crit]);
    for (u32 i = 0; i < n_ops; i++) {
      u32 op = op_indices[i];
      f32 best = 1e38f;
      u32 winner = 0;

      for (u32 fmt = 0; fmt < 3; fmt++) {
        f32 val = get_val(rows[fmt][op], crit);

        if (val < best) {
          best = val;
          winner = fmt;
        }
      }

      printf("  " GREEN "%-46s" RESET, fmt_names[winner]);
    }
    printf("\n");
  }
  printf("\n");
}

static void print_comparison(const format_stats& fs8, const format_stats& fs16, const format_stats& fsbf) {
  const op_stats* rows[3][5] = {
    { &fs8.rt,  &fs8.mul,  &fs8.div_,  &fs8.stats_add,  &fs8.stats_sub  },
    { &fs16.rt, &fs16.mul, &fs16.div_, &fs16.stats_add, &fs16.stats_sub },
    { &fsbf.rt, &fsbf.mul, &fsbf.div_, &fsbf.stats_add, &fsbf.stats_sub },
  };
  const char* fmt_names[3] = { "lns8", "lns16", "bfloat16" };
  const char* op_names[5]  = { "rt", "mul", "div", "add", "sub" };

  printf("\n");
  printf(BLUE "══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" RESET);
  printf(BLUE "  COMPARISON TABLE\n" RESET);
  printf(BLUE "══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" RESET);
  printf("\n");

  const u32 rt_ops[]     = { 0 };
  const u32 muldiv_ops[] = { 1, 2 };
  const u32 addsub_ops[] = { 3, 4 };

  print_comparison_table("Round-trip",       1, rt_ops,     op_names, rows, fmt_names);
  print_comparison_table("Mul & Div",        2, muldiv_ops, op_names, rows, fmt_names);
  print_comparison_table("Add & Sub",        2, addsub_ops, op_names, rows, fmt_names);

  printf(BLUE "══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" RESET);
}

i32 main(const i32 argc, const char* argv[]) {
  u32 n_tests = 1000;

  assert(argc >= 2);

  const char
    *lns8_table_filename  = argv[1],
    *lns16_table_filename = argv[2];

  lns8_read_tables(lns8_table_filename);
  lns16_read_tables(lns16_table_filename);

  if (argc > 3)
    n_tests = (u32)atoi(argv[3]);

  format_stats fs8  = test_lns8(n_tests);
  printf("\n");

  format_stats fs16 = test_lns16(n_tests);
  printf("\n");

  format_stats fsbf = test_bf16(n_tests);
 
  print_comparison(fs8, fs16, fsbf);
  
  lns_close();

  return 0;
}
