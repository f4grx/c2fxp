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

#include "c2fxp.h"

/* 48 tap 600Hz low pass FIR filter coefficients */
/* Converted from original codec2 implementation */
static int16_t nlp_fir[48] =
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

/* ========================================================================== */
/*
 * Non linear pitch prediction. This algorithm extracts the fundamental
 * frequency of the speech signal. To do that a number of steps are required:
 * - preprocessing: DC notch and 600 Hz LPF (48-tap FIR)
 * - decimation by 5 (this gives 64 samples)
 * - padding to 512 samples by appending 448 zero samples
 * - Perform DFT
 * - basic estimation,
 * - MBE inspired post processing,
 * - coarse then fine refinements
 * The algorithm is based on the DFT of the squared speech signal.
 */
static void c2enc_nlp(struct c2enc_context_s *ctx)
{
  uint32_t buf;

  /* Square the last samples */

  /* samples are 16-bit signed, mask the sign bit before mult */
  for(i=0;i<80;i++)
    {
      buf = (ctx->input[240+i]&0x7FFF) * (ctx->input[240+i]&0x7FFF);
      buf >>= 16;
      ctx->nlpfftr[i] = buf;
      ctx->nlpffti[i] = 0; /* while we're here, zero the imaginary part*/
    }

  /* Notch filter at DC */

  /* Low pass FIR */

  /* Decimation */
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

