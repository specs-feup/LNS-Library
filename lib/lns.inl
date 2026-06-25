#ifndef __LNS_INL__
#define __LNS_INL__

/*
 * NOTE: Inline assembly with "f" constraints is used here because these custom
 * LNS instructions are not supported by the standard GCC compiler back-end.
 * By using "f" constraints and f32 types, we force the compiler to use the
 * floating-point register file, which the custom hardware hijacks for LNS logic.
 */

/*
 * NOTE: "memory" clobber is required here, not just "volatile".
 *
 * "volatile" only prevents GCC from deleting or reordering the asm
 * relative to other volatile ops. It does NOT stop GCC from treating
 * two separate instances of this asm block as equivalent/redundant if
 * their input operands look identical after optimization (register
 * allocation, inlining, CSE/GCSE at -O2+). GCC has no model of the
 * custom LNS functional unit and assumes it's a pure function of its
 * declared "f"/"i" inputs -- so at -O2 it silently merged two distinct
 * custom instructions into one, dropping an operation and producing
 * wrong results, since the hardware unit is not actually pure/stateless
 * in the way GCC assumes.
 *
 * The "memory" clobber forces GCC to treat every instance as having an
 * unknown side effect, which blocks this cross-instance merging/reuse.
 * This is heavier than strictly necessary (it also blocks reordering
 * relative to unrelated loads/stores), but it's the standard fix for
 * custom opcodes GCC doesn't understand the semantics of.
 *
 * If we ever need finer-grained control (e.g. to claw back optimizer
 * headroom), look into making each call provably distinct to GCC
 * without a full memory barrier -- but do NOT remove this clobber
 * without re-verifying instruction counts in -O2 disassembly against
 * source-level operation counts.
 */

#define CUSTOM_R(rd, rs1, rs2, opcode, funct3, funct7) \
  asm volatile ( \
    ".insn r %3, %4, %5, %0, %1, %2" \
    : "=&f" (rd) \
    : "f" (rs1), "f" (rs2), "i" (opcode), "i" (funct3), "i" (funct7) \
    : "memory" /* prevents GCC from CSE-merging distinct custom ops at -O2+, see note above */ \
  )

#define LNS_COMPARE(rd, rs1, rs2, opcode, funct3, funct7) \
  asm volatile ( \
    ".insn r %3, %4, %5, %0, %1, %2" \
    : "=&r" (rd) \
    : "f" (rs1), "f" (rs2), "i" (opcode), "i" (funct3), "i" (funct7) \
    : "memory" /* prevents GCC from CSE-merging distinct custom ops at -O2+, see note above */ \
  )

#define CUSTOM_I(rd, rs1, imm, opcode, funct3) \
  asm volatile ( \
    ".insn i %3, %4, %0, %2(%1)" \
    : "=&f" (rd) \
    : "r" (rs1), "i" (imm), "i" (opcode), "i" (funct3) \
    : "memory" /* prevents GCC from CSE-merging distinct custom ops at -O2+, see note above */ \
  )

#define CUSTOM_S(rs2, rs1, imm, opcode, funct3) \
  asm volatile ( \
    ".insn s %3, %4, %0, %2(%1)" \
    : \
    : "f" (rs2), "r" (rs1), "i" (imm), "i" (opcode), "i" (funct3) \
    : "memory" /* prevents GCC from CSE-merging distinct custom ops at -O2+, see note above */ \
  )

#define CUSTOM0 0x0b
#define CUSTOM1 0x2b
#define CUSTOM2 0x5b
#define CUSTOM3 0x7b

#define LNS_ADD_FUNCT7  0x00
#define LNS_SUB_FUNCT7  0x04
#define LNS_MUL_FUNCT7  0x08
#define LNS_DIV_FUNCT7  0x0C
#define LNS_SQRT_FUNCT7 0x10
#define LNS_EQ_FUNCT7   0x14
#define LNS_LT_FUNCT7   0x18
#define LNS_LE_FUNCT7   0x1C
#define LNS_CVT_FUNCT7  0x20

#define LNS8  0x00
#define LNS16 0x01
#define LNS32 0x02
#define LNS64 0x03

#define FUNCT3(n) (n == 8 ? LNS8 : n == 16 ? LNS16 : n == 32 ? LNS32 : LNS64)

#define LNS_ADD(rd, rs1, rs2, funct3)    CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_ADD_FUNCT7)
#define LNS_SUB(rd, rs1, rs2, funct3)    CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_SUB_FUNCT7)
#define LNS_MUL(rd, rs1, rs2, funct3)    CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_MUL_FUNCT7)
#define LNS_DIV(rd, rs1, rs2, funct3)    CUSTOM_R(rd, rs1, rs2, CUSTOM0, funct3, LNS_DIV_FUNCT7)
#define LNS_SQRT(rd, rs1, funct3)        CUSTOM_R(rd, rs1, rs1, CUSTOM0, funct3, LNS_SQRT_FUNCT7)
#define LNS_EQ(rd, rs1, rs2, funct3)     LNS_COMPARE(rd, rs1, rs2, CUSTOM0, funct3, LNS_EQ_FUNCT7)
#define LNS_LT(rd, rs1, rs2, funct3)     LNS_COMPARE(rd, rs1, rs2, CUSTOM0, funct3, LNS_LT_FUNCT7)
#define LNS_LE(rd, rs1, rs2, funct3)     LNS_COMPARE(rd, rs1, rs2, CUSTOM0, funct3, LNS_LE_FUNCT7)
#define LNS_CVT_F32(rd, rs1, funct3)     CUSTOM_R(rd, rs1, rs1, CUSTOM0, funct3, LNS_CVT_FUNCT7)
#define LNS_LOAD(rd, rs1, imm, funct3)   CUSTOM_I(rd, rs1, imm, CUSTOM1, funct3)
#define LNS_STORE(rs2, rs1, imm, funct3) CUSTOM_S(rs2, rs1, imm, CUSTOM2, funct3)

union f32_to_u32 {
  f32 f;
  u32 u;
};

template<u8 n>
lns<n>::lns() : bits(0.0f) {}


template<u8 n>
lns<n>::lns(const lns& other) {
  this->bits = other.bits; 
}

template<u8 n>
lns<n>::lns(u32 raw) {
  f32_to_u32 pun;
  pun.u = raw;
  bits = pun.f;
}

template<u8 n>
lns<n>::operator f32() const {
  f32 
    in_bits = this->bits,
    out_bits;

  LNS_CVT_F32(out_bits, in_bits, FUNCT3(n));

  return out_bits;
}

template<u8 n>
lns<n> lns<n>::operator+(const lns other) const {
  f32 
    left_bits  = this->bits,
    right_bits = other.bits,
    out_bits;

  LNS_ADD(out_bits, left_bits, right_bits, FUNCT3(n));

  f32_to_u32 pun;
  pun.f = out_bits;

  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::operator-(const lns other) const {
  f32 
    left_bits  = this->bits,
    right_bits = other.bits,
    out_bits;

  LNS_SUB(out_bits, left_bits, right_bits, FUNCT3(n));
 
  f32_to_u32 pun;
  pun.f = out_bits;

  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::operator-() const {
  lns r;
  f32_to_u32 pun;

  pun.f = this->bits;
  pun.u ^= (1u << (n - 1));

  r.bits = pun.f;
  return r;
}

template<u8 n>
lns<n> lns<n>::operator*(const lns other) const {
  f32 
    left_bits  = this->bits,
    right_bits = other.bits,
    out_bits;

  LNS_MUL(out_bits, left_bits, right_bits, FUNCT3(n));
 
  f32_to_u32 pun;
  pun.f = out_bits;

  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::operator/(const lns other) const {
  f32 
    left_bits  = this->bits,
    right_bits = other.bits,
    out_bits;

  LNS_DIV(out_bits, left_bits, right_bits, FUNCT3(n));

  f32_to_u32 pun;
  pun.f = out_bits;

  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::sqrt() const {
  f32 
    input_bits = this->bits,
    out_bits;

  LNS_SQRT(out_bits, input_bits, FUNCT3(n));
 
  f32_to_u32 pun;
  pun.f = out_bits;

  return lns(pun.u);
}

template<u8 n>
lns<n>& lns<n>::operator+=(const lns other) {
  *this = *this + other;
  return *this;
}

template<u8 n>
lns<n>& lns<n>::operator-=(const lns other) {
  *this = *this - other;
  return *this;
}

template<u8 n>
lns<n>& lns<n>::operator*=(const lns other) {
  *this = *this * other;
  return *this;
}

template<u8 n>
lns<n>& lns<n>::operator/=(const lns other) {
  *this = *this / other;
  return *this;
}

template<u8 n>
lns<n>& lns<n>::operator=(const lns other) {
  this->bits = other.bits; 
  return *this;
}

template<u8 n>
void lns<n>::operator=(const lns other) volatile {
  this->bits = other.bits; 
}

template<u8 n>
bool lns<n>::operator==(const lns other) const {
  u32 result;

  f32 
    left_bits  = this->bits,
    right_bits = other.bits;

  LNS_EQ(result, left_bits, right_bits, FUNCT3(n));

  return (bool)result;
}

template<u8 n>
bool lns<n>::operator!=(const lns other) const {
  return !(*this == other);
}

template<u8 n>
bool lns<n>::operator<(const lns other) const {
  u32 result;

  f32 
    left_bits  = this->bits,
    right_bits = other.bits;

  LNS_LT(result, left_bits, right_bits, FUNCT3(n));

  return (bool)result;
}

template<u8 n>
bool lns<n>::operator<=(const lns other) const {
  u32 result;

  f32 
    left_bits  = this->bits,
    right_bits = other.bits;

  LNS_LE(result, left_bits, right_bits, FUNCT3(n));

  return (bool)result;
}

template<u8 n>
bool lns<n>::operator>(const lns other) const {
  return other < *this;
}

template<u8 n>
bool lns<n>::operator>=(const lns other) const {
  return other <= *this;
}

template<u8 n>
lns<n> lns<n>::load(const void* addr, u32 imm) {
  const void* local_addr = addr;

  f32 out_bits;

  LNS_LOAD(out_bits, local_addr, imm, FUNCT3(n));

  f32_to_u32 pun;
  pun.f = out_bits;

  return lns(pun.u);
}

template<u8 n>
void lns<n>::store(void* addr, u32 imm) const {
  void* local_addr = addr;
  f32   local_bits = this->bits;
  LNS_STORE(local_bits, local_addr, imm, FUNCT3(n));
}

#endif // !__LNS_INL__
