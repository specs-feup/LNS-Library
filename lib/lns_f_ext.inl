#ifndef __LNS_F_EXT_INL__
#define __LNS_F_EXT_INL__

/*
 * This version targets the standard RISC-V "F" (single-precision float)
 * extension only. Unlike the custom-opcode version, GCC has a correct
 * model of fadd.s/fsub.s/fmul.s/fdiv.s/fsqrt.s/feq.s/flt.s/fle.s, so we
 * can just write plain C++ float arithmetic and let the compiler emit
 * the right instructions. No inline asm, no "memory" clobber, no
 * CSE-merging hazard to work around.
 *
 * NOTE: this class still stores its value as a raw-bit-punned f32 (via
 * the union below) so that construction from a raw u32 matches the
 * original LNS-backed version. The actual arithmetic, though, is just
 * normal float math now.
 */

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
  // Deliberately does not compile. In the custom-opcode version this
  // called LNS_CVT_F32 to convert from the LNS log-number encoding to
  // a real IEEE-754 f32 -- a conversion that only makes sense because
  // the LNSU stores values as logarithms.
  //
  // This version of the header targets the plain RISC-V F extension,
  // where lns<n>'s stored bits are already an ordinary f32 (see the
  // constructors above). There is no LNS encoding here to convert
  // *from*, so a call to this operator almost certainly means the
  // caller still thinks they're talking to an LNSU/custom LNS unit
  // rather than the FPU. Silently returning `bits` would compile but
  // give a value that looks right and means something different
  // (skips the log->linear conversion the original code depended on),
  // so we fail the build instead of the caller's math.
  //
  // If you actually need f32, and you're sure this is the F-extension
  // build, just use the value directly -- lns<n>'s internal storage
  // already *is* an f32.
  static_assert(sizeof(n) == 0,
      "lns<n>::operator f32() is unavailable in the F-extension build: "
      "there is no LNS encoding to convert from an FPU. This function "
      "existed to invoke the LNSU's log->linear conversion instruction; "
      "using it here indicates code written against the LNSU, not the "
      "plain F-extension. Access lns<n>::bits directly instead.");
  return 0;
}

template<u8 n>
lns<n> lns<n>::operator+(const lns other) const {
  f32_to_u32 pun;
  pun.f = this->bits + other.bits;
  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::operator-(const lns other) const {
  f32_to_u32 pun;
  pun.f = this->bits - other.bits;
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
  f32_to_u32 pun;
  pun.f = this->bits * other.bits;
  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::operator/(const lns other) const {
  f32_to_u32 pun;
  pun.f = this->bits / other.bits;
  return lns(pun.u);
}

template<u8 n>
lns<n> lns<n>::sqrt() const {
  f32_to_u32 pun;
  pun.f = __builtin_sqrtf(this->bits); // compiles to fsqrt.s
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
  return this->bits == other.bits; // compiles to feq.s
}

template<u8 n>
bool lns<n>::operator!=(const lns other) const {
  return !(*this == other);
}

template<u8 n>
bool lns<n>::operator<(const lns other) const {
  return this->bits < other.bits; // compiles to flt.s
}

template<u8 n>
bool lns<n>::operator<=(const lns other) const {
  return this->bits <= other.bits; // compiles to fle.s
}

template<u8 n>
bool lns<n>::operator>(const lns other) const {
  return other < *this;
}

template<u8 n>
bool lns<n>::operator>=(const lns other) const {
  return other <= *this;
}

#endif // !__LNS_F_EXT_INL__
