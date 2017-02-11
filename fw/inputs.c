#include "ch.h"
#include "hal.h"
#include "inputs.h"

typedef struct
{
    GPIO_TypeDef * port;
    uint8_t pin;
} gpio_t;

static const gpio_t _inputs[INPUT_NUMBER] =
{
    {GPIOA, 3}, //door locked
    {GPIOA, 4}, //fuse zbytek
    {GPIOA, 5}, //fuse motor
    {GPIOA, 6}, //fuse heat
    {GPIOB, 13}, //low level
    {GPIOB, 14}, //high level
    {GPIOB, 15}, //temperature
    {GPIOA, 8},  //nezdimat
    {GPIOA, 2} //enkoder switch
};

inputs_t inputs;
uint8_t w, v;

void inputs_read(void)
{
    static uint8_t old[INPUT_NUMBER];
    uint8_t temp;
    uint8_t i ;
    for (i = 0 ; i < INPUT_NUMBER ; i++)
    {
        temp = palReadPad(_inputs[i].port, _inputs[i].pin);

        if (old[i] == temp)
        {
            inputs.w &= ~(1<<i);
            if (temp)
                inputs.w |= (1<<i);
        }



        old[i] = temp;

    }


}

void inputs_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
    uint8_t i;
    for (i = 0; i < INPUT_NUMBER; i++)
        palSetPadMode(_inputs[i].port, _inputs[i].pin, PAL_MODE_INPUT_PULLDOWN);

}
