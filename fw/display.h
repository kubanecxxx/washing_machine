#ifndef DISPLAY_H
#define DISPLAY_H

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

I2CDriver * display_init(void);
#define PCF_ADDRESS 0b0111111


#ifdef __cplusplus
}
#endif


#endif // DISPLAY_H
