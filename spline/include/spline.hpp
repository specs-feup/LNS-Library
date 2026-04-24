#ifndef __SPLINE_HPP__
#define __SPLINE_HPP__

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

typedef struct spline_point {
  i16 x, y, m, b;
} SplinePoint;

void greedy_spline_float2lns (std::vector<SplinePoint>& table, const size_t s_table, const i16 precision);
void greedy_spline_lns2float (std::vector<SplinePoint>& table, const size_t s_table, const i16 precision);
void greedy_spline_add_func  (std::vector<SplinePoint>& table, const size_t s_table, const i16 precision);
void greedy_spline_sub_func  (std::vector<SplinePoint>& table, const size_t s_table, const i16 precision);

void test_float2lns_table_sizes (const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision);
void test_lns2float_table_sizes (const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision);
void test_add_table_sizes       (const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision);
void test_sub_table_sizes       (const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision);

void write_tables_binary(
  const std::vector<SplinePoint>& spline_plus, const std::vector<SplinePoint>& spline_minus,
  const bool lns16, const bool xf, const i16 int_digits, const i16 precision
);

void write_convert_table_binary(
  const std::vector<SplinePoint>& spline,
  const bool lns16, const bool f2l, const i16 int_digits, const i16 precision
);

i16 calculate_xmb (const std::vector<SplinePoint>& table, const i16 x, const i16 s_table, const i16 precision);
i16 calculate_xf  (const std::vector<SplinePoint>& table, const i16 x, const i16 s_table);

f64 float2lns     (const f64 m_x);
f64 lns2float     (const f64 lns_f_x);
f64 add_func      (const f64 diff);
f64 sub_func      (const f64 diff);

f64 float2lns_err (const f64 a, const f64 h);
f64 lns2float_err (const f64 b, const f64 h);
f64 add_func_err  (const f64 b, const f64 h);
f64 sub_func_err  (const f64 b, const f64 h);

f64 M_to_f64      (const i16 a);
i16 f64_to_M      (const f64 a);
f64 QI_M_to_f64   (const i16 a, const i16 precision);
i16 f64_to_QI_M   (const f64 a, const i16 precision);

u8  log2_int      (const i16 n);

#endif /* __SPLINE_HPP__ */
