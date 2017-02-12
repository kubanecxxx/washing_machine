#include "relays.h"
#include "ch.h"
#include "hal.h"
#include "inputs.h"

 const  port_t relays[RELAY_COUNT] = {
    {GPIOA, 7},  // lock
    {GPIOB, 0},  // valve
    {GPIOB, 1}, // pump
    {GPIOB, 2}, // m3
    {GPIOB, 10}, // m2
    {GPIOB, 11}, // m1
    {GPIOB, 12}, // heat
};


relay_status_t outputs;
relay_status_t outputs_forced;

static void incompatible(void);

void user_unhandled_exception(void)
{
    //disable all outputs in case of failure
    outputs.status  = 0;
    relay_writeall();

    asm("bkpt");
}



static void relay_write(uint8_t number, uint8_t enable);

void relay_write(uint8_t number, uint8_t enabled) {
  if (enabled) {

    palSetPad(relays[number].port, relays[number].pin);
  } else {
    palClearPad(relays[number].port, relays[number].pin);
  }
}

void relay_stop_motor(void)
{
    incompatible();
}

void relay_start_motor(direction_t dir, speed_t speed)
{
    relay_stop_motor();

    if (speed == LOW_SPEED)
    {
        if (dir == LEFT)
        {
            outputs.u.motor_slow_l = 1;
        }
        else if (dir == RIGHT)
        {
            outputs.u.motor_slow_r = 1;
        }
    }
    else if (speed == HIGH_SPEED)
    {
        outputs.u.motor_fast = 1;
    }
}

void incompatible(void)
{
    outputs.u.motor_fast = 0;
    outputs.u.motor_slow_l =  0;
    outputs.u.motor_slow_r =  0;
}

void relay_writeall()
{
    uint8_t i;
    //check incompatible outputs    
    uint8_t a;
    a = outputs.u.motor_fast + outputs.u.motor_slow_l + outputs.u.motor_slow_r;
    if (a > 1)
    {
        //wrong
        //alarm
        incompatible();        
    }   

    uint8_t v,h;
    v = outputs.u.valve;
    h = outputs.u.heater;
    /*
    outputs.u.heater = outputs.u.heater && inputs.b.low_level &&
            !inputs.b.temperature;
    outputs.u.valve = outputs.u.valve && !inputs.b.high_level;
    */


    for (i = 0; i < RELAY_COUNT; i++) {   

      relay_write(i, (((outputs.status >> i) & 1) || ((outputs_forced.status >> i)) & 1));
    }

    outputs.u.valve = v;
    outputs.u.heater = h;
}

void relay_init(void) {
  uint8_t i;
  const port_t *p = relays;

  for (i = 0; i < RELAY_COUNT; i++) {
    palSetPadMode(p->port, p->pin, PAL_MODE_OUTPUT_PUSHPULL);
    palClearPad(p->port, p->pin);

    p++;
  }
}
