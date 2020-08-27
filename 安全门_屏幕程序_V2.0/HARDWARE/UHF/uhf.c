#include "uhf.h"
#include "hmi_user_uart.h"
#include "UHFhw_config.h"
#include "W5500.h"
#include "string.h"
#include "w25qxx.h"
#include "Screen.h"
#include "beep.h"
#include "hmi_driver.h"
#include "timer.h"





union
{
	unsigned char ucStr[12];
	struct
	{
		unsigned char ucSorting;                   //�ּ�
		unsigned char ucVersion;                   //�汾
		unsigned char ucContentIndex[2];           //��������
		unsigned char ucItemIdentifier[8];         //�ݲر�ʶ��
	} sPackDat;
} unEPCDat;





unsigned   int					 ReaderTimedOut = 0;   //��д����ʱ��־
int          waiting_time1 = 0;   //�жϽ���ʱ��
int          waiting_time2 = 0;   //�жϽ���ʱ��
int          clean_time =0;       //�����ʱ������־λ
int          save_time = 0;       //�洢��������
int          TestAntTimeOut =0 ;  //���߲��Գ�ʱ��־

u8   iStatus = 1;   //��ȫλ��Ϣ  1 Ϊ�����0 Ϊ����
u8   iVersion = 0;  //�汾�� ĿǰΪ 00��01��02��03
u8   alarmwork = 0;
u8   k = 0;
u8   NetCmd = 1;               //���籨������
u8   SendEPC = 0;              //����EPC��־λ
u32  EPC_LEN = 0;     			   //EPC����
u8   InventoryEPC[120];      //�̵㻺��EPC��,100��*12g���ֽ�(UID)
u8   Start = 0;
u8   touch =0;   //������Ļ��־
u8   touch1 =0;
u8   touch2 =0;
u8   sInvertNum = 1;            //��ת����
u8   OutOfOrder[]={0xb6,0xc1,0xd0,0xb4,0xc6,0xf7,0xb9,0xca,0xd5,0xcf,0x00};    //��д������
u8   AntNormal[] ={0xd5,0xfd,0xb3,0xa3,0x00};   //��������
u8   AntErr[] = {0xd2,0xec,0xb3,0xa3,0x00};     //�����쳣
u8   sInNum[] = {0xbd,0xf8,0xb9,0xdd,0xc8,0xcb,0xca,0xfd,0x00};  //����������ʾ
u8   sOutNum[] = {0xb3,0xf6,0xb9,0xdd,0xc8,0xcb,0xca,0xfd,0x00}; //����������ʾ
u8 	 NETSEND = 0;          //epc���͵ȴ����緵�ر�־
u8   Scanmode = 0 ;   //1 ���ߵ�ƽ��Ч�� 0���͵�ƽ��Ч; 2: �����̵�״̬
u8   PlusNum = 0; //�����鼮����
u8   tempEPC[901];
u8   UIDFlag = 0; //�ϴ�UID��־λ
u8   TotalEPC[100][50];  //����ͼ��
u8   BookCounter = 0;    //����ͼ����
u8   EPCCounter = 0;    //����EPC��
u8   SaveEPCFlag;       //�洢EPC��־
u8   StartAlarmFlag = 0; //������ʾ
int  KeepAlarm_time =0; //����ʱ��
int  KeepGateLinkage_time =0; //բ������ʱ��
u8   GateLinkage = 0; //բ������

CompareEPC_t CompareEPC_T;                             //�����Ƚ����ݽṹ�����

//---------------w25q16----------


/******************************************************************
	*���ƣUu8 UHF[]
	*���룺��
	*�������
	*����: UHFָ��
	*˵����
******************************************************************/
u8  UHF[] = {0x5a,0x55,0x06,0x00,0x0d,0x06,0x00,0xc8,0x6a,0x69//��ȡSN����ָ��
             ,0x5a,0x55,0x06,0x00,0x0d,0x03,0x00,0xc5,0x6a,0x69//�жϲ���ָ��
             ,0x5a,0x55,0x08,0x00,0x0d,0x03,0x50,0x01,0x02,0x1a,0x6a,0x69//�Ĵ�����ȡָ��
             ,0x5a,0x55,0x0c,0x00,0x0d,0x02,0x50,0x01,0x02,0x12,0x02,0x00,0x00,0x31,0x6a,0x69//�Ĵ���д��ָ��//�޷���ֵ
             ,0x5a,0x55,0x08,0x00,0x0d,0x03,0x50,0x05,0x00,0x1c,0x6a,0x69//�Ĵ�����ȡָ��
             ,0x5a,0x55,0x08,0x00,0x0d,0x03,0x50,0x00,0x07,0x1e,0x6a,0x69//�Ĵ�����ȡָ��
             ,0x5a,0x55,0x07,0x00,0x0d,0x1b,0x00,0x00,0xde,0x6a,0x69//��ȡ����״ָ̬��
             ,0x5a,0x55,0x07,0x00,0x0d,0x19,0x00,0x00,0xdc,0x6a,0x69//��ȡ��������ָ��
             ,0x5a,0x55,0x0c,0x00,0x0d,0x1c,0x00,0x00,0x2c,0x01,0x00,0x00,0x00,0x11,0x6a,0x69//��ȡ����פ����ָ��
             ,0x5a,0x55,0x07,0x00,0x0d,0x19,0x00,0x00,0xdc,0x6a,0x69//��ȡ��������ָ��
             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x00,0x01,0xdf,0x6a,0x69//��������״ָ̬��
             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x00,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3b,0x6a,0x69//������������ָ��


             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x01,0x01,0xe0,0x6a,0x69//��������״ָ̬//2
             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x01,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3c,0x6a,0x69//������������ָ��//2

             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x02,0x01,0xe1,0x6a,0x69//��������״ָ̬//3
             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x02,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3d,0x6a,0x69//������������ָ��//3

//             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x03,0x01,0xe2,0x6a,0x69//��������״ָ̬//4
//             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x03,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3e,0x6a,0x69//������������ָ��//4



             ,0x5A,0x55,0x0C,0x00,0x0D,0x02,0x50,0x00,0x07,0xFF,0xFF,0x00,0x00,0x1F,0x6A,0x69//�Ĵ�����ȡָ��
             ,0x5A,0x55,0x06,0x00,0x0D,0x20,0x00,0xE2,0x6A,0x69//��ȡ�Ự��Ϣָ��
             ,0x5A,0x55,0x07,0x00,0x0D,0x1F,0x00,0x00,0xE2,0x6A,0x69//���ûỰָ��
             ,0x5A,0x55,0x0E,0x00,0x0D,0x21,0x00,0x01,0x04,0x00,0x01,0x03,0x00,0x0F,0x04,0x07,0x6A,0x69//���õ�����ǩ�㷨ָ��
             ,0x5A,0x55,0x07,0x00,0x0D,0x1E,0x00,0x01,0xE2,0x6A,0x69//����link profile������Ϣָ��
             ,0x5A,0x55,0x08,0x00,0x0D,0x03,0x50,0x00,0x00,0x17,0x6A,0x69//�Ĵ�����ȡָ��
             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x00,0xDE,0x6A,0x69//��ȡ����״ָ̬��
             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x00,0xDC,0x6A,0x69//��ȡ��������ָ��
             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x00,0x2C,0x01,0x00,0x00,0x00,0x11,0x6A,0x69//��ȡ����פ����ָ��
             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x01,0xDF,0x6A,0x69//��ȡ����״ָ̬��
             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x01,0xDD,0x6A,0x69//��ȡ��������ָ��
             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0xE5,0x6A,0x69//��ȡ����פ����ָ��
             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x02,0xE0,0x6A,0x69//��ȡ����״ָ̬��
             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x02,0xDE,0x6A,0x69//��ȡ��������ָ��
             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0xE6,0x6A,0x69//��ȡ����פ����ָ��
//             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x03,0xE1,0x6A,0x69//��ȡ����״ָ̬��
//             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x03,0xDF,0x6A,0x69//��ȡ��������ָ��
//             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xE7,0x6A,0x69// ��ȡ����פ����ָ��
             ,0x5A,0x55,0x06,0x00,0x0D,0x1D,0x00,0xDF,0x6A,0x69//��ȡlinkprofile������Ϣָ��
             ,0x5A,0x55,0x08,0x00,0x0D,0x03,0x50,0x00,0x07,0x1E,0x6A,0x69//�Ĵ�����ȡָ��
             ,0x5A,0x55,0x06,0x00,0x0D,0x20,0x00,0xE2,0x6A,0x69//��ȡ�Ự��Ϣָ��
             ,0x5A,0x55,0x06,0x00,0x0D,0x25,0x00,0xE7,0x6A,0x69// ��ȡmask����ָ��
             ,0x5A,0x55,0x06,0x00,0x0D,0x22,0x00,0xE4,0x6A,0x69//��ȡ��λ����ǩ�㷨ָ��
             ,0x00
            };



vu8   RecvSucceed = 0;   //volatile  unsigned char
vu8   RecvBuf[100];
vu8   MidBuf[100];
u32   adder = 0 ;       //unsigned int
u32   UHFLen = 0;
u8 TransBuf[100];       //ת�����Buf
/******************************************************************
	*����: void UHFInit(u32 POWER)
	*���룺��
	*�������
	*����: ��ʼ��UHF�����ù���
	*˵����
******************************************************************/


void UHFInit(u32 POWER)
{
	u8 i;
	int TryTimer = 0;												       //ָ��Լ���
	while(1)
	{
		memset(RecvBuf,0,sizeof(RecvBuf));
		UHFLen =(UHF[adder+3]<<8)+UHF[adder+2]+4;
		memcpy(MidBuf,UHF+adder,UHFLen);


//---------------SET POWER--------------------
		if((MidBuf[2]==0x13)&&(MidBuf[3]==0x00)&&(MidBuf[4]==0x0d)&&(MidBuf[5]==0x18)&&(MidBuf[6]==0x00))
		{
			MidBuf[8] =  POWER%0x100;
			MidBuf[9] =  POWER/0x100;

			MidBuf[20]  = 0;
			for(i=0; i<20; i++)
			{
				MidBuf[20] +=MidBuf[i];  //У���
			}
		}
//---------------------------------------------
		USART1_SendData(MidBuf,UHFLen);
		if((MidBuf[5]==0x02)&&(MidBuf[6]==0x50))
		{
			;
		}
		else
		{
			while(RecvSucceed == 0)
			{
				if( ReaderTimedOut >=2)                              //����1sû�н������
				{
					USART1_SendData(MidBuf,UHFLen);                    //�ٴη����̵�ָ��
					ReaderTimedOut = 0; 								  					 //���ó�ʱ������
					TryTimer++;																			 //���Դ���+1
					if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		      //��ջ���
						break;
					}
				}
			}
			RecvSucceed = 0;

		}
		adder += UHFLen;
		if(UHF[adder] == 0x00)break;
	}

}


/******************************************************************
	*����: void StartScan(void)
	*���룺��
	*�������
	*����: ��ʼɨ��
	*˵����
******************************************************************/
void StartScan(void)
{
	int TryTimer = 0;												       //�̵�ָ��Լ���
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x11\x00\x00\x01\xD6\x6A\x69",12);          //�����̵�ָ��
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{

			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\x5A\x55\x08\x00\x0D\x11\x00\x00\x01\xD6\x6A\x69",12);  //�ٴη����̵�ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
				{
					memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
					StopScan();
					SetTextValue(0,4,OutOfOrder);
					break;
				}
			}
		}
		RecvSucceed = 0;
	}
}

/******************************************************************
	*����: void TagRead(void)
	*���룺��
	*�������
	*����: ��ǩ��ȡ
	*˵������ȡEPC��TID
******************************************************************/
void TagRead(void)
{
	int TryTimer = 0;												       //�̵�ָ��Լ���
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x11\x00\x0D\x12\x00\x02\x00\x00\x06\x00\x00\x00\x00\x00\x01\x01\xe9\x6A\x69",21);          //�����̵�ָ��
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{

			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\x5A\x55\x11\x00\x0D\x12\x00\x02\x00\x00\x06\x00\x00\x00\x00\x00\x01\x01\xe9\x6A\x69",21);          //�ٴη����̵�ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
				{
					memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
					StopScan();
					SetTextValue(0,4,OutOfOrder);
					break;
				}
			}
		}
		RecvSucceed = 0;
	}
}


/******************************************************************
	*����: void StopScan(void)
	*���룺��
	*�������
	*����: ֹͣɨ��
	*˵����
******************************************************************/
void StopScan(void)
{
	int TryTimer = 0;												       //ָ��Լ���
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x06\x00\x0D\x03\x00\xc5\x6A\x69",10);
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	while(ABORT_Succeed == 0)
	{
		if( ReaderTimedOut >= 4)                              //����3sû�н������
		{
			USART1_SendData("\x5A\x55\x06\x00\x0D\x03\x00\xc5\x6A\x69",10);  //�ٴη����̵�ָ��
			ReaderTimedOut = 0; 								  					 //���ó�ʱ������
			TryTimer++;																			 //���Դ���+1
			if(TryTimer >=3)																 //����3�γ���û�з���
			{
				memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
				SetTextValue(0,4,OutOfOrder);
				break;
			}
		}
	}
	RecvSucceed = 0;

}
void ScanSetC()
{
	USART1_SendData("\x5A\x55\x0c\x00\x0D\x02\x50\x00\x07\x01\x00\x00\x00\x22\x6A\x69",16);
	delay_ms(500);
	USART1_SendData("\x5A\x55\x0c\x00\x0D\x02\x50\x00\x05\x00\x05\x00\x00\x24\x6A\x69",16);
	delay_ms(500);
	USART1_SendData("\x5A\x55\x0c\x00\x0D\x02\x50\x01\x05\xc8\x00\x00\x00\xe8\x6A\x69",16);
	delay_ms(500);
	USART1_SendData("\x5A\x55\x0c\x00\x0D\x02\x50\x00\xf0\x02\x00\x00\x00\x0c\x6A\x69",16);
	delay_ms(500);
}
void AntSetC()
{
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x01\xdf\x6A\x69",12);
	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x01\x01\xe0\x6A\x69",12);
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x02\x01\xe1\x6A\x69",12);
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x03\x01\xe2\x6A\x69",12);
//	delay_ms(2000);
		USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x01\x01\xe0\x6A\x69",12);
	delay_ms(2000);
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x02\x02\xe1\x6A\x69",12);
	delay_ms(2000);
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x03\x03\xe2\x6A\x69",12);
	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x00\xa0\x00\x00\x00\x96\x00\x00\x00\xc8\x00\x00\x00\xe5\x6a\x69",23); //16w
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x01\xa0\x00\x00\x00\x96\x00\x00\x00\xc8\x00\x00\x00\xe6\x6a\x69",23);
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x02\xa0\x00\x00\x00\x96\x00\x00\x00\xc8\x00\x00\x00\xe7\x6a\x69",23);
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x03\xa0\x00\x00\x00\x96\x00\x00\x00\xc8\x00\x00\x00\xe8\x6a\x69",23);
//	delay_ms(2000);
	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x00\xFA\x00\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x62\x6a\x69",23);    //25w
	delay_ms(2000);
	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x01\xFA\x00\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x63\x6a\x69",23);    //25w
	delay_ms(2000);
	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x02\xFA\x00\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x64\x6a\x69",23);    //25w
	delay_ms(2000);
	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x03\xFA\x00\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x65\x6a\x69",23);    //25w
	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x00\x2c\x01\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x95\x6a\x69",23);    //30w
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x01\x2c\x01\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x96\x6a\x69",23);    //30w
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x02\x2c\x01\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x96\x6a\x69",23);    //30w
//	delay_ms(2000);
//	USART1_SendData("\x5A\x55\x13\x00\x0D\x18\x00\x03\x2c\x01\x00\x00\xAA\x00\x00\x00\xD0\x07\x00\x00\x97\x6a\x69",23);    //30w
	delay_ms(2000);
	//USART1_SendData("\x5A\x55\x08\x00\x0D\x11\x00\x00\x01\xd6\x6A\x69",12);  //�ٴη����̵�ָ��
}


/******************************************************************
	*����: void StartAlarm(void)
	*���룺u8 TurnstileMode; u8 TryTimerNum;
	*�������
	*����: ��ʼ����
	*˵����TurnstileMode: 0Ϊ��բ��ģʽ��1Ϊբ��ģʽ��
******************************************************************/
void StartAlarm(void)
{
	if(NetCmd == 1)
	{
		NetCmd = 0;
		StartAlarmFlag = 1;
		KeepAlarm_time =0;
		KeepGateLinkage_time =0;
		GateLinkage = 1;
	}
}

/******************************************************************
	*����: void TriggerSensors(void)
	*���룺��
	*�������
	*����: ������Ļ����ʼɨ��
	*˵���������Ļ����������ʼɨ��
******************************************************************/
void TriggerSensors(void)
{
	u8 state,p_Counter,i;
		u8 FrameContent[50];     //
	state = Sensor_Scan(Scanmode);
	if(Scanmode <2)
	{
		if(clean_time>=4)     //2�� û�д�����һ������������մ�����־λ
		{
			touch1=touch2=0;
		}
		if(sInvertNum == 1)
		{
			if(touch1 && touch2&& (waiting_time1 - waiting_time2 >= 1) )
			{
				touch1=touch2=0;
				waiting_time1 = waiting_time2=0;
				OutNum +=1;
				SetTextValueInt32(0,3,OutNum);  //������ʾ��������
				SetTextValueInt32(2,3,OutNum);  //������ʾ��������
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
				if(phone == 1)
				{
					MakeTCPFrame(0x1B,0xfe,FrameContent);
				}
					W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
				
			}
			else if(touch2 && touch1 && (waiting_time2 - waiting_time1 >= 1))
			{
				touch1=touch2=0;
				waiting_time1 = waiting_time2 = 0;
				InNum +=1;
				SetTextValueInt32(0,2,InNum);  //���½�������
				SetTextValueInt32(2,2,InNum);  //���½�������
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
				if(phone ==1)
				{
					MakeTCPFrame(0x1B,0xfe,FrameContent);
				}
				W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));				
			}
		}
		else if(sInvertNum == 2)
		{
			if(touch1 && touch2&& (waiting_time1 - waiting_time2 >= 1) )
			{
				touch1=touch2=0;
				waiting_time1 = waiting_time2=0;
				InNum +=1;
				SetTextValueInt32(0,2,InNum);  //������ʾ��������
				SetTextValueInt32(2,2,InNum);  //������ʾ��������
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
				if(phone == 1)
				{
					MakeTCPFrame(0x1B,0xfe,FrameContent);
				}
				W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			}
			else if(touch2 && touch1 && (waiting_time2 - waiting_time1 >= 1))
			{
				touch1=touch2=0;
				waiting_time1 = waiting_time2 = 0;
				OutNum +=1;
				SetTextValueInt32(0,3,OutNum);  //���½�������
				SetTextValueInt32(2,3,OutNum);  //���½�������
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
				if(phone ==1 )
				{
					MakeTCPFrame(0x1B,0xfe,FrameContent);
				}
				W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
			}
		}
		if(state == 1)
		{
      startscanFlag =1;
			KeepScan_time =0; //�̵�ʱ������
			if(touch == 0)
			{

				touch =1;
				StartScan(); /* ��ʼɨ��*/
				//TagRead();
				touch = 1;
				Start = 1;

			}
			k = 0;
		}
		else
		{
			;
		}
	}
	else if(Scanmode == 2)
	{
		Start = 1;
	}
		
}


/******************************************************************
*����: void DetectAlarm(u32 state)
*���룺ɨ��״̬
*�������
*����: �ж��Ƿ񱨾�
*˵����
******************************************************************/
void DetectAlarm(u32 state, u8 mode)
{
	int i,j,k;
	u8  pContent =0;
	u16  CrcVal;
	u8 tempdata[30];
	SaveEPCFlag =0;
	if(state == 1)/* ɨ��ͼ��*/
	{
		if(RecvSucceed == 0x01)
		{
			if(((RecvBuf[RevcLens+2] == 0x6a)&&(RecvBuf[RevcLens+3] == 0x69))||(((RecvBuf[4]==0x0d)&&(RecvBuf[5] == 0x05))))
			{
				Transform(RecvBuf, RevcLens);
				if((TransBuf[4]==0x0d)&&(TransBuf[5] == 0x05)&&(TransBuf[6]==0x00))
				{
					EPC_LEN = (TransBuf[16]<<8)+(TransBuf[15]);
					for(i=0; i<12; ++i)
					{
						unEPCDat.ucStr[i] = TransBuf[17+i];
					}
					iStatus =(unsigned int)unEPCDat.sPackDat.ucSorting & 0x80;
					iVersion = (unsigned int)unEPCDat.sPackDat.ucVersion;
#if debug
					printf("EPC_LEN:%d \r\n",EPC_LEN);
#endif
					switch (mode)
					{
						case LOU:
						  if((iStatus == 0)&& (iVersion <4))
							{
								//NetCmd = 1;
								StartAlarm();
              }
							break;
						case HaiNan:
							if(unEPCDat.sPackDat.ucSorting == 0xc2)  //Զ����ģʽ
							{
								if((unEPCDat.sPackDat.ucItemIdentifier[7] == 0x05) || (unEPCDat.sPackDat.ucItemIdentifier[7] == 0x09))
								{
								//NetCmd = 1;
								StartAlarm();
								}
							}
							break;
						case XiaMen:
							if((unEPCDat.sPackDat.ucSorting == 0xc2) && (unEPCDat.sPackDat.ucItemIdentifier[7]==0x04))  //Զ����ģʽ
							{
								//NetCmd = 1;
								StartAlarm();
							}
							break;
						case LOU_Phone:
							if((iStatus == 0) && (iVersion <4)) /* ��׼0 ��ʱ�򱨾�*/
							{
									UIDFlag = 1;
								  CompareRepeatAndSend(unEPCDat.ucStr);
							}
							break;
						case HaiNan_Phone:
							if(unEPCDat.sPackDat.ucSorting == 0xc2)  //Զ����ģʽ
							{
								if((unEPCDat.sPackDat.ucItemIdentifier[7] == 0x05) || (unEPCDat.sPackDat.ucItemIdentifier[7] == 0x09))
								{
									UIDFlag = 1;
									InventoryEPC[pContent++] = 0x05;               //֡ͷ
									InventoryEPC[pContent++] = (EPC_LEN + 5);      //����
									InventoryEPC[pContent++] = 0xf1;               //����EPC������
									for(i=0; i<EPC_LEN; i++)
									{
										InventoryEPC[pContent++] = unEPCDat.ucStr[i];    //����
									}
									CrcVal = cal_crc16(InventoryEPC,EPC_LEN+3);    //Crc16 У��
									InventoryEPC[pContent++] = CrcVal>>8;
									InventoryEPC[pContent++] = CrcVal;
									InventoryEPC[pContent++] = 0x06;               //֡β
									W5500_TCP_SEND(InventoryEPC,EPC_LEN+6);
								}
							}
							break;
						case XiaMen_Phone:
							if((TransBuf[24] &0x04)==0x04) //Զ����ģʽ
							{
									UIDFlag = 1;
									InventoryEPC[pContent++] = 0x05;               //֡ͷ
									InventoryEPC[pContent++] = (EPC_LEN + 5);      //����
									InventoryEPC[pContent++] = 0xf1;               //����EPC������
									for(i=0; i<EPC_LEN; i++)
									{
										InventoryEPC[pContent++] = unEPCDat.ucStr[i];    //����
									}
									CrcVal = cal_crc16(InventoryEPC,EPC_LEN+3);    //Crc16 У��
									InventoryEPC[pContent++] = CrcVal>>8;
									InventoryEPC[pContent++] = CrcVal;
									InventoryEPC[pContent++] = 0x06;               //֡β
									W5500_TCP_SEND(InventoryEPC,EPC_LEN+6);
							}
							break;
						case Custom:               //�Զ��壬����δ��ɡ�����
							if(unEPCDat.ucStr[0] == 0x01)
							{
								NetCmd = 1;
								StartAlarm();
								delay_ms(2000);
							}
							break;
								
						default:
							break;
          }
				
			}
			if((TransBuf[0]==0x0d)&&(TransBuf[1] == 0x06)&&(UIDFlag == 0x01))
			{
				UIDFlag = 0;
				InventoryEPC[pContent++] = 0x05;               //֡ͷ
				InventoryEPC[pContent++] = (12 +5);      //����
				InventoryEPC[pContent++] = 0xf2;               //����TID
				for(i=0; i<12; i++)
				{
					InventoryEPC[pContent++] = TransBuf[i+14];    //����
				}
				CrcVal = cal_crc16(InventoryEPC,12+3);    //Crc16 У��
				InventoryEPC[pContent++] = CrcVal>>8;
				InventoryEPC[pContent++] = CrcVal;
				InventoryEPC[pContent++] = 0x06;               //֡β
				W5500_UDP_SEND(InventoryEPC,12+6);
				delay_ms(500);
			}
		}
			else
			{
				// get out;
			}
			RecvFlag = 0x00;
			RecvSucceed = 0x00;
			memset(RecvBuf,0,sizeof(RecvBuf));
		  memset(TransBuf,0,sizeof(TransBuf));
		}
	}
}



/******************************************************************
*����: u8 Sensor_Scan(u8 scanmode)
*���룺��
*�������
*����: �ж��Ƿ񴥷���Ļ
*˵��������1������������0��δ������
******************************************************************/
u8 Sensor_Scan(u8 scanmode)
{
	u8 cleanscreen[] = {0};
	if((INP_PIN1 == scanmode)||(INP_PIN2 == scanmode))
	{
		delay_ms(10);                                     //������
		if((INP_PIN1 == scanmode))
		{
			waiting_time1 = 0;
			clean_time = 0;
			touch1 =1;
			return 1;
		}
		if((INP_PIN2 == scanmode))
		{
			waiting_time2 = 0;
			clean_time = 0;
			touch2=1;
			return 1;
		}
		else
			return 0;
	}
	else
	{
		return 0;
	}

}


/******************************************************************
	*����: void Transform(unsigned char * buf, unsigned long datalen) 
	*����:buf; datalen
	*���:buf
	*����: ת�� 99 66  -> 99
	*˵��:
******************************************************************/
void Transform(u8 * buf, unsigned long datalen)   //, unsigned long * len
{
    u8 bNeedTransform = 0;
    u8 len;
 
	  
	  int i = 0, j = 0;
	   len=datalen;
   // *len = datalen;
    
    for (i = 0; i < datalen; i++)
    {
        if (bNeedTransform)
        {
            buf[i] ^= 0xFF;
            TransBuf[j++] = buf[i];
            (len)--;
            bNeedTransform = 0;
        }
        else
        {
            if (buf[i] == 0x99)
            {
                bNeedTransform = 1;
            }
            else
            {
                TransBuf[j++] = buf[i];
            }
        }
    }
}


/******************************************************************
	*����: void TestANT(u8 ANTID) 
	*����:ANTID
	*���:����״��
	*����: �������
	*˵��:
******************************************************************/
void TestANT(u8 ANTID)
{
	switch(ANTID)
	{
		case 1:
			OpenANT1();
		  StopANT2();
			StopANT3();
			StopANT4();
		  StartScan();
		  delay_ms(100);
		  TestAntTimeOut =0;
			while (1)
			{
				if(RecvSucceed == 0x01)
				{
					if(RecvBuf[0] == 0x0d)
					{
						if(RecvBuf[1] == 0x0e)
						{
							RecvSucceed = 0;
							SetTextValue(7,2,AntNormal);         //һ������
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,2,AntErr);           //�쳣
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}	
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //��д���쳣
					SoftReset();
				}
			}			
			break;
		case 2:
			OpenANT2();
		  StopANT1();
			StopANT3();
			StopANT4();
		  StartScan();
		  TestAntTimeOut = 0;
			while(1)
			{
				if(RecvSucceed == 0x01)
				{
					if(RecvBuf[0] == 0x0d)
					{
						if(RecvBuf[1] == 0x0e)
						{
							RecvSucceed = 0;
							SetTextValue(7,3,AntNormal);         //һ������
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,3,AntErr);           //�쳣
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}	
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //��д���쳣
					SoftReset();
				}
		  }
			break;
		case 3:
			OpenANT3();
		  StopANT1();
			StopANT2();
			StopANT4();
		  StartScan();
		  TestAntTimeOut =0;
			while(1)
			{
				if(RecvSucceed == 0x01)
				{
					if(RecvBuf[0] == 0x0d)
					{
						if(RecvBuf[1] == 0x0e)
						{
							RecvSucceed = 0;
							SetTextValue(7,4,AntNormal);         //һ������
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,4,AntErr);           //�쳣
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}	
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //��д���쳣
					SoftReset();
				}			
			}
			break;
		case 4:
			OpenANT4();
		  StopANT1();
			StopANT2();
			StopANT3();
		  StartScan();
		  TestAntTimeOut =0;
			while(1)
			{
				if(RecvSucceed == 0x01)
				{
					if(RecvBuf[0] == 0x0d)
					{
						if(RecvBuf[1] == 0x0e)
						{
							RecvSucceed = 0;
							SetTextValue(7,5,AntNormal);         //һ������
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,5,AntErr);           //�쳣
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //��д���쳣
					SoftReset();
				}			
			}
			break;
		default:
			break;
	}

}

void OpenANT1 (void)
{
	int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x01\xdf\x6A\x69",12);          //��һ������ 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x01\xdf\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}

void OpenANT2 (void)
{
	int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x01\x01\xe0\x6A\x69",12);          //���������� 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x01\x01\xe0\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}
void OpenANT3 (void)
{
		int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x02\x01\xe1\x6A\x69",12);          //���������� 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x02\x01\xe1\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}
void OpenANT4 (void)
{
		int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x03\x01\xe2\x6A\x69",12);          //���ĺ����� 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x03\x01\xe2\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}
void StopANT1 (void)
{
		int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x00\xde\x6A\x69",12);          //��һ������ 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x00\x00\xde\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}
void StopANT2 (void)
{
		int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x01\x00\xdf\x6A\x69",12);          //�ض������� 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x01\x00\xdf\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}
void StopANT3 (void)
{
		int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x02\x00\xe0\x6A\x69",12);          //���������� 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x02\x00\xe0\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}
void StopANT4 (void)
{
		int TryTimer = 0;
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x03\x00\xe1\x6A\x69",12);          //���ĺ����� 
	ReaderTimedOut = 0; 																   //���ó�ʱ������
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //����1sû�н������
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x03\x00\xe1\x6A\x69",12);  //�ٴη���ָ��
				ReaderTimedOut = 0; 								  					 //���ó�ʱ������
				TryTimer++;																			 //���Դ���+1
				if(TryTimer >=3)																 //����3�γ���û�з���
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //��ջ���
						SetTextValue(7,11,OutOfOrder);          // ��������ֶ�д������
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}

/****************************************************************************************************

����ԭ��:    	int CompareRepeatAndSend(uint8_t *EPCAndTID)
��������:			�Ƚ����ݣ���ͬ���ݾͲ����룬��ͬ����						
�������:			u8 *TempContent
�������:      ��
����ֵ:        ��
****************************************************************************************************/
int CompareRepeatAndSend(u8 *EPCAndTID)
{
	u8 i,j,m,RWFlag;
	u8 tempdata[12];
	RWFlag = 0;
	for(i=0;i<CompareEPC_T.s_EPCCounter;i++)                                					 //�������еı�ǩ
	{
	  if(memcmp(&EPCAndTID[0],&CompareEPC_T.s_EPC[i][0],12) == 0x00)                   //���EPC��ͬ
		{
			RWFlag = 1;                                                                    //�����
		  return 0xFF;                                                                   //�˳�����
		}
	}
	if(RWFlag == 0x00)                                                                 //���û���ظ�EPC
	{
		memcpy(&CompareEPC_T.s_EPC[CompareEPC_T.s_EPCCounter++][0],&EPCAndTID[0],12);    //����EPC�����б�
		memcpy(tempdata,&EPCAndTID[0],12);
		MakeTCPFrame(18,0xf1,tempdata);                                                  //�����µ�EPC
		return 0x01;
	}	
}


/****************************************************************************************************

����ԭ��:    	u8 check_sum(u8 *buf, int len)
��������:			У���					
�������:			u8 *buf
�������:      ��
����ֵ:        ����0����˵�����յ���������ȷ�ģ�
****************************************************************************************************/
u8 check_sum(u8 *buf, int len)
{
	int count = len;
	u8 i,sum = 0;
	u8 *w=buf;

	
	for(i=0;i<count;i++)
	{
		sum+=*w++;
	}
	return sum;
	
}


//**************



