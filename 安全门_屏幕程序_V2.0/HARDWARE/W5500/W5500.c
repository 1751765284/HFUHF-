/**********************************************************************************
 * 文件名  ：W5500.c
 * 描述    ：W5500 驱动函数库
 * 库版本  ：ST_v3.5
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








/***************----- 网络参数变量定义 -----***************/

unsigned char Phy_Addr[6]   =  {0x0c,0x29,0xab,0x7c,0x00,0x01};			//物理地址(MAC)

unsigned char IP_Addr[4]    =  {192, 168, 0,  133};									//本机IP地址
unsigned char Sub_Mask[4]   =  {255, 255, 255, 0};									//子网掩码
unsigned char Gateway_IP[4] =  {192, 168, 0,  252};									  //网关IP地址
uint16_t S0_Port  = 6000;																						//端口0的端口号(6000) (客户端端口号)

unsigned char S0_DIP[4]     =  {192, 168, 0, 95};			        	  	//端口0目的IP地址(服务器IP地址)
uint16_t S0_DPort = 20000;																					//端口0目的端口号(服务器端口号)

unsigned char UDP_DIPR[4]   =  {192, 168, 0, 95}  	;			//UDP(广播)模式,目的主机IP地址
uint16_t UDP_DPORT = 20000	;			//UDP(广播)模式,目的主机端口号

/***************----- 端口的运行模式 -----***************/
unsigned char S0_Mode =3;				//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
unsigned char S1_Mode =3;				//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
#define TCP_SERVER	0x00				//TCP服务器模式
#define TCP_CLIENT	0x01				//TCP客户端模式 
#define UDP_MODE	  0x02				//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
unsigned char S0_State =0;			//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据)
unsigned char S1_State =0;			//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据)
#define S_INIT		0x01					//端口完成初始化 
#define S_CONN		0x02					//端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
unsigned char S0_Data;					//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成
unsigned char S1_Data;					//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
#define S_RECEIVE	 0x01					//端口接收到一个数据包 
#define S_TRANSMITOK 0x02				//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
unsigned char S0_Rx_Buffer[2048];	//端口接收数据缓冲区 
unsigned char S0_Tx_Buffer[2048];	//端口发送数据缓冲区 
unsigned char S1_Rx_Buffer[2048];	//端口接收数据缓冲区 
unsigned char S1_Tx_Buffer[2048];	//端口发送数据缓冲区 

unsigned char W5500_Interrupt;	//W5500中断标志(0:无中断,1:有中断)

#define FeedBack_EPC    0xf1       //接收回复EPC
#define Alarm       		0xf2       //报警指令
#define BookName   		  0xf3       //显示书名
#define INNUM           0xf4       //进馆人数
#define OUTNUM          0xf5       //出馆人数
#define LocalIP         0xf6       //本地IP
#define ServerIP        0xf7       //服务器IP
#define AlarmMode       0xf8       //报警模式
#define PowerSet        0xf9       //功率设置
#define VolumeSet       0xfa       //声音设置
#define GetIP           0xfb       //获取IP
#define SetOutOrCont    0xfc       //设置持续报警还是高低电平有效触发报警
#define FeedBack_TID    0xfd       //上传EPC和TID
#define INOUTNUM        0xfe       //进出人数
#define Relay1          0xe1       //继电器1
#define Relay2          0xe2       //继电器2
#define CleanNum        0xe3       //清零进出人数
#define InvertNum       0xe4       //反转进出人数
#define SetPassword     0xe5       //修改屏幕密码
#define SetDoorPort     0xe6       //设置门的端口
#define SetServerPort   0xe7       //设置服务器的端口
#define NetHeartBeat    0xe8       //网络心跳

u8             Serial;         //序号
u8             FrameLength;    //数据长度
u8             FrameCMD;       //收到命令
int             NetWait = 0;
u8             NetNotFound = 0; //网络超时标志
u8             NetHeartFlag = 1;    //网络心跳标志
u8             FBStand_Content[1] = {0x02};
int KeepBookName_time = 0;
u8  BookNameKeep_flag = 0;


/*******************************************************************************
* 函数名  : W5500_TCP_SEND
* 描述    : 通过W5500发送TCP数据
* 输入    : uint8_t *ch:发送数据的起始地址   uint8_t Num:要发送的字节数
* 输出    : 无
* 返回    : 1:成功  0:失败 
* 说明    : 无
*******************************************************************************/
uint8_t W5500_TCP_SEND(uint8_t *ch,uint8_t Num)
{
		int i;
		if(S0_State == (S_INIT|S_CONN))					//如果端口初始化完成
		{
				S0_Data &= ~S_TRANSMITOK;
				memcpy(S0_Tx_Buffer,ch, Num);	
			
#if DEBUG_PRINT  
				printf("发送TCP数据包:内容为: ");
				for(i = 0;i<Num;i++)
				{
						printf(" %X",S0_Tx_Buffer[i]);
				}
				printf("\r\n\r\n");
#endif
				
				Write_SOCK_Data_Buffer(0, S0_Tx_Buffer, Num);//指定Socket(0~7)发送数据处理,端口0发送23字节数据		
				return 1;	
		}
		else
		{		
				Socket_Init(0);		     //指定Socket(0~7)初始化,初始化端口0
				W5500_Socket_Set();		//W5500端口Connect()
				return 0;
		}
}

/*******************************************************************************
* 函数名  : W5500_UDP_SEND   W5500_UDP_SEND
* 描述    : 通过W5500发送TCP数据      UDP
* 输入    : uint8_t *ch:发送数据的起始地址   uint8_t Num:要发送的字节数
* 输出    : 无
* 返回    : 1:成功  0:失败
* 说明    : 无
*******************************************************************************/
uint8_t W5500_UDP_SEND(uint8_t *ch,uint8_t Num)
{
		int i;
		if(S1_State == (S_INIT|S_CONN))					//如果端口初始化完成
		{
				S1_Data &= ~S_TRANSMITOK;
				memcpy(S1_Tx_Buffer,ch, Num);	
			
#if DEBUG_PRINT  
				printf("发送UDP数据包:内容为: ");
				for(i = 0;i<Num;i++)
				{
						printf(" %X",S1_Tx_Buffer[i]);
				}
				printf("\r\n\r\n");
#endif
				
				Write_SOCK_Data_Buffer(1, S1_Tx_Buffer, Num);//指定Socket(0~7)发送数据处理,端口1发送23字节数据		
				return 1;	
		}
		else
		{		
				Socket_Init(1);		     //指定Socket(0~7)初始化,初始化端口0
				W5500_Socket_Set();		//W5500端口Connect()
				return 0;
		}
}
/*******************************************************************************
* 函数名  : W5500_HARDWARE_INIT
* 描述    : W5500硬件初始化
* 输入    : 无
* 输出    : 无
* 返回    : 无
* 说明    : 无
*******************************************************************************/
void W5500_HARDWARE_INIT(void)
{

	W5500_GPIO_Configuration();	 //W5500 GPIO初始化配置
	SPI_Configuration();				//W5500 SPI初始化配置(STM32 SPI1)
	Load_Net_Parameters();		 //装载网络参数
	W5500_Hardware_Reset();		//硬件复位W5500
	W5500_NVIC_Configuration();		//W5500外部中断INT初始化
	W5500_Init();			       //初始化W5500寄存器函数
	//Detect_Gateway();	    //检查网关服务器 (应用于内网,不需要网关连接外网)
	Socket_Init(0);		     //指定Socket(0~7)初始化,初始化端口0
	W5500_Socket_Set();		//W5500端口Connect()

}


/**************************************************************************

函数原型:    	uint8_t CalcDeviceID()
功能描述:	    根据拨码盘计算设备ID(IP地址)
输入参数:			无

输出参数:     无
返回值:       返回拨码盘值,作为设备ID和IP地址和MAC地址最后一组使用(根据16进制计算)
( GPIO: SW1:PC14 SW2:PC15 SW3:PA0 SW4:PC5 SW5:PA11 SW6:PA12 )
**************************************************************************/
uint8_t CalcDeviceID()
{
	uint8_t DeviceID = 0;

	//SWx为0时有效, DeviceID = 32*SW1 + 16*SW2 + 8*SW3 + 4*SW4 + 2*SW5 + 1*SW6
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
	//printf("DeviceID 为:%d\r\n",DeviceID);

	return DeviceID;

}

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 装载网络参数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
*******************************************************************************/
void Load_Net_Parameters(void)
{
//	IP_Addr[3] = ( CalcDeviceID() == 0) ? IP_Addr[3]:CalcDeviceID();  			 //获取拨码盘值,如果为0保持默认IP,如果不为0则设IP地址为拨码盘值

//	Phy_Addr[5] = ( CalcDeviceID() == 0) ? Phy_Addr[5]:CalcDeviceID();      //获取拨码盘值,如果为0保持默认MAC,如果不为0则设MAC地址为拨码盘值

	S0_Mode=TCP_CLIENT;//加载端口0的工作模式,TCP客户端模式
	S1_Mode=UDP_MODE;	 //加载端口1的工作模式,UDP模式

//	S0_Mode=UDP_MODE;                                               //UDP模式
}

/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5500_Socket_Set(void)
{	
		if(S0_State==0x00)										//端口0初始化配置
		{
				if(S0_Mode==TCP_SERVER)				//TCP服务器模式 
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
				else if(S0_Mode==TCP_CLIENT)	//TCP客户端模式 
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
				else													//UDP模式 
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
		
		
		if(S1_State==0x00)										//端口1初始化配置
		{
				if(S1_Mode==TCP_SERVER)				//TCP服务器模式 
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
				else if(S1_Mode==TCP_CLIENT)	//TCP客户端模式 
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
				else													//UDP模式 
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
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_proces s()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
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
					printf("Socket0接收到数据包:内容为: ");
					for(i = 0;i<size;i++)
					{
							printf(" %X",S0_Rx_Buffer[i]);
					}
					printf("\r\n\r\n");
#endif
				
					
		DecodeTCPFrame(S0_Rx_Buffer,size);   	 //解码收到的TCP数据包
					
	}
	
  if(s == 1)
	{
		size=Read_SOCK_Data_Buffer(s, S1_Rx_Buffer);
		for(i=0; i<4; i++)
		{
			UDP_DIPR[i]= S1_Rx_Buffer[p_Counter++];    //获取IP内容
		}
	#if DEBUG
		printf("接收到UDP_IP数据包:内容为: ");

		printf(" IP地址   : %d.%d.%d.%d\r\n", UDP_DIPR[0],UDP_DIPR[1],UDP_DIPR[2],UDP_DIPR[3]);

		printf("\r\n\r\n");
	#endif
		UDP_DPORT = (S1_Rx_Buffer[p_Counter++]<<8) ;  	 	       //获取端口号
		UDP_DPORT += (S1_Rx_Buffer[p_Counter++]) ;
	#if DEBUG
		printf("接收到UDP_port数据包:内容为: ");
		printf(" UDP端口为:%d \r\n",UDP_DPORT);
		printf("\r\n\r\n");
		printf("接收到UDP数据包:内容为: ");
		for(i = 0; i<size; i++)
		{
			printf(" %X",S1_Rx_Buffer[i]);
		}
		printf("\r\n\r\n");
	#endif
		DecodeUDPFrame(&S1_Rx_Buffer[8],size-8);   	 //解码收到的UDP数据包
		//Write_SOCK_Data_Buffer(s,&Rx_Buffer[8],size-8);
		//Write_SOCK_Data_Buffer(s,Rx_Buffer,size);
	}
		return size;

}


/*******************************************************************************
* 函数名  : W5500_GPIO_Configuration
* 描述    : W5500 GPIO初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC,ENABLE);

	/* W5500_RST引脚初始化配置 */
	GPIO_InitStructure.GPIO_Pin  = W5500_RST;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(W5500_RST_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);

	/* W5500_INT引脚初始化配置 */
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
* 函数名  : W5500_NVIC_Configuration
* 描述    : W5500 接收引脚中断优先级设置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
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
* 函数名  : W5500_NVIC_Close_Configuration
* 描述    : W5500 接收引脚中断优先级设置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 关闭中断
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
* 函数名  : EXTI4_IRQHandler
* 描述    : 中断线4中断服务函数(W5500 INT引脚中断服务函数)
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line8);
#if DEBUG
		printf("进入外部中断服务程序\r\n");
#endif
		W5500_Interrupt=1;
		W5500_Interrupt_Process();  //调用W5500中断服务处理程序

	}
}

/*******************************************************************************
* 函数名  : SPI_Configuration
* 描述    : W5500 SPI初始化配置(STM32 SPI1)
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI_Configuration(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef   	SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);

	/* 初始化SCK、MISO、MOSI引脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	/* 初始化CS引脚 */
	GPIO_InitStructure.GPIO_Pin = W5500_SCS;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(W5500_SCS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);

	/* 初始化配置STM32 SPI1 */
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;												//设置为主SPI
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;										//SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;													//时钟悬空低
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;												//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;														//NSS由外部管脚管理
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//波特率预分频值为2
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;									//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial=7;														//CRC多项式为7
	SPI_Init(SPI1,&SPI_InitStructure);																//根据SPI_InitStruct中指定的参数初始化外设SPI1寄存器

	SPI_Cmd(SPI1,ENABLE);	//STM32使能SPI1
}



/*******************************************************************************
* 函数名  : SPI1_Send_Byte
* 描述    : SPI1发送1个字节数据
* 输入    : dat:待发送的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Byte(unsigned char dat)
{
	SPI_I2S_SendData(SPI1,dat);																		//写1个字节数据
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);//等待数据寄存器空
}

/*******************************************************************************
* 函数名  : SPI1_Send_Short
* 描述    : SPI1发送2个字节数据(16位)
* 输入    : dat:待发送的16位数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Short(unsigned short dat)
{
	SPI1_Send_Byte(dat/256);	//写数据高位
	SPI1_Send_Byte(dat);			//写数据低位
}

/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : 通过SPI1向指定地址寄存器写1个字节数据
* 输入    : reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(reg);											//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_WRITE|COMMON_R);	//通过SPI1写控制字节,1个字节数据长度,写数据,选择通用寄存器
	SPI1_Send_Byte(dat);											//写1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);	//置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_2Byte
* 描述    : 通过SPI1向指定地址寄存器写2个字节数据
* 输入    : reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(reg);											//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM2|RWB_WRITE|COMMON_R);	//通过SPI1写控制字节,2个字节数据长度,写数据,选择通用寄存器
	SPI1_Send_Short(dat);											//写16位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 	//置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : 通过SPI1向指定地址寄存器写n个字节数据
* 输入    : reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,size:待写入的数据长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(reg);											//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|COMMON_R);		//通过SPI1写控制字节,N个字节数据长度,写数据,选择通用寄存器

	for(i=0; i<size; i++)												//循环将缓冲区的size个字节数据写入W5500
	{
		SPI1_Send_Byte(*dat_ptr++);							//写一个字节数据
	}

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);	//置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//置W5500的SCS为低电平

	SPI1_Send_Short(reg);													//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_WRITE|(s*0x20+0x08));	//通过SPI1写控制字节,1个字节数据长度,写数据,选择端口s的寄存器
	SPI1_Send_Byte(dat);													//写1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 			//置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : 通过SPI1向指定端口寄存器写2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//置W5500的SCS为低电平

	SPI1_Send_Short(reg);													//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM2|RWB_WRITE|(s*0x20+0x08));	//通过SPI1写控制字节,2个字节数据长度,写数据,选择端口s的寄存器
	SPI1_Send_Short(dat);													//写16位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 			//置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : 通过SPI1向指定端口寄存器写4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//置W5500的SCS为低电平

	SPI1_Send_Short(reg);													//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM4|RWB_WRITE|(s*0x20+0x08));	//通过SPI1写控制字节,4个字节数据长度,写数据,选择端口s的寄存器

	SPI1_Send_Byte(*dat_ptr++);										//写第1个字节数据
	SPI1_Send_Byte(*dat_ptr++);										//写第2个字节数据
	SPI1_Send_Byte(*dat_ptr++);										//写第3个字节数据
	SPI1_Send_Byte(*dat_ptr++);										//写第4个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);			//置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : 读W5500指定地址寄存器的1个字节数据
* 输入    : reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//置W5500的SCS为低电平

	SPI1_Send_Short(reg);													//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_READ|COMMON_R);				//通过SPI1写控制字节,1个字节数据长度,读数据,选择通用寄存器

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);													//发送一个哑数据
	i=SPI_I2S_ReceiveData(SPI1);									//读取1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);			//置W5500的SCS为高电平
	return i;																			//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : 读W5500指定端口寄存器的1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//置W5500的SCS为低电平

	SPI1_Send_Short(reg);													//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_READ|(s*0x20+0x08));	//通过SPI1写控制字节,1个字节数据长度,读数据,选择端口s的寄存器

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);													//发送一个哑数据
	i=SPI_I2S_ReceiveData(SPI1);									//读取1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);			//置W5500的SCS为高电平
	return i;																			//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读W5500指定端口寄存器的2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的2个字节数据(16位)
* 说明    : 无
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	unsigned short i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);			//置W5500的SCS为低电平

	SPI1_Send_Short(reg);														//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM2|RWB_READ|(s*0x20+0x08));		//通过SPI1写控制字节,2个字节数据长度,读数据,选择端口s的寄存器

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);														//发送一个哑数据
	i=SPI_I2S_ReceiveData(SPI1);										//读取高位数据
	SPI1_Send_Byte(0x00);														//发送一个哑数据
	i*=256;
	i+=SPI_I2S_ReceiveData(SPI1);										//读取低位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);				//置W5500的SCS为高电平
	return i;																				//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_SOCK_Data_Buffer
* 描述    : 从W5500接收数据缓冲区中读取数据
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针
* 输出    : 无
* 返回值  : 读取到的数据长度,rx_size个字节
* 说明    : 无
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size==0) return 0;											//没接收到数据则返回
	if(rx_size>1460) rx_size=1460;

	offset=Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);												//计算实际的物理地址

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);		//置W5500的SCS为低电平

	SPI1_Send_Short(offset);											//写16位地址
	SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));		//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
	j=SPI_I2S_ReceiveData(SPI1);

	if((offset+rx_size)<S_RX_SIZE)								//如果最大地址未超过W5500接收缓冲区寄存器的最大地址
	{
		for(i=0; i<rx_size; i++)											//循环读取rx_size个字节数据
		{
			SPI1_Send_Byte(0x00);											//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);							//读取1个字节数据
			*dat_ptr=j;																//将读取到的数据保存到数据保存缓冲区
			dat_ptr++;																//数据保存缓冲区指针地址自增1
		}
	}
	else																					//如果最大地址超过W5500接收缓冲区寄存器的最大地址
	{
		offset=S_RX_SIZE-offset;
		for(i=0; i<offset; i++)												//循环读取出前offset个字节数据
		{
			SPI1_Send_Byte(0x00);											//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);							//读取1个字节数据
			*dat_ptr=j;																//将读取到的数据保存到数据保存缓冲区
			dat_ptr++;																//数据保存缓冲区指针地址自增1
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 		//置W5500的SCS为高电平

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);	//置W5500的SCS为低电平

		SPI1_Send_Short(0x00);											//写16位地址
		SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));	//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
		j=SPI_I2S_ReceiveData(SPI1);

		for(; i<rx_size; i++)													//循环读取后rx_size-offset个字节数据
		{
			SPI1_Send_Byte(0x00);											//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);							//读取1个字节数据
			*dat_ptr=j;																//将读取到的数据保存到数据保存缓冲区
			dat_ptr++;																//数据保存缓冲区指针地址自增1
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 			//置W5500的SCS为高电平

	offset1+=rx_size;															//更新实际物理地址,即下次读取接收到的数据的起始地址
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);				//发送启动接收命令
	return rx_size;																//返回接收到数据的长度
}

/*******************************************************************************
* 函数名  : Write_SOCK_Data_Buffer
* 描述    : 将数据写入W5500的数据发送缓冲区
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针,size:待写入数据的长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	//如果是UDP模式,可以在此设置目的主机的IP和端口号
	if((Read_W5500_SOCK_1Byte(s,Sn_MR)&0x0f) != SOCK_UDP)										//如果Socket打开失败
	{
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);													//设置目的主机IP
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT);	//设置目的主机端口号
#if DEBUG
		printf("  UDP 模式\r\n");
#endif
	}

	offset=Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);															//计算实际的物理地址

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);					//置W5500的SCS为低电平

	SPI1_Send_Short(offset);														//写16位地址
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));				//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

	if((offset+size)<S_TX_SIZE)													//如果最大地址未超过W5500发送缓冲区寄存器的最大地址
	{
		for(i=0; i<size; i++)																//循环写入size个字节数据
		{
			SPI1_Send_Byte(*dat_ptr++);											//写入一个字节的数据
		}
	}
	else																								//如果最大地址超过W5500发送缓冲区寄存器的最大地址
	{
		offset=S_TX_SIZE-offset;
		for(i=0; i<offset; i++)															//循环写入前offset个字节数据
		{
			SPI1_Send_Byte(*dat_ptr++);											//写入一个字节的数据
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); 					//置W5500的SCS为高电平

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);				//置W5500的SCS为低电平

		SPI1_Send_Short(0x00);														//写16位地址
		SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));			//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

		for(; i<size; i++)																	//循环写入size-offset个字节数据
		{
			SPI1_Send_Byte(*dat_ptr++);											//写入一个字节的数据
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);						//置W5500的SCS为高电平

	offset1+=size;																			//更新实际物理地址,即下次写待发送数据到发送数据缓冲区的起始地址
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);							//发送启动发送命令
}

/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : 硬件复位W5500
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : W5500的复位引脚保持低电平至少500us以上,才能重围W5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);		//复位引脚拉低
	delay_ms(50);
	GPIO_SetBits(W5500_RST_PORT, W5500_RST);			//复位引脚拉高
	delay_ms(200);
	//while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);		//等待以太网连接完成

}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化W5500寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5500之前，先对W5500初始化
*******************************************************************************/
void W5500_Init(void)
{
	u8 i=0;

	Write_W5500_1Byte(MR, RST);//软件复位W5500,置1有效,复位后自动清0
	delay_ms(10);//延时10ms,自己定义该函数

	//设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义
	//使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
	Write_W5500_nByte(GAR, Gateway_IP, 4);

	//设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
	//子网掩码用于子网运算
	Write_W5500_nByte(SUBR,Sub_Mask,4);

	//设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
	//该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//如果自己定义物理地址，注意第一个字节必须为偶数
	Write_W5500_nByte(SHAR,Phy_Addr,6);

	//设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
	//注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	Write_W5500_nByte(SIPR,IP_Addr,4);

	//设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
	for(i=0; i<8; i++)
	{
		Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
	}

	//设置重试时间，默认为2000(200ms)
	//每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
	Write_W5500_2Byte(RTR, 0x07d0);

	//设置重试次数，默认为8次
	//如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
	Write_W5500_1Byte(RCR,8);

	//启动中断，参考W5500数据手册确定自己需要的中断类型
	//IMR_CONFLICT是IP地址冲突异常中断,IMR_UNREACH是UDP通信时，地址无法到达的异常中断
	//其它是Socket事件中断，根据需要添加
	Write_W5500_1Byte(IMR,IM_IR7 | IM_IR6);		 //启用IP地址冲突和目标地址不可达中断
	Write_W5500_1Byte(SIMR,S0_IMR);						 //启用Socket0中断
	Write_W5500_SOCK_1Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	Write_W5500_SOCK_1Byte(0,Sn_KPALVTR,0x01);			//每隔5s发送KeepAlive心跳包

}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : 检查网关服务器
* 输入    : 无
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 无
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=IP_Addr[0]+1;
	ip_adde[1]=IP_Addr[1]+1;
	ip_adde[2]=IP_Addr[2]+1;
	ip_adde[3]=IP_Addr[3]+1;

	//检查网关及获取网关的物理地址
	Write_W5500_SOCK_4Byte(0,Sn_DIPR,ip_adde);			//向目的地址寄存器写入与本机IP不同的IP值
	Write_W5500_SOCK_1Byte(0,Sn_MR,MR_UDP);					//设置socket为UDP模式
	Write_W5500_SOCK_1Byte(0,Sn_CR,OPEN);						//打开Socket
	delay_ms(5);																		//延时5ms

	if(Read_W5500_SOCK_1Byte(0,Sn_SR) != SOCK_INIT)	//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);				//打开不成功,关闭Socket
		return FALSE;																	//返回FALSE(0x00)
	}

	Write_W5500_SOCK_1Byte(0,Sn_CR,CONNECT);				//设置Socket为Connect模式

	do
	{
		u8 j=0;
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);							//读取Socket0中断标志寄存器
		if(j!=0)
			Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		delay_ms(5);																			//延时5ms
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;
		}
		else if(Read_W5500_SOCK_1Byte(0,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);			//关闭Socket
			return TRUE;
		}
	}
	while(1);
}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定Socket(0~7)初始化
* 输入    : s:待初始化的端口
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	//设置分片长度，参考W5500数据手册，该值可以不修改
	Write_W5500_SOCK_2Byte(s, Sn_MSSR, 1460);//最大分片字节数=1460(0x5b4)
	//设置指定端口
	switch(s)
	{
	case 0:
		//设置端口0的端口号
		Write_W5500_SOCK_2Byte(s, Sn_PORT, S0_Port);
		//设置端口0目的(远程)端口号
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT);
		//设置端口0目的(远程)IP地址
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, S0_DIP);

		break;

	case 1:
			//设置端口0的端口号
			Write_W5500_SOCK_2Byte(s, Sn_PORT, S0_Port);
			//设置端口0目的(远程)端口号
			Write_W5500_SOCK_2Byte(s, Sn_DPORTR, S0_DPort);
			//设置端口0目的(远程)IP地址
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
* 函数名  : Socket_Connect
* 描述    : 设置指定Socket(0~7)为客户端与远程服务器连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在客户端模式时,引用该程序,与远程服务器建立连接
*			如果启动连接后出现超时中断，则与服务器连接失败,需要重新调用该程序连接
*			该程序每调用一次,就与服务器产生一次连接
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);					//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);						//打开Socket
	delay_ms(5);																		//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)		//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);				//打开不成功,关闭Socket
		return FALSE;																	//返回FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);				//设置Socket为Connect模式
	return TRUE;																		//返回TRUE,设置成功
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : 设置指定Socket(0~7)作为服务器等待远程主机的连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在服务器模式时,引用该程序,等等远程主机的连接
*			该程序只调用一次,就使W5500设置为服务器模式
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);					//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);						//打开Socket
	delay_ms(5);																				//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)		//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);				//打开不成功,关闭Socket
		return FALSE;																	//返回FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,LISTEN);					//设置Socket为侦听模式
	delay_ms(5);																				//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_LISTEN)	//如果socket设置失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);				//设置不成功,关闭Socket
		return FALSE;																	//返回FALSE(0x00)
	}

	return TRUE;

	//至此完成了Socket的打开和设置侦听工作,至于远程客户端是否与它建立连接,则需要等待Socket中断，
	//以判断Socket的连接是否成功。参考W5500数据手册的Socket中断状态
	//在服务器侦听模式不需要设置目的IP和目的端口号
}

/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : 设置指定Socket(0~7)为UDP模式
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 如果Socket工作在UDP模式,引用该程序,在UDP模式下,Socket通信不需要建立连接
*			该程序只调用一次，就使W5500设置为UDP模式
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);				//设置Socket为UDP模式*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);					//打开Socket*/
	delay_ms(5);																			//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)	//如果Socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);			//打开不成功,关闭Socket
		return FALSE;																//返回FALSE(0x00)
	}
	else
		return TRUE;

	//至此完成了Socket的打开和UDP模式设置,在这种模式下它不需要与远程主机建立连接
	//因为Socket不需要建立连接,所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	//如果目的主机IP和目的Socket的端口号是固定的,在运行过程中没有改变,那么也可以在这里设置
}

/*******************************************************************************
* 函数名  : W5500_Interrupt_Process
* 描述    : W5500中断处理程序框架
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	unsigned char i,j;

IntDispose:
	W5500_Interrupt=0;								//清零中断标志
	i = Read_W5500_1Byte(IR);					//读取中断标志寄存器
	Write_W5500_1Byte(IR, (i&0xf0));	//回写清除中断标志

	if((i & CONFLICT) == CONFLICT)		//IP地址冲突异常处理
	{
		//自己添加代码
	}

	if((i & UNREACH) == UNREACH)			//UDP模式下地址无法到达异常处理
	{
		//自己添加代码
	}

	i=Read_W5500_1Byte(SIR);										//读取端口中断标志寄存器
	if((i & S0_INT) == S0_INT)									//Socket0事件处理
	{
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);					//读取Socket0中断标志寄存器
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		if(j&IR_CON)															//在TCP模式下,Socket0成功连接
		{
			S0_State |= S_CONN;											//网络连接状态0x02,端口完成连接，可以正常传输数据
#if DEBUG
			printf("TCP服务器连接完成，可以正常传输数据\r\n\r\n");
#endif
			delay_ms(500);
//			SPI_FLASH_BufferRead(ShowG, 0x16000, 300);
//			ShowContent(1,ShowData);
		}
		if(j&IR_DISCON)														//在TCP模式下Socket断开连接处理
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);	//关闭端口,等待重新打开连接
			Socket_Init(0);													//指定Socket(0~7)初始化,初始化端口0
			S0_State=0;															//网络连接状态0x00,端口连接失败
			W5500_Socket_Set();
#if DEBUG
			printf("服务器连接失败,正在重试\r\n\r\n");
#endif
			delay_ms(500);
			//		ShowContent(22,ShowData);        				//服务器未连接LED显示

		}
		if(j&IR_SEND_OK)													//Socket0数据发送完成,可以再次启动S_tx_process()函数发送数据
		{
			S0_Data|=S_TRANSMITOK;									//端口发送一个数据包完成
#if DEBUG
			printf("UDP端口发送一个数据包完成\r\n\r\n");
#endif
		}
		if(j&IR_RECV)															//Socket接收到数据,可以启动S_rx_process()函数
		{
			S0_Data|=S_RECEIVE;											//端口接收到一个数据包
#if DEBUG
			printf("UDP端口接收到一个数据包\r\n\r\n");
#endif
			Process_Socket_Data(0);									//W5500接收中断处理程序

			S0_Data &= ~S_RECEIVE;
		}
		if(j&IR_TIMEOUT)													//Socket连接或数据传输超时处理
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);	// 关闭端口,等待重新打开连接
			Socket_Init(0);													//指定Socket(0~7)初始化,初始化端口0
#if DEBUG
			printf("Socket连接或数据传输超时处理 \r\n\r\n");
#endif
			delay_ms(500);
			if((Read_W5500_1Byte(PHYCFGR)&LINK)==0)				//网线未连接
			{
				//	ShowContent(23,ShowData);	//网线断开...
			}
			else
			{
				//ShowContent(22,ShowData);        				//服务器未连接LED显示
			}
			S0_State=0;															//网络连接状态0x00,端口连接失败
			W5500_Socket_Set();											//W5500端口Connect()
		}
	}

	if(Read_W5500_1Byte(SIR) != 0)
		goto IntDispose;
}


/**********************************************************************************************************

函数原型:    	void DecodeUDPFrame(uint8_t *Frame,uint8_t Len)
功能描述:	    根据解码到的UDP协议帧,选择执行不同的功能
输入参数:			uint8_t *Frame: TCP协议帧起始指针
							uint8_t Len: 		帧长度

输出参数:      无
返回值:        无
***********************************************************************************************************/


void DecodeUDPFrame(uint8_t *Frame,uint8_t Len)
{
	u8 i;
	u8 p_Counter = 0;
	u8 FrameContent[50];     //
	//u8 tempEPC[900];
	u16 EndFrame;
	uint16_t CAL_CRC16,RECV_CRC16;
	//TimeOutCounter = 0;    //计数清零
	if(Frame[p_Counter++] == 0x05 )                    //判断帧头帧尾
	{
		FrameLength = Frame[p_Counter++];                //获取帧长度
		FrameCMD    = Frame[p_Counter++];			           //获取CMD
		for(i=0; i<Len-6; i++)
			{
				FrameContent[i] = Frame[p_Counter++];        //获取参数内容
			}  
    RECV_CRC16 =  (Frame[p_Counter++]<<8);           //获取CRC16校验
		RECV_CRC16 += (Frame[p_Counter++]);
		EndFrame = 0x06;                                 //获取帧尾
		//CAL_CRC16 = cal_crc16(&Frame[1],Len-4); 	       //计算CRC16校验

#if DEBUG
			printf("接收到的UDP帧长度为: %d ,命令为: %X ,参数为: %X\r\n\r\n",FrameLength,FrameCMD,FrameContent);
			printf("计算得到的CRC16为: %X  ,",CAL_CRC16);
			printf("接收到的CRC16为: %X\r\n\r\n",RECV_CRC16);
#endif

			//if(RECV_CRC16 == CAL_CRC16)            //进行CRC16校验
				switch(FrameCMD)        				  	//根据命令类型进行相应的操作
				{
				case FeedBack_EPC:           //上传EPC
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
				case Alarm:                  //报警指令
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
					sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
					Feedback(0x10,FrameCMD,InNumtempW);
				  break;
				case OUTNUM:
					sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
					Feedback(0x10,FrameCMD,OutNumtempW);
				  break;
				case LocalIP:                  //更新本地IP
					IpAddTemp1[0] = FrameContent[0];
				  IpAddTemp1[1] = FrameContent[1];
				  IpAddTemp1[2] = FrameContent[2];
				  IpAddTemp1[3] = FrameContent[3];
				  W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
					Feedback(0x07,FrameCMD,FBStand_Content);
				  break;
				case ServerIP:                  //更新本地IP
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
					SetMeterValue(4,2,Volumetemp[0]); //更新进度条数值
					AdjustVolume(Volumetemp[0]);                               //调节声音
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
				case INOUTNUM:           //获取进出人数
					sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
				  sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
				  p_Counter = 0;
				  for(i=0; i<10; i++)
					{
						FrameContent[i] = InNumtempW[p_Counter++];        //获取参数内容
					} 
          FrameContent[10] = 0x2c; //,分割
					p_Counter = 0;
					for(i=0; i<10; i++)
					{
						FrameContent[11+i] = OutNumtempW[p_Counter++];        //获取参数内容
					} 
					Feedback(0x1B,FrameCMD,FrameContent);
				  break;
				case Relay1:             // 开一号门
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED2(1);
				  delay_ms(500);
				  LED2(0);
			    Feedback(Len,FrameCMD,FBStand_Content);
					break;
				case Relay2:            //开二号门
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

函数原型:    	void DecodeTCPFrame(uint8_t *Frame,uint8_t Len)
功能描述:	    根据解码到的TCP协议帧,选择执行不同的功能
输入参数:			uint8_t *Frame: TCP协议帧起始指针
							uint8_t Len: 		帧长度

输出参数:      无
返回值:        无
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

	//TimeOutCounter = 0;    //计数清零
	if(Frame[p_Counter++] == 0x05 )                    //判断帧头帧尾
	{
		FrameLength = Frame[p_Counter++];                //获取帧长度
		FrameCMD    = Frame[p_Counter++];			           //获取CMD
		for(i=0; i<Len-6; i++)
			{
				FrameContent[i] = Frame[p_Counter++];        //获取参数内容
			}  
    RECV_CRC16 =  (Frame[p_Counter++]<<8);           //获取CRC16校验
		RECV_CRC16 += (Frame[p_Counter++]);
		EndFrame = 0x06;                                 //获取帧尾
		//CAL_CRC16 = cal_crc16(&Frame[1],Len-4); 	       //计算CRC16校验

#if DEBUG
			printf("接收到的UDP帧长度为: %d ,命令为: %X ,参数为: %X\r\n\r\n",FrameLength,FrameCMD,FrameContent);
			printf("计算得到的CRC16为: %X  ,",CAL_CRC16);
			printf("接收到的CRC16为: %X\r\n\r\n",RECV_CRC16);
#endif

			//if(RECV_CRC16 == CAL_CRC16)            //进行CRC16校验
				switch(FrameCMD)        				  	//根据命令类型进行相应的操作
				{
				case FeedBack_EPC:           //上传EPC
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
				case Alarm:                  //报警指令
					//NetCmd = 1;
					StartAlarm();
				  FBStand_Content[0] = 0x02;
					MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case BookName:                                //显示报警书名
					FBStand_Content[0] = 0x02;
					BookNameKeep_flag = 1;
				  KeepBookName_time = 0;
					SetTextValue(0,4,FrameContent);
				  MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case INNUM:
					sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
					MakeTCPFrame(0x10,FrameCMD,InNumtempW);
				  break;
				case OUTNUM:
					sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
					MakeTCPFrame(0x10,FrameCMD,OutNumtempW);
				  break;
				case LocalIP:                  //更新本地IP
					memcpy(IpAddTemp1,FrameContent,sizeof(FrameContent));
				  W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
					MakeTCPFrame(0x07,FrameCMD,FBStand_Content);
				  SoftReset(); 
				  break;
				case ServerIP:                  //更新本地IP
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
					SetMeterValue(4,2,Volumetemp[0]); //更新进度条数值
					SetSliderValue(4,3,Volumetemp[0]);
					AdjustVolume(Volumetemp[0]);                               //调节声音
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
				case INOUTNUM:           //获取进出人数
					sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
				  sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
				  p_Counter = 0;
				  for(i=0; i<10; i++)
					{
						FrameContent[i] = InNumtempW[p_Counter++];        //获取参数内容
					} 
          FrameContent[10] = 0x2c; //,分割
					p_Counter = 0;
					for(i=0; i<10; i++)
					{
						FrameContent[11+i] = OutNumtempW[p_Counter++];        //获取参数内容
					} 
					MakeTCPFrame(0x1B,FrameCMD,FrameContent);
				  break;
				case Relay1:             // 开一号门
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED2(1);
				  delay_ms(500);
				  LED2(0);
			    MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case Relay2:            //开二号门
					BookCounter = 0;
					memset(TotalEPC,0,sizeof(TotalEPC));
					LED3(1);
					delay_ms(500);
				  LED3(0);
				  MakeTCPFrame(Len,FrameCMD,FBStand_Content);
					break;
				case CleanNum:            //清零进出人数
					InNum =OutNum =0;
					SetTextValueInt32(2,2,0);  //更新进馆人数
					SetTextValueInt32(2,3,0);  //更新进馆人数
					SetTextValueInt32(0,2,0);  //更新显示进馆人数
					SetTextValueInt32(0,3,0);  //更新显示进馆人数
					sprintf(InNumtempW,"%ld",0); //把整数转换为字符串
					W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
					sprintf(OutNumtempW,"%ld",0); //把整数转换为字符串
					W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
				  MakeTCPFrame(7,FrameCMD,FBStand_Content);
					break;
				case InvertNum:           //反正进出人数
					sscanf(sInvertNumtemp,"%ld",&sInvertNum); //把字符串转换为整数
					if(sInvertNum == 1)
					{
						sInvertNum = 2;
					}
					else if(sInvertNum == 2)
					{
						sInvertNum = 1;
					}
					sprintf(sInvertNumtemp,"%ld",sInvertNum); //把整数转换为字符串
					W25QXX_Write(sInvertNumtemp,sInvertNum_Modeadd,sizeof(sInvertNumtemp));
					MakeTCPFrame(7,FrameCMD,FBStand_Content);
					break;
				case SetPassword:         //修改屏幕密码
					memcpy(SetPasswordTemp,FrameContent,sizeof(FrameContent));
				  PasswordLen[0] = Len-6;
				  W25QXX_Write((u8*)SetPasswordTemp,SetPasswordadd,sizeof(SetPasswordTemp));
				  W25QXX_Write(PasswordLen,PasswordLenadd,sizeof(PasswordLen));
				
					MakeTCPFrame(0x07,FrameCMD,FBStand_Content);
					break;
				case SetDoorPort:         //设置门的端口
					sscanf(FrameContent,"%ld",&DoorPort); //把字符串转换为整数
					sprintf(DoorPortTemp,"%ld",DoorPort); //把整数转换为字符串
					W25QXX_Write((u8*)DoorPortTemp,DoorPortadd,sizeof(DoorPortTemp));
					MakeTCPFrame(7,FrameCMD,FBStand_Content);
					SoftReset(); 
					break;
				case SetServerPort:       //设置服务器的端口
					sscanf(FrameContent,"%ld",&ServerPort);
					SetTextValueInt32(8,17,ServerPort);  //更新服务器端口
					sprintf(ServerPortTemp,"%ld",ServerPort); //把整数转换为字符串
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

函数原型:    	uint16_t cal_crc16(u8 *ptr,  u16 len)
功能描述:	    计算CRC16校验

输入参数:			u8 *ptr: 起始指针
							u16 len: 长度

输出参数:      无
返回值:        无
****************************************************************************************************/


uint16_t cal_crc16(u8 *ptr,  u16 len)
{
	uint16_t crc;
	uint16_t i,j;
	crc=0xffff;                 // 初始值
	for(i=0; i<len; i++)
	{
		crc^=ptr[i];
		for(j=0; j<8; j++)
		{
			if(crc&0x0001)
				crc=(crc>>1)^0x8408;   // 多项式
			else
				crc=(crc>>1);
		}
	}
	return(crc);
}

/******************************************************************
	*名称: void Feedback(void)
	*输入：uint8_t FBLen: 		帧长度
	*输出：无
	*功能: 开始报警
	*说明：回馈报警成功
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

	send[p_Counter++] = 0x05;            //帧头
	send[p_Counter++] = FBLen-1;         //长度
	send[p_Counter++] = FBCMD;           //CMD
	//send[p_Counter++] = FBContent;       //内容
	for(i=0; i<FBLen-6; i++)
	{
	 send[p_Counter++] = FBContent[i];        //获取参数内容
	} 
	CrcVal = cal_crc16(&send[1],FBLen-4);
	send[p_Counter++] = CrcVal>>8;
	send[p_Counter++] = CrcVal;
	send[p_Counter++] = 0x06;            //帧尾
	W5500_UDP_SEND(send,FBLen);

}

/******************************************************************
	*名称: void Feedback(void)
	*输入：uint8_t FBLen: 		帧长度
	*输出：无
	*功能: 开始报警
	*说明：回馈报警成功
******************************************************************/
void MakeTCPFrame(u8 FBLen,u8 FBCMD, u8 *FBContent)
{
	uint8_t  p_Counter = 0;
	u8 send[1024];
	u16  CrcVal;
	int i;
	
	send[p_Counter++] = 0x05;            //帧头
	send[p_Counter++] = FBLen-1;         //长度
	send[p_Counter++] = FBCMD;           //CMD
	//send[p_Counter++] = FBContent;       //内容
	for(i=0; i<FBLen-6; i++)
	{
	 send[p_Counter++] = FBContent[i];        //获取参数内容
	} 
	CrcVal = cal_crc16(&send[1],FBLen-4);
	send[p_Counter++] = CrcVal>>8;
	send[p_Counter++] = CrcVal;
	send[p_Counter++] = 0x06;            //帧尾
	W5500_TCP_SEND(send,FBLen);

}


/****************************************************************************************************

函数原型:    	u8 WaitNet( void )
功能描述:
输入参数:			无
输出参数:      无
返回值:        无
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


