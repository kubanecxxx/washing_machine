#ifndef INPUTS_H
#define INPUTS_H

#include "ch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INPUT_NUMBER 9

typedef union
{
    struct
    {
        uint8_t door : 1;
        uint8_t fuse_zbytek:1;
        uint8_t fuse_motor:1;
        uint8_t fuse_heat:1;
        uint8_t low_level: 1;
        uint8_t high_level: 1;
        uint8_t temperature : 1;
        uint8_t nezdimat: 1;
        uint8_t enc_switch :1;        
    } b;
    uint16_t w;
} inputs_t;


extern inputs_t inputs;
void inputs_read(void);
void inputs_init(void);

#ifdef __cplusplus
}
#endif

#endif // INPUTS_H
