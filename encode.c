/*
 * c2fxp - codec2 fixed point encoder/decoder.
 * Copyright (C) 2017  Sebastien F4GRX <f4grx@f4grx.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* main program to run the encoder */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "c2fxp.h"

/* Read RAW input file, format is 8000 Hz Mono, int16_t */

#define NSAMPLES CODEC2_INPUTSAMPLES
#define BUFSIZE (NSAMPLES * sizeof(int16_t))

struct c2enc_context_s ctx;

/* ========================================================================== */
int main(int argc, char **argv)
{
  int fd;
  uint16_t *buf = NULL;
  int ret = 0;

  printf("sizeof(struct c2enc_context_s) = %lu\n", sizeof(struct c2enc_context_s));
  buf = malloc(BUFSIZE);

  if (!buf)
    {
      fprintf(stderr, "cannot allocate sample buffer\n");
      return 1;
    }

  fd = open(argv[1], O_RDONLY);

  if(fd<0)
    {
      fprintf(stderr, "cannot open: %s (%s)\n", argv[1], strerror(errno));
      ret = 1;
      goto retfree;
    }

  ret = c2enc_init(&ctx);
  if(ret != 0)
    {
      fprintf(stderr, "encoder init failed\n");
      ret = 1;
      goto retclose;
    }

  do
    {
      ret = read(fd, buf, BUFSIZE);
      if(ret < BUFSIZE)
        {
          memset((uint8_t*)buf+ret, 0, BUFSIZE - ret); /* pad */
        }
      c2enc_write(&ctx, buf, NSAMPLES);
//      printf("Managed %d samples\n", ret );
    }
  while(ret>0);

retclose:
  close(fd);

retfree:
  free(buf);
  return ret;
}

