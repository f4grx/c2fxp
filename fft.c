/*
 * fft - fixed point Fast Fourier Transform
 * Copyright (C) 2017  Sebastien F4GRX <f4grx@f4grx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdio.h>

#include "fxpmath.h"

/* log2 */
/* http://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers */
/* on ARM use CLZ for arch > armv5 : return ( 31 - __builtin_clz(bb) ); or return  __builtin_clz(bb & -bb)  ^ 31; */

static const uint32_t tab32[32] =
{
   0,  9,  1, 10, 13, 21,  2, 29,
  11, 14, 16, 18, 22, 25,  3, 30,
   8, 12, 20, 28, 15, 17, 24,  7,
  19, 27, 23,  6, 26,  5,  4, 31
};

/* ========================================================================== */
uint32_t log2_32(uint32_t value)
{
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return tab32[(uint32_t)(value*0x07C4ACDD) >> 27];
}


/* ========================================================================== */
static inline void q15_swap2(q15_t *arr1, q15_t *arr2, int a, int b)
{
  q15_t x;
  x = arr1[a]; arr1[a] = arr1[b]; arr1[b] = x;
  x = arr2[a]; arr2[a] = arr2[b]; arr2[b] = x;
}

static inline void q31_swap2(q31_t *arr1, q31_t *arr2, int a, int b)
{
  q31_t x;
  x = arr1[a]; arr1[a] = arr1[b]; arr1[b] = x;
  x = arr2[a]; arr2[a] = arr2[b]; arr2[b] = x;
}

/* ========================================================================== */
/* Fast Bit reversal */
/* A FAST RECURSIVE BIT-REVERSAL ALGORITHM
 * Jechang Jeong and William J. Williams, doi:10.1109@ICASSP.1990.115695
 */
void q15_bitreverse2(q15_t *data1, q15_t *data2, int m)
{
  //int br[256]; //enough for 131072 points (m=17, m2=8)
  //int br[128]; //enough for 32768 points (m=15, m2=7)
  //int br[64]; //enough for 8192 points (m=13, m2=6)
  int br[32]; //enough for 2048 points (m=11, m2=5)
  int m2,c,odd,offset,b_size,i,j,k;
  m2 = m >> 1;
  if(m2>5) return; //Error

  c  = 1 << m2;
  odd = 0;
  if(m != m2 << 1)
    {
      odd = 1;
    }
  offset = 1 << (m - 1);
  b_size = 2;
  br[0]=0;
  br[1]=offset;
  q15_swap2(data1, data2, 1, offset);
  if(odd)
    {
      q15_swap2(data1, data2, 1 + c, offset + c);
    }
  while(b_size < c)
    {
      offset >>= 1;
      for(i = b_size; i < (b_size << 1); i++)
        {
          k = br[i - b_size] + offset;
          br[i]=k;
          q15_swap2(data1, data2, i, k);
          if(odd)
            {
              q15_swap2(data1, data2, i+c, k+c);
            }
          for(j = 1; j < i; j++)
            {
              q15_swap2(data1, data2, i + br[j], k + j);
              if(odd)
                {
                  q15_swap2(data1, data2, i + br[j] + c, k + j + c);
                }
            }
        }
      b_size <<= 1;
    }
}

void q31_bitreverse2(q31_t *data1, q31_t *data2, int m)
{
  //int br[256]; //enough for 131072 points (m=17, m2=8)
  //int br[128]; //enough for 32768 points (m=15, m2=7)
  //int br[64]; //enough for 8192 points (m=13, m2=6)
  int br[32]; //enough for 2048 points (m=11, m2=5)
  int m2,c,odd,offset,b_size,i,j,k;
  m2 = m >> 1;
  if(m2>5) return; //Error

  c  = 1 << m2;
  odd = 0;
  if(m != m2 << 1)
    {
      odd = 1;
    }
  offset = 1 << (m - 1);
  b_size = 2;
  br[0]=0;
  br[1]=offset;
  q31_swap2(data1, data2, 1, offset);
  if(odd)
    {
      q31_swap2(data1, data2, 1 + c, offset + c);
    }
  while(b_size < c)
    {
      offset >>= 1;
      for(i = b_size; i < (b_size << 1); i++)
        {
          k = br[i - b_size] + offset;
          br[i]=k;
          q31_swap2(data1, data2, i, k);
          if(odd)
            {
              q31_swap2(data1, data2, i+c, k+c);
            }
          for(j = 1; j < i; j++)
            {
              q31_swap2(data1, data2, i + br[j], k + j);
              if(odd)
                {
                  q31_swap2(data1, data2, i + br[j] + c, k + j + c);
                }
            }
        }
      b_size <<= 1;
    }
}

/* ========================================================================== */
#include <math.h>
void cordicfq15(q15_t *vcos, q15_t *vsin, float angle) {
    *vcos = FTOQ15(cos(angle));
    *vsin = FTOQ15(sin(angle));
}

void cordicfq31(q31_t *vcos, q31_t *vsin, float angle) {
    *vcos = FTOQ31(cos(angle));
    *vsin = FTOQ31(sin(angle));
}

/* ========================================================================== */
/* naive implementation (wikipedia algorithm, with some improvements) */
/* fixed point conversion needs data scaling, see:
 * https://fr.mathworks.com/help/fixedpoint/ug/convert-fast-fourier-transform-fft-to-fixed-point.html */
int q15_fft(q15_t *datar, q15_t *datai, uint32_t n)
{
  uint32_t s;
  uint32_t m,m2;
  uint32_t k;
  uint32_t j;
  uint32_t rounds = log2_32(n);

  q31_t wmr, wmi; //complex twiddle factor
  q31_t wr, wi;
  q31_t tr, ti;
  q15_t ur, ui;

  q15_bitreverse2(datar, datai, rounds);

  for(s=1; s<=rounds; s++)
    {
      float angle;
      m = 1 << s;
      m2 = m >> 1;
      angle = -2.0*M_PI/(float)m;
      cordicfq31(&wmr, &wmi, angle);
      for(k = 0; k < n; k += m)
        {
          wr=FTOQ31(0.9); wi=0; /* 0.5 + 0. j */

          for(j = 0 ; j < m2; j++)
            {
              q31_cmul(&tr,&ti, wr,wi, Q15TOQ31(datar[k + j + m2]), Q15TOQ31(datai[k + j + m2]));

              ur = datar[k + j];
              ui = datai[k + j];

              datar[k + j] = q15_add(ur, Q31TOQ15(tr));
              datai[k + j] = q15_add(ui, Q31TOQ15(ti));

              datar[k + j + m2] = q15_sub(ur, Q31TOQ15(tr));
              datai[k + j + m2] = q15_sub(ui, Q31TOQ15(ti));

            q31_cmul(&wr,&wi, wr,wi, wmr,wmi);
            }
        }
    }

  return 0;
}

int q31_fft(q31_t *datar, q31_t *datai, uint32_t n)
{
  uint32_t s;
  uint32_t m,m2;
  uint32_t k;
  uint32_t j;
  uint32_t rounds = log2_32(n);

  q31_t wmr, wmi; //complex twiddle factor
  q31_t wr, wi;
  q31_t tr, ti;
  q31_t ur, ui;

  q31_bitreverse2(datar, datai, rounds);

  for(s=1; s<=rounds; s++)
    {
      float angle;
      m = 1 << s;
      m2 = m >> 1;
      angle = -2.0*M_PI/(float)m;
      cordicfq31(&wmr, &wmi, angle);
      for(k = 0; k < n; k += m)
        {
          wr=FTOQ31(0.9); wi=0; /* 0.5 + 0. j */

          for(j = 0 ; j < m2; j++)
            {
              q31_cmul(&tr,&ti, wr,wi, datar[k + j + m2], datai[k + j + m2]);

              ur = datar[k + j];
              ui = datai[k + j];

              datar[k + j] = q31_add(ur, tr);
              datai[k + j] = q31_add(ui, ti);

              datar[k + j + m2] = q31_sub(ur, tr);
              datai[k + j + m2] = q31_sub(ui, ti);

            q31_cmul(&wr,&wi, wr,wi, wmr,wmi);
            }
        }
    }

  return 0;
}
#ifdef TEST
#define N 8

int main(int argc, char **argv)
{
  q15_t re[N];
  q15_t im[N];
  int i;

  re[0] = FTOQ15(1/8.0); re[1]=FTOQ15(2/8.0); re[2]=FTOQ15(3/8.0); re[3]=FTOQ15(4/8.0);
  re[4] = FTOQ15(5/8.0); re[5]=FTOQ15(6/8.0); re[6]=FTOQ15(7/8.0); re[7]=FTOQ15(8/8.0);
  im[0] = 0; im[1]=0; im[2]=0; im[3]=0; im[4] = 0; im[5]=0; im[6]=0; im[7]=0;

  printf("input\n");
  for(i=0;i<N;i++) printf("%f%+fj\n",Q15TOF(re[i]),Q15TOF(im[i]));

  q15_fft(re,im,N);

  printf("output\n");
  for(i=0;i<N;i++) printf("%f%+fj\n",Q15TOF(re[i])*N,Q15TOF(im[i])*N);
}
#endif


