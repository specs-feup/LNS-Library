#ifndef __LNS_SIM_H__
#define __LNS_SIM_H__

#include <lnsluts.hpp>
#include <cmath>
#include <cfloat>

#define LNS_ZERO(n)                         (1 << (n - 2))
#define F32_SIGN(raw)                       ((raw >> 31) & 1)
#define F32_EXP(raw)                        ((raw >> 23) & 0xFF)
#define F32_FRAC(raw)                       (raw & 0x7FFFFF)
#define F32_FRAC_U32_TO_FLOAT(frac)         ((f64)frac / (f64)(1 << 23))
#define F32_TO_LNS_FRAC(mantissa, n, prec)  ((uint_t<n>)(lns_f2l_compute(mantissa) >> (n - 1 - prec)))

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

    void print_hex()                     const;
    void print_bin()                     const;
    void debug_print(const char* label)  const;

private:
    int_t<n> lns_f2l_compute        (const int_t<n> mantissa)             const;
    int_t<n> lns_l2f_compute        (const int_t<n> lns_f)                const;
    int_t<n> lns_add_and_sub_compute (bool use_add, const int_t<n> diff)  const;
};

#include "lnssim.inl"

#endif // !__LNS_SIM_H__
