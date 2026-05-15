#ifndef __LNS_H__
#define __LNS_H__

#include "utils.h"

/* 
 * NOTE: Inline assembly with "f" constraints is used here because these custom 
 * LNS instructions are not supported by the standard GCC compiler back-end. 
 * By using "f" constraints and f32 types, we force the compiler to use the 
 * floating-point register file, which the custom hardware hijacks for LNS logic.
 */

// Change [rd] "=r" to [rd] "=f" 
// Change [rs1] "r" to [rs1] "f"
#define CUSTOM_R(rd, rs1, rs2, opcode, funct3, funct7) \
  asm volatile ( \
    ".insn r %3, %4, %5, %0, %1, %2" \
    : "=f" (rd) \
    : "f" (rs1), \
      "f" (rs2), \
      "i" (opcode), \
      "i" (funct3), \
      "i" (funct7) \
  )

/* 
 * Dedicated Comparison Macro
 * Inputs: Floating point (f) registers
 * Output: Integer (r) register (required for C++ boolean logic/branching)
 */
#define LNS_COMPARE(rd, rs1, rs2, opcode, funct3, funct7) \
  asm volatile ( \
    ".insn r %3, %4, %5, %0, %1, %2" \
    : "=r" (rd) \
    : "f" (rs1), \
      "f" (rs2), \
      "i" (opcode), \
      "i" (funct3), \
      "i" (funct7) \
  )

// Update Load/Store to use "f" for the data register
#define CUSTOM_I(rd, rs1, imm, opcode, funct3) \
  asm volatile ( \
    ".insn i %3, %4, %0, %2(%1)" \
    : "=f" (rd) \
    : "r" (rs1), \
      "i" (imm), \
      "i" (opcode), \
      "i" (funct3) \
  )

#define CUSTOM_S(rs2, rs1, imm, opcode, funct3) \
  asm volatile ( \
    ".insn s %3, %4, %0, %2(%1)" \
    : \
    : "f" (rs2), \
      "r" (rs1), \
      "i" (imm), \
      "i" (opcode), \
      "i" (funct3) \
  )
#define CUSTOM0 0x0b
#define CUSTOM1 0x2b
#define CUSTOM2 0x5b
#define CUSTOM3 0x7b

#define LNS8  0x00
#define LNS16 0x01
#define LNS32 0x02
#define LNS64 0x03

#define LNS_ADD_FUNCT7  0x00 // 0b0000000
#define LNS_SUB_FUNCT7  0x04 // 0b0000100
#define LNS_MUL_FUNCT7  0x08 // 0b0001000
#define LNS_DIV_FUNCT7  0x0C // 0b0001100
#define LNS_SQRT_FUNCT7 0x10 // 0b0010000

#define LNS_EQ_FUNCT7   0x14 // 0b0010100
#define LNS_LT_FUNCT7   0x18 // 0b0011000
#define LNS_LE_FUNCT7   0x1C // 0b0011100

#define LNS_CVT_FUNCT7  0x20 // 0b0100000

#define LNS_ADD(rd, rs1, rs2, funct3) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_ADD_FUNCT7)
#define LNS_SUB(rd, rs1, rs2, funct3) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_SUB_FUNCT7)
#define LNS_MUL(rd, rs1, rs2, funct3) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_MUL_FUNCT7)
#define LNS_DIV(rd, rs1, rs2, funct3) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_DIV_FUNCT7)
#define LNS_SQRT(rd, rs1, funct3) \
  CUSTOM_R(rd, rs1, rs1, CUSTOM0, funct3, LNS_SQRT_FUNCT7)

#define LNS_EQ(rd, rs1, rs2, funct3) \
  LNS_COMPARE(rd, rs1, rs2, CUSTOM0, funct3, LNS_EQ_FUNCT7)
#define LNS_LT(rd, rs1, rs2, funct3) \
  LNS_COMPARE(rd, rs1, rs2, CUSTOM0, funct3, LNS_LT_FUNCT7)
#define LNS_LE(rd, rs1, rs2, funct3) \
  LNS_COMPARE(rd, rs1, rs2, CUSTOM0, funct3, LNS_LE_FUNCT7)

#define LNS_CVT_F32(rd, rs1, funct3) \
  CUSTOM_R(rd, rs1, rs1, CUSTOM0, funct3, LNS_CVT_FUNCT7)

#define LNS_LOAD_FUNCT3  0x5
#define LNS_STORE_FUNCT3 0x6

#define LNS_LOAD(rd, rs1, imm, funct7) \
  CUSTOM_I(rd, rs1, imm, CUSTOM1, LNS_LOAD_FUNCT3)

#define LNS_STORE(rs2, rs1, imm, funct7) \
  CUSTOM_S(rs2, rs1, imm, CUSTOM1, LNS_STORE_FUNCT3)

template<u8 n>
struct lns {
  f32 bits;

  static inline lns from_bits(f32 raw) {
    lns r;
    r.bits = raw;
    return r;
  }

  lns() : bits(0.0f) {}

  lns(f32 x) {
    u32 raw;
    memcpy(&raw, &x, sizeof(u32));

    constexpr u8 f_width = n / 2;  // fractional bits: 4 for lns8, 8 for lns16

    // Zero / subnormal → LNS zero sentinel
    if ((raw & 0x7FFFFFFFu) == 0 || ((raw >> 23) & 0xFF) == 0) {
      u32 sentinel = ((raw >> 31) << (n - 1)) | (1u << (n - 2));
      memcpy(&bits, &sentinel, sizeof(u32));  // bits is f32 but holds raw LNS
      return;
    }

    const u32 f32_sign = (raw >> 31) & 1;
    const i32 f32_exp  = (i32)((raw >> 23) & 0xFF) - 127;
    const u32 f32_frac = raw & 0x7FFFFFu;

    // Approximate log2(1 + mantissa) ≈ mantissa (good to ~1 ULP for small f)
    // Take the top f_width bits of the 23-bit IEEE mantissa
    const u32 lns_frac = f32_frac >> (23 - f_width);

    constexpr u32 exp_int_mask  = (1u << (n - f_width - 1)) - 1;
    constexpr u32 exp_frac_mask = (1u << f_width) - 1;

    u32 lns_raw =
        (f32_sign << (n - 1)) |
        (((u32)(f32_exp) & exp_int_mask) << f_width) |
        (lns_frac & exp_frac_mask);

    memcpy(&bits, &lns_raw, sizeof(u32));
  }

  static constexpr u8 funct7 =
    n == 8  ? LNS8  :
    n == 16 ? LNS16 :
    n == 32 ? LNS32 : LNS64;

  explicit operator f32() const {
    f32 in_bits  = this->bits;
    f32 out_bits;
    LNS_CVT_F32(out_bits, in_bits, funct7);
    return out_bits;
  }

  inline lns operator+(const lns other) const {
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    f32 out_bits;
    LNS_ADD(out_bits, left_bits, right_bits, funct7);
    return from_bits(out_bits);
  }

  inline lns operator-(const lns other) const {
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    f32 out_bits;
    LNS_SUB(out_bits, left_bits, right_bits, funct7);
    return from_bits(out_bits);
  }

  inline lns operator-() const {
    return lns(0.f) - *this;
  }

  inline lns operator*(const lns other) const {
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    f32 out_bits;
    LNS_MUL(out_bits, left_bits, right_bits, funct7);
    return from_bits(out_bits);
  }

  inline lns operator/(const lns other) const {
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    f32 out_bits;
    LNS_DIV(out_bits, left_bits, right_bits, funct7);
    return from_bits(out_bits);
  }

  inline lns sqrt() const {
    f32 input_bits = this->bits;
    f32 out_bits;
    LNS_SQRT(out_bits, input_bits, funct7);
    return from_bits(out_bits);
  }

  static inline lns load(const void* addr, u32 imm = 0) {
    const void* local_addr = addr;
    f32 out_bits;
    LNS_LOAD(out_bits, local_addr, imm, funct7);
    return from_bits(out_bits);
  }

  inline void store(void* addr, u32 imm = 0) const {
    void* local_addr = addr;
    f32 local_bits = this->bits;
    LNS_STORE(local_bits, local_addr, imm, funct7);
  }

  inline bool operator==(const lns other) const {
    u32 result;
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    LNS_EQ(result, left_bits, right_bits, funct7);
    return (bool)result;
  }

  inline bool operator!=(const lns other) const {
    return !(*this == other);
  }

  inline bool operator<(const lns other) const {
    u32 result;
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    LNS_LT(result, left_bits, right_bits, funct7);
    return (bool)result;
  }

  inline bool operator<=(const lns other) const {
    u32 result;
    f32 left_bits  = this->bits;
    f32 right_bits = other.bits;
    LNS_LE(result, left_bits, right_bits, funct7);
    return (bool)result;
  }

  inline bool operator>(const lns other) const {
    // a > b  ≡  b < a  — reuse LT, saves a funct3 slot
    return other < *this;
  }

  inline bool operator>=(const lns other) const {
    // a >= b  ≡  b <= a  — reuse LE
    return other <= *this;
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
};

using lns8  = lns<8>;
using lns16 = lns<16>;
using lns32 = lns<32>;
using lns64 = lns<64>;

#endif // !__LNS_H__
