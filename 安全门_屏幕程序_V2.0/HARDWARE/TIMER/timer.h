#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#include "W5500.h"
#include "Screen.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//ͨ�ö�ʱ�� ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
extern u8 stopscanFlag;
extern int KeepScan_time;
extern u8 startscanFlag;   //��ʼɨ���־
extern u8 stopalarmFlag;

void Timer2_Init(void); 
void TIM3_PWM_Init(u16 arr,u16 psc);
void Timer4_Init(void);
#endif
