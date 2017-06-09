/* fft computations */
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

uint32_t log2_32(uint32_t value)
{
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return tab32[(uint32_t)(value*0x07C4ACDD) >> 27];
}

/* Fast Bit reversal */
/* A FAST RECURSIVE BIT-REVERSAL ALGORITHM' Jechang Jeong and William J. Williams, doi:10.1109@ICASSP.1990.115695 */

/* same for 2 arrays simultaneously */
static inline void q15_swap2(q15_t *arr1, q15_t *arr2, int a, int b) {
  q15_t x;
  x = arr1[a]; arr1[a] = arr1[b]; arr1[b] = x;
  x = arr2[a]; arr2[a] = arr2[b]; arr2[b] = x;
}

void q15_bitreverse2(q15_t *data1, q15_t *data2, int m)
{
  //int br[256]; //enough for 131072 points (m=17, m2=8)
  //int br[128]; //enough for 32768 points (m=15, m2=7)
  int br[64]; //enough for 8192 points (m=13, m2=6)
  int m2,c,odd,offset,b_size,i,j,k;
  m2 = m >> 1;
  if(m2>7) return; //Error

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

/* naive float implementation (wikipedia algorithm, with some improvements) */
#include <math.h>

void cordicfq15(q15_t *vcos, q15_t *vsin, float angle) {
    *vcos = FTOQ15(cos(angle));
    *vsin = FTOQ15(sin(angle));
}

#define cpmulq15(dr,di, ar,ai, br,bi) do { \
  q15_t __tr__ = q15_mul(ar, br) - q15_mul(ai, bi); \
  q15_t __ti__ = q15_mul(ar, bi) + q15_mul(br, ai); \
  dr = __tr__; \
  di = __ti__; \
} while(0)

int q15_fft(q15_t *datar, q15_t *datai, uint32_t n)
{
  uint32_t s;
  uint32_t m,m2;
  uint32_t k;
  uint32_t j;
  uint32_t rounds = log2_32(n);

  q15_t wmr, wmi; //complex twiddle factor
  q15_t wr, wi;
  q15_t tr, ti;
  q15_t ur, ui;

  //printf("fft %d points, log2=%d\n",n,rounds);
  q15_bitreverse2(datar, datai, rounds);

  //printf("data bitreversed\n");

  for(s=1; s<=rounds; s++)
    {
      float angle;
      m = 1 << s;
      m2 = m >> 1;
      angle = -2.0*M_PI/(float)m;
      cordicfq15(&wmr, &wmi, angle);
      //printf("round %d m=%d angle=%f wm=%f%+fj\n",s,m,angle, Q15TOF(wmr),Q15TOF(wmi) );

      for(k = 0; k < n; k += m)
        {
          //printf("  k=%d\n",k);
          wr=1; wi=0;

          for(j = 0 ; j < m2; j++)
            {
              //printf("    j=%d w=%f%+fj, indexes %d and %d\n",j, Q15TOF(wr),Q15TOF(wi), k+j+m2, k+j);

              cpmulq15(tr,ti, wr,wi, datar[k + j + m2], datai[k + j + m2]);

              ur = datar[k + j];
              ui = datai[k + j];

              datar[k + j] = ur + tr;
              datai[k + j] = ui + ti;

              datar[k + j + m2] = ur - tr;
              datai[k + j + m2] = ui - ti;

              //printf("      old w %f%+fj\n", wr,wi);
              cpmulq15(wr,wi, wr,wi, wmr,wmi);
              //printf("         wm %f%+fj\n", wmr,wmi);
              //printf("      new w %f%+fj\n", wr,wi);
            }
        }
    }
  //printf("done\n");
  return 0;
}

