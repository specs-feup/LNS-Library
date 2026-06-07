#ifndef __LNS_SIM_H__
#define __LNS_SIM_H__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <cstring>

#include <lnsluts.hpp>

template<u8 n, u8 i, u8 f>
struct lns {
  uint_t<n> bits;

  lns();
  lns(uint_t<n> raw, bool);
  lns(i32 x);
  lns(f32 x);

  explicit operator f32() const;

  u8        sign()    const;
  int_t<n>  exp()     const;
  bool      is_zero() const;

  lns  operator+ (const lns other) const;
  lns  operator- (const lns other) const;
  lns  operator- ()                const;
  lns  operator* (const lns other) const;
  lns  operator/ (const lns other) const;

  lns sqrt() const;

  lns& operator+=(const lns other);
  lns& operator-=(const lns other);
  lns& operator*=(const lns other);
  lns& operator/=(const lns other);

  bool operator==(const lns other) const;
  bool operator< (const lns other) const;
  bool operator> (const lns other) const;
  bool operator<=(const lns other) const;
  bool operator>=(const lns other) const;

  void print_hex()                    const;
  void print_bin()                    const;
  void debug_print(const char* label) const;

private:
  int_t<n> lns_f2l_compute         (const int_t<n> mantissa)           const;
  int_t<n> lns_l2f_compute         (const int_t<n> lns_f)              const;
  int_t<n> lns_add_and_sub_compute (bool use_add, const int_t<n> diff) const;
};

#include "lnssim.inl"

#endif // !__LNS_SIM_H__
