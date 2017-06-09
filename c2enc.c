/*
 * c2fxp - codec2 fixed point encoder/decoder.
 * Copyright (C) 2017  Sebastien F4GRX <f4grx@f4grx.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* codec2 encoder implementation */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "fxpmath.h"
#include "fft.h"
#include "c2fxp.h"

/* notch filter parameter, 0.95 << 16 */
#define COEFF 31130

/* 48 tap 600Hz low pass FIR filter coefficients */
/* Converted from original codec2 implementation */
static const int16_t nlpfir[48] =
{
   -238,  -243,  -204,   -93,   121,   441,
    817,  1134,  1233,   948,   177, -1062,
  -2580, -4011, -4863, -4611, -2823,   710,
   5881, 12236, 19021, 25301, 30137, 32767,
  32767, 30137, 25301, 19021, 12236,  5881,
    710, -2823, -4611, -4863, -4011, -2580,
  -1062,   177,   948,  1233,  1134,   817,
    441,   121,   -93,  -204,  -243,  -238,
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
  q15_t tmp;

  /* Square the last samples */

  for(i=240;i<320;i++)
    {
      /* Samples are 16-bit signed, mask the sign bit before mult */
      ctx->nlpsq[i] = q15_mul(ctx->input[i], ctx->input[i]);
    }

  /* Notch filter at DC the last samples */

  for(i=240; i<320; i++)
    {
      tmp  = ctx->nlpsq[i] - ctx->nlpmemx;
      tmp += q15_mul(COEFF, ctx->nlpmemy);
      ctx->nlpmemx = ctx->nlpsq[i];
      ctx->nlpmemy = tmp;
    }

  /* Low pass FIR the last samples */

  for(i=240; i<320; i++)
    {
      for(j=0; j<NLPFIRCOUNT-1; j++)
        {
          ctx->nlpmemfir[j] = ctx->nlpmemfir[j+1];
        }
      ctx->nlpmemfir[NLPFIRCOUNT-1] = ctx->nlpsq[i];

      tmp = 0.0;
      for(j=0; j<NLPFIRCOUNT; j++)
        {
          tmp += q15_mul(ctx->nlpmemfir[j], nlpfir[j]);
        }
      ctx->nlpsq[i] = tmp;
    }

  /* Decimation for ALL samples. This means that the result is an overlapped analysis. */

  for(i=0; i<64; i++)
    {
      ctx->nlpfftr[i] = q15_mul(ctx->nlpsq[i*5], nlpwin[i]);
      ctx->nlpffti[i] = 0; /* while we're here, zero the imaginary part*/
    }

  /* Padding before FFT */

  for(i=64; i<CODEC2_FFTSAMPLES; i++)
    {
      ctx->nlpfftr[i] = 0;
      ctx->nlpffti[i] = 0; /* while we're here, zero the imaginary part*/
    }

  /* FFT of filtered squared samples */
  q15_fft(ctx->nlpfftr, ctx->nlpffti, CODEC2_FFTSAMPLES);

  /* Find global peak */

  /* Post process using the sub-multiples method (MBE is not used) */

  /* we have best_f0 */
}

/* ========================================================================== */
/*
 * Encode a batch of input speech samples.
 * buf - pointer to an array of 80 signed 16-bit numbers
 */
static int c2enc_process_samples(struct c2enc_context_s *ctx, int16_t *buf)
{
  printf("process a block\n");

  /* Second step: add these samples to the end of the rolling input buffer */

  memmove(ctx->input, ctx->input+CODEC2_INPUTSAMPLES, 3*CODEC2_INPUTSAMPLES);
  memcpy(ctx->input + 3*CODEC2_INPUTSAMPLES, buf, CODEC2_INPUTSAMPLES);

  /* Run the non linear pitch estimation algorithm */

  c2enc_nlp(ctx);

  return 0;
}

/* ========================================================================== */
/*
 * Initialize the encoder
 */
int c2enc_init(struct c2enc_context_s *ctx)
{
  int i;

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

