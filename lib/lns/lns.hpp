#ifndef __LNS_H__
#define __LNS_H__

#include "utils.h"

#define LNS8  0x00
#define LNS16 0x01
#define LNS32 0x02
#define LNS64 0x03

template<u8 n>
struct lns {
  f32 bits;

  static constexpr u8 funct3 =
    n == 8  ? LNS8  :
    n == 16 ? LNS16 :
    n == 32 ? LNS32 : LNS64;

  lns();
  lns(f32 x);

  static inline lns from_bits(f32 raw);

  explicit operator f32() const;

  lns  operator+ (const lns other) const;
  lns  operator- (const lns other) const;
  lns  operator- ()                const;
  lns  operator* (const lns other) const;
  lns  operator/ (const lns other) const;

  lns  sqrt      ()                const;

  lns& operator+=(const lns other);
  lns& operator-=(const lns other);
  lns& operator*=(const lns other);
  lns& operator/=(const lns other);

  bool operator==(const lns other) const;
  bool operator!=(const lns other) const;
  bool operator< (const lns other) const;
  bool operator<=(const lns other) const;
  bool operator> (const lns other) const;
  bool operator>=(const lns other) const;

  static inline lns load(const void* addr, u32 imm = 0);
  inline void store(void* addr, u32 imm = 0) const;
};

using lns8  = lns<8>;
using lns16 = lns<16>;
using lns32 = lns<32>;
using lns64 = lns<64>;

#include "lns.inl"

#endif // !__LNS_H__
