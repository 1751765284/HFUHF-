#include "UHFhw_config.h"
#include "hmi_user_uart.h"


/* 输出继电器LED */
#define RCC_APB2Periph_OUT	RCC_APB2Periph_GPIOC
#define OUT_PORT	GPIOC
#define OUT1_PIN	       GPIO_Pin_0
#define OUT2_PIN	       GPIO_Pin_1
#define OUT3_PIN	       GPIO_Pin_2

#define LED1(x)   ((x) ? (GPIO_ResetBits(OUT_PORT, OUT1_PIN)) : (GPIO_SetBits(OUT_PORT, OUT1_PIN)));
#define LED2(x)   ((x) ? (GPIO_ResetBits(OUT_PORT, OUT2_PIN)) : (GPIO_SetBits(OUT_PORT, OUT2_PIN)));
#define LED3(x)   ((x) ? (GPIO_ResetBits(OUT_PORT, OUT3_PIN)) : (GPIO_SetBits(OUT_PORT, OUT3_PIN)));

/*光幕输入信号 In*/
#define  RCC_APB2Periph_IN	RCC_APB2Periph_GPIOB
#define  IN_PORT	       GPIOB
#define  IN_PIN1	       GPIO_Pin_0
#define  IN_PIN2	       GPIO_Pin_1

#define  INP_PIN1   GPIO_ReadInputDataBit(IN_PORT,IN_PIN1)
#define  INP_PIN2   GPIO_ReadInputDataBit(IN_PORT,IN_PIN2)

/******************************************************************
	*名称：void hw_Init(void)
	*输入：无
	*输出：无
	*功能：OUT_LED,光幕的初始化配置
	*说明：
******************************************************************/
void UHFhw_Init(void)
{
 	GPIO_InitTypeDef  GPIO_InitStructure; 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_OUT , ENABLE);	        //使能LED端口时钟	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_IN, ENABLE);          //使能光幕端口时钟
	  /*---OUT_LED配置---*/	
	GPIO_InitStructure.GPIO_Pin = OUT1_PIN | OUT2_PIN |OUT3_PIN;		 //OUT_LED1-->PC.0；LED2-->PC.1 端口配置
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		   //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		   //IO口速度为50MHz
  GPIO_Init(OUT_PORT, &GPIO_InitStructure);					       //根据设定参数初始化GPIOC
	  /*---Alarm配置---*/	
//  GPIO_InitStructure.GPIO_Pin = Alarm_PIN  ;             //Alarm-->PD.8端口配置
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
//	GPIO_Init(Alarm_PORT, &GPIO_InitStructure);
	  /*---Alarm配置---*/	
	GPIO_InitStructure.GPIO_Pin = IN_PIN1|IN_PIN2;         //光幕1-->PB.0；光幕2-->PB.1 端口配置
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;	       //上拉输入
	GPIO_Init(IN_PORT, &GPIO_InitStructure);	
	  /*---------初始化状态四个LED全OFF------------*/
	LED1(0);
	LED2(0);
	LED3(0);
    /*---------初始化状态AlarmOFF------------*/
	//Alarm(1);
}

 
