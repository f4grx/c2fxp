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

#include "c2enc.h"

static int c2enc_process_samples(struct c2enc_context_s *ctx, int16_t *buf)
{
  printf("process a block\n");
}

int c2enc_init(struct c2enc_context_s *ctx)
{
  return 0;
}

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

