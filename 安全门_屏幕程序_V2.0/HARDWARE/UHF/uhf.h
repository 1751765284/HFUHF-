#ifndef __UHF_H_
#define __UHF_H_
#include "stm32f10x_conf.h"
#include "sys.h"  

typedef struct      			    //过滤重复数据的缓存结构体
{		
		int     s_EPCCounter;			//EPC
		uint8_t s_EPC[100][12];   //EPC&TID缓存数组
		
} CompareEPC_t;


extern  vu8   RecvSucceed;   //volatile  unsigned char
extern  vu8   RecvBuf[100];
extern  vu8   MidBuf[100];
extern  u32   adder ;       //unsigned int
extern  u32   UHFLen;  
extern  u8    UHF[];
extern  u8    Start ;
extern  u8    NetCmd ;
extern  u8    SendEPC ;
extern unsigned   int					 ReaderTimedOut;      //读写器超时标志
extern    int          waiting_time1 ;   //判断进出时间
extern    int          waiting_time2 ;   //判断进出时间
extern    int          clean_time;       //清除超时进出标志位
extern    int          save_time;        //存储进出人数
extern    u8           touch ;   //触发光幕标志
extern    u8           NETSEND;  //epc发送网络返回标志
extern    int    TurnstileTimeOut; //闸机持续时间
extern    u8   sInvertNum;            //反转人数
extern    u8   Scanmode;              //1 ：高电平有效； 0：低电平有效
extern u8 PlusNum; //缓存书籍数量
extern u8   TotalEPC[100][50];
extern u8   BookCounter ;
extern u32 EPC_LEN;
extern u8  tempEPC[901];
extern int  scantime ;
extern u8   StartAlarmFlag; //报警标示
extern CompareEPC_t CompareEPC_T;                             //声明比较数据结构体变量
extern u8   GateLinkage; //闸机联动





/* 函数声明 */
extern void AntSetC();
extern void ScanSetC();
void UHFInit(u32 POWER);
void StartScan(void);
void StopScan(void);
void StartAlarm(void);
void TriggerSensors(void);
void DetectAlarm(u32 state, u8 mode);
u8 Sensor_Scan(u8 scanmode);
void Counter_people(void);
void Transform(u8 * buf, unsigned long datalen); 
void OpenANT1 (void);
void OpenANT2 (void);
void OpenANT3 (void);
void OpenANT4 (void);
void StopANT1 (void);
void StopANT2 (void);
void StopANT3 (void);
void StopANT4 (void);
void TestANT(u8 ANTID);
u8 CompareData(u8 *TempContent);
void TagRead(void);
u8 check_sum(u8 *buf, int len);
int CompareRepeatAndSend(u8 *EPCAndTID);
#endif


