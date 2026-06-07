#ifndef __LNS_SIM_INL__
#define __LNS_SIM_INL__

#define LNS_ZERO(n)                         (1 << (n - 2))
#define F32_SIGN(raw)                       ((raw >> 31) & 1)
#define F32_EXP(raw)                        ((raw >> 23) & 0xFF)
#define F32_FRAC(raw)                       (raw & 0x7FFFFF)
#define F32_FRAC_U32_TO_FLOAT(frac)         ((f64)frac / (f64)(1 << 23))
#define F32_TO_LNS_FRAC(mantissa, n, prec)  ((uint_t<n>)(lns_f2l_compute(mantissa) >> (n - 1 - prec)))

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
    const     uint_t<n> sign                      = (raw >> 31) & 1;
    constexpr uint_t<n> lns_exp_negative_infinity = 1 << (n - 2);
    bits = (sign << (n - 1)) | lns_exp_negative_infinity;
    return;
  }

  const uint_t<n> lns_sign       = F32_SIGN(raw);
  const int_t<n>  lns_exp_int    = F32_EXP(raw) - 127;
  const u32       float_exp_frac = F32_FRAC(raw);

  int_t<n> float_mantissa = 0;
  if constexpr (n == 64) {
    float_mantissa = float_exp_frac << 30;
  } else {
    float_mantissa = float_exp_frac >> (23 - (n - 1));
  }

  const uint_t<n> lns_exp_frac = F32_TO_LNS_FRAC(float_mantissa, n, f);

  constexpr uint_t<n>
    lns_exp_int_mask  = ((1 << (n - f - 1)) - 1),
    lns_exp_frac_mask = (1 << f) - 1;

  bits =
    (lns_sign << (n - 1)) |
    ((lns_exp_int & lns_exp_int_mask) << f) |
    (lns_exp_frac & lns_exp_frac_mask);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>::operator f32() const {
  if (is_zero())
    return 0.0f;
  if (bits == 0)
    return 1.0f;

  const uint_t<n> f32_exp  = (exponent() >> f) + 127;
  const uint_t<n> exp_frac = exponent() & ((1 << f) - 1);
  const int_t<n>  mantissa = lns_l2f_compute(exp_frac << (n - 1 - f));

  u32 f32_frac = 0;
  if constexpr (n == 64) {
    f32_frac = (u32)mantissa >> 30;
  } else {
    f32_frac = (u32)mantissa << (23 - (n - 1));
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
u8 lns<n,i,f>::sign() const {
  return (u8)(bits >> (n - 1));
}

template<u8 n, u8 i, u8 f>
int_t<n> lns<n,i,f>::exponent() const {
  return ((bits & (1 << (n - 2))) << 1) | (bits & ((1 << (n - 1)) - 1));
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::is_zero() const {
  return (bits & ((1 << (n - 1)) - 1)) == LNS_ZERO(n);
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
    exp1  ^= exp2; exp2  ^= exp1; exp1  ^= exp2;
    sign1 ^= sign2; sign2 ^= sign1; sign1 ^= sign2;
    diff  *= -1;
  }

  const bool use_add = !(sign1 ^ sign2);
  if (!use_add && diff >= 0)
    return lns(LNS_ZERO(n), false);

  result  = exp1 + lns_add_and_sub_compute(use_add, diff);
  result &= (1 << (n - 1)) - 1;
  result |= sign1 << (n - 1);

  return lns((uint_t<n>)result, false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator-() const {
  return lns(bits ^ (uint_t<n>(1) << (n - 1)), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator-(const lns other) const {
  return *this + (-other);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::operator*(const lns other) const {
  if (is_zero() || other.is_zero())
    return lns(LNS_ZERO(n), false);

  const uint_t<n> _sign     = (uint_t<n>)(sign() ^ other.sign()) << (n - 1);
  const int_t<n>  combined_exp = exponent() + other.exponent();

  uint_t<n> _exp_bits = (uint_t<n>)combined_exp & ((1 << (n - 1)) - 1);
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

  const uint_t<n> _sign = (uint_t<n>)(sign() ^ other.sign()) << (n - 1);
  const uint_t<n> _exp  = (uint_t<n>)(exponent() - other.exponent()) & ((1 << (n - 1)) - 1);

  return lns(_sign | _exp, false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::root(const u8 k) const {
  assert(!sign());
  return lns((exponent() >> k) & ((1 << (n - 1)) - 1), false);
}

template<u8 n, u8 i, u8 f>
lns<n,i,f> lns<n,i,f>::sqrt() const {
  assert(!sign());
  return lns((exponent() >> 1) & ((1 << (n - 1)) - 1), false);
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
  *this = *this + other; return *this;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator-=(const lns other) {
  *this = *this - other; return *this;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator*=(const lns other) {
  *this = *this * other; return *this;
}

template<u8 n, u8 i, u8 f>
lns<n,i,f>& lns<n,i,f>::operator/=(const lns other) {
  *this = *this / other; return *this;
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator==(const lns other) const {
  return bits == other.bits;
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator<(const lns other) const {
  const u8 sign1 = sign(), sign2 = other.sign();
  const int_t<n> exp1 = exponent(), exp2 = other.exponent();
  return (sign1 && (!sign2 || exp1 > exp2)) ||
       (!sign1 && !sign2 && exp1 < exp2);
}

template<u8 n, u8 i, u8 f>
bool lns<n,i,f>::operator>(const lns other) const {
  const u8 sign1 = sign(), sign2 = other.sign();
  const int_t<n> exp1 = exponent(), exp2 = other.exponent();
  return (sign1 && sign2 && exp1 < exp2) ||
       (!sign1 && (sign2 || exp1 > exp2));
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
    if (k == n - 1) printf("_");
    if (k == f)   printf(".");
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
  else { 
    fprintf(stderr, "[ERROR]: 32 and 64 bit LNS not implemented");
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
  else {
    fprintf(stderr, "[ERROR]: 32 and 64 bit LNS not implemented");
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
  else {
    fprintf(stderr, "[ERROR]: 32 and 64 bit LNS not implemented");
    exit(0);
    return 0;
  }
}

#endif // !__LNS_SIM_INL__
