/************************************版权申明********************************************
**                             广州大彩光电科技有限公司
**                             http://www.gz-dc.com
**-----------------------------------文件信息--------------------------------------------
** 文件名称:   hmi_user_uart.c
** 修改时间:   2011-05-18
** 文件说明:   用户MCU串口驱动函数库
** 技术支持：  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------
                                  使用必读
   hmi_user_uart.c中的串口发送接收函数共3个函数：串口初始化Uartinti()、发送1个字节SendChar()、
   发送字符串SendStrings().若移植到其他平台，需要修改底层寄
   存器设置,但禁止修改函数名称，否则无法与HMI驱动库(hmi_driver.c)匹配。
--------------------------------------------------------------------------------------



----------------------------------------------------------------------------------------
                          1. 基于STM32平台串口驱动
----------------------------------------------------------------------------------------*/
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "stdio.h"
#include "string.h"
u8    RecvFlag;
u8    RevcLen[2];
u32   Recvi;                  //unsigned int
u32   RevcLens;
u8 USART_RX_BUF[1024];   							//接收数据缓存
int RxCounter = 0;       			//接收数据记数指针
u8 ABORT_Succeed = 0;     //中止成功标志

extern  vu8  RecvSucceed;     //volatile  unsigned char
extern  vu8  RecvBuf[100];

extern volatile uint32_t timer_tick_count;



//加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//标准库需要的支持函数
struct __FILE
{
	int handle;

};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
_sys_exit(int x)
{
	x = x;
}
//重定义fputc函数
int fputc(int ch, FILE *f)
{
	while((USART2->SR&0X40)==0);//循环发送,直到发送完毕
	USART2->DR = (u8) ch;
	return ch;
}
#endif


/****************************************************************************
* 名    称： UartInit()
* 功    能： 串口初始化
* 入口参数： 无
* 出口参数： 无
****************************************************************************/

void Uart1_Init(uint32 BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);

	//USART_DeInit(USART1);
	/* Configure USART1 Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//  /* Configure USART1 Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//USART_DeInit(USART2);
	//USART2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	//Usart2 NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);

	//Usart 初始化设置
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx;

	/* USART configuration */
	//USART_InitStructure.USART_BaudRate = 115200;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //接收中断使能
	USART_Cmd(USART1, ENABLE); //使能串口1

	//USART_InitStructure.USART_BaudRate = 115200;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART2, ENABLE); //使能串口2
}




/******************************************************************
	*名称：void USART1_IRQHandler(void)
	*输入：无
	*输出：无
	*功能：
	*说明：
******************************************************************/
void USART1_IRQHandler(void)
{
	u8 mid,i;
	if(USART_GetITStatus(USART1, USART_IT_RXNE)!= RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		USART_RX_BUF[RxCounter++] = USART_ReceiveData(USART1);
		if(USART_RX_BUF[RxCounter-1] == 0x69)
		{
			if(USART_RX_BUF[RxCounter-2] == 0x6A)
			{
				if(USART_RX_BUF[0] == 0x5a)
				{
					if(USART_RX_BUF[1] == 0x55)
					{
						if(USART_RX_BUF[RxCounter-3] == check_sum(USART_RX_BUF,RxCounter-3))
						{
							RevcLens = (USART_RX_BUF[3]<<8)+USART_RX_BUF[2];
							memcpy(RecvBuf,USART_RX_BUF,RxCounter); 	//复制参数内容
							RxCounter = 0;
							RecvSucceed = 0x01;
							
							if((USART_RX_BUF[5] = 0x11)&&(USART_RX_BUF[7] = 0x01))
							{
								ABORT_Succeed = 1;
							}
							else ABORT_Succeed = 0;
							memset(&USART_RX_BUF,0,1024*sizeof(u8));

						}
						else
						{
							RxCounter =0;
							memset(&USART_RX_BUF,0,1024*sizeof(u8));
						}

					}
					else
					{
						RxCounter =0;
							memset(&USART_RX_BUF,0,1024*sizeof(u8));
					}
				}
				else
				{
					RxCounter =0;
							memset(&USART_RX_BUF,0,1024*sizeof(u8));
				}
			}
		}
	}
}


/******************************************************************
	*名称：void USART1_SendString(uint8_t *ch)
	*输入：无
	*输出：无
	*功能：发送字符串
	*说明：
******************************************************************/
void USART1_SendString(uint8_t *ch)
{
	while(*ch!=0)
	{
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
		USART_SendData(USART1, *ch);
		ch++;
	}
}


/******************************************************************
  *名称:void USART1_SendData(uint8_t *ch,uint8_t Num)
	*输入：无
	*输出：无
	*功能：发送数据
	*说明：
******************************************************************/
void USART1_SendData(uint8_t *ch,uint8_t Num)
{
	uint8_t i;
	for(i=0; i<Num; i++)
	{
		while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));
		USART_SendData(USART1, *ch);
		ch++;
	}
}



/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{
	  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    { 
			  uint8_t data = USART_ReceiveData(USART2);
        queue_push(data);
    }
}

/*****************************************************************
* 名    称： SendChar()
* 功    能： 发送1个字节
* 入口参数： t  发送的字节
* 出口参数： 无
 *****************************************************************/
void  SendChar(u8 t)
{
	USART_SendData(USART2,t);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	while((USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET));//等待串口发送完毕
}
