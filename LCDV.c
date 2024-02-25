#include "LCDV.h"
#include "delayV.h"
#include <stdio.h>

struct __FILE 
{
	int dummy;
};
FILE __stdout;
int LCD_PutChar(int c, FILE *f) 
{
	I2CLCD_SendData(c);
	return c;
}

GPIO_InitTypeDef GPIO_InitStructure;
I2C_InitTypeDef I2C_InitStructure;

void I2CLCD_Config(void)
{
    // cap clock cho ngoai vi va I2C
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);							// su dung kenh I2C2 cua STM32
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // cau hinh chan SDA va SCL
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;						//PB10 - SCL, PB11 - SDA
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // cau hinh I2C
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0; // 
    I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;       
    I2C_InitStructure.I2C_ClockSpeed = 100000;										
    I2C_Init(I2C2, &I2C_InitStructure);
    // cho phep bo I2C hoat dong
    I2C_Cmd(I2C2, ENABLE);
}

void I2CLCD_Send_byte(char data)
{
    I2C_GenerateSTART(I2C_Chanel, ENABLE);
    while (!I2C_CheckEvent(I2C_Chanel, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C_Chanel, PCF8574A_Address, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C_Chanel, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C_Chanel, data);
    while (!I2C_CheckEvent(I2C_Chanel, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C_Chanel, ENABLE);		
}
void I2CLCD_Init(void)
{
	Delay_Init();
	I2CLCD_Config();
	LCD_SendCommand(0x33);		 
	Delay_Ms(10);
	LCD_SendCommand(0x32);
	Delay_Ms(50);
	LCD_SendCommand(0x28);
	Delay_Ms(50);	
	LCD_SendCommand(0x01);
	Delay_Ms(50);	
	LCD_SendCommand(0x06);	Delay_Ms(50);;
	LCD_SendCommand(0x0C);	Delay_Ms(50);;
	LCD_SendCommand(0x02);	Delay_Ms(50);
}
void I2CLCD_SendData(char data)
{
	char data_u, data_l;
	uint8_t data_t[4],i=0;
	data_u = data&0xf0;
	data_l = (data<<4)&0xf0;
	data_t[0] = data_u|0x0d;  //en=1, rs=0
	data_t[1] = data_u|0x09;  //en=0, rs=0
	data_t[2] = data_l|0x0d;  //en=1, rs=0
	data_t[3] = data_l|0x09;  //en=0, rs=0
	for(i = 0;i<4;i++)
	{
	     I2CLCD_Send_byte(data_t[i]);
	}

}
void LCD_SendCommand(char data)
{
    char data_u, data_l;
	uint8_t data_t[4],i=0;
	data_u = data&0xf0;
	data_l = (data<<4)&0xf0;
	data_t[0] = data_u|0x04;  //en=1, rs=0
	data_t[1] = data_u;  //en=0, rs=0
	data_t[2] = data_l|0x04;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	for(i = 0;i<4;i++)
	{
	     I2CLCD_Send_byte(data_t[i]);
	}
}

void LCD_Puts (char *str)
{
	while (*str) I2CLCD_SendData(*str++);
}
void I2CLCD_Clear(void)
{
	LCD_SendCommand(0x01);
	Delay_Ms(10);
}

void LCD_Gotoxy(unsigned char x, unsigned char y) // t?a d? ch? tr?n lcd
{
  unsigned char address;
  if(y == 0)
	{	
		address=(0x80 + x); // 0x80 l? k?o con tr? v? h?ng 0 cot 0
	}
  else if(y==1)
	{
		address=(0xC0 + x); // 0xC0 l? k?o con tr? v? h?ng 0 c?t 1
  	}
	LCD_SendCommand(address);
}

