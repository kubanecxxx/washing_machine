#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

void rotenc_init();
void rotenc_task();
uint8_t rotenc_get(uint8_t * value, int8_t * dir);

#ifdef __cplusplus
}
#endif

#endif // ROTARY_ENCODER_H
