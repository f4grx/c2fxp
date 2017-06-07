#include <stdint.h>
#include <stdio.h>

#include "c2dec.h"

int c2dec_init(struct c2dec_context_s *ctx)
{
  return 0;
}

int c2dec_process(struct c2dec_context_s *ctx, uint8_t *buf, uint32_t nsamples)
{
  printf("c2dec process, len=%d\n",nsamples);
}

