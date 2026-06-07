#include "spline.hpp"

void greedy_spline_float2lns(std::vector<SplinePoint>& table, const size_t s_table) {
  const i16
    start = 0,
    end   = (1 << 15) - 1; // the closest to 1 we can get, 1 - 2^(-16)

  std::vector<i16> hs;
  std::vector<f64> error;

  table.push_back({ start, 0, 0, 0 });
  table.push_back({ end,   0, 0, 0 });

  for (size_t i = 0; i < s_table - 2; i++) {
    for (size_t j = 0; j < table.size() - 1; j++) {
      hs.push_back(table[j + 1].x - table[j].x);
      error.push_back(float2lns_err(M_to_f64(table[j].x), M_to_f64(hs[j])));
    }

    size_t max_index = 0;
    for (size_t j = 1; j < hs.size(); j++)
      if ((error[j] > error[max_index]) && (hs[j] > 1))
        max_index = j;

    table.insert(
      table.begin() + max_index + 1,
      { (i16)(table[max_index].x + (hs[max_index] >> 1)), 0, 0, 0 }
    );

    hs.clear();
    error.clear();
  }

  for (size_t i = 0; i < s_table; i++)
    table[i].y = f64_to_M(float2lns(M_to_f64(table[i].x)));

  for (size_t i = 1; i < s_table; i++) {
    i32 temp = (i32)(table[i].y) - (i32)(table[i - 1].y);
    const u8 h_i_log2 = log2_int(table[i].x - table[i - 1].x);
    table[i].m = (i16)(temp >> h_i_log2);
    table[i].b = table[i].y - (i16)((i32)(table[i].m) * (i32)(table[i].x) >> h_i_log2);
  }
}

void greedy_spline_lns2float(std::vector<SplinePoint>& table, const size_t s_table) {
  const i16
    start = 0,
    end   = (1 << 15) - 1; // the closest to 1 we can get, 1 - 2^(-16)

  std::vector<i16> hs;
  std::vector<f64> error;

  table.push_back({ start, 0, 0, 0 });
  table.push_back({ end,   0, 0, 0 });

  for (size_t i = 0; i < s_table - 2; i++) {
    for (size_t j = 0; j < table.size() - 1; j++) {
      hs.push_back(table[j + 1].x - table[j].x);
      error.push_back(lns2float_err(M_to_f64(table[j].x), M_to_f64(hs[j])));
    }

    size_t max_index = 0;
    for (size_t j = 1; j < hs.size(); j++)
      if ((error[j] > error[max_index]) && (hs[j] > 1))
        max_index = j;

    table.insert(
      table.begin() + max_index + 1,
      { (i16)(table[max_index].x + (hs[max_index] >> 1)), 0, 0, 0 }
    );

    hs.clear();
    error.clear();
  }

  for (size_t i = 0; i < s_table; i++)
    table[i].y = f64_to_M(lns2float(M_to_f64(table[i].x)));

  for (size_t i = 1; i < s_table; i++) {
    i32 temp = (i32)(table[i].y) - (i32)(table[i - 1].y);
    const u8 h_i_log2 = log2_int(table[i].x - table[i - 1].x);
    table[i].m = (i16)(temp >> h_i_log2);
    table[i].b = table[i].y - (i16)((i32)(table[i].m) * (i32)(table[i].x) >> h_i_log2);
  }
}

void greedy_spline_add_func(std::vector<SplinePoint>& table, const size_t s_table, const i16 precision) {
  const i16
    start = ~(1 << (3 + precision)) + 1, // -8 at any precision, we take the complement of it
    end   = 0;                           // 0

  std::vector<i16> hs;
  std::vector<f64> error;

  table.push_back({ start, 0, 0, 0 });
  table.push_back({ end,   0, 0, 0 });

  for (size_t i = 0; i < s_table - 2; i++) {
    for (size_t j = 0; j < table.size() - 1; j++) {
      hs.push_back(table[j + 1].x - table[j].x);
      error.push_back(add_func_err(QI_M_to_f64(table[j].x, precision), QI_M_to_f64(hs[j], precision)));
    }

    size_t max_index = 0;
    for (size_t j = 1; j < hs.size(); j++)
      if ((error[j] > error[max_index]) && (hs[j] > 1))
        max_index = j;

    table.insert(
      table.begin() + max_index + 1,
      { (i16)(table[max_index].x + (hs[max_index] >> 1)), 0, 0, 0 }
    );

    hs.clear();
    error.clear();
  }

  for (size_t i = 0; i < s_table; i++)
    table[i].y = f64_to_QI_M(add_func(QI_M_to_f64(table[i].x, precision)), precision);

  for (size_t i = 1; i < s_table; i++) {
    i32 temp = (i32)(table[i].y) - (i32)(table[i - 1].y);
    temp <<= precision;
    temp >>= log2_int(table[i].x - table[i - 1].x);
    table[i].m = (i16)(temp);
    table[i].b = table[i].y - (i16)((i32)(table[i].m) * (i32)(table[i].x) >> precision);
  }
}

void greedy_spline_sub_func(std::vector<SplinePoint>& table, const size_t s_table, const i16 precision) {
  const i16
    start = ~(1 << (3 + precision)), // -8 - 2^precision, we take the complement and subtract one (which is just taking the ~)
    end   = -1;                      // -2^precision

  std::vector<i16> hs;
  std::vector<f64> error;

  table.push_back({ start, 0, 0, 0 });
  table.push_back({ end,   0, 0, 0 });

  for (size_t i = 0; i < s_table - 2; i++) {
    for (size_t j = 0; j < table.size() - 1; j++) {
      hs.push_back(table[j + 1].x - table[j].x);
      error.push_back(sub_func_err(QI_M_to_f64(table[j].x, precision), QI_M_to_f64(hs[j], precision)));
    }

    size_t max_index = 0;
    for (size_t j = 1; j < hs.size(); j++)
      if ((error[j] > error[max_index]) && (hs[j] > 1))
        max_index = j;

    table.insert(
      table.begin() + max_index + 1,
      { (i16)(table[max_index].x + (hs[max_index] >> 1)), 0, 0, 0 }
    );

    hs.clear();
    error.clear();
  }

  for (size_t i = 0; i < s_table; i++)
    table[i].y = f64_to_QI_M(sub_func(QI_M_to_f64(table[i].x, precision)), precision);

  for (size_t i = 1; i < s_table; i++) {
    i32 temp = (i32)(table[i].y) - (i32)(table[i - 1].y);
    temp <<= precision;
    temp >>= log2_int(table[i].x - table[i - 1].x);
    table[i].m = (i16)(temp);
    table[i].b = table[i].y - (i16)((i32)(table[i].m) * (i32)(table[i].x) >> precision);
  }
}

void test_float2lns_table_sizes(const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision) {
  const i16 s_precision_values = 16;
  std::vector<i32> precision_values(s_precision_values, -1);

  const i16
    epsilon = (1 << (16 - precision)), // min value of the format
    start   = 0,
    end     = (1 << 15) - 1; // the closest to 1 we can get, 1 - 2^(-16)
  
  for (size_t s_table = 2; s_table < s_max + 1; s_table++) {
    std::vector<SplinePoint> table;
    greedy_spline_float2lns(table, s_table);

    i16 max_err = 0;
    for (i16 x = start; x <= end - epsilon; x += epsilon) {
      const i16 int_result =
        xf_mode ?
          calculate_xf(table, x, s_table) :
          calculate_xmb(table, x, s_table, 15);
      const f64 f64_result = float2lns(M_to_f64(x));

      const i16 err = std::abs(int_result - f64_to_M(f64_result));
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
            << " float2lns"
            << (xf_mode ? " xf" : " xmb")
            << ":"
            << std::endl;

  for (size_t i = 0; i < precision_values.size(); i++)
    if (precision_values[i] != -1)
      std::cout << "Error " << i << ": " << precision_values[i] << " rows" << std::endl;
}

void test_lns2float_table_sizes(const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision) {
  const i16 s_precision_values = 16;
  std::vector<i32> precision_values(s_precision_values, -1);

  const i16
    epsilon = (1 << (16 - precision)), // min value of the format
    start   = 0,
    end     = (1 << 15) - 1; // the closest to 1 we can get, 1 - 2^(-16)
  
  for (size_t s_table = 2; s_table < s_max + 1; s_table++) {
    std::vector<SplinePoint> table;
    greedy_spline_lns2float(table, s_table);

    i16 max_err = 0;
    for (i16 x = start; x <= end - epsilon; x += epsilon) {
      const i16 int_result =
        xf_mode ?
          calculate_xf(table, x, s_table) :
          calculate_xmb(table, x, s_table, 15);
      const f64 f64_result = lns2float(M_to_f64(x));

      i16 err = std::abs(int_result - f64_to_M(f64_result));
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
            << " lns2float"
            << (xf_mode ? " xf" : " xmb")
            << ":"
            << std::endl;

  for (size_t i = 0; i < precision_values.size(); i++)
    if (precision_values[i] != -1)
      std::cout << "Error " << i << ": " << precision_values[i] << " rows" << std::endl;
}

void test_add_table_sizes(const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision) {
  const i16 s_precision_values = precision + 1;
  std::vector<i32> precision_values(s_precision_values, -1);

  const i16
    start = ~(1 << (3 + precision)) + 1, // -8 at any precision, we take the complement of it
    end   = 0;                           // 0
  
  for (size_t s_table = 2; s_table < s_max + 1; s_table++) {
    std::vector<SplinePoint> table;
    greedy_spline_add_func(table, s_table, precision);

    i16 max_err = 0;
    for (i16 x = start; x < end + 1; x++) {
      const i16 int_result =
        xf_mode ?
          calculate_xf(table, x, s_table) :
          calculate_xmb(table, x, s_table, precision);
      const f64 f64_result = add_func(QI_M_to_f64(x, precision));

      const i16 err = std::abs(int_result - f64_to_QI_M(f64_result, precision));
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
            << " addition"
            << (xf_mode ? " xf" : " xmb")
            << ":"
            << std::endl;

  for (size_t i = 0; i < precision_values.size(); i++)
    if (precision_values[i] != -1)
      std::cout << "Error " << i << ": " << precision_values[i] << " rows" << std::endl;
}

void test_sub_table_sizes(const size_t s_max, const bool xf_mode, const bool lns16, const i16 precision) {
  const i16 s_precision_values = precision + 1;
  std::vector<i32> precision_values(s_precision_values, -1);

  const i16
    start = ~(1 << (3 + precision)), // -8 - 2^precision, we take the complement and subtract one (which is just taking the ~)
    end   = -1;                      // -2^precision

  for (size_t s_table = 2; s_table < s_max + 1; s_table++) {
    std::vector<SplinePoint> table;
    greedy_spline_sub_func(table, s_table, precision);

    i16 max_err = 0;
    for (i16 x = start; x < end + 1; x++) {
      const i16 int_result = 
        xf_mode ? 
          calculate_xf(table, x, s_table) :
          calculate_xmb(table, x, s_table, precision);
      const f64 f64_result = sub_func(QI_M_to_f64(x, precision));

      const i16 err = std::abs(int_result - f64_to_QI_M(f64_result, precision));
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
            << " subtraction"
            << (xf_mode ? " xf" : " xmb")
            << ":"
            << std::endl;

  for (size_t i = 0; i < precision_values.size(); i++)
    if (precision_values[i] != -1)
      std::cout << "Error " << i << ": " << precision_values[i] << " rows" << std::endl;
}

void write_tables_binary(
  const std::vector<SplinePoint>& spline_plus,
  const std::vector<SplinePoint>& spline_minus,
  const std::vector<SplinePoint>& spline_f2l,
  const std::vector<SplinePoint>& spline_l2f,
  const bool lns16, const bool xf,
  const i16 int_digits, const i16 precision
) {
  mkdir("lns_tables", 0755); // ignored if already exists

  std::ostringstream fname;
  fname << "lns_tables/lns" 
        << (lns16 ? "16" : "8")
        << "_q" << int_digits
        << "_" << precision
        << (xf ? "_xf" : "_xmb")
        << ".lns";

  std::ofstream file(fname.str(), std::ios::binary);
  if (!file) {
    std::cerr << "[ERROR]: could not open " << fname.str() << " for writing" << std::endl;
    exit(1);
  }

  const u32 
    s_plus  = (u32)spline_plus.size(),
    s_minus = (u32)spline_minus.size(),
    s_f2l   = (u32)spline_f2l.size(),
    s_l2f   = (u32)spline_l2f.size();

  file.write(reinterpret_cast<const char*>(&s_plus),  sizeof(u32));
  file.write(reinterpret_cast<const char*>(&s_minus), sizeof(u32));
  file.write(reinterpret_cast<const char*>(&s_f2l),   sizeof(u32));
  file.write(reinterpret_cast<const char*>(&s_l2f),   sizeof(u32));

  if (lns16) {
    if (xf) {
      for (const SplinePoint& p : spline_plus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i16));
      }
        
      for (const SplinePoint& p : spline_minus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i16));
      }

      for (const SplinePoint& p : spline_f2l) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i16));
      }

      for (const SplinePoint& p : spline_l2f) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i16));
      }

    } else {
      for (const SplinePoint& p : spline_plus)  {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i16));
      }

      for (const SplinePoint& p : spline_minus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i16));
      }

      for (const SplinePoint& p : spline_f2l) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i16));
      }

      for (const SplinePoint& p : spline_l2f) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i16));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i16));
      }
    }
  } else {
    if (xf) {
      for (const SplinePoint& p : spline_plus)  {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i8));
      }

      for (const SplinePoint& p : spline_minus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&p.y), sizeof(i8));
      }

      for (const SplinePoint& p : spline_f2l) {
        // Shift right by 8 to move the high byte into the low byte position, then cast
        const i8
          x_byte = static_cast<i8>(p.x >> 8),
          y_byte = static_cast<i8>(p.y >> 8);

        file.write(reinterpret_cast<const char*>(&x_byte), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&y_byte), sizeof(i8));
      }

      for (const SplinePoint& p : spline_l2f) {
        // Shift right by 8 to move the high byte into the low byte position, then cast
        const i8
          x_byte = static_cast<i8>(p.x >> 8),
          y_byte = static_cast<i8>(p.y >> 8);

        file.write(reinterpret_cast<const char*>(&x_byte), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&y_byte), sizeof(i8));
      }

    } else {
      for (const SplinePoint& p : spline_plus)  {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i8));
      }

      for (const SplinePoint& p : spline_minus) {
        file.write(reinterpret_cast<const char*>(&p.x), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&p.m), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&p.b), sizeof(i8));
      }

      for (const SplinePoint& p : spline_f2l) {
        // Shift right by 8 to move the high byte into the low byte position, then cast
        const i8
          x_byte = static_cast<i8>(p.x >> 8),
          m_byte = static_cast<i8>(p.m >> 8),
          b_byte = static_cast<i8>(p.b >> 8);

        file.write(reinterpret_cast<const char*>(&x_byte), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&m_byte), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&b_byte), sizeof(i8));
      }

      for (const SplinePoint& p : spline_l2f) {
        // Shift right by 8 to move the high byte into the low byte position, then cast
        const i8
          x_byte = static_cast<i8>(p.x >> 8),
          m_byte = static_cast<i8>(p.m >> 8),
          b_byte = static_cast<i8>(p.b >> 8);

        file.write(reinterpret_cast<const char*>(&x_byte), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&m_byte), sizeof(i8));
        file.write(reinterpret_cast<const char*>(&b_byte), sizeof(i8));
      }
    }
  }

  file.close();
  std::cout << "[INFO]: Written to " << fname.str() << std::endl;
}

i16 calculate_xmb(const std::vector<SplinePoint>& table, const i16 x, const i16 s_table, const i16 precision) {
  i16 i = 0;
  for (; x > table[i].x && i < s_table; i++);
  return (((i32)table[i].m * (i32)x) >> precision) + table[i].b;
}

i16 calculate_xf(const std::vector<SplinePoint>& table, const i16 x, const i16 s_table) {
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

f64 float2lns(const f64 m_x) {
  return std::log2(1.0 + m_x);
}

f64 lns2float(const f64 lns_f_x) {
  return std::pow(2.0, lns_f_x) - 1.0;
}

f64 add_func(const f64 diff) {
  return std::log2(1.0 + std::exp2(diff));
}

f64 sub_func(const f64 diff) {
  return std::log2(1.0 - std::exp2(diff));
}

f64 float2lns_err(const f64 a, const f64 h) {
  // the constant factors don't change the comparison, so we do not need to include them
  return std::pow(h, 2) / std::pow(1 + a, 2);
}

f64 lns2float_err(const f64 b, const f64 h) {
  // the constant factors don't change the comparison, so we do not need to include them
  return std::pow(2.0, b) * std::pow(h, 2);
}

f64 add_func_err(const f64 b, const f64 h) {
  // the constant factors don't change the comparison, so we do not need to include them
  const f64 
    numerator = std::pow(h, 2) * std::pow(2.0, b),
    denominator = std::pow(1.0 + std::pow(2.0, b), 2);
  return numerator / denominator;
}

f64 sub_func_err(const f64 b, const f64 h) {
  // the constant factors don't change the comparison, so we do not need to include them
  const f64 
    numerator = std::pow(h, 2) * std::pow(2.0, b),
    denominator = std::pow(1.0 - std::pow(2.0, b), 2);
  return numerator / denominator;
}

f64 M_to_f64(const i16 a) {
  return static_cast<f64>(a) / (f64)(1 << 15);
}

i16 f64_to_M(const f64 a) {
  return static_cast<i16>(a * (f64)(1 << 15));
}

f64 QI_M_to_f64(const i16 a, const i16 precision) {
  return static_cast<f64>(a) / (f64)(1 << precision);
}

i16 f64_to_QI_M(const f64 a, const i16 precision) {
  return static_cast<i16>(a * (f64)(1 << precision));
}

u8 log2_int(const i16 n) {
  assert(n >= 0);

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
  if (n <= 0x7FFF)
    return 0x0F;
  std::cerr << "[ERROR]: invalid domain for log2_int16" << std::endl;
  exit(1);
}
