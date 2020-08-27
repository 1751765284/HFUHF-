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

// ���ͣ� 05 08 ff 01 f5 03 01 c5fd
// ����:  05 08 01 01 f5 03 00 6a2a
volatile u32  timer_tick_count = 0; //��ʱ������


//�������
int main()
{
	
	qsize  size = 0;
	SystemInit();     	                                         //ϵͳ��ʼ��
	delay_init();      		                                       //����ʱ�ӽ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);							 //�ж�������ʼ��
	Uart1_Init(115200);                                        	 //���ڳ�ʼ��,����������Ϊ115200
	BEEP_Init();                                                 //��ʼ���������˿�
	LED_Init();		 			 							                           //LED�˿ڳ�ʼ��
		Timer2_Init();											                         //10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms
	TIM3_PWM_Init(899,0);	                                       //����Ƶ��PWMƵ��=72000000/900=80Khz
	Timer4_Init();											                         //10Khz�ļ���Ƶ�ʣ�������1000Ϊ100ms
	AdjustVolume((CalcDeviceID()*10)%100);
	UHFhw_Init();                                                //��ʼ��LED��ALARM
	W25QXX_Init();			                                         //W25QXX��ʼ��
  queue_reset();                                             	 //��մ��ڽ��ջ�����
	delay_ms(1000);           	                                 //��ʱ�ȴ���������ʼ�����,����ȴ�300ms
  IWDG_Init(IWDG_Prescaler_64,1875); 	                         //�������Ź�,3s�������
	W5500_HARDWARE_INIT();                                       //��ʼ��W5500
	SystemReset();                                               //������������
	if( (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11 ) !=0 ) ? 0 : 1)
	{
		W5500_NVIC_Close_Configuration();
	}
	save_time =0;
  //W25QXX_Erase_Chip();                                         //����flash
	ScanSetC();
	AntSetC();
	StartScan(); /* ��ʼɨ��*/

	while(1)
	{
		if(stopscanFlag == 1)
		{
			stopscanFlag = 0;
			Start = 0;
	  	//StopScan();  /* ֹͣɨ��*/
		  touch = 0;
			CompareEPC_T.s_EPCCounter = 0;
			memset(CompareEPC_T.s_EPC,0,sizeof(CompareEPC_T.s_EPC));
		}
		//TriggerSensors();
		DetectAlarm(1,Door_Mode); //Door_Mode
		delay_ms(10);
//		size = queue_find_cmd(cmd_buffer,CMD_MAX_SIZE); //�ӻ������л�ȡһ��ָ��
//		if(size>0)//���յ�ָ��
//		{
//			ProcessMessage((PCTRL_MSG)cmd_buffer, size);//ָ���
//		}
	}
}





