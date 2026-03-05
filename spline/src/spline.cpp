#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include <sys/stat.h>

#include <cstdint>
#include <cctype>
#include <cstring>

#include "utils.h"

#define DEBUG

typedef struct point {
  i16 x, y, m, b;
} Point;

u8 log2_int(const i16 n) {
  if (n <= 0x0001)
    return 0x00;
  if (n <= 0x0002)
    return 0x01;
  if (n <= 0x0004)
    return 0x02;
  if (n <= 0x0008)
    return 0x03;
  if (n <= 0x0010)
    return 0x04;
  if (n <= 0x0020)
    return 0x05;
  if (n <= 0x0040)
    return 0x06;
  if (n <= 0x0080)
    return 0x07;
  if (n <= 0x0100)
    return 0x08;
  if (n <= 0x0200)
    return 0x09;
  if (n <= 0x0400)
    return 0x0A;
  if (n <= 0x0800)
    return 0x0B;
  if (n <= 0x1000)
    return 0x0C;
  if (n <= 0x2000)
    return 0x0D;
  if (n <= 0x4000)
    return 0x0E;
  std::cerr << "[ERROR]: invalid domain for log2_int16" << std::endl;
  exit(1);
}

i16 calculate_xmb(const std::vector<Point>& table, const i16 x, const i16 s_table, const i16 precision) {
  i16 i = 0;
  for (; x > table[i].x && i < s_table; i++);
  return (((i32)table[i].m * (i32)x) >> precision) + table[i].b;
}

i16 calculate_xf(const std::vector<Point>& table, const i16 x, const i16 s_table) {
  i16 i = 0;
  for (; x > table[i].x && i < s_table; i++);
  if (i == 0)
    return 0;

  const i16 h = table[i].x - table[i - 1].x;
  const i16 log2_h = log2_int(h);

  const i32 
    a = ((i32)(x - table[i - 1].x)) * table[i].y, // QI.M * QI.M = Q2I.2M
    b = ((i32)(table[i].x - x)) * table[i - 1].y, // QI.M * QI.M = Q2I.2M
    c = (a + b) >> log2_h; // Q2I.2M >> log2_h = Q(2I + M).M
  
  return (i16)c;
}

f64 err(const f64 b, const f64 h, const i8 sign) {
  f64 numerator = std::pow(h, 2) * std::pow(2.0, b);
  f64 denominator = std::pow(1.0 + std::pow(-1.0, sign) * std::pow(2.0, b), 2);
  return numerator / denominator;
}

f64 add_func(const f64 diff) {
  return std::log2(1.0 + std::exp2(diff));
}

f64 sub_func(const f64 diff) {
  return std::log2(1.0 - std::exp2(diff));
}

f64 QI_M_to_f64(const i16 a, const i16 precision) {
  return static_cast<f64>(a) / (f64)(1 << precision);
}

i16 f64_to_QI_M(const f64 a, const i16 precision) {
  return static_cast<i16>(a * (f64)(1 << precision));
}

void greedy_spline(
  std::vector<Point>& table, size_t s_table,
  const i8 sign, const i16 precision
) {
  const i16
    add_start = ~(1 << (3 + precision)) + 1, // -8 at any precision, we take the complement of it
    sub_start = ~(1 << (3 + precision)),     // -8 - 2^precision, we take the complement and subtract one (which is just taking the ~)
    add_end   = 0,                           // 0
    sub_end   = -1;                          // -2^precision

  std::vector<i16> hs;
  std::vector<f64> error;

  table.push_back({(i16)(sign ? sub_start : add_start), 0, 0, 0});
  table.push_back({(i16(sign ? sub_end : add_end)), 0, 0, 0});

  for (size_t i = 0; i < s_table - 2; i++) {
    for (size_t j = 0; j < table.size() - 1; j++) {
      hs.push_back(table[j + 1].x - table[j].x);
      error.push_back(err(QI_M_to_f64(table[j].x, precision), QI_M_to_f64(hs[j], precision), sign));
    }

    size_t max_index = 0;
    for (size_t j = 1; j < hs.size(); j++)
      if ((error[j] > error[max_index]) && (hs[j] > 1))
        max_index = j;

    table.insert(
      table.begin() + max_index + 1,
      {(i16)(table[max_index].x + (hs[max_index] >> 1)), 0, 0, 0}
    );

    hs.clear();
    error.clear();
  }

  for (size_t i = 0; i < s_table; i++)
    table[i].y = f64_to_QI_M(
      sign == 1 ? 
        (sub_func(QI_M_to_f64(table[i].x, precision)))
      : (add_func(QI_M_to_f64(table[i].x, precision))),
      precision
    );

  for (size_t i = 1; i < s_table; i++) {
    i32 temp = (i32)(table[i].y) - (i32)(table[i - 1].y);
    temp <<= precision;
    temp >>= log2_int(table[i].x - table[i - 1].x);
    table[i].m = (i16)(temp);
    table[i].b = table[i].y - (i16)((i32)(table[i].m) * (i32)(table[i].x) >> precision);
  }
}

void test_table_sizes(const size_t s_max, const i8 sign, const bool xf_mode, const bool lns16, const i16 precision) {
  const i16 s_precision_values = precision + 1;
  std::vector<i32> precision_values(s_precision_values, -1);

  const i16
    add_start = ~(1 << (3 + precision)) + 1, // -8 at any precision, we take the complement of it
    sub_start = ~(1 << (3 + precision)),     // -8 - 2^precision, we take the complement and subtract one (which is just taking the ~)
    add_end   = 0,                           // 0
    sub_end   = -1;                          // -2^precision
  
  i16
    start = sign == 1 ? sub_start : add_start,
    end = sign == 1 ? sub_end : add_end;

  for (size_t s_table = 2; s_table < s_max + 1; s_table++) {
    std::vector<Point> table;
    greedy_spline(table, s_table, sign, precision);

    i16 max_err = 0;
    for (i16 x = start; x < end + 1; x++) {
      i16 int_result = xf_mode ? calculate_xf(table, x, s_table) : calculate_xmb(table, x, s_table, precision);
      f64 f64_result = sign == 1 ? sub_func(QI_M_to_f64(x, precision)) : add_func(QI_M_to_f64(x, precision));

      i16 err = std::abs(int_result - f64_to_QI_M(f64_result, precision));
      if (err > max_err)
        max_err = err;
    }

    const i16 error_bit = log2_int(max_err + 1);
    if (error_bit < s_precision_values && precision_values[error_bit] == -1)
      precision_values[error_bit] = s_table;

    if (error_bit == 0)
      break;
  }

  std::cout << "Table Size for each error in LNS"
            << (lns16 ? "16" : "8")
            << (sign ? " subtraction" : " addition")
            << (xf_mode ? " xf" : " xmb")
            << ":"
            << std::endl;

  for (size_t i = 0; i < precision_values.size(); i++)
    if (precision_values[i] != -1)
      std::cout << "Error " << i << ": " << precision_values[i] << " rows" << std::endl;
}

void print_help() {
  std::cout << "spline <--gen | --test> <args> <lns>" << std::endl;
  std::cout << "--gen <--xf | --xmb> <s_table (rows) +: [8,1024]> <s_table (rows) -: [8,1024]> <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>"
            << std::endl;
  std::cout << "--test <--xf | --xmb> <max s_table (rows): [8,1024]> <--lns16 | --lns8> <int digits: lns8 - [4,6], lns16 - [4,14]>"
            << std::endl;
}

void print_error() {
  std::cerr << "[ERROR]: must choose between generating tables into output file or testing tables" << std::endl;
  print_help();
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

void write_tables_binary(
  const std::vector<Point>& spline_plus,
  const std::vector<Point>& spline_minus,
  const bool lns16, const bool xf,
  const i16 int_digits, const i16 precision
) {
  mkdir("lns_tables", 0755); // ignored if already exists

  std::ostringstream fname;
  fname << "lns_tables/" 
        << (xf ? "xf_" : "xmb_")
        << (lns16 ? "16" : "8")
        << "_q" << int_digits
        << "_" << precision
        << ".lns";

  std::ofstream file(fname.str(), std::ios::binary);
  if (!file) {
    std::cerr << "[ERROR]: could not open " << fname.str() << " for writing" << std::endl;
    exit(1);
  }

  const u32 
    s_plus  = (u32)spline_plus.size(),
    s_minus = (u32)spline_minus.size();

  file.write(reinterpret_cast<const char*>(&s_plus),  sizeof(u32));
  file.write(reinterpret_cast<const char*>(&s_minus), sizeof(u32));

  if (lns16) {
    if (xf) {
      for (const auto& p : spline_plus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i16));
      }
        
      for (const auto& p : spline_minus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i16));
      }

    } else {
      for (const auto& p : spline_plus)  {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i16));
      }

      for (const auto& p : spline_minus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i16));
      }
    }
  } else {
    if (xf) {
      for (const auto& p : spline_plus)  {
        const i8 
          x = p.x,
          y = p.y;

        file.write(reinterpret_cast<const char*>(&x), 1);
        file.write(reinterpret_cast<const char*>(&y), 1);
      }

      for (const auto& p : spline_minus) {
        const i8 
          x = p.x,
          y = p.y;

        file.write(reinterpret_cast<const char*>(&x), 1);
        file.write(reinterpret_cast<const char*>(&y), 1);
      }

    } else {
      for (const auto& p : spline_plus)  {
        const i8
          x = p.x,
          m = p.m,
          b = p.b;

        file.write(reinterpret_cast<const char*>(&x), 1);
        file.write(reinterpret_cast<const char*>(&m), 1);
        file.write(reinterpret_cast<const char*>(&b), 1);
      }

      for (const auto& p : spline_minus) {
        const i8
          x = p.x,
          m = p.m,
          b = p.b;

        file.write(reinterpret_cast<const char*>(&x), 1);
        file.write(reinterpret_cast<const char*>(&m), 1);
        file.write(reinterpret_cast<const char*>(&b), 1);
      }
    }
  }

  file.close();
  std::cout << "[INFO]: Written to " << fname.str() << std::endl;
}

i32 main(i32 argc, char* argv[]) {
  if (argc < 6 || argc > 7) {
    if (argc == 2 && strcmp(argv[1], "--help") == 0) { 
      print_help();
      return 0;
    }
    print_error();
    exit(1);
  }

  char* command = argv[1];
  const bool gen   = strcmp(command, "--gen") == 0;
  const bool test  = strcmp(command, "--test") == 0;
  const bool xf    = strcmp(argv[2], "--xf") == 0;
  const bool xmb   = strcmp(argv[2], "--xmb") == 0;

  if ((!gen && !test) || (!xf && !xmb)) {
    #ifdef DEBUG
      std::cout << "2" << std::endl;
    #endif
    print_error();
    exit(1);
  }

  const bool lns16 = strcmp(argv[4 + gen], "--lns16") == 0;
  const bool lns8  = strcmp(argv[4 + gen], "--lns8") == 0;
  if (!lns16 && !lns8) {
    #ifdef DEBUG
      std::cout << "3" << std::endl;
    #endif
    print_error();
    exit(1);
  }
  if (!strisdigit(argv[5 + gen])) {
    std::cerr << "[ERROR]: integer digits must be number " << argv[5 + gen] << std::endl;
    print_help();
    exit(1);
  }
  const i16 int_digits = (i16)strtol(argv[5 + gen], NULL, 10);
  if (int_digits < 4 || int_digits > 14 || (int_digits > 6 && lns8)) {
    std::cerr << "[ERROR]: integer digits out of range" << std::endl;
    print_help();
    exit(1);
  }
  const i16 precision = (lns16 ? 15 : 7) - int_digits;

  if (gen) {
    if (!strisdigit(argv[3]) || !strisdigit(argv[4])) {
      std::cerr << "[ERROR]: spline sizes must be numbers" << std::endl;
      print_help();
      exit(1);
    }

    const size_t
      s_table_plus  = strtol(argv[3], NULL, 10),
      s_table_minus = strtol(argv[4], NULL, 10);

    if (s_table_plus < 2 || s_table_plus > 1024 || s_table_minus < 2 || s_table_minus > 1024) {
      std::cerr << "[ERROR]: table sizes must be between 2 and 1024" << std::endl;
      exit(1);
    }

    std::vector<Point> spline_plus, spline_minus;
    greedy_spline(spline_plus,  s_table_plus,  0, precision);
    greedy_spline(spline_minus, s_table_minus, 1, precision);

    write_tables_binary(spline_plus, spline_minus, lns16, xf, int_digits, precision);

    return 0;
  }

  if (!strisdigit(argv[3])) {
    #ifdef DEBUG
      std::cout << "3" << std::endl;
    #endif
    std::cerr << "[ERROR]: spline sizes must be numbers" << std::endl;
    print_help();
    exit(1);
  }

  size_t s_max = strtol(argv[3], NULL, 10);
  if (s_max < 8 || s_max > 1024) {
    std::cerr << "[ERROR]: table sizes must be between 8 and 128" << std::endl;
    exit(1);
  }

  std::cout << "Test: " << std::endl;
  std::cout << "  " << (xf ? "xf" : "xmb") << std::endl;
  std::cout << "  max table size: " << s_max << std::endl;
  std::cout << "  lns" << (lns16 ? "16" : "8") << " Q" << (15 - precision) << "." << precision << std::endl;

  test_table_sizes(s_max, 0, xf, lns16, precision);
  test_table_sizes(s_max, 1, xf, lns16, precision);

  return 0;
}
