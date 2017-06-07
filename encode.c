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

/* main program to run the encoder */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "c2enc.h"

/* Read RAW input file, format is 8000 Hz Mono, int16_t */

#define NSAMPLES CODEC2_INPUTSAMPLES
#define BUFSIZE (NSAMPLES * sizeof(int16_t))


int main(int argc, char **argv)
{
  int fd;
  struct c2enc_context_s ctx;
  uint16_t *buf = NULL;
  int ret = 0;

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

      printf("Managed %d samples\n", c2enc_write(&ctx, buf, NSAMPLES) );
    }
  while(ret>0);

retclose:
  close(fd);

retfree:
  free(buf);
  return ret;
}

