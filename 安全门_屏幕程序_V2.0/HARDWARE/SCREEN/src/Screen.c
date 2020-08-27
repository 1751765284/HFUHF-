#include "Screen.h"
#include "hmi_driver.h"
#include "cmd_queue.h"
#include "cmd_process.h"
#include "stdio.h"
#include "hw_config.h"
#include "delay.h"
#include <stdint.h>
#include "w25qxx.h"
#include "uhf.h"
#include "beep.h"
#include "string.h"



extern volatile uint32_t timer_tick_count;

uint8 cmd_buffer[CMD_MAX_SIZE];//ָ���

static uint16 current_screen_id = 0;//��ǰ����ID

//static int32 test_value = 0;//����ֵ


int32 value = 0;
u8 pass = 0;        //������ȷ��־
/////////flash���//////////////////////////////////////
int32 InNum =0;
int32 OutNum =0;
u32 PowerValue;
int32 VolumeValue=0;
u8 phone = 0;

u8 InNumtemp[10];             //��������
u8 OutNumtemp[10];            //��������
u8 Powertemp[10];                //��д������
u8 Volumetemp[10];               //������С
u8 Door_Modetemp[2];             //����ģʽ
u8 sInvertNumtemp[2];           //������תģʽ
u8 Scanmodetemp[2];             //ɨ��ģʽ


u8 InNumtempW[10];             //��������
u8 OutNumtempW[10];            //��������
u8 PowertempW[10];                //��д������
u8 VolumetempW[10];               //������С
u8 Door_ModetempW[2];                    //ģʽ



u8 IpAddTemp1[10];            //ip��ַ1
u8 SubnetMaskTemp1[10];       //��������1
u8 GateWayTemp1[10];          //����1
u8 ServerIpTemp1[10];         //������IP
int ServerPort = 20000;
u8 ServerPortTemp[10];        //�������˿�
u8 MacAddTemp1[10];
int DoorPort = 6000;          //�ŵĶ˿�
u8 DoorPortTemp[10];        //�Ŷ˿�
u8 SetPasswordTemp[10];
u8 SetPassword[4] ={0x72,0x66,0x69,0x64};
u8 PasswordLen[1] = {4};      //���볤��


/***************----- ����ģʽ -----***************/
u8 Door_Mode =0;				        //����ģʽ,Ĭ��Ϊ��У����ģʽ


/*!
 *  \brief  ��Ϣ��������
 *  \param msg ��������Ϣ
 *  \param size ��Ϣ����
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//ָ������
	uint8 ctrl_msg = msg->ctrl_msg;   //��Ϣ������
	uint8 control_type = msg->control_type;//�ؼ�����
	uint16 screen_id = PTR2U16(&msg->screen_id);//����ID
	uint16 control_id = PTR2U16(&msg->control_id);//�ؼ�ID
	uint32 value = PTR2U32(msg->param);//��ֵ

	switch(cmd_type)
	{
	case NOTIFY_TOUCH_PRESS://����������
	case NOTIFY_TOUCH_RELEASE://�������ɿ�
		NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;
	case NOTIFY_WRITE_FLASH_OK://дFLASH�ɹ�
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://дFLASHʧ��
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://��ȡFLASH�ɹ�
		NotifyReadFlash(1,cmd_buffer+2,size-6);//ȥ��֡ͷ֡β
		break;
	case NOTIFY_READ_FLASH_FAILD://��ȡFLASHʧ��
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://��ȡRTCʱ��
		NotifyReadRTC(cmd_buffer[1],cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
	{
		if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//����ID�仯֪ͨ
		{
			NotifyScreen(screen_id);
		}
		else
		{
			switch(control_type)
			{
			case kCtrlButton: //��ť�ؼ�
				NotifyButton(screen_id,control_id,msg->param[1]);
				break;
			case kCtrlText://�ı��ؼ�
				NotifyText(screen_id,control_id,msg->param);
				break;
			case kCtrlProgress: //�������ؼ�
				NotifyProgress(screen_id,control_id,value);
				break;
			case kCtrlSlider: //�������ؼ�
				NotifySlider(screen_id,control_id,value);
				break;
			case kCtrlMeter: //�Ǳ�ؼ�
				NotifyMeter(screen_id,control_id,value);
				break;
			case kCtrlMenu://�˵��ؼ�
				NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
				break;
			case kCtrlSelector://ѡ��ؼ�
				NotifySelector(screen_id,control_id,msg->param[0]);
				break;
			case kCtrlRTC://����ʱ�ؼ�
				NotifyTimer(screen_id,control_id);
				break;
			default:
				break;
			}
		}
	}
	break;
	default:
		break;
	}
}

/*!
 *  \brief  �����л�֪ͨ
 *  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
 *  \param screen_id ��ǰ����ID
 */
void NotifyScreen(uint16 screen_id)
{
	//TODO: ����û�����
	current_screen_id = screen_id;//�ڹ��������п��������л�֪ͨ����¼��ǰ����ID

	if(current_screen_id==40)//�¶�����
	{
		uint16 i = 0;
		uint8 dat[100] = {0};

		//���ɷ���
		for (i=0; i<100; ++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//������ݵ�ͨ��0

		//���ɾ�ݲ�
		for (i=0; i<100; ++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//������ݵ�ͨ��1
	}
	else if(current_screen_id==9)//��ά��
	{
		//��ά��ؼ���ʾ�����ַ�ʱ����Ҫת��ΪUTF8���룬
		//ͨ����ָ�����֡���ת�������ݴ��123�� ���õ��ַ�����������
		uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*!
 *  \brief  ���������¼���Ӧ
 *  \param press 1���´�������3�ɿ�������
 *  \param x x����
 *  \param y y����
 */
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{
	//TODO: ����û�����
}

void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	u8 buffer[12] = {0};
	sprintf(buffer,"%ld",value); //������ת��Ϊ�ַ���
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(uint16 screen_id, uint16 control_id,float value)
{
	u8 buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//�Ѹ�����ת��Ϊ�ַ���(����һλС��)
	SetTextValue(screen_id,control_id,buffer);
}

void UpdateUI()
{
	if(current_screen_id==0)//�ı����ú���ʾ
	{
		;
	}
}

/*!
 *  \brief  ��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	u8 cleanscreen[] = {0};

	//TODO: ����û�����
	if(screen_id == 1)          //���ϼ���л�����
	{
		if(control_id == 7)
		{
			SetTextValue(7,2,cleanscreen);
			SetTextValue(7,3,cleanscreen);
			SetTextValue(7,4,cleanscreen);
			SetTextValue(7,5,cleanscreen);
			SetTextValue(7,11,cleanscreen);
			SetButtonValue(7,1,0);
			SetButtonValue(7,8,0);
			SetButtonValue(7,9,0);
			SetButtonValue(7,10,0);
			SetScreen(7);
		}
	}
  if(screen_id==2)//��ť��ͼ�ꡢ��������
	{
		if(control_id==4)//���мӰ�ť
		{
			InNum += 1;
			OutNum += 1;
			SetTextValueInt32(2,2,InNum);  //���½�������
			SetTextValueInt32(2,3,OutNum);  //���½�������
			SetTextValueInt32(0,2,InNum);  //������ʾ��������
			SetTextValueInt32(0,3,OutNum);  //������ʾ��������
			sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
		}
		else if(control_id==5)//���м���ť
		{
			InNum -= 1;
			OutNum -= 1;
			SetTextValueInt32(2,2,InNum);  //���½�������
			SetTextValueInt32(2,3,OutNum);  //���½�������
			SetTextValueInt32(0,2,InNum);  //������ʾ��������
			SetTextValueInt32(0,3,OutNum);  //������ʾ��������
			sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
		}
		else if(control_id==6)//��λ��ť
		{
			value = InNum = OutNum =0;
			SetTextValueInt32(2,2,value);  //���½�������
			SetTextValueInt32(2,3,value);  //���½�������
			SetTextValueInt32(0,2,value);  //������ʾ��������
			SetTextValueInt32(0,3,value);  //������ʾ��������
			sprintf(InNumtempW,"%ld",value); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			sprintf(OutNumtempW,"%ld",value); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));

		}
		else if(control_id==7)       //��ת��������
		{
			if(sInvertNum == 1)
			{
				sInvertNum = 2;
			}
			else if(sInvertNum == 2)
			{
				sInvertNum = 1;
			}
			sprintf(sInvertNumtemp,"%ld",sInvertNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)sInvertNum,sInvertNum_Modeadd,sizeof(sInvertNum));
		}
	}

	else if(screen_id==5)//��ť��ͼ�ꡢ��������
	{
		if(control_id==2 && pass == 1)//���а�ť
			//if(control_id==2 )//���а�ť
		{

			pass =0;
			SetScreen(1);

		}
	}
	else if(screen_id == 6)   //Mode
	{
		if(control_id==5 )
		{
			if( phone == 0 )
			{
				if(Door_Mode == LOU)
				{
					phone = 1;
					Door_Mode = LOU_Phone;
				}
				else if(Door_Mode == HaiNan)
				{
					phone = 1;
					Door_Mode = HaiNan_Phone;
				}
				else if(Door_Mode == XiaMen)
				{
					phone = 1;
					Door_Mode = XiaMen_Phone;
				}
			}
			else if( phone == 1)
			{
				if(Door_Mode == LOU_Phone)
				{
					phone = 0;
					Door_Mode = LOU;
				}
				else if(Door_Mode == HaiNan_Phone)
				{
					phone = 0;
					Door_Mode = HaiNan;
				}
				else if(Door_Mode == XiaMen_Phone)
				{
					phone = 0;
					Door_Mode = XiaMen;
				}
			}

		}
		if(control_id==1 && phone ==0)
		{
			Door_Mode = LOU;
		}
		else if(control_id==2 && phone ==0)
		{
			Door_Mode = HaiNan;
		}
		else if(control_id==3 && phone ==0)
		{
			Door_Mode = XiaMen;
		}
		if(control_id==1 && phone ==1)
		{
			Door_Mode = LOU_Phone;
		}
		else if(control_id==2 && phone ==1)
		{
			Door_Mode = HaiNan_Phone;
		}
		else if(control_id==3 && phone ==1)
		{
			Door_Mode = XiaMen_Phone;
		}
		//sprintf(Door_Modetemp,"%ld",Door_Mode); //������ת��Ϊ�ַ���
		Door_Modetemp[0] = Door_Mode;
		W25QXX_Write((u8*)Door_Modetemp,Door_Modeadd,sizeof(Door_Modetemp));
		SoftReset();
	}
	else if(screen_id == 7)   //��������
	{
		if(control_id== 1)
		{
			TestANT(1);			
		}
		if(control_id== 8)
		{
			TestANT(2);			
		}
		if(control_id== 9)
		{
			TestANT(3);			
		}
		if(control_id== 10)
		{
			TestANT(4);			
		}
	}
	else if(screen_id == 8)   // ����ip����
	{
		if(control_id== 18)
		{
			SoftReset();          //����			
		}
	}			
}

/*!
 *  \brief  �ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
		u8 abc;
	//TODO: ����û�����
//	int32 value = 0;
	if(screen_id==2)//����ID2���ı����ú���ʾ
	{
		sscanf(str,"%ld",&value);//���ַ���ת��Ϊ����

		if(control_id==2)//����������ť����
		{
			//�޶���ֵ��Χ��Ҳ�������ı��ؼ����������ã�
			if(value<0)
				value = 0;
			else if(value>9999999)
				value = 9999999;
			InNum = value;
			sprintf(InNumtempW,"%ld",InNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			SetTextValueInt32(2,2,InNum);  //���½�������
			SetTextValueInt32(0,2,InNum);  //������ʾ��������
		}
		if(control_id==3)//����������ť����
		{
			//�޶���ֵ��Χ��Ҳ�������ı��ؼ����������ã�
			if(value<0)
				value = 0;
			else if(value>9999999)
				value = 9999999;
			OutNum = value;
			sprintf(OutNumtempW,"%ld",OutNum); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
			SetTextValueInt32(2,3,OutNum);  //���³�������
			SetTextValueInt32(0,3,OutNum);  //������ʾ��������
		}
	}
	if(screen_id==3)//����ID3���ı����ú���ʾ
	{
		sscanf(str,"%ld",&value);//���ַ���ת��Ϊ����
		if(control_id==4)            //���ù���
		{
			if(value<10)
				value = 10;
			else if(value>30)
				value = 30;
			PowertempW[0] = value;
			SetMeterValue(3,2,PowertempW[0]);         //�����Ǳ�
			SetSliderValue(3,3,PowertempW[0]);        //���½�������
			//sprintf(PowertempW,"%ld",PowerValue); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)PowertempW,Poweradd,sizeof(PowertempW));
			SoftReset();
		}
	}

	if(screen_id==5)//����ID5����������
	{
		if(control_id == 4)
		{
			if(memcmp(str,SetPasswordTemp,PasswordLen[0]) == 0)
			{
				pass = 1;
			}
		}
	}
		if(screen_id==8)                                               //����ID8 �� �������ý���
	{
		sscanf(str,"%ld",&value);//���ַ���ת��Ϊ����
		if(control_id==1)            //����ip1
		{
			IpAddTemp1[0] = value;
      SetTextValueInt32(8,1,IpAddTemp1[0]);  //����IP1
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==2)            //����ip2
		{
			IpAddTemp1[1] = value;
			SetTextValueInt32(8,2,IpAddTemp1[1]);  //����IP2	
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==3)            //����ip3
		{
			IpAddTemp1[2] = value;
			SetTextValueInt32(8,3,IpAddTemp1[2]);  //����IP2		
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==4)            //����ip4
		{
			IpAddTemp1[3] = value;
			SetTextValueInt32(8,4,IpAddTemp1[3]);  //����IP4
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==5)            //������������1
		{
			SubnetMaskTemp1[0] = value;
			SetTextValueInt32(8,5,SubnetMaskTemp1[0]);  //������������1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==6)            //������������2
		{
			SubnetMaskTemp1[1] = value;
			SetTextValueInt32(8,6,SubnetMaskTemp1[1]);  //������������1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==7)            //������������3
		{
			SubnetMaskTemp1[2] = value;
			SetTextValueInt32(8,7,SubnetMaskTemp1[2]);  //������������1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==8)            //������������4
		{
			SubnetMaskTemp1[3] = value;
			SetTextValueInt32(8,8,SubnetMaskTemp1[3]);  //������������1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==9)            //����Ĭ������1
		{
			GateWayTemp1[0] = value;
			SetTextValueInt32(8,9,GateWayTemp1[0]);  //������������
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==10)            //����Ĭ������2
		{
			GateWayTemp1[1] = value;
			SetTextValueInt32(8,10,GateWayTemp1[1]);  //������������
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==11)            //����Ĭ������3
		{
			GateWayTemp1[2] = value;
			SetTextValueInt32(8,11,GateWayTemp1[2]);  //������������
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==12)            //����Ĭ������4
		{
			GateWayTemp1[3] = value;
			SetTextValueInt32(8,12,GateWayTemp1[3]);  //������������
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==13)            //���÷�����ip1
		{
			ServerIpTemp1[0] = value;
			SetTextValueInt32(8,13,ServerIpTemp1[0]);  //���·�����IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==14)            //���÷�����ip2
		{
			ServerIpTemp1[1] = value;
			SetTextValueInt32(8,14,ServerIpTemp1[1]);  //���·�����IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==15)            //���÷�����ip3
		{
			ServerIpTemp1[2] = value;
			SetTextValueInt32(8,15,ServerIpTemp1[2]);  //���·�����IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==16)            //���÷�����ip4
		{
			ServerIpTemp1[3] = value;
			SetTextValueInt32(8,16,ServerIpTemp1[3]);  //���·�����IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==17)                                                                 //���÷������˿�
		{
			ServerPort = value;
			SetTextValueInt32(8,17,ServerPort);  //���·������˿�
			sprintf(ServerPortTemp,"%ld",ServerPort); //������ת��Ϊ�ַ���
			W25QXX_Write((u8*)ServerPortTemp,ServerPortadd,sizeof(ServerPortTemp));
		}
		if(control_id==23)            //����MAC��ַ5
		{
			MacAddTemp1[0] = value;
			SetTextValueInt32(8,23,MacAddTemp1[0]);  //����MAC��ַ
			W25QXX_Write((u8*)MacAddTemp1,MacAddadd1,10);
		}
		if(control_id==24)            //����MAC��ַ6
		{
			MacAddTemp1[1] = value;
			SetTextValueInt32(8,24,MacAddTemp1[1]);  //����MAC��ַ
			W25QXX_Write((u8*)MacAddTemp1,MacAddadd1,10);
		}
	}
}

/*!
 *  \brief  �������ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: ����û�����
}

/*!
 *  \brief  �������ؼ�֪ͨ
 *  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: ����û�����
	if(screen_id==3&&control_id==3)//�������
	{
		PowertempW[0] = value;
		SetMeterValue(3,2,PowertempW[0]);                          //���ڹ���
		SetTextValueInt32(3,4,PowertempW[0]);  
		//sprintf(PowertempW,"%ld", PowertempW[0]); //������ת��Ϊ�ַ���
		W25QXX_Write((u8*)PowertempW,Poweradd,sizeof(PowertempW));
		SoftReset();
	}
	if(screen_id==4&&control_id==3)//�������
	{
		VolumetempW[0] = value;
		SetMeterValue(4,2,value); //���½�������ֵ
		AdjustVolume(value);                               //��������
		OpenBEEP(Beeppwmval);
		delay_ms(500);
		StopBEEP;
		delay_ms(100);
		OpenBEEP(Beeppwmval);
		delay_ms(500);
		StopBEEP;
		//sprintf(VolumetempW,"%ld",value); //������ת��Ϊ�ַ���
		W25QXX_Write((u8*)VolumetempW,Volumeadd,sizeof(VolumetempW));
	}
}

/*!
 *  \brief  �Ǳ�ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: ����û�����
}

/*!
 *  \brief  �˵��ؼ�֪ͨ
 *  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item �˵�������
 *  \param state ��ť״̬��0�ɿ���1����
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
	//TODO: ����û�����
}

/*!
 *  \brief  ѡ��ؼ�֪ͨ
 *  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item ��ǰѡ��
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
	//TODO: ����û�����
}

/*!
 *  \brief  ��ʱ����ʱ֪ͨ����
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
	//TODO: ����û�����
}

/*!
 *  \brief  ��ȡ�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 *  \param _data ��������
 *  \param length ���ݳ���
 */
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
	//TODO: ����û�����
}

/*!
 *  \brief  д�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 */
void NotifyWriteFlash(uint8 status)
{
	//TODO: ����û�����
}

/*!
 *  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
 *  \param year �꣨BCD��
 *  \param month �£�BCD��
 *  \param week ���ڣ�BCD��
 *  \param day �գ�BCD��
 *  \param hour ʱ��BCD��
 *  \param minute �֣�BCD��
 *  \param second �루BCD��
 */
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second)
{
}


//�豸����
void SystemReset(void)
{
	W25QXX_Read(Powertemp,Poweradd,sizeof(Powertemp));   //���ù���
	if((Powertemp[0] == 0)||(Powertemp[0] == 0xff))
	{
		Powertemp[0] = 16;
	}
	SetSliderValue(3,3,Powertemp[0]);
	SetMeterValue(3,2,Powertemp[0]); //���½�������ֵ
	SetTextValueInt32(3,4,Powertemp[0]);  //���¹���
	UHFInit(Powertemp[0]*10);

	W25QXX_Read(InNumtemp,InNumadd,sizeof(InNumtemp));   //���ý�������
	sscanf(InNumtemp,"%ld",&InNum);             //���ַ���ת��Ϊ����
	SetTextValueInt32(0,2,InNum);               //������ʾ��������
	SetTextValueInt32(2,2,InNum);               //������ʾ��������

	W25QXX_Read(OutNumtemp,OutNumadd,sizeof(OutNumtemp));   //���ó�������
	sscanf(OutNumtemp,"%ld",&OutNum);             //���ַ���ת��Ϊ����
	SetTextValueInt32(0,3,OutNum);               //������ʾ��������
	SetTextValueInt32(2,3,OutNum);               //������ʾ��������

	W25QXX_Read(Volumetemp,Volumeadd,sizeof(Volumetemp));   //��������
	if(Volumetemp[0] == 0xff)
	{
		Volumetemp[0] = 50;
	}
	SetSliderValue(4,3,Volumetemp[0]);
	SetMeterValue(4,2,Volumetemp[0]); //���½�������ֵ
//	AdjustVolume(Volumetemp[0]);

	W25QXX_Read(Door_Modetemp,Door_Modeadd,sizeof(Door_Modetemp));   //����ģʽ
	if(Door_Modetemp[0] == 0xff)
	{
		Door_Mode = Door_Modetemp[0] = 0;
	}
	else Door_Mode = Door_Modetemp[0];
	ModeReset(Door_Mode);
	
	W25QXX_Read(sInvertNumtemp,sInvertNum_Modeadd,sizeof(sInvertNumtemp));    //���÷�תģʽ
	sscanf(sInvertNumtemp,"%ld",&sInvertNum); //���ַ���ת��Ϊ����
	if(sInvertNum == 0xff)
	{
		sInvertNum = 1;
	}
	
	W25QXX_Read(SetPasswordTemp,SetPasswordadd,sizeof(SetPasswordTemp));    //��������
	if((SetPasswordTemp[0] == 0xff)|| (SetPasswordTemp[0] == 0x00))
	{
		memcpy(SetPasswordTemp,SetPassword,sizeof(SetPassword));
	}
	W25QXX_Read(PasswordLen,PasswordLenadd,sizeof(PasswordLen));    //�������볤��
	


}
/****************************************************************************************************

����ԭ��:    	void SoftReset( void )
��������:	    ��λ

�������:			��


�������:      ��
����ֵ:        ��
****************************************************************************************************/
void SoftReset( void )
{
#if DEBUG
	printf("������λָ��\r\n\r\n");
#endif
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}



/****************************************************************************************************

����ԭ��:    	void ModeReset (u8 mode)
��������:	    mode ��λ

�������:			ģʽ


�������:      ��
����ֵ:        ��
****************************************************************************************************/
void ModeReset (u8 mode)
{
	switch(mode)
	{
	case LOU:
		SetButtonValue(6,1,1);
		SetButtonValue(6,5,0);
		phone =0;
		break;
	case HaiNan:
		SetButtonValue(6,2,1);
		SetButtonValue(6,5,0);
		phone =0;
		break;
	case XiaMen:
		SetButtonValue(6,3,1);
		SetButtonValue(6,5,0);
		phone =0;
		break;
	case LOU_Phone:
		SetButtonValue(6,1,1);
		SetButtonValue(6,5,1);
		phone =1;
		break;
	case HaiNan_Phone:
		SetButtonValue(6,2,1);
		SetButtonValue(6,5,1);
		phone = 1;
		break;
	case XiaMen_Phone:
		SetButtonValue(6,3,1);
		SetButtonValue(6,5,1);
		phone =1;
		break;
	default:
		break;
	}

}

unsigned char CharToHex(unsigned char bHex)
{
if((bHex>=0)&&(bHex<=9))
bHex += 0x30;
else if((bHex>=10)&&(bHex<=15))
bHex += 0x37;
else bHex = 0xff;
return bHex;
}

unsigned char HexToChar(unsigned char bChar)
{
if((bChar>=0x30)&&(bChar<=0x39))
bChar -= 0x30;
else if((bChar>=0x41)&&(bChar<=0x46))
bChar -= 0x37;
else if((bChar>=0x61)&&(bChar<=0x66))
bChar -= 0x57;
else bChar = 0xff;
return bChar;
}






