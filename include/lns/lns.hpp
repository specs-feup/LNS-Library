#ifndef __LNS_H__
#define __LNS_H__

template<u8 n>
struct lns {
  static constexpr u8 f_width =
    n == 8  ? 3 :
    n == 16 ? 7 :
    n == 32 ? 19 : 47;

  f32 bits;

  lns();
  lns(u32 raw);

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

  void operator= (const lns other) volatile;

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
