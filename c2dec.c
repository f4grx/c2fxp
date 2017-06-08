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

