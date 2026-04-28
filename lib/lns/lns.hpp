#ifndef __LNS_H__
#define __LNS_H__
#include <utils.h>

#define CUSTOM_R(rd, rs1, rs2, opcode, funct3, funct7) \
  asm volatile ( \
    ".insn r %[op], %[f3], %[f7], %[rd], %[rs1], %[rs2]" \
    : [rd]  "=r" (rd) \
    : [rs1] "r"  (rs1), \
      [rs2] "r"  (rs2), \
      [op]  "i"  (opcode), \
      [f3]  "i"  (funct3), \
      [f7]  "i"  (funct7) \
  )

#define CUSTOM0 0x0b
#define CUSTOM1 0x2b
#define CUSTOM2 0x5b
#define CUSTOM3 0x7b

#define LNS8  0x00
#define LNS16 0x01
#define LNS32 0x02
#define LNS64 0x03

#define LNS_ADD_FUNCT3  0x0
#define LNS_SUB_FUNCT3  0x1
#define LNS_MUL_FUNCT3  0x2
#define LNS_DIV_FUNCT3  0x3
#define LNS_SQRT_FUNCT3 0x4

#define LNS_ADD(rd, rs1, rs2, funct7) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, LNS_ADD_FUNCT3, funct7)
#define LNS_SUB(rd, rs1, rs2, funct7) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, LNS_SUB_FUNCT3, funct7)
#define LNS_MUL(rd, rs1, rs2, funct7) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, LNS_MUL_FUNCT3, funct7)
#define LNS_DIV(rd, rs1, rs2, funct7) \
  CUSTOM_R(rd, rs1, rs2, CUSTOM0, LNS_DIV_FUNCT3, funct7)
#define LNS_SQRT(rd, rs1, funct7) \
  CUSTOM_R(rd, rs1, CUSTOM0, custom, LNS_SQRT_FUNCT3, funct7)

// Load: I-type
#define CUSTOM_I(rd, rs1, imm, opcode, funct3) \
  asm volatile ( \
    ".insn i %[op], %[f3], %[rd], %[imm](%[rs1])" \
    : [rd]  "=r" (rd) \
    : [rs1] "r"  (rs1), \
      [imm] "i"  (imm), \
      [op]  "i"  (opcode), \
      [f3]  "i"  (funct3) \
  )

// Store: S-type (no output operand)
#define CUSTOM_S(rs2, rs1, imm, opcode, funct3) \
  asm volatile ( \
    ".insn s %[op], %[f3], %[rs2], %[imm](%[rs1])" \
    : \
    : [rs1] "r"  (rs1), \
      [rs2] "r"  (rs2), \
      [imm] "i"  (imm), \
      [op]  "i"  (opcode), \
      [f3]  "i"  (funct3) \
  )

#define LNS_LOAD_FUNCT3  0x5
#define LNS_STORE_FUNCT3 0x6

#define LNS_LOAD(rd, rs1, imm, funct7) \
  CUSTOM_I(rd, rs1, imm, CUSTOM1, LNS_LOAD_FUNCT3)

#define LNS_STORE(rs2, rs1, imm, funct7) \
  CUSTOM_S(rs2, rs1, imm, CUSTOM2, LNS_STORE_FUNCT3)

template<u8 n>
struct lns {
  // uint_t<n> bits;
  f32 bits;

  static constexpr u8 funct7 =
    n == 8  ? LNS8  :
    n == 16 ? LNS16 :
    n == 32 ? LNS32 : LNS64;

  inline lns operator+(const lns other) const {
    lns result;
    LNS_ADD(result.bits, bits, other.bits, funct7);
    return result;
  }

  inline lns operator-(const lns other) const {
    lns result;
    LNS_SUB(result.bits, bits, other.bits, funct7);
    return result;
  }

  inline lns operator*(const lns other) const {
    lns result;
    LNS_MUL(result.bits, bits, other.bits, funct7);
    return result;
  }

  inline lns operator/(const lns other) const {
    lns result;
    LNS_DIV(result.bits, bits, other.bits, funct7);
    return result;
  }

  inline lns sqrt() const {
    lns result;
    LNS_SQRT(result.bits, bits, funct7);
    return result;
  }

  static inline lns load(const void* addr, u32 imm = 0) {
    lns result;
    LNS_LOAD(result.bits, addr, imm, funct7);
    return result;
  }

  inline void store(void* addr, u32 imm = 0) const {
    LNS_STORE(bits, addr, imm, funct7);
  }

  inline bool operator==(const lns other) const {
    return bits == other.bits;
  }

  /*
  inline bool operator< (const lns other) const {
    return (f32)*this <  (f32)other;
  }

  inline bool operator> (const lns other) const {
    return (f32)*this >  (f32)other;
  }

  inline bool operator<=(const lns other) const {
    return (f32)*this <= (f32)other;
  }

  inline bool operator>=(const lns other) const {
    return (f32)*this >= (f32)other;
  }
  * */

  inline void operator=(const lns& other) {
    store(&other);
  }
  
  inline lns& operator+=(const lns other) {
    *this = *this + other;
    return result;
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
