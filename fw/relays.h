#ifndef RELAYS_H
#define RELAYS_H

#include "ch.h"
#include "hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RELAY_COUNT 7

typedef struct
{
    GPIO_TypeDef * port;
    uint32_t pin;
} port_t;

extern const port_t relays[RELAY_COUNT];

typedef union
{
  struct
  {   
    uint8_t doorlock : 1;
    uint8_t valve : 1;
    uint8_t pump : 1;
    uint8_t motor_fast : 1;
    uint8_t motor_slow_l : 1;
    uint8_t motor_slow_r : 1;
    uint8_t heater :1;
    uint8_t res:1;
  }u;
  uint8_t status;
} relay_status_t;

typedef enum
{
    LEFT, RIGHT, STOP
} direction_t;

typedef enum
{
    LOW_SPEED, HIGH_SPEED
} speed_t;

void relay_init(void);
void relay_writeall();

void relay_stop_motor(void);
void relay_start_motor(direction_t direction, speed_t speed);

extern relay_status_t outputs;

#ifdef __cplusplus
}
#endif

#endif // RELAYS_H
