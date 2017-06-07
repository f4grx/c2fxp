#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "c2enc.h"

/* Read RAW input file, format is 8000 Hz Mono, int16_t */

#define NSAMPLES 512
#define BUFSIZE (NSAMPLES * sizeof(int16_t))


int main(int argc, char **argv)
{
  int fd;
  struct c2enc_context_s ctx;
  uint8_t *buf = NULL;
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
          memset(buf+ret, 0, BUFSIZE - ret); /* pad */
        }

      c2enc_process(&ctx, buf, BUFSIZE/2);
    }
  while(ret>0);

retclose:
  close(fd);

retfree:
  free(buf);
  return ret;
}

