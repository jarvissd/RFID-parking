#include "stm32f10x.h"
#include "SPIV.h"
#include "RC522.h"
#include "delayV.h"
#include "LCDV.h"
#include "stdio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include <string.h>

void GPIOFN(void);
void fn_PWM(void);
void fn_UART(void);
void SysTick_Hand(void);
void delay(uint32_t ms);
void processRFIDcard(uint8_t card[]);
void compareRFID(unsigned char card[],unsigned char cards[4][5],int *found); 
void SendESP32(USART_TypeDef* USARTx,const char* str1);
void Convert(const unsigned char *rfidData,char *charArray,size_t length);

int count = 4, i,j,k=4, found,n,m,X;
unsigned  char card[5], cards[4][5]={0},ismatch;
char Buff[100], str[5],cam,esp[5];
long duty=0,duty2=10;
 
int main() {
		
    Delay_Init();
    GPIOFN();
		fn_UART();
    TM_MFRC522_Init();
    I2CLCD_Init();
    fn_PWM();
 
    while (1) {
        I2CLCD_Clear();
        LCD_Gotoxy(0, 0);
        LCD_Puts("Slot parking: ");
        snprintf(str, sizeof(str), "%d", count);
        LCD_Gotoxy(15, 0);
        LCD_Puts(str);
        while (TM_MFRC522_Check(card) == MI_OK) {
          compareRFID(card,cards,&found);  
					processRFIDcard(card);
				Delay_Ms(700);
        }

				duty=100;
					TIM_SetCompare1(TIM2,duty*10);
				for(i=0;i<=4;i++)
				{
					card[i]=0;
				}

    }
		
}

void processRFIDcard(uint8_t card[]) {
		size_t lenth=4;
		char lenth2[2*4+1];
		if(found)
		{	
			duty=50;
			TIM_SetCompare1(TIM2,duty*10);
			count++;
			for(i=0;i<5;i++)
			{	
				cards[X][i]=0;
			}
			if(count>4)
			{
				count=4;
			}
			ismatch=1;
		}
 
    if (!found && count >0) {
			for(k=0;k<4;k++)
			{
				if(cards[k][0]==0)
				{
					break;
				}
			}
			for(i=0;i<5;i++)
			{
				cards[k][i]=card[i];
				esp[i]=card[i];
			}
			count--;
 			Convert(esp,lenth2,5);
			SendESP32(USART1,lenth2);
			
			duty=50;
			TIM_SetCompare1(TIM2,duty*10);
	}	 
		found=0;
    sprintf(Buff, "%02X%02X%02X%02X%02X", card[0], card[1], card[2], card[3], card[4]);
    LCD_Gotoxy(0, 1);
    LCD_Puts("ID:");
    LCD_Gotoxy(4, 1);
    LCD_Puts(Buff);

    GPIO_SetBits(GPIOC, GPIO_Pin_13);
    Delay_Ms(200);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
 
			if(count==0)
			{	
				I2CLCD_Clear();
				LCD_Gotoxy(0,1);
				LCD_Puts("No Space");
				GPIO_SetBits(GPIOC, GPIO_Pin_13);
				Delay_Ms(1000);
				GPIO_ResetBits(GPIOC, GPIO_Pin_13);
 
			}

}
void GPIOFN(void) {
    GPIO_InitTypeDef GPIO_Inis;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_Inis.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Inis.GPIO_Pin = GPIO_Pin_13;
    GPIO_Inis.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_Inis);
}

void fn_PWM(void) {
 
    GPIO_InitTypeDef GPIO_Iniss;
    TIM_TimeBaseInitTypeDef TIM_Inis;
    TIM_OCInitTypeDef PWM_Inis;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    GPIO_Iniss.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Iniss.GPIO_Pin = GPIO_Pin_0;
    GPIO_Iniss.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &GPIO_Iniss);

    TIM_Inis.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_Inis.TIM_Period = 1000;
    TIM_Inis.TIM_Prescaler = 120;
    TIM_Inis.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM2, &TIM_Inis);
    TIM_Cmd(TIM2, ENABLE);

    PWM_Inis.TIM_OCMode = TIM_OCMode_PWM1;
    PWM_Inis.TIM_OCPolarity = TIM_OCPolarity_High;
    PWM_Inis.TIM_Pulse = (int)(duty*10 );
    PWM_Inis.TIM_OutputState = TIM_OutputState_Enable;
//    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC1Init(TIM2, &PWM_Inis);
}
void compareRFID(unsigned char card[], unsigned char cards[4][5], int *found) {
    *found = 0;
    for (n = 3; n >= 0; n--) {
        ismatch = 1;
        if (memcmp(card + 1, cards[n] + 1, 4) == 0) {
          X=n;  
					*found = 1;
            break;
        }
    }
}
void fn_UART(void)
{	
	GPIO_InitTypeDef GPIO_Inis;
	USART_InitTypeDef UART_Inis;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_Inis.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Inis.GPIO_Pin=GPIO_Pin_9;//TX
	GPIO_Inis.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_Inis);
	
	GPIO_Inis.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Inis.GPIO_Pin=GPIO_Pin_10;
	GPIO_Init(GPIOA,&GPIO_Inis);
	
	UART_Inis.USART_BaudRate=9600;
	UART_Inis.USART_WordLength=USART_WordLength_8b;
	UART_Inis.USART_StopBits=USART_StopBits_1;
	UART_Inis.USART_Parity=USART_Parity_No;
	UART_Inis.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	UART_Inis.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init(USART1,&UART_Inis);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	USART_Cmd(USART1,ENABLE);
	
}
void SendESP32(USART_TypeDef* USARTx,const char* str1){
	while(*str1){
		USART_SendData(USARTx,*str1++);
		while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
	}
	USART_SendData(USARTx,'\n');
	while(USART_GetFlagStatus(USARTx,USART_FLAG_TXE)==RESET);
}
void Convert(const unsigned char *rfidData, char *charArray, size_t length) { 
    size_t i;
    for (i = 0; i < length; i++) {
        sprintf(&charArray[i * 2], "%02X", rfidData[i]);
    }
}
 
 
 
