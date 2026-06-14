#ifndef __LNS_SIM_INL__
#define __LNS_SIM_INL__

#define LNS_SIGN(n, expr)               ((uint_t<n>(expr) << (n - 1)))
#define LNS_ZERO(n)                     (uint_t<n>(1) << (n - 2))
#define LNS_EXPONENT_BITMASK(n)         ((uint_t<n>(1) << (n - 1)) - 1)
#define LNS_EXP_INT_MASK(n, f)          ((uint_t<n>(1) << (n - 1 - f)) - 1)
#define LNS_EXP_FRAC_MASK(f)            ((uint_t<n>(1) << f) - 1)

#define F32_SIGN(raw)                   ((raw >> 31) & 1)
#define F32_EXP(raw)                    ((raw >> 23) & 0xFF)
#define F32_FRAC(raw)                   (raw & 0x7FFFFF)
#define F32_FRAC_U32_TO_FLOAT(frac)     ((f64)frac / (f64)(1 << 23))
#define F32_TO_LNS_FRAC(mantissa, n, f) ((uint_t<n>)(lns_f2l_compute(mantissa) >> (n - 1 - f)))

template<u8 n, u8 i, u8 f>
lns<n,i,f>::lns()
  : bits(0) {}

template<u8 n, u8 i, u8 f>
lns<n,i,f>::lns(uint_t<n> raw, bool)
  : bits(raw) {}

template<u8 n, u8 i, u8 f>
lns<n,i,f>::lns(i32 x) {
  *this = lns((f32)x);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>::lns(f32 x) {
  u32 raw;
  memcpy(&raw, &x, sizeof(u32));

  if (fabsf(x) < FLT_MIN) {
    const uint_t<n> sign = (raw >> 31) & 1;
    bits = (sign << (n - 1)) | LNS_ZERO(n);
    return;
  }

  const uint_t<n> lns_sign       = F32_SIGN(raw);
  const int_t<n>  lns_exp_int    = F32_EXP(raw) - 127;
  const u32       float_exp_frac = F32_FRAC(raw);
  /*
  printf(
    "x=%.5f or 0x%08X, sign=%d, (exp - 127)=(%d, 0x%08X), frac=0x%08X\n",
    x,(u32)x, lns_sign, lns_exp_int, lns_exp_int, float_exp_frac
  );
  */

  int_t<n> float_mantissa = 0;
  if constexpr (n == 64) {
    float_mantissa = (u64)float_exp_frac << 40;
  }
  else if constexpr (n == 32) {
    float_mantissa = float_exp_frac << 8;
  }
  else {
    float_mantissa = float_exp_frac >> (23 - (n - 1));
  }

  const uint_t<n> lns_exp_frac = F32_TO_LNS_FRAC(float_mantissa, n, f);
  /*
  printf(
    "x=%.5f or 0x%08X, sign=%d, (exp - 127)=(%d, 0x%08X), frac=0x%08X\n",
    x, (u32)x, lns_sign, lns_exp_int, lns_exp_int, lns_exp_frac
  );
  */

  bits =
    (lns_sign << (n - 1)) |
    ((lns_exp_int & LNS_EXP_INT_MASK(n, f)) << f) |
    (lns_exp_frac & LNS_EXP_FRAC_MASK(f));

  // printf("bits=0x%08X\n", bits);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>::operator f32() const {
  if (is_zero())
    return 0.0f;
  if (bits == 0)
    return 1.0f;

  const uint_t<n> f32_exp  = (exponent() >> f) + 127;
  const uint_t<n> exp_frac = exponent() & ((1 << f) - 1);
  /*
  printf(
    "f32_exp=(%d, 0x%08X), exp_frac=0x%08X\n",
    f32_exp, f32_exp, exp_frac
  );
  */
  const int_t<n>  mantissa = lns_l2f_compute(exp_frac << (n - 1 - f));

  u32 f32_frac = 0;
  if constexpr (n - 1 >= 23) {
    f32_frac = (u32)((u64)mantissa >> (n - 1 - 23));
  }
  else {
    f32_frac = (u32)((u64)mantissa << (23 - (n - 1)));
  }

  const u32 f32_bits =
    (sign() << 31) |
    ((f32_exp & 0xFF) << 23) |
    (f32_frac & 0x7FFFFF);

  f32 value;
  memcpy(&value, &f32_bits, sizeof(f32));
  return value;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>::operator f64() const {
  return (f64)(f32)*this;
}

template<u8 n, u8 i, u8 f>
template<u8 n2, u8 i2, u8 f2>
lns<n, i, f>::lns(const lns<n2, i2, f2>& other) {
  if (other.is_zero()) {
    bits = LNS_ZERO(n);
    return;
  }

  const int_t<n2> raw_exp = other.bits & LNS_EXPONENT_BITMASK(n2);

  int_t<n> dst_exp;
  if constexpr (f >= f2) {
    dst_exp = (int_t<n>)raw_exp << (f - f2);

    const bool sign_extend = (raw_exp >> (n2 - 2)) & 1;
    if (sign_extend) {
      constexpr int_t<n> sign_extention = (uint_t<n>)(~0) << (f - f2 + n2 - 1);
      dst_exp |= sign_extention & LNS_EXPONENT_BITMASK(n);
    }
  }
  else
    dst_exp = (int_t<n>)(raw_exp >> (f2 - f)) & LNS_EXPONENT_BITMASK(n);

  if (dst_exp == LNS_ZERO(n))
    dst_exp++; // avoid zero sentinel

  bits = (other.sign() << (n - 1)) ^ dst_exp;

  /*
  if constexpr (n == 8 && n2 == 32) {
    other.debug_print("other");
    printf(
      "sign_extend=%d, raw_exp=0x%02X, dst_exp=(raw_exp << %d)=0x%08X\n",
      sign_extend, raw_exp, f - f2, dst_exp
    );
    this->debug_print("new");
  }
  */
}

template<u8 n, u8 i, u8 f>
u8 lns<n,i,f>::sign() const {
  return (u8)(bits >> (n - 1));
}

template<u8 n, u8 i, u8 f>
int_t<n> lns<n,i,f>::exponent() const {
  return ((bits & LNS_ZERO(n)) << 1) | (bits & LNS_EXPONENT_BITMASK(n));
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::is_zero() const {
  return (bits & LNS_EXPONENT_BITMASK(n)) == LNS_ZERO(n);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator+(const lns other) const {
  uint_t<n>
    sign1 = sign(),
    sign2 = other.sign();

  int_t<n>
    exp1 = exponent(),
    exp2 = other.exponent(),
    diff = exp2 - exp1;

  int_t<n> result = 0;

  if (diff > 0) {
    exp1  ^= exp2;  exp2  ^= exp1;  exp1  ^= exp2;
    sign1 ^= sign2; sign2 ^= sign1; sign1 ^= sign2;
    diff  *= -1;
  }

  const bool use_add = !(sign1 ^ sign2);
  if (!use_add && diff >= 0)
    return lns(LNS_ZERO(n), false);

  result  = exp1 + lns_add_and_sub_compute(use_add, diff);
  result &= (uint_t<n>)(1u << (n - 1)) - 1;
  result |= sign1 << (n - 1);

  return lns((uint_t<n>)result, false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator-() const {
  return lns(bits ^ LNS_SIGN(n, 1), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator-(const lns other) const {
  return *this + (-other);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator*(const lns other) const {
  if (is_zero() || other.is_zero())
    return lns(LNS_ZERO(n), false);

  const uint_t<n> _sign        = LNS_SIGN(n, sign() ^ other.sign());
  const int_t<n>  combined_exp = exponent() + other.exponent();

  uint_t<n> _exp_bits = (uint_t<n>)combined_exp & LNS_EXPONENT_BITMASK(n);
  if (_exp_bits == LNS_ZERO(n))
    _exp_bits++;

  return lns(_sign | _exp_bits, false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator/(const lns other) const {
  if (other.is_zero()) {
    fprintf(stderr, "[LNS FATAL]: Division by Zero detected.\n");
    exit(139);
  }
  if (this->is_zero())
    return *this;

  const uint_t<n> 
    _sign = LNS_SIGN(n, sign() ^ other.sign()),
    _exp  = (exponent() - other.exponent()) & LNS_EXPONENT_BITMASK(n);

  return lns(_sign | _exp, false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::power2_pow(const u8 k) const {
  return lns((exponent() << k) & LNS_EXPONENT_BITMASK(n), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::power2_root(const u8 k) const {
  assert(!sign());
  return lns((exponent() >> k) & LNS_EXPONENT_BITMASK(n), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::square() const {
  return lns((exponent() << 1) & LNS_EXPONENT_BITMASK(n), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::sqrt() const {
  assert(!sign());
  return lns((exponent() >> 1) & LNS_EXPONENT_BITMASK(n), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::exp() const {
  return lns(std::exp((f32)*this));
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::sinh() const {
  const lns<n,i,f> neg_x = -*this;
  return (exp() - neg_x.exp()) / lns(2.0f);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::cosh() const {
  const lns<n,i,f> neg_x = -*this;
  return (exp() + neg_x.exp()) / lns(2.0f);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::tanh() const {
  const lns<n,i,f> 
    two = lns(2.0f),
    one = lns(1.0f), 
    e2x = (two * (*this)).exp();
  return (e2x - one) / (e2x + one);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator+=(const lns other) {
  *this = *this + other;
  return *this;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator-=(const lns other) {
  *this = *this - other;
  return *this;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator*=(const lns other) {
  *this = *this * other;
  return *this;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator/=(const lns other) {
  *this = *this / other;
  return *this;
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator==(const lns other) const {
  return bits == other.bits;
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator<(const lns other) const {
  const u8 
    sign1 = sign(),
    sign2 = other.sign();
  const int_t<n>
    exp1 = exponent(),
    exp2 = other.exponent();
  return (
    (sign1 && (!sign2 || exp1 > exp2)) ||
    (!sign1 && !sign2 && exp1 < exp2)
  );
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator>(const lns other) const {
  const u8 
    sign1 = sign(),
    sign2 = other.sign();
  const int_t<n> 
    exp1 = exponent(),
    exp2 = other.exponent();
  return (
    sign1 && sign2 && exp1 < exp2) ||
    (!sign1 && (sign2 || exp1 > exp2)
  );
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator<=(const lns other) const {
  return *this < other || bits == other.bits;
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator>=(const lns other) const {
  return *this > other || bits == other.bits;
}

template<u8 n, u8 i, u8 f>
void lns<n,i,f>::print_hex() const {
  printf("0x%0*X", n / 4, bits);
}

template<u8 n, u8 i, u8 f>
void lns<n,i,f>::print_bin() const {
  printf("0b");
  for (i16 k = n - 1; k >= 0; k--) {
    printf("%d", (bits >> k) & 1);
    if (k == n - 1)
      printf("_");
    if (k == f)
      printf(".");
  }
}

template<u8 n, u8 i, u8 f>
void lns<n,i,f>::debug_print(const char* label) const {
  if (label[0] != '\0')
    printf("%s: ", label);
  printf("%f (raw: ", (f32)*this);
  print_hex();
  printf(") ");
  print_bin();
  printf("\n");
}

template<u8 n, u8 i, u8 f>
int_t<n> lns<n,i,f>::lns_f2l_compute(const int_t<n> mantissa) const {
  if      constexpr (n == 8)  {
    if (lns8_lut_f2l == nullptr) {
      fprintf(
        stderr,
        "[LNSSIM ERROR]: look up tables for float to lns operation is a nullptr;\n"
        "\tmaybe you forgot to call lns8_read_tables(...)\n"
      );
      exit(1);
      return (int_t<n>)1;
    }

    return lns8_lut_compute  (*lns8_lut_f2l,  mantissa, n - 1);
  }
  else if constexpr (n == 16) {
    if (lns16_lut_f2l == nullptr) {
      fprintf(
        stderr,
        "[LNSSIM ERROR]: look up tables for float to lns operation is a nullptr;\n"
        "\tmaybe you forgot to call lns16_read_tables(...)\n"
      );
      exit(1);
      return (int_t<n>)1;
    }

    return lns16_lut_compute (*lns16_lut_f2l, mantissa, n - 1);
  }
  else if constexpr (n == 32 || n == 64) {
    const f64 
      m          = (f64)mantissa / (f64)(1ull << (n - 1)),
      correction = log2(1.0 + m);

    /*
    printf(
      "m=%.5f or 0x%08X, correction=%.5f or 0x%08X\n",
      m, mantissa, correction, (int_t<n>)(correction * (f64)(1ull << (n - 1))) 
    );
    */

    return (int_t<n>)(correction * (f64)(1ull << (n - 1)));
  }
  else {
    fprintf(stderr, "[ERROR]: LNS bit format not implemented");
    exit(0);
    return 0;
  }
}

template<u8 n, u8 i, u8 f>
int_t<n> lns<n,i,f>::lns_l2f_compute(const int_t<n> lns_f) const {
  if      constexpr (n == 8) {
    if (lns8_lut_l2f == nullptr) {
      fprintf(
        stderr,
        "[LNSSIM ERROR]: look up tables for lns to float operation is a nullptr;\n"
        "\tmaybe you forgot to call lns8_read_tables(...)\n"
      );
      exit(1);
      return (int_t<n>)1;
    }

    return lns8_lut_compute  (*lns8_lut_l2f,  lns_f, n - 1);
  }
  else if constexpr (n == 16) {
    if (lns16_lut_l2f == nullptr) {
      fprintf(
        stderr,
        "[LNSSIM ERROR]: look up tables for convertion operation is a nullptr;\n"
        "\tmaybe you forgot to call lns16_read_tables(...)\n"
      );
      exit(1);
      return (int_t<n>)1;
    }

    return lns16_lut_compute (*lns16_lut_l2f, lns_f, n - 1);
  }
  else if constexpr (n == 32 || n == 64) {
    const f64 
      frac     = (f64)lns_f / (f64)(1ull << (n - 1)),
      mantissa = pow(2, frac) - 1;

    /*
    printf(
      "frac=%.5f or 0x%08X, mantissa=%.5f or 0x%08X\n",
      frac, lns_f, mantissa, (int_t<n>)(mantissa * (f64)(1ull << (n - 1))) 
    );
    */ 

    return (int_t<n>)(mantissa * (f64)(1ull << (n - 1)));
  }
  else {
    fprintf(stderr, "[ERROR]: LNS bit format not implemented");
    exit(0);
    return 0;
  }
}

template<u8 n, u8 i, u8 f>
int_t<n> lns<n,i,f>::lns_add_and_sub_compute(const bool use_add, const int_t<n> diff) const {
  if      constexpr (n == 8)  {
    if (lns8_lut_add == nullptr || lns8_lut_sub == nullptr) {
      fprintf(
        stderr,
        "[LNSSIM ERROR]: look up tables for addition and subtraction operations are nullptr;\n"
        "\tmaybe you forgot to call lns8_read_tables(...)\n"
      );
      exit(1);
      return (int_t<n>)1;
    }

    return lns8_lut_compute  (use_add ? *lns8_lut_add  : *lns8_lut_sub,  diff, f);
  }
  else if constexpr (n == 16) {
    if (lns16_lut_add == nullptr || lns16_lut_sub == nullptr) {
      fprintf(
        stderr,
        "[LNSSIM ERROR]: look up tables for addition and subtraction operations are nullptr;\n"
        "\tmaybe you forgot to call lns16_read_tables(...)\n"
      );
      exit(1);
      return (int_t<n>)1;
    }

    return lns16_lut_compute (use_add ? *lns16_lut_add : *lns16_lut_sub, diff, f);
  }
  else if constexpr (n == 32 || n == 64) {
    const f64 z = (f64)diff / (f64)(1ull << f);

    f64 correction;
    // for very negative z (z < -50), 2^z underflows to 0, correction → 0
    if (use_add) {
      correction = (z < -50.0) ? 0.0 : log2(1.0 + pow(2.0, z));
    } else {
      correction = (z < -50.0) ? 0.0 : log2(1.0 - pow(2.0, z));
    }

    return (int_t<n>)(correction * (f64)(1ull << f));
  }
  else {
    fprintf(stderr, "[ERROR]: LNS bit format not implemented");
    exit(0);
    return 0;
  }
}

#endif // !__LNS_SIM_INL__
