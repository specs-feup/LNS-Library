#ifndef __LNS_LUTS_H__
#define __LNS_LUTS_H__

#include <utils.h>

template<u8 N> struct uint_tt;
template<> struct uint_tt<8>  { using type = u8;  };
template<> struct uint_tt<16> { using type = u16; };
template<> struct uint_tt<32> { using type = u32; };
template<> struct uint_tt<64> { using type = u64; };

template<u8 N> struct int_tt;
template<> struct int_tt<8>  { using type = i8;  };
template<> struct int_tt<16> { using type = i16; };
template<> struct int_tt<32> { using type = i32; };
template<> struct int_tt<64> { using type = i64; };

template<u8 N> using uint_t = typename uint_tt<N>::type;
template<u8 N> using int_t  = typename int_tt<N>::type;

template<u8 n>
struct spline_xf {
  u32 s_spline;
  struct {
    int_t<n> x, f;
  } point[];
};

template<u8 n>
struct spline_xmb {
  u32 s_spline;
  struct {
    int_t<n> x, m, b;
  } point[];
};

i8   lns8_read_tables   (const char* filename);
i8   lns16_read_tables  (const char* filename);

void lns_close          ();

#if defined(SPLINE_XF)

spline_xf<8>  *lns8_lut_add, *lns8_lut_sub;
spline_xf<16> *lns16_lut_add, *lns16_lut_sub;

i8  lns8_lut_compute  (const spline_xf<8>& lut,  const i8 diff, const u8 precision);
i16 lns16_lut_compute (const spline_xf<16>& lut, const i16 diff, const u8 precision);
static u8 log2_int(i64 n);

#elif defined(SPLINE_XMB)

spline_xmb<8>  *lns8_lut_add, *lns8_lut_sub;
spline_xmb<16> *lns16_lut_add, *lns16_lut_sub;

i8  lns8_lut_compute  (const spline_xmb<8>& lut,  const i8 diff, const u8 precision);
i16 lns16_lut_compute (const spline_xmb<16>& lut, const i16 diff, const u8 precision);

#endif

/*
extern newton_dd
  lns32_lut_add, lns32_lut_sub,
  lns64_lut_add, lns64_lut_sub;
*/

#if defined(SPLINE_XF)
i8 lns8_lut_compute(const spline_xf<8>& lut, const i8 diff, const u8 precision) {
  i8 x = diff;

  if (x < lut.point[0].x)
    x = lut.point[0].x;
  else if (x > lut.point[lut.s_spline - 1].x)
    x = lut.point[lut.s_spline - 1].x;

  u32 interval = 1;
  for (
    ;
    interval < lut.s_spline &&
    x > lut.point[interval].x;
    interval++
  );

  if (interval == 0)
    return 0;

  const i8 
    h_i = lut.point[interval].x - lut.point[interval - 1].x,
    h_i_log2 = log2_int(h_i);

  const i16 // Q2i.2f
    tmpa = (x - lut.point[interval - 1].x) * lut.point[interval].f,
    tmpb = (lut.point[interval].x - x) * lut.point[interval - 1].f,
    c    = (tmpa + tmpb) >> h_i_log2;

  return (i8)c;
}

i16 lns16_lut_compute(const spline_xf<16>& lut, const i16 diff, const u8 precision) {
  i16 x = diff;

  if (x < lut.point[0].x)
    x = lut.point[0].x;
  else if (x > lut.point[lut.s_spline - 1].x)
    x = lut.point[lut.s_spline - 1].x;

  u32 interval = 1;
  for (
    ;
    interval < lut.s_spline &&
    x > lut.point[interval].x;
    interval++
  );
  
  if (interval == 0)
    return 0;

  const i16 
    h_i = lut.point[interval].x - lut.point[interval - 1].x,
    h_i_log2 = log2_int(h_i);

  const i32 // Q2i.2f
    tmpa = (x - lut.point[interval - 1].x) * lut.point[interval].f,
    tmpb = (lut.point[interval].x - x) * lut.point[interval - 1].f,
    c    = (tmpa + tmpb) >> h_i_log2;

  return (i16)c;
}

static u8 log2_int(i64 n) {
  assert(n >= 0);

  if (n <= 0x00000001)
    return 0;
  else if (n <= 0x00000002)
    return 1;
  else if (n <= 0x00000004)
    return 2;
  else if (n <= 0x00000008)
    return 3;
  else if (n <= 0x00000010)
    return 4;
  else if (n <= 0x00000020)
    return 5;
  else if (n <= 0x00000040)
    return 6;
  else if (n <= 0x00000080)
    return 7;
  else if (n <= 0x00000100)
    return 8;
  else if (n <= 0x00000200)
    return 9;
  else if (n <= 0x00000400)
    return 10;
  else if (n <= 0x00000800)
    return 11;
  else if (n <= 0x00001000)
    return 12;
  else if (n <= 0x00002000)
    return 13;
  else if (n <= 0x00004000)
    return 14;
  else if (n <= 0x00008000)
    return 15;
  else if (n <= 0x00010000)
    return 16;
  else if (n <= 0x00020000)
    return 17;
  else if (n <= 0x00040000)
    return 18;
  else if (n <= 0x00080000)
    return 19;
  else if (n <= 0x00100000)
    return 20;
  else if (n <= 0x00200000)
    return 21;
  else if (n <= 0x00400000)
    return 22;
  else if (n <= 0x00400000)
    return 22;
  else if (n <= 0x00800000)
    return 23;
  else if (n <= 0x01000000)
    return 24;
  else if (n <= 0x02000000)
    return 25;
  else if (n <= 0x04000000)
    return 26;
  else if (n <= 0x08000000)
    return 27;
  else if (n <= 0x10000000)
    return 28;
  else if (n <= 0x20000000)
    return 29;
  else if (n <= 0x40000000)
    return 30;
  else
    return 31;
}

i8 lns8_read_tables(const char* filename) {
  if (filename == nullptr) {
    fprintf(stderr, "[ERROR]: filename is a nullptr in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -1;
  }

  FILE* file = fopen(filename, "rb");
  if (file == nullptr) {
    fprintf(stderr, "[ERROR]: could not open %s in %s at %u in %s", filename, __FUNCTION__, __LINE__, __FILE__);
    return -2;
  }

  u64 result = 0;

  u32 s_add, s_sub;

  result += fread(&s_add, sizeof(u32), 1, file);
  result += fread(&s_sub, sizeof(u32), 1, file);

  lns8_lut_add = (spline_xf<8>*)malloc(sizeof(spline_xf<8>) + s_add * sizeof(lns8_lut_add->point[0]));
  lns8_lut_sub = (spline_xf<8>*)malloc(sizeof(spline_xf<8>) + s_sub * sizeof(lns8_lut_sub->point[0]));

  lns8_lut_add->s_spline = s_add;
  lns8_lut_sub->s_spline = s_sub;

  result += fread(lns8_lut_add->point, sizeof(lns8_lut_add->point[0]), s_add, file);
  result += fread(lns8_lut_sub->point, sizeof(lns8_lut_sub->point[0]), s_sub, file);

  fclose(file);

  if (result != 2 + lns8_lut_add->s_spline + lns8_lut_sub->s_spline) {
    fprintf(stderr, "[ERROR]: incomplete read in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -3;
  }

  return 0;
}

i8 lns16_read_tables(const char* filename) {
  if (filename == nullptr) {
    fprintf(stderr, "[ERROR]: filename is a nullptr in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -1;
  }

  FILE* file = fopen(filename, "rb");
  if (file == nullptr) {
    fprintf(stderr, "[ERROR]: could not open %s in %s at %u in %s", filename, __FUNCTION__, __LINE__, __FILE__);
    return -2;
  }

  u64 result = 0;

  u32 s_add, s_sub;

  result += fread(&s_add, sizeof(u32), 1, file);
  result += fread(&s_sub, sizeof(u32), 1, file);

  lns16_lut_add = (spline_xf<16>*)malloc(sizeof(spline_xf<16>) + s_add * sizeof(lns16_lut_add->point[0]));
  lns16_lut_sub = (spline_xf<16>*)malloc(sizeof(spline_xf<16>) + s_sub * sizeof(lns16_lut_sub->point[0]));

  lns16_lut_add->s_spline = s_add;
  lns16_lut_sub->s_spline = s_sub;

  result += fread(lns16_lut_add->point, sizeof(lns16_lut_add->point[0]), s_add, file);
  result += fread(lns16_lut_sub->point, sizeof(lns16_lut_sub->point[0]), s_sub, file);

  fclose(file);

  if (result != 2 + lns16_lut_add->s_spline + lns16_lut_sub->s_spline) {
    fprintf(stderr, "[ERROR]: incomplete read in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -3;
  }

  return 0;
}

#elif defined(SPLINE_XMB)
i8 lns8_lut_compute(const spline_xmb<8>& lut,  const i8 diff, const u8 precision) {
  i8 x = diff;

  if (x < lut.point[0].x)
    x = lut.point[0].x;
  else if (x > lut.point[lut.s_spline - 1].x)
    x = lut.point[lut.s_spline - 1].x;

  u32 interval = 0;
  for (
    ;
    interval + 1 < lut.s_spline &&
    x > lut.point[interval + 1].x;
    interval++
  );

  const i16 tmp = lut.point[interval].m * x;
  return (i8)(tmp >> precision) + lut.point[interval].b;
}

i16 lns16_lut_compute(const spline_xmb<16>& lut, const i16 diff, const u8 precision) {
  i16 x = diff;

  if (x < lut.point[0].x)
    x = lut.point[0].x;
  else if (x > lut.point[lut.s_spline - 1].x)
    x = lut.point[lut.s_spline - 1].x;

  u32 interval = 0;
  for (
    ;
    interval + 1 < lut.s_spline &&
    x > lut.point[interval + 1].x;
    interval++
  );

  const i32 tmp = lut.point[interval].m * x;
  return (i16)(tmp >> precision) + lut.point[interval].b;
}

i8 lns8_read_tables(const char* filename) {
  if (filename == nullptr) {
    fprintf(stderr, "[ERROR]: filename is a nullptr in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -1;
  }

  FILE* file = fopen(filename, "rb");
  if (file == nullptr) {
    fprintf(stderr, "[ERROR]: could not open %s in %s at %u in %s", filename, __FUNCTION__, __LINE__, __FILE__);
    return -2;
  }

  u64 result = 0;

  u32 s_add, s_sub;

  result += fread(&s_add, sizeof(u32), 1, file);
  result += fread(&s_sub, sizeof(u32), 1, file);

  lns8_lut_add = (spline_xmb<8>*)malloc(sizeof(spline_xmb<8>) + s_add * sizeof(lns8_lut_add->point[0]));
  lns8_lut_sub = (spline_xmb<8>*)malloc(sizeof(spline_xmb<8>) + s_sub * sizeof(lns8_lut_sub->point[0]));

  lns8_lut_add->s_spline = s_add;
  lns8_lut_sub->s_spline = s_sub;

  result += fread(lns8_lut_add->point, sizeof(lns8_lut_add->point[0]), s_add, file);
  result += fread(lns8_lut_sub->point, sizeof(lns8_lut_sub->point[0]), s_sub, file);

  fclose(file);

  if (result != 2 + lns8_lut_add->s_spline + lns8_lut_sub->s_spline) {
    fprintf(stderr, "[ERROR]: incomplete read in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -3;
  }

  return 0;
}

i8 lns16_read_tables(const char* filename) {
  if (filename == nullptr) {
    fprintf(stderr, "[ERROR]: filename is a nullptr in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -1;
  }

  FILE* file = fopen(filename, "rb");
  if (file == nullptr) {
    fprintf(stderr, "[ERROR]: could not open %s in %s at %u in %s", filename, __FUNCTION__, __LINE__, __FILE__);
    return -2;
  }

  u64 result = 0;

  u32 s_add, s_sub;

  result += fread(&s_add, sizeof(u32), 1, file);
  result += fread(&s_sub, sizeof(u32), 1, file);

  lns16_lut_add = (spline_xmb<16>*)malloc(sizeof(spline_xmb<16>) + s_add * sizeof(lns16_lut_add->point[0]));
  lns16_lut_sub = (spline_xmb<16>*)malloc(sizeof(spline_xmb<16>) + s_sub * sizeof(lns16_lut_sub->point[0]));

  lns16_lut_add->s_spline = s_add;
  lns16_lut_sub->s_spline = s_sub;

  result += fread(lns16_lut_add->point, sizeof(lns16_lut_add->point[0]), s_add, file);
  result += fread(lns16_lut_sub->point, sizeof(lns16_lut_sub->point[0]), s_sub, file);

  fclose(file);

  if (result != 2 + lns16_lut_add->s_spline + lns16_lut_sub->s_spline) {
    fprintf(stderr, "[ERROR]: incomplete read in %s at %u in %s", __FUNCTION__, __LINE__, __FILE__);
    return -3;
  }
  return 0;
}

#endif

void lns_close() {
  free(lns8_lut_add);
  free(lns8_lut_sub);
  free(lns16_lut_add);
  free(lns16_lut_sub);
}

#endif // !__LNS_LUTS_H__
