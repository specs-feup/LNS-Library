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

#define LNS_ADD_FUNCT3 0x0
#define LNS_SUB_FUNCT3 0x1
#define LNS_MUL_FUNCT3 0x2
#define LNS_DIV_FUNCT3 0x3
#define LNS_SQRT_FUNCT3 0x4

#define LNS_ADD(custom, rd, rs1, rs2) \
  CUSTOM_R(rd, rs1, rs2, custom, LNS_ADD_FUNCT3, 0x00)
#define LNS_SUB(custom, rd, rs1, rs2) \
  CUSTOM_R(rd, rs1, rs2, custom, LNS_SUB_FUNCT3, 0x00)
#define LNS_MUL(custom, rd, rs1, rs2) \
  CUSTOM_R(rd, rs1, rs2, custom, LNS_MUL_FUNCT3, 0x00)
#define LNS_DIV(custom, rd, rs1, rs2) \
  CUSTOM_R(rd, rs1, rs2, custom, LNS_DIV_FUNCT3, 0x00)
#define LNS_SQRT(custom, rd, rs1)
  CUSTOM_R(rd, rs1, 0x0, custom, LNS_SQRT_FUNCT3, 0x00)

template<u8 n>
struct lns {
  uint_t<n> bits;

  static constexpr u8 custom =
    n == 8  ? CUSTOM0 :
    n == 16 ? CUSTOM1 :
    n == 32 ? CUSTOM2 : CUSTOM3;

  inline lns operator+(const lns other) const {
    lns result;
    LNS_ADD(custom, result.bits, bits, other.bits);
    return result;
  }

  inline lns operator-(const lns other) const {
    lns result;
    LNS_SUB(custom, result.bits, bits, other.bits);
    return result;
  }

  inline lns operator*(const lns other) const {
    lns result;
    LNS_MUL(custom, result.bits, bits, other.bits);
    return result;
  }

  inline lns operator/(const lns other) const {
    lns result;
    LNS_DIV(custom, result.bits, bits, other.bits);
    return result;
  }

  inline lns sqrt() const {
    lns result;
    LNS_SQRT(custom, result.bits, bits);
    return result;
  }
};

using lns8  = lns<8>;
using lns16 = lns<16>;
using lns32 = lns<32>;
using lns64 = lns<64>;

#endif // !__LNS_H__
