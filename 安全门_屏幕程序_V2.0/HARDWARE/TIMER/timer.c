#include "timer.h"
#include "led.h"
#include "beep.h"
#include "wdg.h"
#include "UHFhw_config.h"
#include "uhf.h"

//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//通用定时器 驱动代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/03
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////
extern unsigned   int            HeartFlag;
extern volatile     unsigned     long         NetWait;
extern unsigned   int					 ReaderTimedOut;      //读写器超时标志
extern volatile uint32_t timer_tick_count;
extern int       waiting_time1 ;   //判断进出时间
extern int       waiting_time2 ;   //判断进出时间
extern int       clean_time;   //清除超时进出标志位
extern int       save_time;
extern int       TestAntTimeOut;     //天线测试超时标志

extern u8 StartAlarmFlag;
extern int KeepAlarm_time ; //报警时间
u8 stopscanFlag = 0;
int KeepScan_time = 0;
u8 startscanFlag = 0;   //开始扫描标志
extern int KeepGateLinkage_time; //开闸机持续时间
extern u8             NetNotFound ; //网络超时标志
int   KeepGate_time =0; //门禁持续时间
u8 stopalarmFlag = 0;
extern int KeepBookName_time; //显示书名时间
extern u8 BookNameKeep_flag; //显示书名标志




//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器2!
void Timer2_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = 5000; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(  //使能或者失能指定的TIM中断
	    TIM2, //TIM2
	    TIM_IT_Update|  //TIM 中断源
	    TIM_IT_Trigger,   //TIM 触发中断源
	    ENABLE  //使能
	    
	);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM2, ENABLE);  //使能TIMx外设

}

void TIM2_IRQHandler(void)   //TIM2中断
{
		u8 cleanscreen[] = {0};
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源
		IWDG_Feed();   //喂独立看门狗
		RUN=!RUN;
		//HeartFlag++;   			//心跳包++
		NetWait++;		 			//网络超时++
	  ReaderTimedOut++;		//读写器超时++
	 ++timer_tick_count;
		waiting_time1++;      //传感器1计数
		waiting_time2++;      //传感器2计数
		clean_time++;         //超时计数
		//save_time++;          //定期保存数据
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
				SetTextValue(0,4,cleanscreen);    //清除显示书名
				BookNameKeep_flag = 0;
			}
			
		}
	}
}


//TIM3 PWM部分初始化 
//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数
void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	//使能定时器3时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE); //Timer3 重映射  TIM3_CH1->PC6    
 
   //设置该引脚为复用输出功能,输出TIM3 CH1的PWM脉冲波形	GPIOC.6
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIO
  GPIO_ResetBits(GPIOC,GPIO_Pin_6);//输出0，关闭蜂鸣器输出
   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC1

	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 
	TIM_Cmd(TIM3, ENABLE);  //使能TIM3
	TIM_SetCompare1(TIM3,899); //输出0，关闭蜂鸣器输出	
}



//定时器3中断服务程序
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
		}
}

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器2!
void Timer4_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = 1000; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到1000为100ms
	TIM_TimeBaseStructure.TIM_Prescaler =(7200-1); //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

	TIM_ITConfig(  //使能或者失能指定的TIM中断
	    TIM4, //TIM4
	    TIM_IT_Update|  //TIM 中断源
	    TIM_IT_Trigger,   //TIM 触发中断源
	    ENABLE  //使能
	    
	);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM4, ENABLE);  //使能TIMx外设

}

void TIM4_IRQHandler(void)   //TIM2中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源
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



