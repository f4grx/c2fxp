/* This file is public domain. */

#ifndef FXPMATH__H
#define FXPMATH__H

#include <stdint.h>

/* Types */

typedef int16_t q15_t;
typedef int32_t q31_t;

/* Signed Fixed point types with 1 sign bit and N-1 fractional bits */

#define Q15BITS 15
#define Q31BITS 31

#define Q15 (1    << Q15BITS)
#define Q31 (1LU  << Q31BITS)

/* Conversions to float types */

#define Q15TOD(q15) (double)((q15)/(double)Q15)
#define Q15TOF(q15) (float )((q15)/(float )Q15)

#define Q31TOD(q31) (double)((q31)/(double)Q31)
#define Q31TOF(q31) (float )((q31)/(float )Q31)

/* Conversion from float types */

#define DTOQ15(d)   (q15_t)((d)* (double)Q15)
#define FTOQ15(f)   (q15_t)((f)* (float )Q15)

#define DTOQ31(d)   (q31_t)((d)* (double)Q31)
#define FTOQ31(f)   (q31_t)((f)* (float )Q31)

/* Conversion between types */

#define Q15TOQ31(v) ((v) << (Q31BITS-Q15BITS))
#define Q31TOQ15(v) ((v) >> (Q31BITS-Q15BITS))

/* Saturation */

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

static inline q31_t q31_sat(int64_t val)
{
  if(val > (int64_t)(Q31-1))
    {
      val = (int64_t)(Q31 - 1);
      printf("+sat!\n");
    }

  if(val < (int64_t)-Q31)
    {
      val = (int64_t)-Q31;
      printf("-sat!\n");
    }

  return (q31_t)val;
}

/* Saturating addition */

static inline q15_t q15_add(q15_t a, q15_t b)
{
  return q15_sat((int32_t)a + (int32_t)b);
}

static inline q31_t q31_add(q31_t a, q31_t b)
{
  return q31_sat((int64_t)a + (int64_t)b);
}

/* Saturating subtraction */

static inline q15_t q15_sub(q15_t a, q15_t b)
{
  return q15_sat((int32_t)a - (int32_t)b);
}

static inline q31_t q31_sub(q31_t a, q31_t b)
{
  return q31_sat((int64_t)a - (int64_t)b);
}

/* Multiplication */

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
  int64_t tmp = (int64_t)a * (int64_t)b;

  if(tmp>0)
    tmp -= (Q31>>1); /* Rounding */
  else
    tmp += (Q31>>1); /* Rounding */

  return q31_sat(tmp >> Q31BITS);
}

/* Complex multiplications */

static inline void q15_cmul(q15_t *dr, q15_t *di, q15_t ar, q15_t ai, q15_t br, q15_t bi)
{
  q15_t tr = q15_sub(q15_mul(ar, br), q15_mul(ai, bi));
  q15_t ti = q15_add(q15_mul(ar, bi), q15_mul(br, ai));
  *dr = tr;
  *di = ti;
}

#endif /* FXPMATH__H */

