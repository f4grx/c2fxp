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

/* codec2 decoder implementation */

#include <stdint.h>
#include <stdio.h>

#include "c2fxp.h"

int c2dec_init(struct c2dec_context_s *ctx)
{
  return 0;
}

int c2dec_write(struct c2dec_context_s *ctx, uint8_t *buf, uint32_t nsamples)
{
  printf("c2dec process, len=%d\n",nsamples);
}

