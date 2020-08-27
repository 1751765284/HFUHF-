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
		unsigned char ucSorting;                   //分拣
		unsigned char ucVersion;                   //版本
		unsigned char ucContentIndex[2];           //内容索引
		unsigned char ucItemIdentifier[8];         //馆藏标识符
	} sPackDat;
} unEPCDat;





unsigned   int					 ReaderTimedOut = 0;   //读写器超时标志
int          waiting_time1 = 0;   //判断进出时间
int          waiting_time2 = 0;   //判断进出时间
int          clean_time =0;       //清除超时进出标志位
int          save_time = 0;       //存储进出人数
int          TestAntTimeOut =0 ;  //天线测试超时标志

u8   iStatus = 1;   //安全位信息  1 为借出，0 为馆内
u8   iVersion = 0;  //版本号 目前为 00；01；02；03
u8   alarmwork = 0;
u8   k = 0;
u8   NetCmd = 1;               //网络报警命令
u8   SendEPC = 0;              //发送EPC标志位
u32  EPC_LEN = 0;     			   //EPC长度
u8   InventoryEPC[120];      //盘点缓存EPC号,100本*12g个字节(UID)
u8   Start = 0;
u8   touch =0;   //触发光幕标志
u8   touch1 =0;
u8   touch2 =0;
u8   sInvertNum = 1;            //反转人数
u8   OutOfOrder[]={0xb6,0xc1,0xd0,0xb4,0xc6,0xf7,0xb9,0xca,0xd5,0xcf,0x00};    //读写器故障
u8   AntNormal[] ={0xd5,0xfd,0xb3,0xa3,0x00};   //天线正常
u8   AntErr[] = {0xd2,0xec,0xb3,0xa3,0x00};     //天线异常
u8   sInNum[] = {0xbd,0xf8,0xb9,0xdd,0xc8,0xcb,0xca,0xfd,0x00};  //进馆人数显示
u8   sOutNum[] = {0xb3,0xf6,0xb9,0xdd,0xc8,0xcb,0xca,0xfd,0x00}; //进馆人数显示
u8 	 NETSEND = 0;          //epc发送等待网络返回标志
u8   Scanmode = 0 ;   //1 ：高电平有效； 0：低电平有效; 2: 持续盘点状态
u8   PlusNum = 0; //缓存书籍数量
u8   tempEPC[901];
u8   UIDFlag = 0; //上传UID标志位
u8   TotalEPC[100][50];  //缓存图书
u8   BookCounter = 0;    //缓存图书量
u8   EPCCounter = 0;    //缓存EPC量
u8   SaveEPCFlag;       //存储EPC标志
u8   StartAlarmFlag = 0; //报警标示
int  KeepAlarm_time =0; //报警时间
int  KeepGateLinkage_time =0; //闸机联动时间
u8   GateLinkage = 0; //闸机联动

CompareEPC_t CompareEPC_T;                             //声明比较数据结构体变量

//---------------w25q16----------


/******************************************************************
	*名称u8 UHF[]
	*输入：无
	*输出：无
	*功能: UHF指令
	*说明：
******************************************************************/
u8  UHF[] = {0x5a,0x55,0x06,0x00,0x0d,0x06,0x00,0xc8,0x6a,0x69//获取SN操作指令
             ,0x5a,0x55,0x06,0x00,0x0d,0x03,0x00,0xc5,0x6a,0x69//中断操作指令
             ,0x5a,0x55,0x08,0x00,0x0d,0x03,0x50,0x01,0x02,0x1a,0x6a,0x69//寄存器读取指令
             ,0x5a,0x55,0x0c,0x00,0x0d,0x02,0x50,0x01,0x02,0x12,0x02,0x00,0x00,0x31,0x6a,0x69//寄存器写入指令//无反回值
             ,0x5a,0x55,0x08,0x00,0x0d,0x03,0x50,0x05,0x00,0x1c,0x6a,0x69//寄存器读取指令
             ,0x5a,0x55,0x08,0x00,0x0d,0x03,0x50,0x00,0x07,0x1e,0x6a,0x69//寄存器读取指令
             ,0x5a,0x55,0x07,0x00,0x0d,0x1b,0x00,0x00,0xde,0x6a,0x69//获取天线状态指令
             ,0x5a,0x55,0x07,0x00,0x0d,0x19,0x00,0x00,0xdc,0x6a,0x69//获取天线配置指令
             ,0x5a,0x55,0x0c,0x00,0x0d,0x1c,0x00,0x00,0x2c,0x01,0x00,0x00,0x00,0x11,0x6a,0x69//获取天线驻波比指令
             ,0x5a,0x55,0x07,0x00,0x0d,0x19,0x00,0x00,0xdc,0x6a,0x69//获取天线配置指令
             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x00,0x01,0xdf,0x6a,0x69//设置天线状态指令
             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x00,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3b,0x6a,0x69//设置天线配置指令


             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x01,0x01,0xe0,0x6a,0x69//设置天线状态指//2
             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x01,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3c,0x6a,0x69//设置天线配置指令//2

             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x02,0x01,0xe1,0x6a,0x69//设置天线状态指//3
             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x02,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3d,0x6a,0x69//设置天线配置指令//3

//             ,0x5a,0x55,0x08,0x00,0x0d,0x1a,0x00,0x03,0x01,0xe2,0x6a,0x69//设置天线状态指//4
//             ,0x5a,0x55,0x13,0x00,0x0d,0x18,0x00,0x03,0x2c,0x01,0x00,0x00,0xc8,0x00,0x00,0x00,0x40,0x1f,0x00,0x00,0x3e,0x6a,0x69//设置天线配置指令//4



             ,0x5A,0x55,0x0C,0x00,0x0D,0x02,0x50,0x00,0x07,0xFF,0xFF,0x00,0x00,0x1F,0x6A,0x69//寄存器读取指令
             ,0x5A,0x55,0x06,0x00,0x0D,0x20,0x00,0xE2,0x6A,0x69//获取会话信息指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x1F,0x00,0x00,0xE2,0x6A,0x69//设置会话指令
             ,0x5A,0x55,0x0E,0x00,0x0D,0x21,0x00,0x01,0x04,0x00,0x01,0x03,0x00,0x0F,0x04,0x07,0x6A,0x69//设置单化标签算法指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x1E,0x00,0x01,0xE2,0x6A,0x69//设置link profile配置信息指令
             ,0x5A,0x55,0x08,0x00,0x0D,0x03,0x50,0x00,0x00,0x17,0x6A,0x69//寄存器读取指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x00,0xDE,0x6A,0x69//获取天线状态指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x00,0xDC,0x6A,0x69//获取天线配置指令
             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x00,0x2C,0x01,0x00,0x00,0x00,0x11,0x6A,0x69//获取天线驻波比指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x01,0xDF,0x6A,0x69//获取天线状态指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x01,0xDD,0x6A,0x69//获取天线配置指令
             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0xE5,0x6A,0x69//获取天线驻波比指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x02,0xE0,0x6A,0x69//获取天线状态指令
             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x02,0xDE,0x6A,0x69//获取天线配置指令
             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0xE6,0x6A,0x69//获取天线驻波比指令
//             ,0x5A,0x55,0x07,0x00,0x0D,0x1B,0x00,0x03,0xE1,0x6A,0x69//获取天线状态指令
//             ,0x5A,0x55,0x07,0x00,0x0D,0x19,0x00,0x03,0xDF,0x6A,0x69//获取天线配置指令
//             ,0x5A,0x55,0x0C,0x00,0x0D,0x1C,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xE7,0x6A,0x69// 获取天线驻波比指令
             ,0x5A,0x55,0x06,0x00,0x0D,0x1D,0x00,0xDF,0x6A,0x69//获取linkprofile配置信息指令
             ,0x5A,0x55,0x08,0x00,0x0D,0x03,0x50,0x00,0x07,0x1E,0x6A,0x69//寄存器读取指令
             ,0x5A,0x55,0x06,0x00,0x0D,0x20,0x00,0xE2,0x6A,0x69//获取会话信息指令
             ,0x5A,0x55,0x06,0x00,0x0D,0x25,0x00,0xE7,0x6A,0x69// 获取mask设置指令
             ,0x5A,0x55,0x06,0x00,0x0D,0x22,0x00,0xE4,0x6A,0x69//获取单位化标签算法指令
             ,0x00
            };



vu8   RecvSucceed = 0;   //volatile  unsigned char
vu8   RecvBuf[100];
vu8   MidBuf[100];
u32   adder = 0 ;       //unsigned int
u32   UHFLen = 0;
u8 TransBuf[100];       //转换后的Buf
/******************************************************************
	*名称: void UHFInit(u32 POWER)
	*输入：无
	*输出：无
	*功能: 初始化UHF，设置功率
	*说明：
******************************************************************/


void UHFInit(u32 POWER)
{
	u8 i;
	int TryTimer = 0;												       //指令尝试计数
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
				MidBuf[20] +=MidBuf[i];  //校验和
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
				if( ReaderTimedOut >=2)                              //超过1s没有结果返回
				{
					USART1_SendData(MidBuf,UHFLen);                    //再次发送盘点指令
					ReaderTimedOut = 0; 								  					 //重置超时计数器
					TryTimer++;																			 //尝试次数+1
					if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		      //清空缓存
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
	*名称: void StartScan(void)
	*输入：无
	*输出：无
	*功能: 开始扫描
	*说明：
******************************************************************/
void StartScan(void)
{
	int TryTimer = 0;												       //盘点指令尝试计数
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x08\x00\x0D\x11\x00\x00\x01\xD6\x6A\x69",12);          //发送盘点指令
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{

			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\x5A\x55\x08\x00\x0D\x11\x00\x00\x01\xD6\x6A\x69",12);  //再次发送盘点指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
				{
					memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
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
	*名称: void TagRead(void)
	*输入：无
	*输出：无
	*功能: 标签读取
	*说明：读取EPC和TID
******************************************************************/
void TagRead(void)
{
	int TryTimer = 0;												       //盘点指令尝试计数
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x11\x00\x0D\x12\x00\x02\x00\x00\x06\x00\x00\x00\x00\x00\x01\x01\xe9\x6A\x69",21);          //发送盘点指令
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{

			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\x5A\x55\x11\x00\x0D\x12\x00\x02\x00\x00\x06\x00\x00\x00\x00\x00\x01\x01\xe9\x6A\x69",21);          //再次发送盘点指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
				{
					memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
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
	*名称: void StopScan(void)
	*输入：无
	*输出：无
	*功能: 停止扫描
	*说明：
******************************************************************/
void StopScan(void)
{
	int TryTimer = 0;												       //指令尝试计数
	memset(RecvBuf,0,sizeof(RecvBuf));
	USART1_SendData("\x5A\x55\x06\x00\x0D\x03\x00\xc5\x6A\x69",10);
	ReaderTimedOut = 0; 																   //重置超时计数器
	while(ABORT_Succeed == 0)
	{
		if( ReaderTimedOut >= 4)                              //超过3s没有结果返回
		{
			USART1_SendData("\x5A\x55\x06\x00\x0D\x03\x00\xc5\x6A\x69",10);  //再次发送盘点指令
			ReaderTimedOut = 0; 								  					 //重置超时计数器
			TryTimer++;																			 //尝试次数+1
			if(TryTimer >=3)																 //超过3次尝试没有返回
			{
				memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
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
	//USART1_SendData("\x5A\x55\x08\x00\x0D\x11\x00\x00\x01\xd6\x6A\x69",12);  //再次发送盘点指令
}


/******************************************************************
	*名称: void StartAlarm(void)
	*输入：u8 TurnstileMode; u8 TryTimerNum;
	*输出：无
	*功能: 开始报警
	*说明：TurnstileMode: 0为非闸机模式，1为闸机模式；
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
	*名称: void TriggerSensors(void)
	*输入：无
	*输出：无
	*功能: 触发光幕，开始扫描
	*说明：任意光幕被触发，开始扫描
******************************************************************/
void TriggerSensors(void)
{
	u8 state,p_Counter,i;
		u8 FrameContent[50];     //
	state = Sensor_Scan(Scanmode);
	if(Scanmode <2)
	{
		if(clean_time>=4)     //2秒 没有触发另一个传感器，清空触发标志位
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
				SetTextValueInt32(0,3,OutNum);  //更新显示出馆人数
				SetTextValueInt32(2,3,OutNum);  //更新显示出馆人数
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
				SetTextValueInt32(0,2,InNum);  //更新进馆人数
				SetTextValueInt32(2,2,InNum);  //更新进馆人数
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
				SetTextValueInt32(0,2,InNum);  //更新显示出馆人数
				SetTextValueInt32(2,2,InNum);  //更新显示出馆人数
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
				SetTextValueInt32(0,3,OutNum);  //更新进馆人数
				SetTextValueInt32(2,3,OutNum);  //更新进馆人数
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
			KeepScan_time =0; //盘点时间清零
			if(touch == 0)
			{

				touch =1;
				StartScan(); /* 开始扫描*/
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
*名称: void DetectAlarm(u32 state)
*输入：扫描状态
*输出：无
*功能: 判断是否报警
*说明：
******************************************************************/
void DetectAlarm(u32 state, u8 mode)
{
	int i,j,k;
	u8  pContent =0;
	u16  CrcVal;
	u8 tempdata[30];
	SaveEPCFlag =0;
	if(state == 1)/* 扫描图书*/
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
							if(unEPCDat.sPackDat.ucSorting == 0xc2)  //远望谷模式
							{
								if((unEPCDat.sPackDat.ucItemIdentifier[7] == 0x05) || (unEPCDat.sPackDat.ucItemIdentifier[7] == 0x09))
								{
								//NetCmd = 1;
								StartAlarm();
								}
							}
							break;
						case XiaMen:
							if((unEPCDat.sPackDat.ucSorting == 0xc2) && (unEPCDat.sPackDat.ucItemIdentifier[7]==0x04))  //远望谷模式
							{
								//NetCmd = 1;
								StartAlarm();
							}
							break;
						case LOU_Phone:
							if((iStatus == 0) && (iVersion <4)) /* 标准0 的时候报警*/
							{
									UIDFlag = 1;
								  CompareRepeatAndSend(unEPCDat.ucStr);
							}
							break;
						case HaiNan_Phone:
							if(unEPCDat.sPackDat.ucSorting == 0xc2)  //远望谷模式
							{
								if((unEPCDat.sPackDat.ucItemIdentifier[7] == 0x05) || (unEPCDat.sPackDat.ucItemIdentifier[7] == 0x09))
								{
									UIDFlag = 1;
									InventoryEPC[pContent++] = 0x05;               //帧头
									InventoryEPC[pContent++] = (EPC_LEN + 5);      //长度
									InventoryEPC[pContent++] = 0xf1;               //发送EPC号命令
									for(i=0; i<EPC_LEN; i++)
									{
										InventoryEPC[pContent++] = unEPCDat.ucStr[i];    //内容
									}
									CrcVal = cal_crc16(InventoryEPC,EPC_LEN+3);    //Crc16 校验
									InventoryEPC[pContent++] = CrcVal>>8;
									InventoryEPC[pContent++] = CrcVal;
									InventoryEPC[pContent++] = 0x06;               //帧尾
									W5500_TCP_SEND(InventoryEPC,EPC_LEN+6);
								}
							}
							break;
						case XiaMen_Phone:
							if((TransBuf[24] &0x04)==0x04) //远望谷模式
							{
									UIDFlag = 1;
									InventoryEPC[pContent++] = 0x05;               //帧头
									InventoryEPC[pContent++] = (EPC_LEN + 5);      //长度
									InventoryEPC[pContent++] = 0xf1;               //发送EPC号命令
									for(i=0; i<EPC_LEN; i++)
									{
										InventoryEPC[pContent++] = unEPCDat.ucStr[i];    //内容
									}
									CrcVal = cal_crc16(InventoryEPC,EPC_LEN+3);    //Crc16 校验
									InventoryEPC[pContent++] = CrcVal>>8;
									InventoryEPC[pContent++] = CrcVal;
									InventoryEPC[pContent++] = 0x06;               //帧尾
									W5500_TCP_SEND(InventoryEPC,EPC_LEN+6);
							}
							break;
						case Custom:               //自定义，开发未完成。。。
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
				InventoryEPC[pContent++] = 0x05;               //帧头
				InventoryEPC[pContent++] = (12 +5);      //长度
				InventoryEPC[pContent++] = 0xf2;               //发送TID
				for(i=0; i<12; i++)
				{
					InventoryEPC[pContent++] = TransBuf[i+14];    //内容
				}
				CrcVal = cal_crc16(InventoryEPC,12+3);    //Crc16 校验
				InventoryEPC[pContent++] = CrcVal>>8;
				InventoryEPC[pContent++] = CrcVal;
				InventoryEPC[pContent++] = 0x06;               //帧尾
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
*名称: u8 Sensor_Scan(u8 scanmode)
*输入：无
*输出：无
*功能: 判断是否触发光幕
*说明：返回1，触发；返回0，未触发。
******************************************************************/
u8 Sensor_Scan(u8 scanmode)
{
	u8 cleanscreen[] = {0};
	if((INP_PIN1 == scanmode)||(INP_PIN2 == scanmode))
	{
		delay_ms(10);                                     //防抖动
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
	*名称: void Transform(unsigned char * buf, unsigned long datalen) 
	*输入:buf; datalen
	*输出:buf
	*功能: 转换 99 66  -> 99
	*说明:
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
	*名称: void TestANT(u8 ANTID) 
	*输入:ANTID
	*输出:天线状况
	*功能: 检测天线
	*说明:
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
							SetTextValue(7,2,AntNormal);         //一号正常
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,2,AntErr);           //异常
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}	
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //读写器异常
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
							SetTextValue(7,3,AntNormal);         //一号正常
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,3,AntErr);           //异常
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}	
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //读写器异常
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
							SetTextValue(7,4,AntNormal);         //一号正常
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,4,AntErr);           //异常
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}	
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //读写器异常
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
							SetTextValue(7,5,AntNormal);         //一号正常
							SoftReset();
							break;
						}
						else if(RecvBuf[1] == 0x01)
						{
							RecvSucceed = 0;
							SetTextValue(7,5,AntErr);           //异常
							SoftReset();
						}
					}
					RecvSucceed = 0;
				}
				if(TestAntTimeOut > 10)
				{
					SetTextValue(7,11,OutOfOrder);           //读写器异常
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x01\xdf\x6A\x69",12);          //开一号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x01\xdf\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x01\x01\xe0\x6A\x69",12);          //开二号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x01\x01\xe0\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x02\x01\xe1\x6A\x69",12);          //开三号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x02\x01\xe1\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x03\x01\xe2\x6A\x69",12);          //开四号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x03\x01\xe2\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x00\x00\xde\x6A\x69",12);          //关一号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x00\x00\xde\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x01\x00\xdf\x6A\x69",12);          //关二号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x01\x00\xdf\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x02\x00\xe0\x6A\x69",12);          //关三号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x02\x00\xe0\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
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
	USART1_SendData("\x5A\x55\x08\x00\x0D\x1a\x00\x03\x00\xe1\x6A\x69",12);          //开四号天线 
	ReaderTimedOut = 0; 																   //重置超时计数器
	if(RecvSucceed == 1)
	{
		RecvSucceed = 0;
	}
	else
	{
		while(RecvSucceed == 0)
		{
			if( ReaderTimedOut >=2)                              //超过1s没有结果返回
			{
				USART1_SendData("\\x5A\x55\x08\x00\x0D\x1a\x00\x03\x00\xe1\x6A\x69",12);  //再次发送指令
				ReaderTimedOut = 0; 								  					 //重置超时计数器
				TryTimer++;																			 //尝试次数+1
				if(TryTimer >=3)																 //超过3次尝试没有返回
					{
						memset(RecvBuf,0,sizeof(RecvBuf)); 		 //清空缓存
						SetTextValue(7,11,OutOfOrder);          // 检测界面出现读写器故障
						break;
					}
			}
		 }
			RecvSucceed = 0;
	}
}

/****************************************************************************************************

函数原型:    	int CompareRepeatAndSend(uint8_t *EPCAndTID)
功能描述:			比较数据，相同数据就不存入，不同存入						
输入参数:			u8 *TempContent
输出参数:      无
返回值:        无
****************************************************************************************************/
int CompareRepeatAndSend(u8 *EPCAndTID)
{
	u8 i,j,m,RWFlag;
	u8 tempdata[12];
	RWFlag = 0;
	for(i=0;i<CompareEPC_T.s_EPCCounter;i++)                                					 //遍历所有的标签
	{
	  if(memcmp(&EPCAndTID[0],&CompareEPC_T.s_EPC[i][0],12) == 0x00)                   //如果EPC相同
		{
			RWFlag = 1;                                                                    //做标记
		  return 0xFF;                                                                   //退出函数
		}
	}
	if(RWFlag == 0x00)                                                                 //如果没有重复EPC
	{
		memcpy(&CompareEPC_T.s_EPC[CompareEPC_T.s_EPCCounter++][0],&EPCAndTID[0],12);    //存入EPC到总列表
		memcpy(tempdata,&EPCAndTID[0],12);
		MakeTCPFrame(18,0xf1,tempdata);                                                  //发送新的EPC
		return 0x01;
	}	
}


/****************************************************************************************************

函数原型:    	u8 check_sum(u8 *buf, int len)
功能描述:			校验和					
输入参数:			u8 *buf
输出参数:      无
返回值:        返回0，则说明接收的数据是正确的，
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



