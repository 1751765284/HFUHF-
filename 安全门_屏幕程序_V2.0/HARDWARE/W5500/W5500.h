#ifndef	_W5500_H_
#define	_W5500_H_
//#include "utility.h"
#include "delay.h"
/***************** Common Register *****************/
#define MR								0x0000      //ģʽ�Ĵ���(Mode Register),����S/W��λ,ping blockģʽ��PPPoEģʽ.
#define RST								0x80				//Bit7, �����λΪ1,�ڲ��Ĵ���������ʼ��,�����ڸ�λ���Զ�����.
#define WOL								0x20				//Bit5, 0:�ر����绽��      1:�������绽��
#define PB								0x10				//Bit4, 0:�ر�Ping block   1:����Ping block
#define PPP								0x08				//Bit3, 0:�ر�PPPoEģʽ     1:����PPPoEģʽ
#define FARP							0x02				//Bit1, 0:�ر�ǿ��ARPģʽ   1:����ǿ��ARPģʽ

#define GAR								0x0001			//����IP��ַ�Ĵ���,��������Ĭ�����ص�ַ
#define SUBR							0x0005			//��������Ĵ���,�����������������ַ
#define SHAR							0x0009			//MAC��ַ�Ĵ���
#define SIPR							0x000f			//IP��ַ�Ĵ���

#define INTLEVEL					0x0013			//�͵�ƽ�ж϶�ʱ���Ĵ���,����������һ���ж���Ч�ȴ���ʱ��.
#define IR								0x0015			//�жϼĴ���,ĳһλΪ1ʱ��ʾ�����ж�,�����Ը�λд��1ʱ,IR�ĸ�λ������.���IR������0x00��INTn������,ֱ����Ϊ0x00ʱ,INTn�Żᱻ����
#define CONFLICT					0x80				//Bit7,IP��ͻ,���յ�APR����ʱ,���ַ��ͷ�IP�뱾��IP�ظ�,��λ����1
#define UNREACH						0x40				//Bit6,Ŀ�겻�ɵִ�.
#define PPPOE							0x20				//Bit5,PPPoE���ӹر�.
#define MP								0x10				//Bit4,Magic Packet,���绽��

#define IMR								0x0016			//�ж����μĴ���.��IMR��ĳһλΪ1,��IR��Ӧ�ĸ�λҲΪ1ʱ,�жϲŷ���.
#define IM_IR7						0x80				//IP��ͻ�ж�����. 0:�ر�IP��ͻ�ж�. 1:����IP��ͻ�ж�.
#define IM_IR6						0x40				//IP��ַ���ɴ��ж�����. 0:�ر��ж�. 1:�����ж�.
#define IM_IR5						0x20				//PPPoE�ر��ж�����.    0:�ر��ж�. 1:�����ж�.
#define IM_IR4						0x10				//Magic Packet�ж�����. 0:�ر��ж�. 1:�����ж�.

#define SIR								0x0017			//Socket�жϼĴ���,ָ����Socket���ж�״̬.SIR����1��һֱ����Ϊ1,ֱ��Sn_IR������д1����.
#define S7_INT						0x80				//Socket7�ж�
#define S6_INT						0x40				//Socket6�ж�
#define S5_INT						0x20				//Socket5�ж�
#define S4_INT						0x10				//Socket4�ж�
#define S3_INT						0x08				//Socket3�ж�
#define S2_INT						0x04				//Socket2�ж�
#define S1_INT						0x02				//Socket1�ж�
#define S0_INT						0x01				//Socket0�ж�

#define SIMR							0x0018			//Socket �ж����μĴ���
#define S7_IMR						0x80				//0:�ر�Socket 7 �ж�   1:����Socket 7 �ж�
#define S6_IMR						0x40				//0:�ر�Socket 6 �ж�   1:����Socket 6 �ж�
#define S5_IMR						0x20				//0:�ر�Socket 5 �ж�   1:����Socket 5 �ж�
#define S4_IMR						0x10				//0:�ر�Socket 4 �ж�   1:����Socket 4 �ж�
#define S3_IMR						0x08				//0:�ر�Socket 3 �ж�   1:����Socket 3 �ж�
#define S2_IMR						0x04				//0:�ر�Socket 2 �ж�   1:����Socket 2 �ж�
#define S1_IMR						0x02				//0:�ر�Socket 1 �ж�   1:����Socket 1 �ж�
#define S0_IMR						0x01				//0:�ر�Socket 0 �ж�   1:����Socket 0 �ж�

#define RTR								0x0019			//����ʱ��ֵ�Ĵ���,�������ش���ʱ��ʱ��ֵ.ÿһ��λ��ֵΪ100΢��.��ʼֵΪ2000(0x07D0),��200ms.
//��RTR���õ�ʱ����,W5500�ȴ�Sn-CR(CONNECT,DISCON,CLOSE,SEND,SEND_MAC,SEND_KEEP command)�����,
//���ԶԷ��Ļ�Ӧ.�����RTRʱ�����û�л�Ӧ,W5500���а��ش��򴥷���ʱ�ж�.

#define RCR								0x001b			//���Լ����Ĵ���,�������´��͵Ĵ���.����"RCR+1"���ش�ʱ,��ʱ�жϻᱻ��1.
//(�жϼĴ���(Sn_IR)���ж�λ,"TIMEOUT"λ ��λ1).

#define PTIMER						0x001c			//PPP���ӿ���Э������ʱ�Ĵ���. ���÷���LCP Echo�����ʱ��,��λʱ����25ms.
#define PMAGIC						0x001d			//PPP���ӿ���Э������Ĵ���. ��������LCP��Ӧ�����4�ֽڻ���(Magic number).
#define PHA								0x001e			//PPPoEģʽ��Ŀ��MAC�Ĵ���. ������PPPoE���ӹ�����д��PPPoE �����������MAC��ַ.
#define PSID							0x0024			//PPPoEģʽ�»ỰID�Ĵ���. ����PPPoE���ӹ�������Ҫ��PPPoE�������ỰID
#define PMRU							0x0026			//PPPoEģʽ�������յ�Ԫ.

#define UIPR							0x0028			//�޷��ִ�IP��ַ�Ĵ���
#define UPORT							0x002c			//�޷��ִ�˿ڼĴ���
//��W5500�������ݸ�һ��δ�����򲻿ɵִ�Ķ˿ں�ʱ,���յ�һ��ICMP��(Ŀ���ַ�޷��ִ�),IR��Ϊ1.
//UIPR��UPORT��ֱ��¼��Ŀ��IP��ַ�Ͷ˿ں�

#define PHYCFGR						0x002e			//W5500 PHY���üĴ���.Ĭ��ֵ: 0b1011 1XXX
#define RST_PHY						0x80				//Reset����. ����λΪ0ʱ,�ڲ�PHY����. ��PHY������,��λ������Ϊ1.
#define OPMODE						0x40				//����PHY����ģʽ. 1:ͨ��PHYCFGR��OPMDC[2:0]λ����.  0:ͨ��Ӳ����������
#define DPX								0x04				//˫������״̬(ֻ��)  1:ȫ˫��.  0:��˫��.
#define SPD								0x02				//�ٶ�״̬(ֻ��) 1:100Mbps based  0:10Mbps based
#define LINK							0x01				//����״̬(ֻ��) 1:������.  0:���ӶϿ�.

#define VERR							0x0039			//W5500�汾�Ĵ���.

/********************* Socket Register *******************/
#define Sn_MR							0x0000			//Socket nģʽ�Ĵ���. ������������SOCKET��ѡ�����Э������. Ĭ��ֵ0x00.
#define MULTI_MFEN				0x80				//Bit7,UDP�ಥģʽ(UDPģʽ) 0:�رնಥ.  1:�����ಥ. ��MACRAWģʽ�¿���MAC��ַ����. 0:�ر�MAC��ַ����. 1:����MAC��ַ����.   
#define BCASTB						0x40				//0:�رչ㲥����  1:�����㲥����.
#define	ND_MC_MMB					0x20				//ʹ������ʱACK(TCPģʽ��)  0:�ر�����ʱACKѡ��. 1:��������ʱACKѡ��.  �ಥ(UDP):0:ʹ��IGMPv2 1:ʹ��IGMPv1 MACRAW�ಥ����: 0:�ر� 1:��.
#define UCASTB_MIP6B			0x10				//UDPģʽ�µĵ�������: 0:�رյ������� 1:������������.  MACRAWģʽ��,IPv6������  0:�ر�IPv6������.  1:����IPv6������.
#define MR_CLOSE					0x00				//�ر�״̬,��Э��.
#define MR_TCP						0x01				//����TCPЭ��.
#define MR_UDP						0x02				//����UDPЭ��.
#define MR_MACRAW					0x04				//MACRAW(MAC����͸��)

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

#define S_RX_SIZE	2048												/*����Socket���ջ������Ĵ�С�����Ը���W5500_RMSR�������޸� */
#define S_TX_SIZE	2048  											/*����Socket���ͻ������Ĵ�С�����Ը���W5500_TMSR�������޸� */

/***************----- W5500 GPIO���� -----***************/
#define W5500_SCS		GPIO_Pin_4								//����W5500��CS����	 
#define W5500_SCS_PORT	GPIOA

#define W5500_RST		GPIO_Pin_9								//����W5500��RST����
#define W5500_RST_PORT	GPIOB

#define W5500_INT		GPIO_Pin_8								//����W5500��INT����
#define W5500_INT_PORT	GPIOB

/***************----- ��������������� -----***************/
extern unsigned char Gateway_IP[4];						//����IP��ַ
extern unsigned char Sub_Mask[4];							//��������
extern unsigned char Phy_Addr[6];							//�����ַ(MAC)
extern unsigned char IP_Addr[4];							//����IP��ַ

extern uint16_t S0_Port;											//�˿�0�Ķ˿ں�(5000)
extern unsigned char S0_DIP[4];								//�˿�0Ŀ��IP��ַ
extern uint16_t S0_DPort;											//�˿�0Ŀ�Ķ˿ں�(60000)

extern unsigned char UDP_DIPR[4];							//UDP(�㲥)ģʽ,Ŀ������IP��ַ
extern uint16_t UDP_DPORT;										//UDP(�㲥)ģʽ,Ŀ�������˿ں�
extern u8 NetNotFound ;                       //���綪ʧ��־

/***************----- �˿ڵ�����ģʽ -----***************/
extern unsigned char S0_Mode;									//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
#define TCP_SERVER		0x00										//TCP������ģʽ
#define TCP_CLIENT		0x01										//TCP�ͻ���ģʽ 
#define UDP_MODE			0x02										//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
extern unsigned char S0_State;								//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������)
#define S_INIT				0x01										//�˿���ɳ�ʼ�� 
#define S_CONN				0x02										//�˿��������,���������������� 

/***************----- �˿��շ����ݵ�״̬ -----***************/
extern unsigned char S0_Data;									//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ����������
#define S_RECEIVE			0x01										//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK	0x02										//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/
extern unsigned char Rx_Buffer[2048];					//�˿ڽ������ݻ�����
extern unsigned char Tx_Buffer[2048];					//�˿ڷ������ݻ�����

extern unsigned char W5500_Interrupt;					//W5500�жϱ�־(0:���ж�,1:���ж�)
typedef unsigned char SOCKET;									//�Զ���˿ں���������
extern void W5500_HARDWARE_INIT(void);        //W5500Ӳ����ʼ������
extern void Load_Net_Parameters(void); 			  //װ���������
extern void W5500_Socket_Set(void);    				//W5500�˿ڳ�ʼ������
extern int Process_Socket_Data(SOCKET s); 		//W5500���ղ����ͽ��յ�������
extern void Delay(unsigned int d);						//��ʱ����(ms)
extern void W5500_GPIO_Configuration(void);		//W5500 GPIO��ʼ������
extern void W5500_NVIC_Configuration(void);		//W5500 ���������ж����ȼ�����
extern void SPI_Configuration(void);					//W5500 SPI��ʼ������(STM32 SPI1)
extern void W5500_Hardware_Reset(void);				//Ӳ����λW5500
extern void W5500_Init(void);									//��ʼ��W5500�Ĵ�������
extern unsigned char Detect_Gateway(void);		//������ط�����
extern void Socket_Init(SOCKET s);						//ָ��Socket(0~7)��ʼ��
extern unsigned char Socket_Connect(SOCKET s);//����ָ��Socket(0~7)Ϊ�ͻ�����Զ�̷���������
extern unsigned char Socket_Listen(SOCKET s);	//����ָ��Socket(0~7)��Ϊ�������ȴ�Զ������������
extern unsigned char Socket_UDP(SOCKET s);		//����ָ��Socket(0~7)ΪUDPģʽ
extern unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr);							//ָ��Socket(0~7)�������ݴ���
extern void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size); 	//ָ��Socket(0~7)�������ݴ���
extern void W5500_Interrupt_Process(void);																									//W5500�жϴ��������
extern uint8_t W5500_UDP_SEND(uint8_t *ch,uint8_t Num); //ͨ��W5500����UDP����
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

