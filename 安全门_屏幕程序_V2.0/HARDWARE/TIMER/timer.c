#include "timer.h"
#include "led.h"
#include "beep.h"
#include "wdg.h"
#include "UHFhw_config.h"
#include "uhf.h"

//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//ͨ�ö�ʱ�� ��������
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////
extern unsigned   int            HeartFlag;
extern volatile     unsigned     long         NetWait;
extern unsigned   int					 ReaderTimedOut;      //��д����ʱ��־
extern volatile uint32_t timer_tick_count;
extern int       waiting_time1 ;   //�жϽ���ʱ��
extern int       waiting_time2 ;   //�жϽ���ʱ��
extern int       clean_time;   //�����ʱ������־λ
extern int       save_time;
extern int       TestAntTimeOut;     //���߲��Գ�ʱ��־

extern u8 StartAlarmFlag;
extern int KeepAlarm_time ; //����ʱ��
u8 stopscanFlag = 0;
int KeepScan_time = 0;
u8 startscanFlag = 0;   //��ʼɨ���־
extern int KeepGateLinkage_time; //��բ������ʱ��
extern u8             NetNotFound ; //���糬ʱ��־
int   KeepGate_time =0; //�Ž�����ʱ��
u8 stopalarmFlag = 0;
extern int KeepBookName_time; //��ʾ����ʱ��
extern u8 BookNameKeep_flag; //��ʾ������־




//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��2!
void Timer2_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = 5000; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
	    TIM2, //TIM2
	    TIM_IT_Update|  //TIM �ж�Դ
	    TIM_IT_Trigger,   //TIM �����ж�Դ
	    ENABLE  //ʹ��
	    
	);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx����

}

void TIM2_IRQHandler(void)   //TIM2�ж�
{
		u8 cleanscreen[] = {0};
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ
		IWDG_Feed();   //ι�������Ź�
		RUN=!RUN;
		//HeartFlag++;   			//������++
		NetWait++;		 			//���糬ʱ++
	  ReaderTimedOut++;		//��д����ʱ++
	 ++timer_tick_count;
		waiting_time1++;      //������1����
		waiting_time2++;      //������2����
		clean_time++;         //��ʱ����
		//save_time++;          //���ڱ�������
		TestAntTimeOut++;
		if( startscanFlag == 1)
		{
			KeepScan_time++;
			if(KeepScan_time >8)           //4s
			{
				stopscanFlag = 1;
				KeepScan_time = 0;
				startscanFlag = 0;
			}
		}
		if(phone == 1)
		{
			if(NetWait == 20)
			{
				WaitNet();
			}
			if(NetWait >30)
			{
				NetNotFound =0;
				SoftReset();
			}
		}
		if(BookNameKeep_flag == 1)
		{
			KeepBookName_time++;
			if(KeepBookName_time>= 10)
			{
				KeepBookName_time = 0;
				SetTextValue(0,4,cleanscreen);    //�����ʾ����
				BookNameKeep_flag = 0;
			}
			
		}
	}
}


//TIM3 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//ʹ�ܶ�ʱ��3ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3 ��ӳ��  TIM3_CH1->PC6    
 
   //���ø�����Ϊ�����������,���TIM3 CH1��PWM���岨��	GPIOC.6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIO
  GPIO_ResetBits(GPIOC,GPIO_Pin_6);//���0���رշ��������
   //��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM3 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC1

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR2�ϵ�Ԥװ�ؼĴ���
 
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
	TIM_SetCompare1(TIM3,899); //���0���رշ��������	
}



//��ʱ��3�жϷ������
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 
		}
}

//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��2!
void Timer4_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = 1000; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������1000Ϊ100ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
	    TIM4, //TIM4
	    TIM_IT_Update|  //TIM �ж�Դ
	    TIM_IT_Trigger,   //TIM �����ж�Դ
	    ENABLE  //ʹ��
	    
	);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIMx����

}

void TIM4_IRQHandler(void)   //TIM2�ж�
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ
		if(GateLinkage == 1)
		{
			LED3(1);
			KeepGateLinkage_time++;
			if(KeepGateLinkage_time >=80)
			{
				LED3(0);
				KeepGateLinkage_time = 0;
				GateLinkage =0;
			}
		}
		if(StartAlarmFlag == 1)
		{
			KeepAlarm_time++;
			if(KeepAlarm_time >= 30)        //3s
			{
				LED1(0);
				LED2(0);

				StopBEEP;
				NetCmd = 1;
				stopscanFlag = 1;
				KeepAlarm_time = 0;
				StartAlarmFlag = 0;
				
			}
			if(KeepAlarm_time%6 == 0) 
			{
				LED1(0);
				LED2(0);
				StopBEEP;
			}
			else
			{
				LED1(1);
				LED2(1);
				OpenBEEP(Beeppwmval);	
			}		
		}
	}
}



