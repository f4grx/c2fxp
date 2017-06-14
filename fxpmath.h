/* This file is public domain. */

#ifndef FXPMATH__H
#define FXPMATH__H

#include <stdint.h>

/* Types */

typedef int16_t q15_t;
typedef int32_t q31_t;
typedef int64_t q63_t;

/* Signed Fixed point types with 1 sign bit and N-1 fractional bits */

#define Q15BITS 15
#define Q31BITS 31
#define Q63BITS 63

#define Q15 (1    << Q15BITS)
#define Q31 (1LU  << Q31BITS)
#define Q63 (1LLU << Q63BITS)

/* Conversions to float types */

#define Q15TOD(q15) (double)((q15)/(double)Q15)
#define Q15TOF(q15) (float )((q15)/(float )Q15)

#define Q31TOD(q31) (double)((q31)/(double)Q31)
#define Q31TOF(q31) (float )((q31)/(float )Q31)

#define Q63TOD(q63) (double)((q63)/(double)Q63)
#define Q63TOF(q63) (float )((q63)/(float )Q63)

/* Conversion from float types */

#define DTOQ15(d)   (q15_t)((d)* (double)Q15)
#define FTOQ15(f)   (q15_t)((f)* (float )Q15)

#define DTOQ31(d)   (q31_t)((d)* (double)Q31)
#define FTOQ31(f)   (q31_t)((f)* (float )Q31)

static inline q15_t q15_sat(int32_t val)
{
  if(val > Q15-1)
    {
      val = Q15 - 1;
      printf("+sat!\n");
    }

  if(val < -Q15)
    {
      val = -Q15;
      printf("-sat!\n");
    }

  return (q15_t)val;
}

/* Saturating additions */
static inline q15_t q15_add(q15_t a, q15_t b)
{
  return q15_sat((int32_t)a + (int32_t)b);
}

static inline q15_t q15_sub(q15_t a, q15_t b)
{
  return q15_sat((int32_t)a - (int32_t)b);
}

/* Multiplications */

static inline q15_t q15_mul(q15_t a, q15_t b)
{
  int32_t tmp = (int32_t)a * (int32_t)b;

  if(tmp>0)
    tmp -= (Q15>>1); /* Rounding */
  else
    tmp += (Q15>>1); /* Rounding */

  return q15_sat(tmp >> Q15BITS);
}

static inline q31_t q31_mul(q31_t a, q31_t b)
{
  return ((int64_t)a * (int64_t)b) >> Q31BITS;
}

/* Complex multiplications */

static inline void q15_cmul(q15_t *dr, q15_t *di, q15_t ar, q15_t ai, q15_t br, q15_t bi)
{
  q15_t tr = q15_add(q15_mul(ar, br), -q15_mul(ai, bi));
  q15_t ti = q15_add(q15_mul(ar, bi),  q15_mul(br, ai));
  *dr = tr;
  *di = ti;
}

#endif /* FXPMATH__H */
