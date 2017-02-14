/*
 ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "ch.h"
#include "hal.h"
//#include "scheduler.hpp"
#include "relays.h"
#include "LCD_ctrl.h"
#include "rotary_encoder.h"
#include "scheduler.h"
#include "inputs.h"
#include "gui.h"
#include "statemachine.h"

systime_t sysTime;

/*
 * Application entry point.
 */

void mcu_init(void);

uint8_t changed = 0;
char lcd_data[100];


void lcd_task(void)
{
    if (changed)
    {
        changed = 0;

        LCD_xy(0,0);
        LCD_puts(lcd_data);
    }
}

static delay_t dt;
void blik(void * a);
void test(void);

static THD_WORKING_AREA(gui_wa, 512);

Gui _gui;


void gui_thread(void *)
{
    LCD_xy(0,0);
    LCD_puts("Dobry den");
    chThdSleepSeconds(2);    
    while(TRUE)
    {
        rotenc_task();
        _gui.task();
    }
}


void enable_watchdog(void)
{
    //setup watchdog
    DBGMCU->CR |= DBGMCU_CR_DBG_IWDG_STOP;
    IWDG->KR = 0x5555;
    IWDG->PR = 6;
    IWDG->RLR = 0xFFF;
    IWDG->KR = 0xCCCC;
}



void clear_watchdog(void)
{
    IWDG->KR = 0xAAAA;
}

int main(void)
{
    halInit();
    chSysInit();

    mcu_init();
    enable_watchdog();

    //test();

    RCC->APB1ENR |= RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
    PWR->CR |= PWR_CR_DBP;

    statemachine _state_machine(params, _gui);


    shFillStruct(&dt, blik, NULL, MS2ST(500), PERIODIC);
    shRegisterStruct(&dt);


    chThdCreateStatic(&gui_wa,sizeof(gui_wa), LOWPRIO, gui_thread, NULL);

	while (TRUE)
	{        
        chThdSleepMilliseconds(5);
        inputs_read();

        clear_watchdog();
        _state_machine.task();
        sysTime = chVTGetSystemTime();
        shPlay();
        relay_writeall();

        //lcd_task();
	}

	return 1;
}

void mcu_init(void)
{
    relay_init();
    outputs.status = 0;
    relay_writeall();

    inputs_init();
    rotenc_init();



    LCD_init();
    LCD_init();


}

void blik(void * a)
{
    return;
    if (!_gui.atMainScreen())
        outputs_forced.u.motor_fast ^= 1;
}

void test(void)
{

    while(TRUE)
    {
        for (unsigned char i = 0; i < 8; i++)
        {
            LCD_clear();
            uint8_t a = i;
            LCD_xy(a,0);
            LCD_puts("Hi!");
            chThdSleepMilliseconds(500);
        }
        for (uint8_t i = 8 ; i > 0 ; i--)
        {
            LCD_clear();
            uint8_t a = i;
            LCD_xy(a,0);
            LCD_puts("Hi!");
            chThdSleepMilliseconds(500);
        }
    }

    while (TRUE)
    {
        inputs_read();
        outputs.u.doorlock = inputs.b.fuse_heat;
        outputs.u.heater = inputs.b.fuse_motor;
        outputs.u.motor_fast = inputs.b.fuse_zbytek;
        outputs.u.valve = inputs.b.door;
        relay_writeall();
    }

    while (TRUE)
    {
        for ( uint8_t i = 0; i < RELAY_COUNT; i++)
        {
            outputs.status = 1 << i;
            relay_writeall();

            chThdSleepMilliseconds(500);
        }
    }
}

#ifdef __cplusplus

extern "C" void __cxa_pure_virtual(void)
{
    asm("bkpt");
    while(1);
}

#endif

