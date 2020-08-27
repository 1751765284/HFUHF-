#ifndef	_W5500_H_
#define	_W5500_H_
//#include "utility.h"
#include "delay.h"
/***************** Common Register *****************/
#define MR								0x0000      //模式寄存器(Mode Register),用于S/W复位,ping block模式和PPPoE模式.
#define RST								0x80				//Bit7, 如果该位为1,内部寄存器将被初始化,它会在复位后自动清零.
#define WOL								0x20				//Bit5, 0:关闭网络唤醒      1:开启网络唤醒
#define PB								0x10				//Bit4, 0:关闭Ping block   1:启用Ping block
#define PPP								0x08				//Bit3, 0:关闭PPPoE模式     1:启动PPPoE模式
#define FARP							0x02				//Bit1, 0:关闭强迫ARP模式   1:启动强迫ARP模式

#define GAR								0x0001			//网关IP地址寄存器,用来设置默认网关地址
#define SUBR							0x0005			//子网掩码寄存器,用来设置子网掩码地址
#define SHAR							0x0009			//MAC地址寄存器
#define SIPR							0x000f			//IP地址寄存器

#define INTLEVEL					0x0013			//低电平中断定时器寄存器,用于设置下一个中断生效等待的时间.
#define IR								0x0015			//中断寄存器,某一位为1时表示发生中断,主机对该位写入1时,IR的该位被清零.如果IR不等于0x00则INTn被拉低,直到变为0x00时,INTn才会被拉高
#define CONFLICT					0x80				//Bit7,IP冲突,在收到APR请求时,发现发送方IP与本地IP重复,该位被置1
#define UNREACH						0x40				//Bit6,目标不可抵达.
#define PPPOE							0x20				//Bit5,PPPoE连接关闭.
#define MP								0x10				//Bit4,Magic Packet,网络唤醒

#define IMR								0x0016			//中断屏蔽寄存器.当IMR的某一位为1,且IR对应的该位也为1时,中断才发生.
#define IM_IR7						0x80				//IP冲突中断屏蔽. 0:关闭IP冲突中断. 1:启用IP冲突中断.
#define IM_IR6						0x40				//IP地址不可达中断屏蔽. 0:关闭中断. 1:启用中断.
#define IM_IR5						0x20				//PPPoE关闭中断屏蔽.    0:关闭中断. 1:启用中断.
#define IM_IR4						0x10				//Magic Packet中断屏蔽. 0:关闭中断. 1:启用中断.

#define SIR								0x0017			//Socket中断寄存器,指明了Socket的中断状态.SIR被置1后将一直保持为1,直到Sn_IR被主机写1清零.
#define S7_INT						0x80				//Socket7中断
#define S6_INT						0x40				//Socket6中断
#define S5_INT						0x20				//Socket5中断
#define S4_INT						0x10				//Socket4中断
#define S3_INT						0x08				//Socket3中断
#define S2_INT						0x04				//Socket2中断
#define S1_INT						0x02				//Socket1中断
#define S0_INT						0x01				//Socket0中断

#define SIMR							0x0018			//Socket 中断屏蔽寄存器
#define S7_IMR						0x80				//0:关闭Socket 7 中断   1:启用Socket 7 中断
#define S6_IMR						0x40				//0:关闭Socket 6 中断   1:启用Socket 6 中断
#define S5_IMR						0x20				//0:关闭Socket 5 中断   1:启用Socket 5 中断
#define S4_IMR						0x10				//0:关闭Socket 4 中断   1:启用Socket 4 中断
#define S3_IMR						0x08				//0:关闭Socket 3 中断   1:启用Socket 3 中断
#define S2_IMR						0x04				//0:关闭Socket 2 中断   1:启用Socket 2 中断
#define S1_IMR						0x02				//0:关闭Socket 1 中断   1:启用Socket 1 中断
#define S0_IMR						0x01				//0:关闭Socket 0 中断   1:启用Socket 0 中断

#define RTR								0x0019			//重试时间值寄存器,配置了重传超时的时间值.每一单位数值为100微秒.初始值为2000(0x07D0),即200ms.
//在RTR配置的时间内,W5500等待Sn-CR(CONNECT,DISCON,CLOSE,SEND,SEND_MAC,SEND_KEEP command)传输后,
//来自对方的回应.如果在RTR时间段内没有回应,W5500进行包重传或触发超时中断.

#define RCR								0x001b			//重试计数寄存器,设置重新传送的次数.当第"RCR+1"次重传时,超时中断会被置1.
//(中断寄存器(Sn_IR)的中断位,"TIMEOUT"位 置位1).

#define PTIMER						0x001c			//PPP连接控制协议请求定时寄存器. 设置发送LCP Echo请求的时间,单位时间是25ms.
#define PMAGIC						0x001d			//PPP连接控制协议幻数寄存器. 配置用于LCP回应请求的4字节幻数(Magic number).
#define PHA								0x001e			//PPPoE模式下目标MAC寄存器. 配置在PPPoE连接过程中写入PPPoE 服务器所需的MAC地址.
#define PSID							0x0024			//PPPoE模式下会话ID寄存器. 填入PPPoE连接过程中需要的PPPoE服务器会话ID
#define PMRU							0x0026			//PPPoE模式下最大接收单元.

#define UIPR							0x0028			//无法抵达IP地址寄存器
#define UPORT							0x002c			//无法抵达端口寄存器
//当W5500发送数据给一个未开启或不可抵达的端口号时,接收到一个ICMP包(目标地址无法抵达),IR变为1.
//UIPR和UPORT会分别记录下目的IP地址和端口号

#define PHYCFGR						0x002e			//W5500 PHY配置寄存器.默认值: 0b1011 1XXX
#define RST_PHY						0x80				//Reset重启. 当该位为0时,内部PHY重启. 在PHY重启后,该位须设置为1.
#define OPMODE						0x40				//配置PHY工作模式. 1:通过PHYCFGR的OPMDC[2:0]位配置.  0:通过硬件引脚配置
#define DPX								0x04				//双工工作状态(只读)  1:全双工.  0:半双工.
#define SPD								0x02				//速度状态(只读) 1:100Mbps based  0:10Mbps based
#define LINK							0x01				//连接状态(只读) 1:已连接.  0:连接断开.

#define VERR							0x0039			//W5500版本寄存器.

/********************* Socket Register *******************/
#define Sn_MR							0x0000			//Socket n模式寄存器. 用于配置所有SOCKET的选项或者协议类型. 默认值0x00.
#define MULTI_MFEN				0x80				//Bit7,UDP多播模式(UDP模式) 0:关闭多播.  1:开启多播. 在MACRAW模式下开启MAC地址过滤. 0:关闭MAC地址过滤. 1:启用MAC地址过滤.   
#define BCASTB						0x40				//0:关闭广播阻塞  1:开启广播阻塞.
#define	ND_MC_MMB					0x20				//使用无延时ACK(TCP模式下)  0:关闭无延时ACK选项. 1:开启无延时ACK选项.  多播(UDP):0:使用IGMPv2 1:使用IGMPv1 MACRAW多播阻塞: 0:关闭 1:打开.
#define UCASTB_MIP6B			0x10				//UDP模式下的单播阻塞: 0:关闭单播阻塞 1:开启单播阻塞.  MACRAW模式下,IPv6包阻塞  0:关闭IPv6包阻塞.  1:开启IPv6包阻塞.
#define MR_CLOSE					0x00				//关闭状态,无协议.
#define MR_TCP						0x01				//开启TCP协议.
#define MR_UDP						0x02				//开启UDP协议.
#define MR_MACRAW					0x04				//MACRAW(MAC数据透传)

#define Sn_CR							0x0001
#define OPEN							0x01
#define LISTEN						0x02
#define CONNECT						0x04
#define DISCON						0x08
#define CLOSE							0x10
#define SEND							0x20
#define SEND_MAC					0x21
#define SEND_KEEP					0x22
#define RECV							0x40

#define Sn_IR							0x0002
#define IR_SEND_OK				0x10
#define IR_TIMEOUT				0x08
#define IR_RECV						0x04
#define IR_DISCON					0x02
#define IR_CON						0x01

#define Sn_SR							0x0003
#define SOCK_CLOSED				0x00
#define SOCK_INIT					0x13
#define SOCK_LISTEN				0x14
#define SOCK_ESTABLISHED	0x17
#define SOCK_CLOSE_WAIT		0x1c
#define SOCK_UDP					0x22
#define SOCK_MACRAW				0x02

#define SOCK_SYNSEND			0x15
#define SOCK_SYNRECV			0x16
#define SOCK_FIN_WAI			0x18
#define SOCK_CLOSING			0x1a
#define SOCK_TIME_WAIT		0x1b
#define SOCK_LAST_ACK			0x1d

#define Sn_PORT						0x0004
#define Sn_DHAR	  			 	0x0006
#define Sn_DIPR						0x000c
#define Sn_DPORTR					0x0010

#define Sn_MSSR						0x0012
#define Sn_TOS						0x0015
#define Sn_TTL						0x0016

#define Sn_RXBUF_SIZE			0x001e
#define Sn_TXBUF_SIZE			0x001f
#define Sn_TX_FSR					0x0020
#define Sn_TX_RD					0x0022
#define Sn_TX_WR					0x0024
#define Sn_RX_RSR					0x0026
#define Sn_RX_RD					0x0028
#define Sn_RX_WR					0x002a

#define Sn_IMR						0x002c
#define IMR_SENDOK				0x10
#define IMR_TIMEOUT				0x08
#define IMR_RECV					0x04
#define IMR_DISCON				0x02
#define IMR_CON						0x01

#define Sn_FRAG						0x002d
#define Sn_KPALVTR				0x002f

/*******************************************************************/
/************************ SPI Control Byte *************************/
/*******************************************************************/
/* Operation mode bits */
#define VDM				0x00
#define FDM1			0x01
#define	FDM2			0x02
#define FDM4			0x03

/* Read_Write control bit */
#define RWB_READ	0x00
#define RWB_WRITE	0x04

/* Block select bits */
#define COMMON_R	0x00

/* Socket 0 */
#define S0_REG		0x08
#define S0_TX_BUF	0x10
#define S0_RX_BUF	0x18

/* Socket 1 */
#define S1_REG		0x28
#define S1_TX_BUF	0x30
#define S1_RX_BUF	0x38

/* Socket 2 */
#define S2_REG		0x48
#define S2_TX_BUF	0x50
#define S2_RX_BUF	0x58

/* Socket 3 */
#define S3_REG		0x68
#define S3_TX_BUF	0x70
#define S3_RX_BUF	0x78

/* Socket 4 */
#define S4_REG		0x88
#define S4_TX_BUF	0x90
#define S4_RX_BUF	0x98

/* Socket 5 */
#define S5_REG		0xa8
#define S5_TX_BUF	0xb0
#define S5_RX_BUF	0xb8

/* Socket 6 */
#define S6_REG		0xc8
#define S6_TX_BUF	0xd0
#define S6_RX_BUF	0xd8

/* Socket 7 */
#define S7_REG		0xe8
#define S7_TX_BUF	0xf0
#define S7_RX_BUF	0xf8

#define TRUE			0xff
#define FALSE			0x00

#define S_RX_SIZE	2048												/*定义Socket接收缓冲区的大小，可以根据W5500_RMSR的设置修改 */
#define S_TX_SIZE	2048  											/*定义Socket发送缓冲区的大小，可以根据W5500_TMSR的设置修改 */

/***************----- W5500 GPIO定义 -----***************/
#define W5500_SCS		GPIO_Pin_4								//定义W5500的CS引脚	 
#define W5500_SCS_PORT	GPIOA

#define W5500_RST		GPIO_Pin_9								//定义W5500的RST引脚
#define W5500_RST_PORT	GPIOB

#define W5500_INT		GPIO_Pin_8								//定义W5500的INT引脚
#define W5500_INT_PORT	GPIOB

/***************----- 网络参数变量定义 -----***************/
extern unsigned char Gateway_IP[4];						//网关IP地址
extern unsigned char Sub_Mask[4];							//子网掩码
extern unsigned char Phy_Addr[6];							//物理地址(MAC)
extern unsigned char IP_Addr[4];							//本机IP地址

extern uint16_t S0_Port;											//端口0的端口号(5000)
extern unsigned char S0_DIP[4];								//端口0目的IP地址
extern uint16_t S0_DPort;											//端口0目的端口号(60000)

extern unsigned char UDP_DIPR[4];							//UDP(广播)模式,目的主机IP地址
extern uint16_t UDP_DPORT;										//UDP(广播)模式,目的主机端口号
extern u8 NetNotFound ;                       //网络丢失标志

/***************----- 端口的运行模式 -----***************/
extern unsigned char S0_Mode;									//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
#define TCP_SERVER		0x00										//TCP服务器模式
#define TCP_CLIENT		0x01										//TCP客户端模式 
#define UDP_MODE			0x02										//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
extern unsigned char S0_State;								//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据)
#define S_INIT				0x01										//端口完成初始化 
#define S_CONN				0x02										//端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
extern unsigned char S0_Data;									//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成
#define S_RECEIVE			0x01										//端口接收到一个数据包 
#define S_TRANSMITOK	0x02										//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
extern unsigned char Rx_Buffer[2048];					//端口接收数据缓冲区
extern unsigned char Tx_Buffer[2048];					//端口发送数据缓冲区

extern unsigned char W5500_Interrupt;					//W5500中断标志(0:无中断,1:有中断)
typedef unsigned char SOCKET;									//自定义端口号数据类型
extern void W5500_HARDWARE_INIT(void);        //W5500硬件初始化配置
extern void Load_Net_Parameters(void); 			  //装载网络参数
extern void W5500_Socket_Set(void);    				//W5500端口初始化配置
extern int Process_Socket_Data(SOCKET s); 		//W5500接收并发送接收到的数据
extern void Delay(unsigned int d);						//延时函数(ms)
extern void W5500_GPIO_Configuration(void);		//W5500 GPIO初始化配置
extern void W5500_NVIC_Configuration(void);		//W5500 接收引脚中断优先级设置
extern void SPI_Configuration(void);					//W5500 SPI初始化配置(STM32 SPI1)
extern void W5500_Hardware_Reset(void);				//硬件复位W5500
extern void W5500_Init(void);									//初始化W5500寄存器函数
extern unsigned char Detect_Gateway(void);		//检查网关服务器
extern void Socket_Init(SOCKET s);						//指定Socket(0~7)初始化
extern unsigned char Socket_Connect(SOCKET s);//设置指定Socket(0~7)为客户端与远程服务器连接
extern unsigned char Socket_Listen(SOCKET s);	//设置指定Socket(0~7)作为服务器等待远程主机的连接
extern unsigned char Socket_UDP(SOCKET s);		//设置指定Socket(0~7)为UDP模式
extern unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr);							//指定Socket(0~7)接收数据处理
extern void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size); 	//指定Socket(0~7)发送数据处理
extern void W5500_Interrupt_Process(void);																									//W5500中断处理程序框架
extern uint8_t W5500_UDP_SEND(uint8_t *ch,uint8_t Num); //通过W5500发送UDP数据
unsigned char Read_W5500_1Byte(unsigned short reg);
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat);
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg);
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg);
void DecodeUDPFrame(uint8_t *Frame,uint8_t Len);
uint16_t cal_crc16(u8 *ptr,  u16 len);
void Feedback(u8 FBLen,u8 FBCMD, u8 *FBContent);
void DecodeTCPFrame(uint8_t *Frame,uint8_t Len);
void MakeTCPFrame(u8 FBLen,u8 FBCMD, u8 *FBContent);
uint8_t W5500_TCP_SEND(uint8_t *ch,uint8_t Num);
u8 WaitNet( void );
void W5500_NVIC_Close_Configuration(void);

#endif

