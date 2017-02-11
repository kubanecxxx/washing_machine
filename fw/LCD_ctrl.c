#include "ch.h"
#include "hal.h"

#include "LCD_ctrl.h"
#include "display.h"

//global variables
static char string[5];

#ifdef LCD_PARALEL
#define LCD_SET(LCD_BIT) bit_set(LCD_control_port,LCD_BIT)
#define LCD_CLR(LCD_BIT) bit_clr(LCD_control_port,LCD_BIT)

// LCD port defitions
#define LCD_PORT 		PORTA
#define LCD_DDR			DDRA

// LCD signals definitions
#define LCD_RS 	3
#define LCD_E	0
#define LCD_RW	1

// LCD control macros
#define LCD_control_port PORTB
#define LCD_control_ddr	DDRB

#else
static I2CDriver * i2c;
static uint8_t port;

#define LCD_RS 	0
#define LCD_E	2
#define LCD_RW	1
#define LCD_D4  4
#define LCD_D5  5
#define LCD_D6  6
#define LCD_D7  7
#define LCD_BACKLIGHT 3

inline static void refresh_port(void)
{
    i2cMasterTransmit(i2c,PCF_ADDRESS, &port, 1, NULL,0);
}

inline static void LCD_CLR(uint8_t bitnum)
{
    port &= ~(1 << bitnum);
    //refresh_port();
}


inline static void LCD_SET(uint8_t bitnum)
{
    port |= 1 << bitnum;
    //refresh_port();
}

#endif

// LCD initialisation procedure
void LCD_init(void)
{	


#ifdef LCD_PARALEL
	LCD_DDR |= 0b00001111;
	LCD_control_ddr |= 0b00001011;
	LCD_CLR(LCD_RS);
	LCD_SET(LCD_E);
    LCD_PORT = (LCD_PORT &0xF0) | 0x04; //cursor home
	LCD_CLR(LCD_E);
	_delay_ms(10);
	LCD_SET(LCD_E);
	LCD_CLR(LCD_E);
	_delay_us(200);
	LCD_SET(LCD_E);
	LCD_CLR(LCD_E);
	_delay_us(60);
#else

    //init i2c driver under
    if (!i2c)
        i2c = display_init();
    port = 0;

    LCD_SET(LCD_BACKLIGHT);
    refresh_port();
    chThdSleepMilliseconds(100);
    refresh_port();
    chThdSleepMilliseconds(100);


    LCD_CLR(LCD_RS);
    refresh_port();
    LCD_SET(LCD_E);
    refresh_port();
    port |= 0x4 << 4; //cursor home
    refresh_port();
    LCD_CLR(LCD_E);
    refresh_port();


/*
    LCD_SET(LCD_E);
    refresh_port();
    LCD_CLR(LCD_E);
    refresh_port();
*/

#endif

	LCD_wr_cmd(0x20);  //4bits, line display, 5x7 dots
	LCD_wr_cmd(0x28);  //2 lines display
	LCD_clear();		
	LCD_wr_cmd(0x08);  //display off , cursor off , blinking off 	
	LCD_wr_cmd(0x0C);  //display on
	LCD_wr_cmd(0x06);  //increment , not shifted

    LCD_xy(0,0);
    LCD_puts("beda");

    asm("nop");

}


//convert input data to bargraph and print it
void LCD_bargraph(unsigned char data){
	unsigned char i,temp;

//	data/=4;
//	LCD_cgram_load_VU();

	i=data;
	i/=5;
	while(i--)
		LCD_wr_data(5);

	temp=i;
	i=data;
	i=i%5;

	LCD_wr_data(i);

	temp=13-temp;
	while(temp--)
		LCD_wr_data(0);

}

// write string to LCD
void LCD_puts (const char *c){
	unsigned char x;

	x=*c;
	while (x!=0) {
		LCD_wr_data(*c++);
		x=*c;
	}
}

// write program memory string to LCD - save RAM (more than brutally)
void LCD_putsflash (const char *c){	
	LCD_puttitle(c,0);
}

void LCD_puttitle(const char *c,unsigned char upper){
	unsigned char i; 
	char  temp[17];

	strcpy_P(temp,c);

	if (upper) strupr(temp);

	for(i=0;temp[i]!=0;i++)
		LCD_wr_data(temp[i]);
	
}

unsigned char LCD_bit_reverse (unsigned char data){
	unsigned char temp=0;
	temp+=(data>>3);
	temp+=((data>>2)&1)<<1;
	temp+=((data>>1)&1)<<2;
	temp+=(data & 1) << 3;
	
	return (temp&0x0f);	
}

// send command to LCD
void LCD_wr_cmd (unsigned char cmd)
{
#ifdef LCD_PARALEL
    unsigned char temp;
	temp=LCD_bit_reverse(cmd>>4);
	LCD_CLR(LCD_RS);
	LCD_SET(LCD_E);
	LCD_PORT = (LCD_PORT & 0xF0) | temp;
	LCD_CLR(LCD_E);
	LCD_SET(LCD_E);

	temp=LCD_bit_reverse(cmd&0x0f);
	LCD_PORT = (LCD_PORT & 0xF0) | temp;
	LCD_CLR(LCD_E);
	_delay_us(60);
	_delay_ms(2);
#else
    LCD_CLR(LCD_RS);
    LCD_SET(LCD_E);
    refresh_port();
    //LCD_PORT = (LCD_PORT & 0xF0) | temp;
    port &= 0b00001111;
    port |= cmd & 0xf0;
    refresh_port();
    LCD_CLR(LCD_E);
    refresh_port();
    LCD_SET(LCD_E);
    refresh_port();

    //LCD_PORT = (LCD_PORT & 0xF0) | temp;
    port &= 0b00001111;
    port |= (cmd &0xf) << 4;
    refresh_port();
    LCD_CLR(LCD_E);
    refresh_port();
    chThdSleepMilliseconds(1);
#endif
}

// send data to LCD
void LCD_wr_data (int data)
{
#ifdef LCD_PARALEL
	LCD_SET(LCD_E);
	LCD_SET(LCD_RS);
	LCD_PORT = (LCD_PORT & 0xF0) | (LCD_bit_reverse(data >> 4));
	LCD_CLR(LCD_E);
	LCD_SET(LCD_E);
	LCD_PORT = (LCD_PORT & 0xF0) | (LCD_bit_reverse(data & 0x0F));
	LCD_CLR(LCD_E);
	_delay_us(60);
#else
    LCD_SET(LCD_E);
    LCD_SET(LCD_RS);
    refresh_port();

    port &= 0b00001111;
    port |= data & 0xf0;
    refresh_port();
    LCD_CLR(LCD_E);
    refresh_port();

    LCD_SET(LCD_E);
    refresh_port();
    port &= 0b00001111;
    port |= (data &0xf) << 4;
    refresh_port();
    LCD_CLR(LCD_E);
    refresh_port();
    chThdSleepMilliseconds(1);
#endif

}


// set X,Y cursor position
void LCD_xy(unsigned char x,unsigned char y)
{
	unsigned char adr;
	y=y<<6;
	adr=x+y;
	adr+=0b10000000; 	//DD RAM address
	LCD_wr_cmd(adr);
}

char * uchar2string(char data_in, unsigned char sign_in){
	char temp;
	char sign;

	if(data_in==0) {
		strcpy(string,"   0");	
		return string;
	}
	
	if(data_in>0){
		if (sign_in) sign='+'; else sign=' ';
	} else {
		sign='-';
		data_in=-data_in;
	}

	*(string)=' ';
	temp=data_in/100;	
	data_in-=100*temp;
	if(!temp) temp=' '-48;
	*(string+1)=temp+48;

	temp=data_in/10;	
	data_in-=10*temp;
	if(!temp && (*(string+1)==' ')) temp=' '-48;
	*(string+2)=temp+48;

	*(string+3)=data_in+48;
	*(string+4)=0;

	if(string[2]==' '){
		string[2]=sign;
		return string;
	}

	if(string[1]==' '){
		string[1]=sign;
		return string;
	}
	string[0]=sign;
	return string;
}

void LCD_putnum(char data_in,unsigned char sign){
	LCD_puts(uchar2string(data_in,sign));
}

void LCD_putnum_left(char data_in,unsigned char sign){
	unsigned char i=0;

	uchar2string(data_in,sign);
	while (string[i] == ' ')
		i++;

	LCD_puts(i+string);
}

void LCD_putunsigned(unsigned char data){
	unsigned char temp;

	temp=data/100;	
	data-=100*temp;
	*(string)=temp+48;

	temp=data/10;	
	data-=10*temp;
	*(string+1)=temp+48;

	*(string+2)=data+48;
	*(string+3)=0;

	LCD_puts(string);
}


unsigned char LCD_log(unsigned char data_in){
	unsigned char temp;

	if (data_in <7){
		temp=3*data_in;
	} else
	if (data_in < 21){
		temp=data_in + 12;
	}
	if (data_in > 20){
		temp=data_in/5 + 28;
	}

	return (temp);
}

