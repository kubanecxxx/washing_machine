#ifndef LCD_opt
#define LCD_opt

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>



// LCD basic commands
#define LCD_clear()		(LCD_wr_cmd(1))
#define LCD_blink_ON()	(LCD_wr_cmd(0b00001111))
#define LCD_blink_OFF()	(LCD_wr_cmd(0b00001110))

// prototypes
void LCD_init(void);
void LCD_cgram_load_VU(void);
void LCD_cgram_load_balance(void);
void LCD_puts (const char *s);
void LCD_putsflash (const char *s);
void LCD_puttitle(const char *s , unsigned char cmd);
void LCD_wr_cmd (unsigned char cmd);
void LCD_wr_data (int data);
void LCD_xy(unsigned char x,unsigned char y);

//conversion funtions
void LCD_bargraph(unsigned char);
void LCD_scrollbar(char data_in, char center,  char lo_lim, char high_lim, unsigned char x1, unsigned char x2, unsigned char y);
unsigned char LCD_bit_reverse(unsigned char);
char * uchar2string(char data_in, unsigned char);
void LCD_putnum(char data_in,unsigned char);;
void LCD_putnum_left(char data_in,unsigned char);;
void LCD_putunsigned(unsigned char);
unsigned char LCD_log(unsigned char);

#ifdef __cplusplus
}
#endif

#endif
