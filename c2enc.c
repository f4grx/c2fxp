#include <stdint.h>
#include <stdio.h>

#include "c2enc.h"

int c2enc_init(struct c2enc_context_s *ctx)
{
  return 0;
}

int c2enc_process(struct c2enc_context_s *ctx, uint8_t *buf, uint32_t nsamples)
{
  printf("process, len=%d\n",nsamples);
}

