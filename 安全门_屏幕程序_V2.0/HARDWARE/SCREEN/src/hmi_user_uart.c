/************************************��Ȩ����********************************************
**                             ���ݴ�ʹ��Ƽ����޹�˾
**                             http://www.gz-dc.com
**-----------------------------------�ļ���Ϣ--------------------------------------------
** �ļ�����:   hmi_user_uart.c
** �޸�ʱ��:   2011-05-18
** �ļ�˵��:   �û�MCU��������������
** ����֧�֣�  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------

--------------------------------------------------------------------------------------
                                  ʹ�ñض�
   hmi_user_uart.c�еĴ��ڷ��ͽ��պ�����3�����������ڳ�ʼ��Uartinti()������1���ֽ�SendChar()��
   �����ַ���SendStrings().����ֲ������ƽ̨����Ҫ�޸ĵײ��
   ��������,����ֹ�޸ĺ������ƣ������޷���HMI������(hmi_driver.c)ƥ�䡣
--------------------------------------------------------------------------------------



----------------------------------------------------------------------------------------
                          1. ����STM32ƽ̨��������
----------------------------------------------------------------------------------------*/
#include "hmi_user_uart.h"
#include "cmd_queue.h"
#include "stdio.h"
#include "string.h"
u8    RecvFlag;
u8    RevcLen[2];
u32   Recvi;                  //unsigned int
u32   RevcLens;
u8 USART_RX_BUF[1024];   							//�������ݻ���
int RxCounter = 0;       			//�������ݼ���ָ��
u8 ABORT_Succeed = 0;     //��ֹ�ɹ���־

extern  vu8  RecvSucceed;     //volatile  unsigned char
extern  vu8  RecvBuf[100];

extern volatile uint32_t timer_tick_count;



//�������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;

};

FILE __stdout;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
_sys_exit(int x)
{
	x = x;
}
//�ض���fputc����
int fputc(int ch, FILE *f)
{
	while((USART2->SR&0X40)==0);//ѭ������,ֱ���������
	USART2->DR = (u8) ch;
	return ch;
}
#endif


/****************************************************************************
* ��    �ƣ� UartInit()
* ��    �ܣ� ���ڳ�ʼ��
* ��ڲ����� ��
* ���ڲ����� ��
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

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	//Usart2 NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);

	//Usart ��ʼ������
	USART_InitStructure.USART_BaudRate = BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =   USART_Mode_Tx|USART_Mode_Rx;

	/* USART configuration */
	//USART_InitStructure.USART_BaudRate = 115200;
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //�����ж�ʹ��
	USART_Cmd(USART1, ENABLE); //ʹ�ܴ���1

	//USART_InitStructure.USART_BaudRate = 115200;
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
	USART_Cmd(USART2, ENABLE); //ʹ�ܴ���2
}




/******************************************************************
	*���ƣ�void USART1_IRQHandler(void)
	*���룺��
	*�������
	*���ܣ�
	*˵����
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
							memcpy(RecvBuf,USART_RX_BUF,RxCounter); 	//���Ʋ�������
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
	*���ƣ�void USART1_SendString(uint8_t *ch)
	*���룺��
	*�������
	*���ܣ������ַ���
	*˵����
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
  *����:void USART1_SendData(uint8_t *ch,uint8_t Num)
	*���룺��
	*�������
	*���ܣ���������
	*˵����
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
* ��    �ƣ� SendChar()
* ��    �ܣ� ����1���ֽ�
* ��ڲ����� t  ���͵��ֽ�
* ���ڲ����� ��
 *****************************************************************/
void  SendChar(u8 t)
{
	USART_SendData(USART2,t);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
	while((USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET));//�ȴ����ڷ������
}
