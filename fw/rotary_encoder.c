#include "ch.h"
#include "hal.h"
#include "rotary_encoder.h"
#include "chsprintf.h"
#include "scheduler.h"

TIM_TypeDef * timer = TIM2;
static delay_t delay;

static void _cb(void * arg);

void rotenc_init()
{

    palSetPadMode(GPIOA, 0, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOA, 1, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    timer->SMCR |= TIM_SMCR_SMS_0;
    timer->SMCR |= TIM_SMCR_SMS_1;
    //TIM2->CCER |= TIM_CCER_CC1E
    timer->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_IC1F;
    timer->CCMR1 |= TIM_CCMR1_CC2S_0 | TIM_CCMR1_IC2F;
    timer->ARR = 0xff;
    timer->CR1 |= TIM_CR1_CEN;

    //TIM2->CCMR1 =

    shFillStruct(&delay, _cb, NULL, MS2ST(50), ONCE);
}

static uint8_t _value;
static uint8_t _changed;
static int8_t _dir;
static uint8_t timeout;

void _cb(void * arg)
{
    timeout = 0;
}

void rotenc_task()
{
    static uint8_t old;
    uint8_t d = timer->CNT;

    d >>= 2;
    if (d != old && !timeout)
    {
        _changed = 1;
        timeout = 1;
        _value = d;
        _dir = (timer->CR1 & 0b10000);
        if (!_dir)
            _dir = -1;
    }

    old = d;
}


uint8_t rotenc_get(uint8_t * val, int8_t * dir)
{
    if (_changed && val)
    {
        *val = _value;
        _changed = 0;
        shRegisterStruct(&delay);

        if (dir)
        {
            *dir = _dir ;
            //_dir = 0;
        }

        return 1;
    }

    return 0;
}
