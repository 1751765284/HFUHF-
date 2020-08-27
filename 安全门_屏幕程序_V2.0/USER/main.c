#include "led.h"
#include "hmi_driver.h"
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "hw_config.h"
#include "delay.h"
#include "beep.h"
#include "uhf.h"
#include "stm32f10x_it.h"
#include "W5500.h"
#include "spi.h"
#include "w25qxx.h"
#include "Screen.h"
#include "led.h"
#include "timer.h"
#include "UHFhw_config.h"
#include "sys.h"
#include "wdg.h"

// 发送： 05 08 ff 01 f5 03 01 c5fd
// 返回:  05 08 01 01 f5 03 00 6a2a
volatile u32  timer_tick_count = 0; //定时器节拍


//程序入口
int main()
{
	
	qsize  size = 0;
	SystemInit();     	                                         //系统初始化
	delay_init();      		                                       //配置时钟节拍
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);							 //中断向量初始化
	Uart1_Init(115200);                                        	 //串口初始化,波特率设置为115200
	BEEP_Init();                                                 //初始化蜂鸣器端口
	LED_Init();		 			 							                           //LED端口初始化
		Timer2_Init();											                         //10Khz的计数频率，计数到5000为500ms
	TIM3_PWM_Init(899,0);	                                       //不分频。PWM频率=72000000/900=80Khz
	Timer4_Init();											                         //10Khz的计数频率，计数到1000为100ms
	AdjustVolume((CalcDeviceID()*10)%100);
	UHFhw_Init();                                                //初始化LED和ALARM
	W25QXX_Init();			                                         //W25QXX初始化
  queue_reset();                                             	 //清空串口接收缓冲区
	delay_ms(1000);           	                                 //延时等待串口屏初始化完毕,必须等待300ms
  IWDG_Init(IWDG_Prescaler_64,1875); 	                         //启动开门狗,3s溢出重启
	W5500_HARDWARE_INIT();                                       //初始化W5500
	SystemReset();                                               //开机更新数据
	if( (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11 ) !=0 ) ? 0 : 1)
	{
		W5500_NVIC_Close_Configuration();
	}
	save_time =0;
  //W25QXX_Erase_Chip();                                         //擦除flash
	ScanSetC();
	AntSetC();
	StartScan(); /* 开始扫描*/

	while(1)
	{
		if(stopscanFlag == 1)
		{
			stopscanFlag = 0;
			Start = 0;
	  	//StopScan();  /* 停止扫描*/
		  touch = 0;
			CompareEPC_T.s_EPCCounter = 0;
			memset(CompareEPC_T.s_EPC,0,sizeof(CompareEPC_T.s_EPC));
		}
		//TriggerSensors();
		DetectAlarm(1,Door_Mode); //Door_Mode
		delay_ms(10);
//		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //从缓冲区中获取一条指令
//		if(size>0)//接收到指令
//		{
//			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//指令处理
//		}
	}
}





