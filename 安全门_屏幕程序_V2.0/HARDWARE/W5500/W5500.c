/**********************************************************************************
 * �ļ���  ��W5500.c
 * ����    ��W5500 ����������
 * ��汾  ��ST_v3.5
**********************************************************************************/

#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "W5500.h"
#include "string.h"
#include "Screen.h"
#include "uhf.h"
#include "hmi_driver.h"
#include "delay.h"
#include "beep.h"
#include "uhf.h"
#include "UHFhw_config.h"
#include <stdio.h>








/***************----- ��������������� -----***************/

unsigned char Phy_Addr[6]   =  {0x0c,0x29,0xab,0x7c,0x00,0x01};			//�����ַ(MAC)

unsigned char IP_Addr[4]    =  {192, 168, 0,  133};									//����IP��ַ
unsigned char Sub_Mask[4]   =  {255, 255, 255, 0};									//��������
unsigned char Gateway_IP[4] =  {192, 168, 0,  252};									  //����IP��ַ
uint16_t S0_Port  = 6000;																						//�˿�0�Ķ˿ں�(6000) (�ͻ��˶˿ں�)

unsigned char S0_DIP[4]     =  {192, 168, 0, 95};			        	  	//�˿�0Ŀ��IP��ַ(������IP��ַ)
uint16_t S0_DPort = 20000;																					//�˿�0Ŀ�Ķ˿ں�(�������˿ں�)

unsigned char UDP_DIPR[4]   =  {192, 168, 0, 95}  	;			//UDP(�㲥)ģʽ,Ŀ������IP��ַ
uint16_t UDP_DPORT = 20000	;			//UDP(�㲥)ģʽ,Ŀ�������˿ں�

/***************----- �˿ڵ�����ģʽ -----***************/
unsigned char S0_Mode =3;				//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
unsigned char S1_Mode =3;				//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
#define TCP_SERVER	0x00				//TCP������ģʽ
#define TCP_CLIENT	0x01				//TCP�ͻ���ģʽ 
#define UDP_MODE	  0x02				//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
unsigned char S0_State =0;			//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������)
unsigned char S1_State =0;			//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������)
#define S_INIT		0x01					//�˿���ɳ�ʼ�� 
#define S_CONN		0x02					//�˿��������,���������������� 

/***************----- �˿��շ����ݵ�״̬ -----***************/
unsigned char S0_Data;					//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ����������
unsigned char S1_Data;					//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ���������� 
#define S_RECEIVE	 0x01					//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK 0x02				//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/
unsigned char S0_Rx_Buffer[2048];	//�˿ڽ������ݻ����� 
unsigned char S0_Tx_Buffer[2048];	//�˿ڷ������ݻ����� 
unsigned char S1_Rx_Buffer[2048];	//�˿ڽ������ݻ����� 
unsigned char S1_Tx_Buffer[2048];	//�˿ڷ������ݻ����� 

unsigned char W5500_Interrupt;	//W5500�жϱ�־(0:���ж�,1:���ж�)

#define FeedBack_EPC    0xf1       //���ջظ�EPC
#define Alarm       		0xf2       //����ָ��
#define BookName   		  0xf3       //��ʾ����
#define INNUM           0xf4       //��������
#define OUTNUM          0xf5       //��������
#define LocalIP         0xf6       //����IP
#define ServerIP        0xf7       //������IP
#define AlarmMode       0xf8       //����ģʽ
#define PowerSet        0xf9       //��������
#define VolumeSet       0xfa       //��������
#define GetIP           0xfb       //��ȡIP
#define SetOutOrCont    0xfc       //���ó����������Ǹߵ͵�ƽ��Ч��������
#define FeedBack_TID    0xfd       //�ϴ�EPC��TID
#define INOUTNUM        0xfe       //��������
#define Relay1          0xe1       //�̵���1
#define Relay2          0xe2       //�̵���2
#define CleanNum        0xe3       //�����������
#define InvertNum       0xe4       //��ת��������
#define SetPassword     0xe5       //�޸���Ļ����
#define SetDoorPort     0xe6       //�����ŵĶ˿�
#define SetServerPort   0xe7       //���÷������Ķ˿�
#define NetHeartBeat    0xe8       //��������

u8             Serial;         //���
u8             FrameLength;    //���ݳ���
u8             FrameCMD;       //�յ�����
int             NetWait = 0;
u8             NetNotFound = 0; //���糬ʱ��־
u8             NetHeartFlag = 1;    //����������־
u8             FBStand_Content[1] = {0x02};
int KeepBookName_time = 0;
u8  BookNameKeep_flag = 0;


/*******************************************************************************
* ������  : W5500_TCP_SEND
* ����    : ͨ��W5500����TCP����
* ����    : uint8_t *ch:�������ݵ���ʼ��ַ   uint8_t Num:Ҫ���͵��ֽ���
* ���    : ��
* ����    : 1:�ɹ�  0:ʧ�� 
* ˵��    : ��
*******************************************************************************/
uint8_t W5500_TCP_SEND(uint8_t *ch,uint8_t Num)
{
		int i;
		if(S0_State == (S_INIT|S_CONN))					//����˿ڳ�ʼ�����
		{
				S0_Data &= ~S_TRANSMITOK;
				memcpy(S0_Tx_Buffer,ch, Num);	
			
#if DEBUG_PRINT  
				printf("����TCP���ݰ�:����Ϊ: ");
				for(i = 0;i<Num;i++)
				{
						printf(" %X",S0_Tx_Buffer[i]);
				}
				printf("\r\n\r\n");
#endif
				
				Write_SOCK_Data_Buffer(0, S0_Tx_Buffer, Num);//ָ��Socket(0~7)�������ݴ���,�˿�0����23�ֽ�����		
				return 1;	
		}
		else
		{		
				Socket_Init(0);		     //ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
				W5500_Socket_Set();		//W5500�˿�Connect()
				return 0;
		}
}

/*******************************************************************************
* ������  : W5500_UDP_SEND   W5500_UDP_SEND
* ����    : ͨ��W5500����TCP����      UDP
* ����    : uint8_t *ch:�������ݵ���ʼ��ַ   uint8_t Num:Ҫ���͵��ֽ���
* ���    : ��
* ����    : 1:�ɹ�  0:ʧ��
* ˵��    : ��
*******************************************************************************/
uint8_t W5500_UDP_SEND(uint8_t *ch,uint8_t Num)
{
		int i;
		if(S1_State == (S_INIT|S_CONN))					//����˿ڳ�ʼ�����
		{
				S1_Data &= ~S_TRANSMITOK;
				memcpy(S1_Tx_Buffer,ch, Num);	
			
#if DEBUG_PRINT  
				printf("����UDP���ݰ�:����Ϊ: ");
				for(i = 0;i<Num;i++)
				{
						printf(" %X",S1_Tx_Buffer[i]);
				}
				printf("\r\n\r\n");
#endif
				
				Write_SOCK_Data_Buffer(1, S1_Tx_Buffer, Num);//ָ��Socket(0~7)�������ݴ���,�˿�1����23�ֽ�����		
				return 1;	
		}
		else
		{		
				Socket_Init(1);		     //ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
				W5500_Socket_Set();		//W5500�˿�Connect()
				return 0;
		}
}
/*******************************************************************************
* ������  : W5500_HARDWARE_INIT
* ����    : W5500Ӳ����ʼ��
* ����    : ��
* ���    : ��
* ����    : ��
* ˵��    : ��
*******************************************************************************/
void W5500_HARDWARE_INIT(void)
{

	W5500_GPIO_Configuration();	 //W5500 GPIO��ʼ������
	SPI_Configuration();				//W5500 SPI��ʼ������(STM32 SPI1)
	Load_Net_Parameters();		 //װ���������
	W5500_Hardware_Reset();		//Ӳ����λW5500
	W5500_NVIC_Configuration();		//W5500�ⲿ�ж�INT��ʼ��
	W5500_Init();			       //��ʼ��W5500�Ĵ�������
	//Detect_Gateway();	    //������ط����� (Ӧ��������,����Ҫ������������)
	Socket_Init(0);		     //ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
	W5500_Socket_Set();		//W5500�˿�Connect()

}


/**************************************************************************

����ԭ��:    	uint8_t CalcDeviceID()
��������:	    ���ݲ����̼����豸ID(IP��ַ)
�������:			��

�������:     ��
����ֵ:       ���ز�����ֵ,��Ϊ�豸ID��IP��ַ��MAC��ַ���һ��ʹ��(����16���Ƽ���)
( GPIO: SW1:PC14 SW2:PC15 SW3:PA0 SW4:PC5 SW5:PA11 SW6:PA12 )
**************************************************************************/
uint8_t CalcDeviceID()
{
	uint8_t DeviceID = 0;

	//SWxΪ0ʱ��Ч, DeviceID = 32*SW1 + 16*SW2 + 8*SW3 + 4*SW4 + 2*SW5 + 1*SW6
	DeviceID += (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7) !=0 ) ? 0 : 1;   //SW1
 	DeviceID += (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) !=0 ) ? 0 : 2;   //SW2
 	DeviceID += (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9 ) !=0 ) ? 0 : 4;	  //SW3
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11 ) !=0 ) ? 0 : 8;	  //SW4
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) !=0 ) ? 0 : 2;    //SW5
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12) !=0 ) ? 0 : 1;    //SW6
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14) !=0 ) ? 0 : 32;   //SW1
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15) !=0 ) ? 0 : 16;   //SW2
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0 ) !=0 ) ? 0 : 8;	  //SW3
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5 ) !=0 ) ? 0 : 4;	  //SW4
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11) !=0 ) ? 0 : 2;    //SW5
// 	DeviceID += (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12) !=0 ) ? 0 : 1;    //SW6
	//printf("DeviceID Ϊ:%d\r\n",DeviceID);

	return DeviceID;

}

/*******************************************************************************
* ������  : Load_Net_Parameters
* ����    : װ���������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢�����ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
*******************************************************************************/
void Load_Net_Parameters(void)
{
//	IP_Addr[3] = ( CalcDeviceID() == 0) ? IP_Addr[3]:CalcDeviceID();  			 //��ȡ������ֵ,���Ϊ0����Ĭ��IP,�����Ϊ0����IP��ַΪ������ֵ

//	Phy_Addr[5] = ( CalcDeviceID() == 0) ? Phy_Addr[5]:CalcDeviceID();      //��ȡ������ֵ,���Ϊ0����Ĭ��MAC,�����Ϊ0����MAC��ַΪ������ֵ

	S0_Mode=TCP_CLIENT;//���ض˿�0�Ĺ���ģʽ,TCP�ͻ���ģʽ
	S1_Mode=UDP_MODE;	 //���ض˿�1�Ĺ���ģʽ,UDPģʽ

//	S0_Mode=UDP_MODE;                                               //UDPģʽ
}

/*******************************************************************************
* ������  : W5500_Socket_Set
* ����    : W5500�˿ڳ�ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ֱ�����4���˿�,���ݶ˿ڹ���ģʽ,���˿�����TCP��������TCP�ͻ��˻�UDPģʽ.
*			�Ӷ˿�״̬�ֽ�Socket_State�����ж϶˿ڵĹ������
*******************************************************************************/
void W5500_Socket_Set(void)
{	
		if(S0_State==0x00)										//�˿�0��ʼ������
		{
				if(S0_Mode==TCP_SERVER)				//TCP������ģʽ 
				{
						if(Socket_Listen(0)==TRUE)
						{
								S0_State=S_INIT;
						}				
						else
						{
								S0_State=0;
						}							
				}
				else if(S0_Mode==TCP_CLIENT)	//TCP�ͻ���ģʽ 
				{
						if(Socket_Connect(0)==TRUE)
						{
							NetWait = 0;
								S0_State=S_INIT;
						}							
						else
						{							
								S0_State=0;
						}							
				}
				else													//UDPģʽ 
				{
						if(Socket_UDP(0)==TRUE)
						{
								S0_State=S_INIT|S_CONN;
						}							
						else
						{							
								S0_State=0;
						}							
				}
		}
		
		
		if(S1_State==0x00)										//�˿�1��ʼ������
		{
				if(S1_Mode==TCP_SERVER)				//TCP������ģʽ 
				{
						if(Socket_Listen(1)==TRUE)
						{
								S1_State=S_INIT;
						}				
						else
						{
								S1_State=0;
						}							
				}
				else if(S1_Mode==TCP_CLIENT)	//TCP�ͻ���ģʽ 
				{
						if(Socket_Connect(1)==TRUE)
						{
								S1_State=S_INIT;
						}							
						else
						{							
								S1_State=0;
						}							
				}
				else													//UDPģʽ 
				{
						if(Socket_UDP(1)==TRUE)
						{
								S1_State=S_INIT|S_CONN;
						}							
						else
						{							
								S1_State=0;
						}							
				}
		}
}

/*******************************************************************************
* ������  : Process_Socket_Data
* ����    : W5500���ղ����ͽ��յ�������
* ����    : s:�˿ں�
* ���    : ��
* ����ֵ  : ��
* ˵��    : �������ȵ���S_rx_proces s()��W5500�Ķ˿ڽ������ݻ�������ȡ����,
*			Ȼ�󽫶�ȡ�����ݴ�Rx_Buffer������Temp_Buffer���������д���
*			������ϣ������ݴ�Temp_Buffer������Tx_Buffer������������S_tx_process()
*			�������ݡ�
*******************************************************************************/
int Process_Socket_Data(SOCKET s)
{
	int i;
	unsigned short size;
	uint8_t p_Counter = 0;
	
	if(s == 0 )            										  //Socket0
	{
		size=Read_SOCK_Data_Buffer(s, S0_Rx_Buffer);
		
#if DEBUG_PRINT  
					printf("Socket0���յ����ݰ�:����Ϊ: ");
					for(i = 0;i<size;i++)
					{
							printf(" %X",S0_Rx_Buffer[i]);
					}
					printf("\r\n\r\n");
#endif
				
					
		DecodeTCPFrame(S0_Rx_Buffer,size);   	 //�����յ���TCP���ݰ�
					
	}
	
  if(s == 1)
	{
		size=Read_SOCK_Data_Buffer(s, S1_Rx_Buffer);
		for(i=0; i<4; i++)
		{
			UDP_DIPR[i]= S1_Rx_Buffer[p_Counter++];    //��ȡIP����
		}
	#if DEBUG
		printf("���յ�UDP_IP���ݰ�:����Ϊ: ");

		printf(" IP��ַ   : %d.%d.%d.%d\r\n", UDP_DIPR[0],UDP_DIPR[1],UDP_DIPR[2],UDP_DIPR[3]);

		printf("\r\n\r\n");
	#endif
		UDP_DPORT = (S1_Rx_Buffer[p_Counter++]<<8) ;  	 	       //��ȡ�˿ں�
		UDP_DPORT += (S1_Rx_Buffer[p_Counter++]) ;
	#if DEBUG
		printf("���յ�UDP_port���ݰ�:����Ϊ: ");
		printf(" UDP�˿�Ϊ:%d \r\n",UDP_DPORT);
		printf("\r\n\r\n");
		printf("���յ�UDP���ݰ�:����Ϊ: ");
		for(i = 0; i<size; i++)
		{
			printf(" %X",S1_Rx_Buffer[i]);
		}
		printf("\r\n\r\n");
	#endif
		DecodeUDPFrame(&S1_Rx_Buffer[8],size-8);   	 //�����յ���UDP���ݰ�
		//Write_SOCK_Data_Buffer(s,&Rx_Buffer[8],size-8);
		//Write_SOCK_Data_Buffer(s,Rx_Buffer,size);
	}
		return size;

}


/*******************************************************************************
* ������  : W5500_GPIO_Configuration
* ����    : W5500 GPIO��ʼ������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);

	/* W5500_RST���ų�ʼ������ */
	GPIO_InitStructure.GPIO_Pin  = W5500_RST;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(W5500_RST_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);

	/* W5500_INT���ų�ʼ������ */
	GPIO_InitStructure.GPIO_Pin = W5500_INT;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(W5500_INT_PORT, &GPIO_InitStructure);

	/* Connect EXTI Line4 to PC4 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);

	/* PC4 as W5500 interrupt input */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
* ������  : W5500_NVIC_Configuration
* ����    : W5500 ���������ж����ȼ�����
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the EXTI9_5_IRQn Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* ������  : W5500_NVIC_Close_Configuration
* ����    : W5500 ���������ж����ȼ�����
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : �ر��ж�
*******************************************************************************/
void W5500_NVIC_Close_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the EXTI9_5_IRQn Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
* ������  : EXTI4_IRQHandler
* ����    : �ж���4�жϷ�����(W5500 INT�����жϷ�����)
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line8);
#if DEBUG
		printf("�����ⲿ�жϷ������\r\n");
#endif
		W5500_Interrupt=1;
		W5500_Interrupt_Process();  //����W5500�жϷ��������

	}
}

/*******************************************************************************
* ������  : SPI_Configuration
* ����    : W5500 SPI��ʼ������(STM32 SPI1)
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void SPI_Configuration(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef   	SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

	/* ��ʼ��SCK��MISO��MOSI���� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	/* ��ʼ��CS���� */
	GPIO_InitStructure.GPIO_Pin = W5500_SCS;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(W5500_SCS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);

	/* ��ʼ������STM32 SPI1 */
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;												//����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;										//SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;													//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;												//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;														//NSS���ⲿ�ܽŹ���
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//������Ԥ��ƵֵΪ2
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;									//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial=7;														//CRC����ʽΪ7
	SPI_Init(SPI1,&SPI_InitStructure);																//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI1�Ĵ���

	SPI_Cmd(SPI1,ENABLE);	//STM32ʹ��SPI1
}



/*******************************************************************************
* ������  : SPI1_Send_Byte
* ����    : SPI1����1���ֽ�����
* ����    : dat:�����͵�����
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void SPI1_Send_Byte(unsigned char dat)
{
	SPI_I2S_SendData(SPI1,dat);																		//д1���ֽ�����
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);//�ȴ����ݼĴ�����
}

/*******************************************************************************
* ������  : SPI1_Send_Short
* ����    : SPI1����2���ֽ�����(16λ)
* ����    : dat:�����͵�16λ����
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void SPI1_Send_Short(unsigned short dat)
{
	SPI1_Send_Byte(dat/256);	//д���ݸ�λ
	SPI1_Send_Byte(dat);			//д���ݵ�λ
}

/*******************************************************************************
* ������  : Write_W5500_1Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);											//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM1|RWB_WRITE|COMMON_R);	//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
	SPI1_Send_Byte(dat);											//д1���ֽ�����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);	//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_2Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д2���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);											//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM2|RWB_WRITE|COMMON_R);	//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
	SPI1_Send_Short(dat);											//д16λ����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 	//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_nByte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���дn���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,*dat_ptr:��д�����ݻ�����ָ��,size:��д������ݳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);											//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(VDM|RWB_WRITE|COMMON_R);		//ͨ��SPI1д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���

	for(i=0; i<size; i++)												//ѭ������������size���ֽ�����д��W5500
	{
		SPI1_Send_Byte(*dat_ptr++);							//дһ���ֽ�����
	}

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);	//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_1Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);													//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM1|RWB_WRITE|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	SPI1_Send_Byte(dat);													//д1���ֽ�����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 			//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_2Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);													//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM2|RWB_WRITE|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	SPI1_Send_Short(dat);													//д16λ����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 			//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_4Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д4���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,*dat_ptr:��д���4���ֽڻ�����ָ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);													//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM4|RWB_WRITE|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,4���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

	SPI1_Send_Byte(*dat_ptr++);										//д��1���ֽ�����
	SPI1_Send_Byte(*dat_ptr++);										//д��2���ֽ�����
	SPI1_Send_Byte(*dat_ptr++);										//д��3���ֽ�����
	SPI1_Send_Byte(*dat_ptr++);										//д��4���ֽ�����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);			//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Read_W5500_1Byte
* ����    : ��W5500ָ����ַ�Ĵ�����1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);													//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM1|RWB_READ|COMMON_R);				//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,������,ѡ��ͨ�üĴ���

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);													//����һ��������
	i=SPI_I2S_ReceiveData(SPI1);									//��ȡ1���ֽ�����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);			//��W5500��SCSΪ�ߵ�ƽ
	return i;																			//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_1Byte
* ����    : ��W5500ָ���˿ڼĴ�����1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);													//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM1|RWB_READ|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);													//����һ��������
	i=SPI_I2S_ReceiveData(SPI1);									//��ȡ1���ֽ�����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);			//��W5500��SCSΪ�ߵ�ƽ
	return i;																			//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_2Byte
* ����    : ��W5500ָ���˿ڼĴ�����2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����2���ֽ�����(16λ)
* ˵��    : ��
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	unsigned short i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);			//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(reg);														//ͨ��SPI1д16λ�Ĵ�����ַ
	SPI1_Send_Byte(FDM2|RWB_READ|(s*0x20+0x08));		//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);														//����һ��������
	i=SPI_I2S_ReceiveData(SPI1);										//��ȡ��λ����
	SPI1_Send_Byte(0x00);														//����һ��������
	i*=256;
	i+=SPI_I2S_ReceiveData(SPI1);										//��ȡ��λ����

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);				//��W5500��SCSΪ�ߵ�ƽ
	return i;																				//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_SOCK_Data_Buffer
* ����    : ��W5500�������ݻ������ж�ȡ����
* ����    : s:�˿ں�,*dat_ptr:���ݱ��滺����ָ��
* ���    : ��
* ����ֵ  : ��ȡ�������ݳ���,rx_size���ֽ�
* ˵��    : ��
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size==0) return 0;											//û���յ������򷵻�
	if(rx_size>1460) rx_size=1460;

	offset=Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);												//����ʵ�ʵ������ַ

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(offset);											//д16λ��ַ
	SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));		//д�����ֽ�,N���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���
	j=SPI_I2S_ReceiveData(SPI1);

	if((offset+rx_size)<S_RX_SIZE)								//�������ַδ����W5500���ջ������Ĵ���������ַ
	{
		for(i=0; i<rx_size; i++)											//ѭ����ȡrx_size���ֽ�����
		{
			SPI1_Send_Byte(0x00);											//����һ��������
			j=SPI_I2S_ReceiveData(SPI1);							//��ȡ1���ֽ�����
			*dat_ptr=j;																//����ȡ�������ݱ��浽���ݱ��滺����
			dat_ptr++;																//���ݱ��滺����ָ���ַ����1
		}
	}
	else																					//�������ַ����W5500���ջ������Ĵ���������ַ
	{
		offset=S_RX_SIZE-offset;
		for(i=0; i<offset; i++)												//ѭ����ȡ��ǰoffset���ֽ�����
		{
			SPI1_Send_Byte(0x00);											//����һ��������
			j=SPI_I2S_ReceiveData(SPI1);							//��ȡ1���ֽ�����
			*dat_ptr=j;																//����ȡ�������ݱ��浽���ݱ��滺����
			dat_ptr++;																//���ݱ��滺����ָ���ַ����1
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 		//��W5500��SCSΪ�ߵ�ƽ

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);	//��W5500��SCSΪ�͵�ƽ

		SPI1_Send_Short(0x00);											//д16λ��ַ
		SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));	//д�����ֽ�,N���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���
		j=SPI_I2S_ReceiveData(SPI1);

		for(; i<rx_size; i++)													//ѭ����ȡ��rx_size-offset���ֽ�����
		{
			SPI1_Send_Byte(0x00);											//����һ��������
			j=SPI_I2S_ReceiveData(SPI1);							//��ȡ1���ֽ�����
			*dat_ptr=j;																//����ȡ�������ݱ��浽���ݱ��滺����
			dat_ptr++;																//���ݱ��滺����ָ���ַ����1
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 			//��W5500��SCSΪ�ߵ�ƽ

	offset1+=rx_size;															//����ʵ�������ַ,���´ζ�ȡ���յ������ݵ���ʼ��ַ
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);				//����������������
	return rx_size;																//���ؽ��յ����ݵĳ���
}

/*******************************************************************************
* ������  : Write_SOCK_Data_Buffer
* ����    : ������д��W5500�����ݷ��ͻ�����
* ����    : s:�˿ں�,*dat_ptr:���ݱ��滺����ָ��,size:��д�����ݵĳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	//�����UDPģʽ,�����ڴ�����Ŀ��������IP�Ͷ˿ں�
	if((Read_W5500_SOCK_1Byte(s,Sn_MR)&0x0f) != SOCK_UDP)										//���Socket��ʧ��
	{
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);													//����Ŀ������IP
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT);	//����Ŀ�������˿ں�
#if DEBUG
		printf("  UDP ģʽ\r\n");
#endif
	}

	offset=Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);															//����ʵ�ʵ������ַ

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);					//��W5500��SCSΪ�͵�ƽ

	SPI1_Send_Short(offset);														//д16λ��ַ
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));				//д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

	if((offset+size)<S_TX_SIZE)													//�������ַδ����W5500���ͻ������Ĵ���������ַ
	{
		for(i=0; i<size; i++)																//ѭ��д��size���ֽ�����
		{
			SPI1_Send_Byte(*dat_ptr++);											//д��һ���ֽڵ�����
		}
	}
	else																								//�������ַ����W5500���ͻ������Ĵ���������ַ
	{
		offset=S_TX_SIZE-offset;
		for(i=0; i<offset; i++)															//ѭ��д��ǰoffset���ֽ�����
		{
			SPI1_Send_Byte(*dat_ptr++);											//д��һ���ֽڵ�����
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 					//��W5500��SCSΪ�ߵ�ƽ

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);				//��W5500��SCSΪ�͵�ƽ

		SPI1_Send_Short(0x00);														//д16λ��ַ
		SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));			//д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

		for(; i<size; i++)																	//ѭ��д��size-offset���ֽ�����
		{
			SPI1_Send_Byte(*dat_ptr++);											//д��һ���ֽڵ�����
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);						//��W5500��SCSΪ�ߵ�ƽ

	offset1+=size;																			//����ʵ�������ַ,���´�д���������ݵ��������ݻ���������ʼ��ַ
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);							//����������������
}

/*******************************************************************************
* ������  : W5500_Hardware_Reset
* ����    : Ӳ����λW5500
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : W5500�ĸ�λ���ű��ֵ͵�ƽ����500us����,������ΧW5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);		//��λ��������
	delay_ms(50);
	GPIO_SetBits(W5500_RST_PORT, W5500_RST);			//��λ��������
	delay_ms(200);
	//while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);		//�ȴ���̫���������

}

/*******************************************************************************
* ������  : W5500_Init
* ����    : ��ʼ��W5500�Ĵ�������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʹ��W5500֮ǰ���ȶ�W5500��ʼ��
*******************************************************************************/
void W5500_Init(void)
{
	u8 i=0;

	Write_W5500_1Byte(MR, RST);//�����λW5500,��1��Ч,��λ���Զ���0
	delay_ms(10);//��ʱ10ms,�Լ�����ú���

	//��������(Gateway)��IP��ַ,Gateway_IPΪ4�ֽ�unsigned char����,�Լ�����
	//ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet
	Write_W5500_nByte(GAR, Gateway_IP, 4);

	//������������(MASK)ֵ,SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����
	//��������������������
	Write_W5500_nByte(SUBR,Sub_Mask,4);

	//���������ַ,PHY_ADDRΪ6�ֽ�unsigned char����,�Լ�����,����Ψһ��ʶ�����豸�������ֵַ
	//�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	//����Լ����������ַ��ע���һ���ֽڱ���Ϊż��
	Write_W5500_nByte(SHAR,Phy_Addr,6);

	//���ñ�����IP��ַ,IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����
	//ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����
	Write_W5500_nByte(SIPR,IP_Addr,4);

	//���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5500�����ֲ�
	for(i=0; i<8; i++)
	{
		Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
	}

	//��������ʱ�䣬Ĭ��Ϊ2000(200ms)
	//ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
	Write_W5500_2Byte(RTR, 0x07d0);

	//�������Դ�����Ĭ��Ϊ8��
	//����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
	Write_W5500_1Byte(RCR,8);

	//�����жϣ��ο�W5500�����ֲ�ȷ���Լ���Ҫ���ж�����
	//IMR_CONFLICT��IP��ַ��ͻ�쳣�ж�,IMR_UNREACH��UDPͨ��ʱ����ַ�޷�������쳣�ж�
	//������Socket�¼��жϣ�������Ҫ���
	Write_W5500_1Byte(IMR,IM_IR7 | IM_IR6);		 //����IP��ַ��ͻ��Ŀ���ַ���ɴ��ж�
	Write_W5500_1Byte(SIMR,S0_IMR);						 //����Socket0�ж�
	Write_W5500_SOCK_1Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	Write_W5500_SOCK_1Byte(0,Sn_KPALVTR,0x01);			//ÿ��5s����KeepAlive������

}

/*******************************************************************************
* ������  : Detect_Gateway
* ����    : ������ط�����
* ����    : ��
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ��
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=IP_Addr[0]+1;
	ip_adde[1]=IP_Addr[1]+1;
	ip_adde[2]=IP_Addr[2]+1;
	ip_adde[3]=IP_Addr[3]+1;

	//������ؼ���ȡ���ص������ַ
	Write_W5500_SOCK_4Byte(0,Sn_DIPR,ip_adde);			//��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ
	Write_W5500_SOCK_1Byte(0,Sn_MR,MR_UDP);					//����socketΪUDPģʽ
	Write_W5500_SOCK_1Byte(0,Sn_CR,OPEN);						//��Socket
	delay_ms(5);																		//��ʱ5ms

	if(Read_W5500_SOCK_1Byte(0,Sn_SR) != SOCK_INIT)	//���socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);				//�򿪲��ɹ�,�ر�Socket
		return FALSE;																	//����FALSE(0x00)
	}

	Write_W5500_SOCK_1Byte(0,Sn_CR,CONNECT);				//����SocketΪConnectģʽ

	do
	{
		u8 j=0;
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);							//��ȡSocket0�жϱ�־�Ĵ���
		if(j!=0)
			Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		delay_ms(5);																			//��ʱ5ms
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;
		}
		else if(Read_W5500_SOCK_1Byte(0,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);			//�ر�Socket
			return TRUE;
		}
	}
	while(1);
}

/*******************************************************************************
* ������  : Socket_Init
* ����    : ָ��Socket(0~7)��ʼ��
* ����    : s:����ʼ���Ķ˿�
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	//���÷�Ƭ���ȣ��ο�W5500�����ֲᣬ��ֵ���Բ��޸�
	Write_W5500_SOCK_2Byte(s, Sn_MSSR, 1460);//����Ƭ�ֽ���=1460(0x5b4)
	//����ָ���˿�
	switch(s)
	{
	case 0:
		//���ö˿�0�Ķ˿ں�
		Write_W5500_SOCK_2Byte(s, Sn_PORT, S0_Port);
		//���ö˿�0Ŀ��(Զ��)�˿ں�
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT);
		//���ö˿�0Ŀ��(Զ��)IP��ַ
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, S0_DIP);

		break;

	case 1:
			//���ö˿�0�Ķ˿ں�
			Write_W5500_SOCK_2Byte(s, Sn_PORT, S0_Port);
			//���ö˿�0Ŀ��(Զ��)�˿ں�
			Write_W5500_SOCK_2Byte(s, Sn_DPORTR, S0_DPort);
			//���ö˿�0Ŀ��(Զ��)IP��ַ
			Write_W5500_SOCK_4Byte(s, Sn_DIPR, S0_DIP);	
		break;

	case 2:
		break;

	case 3:
		break;

	case 4:
		break;

	case 5:
		break;

	case 6:
		break;

	case 7:
		break;

	default:
		break;
	}
}

/*******************************************************************************
* ������  : Socket_Connect
* ����    : ����ָ��Socket(0~7)Ϊ�ͻ�����Զ�̷���������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڿͻ���ģʽʱ,���øó���,��Զ�̷�������������
*			����������Ӻ���ֳ�ʱ�жϣ��������������ʧ��,��Ҫ���µ��øó�������
*			�ó���ÿ����һ��,�������������һ������
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);					//����socketΪTCPģʽ
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);						//��Socket
	delay_ms(5);																		//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)		//���socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);				//�򿪲��ɹ�,�ر�Socket
		return FALSE;																	//����FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);				//����SocketΪConnectģʽ
	return TRUE;																		//����TRUE,���óɹ�
}

/*******************************************************************************
* ������  : Socket_Listen
* ����    : ����ָ��Socket(0~7)��Ϊ�������ȴ�Զ������������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڷ�����ģʽʱ,���øó���,�ȵ�Զ������������
*			�ó���ֻ����һ��,��ʹW5500����Ϊ������ģʽ
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);					//����socketΪTCPģʽ
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);						//��Socket
	delay_ms(5);																				//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)		//���socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);				//�򿪲��ɹ�,�ر�Socket
		return FALSE;																	//����FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,LISTEN);					//����SocketΪ����ģʽ
	delay_ms(5);																				//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_LISTEN)	//���socket����ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);				//���ò��ɹ�,�ر�Socket
		return FALSE;																	//����FALSE(0x00)
	}

	return TRUE;

	//���������Socket�Ĵ򿪺�������������,����Զ�̿ͻ����Ƿ�������������,����Ҫ�ȴ�Socket�жϣ�
	//���ж�Socket�������Ƿ�ɹ����ο�W5500�����ֲ��Socket�ж�״̬
	//�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�
}

/*******************************************************************************
* ������  : Socket_UDP
* ����    : ����ָ��Socket(0~7)ΪUDPģʽ
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ���Socket������UDPģʽ,���øó���,��UDPģʽ��,Socketͨ�Ų���Ҫ��������
*			�ó���ֻ����һ�Σ���ʹW5500����ΪUDPģʽ
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);				//����SocketΪUDPģʽ*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);					//��Socket*/
	delay_ms(5);																			//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)	//���Socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);			//�򿪲��ɹ�,�ر�Socket
		return FALSE;																//����FALSE(0x00)
	}
	else
		return TRUE;

	//���������Socket�Ĵ򿪺�UDPģʽ����,������ģʽ��������Ҫ��Զ��������������
	//��ΪSocket����Ҫ��������,�����ڷ�������ǰ����������Ŀ������IP��Ŀ��Socket�Ķ˿ں�
	//���Ŀ������IP��Ŀ��Socket�Ķ˿ں��ǹ̶���,�����й�����û�иı�,��ôҲ��������������
}

/*******************************************************************************
* ������  : W5500_Interrupt_Process
* ����    : W5500�жϴ��������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	unsigned char i,j;

IntDispose:
	W5500_Interrupt=0;								//�����жϱ�־
	i = Read_W5500_1Byte(IR);					//��ȡ�жϱ�־�Ĵ���
	Write_W5500_1Byte(IR, (i&0xf0));	//��д����жϱ�־

	if((i & CONFLICT) == CONFLICT)		//IP��ַ��ͻ�쳣����
	{
		//�Լ���Ӵ���
	}

	if((i & UNREACH) == UNREACH)			//UDPģʽ�µ�ַ�޷������쳣����
	{
		//�Լ���Ӵ���
	}

	i=Read_W5500_1Byte(SIR);										//��ȡ�˿��жϱ�־�Ĵ���
	if((i & S0_INT) == S0_INT)									//Socket0�¼�����
	{
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);					//��ȡSocket0�жϱ�־�Ĵ���
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		if(j&IR_CON)															//��TCPģʽ��,Socket0�ɹ�����
		{
			S0_State |= S_CONN;											//��������״̬0x02,�˿�������ӣ�����������������
#if DEBUG
			printf("TCP������������ɣ�����������������\r\n\r\n");
#endif
			delay_ms(500);
//			SPI_FLASH_BufferRead(ShowG, 0x16000, 300);
//			ShowContent(1,ShowData);
		}
		if(j&IR_DISCON)														//��TCPģʽ��Socket�Ͽ����Ӵ���
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);	//�رն˿�,�ȴ����´�����
			Socket_Init(0);													//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
			S0_State=0;															//��������״̬0x00,�˿�����ʧ��
			W5500_Socket_Set();
#if DEBUG
			printf("����������ʧ��,��������\r\n\r\n");
#endif
			delay_ms(500);
			//		ShowContent(22,ShowData);        				//������δ����LED��ʾ

		}
		if(j&IR_SEND_OK)													//Socket0���ݷ������,�����ٴ�����S_tx_process()������������
		{
			S0_Data|=S_TRANSMITOK;									//�˿ڷ���һ�����ݰ����
#if DEBUG
			printf("UDP�˿ڷ���һ�����ݰ����\r\n\r\n");
#endif
		}
		if(j&IR_RECV)															//Socket���յ�����,��������S_rx_process()����
		{
			S0_Data|=S_RECEIVE;											//�˿ڽ��յ�һ�����ݰ�
#if DEBUG
			printf("UDP�˿ڽ��յ�һ�����ݰ�\r\n\r\n");
#endif
			Process_Socket_Data(0);									//W5500�����жϴ������

			S0_Data &= ~S_RECEIVE;
		}
		if(j&IR_TIMEOUT)													//Socket���ӻ����ݴ��䳬ʱ����
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);	// �رն˿�,�ȴ����´�����
			Socket_Init(0);													//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
#if DEBUG
			printf("Socket���ӻ����ݴ��䳬ʱ���� \r\n\r\n");
#endif
			delay_ms(500);
			if((Read_W5500_1Byte(PHYCFGR)&LINK)==0)				//����δ����
			{
				//	ShowContent(23,ShowData);	//���߶Ͽ�...
			}
			else
			{
				//ShowContent(22,ShowData);        				//������δ����LED��ʾ
			}
			S0_State=0;															//��������״̬0x00,�˿�����ʧ��
			W5500_Socket_Set();											//W5500�˿�Connect()
		}
	}

	if(Read_W5500_1Byte(SIR) != 0)
		goto IntDispose;
}


/**********************************************************************************************************

����ԭ��:    	void DecodeUDPFrame(uint8_t *Frame,uint8_t Len)
��������:	    ���ݽ��뵽��UDPЭ��֡,ѡ��ִ�в�ͬ�Ĺ���
�������:			uint8_t *Frame: TCPЭ��֡��ʼָ��
							uint8_t Len: 		֡����

�������:      ��
����ֵ:        ��
***********************************************************************************************************/


void DecodeUDPFrame(uint8_t *Frame,uint8_t Len)
{
	u8 i;
	u8 p_Counter = 0;
	u8 FrameContent[50];     //
	//u8 tempEPC[900];
	u16 EndFrame;
	uint16_t CAL_CRC16,RECV_CRC16;
	//TimeOutCounter = 0;    //��������
	if(Frame[p_Counter++] == 0x05 )                    //�ж�֡ͷ֡β
	{
		FrameLength = Frame[p_Counter++];                //��ȡ֡����
		FrameCMD    = Frame[p_Counter++];			           //��ȡCMD
		for(i=0; i<Len-6; i++)
			{
				FrameContent[i] = Frame[p_Counter++];        //��ȡ��������
			}  
    RECV_CRC16 =  (Frame[p_Counter++]<<8);           //��ȡCRC16У��
		RECV_CRC16 += (Frame[p_Counter++]);
		EndFrame = 0x06;                                 //��ȡ֡β
		//CAL_CRC16 = cal_crc16(&Frame[1],Len-4); 	       //����CRC16У��

#if DEBUG
			printf("���յ���UDP֡����Ϊ: %d ,����Ϊ: %X ,����Ϊ: %X\r\n\r\n",FrameLength,FrameCMD,FrameContent);
			printf("����õ���CRC16Ϊ: %X  ,",CAL_CRC16);
			printf("���յ���CRC16Ϊ: %X\r\n\r\n",RECV_CRC16);
#endif

			//if(RECV_CRC16 == CAL_CRC16)            //����CRC16У��
				switch(FrameCMD)        				  	//�����������ͽ�����Ӧ�Ĳ���
				{
				case FeedBack_EPC:           //�ϴ�EPC
				  if(BookCounter !=0)
					{
						int j,k, tempLen;
						tempLen = 0;
						for(j=0; j< BookCounter; j++)
						{
							for(k=0;k<12+1; k++)
							{
								tempEPC[tempLen++] = TotalEPC[j][k];
							}
						}
						Feedback(BookCounter*13+6,FrameCMD,tempEPC);
						BookCounter = 0;
						memset(TotalEPC,0,sizeof(TotalEPC));
						memset(tempEPC,0,sizeof(tempEPC));
					}
					else
					{
						FBStand_Content[0] = 0x02;
					  Feedback(Len,FrameCMD,FBStand_Content);
					}
					break;
				case Alarm:                  //����ָ��
					//NetCmd = 1;
					StartAlarm();
				  FBStand_Content[0] = 0x02;
					Feedback(Len,FrameCMD,FBStand_Content);
					break;
				case BookName:
					FBStand_Content[0] = 0x02;
					SetTextValue(0,4,FrameContent);
				  Feedback(Len,FrameCMD,FBStand_Content);
					break;
				case INNUM:
					sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
					Feedback(0x10,FrameCMD,InNumtempW);
				  break;
				case OUTNUM:
					sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
					Feedback(0x10,FrameCMD,OutNumtempW);
				  break;
				case LocalIP:                  //���±���IP
					IpAddTemp1[0] = FrameContent[0];
				  IpAddTemp1[1] = FrameContent[1];
				  IpAddTemp1[2] = FrameContent[2];
				  IpAddTemp1[3] = FrameContent[3];
				  W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
					Feedback(0x07,FrameCMD,FBStand_Content);
				  break;
				case ServerIP:                  //���±���IP
					ServerIpTemp1[0] = FrameContent[0];
				  ServerIpTemp1[1] = FrameContent[1];
				  ServerIpTemp1[2] = FrameContent[2];
				  ServerIpTemp1[3] = FrameContent[3];
				  W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
					Feedback(0x07,FrameCMD,FBStand_Content);
				  break;
				case AlarmMode:
					Door_Mode = Door_Modetemp[0] = FrameContent[0];
		      W25QXX_Write((u8*)Door_Modetemp,Door_Modeadd,sizeof(Door_Modetemp));
					Feedback(Len,FrameCMD,FBStand_Content);
				  break;
				case PowerSet:
					Powertemp[0] =  FrameContent[0];
					W25QXX_Write((u8*)Powertemp,Poweradd,sizeof(Powertemp));
					Feedback(Len,FrameCMD,FrameContent);
					SoftReset();
					break;
				case VolumeSet:
					Volumetemp[0] = FrameContent[0];
					if(Volumetemp[0] > 100)
						{
							Volumetemp[0] = 50;
						}
					SetMeterValue(4,2,Volumetemp[0]); //���½�������ֵ
					AdjustVolume(Volumetemp[0]);                               //��������
					W25QXX_Write((u8*)Volumetemp,Volumeadd,sizeof(Volumetemp));
					Feedback(Len,FrameCMD,FrameContent);
					break;
				case GetIP:
					FrameContent[0] = IpAddTemp1[0];
					FrameContent[1] = IpAddTemp1[1];
					FrameContent[2] = IpAddTemp1[2];
					FrameContent[3] = IpAddTemp1[3];
					FrameContent[4] = 0x00;
					FrameContent[5] = ServerIpTemp1[0];
					FrameContent[6] = ServerIpTemp1[1];
					FrameContent[7] = ServerIpTemp1[2];
					FrameContent[8] = ServerIpTemp1[3];
					Feedback(0x0f,FrameCMD,FrameContent);
				  break;
				case SetOutOrCont:
//					Scanmode = Scanmodetemp[0] = FrameContent[0];
//					W25QXX_Write((u8*)Scanmodetemp,ScanModeadd,sizeof(Scanmodetemp));
//					Feedback(Len,FrameCMD,Scanmodetemp);
				 StopScan();
				 break;
				case INOUTNUM:           //��ȡ��������
					sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
				  sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
				  p_Counter = 0;
				  for(i=0; i<10; i++)
					{
						FrameContent[i] = InNumtempW[p_Counter++];        //��ȡ��������
					} 
          FrameContent[10] = 0x2c; //,�ָ�
					p_Counter = 0;
					for(i=0; i<10; i++)
					{
						FrameContent[11+i] = OutNumtempW[p_Counter++];        //��ȡ��������
					} 
					Feedback(0x1B,FrameCMD,FrameContent);
				  break;
				case Relay1:             // ��һ����
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED2(1);
				  delay_ms(500);
				  LED2(0);
			    Feedback(Len,FrameCMD,FBStand_Content);
					break;
				case Relay2:            //��������
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED3(1);
					delay_ms(500);
				  LED3(0);
				  Feedback(Len,FrameCMD,FBStand_Content);
					break;
				

				default:

					break;
				}
			

		
	}
}

/**********************************************************************************************************

����ԭ��:    	void DecodeTCPFrame(uint8_t *Frame,uint8_t Len)
��������:	    ���ݽ��뵽��TCPЭ��֡,ѡ��ִ�в�ͬ�Ĺ���
�������:			uint8_t *Frame: TCPЭ��֡��ʼָ��
							uint8_t Len: 		֡����

�������:      ��
����ֵ:        ��
***********************************************************************************************************/


void DecodeTCPFrame(uint8_t *Frame,uint8_t Len)
{
		u8 i;
	u8 p_Counter = 0;
	u8 FrameContent[50];     //
	//u8 tempEPC[900];
	u16 EndFrame;
	uint16_t CAL_CRC16,RECV_CRC16;
	memset(FrameContent,0,sizeof(FrameContent));

	//TimeOutCounter = 0;    //��������
	if(Frame[p_Counter++] == 0x05 )                    //�ж�֡ͷ֡β
	{
		FrameLength = Frame[p_Counter++];                //��ȡ֡����
		FrameCMD    = Frame[p_Counter++];			           //��ȡCMD
		for(i=0; i<Len-6; i++)
			{
				FrameContent[i] = Frame[p_Counter++];        //��ȡ��������
			}  
    RECV_CRC16 =  (Frame[p_Counter++]<<8);           //��ȡCRC16У��
		RECV_CRC16 += (Frame[p_Counter++]);
		EndFrame = 0x06;                                 //��ȡ֡β
		//CAL_CRC16 = cal_crc16(&Frame[1],Len-4); 	       //����CRC16У��

#if DEBUG
			printf("���յ���UDP֡����Ϊ: %d ,����Ϊ: %X ,����Ϊ: %X\r\n\r\n",FrameLength,FrameCMD,FrameContent);
			printf("����õ���CRC16Ϊ: %X  ,",CAL_CRC16);
			printf("���յ���CRC16Ϊ: %X\r\n\r\n",RECV_CRC16);
#endif

			//if(RECV_CRC16 == CAL_CRC16)            //����CRC16У��
				switch(FrameCMD)        				  	//�����������ͽ�����Ӧ�Ĳ���
				{
				case FeedBack_EPC:           //�ϴ�EPC
//				  if(BookCounter !=0)
//					{
//						int j,k, tempLen;
//						tempLen = 0;
//						for(j=0; j< BookCounter; j++)
//						{
//							for(k=0;k<12+1; k++)
//							{
//								tempEPC[tempLen++] = TotalEPC[j][k];
//							}
//						}
//						MakeTCPFrame(BookCounter*13+6,FrameCMD,tempEPC);
//						BookCounter = 0;
//						memset(TotalEPC,0,sizeof(TotalEPC));
//						memset(tempEPC,0,sizeof(tempEPC));
//					}
//					else
//					{
//						FBStand_Content[0] = 0x02;
//					  MakeTCPFrame(Len,FrameCMD,FBStand_Content);
//					}
					break;
				case Alarm:                  //����ָ��
					//NetCmd = 1;
					StartAlarm();
				  FBStand_Content[0] = 0x02;
					MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case BookName:                                //��ʾ��������
					FBStand_Content[0] = 0x02;
					BookNameKeep_flag = 1;
				  KeepBookName_time = 0;
					SetTextValue(0,4,FrameContent);
				  MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case INNUM:
					sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
					MakeTCPFrame(0x10,FrameCMD,InNumtempW);
				  break;
				case OUTNUM:
					sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
					MakeTCPFrame(0x10,FrameCMD,OutNumtempW);
				  break;
				case LocalIP:                  //���±���IP
					memcpy(IpAddTemp1,FrameContent,sizeof(FrameContent));
				  W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
					MakeTCPFrame(0x07,FrameCMD,FBStand_Content);
				  SoftReset(); 
				  break;
				case ServerIP:                  //���±���IP
					memcpy(ServerIpTemp1,FrameContent,sizeof(FrameContent));
				  W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
					MakeTCPFrame(0x07,FrameCMD,FBStand_Content);
				  break;
				case AlarmMode:
					Door_Mode = Door_Modetemp[0] = FrameContent[0];
				  ModeReset(Door_Mode);
		      W25QXX_Write((u8*)Door_Modetemp,Door_Modeadd,sizeof(Door_Modetemp));
					MakeTCPFrame(Len,FrameCMD,FBStand_Content);
				  break;
				case PowerSet:
					Powertemp[0] =  FrameContent[0];
					W25QXX_Write((u8*)Powertemp,Poweradd,sizeof(Powertemp));
					MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					SoftReset();
					break;
				case VolumeSet:
					Volumetemp[0] = FrameContent[0];
					if(Volumetemp[0] > 100)
						{
							Volumetemp[0] = 50;
						}
					SetMeterValue(4,2,Volumetemp[0]); //���½�������ֵ
					SetSliderValue(4,3,Volumetemp[0]);
					AdjustVolume(Volumetemp[0]);                               //��������
					W25QXX_Write((u8*)Volumetemp,Volumeadd,sizeof(Volumetemp));
					MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case GetIP:
					FrameContent[0] = IpAddTemp1[0];
					FrameContent[1] = IpAddTemp1[1];
					FrameContent[2] = IpAddTemp1[2];
					FrameContent[3] = IpAddTemp1[3];
					FrameContent[4] = 0x00;
					FrameContent[5] = ServerIpTemp1[0];
					FrameContent[6] = ServerIpTemp1[1];
					FrameContent[7] = ServerIpTemp1[2];
					FrameContent[8] = ServerIpTemp1[3];
					MakeTCPFrame(0x0f,FrameCMD,FrameContent);
				  break;
				case SetOutOrCont:
//					Scanmode = Scanmodetemp[0] = FrameContent[0];
//					W25QXX_Write((u8*)Scanmodetemp,ScanModeadd,sizeof(Scanmodetemp));
//					Feedback(Len,FrameCMD,Scanmodetemp);
				 StopScan();
				 break;
				case INOUTNUM:           //��ȡ��������
					sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
				  sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
				  p_Counter = 0;
				  for(i=0; i<10; i++)
					{
						FrameContent[i] = InNumtempW[p_Counter++];        //��ȡ��������
					} 
          FrameContent[10] = 0x2c; //,�ָ�
					p_Counter = 0;
					for(i=0; i<10; i++)
					{
						FrameContent[11+i] = OutNumtempW[p_Counter++];        //��ȡ��������
					} 
					MakeTCPFrame(0x1B,FrameCMD,FrameContent);
				  break;
				case Relay1:             // ��һ����
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED2(1);
				  delay_ms(500);
				  LED2(0);
			    MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case Relay2:            //��������
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED3(1);
					delay_ms(500);
				  LED3(0);
				  MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case CleanNum:            //�����������
					InNum =OutNum =0;
					SetTextValueInt32(2,2,0);  //���½�������
					SetTextValueInt32(2,3,0);  //���½�������
					SetTextValueInt32(0,2,0);  //������ʾ��������
					SetTextValueInt32(0,3,0);  //������ʾ��������
					sprintf(InNumtempW,"%ld",0); //������ת��Ϊ�ַ���
					W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
					sprintf(OutNumtempW,"%ld",0); //������ת��Ϊ�ַ���
					W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
				  MakeTCPFrame(7,FrameCMD,FBStand_Content);
					break;
				case InvertNum:           //������������
					sscanf(sInvertNumtemp,"%ld",&sInvertNum); //���ַ���ת��Ϊ����
					if(sInvertNum == 1)
					{
						sInvertNum = 2;
					}
					else if(sInvertNum == 2)
					{
						sInvertNum = 1;
					}
					sprintf(sInvertNumtemp,"%ld",sInvertNum); //������ת��Ϊ�ַ���
					W25QXX_Write(sInvertNumtemp,sInvertNum_Modeadd,sizeof(sInvertNumtemp));
					MakeTCPFrame(7,FrameCMD,FBStand_Content);
					break;
				case SetPassword:         //�޸���Ļ����
					memcpy(SetPasswordTemp,FrameContent,sizeof(FrameContent));
				  PasswordLen[0] = Len-6;
				  W25QXX_Write((u8*)SetPasswordTemp,SetPasswordadd,sizeof(SetPasswordTemp));
				  W25QXX_Write(PasswordLen,PasswordLenadd,sizeof(PasswordLen));
				
					MakeTCPFrame(0x07,FrameCMD,FBStand_Content);
					break;
				case SetDoorPort:         //�����ŵĶ˿�
					sscanf(FrameContent,"%ld",&DoorPort); //���ַ���ת��Ϊ����
					sprintf(DoorPortTemp,"%ld",DoorPort); //������ת��Ϊ�ַ���
					W25QXX_Write((u8*)DoorPortTemp,DoorPortadd,sizeof(DoorPortTemp));
					MakeTCPFrame(7,FrameCMD,FBStand_Content);
					SoftReset(); 
					break;
				case SetServerPort:       //���÷������Ķ˿�
					sscanf(FrameContent,"%ld",&ServerPort);
					SetTextValueInt32(8,17,ServerPort);  //���·������˿�
					sprintf(ServerPortTemp,"%ld",ServerPort); //������ת��Ϊ�ַ���
					W25QXX_Write((u8*)ServerPortTemp,ServerPortadd,sizeof(ServerPortTemp));
					MakeTCPFrame(7,FrameCMD,FBStand_Content);
					SoftReset();
					break;
				case NetHeartBeat:
					NetWait = 0;
					break;
				

				default:

					break;
				}
			}
}


/****************************************************************************************************

����ԭ��:    	uint16_t cal_crc16(u8 *ptr,  u16 len)
��������:	    ����CRC16У��

�������:			u8 *ptr: ��ʼָ��
							u16 len: ����

�������:      ��
����ֵ:        ��
****************************************************************************************************/


uint16_t cal_crc16(u8 *ptr,  u16 len)
{
	uint16_t crc;
	uint16_t i,j;
	crc=0xffff;                 // ��ʼֵ
	for(i=0; i<len; i++)
	{
		crc^=ptr[i];
		for(j=0; j<8; j++)
		{
			if(crc&0x0001)
				crc=(crc>>1)^0x8408;   // ����ʽ
			else
				crc=(crc>>1);
		}
	}
	return(crc);
}

/******************************************************************
	*����: void Feedback(void)
	*���룺uint8_t FBLen: 		֡����
	*�������
	*����: ��ʼ����
	*˵�������������ɹ�
******************************************************************/
void Feedback(u8 FBLen,u8 FBCMD, u8 *FBContent)
{
	uint8_t  p_Counter = 0;
	u8 send[801];
	u16  CrcVal;
	int i;
	
	if(FBLen > 240)
	{
		FBLen = 240;
	}

	send[p_Counter++] = 0x05;            //֡ͷ
	send[p_Counter++] = FBLen-1;         //����
	send[p_Counter++] = FBCMD;           //CMD
	//send[p_Counter++] = FBContent;       //����
	for(i=0; i<FBLen-6; i++)
	{
	 send[p_Counter++] = FBContent[i];        //��ȡ��������
	} 
	CrcVal = cal_crc16(&send[1],FBLen-4);
	send[p_Counter++] = CrcVal>>8;
	send[p_Counter++] = CrcVal;
	send[p_Counter++] = 0x06;            //֡β
	W5500_UDP_SEND(send,FBLen);

}

/******************************************************************
	*����: void Feedback(void)
	*���룺uint8_t FBLen: 		֡����
	*�������
	*����: ��ʼ����
	*˵�������������ɹ�
******************************************************************/
void MakeTCPFrame(u8 FBLen,u8 FBCMD, u8 *FBContent)
{
	uint8_t  p_Counter = 0;
	u8 send[1024];
	u16  CrcVal;
	int i;
	
	send[p_Counter++] = 0x05;            //֡ͷ
	send[p_Counter++] = FBLen-1;         //����
	send[p_Counter++] = FBCMD;           //CMD
	//send[p_Counter++] = FBContent;       //����
	for(i=0; i<FBLen-6; i++)
	{
	 send[p_Counter++] = FBContent[i];        //��ȡ��������
	} 
	CrcVal = cal_crc16(&send[1],FBLen-4);
	send[p_Counter++] = CrcVal>>8;
	send[p_Counter++] = CrcVal;
	send[p_Counter++] = 0x06;            //֡β
	W5500_TCP_SEND(send,FBLen);

}


/****************************************************************************************************

����ԭ��:    	u8 WaitNet( void )
��������:
�������:			��
�������:      ��
����ֵ:        ��
****************************************************************************************************/
u8 WaitNet( void )
{
  u8 NetContent[5];
		NetNotFound = 0;
		NetContent[0] = IpAddTemp1[0];
		NetContent[1] = IpAddTemp1[1];
		NetContent[2] = IpAddTemp1[2];
		NetContent[3] = IpAddTemp1[3];
		MakeTCPFrame(0x0a,0xe8,NetContent);
}


//**************


