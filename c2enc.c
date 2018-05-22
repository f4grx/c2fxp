/*
 * c2fxp - codec2 fixed point encoder/decoder.
 * Copyright (C) 2017  Sebastien F4GRX <f4grx@f4grx.net>
 * Based on original code by David Rowe <david@rowetel.com>
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

/* codec2 encoder implementation */

#define FRAME 88

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fxpmath.h"
#include "fft.h"
#include "c2fxp.h"

/* notch filter parameter, 0.95*/
#define COEFF DTOQ31(0.95)

/* 48 tap 600Hz low pass FIR filter coefficients */
/* Converted from original codec2 implementation */
static const q15_t nlpfir[48] =
{
   -35,  -36,  -30,  -14,   18,   66,  121,  169,
   183,  141,   26, -158, -384, -596, -723, -686,
  -420,  106,  874, 1819, 2828, 3762, 4481, 4872,
  4872, 4481, 3762, 2828, 1819,  874,  106, -420,
  -686, -723, -596, -384, -158,   26,  141,  183,
   169,  121,   66,   18,  -14,  -30,  -36,  -35,
};

static const q31_t nlpfirq31[48] =
{
   -2323174,  -2364024,  -1992196,   -908159,
    1181850,   4301377,   7958254,  11048678,
   12009731,   9242073,   1724097, -10351861,
  -25138036, -39082645, -47384832, -44926668,
  -27506755,   6915923,  57303048, 119229632,
  185340603, 246535246, 293651338, 319278334,
  319278334, 293651338, 246535246, 185340603,
  119229632,  57303048,   6915923, -27506755,
  -44926668, -47384832, -39082645, -25138036,
  -10351861,   1724097,   9242073,  12009731,
   11048678,   7958254,   4301377,   1181850,
    -908159,  -1992196,  -2364024,  -2323174,
};

#define NLPFIRCOUNT (sizeof(nlpfir)/sizeof(nlpfir[0]))

/* 64-bins Hanning window, values of:
 * nlp->w[i] = 0.5 - 0.5*cosf(2*PI*i/(m/DEC-1));
 * Stored value is round(32768 * hanning). */
static const int16_t nlpwin[64] =
{
      0 ,    81 ,   325 ,  728 , 1287  , 1995 ,  2847 ,  3833 ,
   4944 ,  6169 ,  7495 , 8909 ,10398  ,11947 , 13539 , 15160 ,
  16792 , 18421 , 20030 ,21602 ,23123  ,24576 , 25948 , 27225 ,
  28394 , 29444 , 30364 ,31145 ,31780  ,32261 , 32585 , 32748 ,
  32748 , 32585 , 32261 ,31780 ,31145  ,30364 , 29444 , 28394 ,
  27225 , 25948 , 24576 ,23123 ,21602  ,20030 , 18421 , 16792 ,
  15160 , 13539 , 11947 ,10398 , 8909  , 7495 ,  6169 ,  4944 ,
   3833 ,  2847 ,  1995 , 1287 ,  728  ,  325 ,    81 ,     0
};

/* ========================================================================== */
/*
 * Non linear pitch prediction. This algorithm extracts the fundamental
 * frequency of the speech signal. To do that a number of steps are required:
 * - preprocessing: DC notch and 600 Hz LPF (48-tap FIR)
 * - decimation by 5 (this gives 64 samples)
 * - padding to 512 samples by appending 448 zero samples
 * - Perform DFT
 * - basic estimation,
 * - SM post processing,
 * - coarse then fine refinements
 * The algorithm is based on the DFT of the squared speech signal.
 */
static void c2enc_nlp(struct c2enc_context_s *ctx)
{
  int i,j;
  q31_t ntmp;
  q15_t tmp,gmax;
  int gmax_bin;
  uint32_t scale = 512;
  uint32_t acc;

  /* Square the last samples */

  for(i=240;i<320;i++)
    {
      /* Samples are 16-bit signed, mask the sign bit before mult */
      ctx->nlpsq[i] = q15_mul(ctx->input[i], ctx->input[i]);
    }

  /* Notch filter at DC the last samples. This is an IIR filter, it
   * requires more precision to avoid bias. */

  for(i=240; i<320; i++)
    {
      ntmp           = q31_sub(Q15TOQ31(ctx->nlpsq[i]), ctx->nlpmemx);
      ntmp           = q31_add(ntmp, q31_mul(COEFF, ctx->nlpmemy));
      ctx->nlpmemx  = Q15TOQ31(ctx->nlpsq[i]);
      ctx->nlpmemy  = ntmp;
      ctx->nlpsq[i] = Q31TOQ15(ntmp);
    }

  /* Low pass FIR the last samples */

  for(i=240; i<320; i++)
    {
      for(j=0; j<NLPFIRCOUNT-1; j++)
        {
          ctx->nlpmemfir[j] = ctx->nlpmemfir[j+1];
        }
      ctx->nlpmemfir[NLPFIRCOUNT-1] = ctx->nlpsq[i];

      ntmp = 0;
      for(j=0; j<NLPFIRCOUNT; j++)
        {
          ntmp = q31_add(ntmp, q31_mul(Q15TOQ31(ctx->nlpmemfir[j]), nlpfirq31[j]));
        }
      ctx->nlpsq[i] = Q31TOQ15(ntmp);
    }

  /* Decimation, for ALL samples. This means that the result is an overlapped analysis
   * of the last 4 frames. */

  /* Fixed point FFT divides output coefficients by the number of FFT points.
   * This means that for a 512-point FFT, output coefficients will be 512 times smaller
   * than expected. We can compensate for this by pre multiplying FFT inputs by 512,
   * but this may result in clipping. To overcome that, we multiply samples by the highest
   * possible power of two that will not result in clipping. Then, we apply FFT, and we
   * finish by multiplying the FFT coefficients enough to ensure that the total scale is 512.
   */

rescale:
  acc = 0;

  for(i=0; i<64; i++)
    {
      ctx->nlpfftr[i] = q15_mul(ctx->nlpsq[i*5], nlpwin[i]) * scale;

      acc |= q15_abs(ctx->nlpfftr[i]) * scale;

      if(scale != 512) continue;
      ctx->nlpffti[i] = 0; /* while we're here, zero the imaginary part (but only once)*/
    }

  /* detect overflow during scaling */

  if(acc>>16)
    {
      if(scale>1)
        {
          scale /= 2;
          goto rescale;
        }
    }

  /* Padding before FFT */

  for(i=64; i<CODEC2_FFTSAMPLES; i++)
    {
      ctx->nlpfftr[i] = 0;
      ctx->nlpffti[i] = 0;
    }

  /* Execute FFT of filtered squared samples */

  q15_fft(ctx->nlpfftr, ctx->nlpffti, CODEC2_FFTSAMPLES);

#define F_MIN_S 50
#define F_MAX_S 400

  /* Find global peak */

  gmax = 0;
  gmax_bin = CODEC2_FFTSAMPLES*5*F_MIN_S/8000;

  for(i=CODEC2_FFTSAMPLES*5*F_MIN_S/8000; i<=CODEC2_FFTSAMPLES*5*F_MAX_S/8000; i++)
    {
      if (ctx->nlpfftr[i] > gmax)
        {
          gmax = ctx->nlpfftr[i];
          gmax_bin = i;
        }
    }

printf("%d\n", gmax_bin);

  /* Post process using the sub-multiples method (MBE is not used) */

  /* Shift samples in buffer (rolling analysis window of 4 frames) */
  for(i=0; i<240; i++)
    {
      ctx->nlpsq[i] = ctx->nlpsq[i+80];
    }

  /* we have best_f0 */
}

/* ========================================================================== */
/*
 * Encode a batch of input speech samples.
 * buf - pointer to an array of 80 signed 16-bit numbers
 */
static int c2enc_process_samples(struct c2enc_context_s *ctx, int16_t *buf)
{
  /* Second step: add these samples to the end of the rolling input buffer */

  memmove(ctx->input, ctx->input+CODEC2_INPUTSAMPLES, 3*CODEC2_INPUTSAMPLES * sizeof(int16_t));
  memcpy(ctx->input + 3*CODEC2_INPUTSAMPLES, buf, CODEC2_INPUTSAMPLES * sizeof(int16_t));

  /* Run the non linear pitch estimation algorithm */
  /* printf("----- frame %d -----\n", ctx->frame); */
  c2enc_nlp(ctx);

  ctx->frame +=1;

  return 0;
}

/* ========================================================================== */
/*
 * Initialize the encoder
 */
int c2enc_init(struct c2enc_context_s *ctx)
{
  int i;

  ctx->frame=0;

  /* Erase sample history (4 80 sample frames) */

  for(i=0;i<CODEC2_INPUTSAMPLES*4;i++)
    {
      ctx->input[i] = 0;
      ctx->nlpsq[i] = 0;
    }

  /* Erase NLP detector variables */

  ctx->nlpmemx = 0;
  ctx->nlpmemy = 0;
  for(i=0; i<NLPFIRCOUNT; i++)
    {
      ctx->nlpmemfir[i] = 0;
    }

  return 0;
}

/* ========================================================================== */
/*
 * Write some samples to the encoder.
 * Returns: the number of samples submitted to the encoder, always a multiple
 * of 80.
 */
int c2enc_write(struct c2enc_context_s *ctx, int16_t *buf, uint32_t nsamples)
{
  uint32_t done = 0;
  while(nsamples >= CODEC2_INPUTSAMPLES)
    {
      c2enc_process_samples(ctx, buf + done);
      done += CODEC2_INPUTSAMPLES;
      nsamples -= CODEC2_INPUTSAMPLES;
    }
  return done;
}

