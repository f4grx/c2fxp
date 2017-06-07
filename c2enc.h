#ifndef __C2ENC__H__
#define __C2ENC__H__

/* This is the number of samples that must be fed to the encoder */
#define CODEC2_INPUTSAMPLES 80

struct c2enc_context_s
{
  uint16_t samples[4*CODEC2_INPUTSAMPLES];
  int dummy;
};

struct c2enc_context_s;

int c2enc_init(struct c2enc_context_s *ctx);
int c2enc_write(struct c2enc_context_s *ctx, int16_t *samples, uint32_t nsamples);

#endif /* __C2ENC__H__ */

