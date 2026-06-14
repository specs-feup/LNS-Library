#ifndef __BFLOAT_SIM_INL__
#define __BFLOAT_SIM_INL__

bf8::bf8()
  : bits(0) {}

bf8::bf8(u8 raw, bool)
  : bits(raw) {}

bf8::bf8(i32 x) {
  *this = bf8((f32)x);
}

bf8::bf8(bf16 x) {
  *this = bf16((f32)x);
}

bf8::bf8(f32 x) {
  u32 fb;
  memcpy(&fb, &x, sizeof(u32));

  const u32 
    sign     = (fb >> 31) & 1u,
    exp_raw  = (fb >> 23) & 0xFFu,
    frac_f32 = fb & 0x7FFFFFu;

  // Handle zero (including -0.0)
  if (x == 0.0f) {
    bits = (u8)(sign << 7);
    return;
  }

  // Handle NaN and Inf: clamp to max normal (E4M3 has no NaN/Inf encoding)
  if (exp_raw == 0xFFu) {
    bits = (u8)((sign << 7) | 0x77u);
    return;
  }

  // Handle f32 subnormals: flush to signed zero
  if (exp_raw == 0u) {
    bits = (u8)(sign << 7);
    return;
  }

  i32 
    exp_f32 = (i32)exp_raw - 127, // unbias from f32
    exp_bf8 = exp_f32 + 7;        // rebias to 4-bit bias (7)

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

  // Round-to-nearest-even: keep top 3 mantissa bits, round on bit 19
  u32 
    frac3     = (frac_f32 >> 20) & 0x7u,        // top 3 bits kept
    round_bit = (frac_f32 >> 19) & 1u,           // first discarded bit
    sticky    = (frac_f32 & 0x7FFFFu) ? 1u : 0u; // any remaining bits set

  frac3 += round_bit & (sticky | (frac3 & 1u)); // RNE

  // Rounding carry into exponent
  if (frac3 > 0x7u) {
    frac3 = 0;
    exp_bf8 += 1;

    if (exp_bf8 >= 15) {
      bits = (u8)((sign << 7) | 0x77u);
      return;
    }
  }

  bits = (u8)((sign << 7) | ((u32)exp_bf8 << 3) | frac3);
}

bf8::operator f32() const {
  const u32 
    sign = (bits >> 7) & 1u,
    exp  = (bits >> 3) & 0xFu,
    frac = bits & 0x7u;

  if (exp == 0)
    return sign ? -0.0f : 0.0f; // flushed zero

  const i32 exp_f32 = (i32)exp - 7 + 127; // unbias from bf8, rebias to f32
  const u32 fb      = (sign << 31) | ((u32)exp_f32 << 23) | (frac << 20);

  f32 r;
  memcpy(&r, &fb, sizeof(f32));

  return r;
}

inline bool bf8::operator==(const bf8 other) const {
  return bits == other.bits;
}

inline bool bf8::operator<(const bf8 other) const {
  return (f32)*this < (f32)other;
}

inline bool bf8::operator>(const bf8 other) const {
  return (f32)*this > (f32)other;
}

inline bool bf8::operator<=(const bf8 other) const {
  return *this < other || bits == other.bits;
}

inline bool bf8::operator>=(const bf8 other) const {
  return *this > other || bits == other.bits;
}

bf8 bf8::operator+(const bf8 other) const {
  return bf8((f32)*this + (f32)other);
}

bf8 bf8::operator-(const bf8 other) const {
  return bf8((f32)*this - (f32)other);
}

bf8 bf8::operator-() const {
  return bf8(bits ^ (1u << 7), true); // flip sign bit
}

bf8 bf8::operator*(const bf8 other) const {
  return bf8((f32)*this * (f32)other);
}

bf8 bf8::operator/(const bf8 other) const {
  if ((other.bits & 0x7Fu) == 0) {
    fprintf(stderr, "[BF8 FATAL]: Division by Zero detected.\n");

    exit(139);
  }
  return bf8((f32)*this / (f32)other);
}

inline bf8& bf8::operator+=(const bf8 other) {
  *this = *this + other;
  return *this;
}

inline bf8& bf8::operator-=(const bf8 other) {
  *this = *this - other;
  return *this;
}

inline bf8& bf8::operator*=(const bf8 other) {
  *this = *this * other;
  return *this;
}

inline bf8& bf8::operator/=(const bf8 other) {
  *this = *this / other;
  return *this;
}


void bf8::print_hex() const {
  printf("0x%02X", bits);
}

void bf8::print_bin() const {
  printf("0b");
  for (i8 k = 7; k >= 0; k--) {
    printf("%d", (bits >> k) & 1);
    if (k == 7)
      printf("_");
    if (k == 3)
      printf("_");
  }
}

void bf8::debug_print(const char* label) const {
  if (label[0] != '\0')
    printf("%s: ", label);
  printf("%f (raw: ", (f32)*this);
  print_hex();
  printf(") ");
  print_bin();
  printf("\n");
}

bf16::bf16()
  : bits(0) {}

bf16::bf16(u16 raw, bool)
  : bits(raw) {}

bf16::bf16(i32 x) {
  *this = bf16((f32)x);
}

bf16::bf16(bf8 x) {
  *this = bf16((f32)x);
}

bf16::bf16(f32 x) {
  u32 _bits;
  memcpy(&_bits, &x, sizeof(u32));
  _bits &= 0xFFFF0000u;
  bits = _bits >> 16;
}

inline bf16::operator f32() const {
  u32 _bits = bits << 16;
  f32 x;
  memcpy(&x, &_bits, sizeof(f32));
  return x;
}

inline bool bf16::operator==(const bf16 other) const {
  return bits == other.bits;
}

inline bool bf16::operator<(const bf16 other) const {
  return (f32)*this < (f32)other;
}

inline bool bf16::operator>(const bf16 other) const {
  return (f32)*this > (f32)other;
}

inline bool bf16::operator<=(const bf16 other) const {
  return *this < other || bits == other.bits;
}

inline bool bf16::operator>=(const bf16 other) const {
  return *this > other || bits == other.bits;
}

bf16 bf16::operator+(const bf16 other) const {
  return bf16((f32)*this + (f32)other);
}

bf16 bf16::operator-(const bf16 other) const {
  return bf16((f32)*this - (f32)other);
}

bf16 bf16::operator-() const {
  return bf16(bits ^ (1u << 15), false);
}

bf16 bf16::operator*(const bf16 other) const {
  return bf16((f32)*this * (f32)other);
}

bf16 bf16::operator/(const bf16 other) const {
  if ((other.bits & 0x7FFFu) == 0) { // check magnitude only, handles -0.0
    fprintf(stderr, "[BF16 FATAL]: Division by Zero detected.\n");
    exit(139);
  }

  return bf16((f32)*this / (f32)other);
}

inline bf16& bf16::operator+=(const bf16 other) {
  *this = *this + other;
  return *this;
}

inline bf16& bf16::operator-=(const bf16 other) {
  *this = *this - other;
  return *this;
}

inline bf16& bf16::operator*=(const bf16 other) {
  *this = *this * other;
  return *this;
}

inline bf16& bf16::operator/=(const bf16 other) {
  *this = *this / other;
  return *this;
}

void bf16::print_hex() const {
  printf("0x%0*X", 4, bits);
}

void bf16::print_bin() const {
  printf("0b");
  for (i8 k = 15; k >= 0; k--) {
    printf("%d", (bits >> k) & 1);
    if (k == 15)
      printf("_");
    if (k == 7)
      printf(".");
  }
}

void bf16::debug_print(const char* label) const {
  if (label[0] != '\0')
    printf("%s: ", label);
  printf("%f (raw: ", (f32)*this);
  print_hex();
  printf(") ");
  print_bin();
  printf("\n");
}

#endif /* __BFLOAT_SIM_INL__ */
