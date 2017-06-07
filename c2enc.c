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

