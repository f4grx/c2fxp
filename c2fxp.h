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

/* codec2 encoder public API */

#ifndef __C2ENC__H__
#define __C2ENC__H__

#include <stdint.h>
#include "fxpmath.h"

/*
 * The input samples are signed 16-bit numbers interpreted as fixed point
 * fractions with a value between -1 and 1.
 */

/* ========================================================================== */
/* Common stuff */

/* This is the number of samples that must be fed to the encoder */
#define CODEC2_INPUTSAMPLES 80
#define CODEC2_FFTSAMPLES 512

/* ========================================================================== */
/* Encoder stuff */

struct c2enc_context_s
{
  q15_t input[4*CODEC2_INPUTSAMPLES]; /* buffer for input samples, 4 frames */
  uint32_t frame;
  uint32_t logframe;

  /* NLP */
  q15_t nlpsq[4*CODEC2_INPUTSAMPLES]; /* buffer for squared input samples, 4 frames */
  q31_t nlpmemx, nlpmemy; /* NLP notch registers, longer precision */
  q15_t nlpmemfir[48]; /* NLP FIR filter registers */
  q15_t nlpfftr[CODEC2_FFTSAMPLES]; /* Sample buffer for FFT */
  q15_t nlpffti[CODEC2_FFTSAMPLES];
};

int c2enc_init(struct c2enc_context_s *ctx);
int c2enc_write(struct c2enc_context_s *ctx, int16_t *samples, uint32_t nsamples);

/* ========================================================================== */
/* Decoder stuff */

struct c2dec_context_s
{
  int dummy;
};

int c2dec_init(struct c2dec_context_s *ctx);
int c2dec_write(struct c2dec_context_s *ctx, uint8_t *buf, uint32_t nsamples);

#endif /* __C2ENC__H__ */

