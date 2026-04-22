#ifndef __LNS_SIM_H__
#define __LNS_SIM_H__

#include <lnsluts.hpp>
#include <cmath>
#include <cfloat>

#define LNS_ZERO(n)   (1 << (n - 2))

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

  lns(i32 x) {
    *this = lns((f32)x);
  }

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

  explicit operator f32() const {
    if (is_zero())
      return 0.0f;
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

    const bool use_add = !(sign1 ^ sign2);
    if (!use_add && diff >= 0)
      return lns(LNS_ZERO(n), false);

    // LNS Addition Logic:
    // 1. If signs match (sign1 ^ sign2 == 0), we perform magnitude addition using the 'Add' table.
    // 2. If signs differ (sign1 ^ sign2 == 1), we perform magnitude subtraction using the 'Sub' table.
    // The result retains the sign of the larger operand (sign1) to handle negative results correctly.
    result  = exp1 + lns_add_and_sub_compute(use_add, diff);
    result &= (1 << (n - 1)) - 1; // exponent bit mask
    result |= sign1 << (n - 1);

    return lns((uint_t<n>)result, false);
  }

  inline lns operator-() const {
    return lns(bits ^ (uint_t<n>(1) << (n - 1)), false);
  }

  inline lns operator-(const lns other) const {
    // a - b is mathematically equivalent to a + (-b)
    // This delegates all complex spline math and sign handling to operator+
    return *this + (-other);
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

  inline bool is_zero() const {
    return (bits & ((1 << (n - 1)) - 1)) == LNS_ZERO(n);
  }

  inline lns operator*(const lns other) const {
    if (is_zero() || other.is_zero())
      return lns(LNS_ZERO(n), false);

    const uint_t<n> _sign = (uint_t<n>)(sign() ^ other.sign()) << (n - 1);

    const int_t<n> exp1         = exp();
    const int_t<n> exp2         = other.exp();
    const int_t<n> combined_exp = exp1 + exp2;

    const bool exp1_neg   = (exp1         >> (n - 2)) & 1;
    const bool exp2_neg   = (exp2         >> (n - 2)) & 1;
    const bool result_neg = (combined_exp >> (n - 2)) & 1;
    if ((exp1_neg == exp2_neg) && (result_neg != exp1_neg))
      fprintf(stderr, "[LNS WARN]: Overflow detected in multiplication.\n");

    // Prevent arithmetic from accidentally creating a Zero sentinel
    // If we hit 0x4000, we slightly shift it to keep it "Not Zero"
    uint_t<n> _exp_bits = (uint_t<n>)combined_exp & ((1 << (n - 1)) - 1);
    if (_exp_bits == LNS_ZERO(n))
      _exp_bits++;

    return lns(_sign | _exp_bits, false);
  }

  inline lns operator/(const lns other) const {
    if (other.is_zero()) {
      fprintf(stderr, "[LNS FATAL]: Division by Zero detected.\n");
      exit(139);
    }

    if (this->is_zero())
      return *this;

    const int_t<n> exp1       = exp();
    const int_t<n> exp2       = other.exp();
    const int_t<n> result_exp = exp1 - exp2;

    const bool exp1_neg   = (exp1       >> (n - 2)) & 1;
    const bool exp2_neg   = (exp2       >> (n - 2)) & 1;
    const bool result_neg = (result_exp >> (n - 2)) & 1;
    if ((exp1_neg != exp2_neg) && (result_neg != exp1_neg))
      fprintf(stderr, "[LNS WARN]: Overflow detected in division.\n");

    const uint_t<n> _sign = (uint_t<n>)(sign() ^ other.sign()) << (n - 1);
    const uint_t<n> _exp  = (uint_t<n>)result_exp & ((1 << (n - 1)) - 1);

    return lns(_sign | _exp, false);
  }

  inline lns sqrt() const {
    assert(!sign());
    return lns((exp() >> 1) & ((1 << (n - 1)) - 1), false);
  }

  inline lns& operator+=(const lns other) {
    *this = *this + other;
    return *this;
  }

  inline lns& operator-=(const lns other) {
    *this = *this - other;
    return *this;
  }

  inline lns& operator*=(const lns other) {
    *this = *this * other;
    return *this;
  }

  inline lns& operator/=(const lns other) {
    *this = *this / other;
    return *this;
  } 

  void print_hex() const {
    printf("0x%0*X", n / 4, bits);
  }

  void print_bin() const {
    printf("0b");
    for (i16 k = n - 1; k >= 0; k--) {
      printf("%d", (bits >> k) & 1);
      if (k == n - 1)
        printf("_");
      if (k == f)
        printf(".");
    }
  }

  void debug_print(const char* label = "") const {
    if (label[0] != '\0')
      printf("%s: ", label);
    printf("%f (raw: ", (f32)*this);
    print_hex();
    printf(") ");
    print_bin();
    printf("\n");
  }
};

#endif // !__LNS_SIM_H__
