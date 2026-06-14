#ifndef __BFLOAT_SIM_H__
#define __BFLOAT_SIM_H__

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "utils.h"

struct bf8;
struct bf16;

struct bf8 {
  u8 bits; // E4M3: [S | EEEE | MMM]

  bf8();
  bf8(u8 raw, bool);
  bf8(i32 x);
  bf8(bf16 x);
  bf8(f32 x);

  operator f32() const;

  inline bool operator== (const bf8 other) const;
  inline bool operator<  (const bf8 other) const;
  inline bool operator>  (const bf8 other) const;
  inline bool operator<= (const bf8 other) const;
  inline bool operator>= (const bf8 other) const; 

  bf8 operator+          (const bf8 other) const; 
  bf8 operator-          (const bf8 other) const; 
  bf8 operator-          ()                const; 
  bf8 operator*          (const bf8 other) const; 
  bf8 operator/          (const bf8 other) const; 

  inline bf8& operator+= (const bf8 other); 
  inline bf8& operator-= (const bf8 other); 
  inline bf8& operator*= (const bf8 other); 
  inline bf8& operator/= (const bf8 other); 

  void print_hex         () const; 
  void print_bin         () const; 
  void debug_print       (const char* label = "") const; 
};

struct bf16 {
  u16 bits; // E8M7: [S | EEEEEEEE | MMMMMMM]

  bf16();
  bf16(u16 raw, bool);
  bf16(i32 x);
  bf16(bf8 x);
  bf16(f32 x);

  inline operator f32() const;

  inline bool operator== (const bf16 other) const;
  inline bool operator<  (const bf16 other) const;
  inline bool operator>  (const bf16 other) const;
  inline bool operator<= (const bf16 other) const;
  inline bool operator>= (const bf16 other) const; 

  bf16 operator+          (const bf16 other) const; 
  bf16 operator-          (const bf16 other) const; 
  bf16 operator-          ()                const; 
  bf16 operator*          (const bf16 other) const; 
  bf16 operator/          (const bf16 other) const; 

  inline bf16& operator+= (const bf16 other); 
  inline bf16& operator-= (const bf16 other); 
  inline bf16& operator*= (const bf16 other); 
  inline bf16& operator/= (const bf16 other); 

  void print_hex          () const; 
  void print_bin          () const; 
  void debug_print        (const char* label = "") const; 
};

#include "bfloatsim.inl"

#endif // !__BFLOAT_SIM_H__
