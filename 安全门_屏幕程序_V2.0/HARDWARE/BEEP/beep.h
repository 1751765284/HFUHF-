#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
//�������˿ڶ���
#define BEEP(x)   ((x) ? (GPIO_ResetBits(GPIOC, GPIO_Pin_6)) : (GPIO_SetBits(GPIOC, GPIO_Pin_6)));
#define OpenBEEP(value)    TIM_SetCompare1(TIM3,value)
#define StopBEEP           TIM_SetCompare1(TIM3,899)

extern u16 Beeppwmval; 
void BEEP_Init(void);	//��ʼ��
void AdjustVolume(u32 Volume);
		 				    
#endif

