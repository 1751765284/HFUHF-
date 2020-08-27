#ifndef __HW_CONFIG_H_
#define __HW_CONFIG_H_
#include "stm32f10x_conf.h"
//#include "stm32f10x_tim.h"

/*使用位带操作，对单个IO灵活操作*/
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
   

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 

//#define PFout(n)	*((volatile unsigned long *)(0x42000000+((GPIOF_ODR_Addr-0x40000000)<<5)+(n<<2)))

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


/* 函数声明 */
void UHFhw_Init(void);

#endif
