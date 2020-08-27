#ifndef _USER_UART__
#define _USER_UART__

#include "stm32f10x_it.h"     //根据用户MCU进行修改
#include "delay.h"
#include "string.h"
#include "uhf.h"
//#define uchar    unsigned char
//#define uint8    unsigned char
//#define uint16   unsigned short int
//#define uint32   unsigned long
//#define int16    short int
//#define int32    long
extern u8    RecvFlag;
extern u32   RevcLens;
extern u8 ABORT_Succeed;     //中止成功标志
/****************************************************************************
* 名    称： UartInit()
* 功    能： 串口初始化
* 入口参数： 无
* 出口参数： 无
****************************************************************************/
void Uart1_Init(uint32 BaudRate);
void uart2_init(u32 bound);
void USART1_SendString(uint8_t *ch);
void USART1_SendData(uint8_t *ch,uint8_t Num);

void  SendChar(u8 t);


#endif
