#ifndef __LNS_SIM_H__
#define __LNS_SIM_H__

#include <lns_luts.hpp>
#include <cmath>
#include <cfloat>

#define F32_SIGN(raw) ((raw >> 31) & 1)
#define F32_EXP(raw)  ((raw >> 23) & 0xFF)
#define F32_FRAC(raw) (raw & 0x7FFFFF)

#define F32_FRAC_U32_TO_FLOAT(frac) \
  (f64)frac / (f64)(1 << 23)
#define F32_TO_LNS_FRAC(mantissa, precision) \
  (uint_t<n>)(log2(1.0 + mantissa) * (f64)(1 << precision))

template<u8 n, u8 i, u8 f>
struct lns {
  uint_t<n> bits;

  lns()
    : bits(0) {}

  lns(uint_t<n> raw, bool)
    : bits(raw) {}

  lns(f32 x) {
    u32 raw;
    memcpy(&raw, &x, sizeof(u32));

    if (fabsf(x) < FLT_MIN) {
      const uint_t<n> sign = (raw >> 31) & 1;
      constexpr uint_t<n> lns_exp_negative_infinity = 1 << (n - 2);
      bits = (sign << (n - 1)) | lns_exp_negative_infinity;
      return;
    }

    const uint_t<n> lns_sign       = F32_SIGN(raw);
    const int_t<n>  lns_exp_int    = F32_EXP(raw) - 127;
    const u32       float_exp_frac = F32_FRAC(raw);

    // Calculating lns_frac := log2(1 + f32_mantissa)
    const f64       float_mantissa = F32_FRAC_U32_TO_FLOAT(float_exp_frac);
    const uint_t<n> lns_exp_frac   = F32_TO_LNS_FRAC(float_mantissa, f);

    /*
     * LNS<N> QI-F
     * Bit  N - 1:       Sign
     * Bits [N - 2 : F]: Exponent Interger Part
     * Bits [F - 1 : 0]: Exponent Fractional Part
     * */

    constexpr uint_t<n> 
      lns_exp_int_mask  = ((1 << (n - f - 1)) - 1),
      lns_exp_frac_mask = (1 << f) - 1;
    
    bits =
      (lns_sign << (n - 1)) |
      ((lns_exp_int & lns_exp_int_mask) << f) |
      (lns_exp_frac & lns_exp_frac_mask)
    ;
  }

  f32 convert() const {
    if (bits == 0)
      return 1.0f;

    const uint_t<n> f32_exp  = (exp() >> f) + 127;
    const uint_t<n> exp_frac = exp() & ((1 << f) - 1);

    const f64 mantissa = pow(2.0, (f64)exp_frac / (f64)(1 << f)) - 1.0;
    const u32 f32_frac = (u32)(mantissa * (f64)(1 << 23));

    const u32 f32_bits =
      (sign() << 31) |
      ((f32_exp & 0xFF) << 23) |
      (f32_frac & 0x7FFFFF);

    f32 value;
    memcpy(&value, &f32_bits, sizeof(f32));

    return value;
  }

  inline u8 sign() const {
    return (u8)(bits >> (n - 1));
  }

  inline int_t<n> exp() const {
    return ((bits & (1 << (n - 2))) << 1) | (bits & ((1 << (n - 1)) - 1));
  }

  inline bool operator==(const lns other) const {
    return bits == other.bits;
  }

  inline bool operator<(const lns other) const {
    const u8 
      sign1 = sign(),
      sign2 = other.sign();

    const int_t<n> 
      exp1 = exp(),
      exp2 = other.exp();

    return
      (sign1 && (!sign2 || exp1 > exp2)) ||
      (!sign1 && !sign2 && exp1 < exp2);
  }

  inline bool operator>(const lns other) const {
    const u8
      sign1 = sign(),
      sign2 = other.sign();

    const int_t<n>
      exp1 = exp(),
      exp2 = other.exp();

    return
      (sign1 && sign2 && exp1 < exp2) ||
      (!sign1 && (sign2 || exp1 > exp2));
  }

  inline bool operator<=(const lns other) const {
    return *this < other || bits == other.bits;
  }

  inline bool operator>=(const lns other) const {
    return *this > other || bits == other.bits;
  }

  lns operator+(const lns other) const {
    uint_t<n> 
      sign1 = sign(),
      sign2 = other.sign();

    int_t<n>
      exp1 = exp(),
      exp2 = other.exp(),
      diff = exp2 - exp1;

    int_t<n> result = 0;

    if (diff > 0) {
      exp1  ^= exp2;
      exp2  ^= exp1;
      exp1  ^= exp2;

      sign1 ^= sign2;
      sign2 ^= sign1;
      sign1 ^= sign2;

      diff  *= -1;
    }

    // because I inline the add and subtraction tables,
    // and the order of choosing is the opposite of subtraction
    // according to the sign of the second parameter, here I
    // negate the sign to correctly encode that behaviour
    result  = exp1 + lns_add_and_sub_compute(!sign2, diff);
    result &= (1 << (n - 1)) - 1; // exponent bit mask
    result |= sign1 << (n - 1);

    return lns((uint_t<n>)result, false);
  }

  inline lns operator-(const lns other) const {
    uint_t<n> 
      sign1 = sign(),
      sign2 = other.sign();

    int_t<n>
      exp1 = exp(),
      exp2 = other.exp(),
      diff = exp2 - exp1;

    int_t<n> result = 0;

    if (diff > 0) {
      exp1  ^= exp2;
      exp2  ^= exp1;
      exp1  ^= exp2;

      sign1 ^= sign2;
      sign2 ^= sign1;
      sign1 ^= sign2;

      diff  *= -1;
    }

    result  = exp1 + lns_add_and_sub_compute(sign2, diff);
    result &= (1 << (n - 1)) - 1; // exponent bit mask
    result |= sign1 << (n - 1);

    return lns((uint_t<n>)result, false);
  }

  inline int_t<n> lns_add_and_sub_compute(const bool use_add, const int_t<n> diff) const {
    if constexpr      (n == 8)  
      return lns8_lut_compute (use_add ? *lns8_lut_add  : *lns8_lut_sub,  diff, f);
    else if constexpr (n == 16) 
      return lns16_lut_compute(use_add ? *lns16_lut_add : *lns16_lut_sub, diff, f);
    else if constexpr (n == 32) {
      fprintf(stderr, "[ERROR]: 32 and 64 bit LNS not implemented");
      exit(0);
      return 0;
    }
    else if constexpr (n == 64) {
      fprintf(stderr, "[ERROR]: 32 and 64 bit LNS not implemented");
      exit(0);
      return 0;
    }
  }

  inline lns operator*(const lns other) const {
    const uint_t<n>
      _sign = (uint_t<n>)(sign() ^ other.sign()) << (n - 1),
      _exp  = (exp() + other.exp()) & ((1 << (n - 1)) - 1);
    return lns(_sign | _exp, false);
  }

  inline lns operator/(const lns other) const {
    const uint_t<n>
      _sign = (uint_t<n>)(sign() ^ other.sign()) << (n - 1),
      _exp  = (exp() - other.exp()) & ((1 << (n - 1)) - 1);
    return lns(_sign | _exp, false);
  }

  inline lns sqrt() const {
    assert(!sign());
    return lns((exp() >> 1) & ((1 << (n - 1)) - 1), false);
  }
};

#endif // !__LNS_SIM_H__
