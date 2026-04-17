#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <cassert>
#include <cfloat>

#include <cstdint>
#include <cctype>
#include <cstring>

#include "utils.h"

#define DEBUG

struct Vec2D {
  f64 x, y;
};

template<typename T>
struct newtons_dd {
  std::vector<T> coefs, xs;
};

template<typename T>
using NewtonsDD = std::vector<newtons_dd<T>>;

#define NEWTONS_DD(n) std::vector<newtons_dd<T>>(n)

u8 log2_int(const i64 n) {
  if (n <= 0) {
    std::cerr << "[ERROR]: invalid domain for log2_i64" << std::endl;
    exit(1);
  }
  return 63 - __builtin_clzll(static_cast<u64>(n));
}

// Returns the minimum x such that f+/f-(x) is sensitive to changes at LNS
// precision p. sign=0 -> f+ domain, sign=1 -> f- domain.
// Formula: x_min = log2(±(2^(±2^-p) - 1))  (Table 3 / Section 4.1.2)
inline f64 lns_min(const u8 precision, const u8 sign) {
  const f64 two_neg_p = std::pow(2.0, -(f64)precision);
  // sign = 0 (f+): x = log2(2^(+2^-p) - 1)
  // sign = 1 (f-): x = log2(1 - 2^(-2^-p))
  const f64 inner = std::pow(2.0, (sign ? -1.0 : 1.0) * two_neg_p) - 1.0;
  return std::log2(std::abs(inner)) * (sign ? -1.0 : 1.0);
}

// Searches for the NDD segment whose interval contains x, using the stored
// xs boundaries (value-range lookup). Returns segment index.
template<typename T>
u64 newtons_dd_find_segment(const NewtonsDD<T>& ndd, const T x) {
  // xs[i] stores the left boundary of segment i in fixed-point.
  // We want the last segment whose left boundary <= x.
  u64 lo = 0, hi = ndd.size() - 1;
  while (lo < hi) {
    u64 mid = (lo + hi + 1) / 2;
    if (ndd[mid].xs[0] <= x)
      lo = mid;
    else
      hi = mid - 1;
  }
  return lo;
}

template<typename T>
T newtons_dd_calculate(const NewtonsDD<T>& ndd, const T x) {
  u64 i = newtons_dd_find_segment(ndd, x);

  assert(i < ndd.size());

  T
    y       = ndd[i].coefs[0],
    x_to_j  = x - ndd[i].xs[0];

  for (u64 j = 1; j < ndd[i].coefs.size(); j++) {
    y += ndd[i].coefs[j] * x_to_j;
    // xs has size() == coefs.size() - 1, guard the last iteration
    if (j < ndd[i].xs.size())
      x_to_j *= x - ndd[i].xs[j];
  }

  return y;
}

f64 sum_log2_factorial(u16 n) {
  f64 x = 0;
  for (; n > 1; x += std::log2(n), n--);
  return x;
}

// Returns the minimum polynomial degree n such that the interpolation error
// on [x_i - h_i, x_i] is <= 2^-precision for f- (sign=1).
// Uses the asymptotic bound (eq. 14/15 in the paper):
//   p- ~ Omega(log2((n+1)!) + (n+1)*log2(h^-1) - 2*x_i + log2(1 - 2^x_i))
// The sign=0 (f+) bound drops the log2(1-2^x_i) curvature term.
u16 newtons_dd_degree(
  const u8 precision, const f64 h_i, const f64 x_i,
  const u8 n_max, const u8 sign
) {
  assert(std::abs(x_i)  > DBL_MIN);
  assert(std::abs(x_i)  > std::pow(2.0, -(f64)precision));
  assert(std::abs(h_i)  > DBL_MIN);

  const f64
    interval_const = std::log2(1.0 / h_i),
    // curvature term: -2*x_i for f+, -2*x_i + log2(1-2^x_i) for f-
    curvature = -2.0 * x_i + (sign ? std::log2(1.0 - std::pow(2.0, x_i)) : 0.0);

  const i8 max = (i8)std::floor(
        sum_log2_factorial(n_max)
      + (n_max + 1) * interval_const
      + curvature
    );

  // Sanity: n_max must already be sufficient
  std::cout << x_i << ", " << h_i << std::endl;
  std::cout << (i32)precision << ", " << (i32)max << std::endl;

  u8 n = 0;
  for (
    f64 log_factorial_n_plus_1 = 0.0; // log2((n+1)!)
    (i8)precision > (i8)std::floor(
      log_factorial_n_plus_1
      + (n + 1) * interval_const
      + curvature
    )
    && 
    n <= n_max;
    n++, log_factorial_n_plus_1 += std::log2((f64)(n + 1))
  );

  std::cout << "Returned: " << (i32)n << std::endl << std::endl;

  return n;
}

inline f64 add_func(const f64 x) {
  return std::log2(1.0 + std::exp2(x));
}

inline f64 sub_func(const f64 x) {
  return std::log2(1.0 - std::exp2(x));
}

template<typename T>
inline f64 QI_M_to_f64(const T x, const u8 precision) {
  return static_cast<f64>(x) / (f64)(1 << precision);
}

template<typename T>
inline T f64_to_QI_M(const f64 x, const u8 precision) {
  return static_cast<T>(x * (f64)(1 << precision));
}

// Build a single NDD struct from an interval of Vec2D sample points.
template<typename T>
newtons_dd<T> newtons_dd_create(const std::vector<Vec2D>& interval, const u8 precision) {
  const u64 m = interval.size();
  std::vector<std::vector<f64>> lut(m, std::vector<f64>(m, 0.0));

  // 0th order divided differences = function values
  for (u64 j = 0; j < m; j++)
    lut[0][j] = interval[j].y;

  // Higher order divided differences
  for (u64 i = 1; i < m; i++)
    for (u64 j = 0; j < m - i; j++)
      lut[i][j] = (lut[i - 1][j + 1] - lut[i - 1][j])
                / (interval[j + i].x - interval[j].x);

  newtons_dd<T> ndd;
  ndd.coefs.resize(m);
  ndd.xs.resize(m - 1);

  for (u64 i = 0; i < m; i++)
    ndd.coefs[i] = f64_to_QI_M<T>(lut[i][0], precision);
  for (u64 i = 0; i < m - 1; i++)
    ndd.xs[i] = f64_to_QI_M<T>(interval[i].x, precision);

  return ndd;
}

template<typename T>
NewtonsDD<T> newtons_dd_greedy(const std::vector<std::vector<Vec2D>>& intervals, const u8 precision) {
  NewtonsDD<T> ndd(intervals.size());
  for (u64 i = 0; i < ndd.size(); i++)
    ndd[i] = newtons_dd_create<T>(intervals[i], precision);
  return ndd;
}

// Prints a table of (max_degree, total_bytes) for each h_0 = 2^-h_0_log2
// strategy, across both the "dyadic sweep from x_min" partition and the
// finer uniform-h partitions, for sign in {0,1}.
template<typename T>
void test_ndd_sizes(const u8 lns, const u8 precision, const f64 h_min, const u8 n_max, const u8 sign) {
  const u8
    h_min_log2 = std::min(precision, (u8)std::floor(-std::log2(h_min))),
    s_hs       = h_min_log2 + 1;

  // Baseline row: dyadic partition from x_min to -1
  u8  n_base = 0;
  u32 s_base = 0;
  const f64 x_1 = 1 - std::pow(2.0, -precision);

  for (f64 x = lns_min(precision, sign); x <= x_1; ) {
    const f64 x_i = x / 2.0;   // next boundary (halving toward 0)
    const f64 h_i = x_i - x;

    const u8 n_i = (u8)newtons_dd_degree(precision, h_i, x_i, n_max, sign);
    n_base = std::max(n_base, n_i);
    s_base += (2 * n_i + 1) * (u32)sizeof(T);

    x = x_i;
  }

  // One row per h_0 = 2^-h_0_log2
  std::vector<std::pair<u32, u32>> precision_table(s_hs,
    std::pair<u32, u32>((u32)n_base, s_base));

  for (u8 h_0_log2 = 0; h_0_log2 <= h_min_log2; h_0_log2++) {
    f64 x = x_1;

    for (u8 h_i_log2 = 1; h_i_log2 <= h_0_log2; h_i_log2++) {
      const f64
        h_i = std::pow(2.0, -(f64)h_i_log2),
        x_i = x + h_i;

      const u8 n_i = (u8)newtons_dd_degree(precision, h_i, x_i, n_max, sign);
      precision_table[h_0_log2].first =
        std::max(precision_table[h_0_log2].first, (u32)n_i);
      precision_table[h_0_log2].second += (2 * n_i + 1) * (u32)sizeof(T);

      x = x_i;
    }

    // Final segment up to 0
    {
      const f64
        h_i = std::pow(2.0, -(f64)h_0_log2),
        x_i = x + h_i;

      const u8 n_i = (u8)newtons_dd_degree(precision, h_i, x_i, n_max, sign);
      precision_table[h_0_log2].first =
        std::max(precision_table[h_0_log2].first, (u32)n_i);
      precision_table[h_0_log2].second += (2 * n_i + 1) * (u32)sizeof(T);
    }
  }

  const char* fname = sign ? "f-" : "f+";
  std::cout << "NewtonsDD Maximum Degree and Size for " << fname
            << " in LNS" << (i32)lns
            << " Q" << (lns - 1 - precision) << "." << (i32)precision
            << ":\n";

  std::cout << std::setw(4) << " ";
  for (u8 h_i = 0; h_i < s_hs; h_i++)
    std::cout << std::setw(16) << ("h=2^-" + std::to_string(h_i));
  std::cout << "\n";

  std::cout << std::setw(4) << " ";
  for (u64 j = 0; j < s_hs; j++)
    std::cout
      << std::setw(16)
      << ("(" + std::to_string(precision_table[j].first)
          + ", " + std::to_string(precision_table[j].second) + ")");
  std::cout << "\n";
}

template<typename T>
void write_tables_binary(
  const NewtonsDD<T>& ndd_plus,
  const NewtonsDD<T>& ndd_minus,
  const u8 lns, const u8 int_digits, const u8 precision
) {
  mkdir("lns_tables", 0755);

  std::ostringstream fname;
  fname << "lns_tables/"
        << "newtonsdd_"
        << (i32)lns
        << "_q" << (i32)int_digits
        << "_" << (i32)precision
        << ".lns";

  std::ofstream file(fname.str(), std::ios::binary);
  if (!file) {
    std::cerr << "[ERROR]: could not open " << fname.str()
              << " for writing" << std::endl;
    exit(1);
  }

  const u32
    s_plus  = (u32)ndd_plus.size(),
    s_minus = (u32)ndd_minus.size();

  file.write(reinterpret_cast<const char*>(&s_plus),  sizeof(u32));
  file.write(reinterpret_cast<const char*>(&s_minus), sizeof(u32));

  for (u32 i = 0; i < s_plus; i++) {
    const u32
      s_coefs = (u32)ndd_plus[i].coefs.size(),
      s_xs    = (u32)ndd_plus[i].xs.size();

    file.write(reinterpret_cast<const char*>(&s_coefs), sizeof(u32));
    file.write(reinterpret_cast<const char*>(&s_xs),    sizeof(u32));

    file.write(reinterpret_cast<const char*>(ndd_plus[i].coefs.data()), sizeof(T) * s_coefs);
    file.write(reinterpret_cast<const char*>(ndd_plus[i].xs.data()),    sizeof(T) * s_xs);
  }

  for (u32 i = 0; i < s_minus; i++) {
    const u32
      s_coefs = (u32)ndd_minus[i].coefs.size(),
      s_xs    = (u32)ndd_minus[i].xs.size();

    file.write(reinterpret_cast<const char*>(&s_coefs), sizeof(u32));
    file.write(reinterpret_cast<const char*>(&s_xs),    sizeof(u32));

    file.write(reinterpret_cast<const char*>(ndd_minus[i].coefs.data()), sizeof(T) * s_coefs);
    file.write(reinterpret_cast<const char*>(ndd_minus[i].xs.data()),    sizeof(T) * s_xs);
  }

  file.close();
  std::cout << "[INFO]: Written to " << fname.str() << std::endl;
}

bool strisdigit(char* str) {
  if (str == NULL || *str == '\0')
    return false;
  str += *str == '+' || *str == '-';
  for (; *str != '\0'; str++)
    if (!isdigit(*str))
      return false;
  return true;
}

void print_help() {
  std::cout << "newtonsdd <--gen | --test> <args> <lns>" << std::endl;
  std::cout << "--gen <precision (fraction): [3,52]> <-log2(h_min): [0,precision]> <lns: { 8, 16, 32, 64 }> <int digits: lns8 - [4,6], lns16 - [4,9], lns32 - [6, 12], lns64 - [8,16]>" << std::endl;
  std::cout << "--test <lns: { 8, 16, 32, 64 }> <int digits: lns8 - [4,6], lns16 - [4,9], lns32 - [6, 12], lns64 - [8,16]>"
            << "<h_min: [0, lns - int digits - 1]>"
            << "<n_max: [4, 24]>" << std::endl;
}

void print_error() {
  std::cerr << "[ERROR]: must choose between generating tables into output file or testing tables" << std::endl;
  print_help();
}

// Validates lns bits and returns max allowed int_digits, or -1 if invalid.
static i8 lns_max_int(const u8 lns_bits) {
  switch (lns_bits) {
    case  8: return 6;
    case 16: return 9;
    case 32: return 12;
    case 64: return 16;
    default: return -1;
  }
}

// Validates lns bits and returns min allowed int_digits, or -1 if invalid.
static i8 lns_min_int(const u8 lns_bits) {
  switch (lns_bits) {
    case  8: return 4;
    case 16: return 4;
    case 32: return 6;
    case 64: return 8;
    default: return -1;
  }
}

i32 main(i32 argc, char* argv[]) {
  if (argc == 2 && strcmp(argv[1], "--help") == 0) {
    print_help();
    return 0;
  }

  // Both modes need at least: <cmd> + 4 args = 5
  if (argc < 5) {
    print_error();
    exit(1);
  }

  const char* command = argv[1];
  const bool gen  = strcmp(command, "--gen")  == 0;
  const bool test = strcmp(command, "--test") == 0;

  if (!gen && !test) {
#ifdef DEBUG
    std::cout << "[DEBUG] unknown command: " << command << std::endl;
#endif
    print_error();
    exit(1);
  }

  // ── --gen <precision> <-log2(h_min)> <lns> <int_digits> ──────────────────
  if (gen) {
    // needs exactly 5 args after argv[0]
    if (argc != 6) {
      print_error();
      exit(1);
    }

    if (!strisdigit(argv[2])) {
      std::cerr << "[ERROR]: precision must be a number: " << argv[2] << std::endl;
      print_help(); exit(1);
    }
    const u8 precision = (u8)strtol(argv[2], NULL, 10);
    if (precision < 3 || precision > 52) {
      std::cerr << "[ERROR]: precision must be in [3, 52]" << std::endl;
      exit(1);
    }

    if (!strisdigit(argv[3])) {
      std::cerr << "[ERROR]: -log2(h_min) must be a number: " << argv[3] << std::endl;
      print_help(); exit(1);
    }
    const u8 h_min_log2 = (u8)strtol(argv[3], NULL, 10);
    if (h_min_log2 > precision) {
      std::cerr << "[ERROR]: -log2(h_min) must be in [0, precision]" << std::endl;
      exit(1);
    }

    if (!strisdigit(argv[4])) {
      std::cerr << "[ERROR]: lns must be a number: " << argv[4] << std::endl;
      print_help(); exit(1);
    }
    const u8 lns_bits = (u8)strtol(argv[4], NULL, 10);
    if (lns_max_int(lns_bits) < 0) {
      std::cerr << "[ERROR]: lns must be one of { 8, 16, 32, 64 }" << std::endl;
      exit(1);
    }

    if (!strisdigit(argv[5])) {
      std::cerr << "[ERROR]: int_digits must be a number: " << argv[5] << std::endl;
      print_help(); exit(1);
    }
    const u8 int_digits = (u8)strtol(argv[5], NULL, 10);
    if (int_digits < lns_min_int(lns_bits) || int_digits > lns_max_int(lns_bits)) {
      std::cerr << "[ERROR]: int_digits out of range for lns" << (i32)lns_bits << std::endl;
      exit(1);
    }

    std::cout << "[INFO]: --gen interval builder not yet implemented." << std::endl;
    // TODO: build intervals, call newtons_dd_greedy, call write_tables_binary
    return 0;
  }

  // ── --test <lns> <int_digits> <h_min_log2> <n_max> ───────────────────────
  // needs exactly 5 args after argv[0]
  if (argc != 6) {
    print_error();
    exit(1);
  }

  if (!strisdigit(argv[2])) {
    std::cerr << "[ERROR]: lns must be a number: " << argv[2] << std::endl;
    print_help(); exit(1);
  }
  const u8 lns_bits = (u8)strtol(argv[2], NULL, 10);
  if (lns_max_int(lns_bits) < 0) {
    std::cerr << "[ERROR]: lns must be one of { 8, 16, 32, 64 }" << std::endl;
    exit(1);
  }

  if (!strisdigit(argv[3])) {
    std::cerr << "[ERROR]: int_digits must be a number: " << argv[3] << std::endl;
    print_help(); exit(1);
  }
  const u8 int_digits = (u8)strtol(argv[3], NULL, 10);
  if (int_digits < lns_min_int(lns_bits) || int_digits > lns_max_int(lns_bits)) {
    std::cerr << "[ERROR]: int_digits out of range for lns" << (i32)lns_bits << std::endl;
    exit(1);
  }
  const u8 precision = (u8)(lns_bits - 1 - int_digits);

  if (!strisdigit(argv[4])) {
    std::cerr << "[ERROR]: h_min must be a number: " << argv[4] << std::endl;
    print_help(); exit(1);
  }
  const u8 h_min_log2 = (u8)strtol(argv[4], NULL, 10);
  if (h_min_log2 > lns_bits - int_digits - 1) {
    std::cerr << "[ERROR]: h_min out of range [0, lns - int_digits - 1]" << std::endl;
    exit(1);
  }

  if (!strisdigit(argv[5])) {
    std::cerr << "[ERROR]: n_max must be a number: " << argv[5] << std::endl;
    print_help(); exit(1);
  }
  const u8 n_max = (u8)strtol(argv[5], NULL, 10);
  if (n_max < 4 || n_max > 24) {
    std::cerr << "[ERROR]: n_max must be in [4, 24]" << std::endl;
    exit(1);
  }

  const f64 h_min = std::pow(2.0, -(f64)h_min_log2);

  std::cout << "Test:\n"
            << "  lns"    << (i32)lns_bits
            << " Q"       << (i32)int_digits << "." << (i32)precision << "\n"
            << "  -log2(h_min) = " << (i32)h_min_log2 << "\n"
            << "  n_max = "        << (i32)n_max << "\n\n";

  // Dispatch on lns_bits for the fixed-point type width.
  // lns8  -> i8 coefficients, lns16 -> i16, lns32/64 -> i32
  // (lns64 would ideally use i64 but that's a future concern)
  switch (lns_bits) {
    case 8:
      test_ndd_sizes<i8> (lns_bits, precision, h_min, n_max, 0);
      std::cout << "\n";
      test_ndd_sizes<i8> (lns_bits, precision, h_min, n_max, 1);
      break;
    case 16:
      test_ndd_sizes<i16>(lns_bits, precision, h_min, n_max, 0);
      std::cout << "\n";
      test_ndd_sizes<i16>(lns_bits, precision, h_min, n_max, 1);
      break;
    case 32:
    case 64:
      test_ndd_sizes<i32>(lns_bits, precision, h_min, n_max, 0);
      std::cout << "\n";
      test_ndd_sizes<i32>(lns_bits, precision, h_min, n_max, 1);
      break;
  }

  return 0;
}
