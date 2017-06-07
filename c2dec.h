#ifndef __C2DEC__H__
#define __C2DEC__H__

struct c2dec_context_s
{
  int dummy;
};

struct c2dec_context_s;

int c2dec_init(struct c2dec_context_s *ctx);
int c2dec_process(struct c2dec_context_s *ctx, uint8_t *buf, uint32_t nsamples);

#endif /* __C2DEC__H__ */

