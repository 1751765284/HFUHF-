#ifndef	_Screenmain_H_
#define	_Screenmain_H_


#include "types.h"
#include "sys.h"

//写入数据寄存器
extern u8 InNumtempW[10];             //进馆人数
extern u8 OutNumtempW[10];            //出馆人数
extern u8 PowertempW[10];                //读写器功率
extern u8 VolumetempW[10];               //音量大小
extern u8 Door_Modetemp[2];             //报警模式
extern u8 Door_ModetempW[2];                    //模式
extern u8 Scanmodetemp[2];             //扫描模式
extern u8 sInvertNumtemp[2];           //进出反转模式

extern u8 InNumtemp[10];             //进馆人数
extern u8 OutNumtemp[10];            //出馆人数
extern u8 Powertemp[10];                //读写器功率
extern u8 Volumetemp[10];               //音量大小

extern int32 InNum ;
extern int32 OutNum ;


extern u8 IpAddTemp1[10];            //ip地址1
extern u8 SubnetMaskTemp1[10];       //子网掩码1
extern u8 GateWayTemp1[10];          //网关1
extern u8 ServerIpTemp1[10];         //服务器IP
extern int ServerPort ;
extern u8 ServerPortTemp[10];        //服务器端口
extern u8 MacAddTemp1[10];
extern int DoorPort;          //门的端口
extern u8 DoorPortTemp[10];        //服务器端口
extern u8 SetPasswordTemp[10];     //设置密码
extern u8 PasswordLen[1];             //密码长度




extern volatile u32  timer_tick_count;
extern uint8 cmd_buffer[];//指令缓存
extern u8 pass ;        //密码正确标志
extern u8 Door_Mode;
extern u8 phone;        //手机借还模式

#define LOU        	    0x00				//高校联盟模式 
#define HaiNan	        0x01				//海南模式 24 -5 (远望谷)
#define XiaMen	        0x02				//厦门模式 24 -4（远望谷）
#define LOU_Phone      	0x03				//高校联盟手机借还模式
#define HaiNan_Phone    0x04				//海南手机借还模式
#define XiaMen_Phone   	0x05				//厦门手机借还模式
#define Custom          0x06        //自定义设置


#define Poweradd       			 	0
#define Volumeadd    					10
#define InNumadd    	  			20
#define OutNumadd    					30
#define Door_Modeadd 					40
#define Ipaddadd1       			50
#define SubnetMaskadd1  			60
#define GateWayadd1     			70
#define ServerIPadd1    			80
#define ServerPortadd   			90
#define MacAddadd1      			100
#define sInvertNum_Modeadd 		110
#define ScanModeadd     			120
#define DoorPortadd           130
#define SetPasswordadd        140
#define PasswordLenadd        150


#define TIME_100MS   		10  //100毫秒(10个单位)


void Screenmain(void);
void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value);
void SystemReset(void);  //重置参数
void UpdateUI(void);
void SoftReset( void );  //软件重启
void ModeReset (u8 mode);
unsigned char CharToHex(unsigned char bHex);
unsigned char HexToChar(unsigned char bChar);
#endif


