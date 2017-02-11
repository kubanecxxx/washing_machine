#include "ch.h"
#include "hal.h"
#include "display.h"

static const I2CConfig i2conf =
{ OPMODE_I2C, 50000, STD_DUTY_CYCLE };

I2CDriver * i2c = &I2CD1;



/**
 * PCA register set
 */
#define PCA_IDR         0
#define PCA_ODR         1
#define PCA_POL         2
#define PCA_DDR         3

/**
 * PCA port direction
 */
#define PCA_OUTPUT  0
#define PCA_INPUT       1

I2CDriver* display_init(void)
{
    //low level init

    palSetPadMode(GPIOB,9, PAL_MODE_INPUT_PULLDOWN);
    palSetPadMode(GPIOB,8, PAL_MODE_INPUT_PULLDOWN);

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    palSetPadMode(GPIOB, 8, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);
    palSetPadMode(GPIOB, 9, PAL_MODE_STM32_ALTERNATE_OPENDRAIN);

    AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;


    //i2c driver init
    i2cStart(i2c, &i2conf);

    return i2c;
}

void i2c_scan(I2CDriver * i2c)
{
    uint8_t buffer[2];
    uint8_t i;
    msg_t ret;
    for (i = 1 ; i < 0b1111111; i++)
    {
        ret = i2cMasterTransmitTimeout(i2c, i, buffer,2,NULL,0,  MS2ST(100));
        if(ret == 0)
        {
            asm("nop");
        }
        else
        {
            i2cStop(i2c);
            i2cStart(i2c,&i2conf);
        }
    }
}


