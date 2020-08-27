#include "UHFhw_config.h"
#include "hmi_user_uart.h"


/* ����̵���LED */
#define RCC_APB2Periph_OUT	RCC_APB2Periph_GPIOC
#define OUT_PORT	GPIOC
#define OUT1_PIN	       GPIO_Pin_0
#define OUT2_PIN	       GPIO_Pin_1
#define OUT3_PIN	       GPIO_Pin_2

#define LED1(x)   ((x) ? (GPIO_ResetBits(OUT_PORT, OUT1_PIN)) : (GPIO_SetBits(OUT_PORT, OUT1_PIN)));
#define LED2(x)   ((x) ? (GPIO_ResetBits(OUT_PORT, OUT2_PIN)) : (GPIO_SetBits(OUT_PORT, OUT2_PIN)));
#define LED3(x)   ((x) ? (GPIO_ResetBits(OUT_PORT, OUT3_PIN)) : (GPIO_SetBits(OUT_PORT, OUT3_PIN)));

/*��Ļ�����ź� In*/
#define  RCC_APB2Periph_IN	RCC_APB2Periph_GPIOB
#define  IN_PORT	       GPIOB
#define  IN_PIN1	       GPIO_Pin_0
#define  IN_PIN2	       GPIO_Pin_1

#define  INP_PIN1   GPIO_ReadInputDataBit(IN_PORT,IN_PIN1)
#define  INP_PIN2   GPIO_ReadInputDataBit(IN_PORT,IN_PIN2)

/******************************************************************
	*���ƣ�void hw_Init(void)
	*���룺��
	*�������
	*���ܣ�OUT_LED,��Ļ�ĳ�ʼ������
	*˵����
******************************************************************/
void UHFhw_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure; 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_OUT , ENABLE);	        //ʹ��LED�˿�ʱ��	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_IN, ENABLE);          //ʹ�ܹ�Ļ�˿�ʱ��
	  /*---OUT_LED����---*/	
	GPIO_InitStructure.GPIO_Pin = OUT1_PIN | OUT2_PIN |OUT3_PIN;		 //OUT_LED1-->PC.0��LED2-->PC.1 �˿�����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		   //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO���ٶ�Ϊ50MHz
  GPIO_Init(OUT_PORT, &GPIO_InitStructure);					       //�����趨������ʼ��GPIOC
	  /*---Alarm����---*/	
//  GPIO_InitStructure.GPIO_Pin = Alarm_PIN  ;             //Alarm-->PD.8�˿�����
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
//	GPIO_Init(Alarm_PORT, &GPIO_InitStructure);
	  /*---Alarm����---*/	
	GPIO_InitStructure.GPIO_Pin = IN_PIN1|IN_PIN2;         //��Ļ1-->PB.0����Ļ2-->PB.1 �˿�����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;	       //��������
	GPIO_Init(IN_PORT, &GPIO_InitStructure);	
	  /*---------��ʼ��״̬�ĸ�LEDȫOFF------------*/
	LED1(0);
	LED2(0);
	LED3(0);
    /*---------��ʼ��״̬AlarmOFF------------*/
	//Alarm(1);
}

 
