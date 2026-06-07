#ifndef __BFLOAT_SIM_H_
#define __BFLOAT_SIM_H_

#include <cstdlib>
#include <cstring>

#include "utils.h"

struct bf8 {
  u8 bits; // E4M3: [S | EEEE | MMM]

  bf8()
    : bits(0) {}

  bf8(u8 raw, bool)
    : bits(raw) {}

  bf8(i32 x) {
    *this = bf8((f32)x);
  }

  bf8(f32 x) {
    if (x == 0.0f) {
      bits = 0;
      return;
    }

    u32 fb;
    memcpy(&fb, &x, sizeof(u32));
    u32 sign    = (fb >> 31) & 1u;
    i32 exp_f32 = (i32)((fb >> 23) & 0xFFu) - 127; // unbias from f32
    u32 frac3   = (fb >> 20) & 0x7u;               // top 3 mantissa bits

    i32 exp_bf8 = exp_f32 + 7; // rebias to 4-bit bias (7)

    if (exp_bf8 >= 15) {
      // Clamp to max normal: exp=1110, frac=111
      bits = (u8)((sign << 7) | 0x77u);
      return;
    }
    if (exp_bf8 <= 0) {
      // Flush to signed zero
      bits = (u8)(sign << 7);
      return;
    }

    bits = (u8)((sign << 7) | ((u32)exp_bf8 << 3) | frac3);
  }

  explicit operator f32() const {
    u32 sign = (bits >> 7) & 1u;
    u32 exp  = (bits >> 3) & 0xFu;
    u32 frac = bits & 0x7u;

    if (exp == 0)
      return sign ? -0.0f : 0.0f; // flushed zero

    i32 exp_f32 = (i32)exp - 7 + 127; // unbias from bf8, rebias to f32
    u32 fb = (sign << 31) | ((u32)exp_f32 << 23) | (frac << 20);

    f32 r;
    memcpy(&r, &fb, sizeof(f32));

    return r;
  }

  inline bool operator==(const bf8 other) const {
    return bits == other.bits;
  }
  inline bool operator<(const bf8 other) const {
    return (f32)*this < (f32)other;
  }
  inline bool operator>(const bf8 other) const {
    return (f32)*this > (f32)other;
  }
  inline bool operator<=(const bf8 other) const {
    return *this < other || bits == other.bits;
  }
  inline bool operator>=(const bf8 other) const {
    return *this > other || bits == other.bits;
  }

  bf8 operator+(const bf8 other) const {
    return bf8((f32)*this + (f32)other);
  }
  bf8 operator-(const bf8 other) const {
    return bf8((f32)*this - (f32)other);
  }
  bf8 operator-() const {
    return bf8(bits ^ (1u << 7), true); // flip sign bit (bit 7, not 15)
  }
  bf8 operator*(const bf8 other) const {
    return bf8((f32)*this * (f32)other);
  }
  bf8 operator/(const bf8 other) const {
    if ((other.bits & 0x7Fu) == 0) { // zero magnitude (ignore sign)
      fprintf(stderr, "[BF8 FATAL]: Division by Zero detected.\n");
      exit(139);
    }
    return bf8((f32)*this / (f32)other);
  }

  inline bf8& operator+=(const bf8 other) {
    *this = *this + other;
    return *this;
  }
  inline bf8& operator-=(const bf8 other) {
    *this = *this - other;
    return *this;
  }
  inline bf8& operator*=(const bf8 other) {
    *this = *this * other;
    return *this;
  }
  inline bf8& operator/=(const bf8 other) {
    *this = *this / other;
    return *this;
  }

  void print_hex() const {
    printf("0x%02X", bits);
  }

  void print_bin() const {
    printf("0b");
    for (i8 k = 7; k >= 0; k--) {
      printf("%d", (bits >> k) & 1);
      if (k == 7)
        printf("_");
      if (k == 3)
        printf("_");
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

struct bf16 {
  u16 bits; // E4M3: [S | EEEEEEEE | MMMMMMM]

  bf16()
    : bits(0) {}

  bf16(u16 raw, bool)
    : bits(raw) {}

  bf16(i32 x) {
    *this = bf16((f32)x);
  }

  bf16(f32 x) {
    u32 _bits;
    memcpy(&_bits, &x, sizeof(u32));
    _bits &= 0xFFFF0000u;
    bits = _bits >> 16;
  }

  explicit operator f32() const {
    u32 _bits = bits << 16;
    f32 x;
    memcpy(&x, &_bits, sizeof(f32));
    return x;
  }

  inline bool operator==(const bf16 other) const {
    return bits == other.bits;
  }

  inline bool operator<(const bf16 other) const {
    return (f32)*this < (f32)other;
  }

  inline bool operator>(const bf16 other) const {
    return (f32)*this > (f32)other;
  }

  inline bool operator<=(const bf16 other) const {
    return *this < other || bits == other.bits;
  }

  inline bool operator>=(const bf16 other) const {
    return *this > other || bits == other.bits;
  }

  bf16 operator+(const bf16 other) const {
    return bf16((f32)*this + (f32)other);
  }

  bf16 operator-(const bf16 other) const {
    return bf16((f32)*this - (f32)other);
  }

  bf16 operator-() const {
    return bf16(bits ^ (1u << 15), false);
  }

  bf16 operator*(const bf16 other) const {
    return bf16((f32)*this * (f32)other);
  }

  bf16 operator/(const bf16 other) const {
    if (other.bits == 0) {
      fprintf(stderr, "[LNS FATAL]: Division by Zero detected.\n");
      exit(139);
    }

    return bf16((f32)*this / (f32)other);
  }

  inline bf16& operator+=(const bf16 other) {
    *this = *this + other;
    return *this;
  }

  inline bf16& operator-=(const bf16 other) {
    *this = *this - other;
    return *this;
  }

  inline bf16& operator*=(const bf16 other) {
    *this = *this * other;
    return *this;
  }

  inline bf16& operator/=(const bf16 other) {
    *this = *this / other;
    return *this;
  } 

  void print_hex() const {
    printf("0x%0*X", 4, bits);
  }

  void print_bin() const {
    printf("0b");
    for (i8 k = 15; k >= 0; k--) {
      printf("%d", (bits >> k) & 1);
      if (k == 15)
        printf("_");
      if (k == 7)
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

#endif // !__BFLOAT_SIM_H_
