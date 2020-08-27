#include "beep.h"
#include "timer.h"
//#include "ulitity.h"


#define ADJUSTBEEP 1                      //是否开启调节BEEP模式
#define MaxV      0xff                     //最大音量
#define MinV      0                        //禁音
//#define BEEP(x)   ((x) ? (GPIO_ResetBits(GPIOC, GPIO_Pin_6)) : (GPIO_SetBits(GPIOC, GPIO_Pin_6)));

u16 Beeppwmval=0;                       
                      


#if  ADJUSTBEEP

//初始化PB8为输出口.并使能这个口的时钟		    
//蜂鸣器初始化
void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能GPIOC端口时钟
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 //BEEP-->PC.6 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);	 //根据参数初始化GPIOC.6
 
 GPIO_ResetBits(GPIOC,GPIO_Pin_6);//输出0，关闭蜂鸣器输出

}


/*******************************************************************************
* 函数名  : AdjustVolume
* 描述    : 根据终端上获得输入， 调节音量
* 输入    : getch
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void AdjustVolume(u32 Volume)
{
	OpenBEEP(Beeppwmval);
	//TIM_SetCompare1(TIM3,Beeppwmval);     //设置PWM占空比
	if (Volume == 0)
	{
	//	StopBEEP;//输出0，关闭蜂鸣器输出
		Volume++;
	}
	 if(Volume >=1)
	{
	  Beeppwmval = 900-Volume*9;
		OpenBEEP(Beeppwmval);
		StopBEEP;
	}

}
#endif
