#ifndef __LCD_I2C_H
#define __LCD_I2C_H

#include "stm32f10x.h"

#define PCF8574A_Address      	0x4e
#define I2C_Chanel      				I2C2

void I2CLCD_Config(void);
void I2CLCD_Send_byte(char data);
void I2CLCD_Init(void);
void I2CLCD_SendData(char data);
void LCD_SendCommand(char data);
void LCD_Puts(char *str);
void I2CLCD_Clear(void);
void LCD_Gotoxy(unsigned char x, unsigned char y);

#endif