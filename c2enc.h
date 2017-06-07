#ifndef __C2ENC__H__
#define __C2ENC__H__

struct c2enc_context_s
{
  int dummy;
};

struct c2enc_context_s;

int c2enc_init(struct c2enc_context_s *ctx);
int c2enc_process(struct c2enc_context_s *ctx, uint8_t *buf, uint32_t nsamples);

#endif /* __C2ENC__H__ */

