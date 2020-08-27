#ifndef	_Screenmain_H_
#define	_Screenmain_H_


#include "types.h"
#include "sys.h"

//д�����ݼĴ���
extern u8 InNumtempW[10];             //��������
extern u8 OutNumtempW[10];            //��������
extern u8 PowertempW[10];                //��д������
extern u8 VolumetempW[10];               //������С
extern u8 Door_Modetemp[2];             //����ģʽ
extern u8 Door_ModetempW[2];                    //ģʽ
extern u8 Scanmodetemp[2];             //ɨ��ģʽ
extern u8 sInvertNumtemp[2];           //������תģʽ

extern u8 InNumtemp[10];             //��������
extern u8 OutNumtemp[10];            //��������
extern u8 Powertemp[10];                //��д������
extern u8 Volumetemp[10];               //������С

extern int32 InNum ;
extern int32 OutNum ;


extern u8 IpAddTemp1[10];            //ip��ַ1
extern u8 SubnetMaskTemp1[10];       //��������1
extern u8 GateWayTemp1[10];          //����1
extern u8 ServerIpTemp1[10];         //������IP
extern int ServerPort ;
extern u8 ServerPortTemp[10];        //�������˿�
extern u8 MacAddTemp1[10];
extern int DoorPort;          //�ŵĶ˿�
extern u8 DoorPortTemp[10];        //�������˿�
extern u8 SetPasswordTemp[10];     //��������
extern u8 PasswordLen[1];             //���볤��




extern volatile u32  timer_tick_count;
extern uint8 cmd_buffer[];//ָ���
extern u8 pass ;        //������ȷ��־
extern u8 Door_Mode;
extern u8 phone;        //�ֻ��軹ģʽ

#define LOU        	    0x00				//��У����ģʽ 
#define HaiNan	        0x01				//����ģʽ 24 -5 (Զ����)
#define XiaMen	        0x02				//����ģʽ 24 -4��Զ���ȣ�
#define LOU_Phone      	0x03				//��У�����ֻ��軹ģʽ
#define HaiNan_Phone    0x04				//�����ֻ��軹ģʽ
#define XiaMen_Phone   	0x05				//�����ֻ��軹ģʽ
#define Custom          0x06        //�Զ�������


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


#define TIME_100MS   		10  //100����(10����λ)


void Screenmain(void);
void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value);
void SystemReset(void);  //���ò���
void UpdateUI(void);
void SoftReset( void );  //�������
void ModeReset (u8 mode);
unsigned char CharToHex(unsigned char bHex);
unsigned char HexToChar(unsigned char bChar);
#endif


