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

uint8 cmd_buffer[CMD_MAX_SIZE];//指令缓存

static uint16 current_screen_id = 0;//当前画面ID

//static int32 test_value = 0;//测试值


int32 value = 0;
u8 pass = 0;        //密码正确标志
/////////flash添加//////////////////////////////////////
int32 InNum =0;
int32 OutNum =0;
u32 PowerValue;
int32 VolumeValue=0;
u8 phone = 0;

u8 InNumtemp[10];             //进馆人数
u8 OutNumtemp[10];            //出馆人数
u8 Powertemp[10];                //读写器功率
u8 Volumetemp[10];               //音量大小
u8 Door_Modetemp[2];             //报警模式
u8 sInvertNumtemp[2];           //进出反转模式
u8 Scanmodetemp[2];             //扫描模式


u8 InNumtempW[10];             //进馆人数
u8 OutNumtempW[10];            //出馆人数
u8 PowertempW[10];                //读写器功率
u8 VolumetempW[10];               //音量大小
u8 Door_ModetempW[2];                    //模式



u8 IpAddTemp1[10];            //ip地址1
u8 SubnetMaskTemp1[10];       //子网掩码1
u8 GateWayTemp1[10];          //网关1
u8 ServerIpTemp1[10];         //服务器IP
int ServerPort = 20000;
u8 ServerPortTemp[10];        //服务器端口
u8 MacAddTemp1[10];
int DoorPort = 6000;          //门的端口
u8 DoorPortTemp[10];        //门端口
u8 SetPasswordTemp[10];
u8 SetPassword[4] ={0x72,0x66,0x69,0x64};
u8 PasswordLen[1] = {4};      //密码长度


/***************----- 运行模式 -----***************/
u8 Door_Mode =0;				        //运行模式,默认为奥校联盟模式


/*!
 *  \brief  消息处理流程
 *  \param msg 待处理消息
 *  \param size 消息长度
 */
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
	uint8 cmd_type = msg->cmd_type;//指令类型
	uint8 ctrl_msg = msg->ctrl_msg;   //消息的类型
	uint8 control_type = msg->control_type;//控件类型
	uint16 screen_id = PTR2U16(&msg->screen_id);//画面ID
	uint16 control_id = PTR2U16(&msg->control_id);//控件ID
	uint32 value = PTR2U32(msg->param);//数值

	switch(cmd_type)
	{
	case NOTIFY_TOUCH_PRESS://触摸屏按下
	case NOTIFY_TOUCH_RELEASE://触摸屏松开
		NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4));
		break;
	case NOTIFY_WRITE_FLASH_OK://写FLASH成功
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://写FLASH失败
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://读取FLASH成功
		NotifyReadFlash(1,cmd_buffer+2,size-6);//去除帧头帧尾
		break;
	case NOTIFY_READ_FLASH_FAILD://读取FLASH失败
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://读取RTC时间
		NotifyReadRTC(cmd_buffer[1],cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
	{
		if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
		{
			NotifyScreen(screen_id);
		}
		else
		{
			switch(control_type)
			{
			case kCtrlButton: //按钮控件
				NotifyButton(screen_id,control_id,msg->param[1]);
				break;
			case kCtrlText://文本控件
				NotifyText(screen_id,control_id,msg->param);
				break;
			case kCtrlProgress: //进度条控件
				NotifyProgress(screen_id,control_id,value);
				break;
			case kCtrlSlider: //滑动条控件
				NotifySlider(screen_id,control_id,value);
				break;
			case kCtrlMeter: //仪表控件
				NotifyMeter(screen_id,control_id,value);
				break;
			case kCtrlMenu://菜单控件
				NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
				break;
			case kCtrlSelector://选择控件
				NotifySelector(screen_id,control_id,msg->param[0]);
				break;
			case kCtrlRTC://倒计时控件
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
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(uint16 screen_id)
{
	//TODO: 添加用户代码
	current_screen_id = screen_id;//在工程配置中开启画面切换通知，记录当前画面ID

	if(current_screen_id==40)//温度曲线
	{
		uint16 i = 0;
		uint8 dat[100] = {0};

		//生成方波
		for (i=0; i<100; ++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//添加数据到通道0

		//生成锯齿波
		for (i=0; i<100; ++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//添加数据到通道1
	}
	else if(current_screen_id==9)//二维码
	{
		//二维码控件显示中文字符时，需要转换为UTF8编码，
		//通过“指令助手”，转换“广州大彩123” ，得到字符串编码如下
		uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*!
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(uint8 press,uint16 x,uint16 y)
{
	//TODO: 添加用户代码
}

void SetTextValueInt32(uint16 screen_id, uint16 control_id,int32 value)
{
	u8 buffer[12] = {0};
	sprintf(buffer,"%ld",value); //把整数转换为字符串
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(uint16 screen_id, uint16 control_id,float value)
{
	u8 buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//把浮点数转换为字符串(保留一位小数)
	SetTextValue(screen_id,control_id,buffer);
}

void UpdateUI()
{
	if(current_screen_id==0)//文本设置和显示
	{
		;
	}
}

/*!
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
	u8 cleanscreen[] = {0};

	//TODO: 添加用户代码
	if(screen_id == 1)          //故障检测切换界面
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
  if(screen_id==2)//按钮、图标、动画控制
	{
		if(control_id==4)//运行加按钮
		{
			InNum += 1;
			OutNum += 1;
			SetTextValueInt32(2,2,InNum);  //更新进馆人数
			SetTextValueInt32(2,3,OutNum);  //更新进馆人数
			SetTextValueInt32(0,2,InNum);  //更新显示进馆人数
			SetTextValueInt32(0,3,OutNum);  //更新显示进馆人数
			sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
		}
		else if(control_id==5)//运行减按钮
		{
			InNum -= 1;
			OutNum -= 1;
			SetTextValueInt32(2,2,InNum);  //更新进馆人数
			SetTextValueInt32(2,3,OutNum);  //更新进馆人数
			SetTextValueInt32(0,2,InNum);  //更新显示进馆人数
			SetTextValueInt32(0,3,OutNum);  //更新显示进馆人数
			sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
		}
		else if(control_id==6)//复位按钮
		{
			value = InNum = OutNum =0;
			SetTextValueInt32(2,2,value);  //更新进馆人数
			SetTextValueInt32(2,3,value);  //更新进馆人数
			SetTextValueInt32(0,2,value);  //更新显示进馆人数
			SetTextValueInt32(0,3,value);  //更新显示进馆人数
			sprintf(InNumtempW,"%ld",value); //把整数转换为字符串
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			sprintf(OutNumtempW,"%ld",value); //把整数转换为字符串
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));

		}
		else if(control_id==7)       //反转进出人数
		{
			if(sInvertNum == 1)
			{
				sInvertNum = 2;
			}
			else if(sInvertNum == 2)
			{
				sInvertNum = 1;
			}
			sprintf(sInvertNumtemp,"%ld",sInvertNum); //把整数转换为字符串
			W25QXX_Write((u8*)sInvertNum,sInvertNum_Modeadd,sizeof(sInvertNum));
		}
	}

	else if(screen_id==5)//按钮、图标、动画控制
	{
		if(control_id==2 && pass == 1)//运行按钮
			//if(control_id==2 )//运行按钮
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
		//sprintf(Door_Modetemp,"%ld",Door_Mode); //把整数转换为字符串
		Door_Modetemp[0] = Door_Mode;
		W25QXX_Write((u8*)Door_Modetemp,Door_Modeadd,sizeof(Door_Modetemp));
		SoftReset();
	}
	else if(screen_id == 7)   //测试天线
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
	else if(screen_id == 8)   // 网络ip设置
	{
		if(control_id== 18)
		{
			SoftReset();          //重置			
		}
	}			
}

/*!
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
		u8 abc;
	//TODO: 添加用户代码
//	int32 value = 0;
	if(screen_id==2)//画面ID2：文本设置和显示
	{
		sscanf(str,"%ld",&value);//把字符串转换为整数

		if(control_id==2)//进馆人数按钮按下
		{
			//限定数值范围（也可以在文本控件属性中设置）
			if(value<0)
				value = 0;
			else if(value>9999999)
				value = 9999999;
			InNum = value;
			sprintf(InNumtempW,"%ld",InNum); //把整数转换为字符串
			W25QXX_Write((u8*)InNumtempW,InNumadd,sizeof(InNumtempW));
			SetTextValueInt32(2,2,InNum);  //更新进馆人数
			SetTextValueInt32(0,2,InNum);  //更新显示进馆人数
		}
		if(control_id==3)//出馆人数按钮按下
		{
			//限定数值范围（也可以在文本控件属性中设置）
			if(value<0)
				value = 0;
			else if(value>9999999)
				value = 9999999;
			OutNum = value;
			sprintf(OutNumtempW,"%ld",OutNum); //把整数转换为字符串
			W25QXX_Write((u8*)OutNumtempW,OutNumadd,sizeof(OutNumtempW));
			SetTextValueInt32(2,3,OutNum);  //更新出馆人数
			SetTextValueInt32(0,3,OutNum);  //更新显示出馆人数
		}
	}
	if(screen_id==3)//画面ID3：文本设置和显示
	{
		sscanf(str,"%ld",&value);//把字符串转换为整数
		if(control_id==4)            //设置功率
		{
			if(value<10)
				value = 10;
			else if(value>30)
				value = 30;
			PowertempW[0] = value;
			SetMeterValue(3,2,PowertempW[0]);         //更新仪表
			SetSliderValue(3,3,PowertempW[0]);        //更新进度条数
			//sprintf(PowertempW,"%ld",PowerValue); //把整数转换为字符串
			W25QXX_Write((u8*)PowertempW,Poweradd,sizeof(PowertempW));
			SoftReset();
		}
	}

	if(screen_id==5)//画面ID5：密码设置
	{
		if(control_id == 4)
		{
			if(memcmp(str,SetPasswordTemp,PasswordLen[0]) == 0)
			{
				pass = 1;
			}
		}
	}
		if(screen_id==8)                                               //画面ID8 ： 网络设置界面
	{
		sscanf(str,"%ld",&value);//把字符串转换为整数
		if(control_id==1)            //设置ip1
		{
			IpAddTemp1[0] = value;
      SetTextValueInt32(8,1,IpAddTemp1[0]);  //更新IP1
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==2)            //设置ip2
		{
			IpAddTemp1[1] = value;
			SetTextValueInt32(8,2,IpAddTemp1[1]);  //更新IP2	
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==3)            //设置ip3
		{
			IpAddTemp1[2] = value;
			SetTextValueInt32(8,3,IpAddTemp1[2]);  //更新IP2		
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==4)            //设置ip4
		{
			IpAddTemp1[3] = value;
			SetTextValueInt32(8,4,IpAddTemp1[3]);  //更新IP4
			W25QXX_Write((u8*)IpAddTemp1,Ipaddadd1,sizeof(IpAddTemp1));
		}
		if(control_id==5)            //设置子网掩码1
		{
			SubnetMaskTemp1[0] = value;
			SetTextValueInt32(8,5,SubnetMaskTemp1[0]);  //更新子网掩码1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==6)            //设置子网掩码2
		{
			SubnetMaskTemp1[1] = value;
			SetTextValueInt32(8,6,SubnetMaskTemp1[1]);  //更新子网掩码1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==7)            //设置子网掩码3
		{
			SubnetMaskTemp1[2] = value;
			SetTextValueInt32(8,7,SubnetMaskTemp1[2]);  //更新子网掩码1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==8)            //设置子网掩码4
		{
			SubnetMaskTemp1[3] = value;
			SetTextValueInt32(8,8,SubnetMaskTemp1[3]);  //更新子网掩码1
			W25QXX_Write((u8*)SubnetMaskTemp1,SubnetMaskadd1,sizeof(SubnetMaskTemp1));
		}
		if(control_id==9)            //设置默认网关1
		{
			GateWayTemp1[0] = value;
			SetTextValueInt32(8,9,GateWayTemp1[0]);  //更新子网掩码
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==10)            //设置默认网关2
		{
			GateWayTemp1[1] = value;
			SetTextValueInt32(8,10,GateWayTemp1[1]);  //更新子网掩码
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==11)            //设置默认网关3
		{
			GateWayTemp1[2] = value;
			SetTextValueInt32(8,11,GateWayTemp1[2]);  //更新子网掩码
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==12)            //设置默认网关4
		{
			GateWayTemp1[3] = value;
			SetTextValueInt32(8,12,GateWayTemp1[3]);  //更新子网掩码
			W25QXX_Write((u8*)GateWayTemp1,GateWayadd1,sizeof(GateWayTemp1));
		}
		if(control_id==13)            //设置服务器ip1
		{
			ServerIpTemp1[0] = value;
			SetTextValueInt32(8,13,ServerIpTemp1[0]);  //更新服务器IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==14)            //设置服务器ip2
		{
			ServerIpTemp1[1] = value;
			SetTextValueInt32(8,14,ServerIpTemp1[1]);  //更新服务器IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==15)            //设置服务器ip3
		{
			ServerIpTemp1[2] = value;
			SetTextValueInt32(8,15,ServerIpTemp1[2]);  //更新服务器IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==16)            //设置服务器ip4
		{
			ServerIpTemp1[3] = value;
			SetTextValueInt32(8,16,ServerIpTemp1[3]);  //更新服务器IP1
			W25QXX_Write((u8*)ServerIpTemp1,ServerIPadd1,sizeof(ServerIpTemp1));
		}
		if(control_id==17)                                                                 //设置服务器端口
		{
			ServerPort = value;
			SetTextValueInt32(8,17,ServerPort);  //更新服务器端口
			sprintf(ServerPortTemp,"%ld",ServerPort); //把整数转换为字符串
			W25QXX_Write((u8*)ServerPortTemp,ServerPortadd,sizeof(ServerPortTemp));
		}
		if(control_id==23)            //设置MAC地址5
		{
			MacAddTemp1[0] = value;
			SetTextValueInt32(8,23,MacAddTemp1[0]);  //更新MAC地址
			W25QXX_Write((u8*)MacAddTemp1,MacAddadd1,10);
		}
		if(control_id==24)            //设置MAC地址6
		{
			MacAddTemp1[1] = value;
			SetTextValueInt32(8,24,MacAddTemp1[1]);  //更新MAC地址
			W25QXX_Write((u8*)MacAddTemp1,MacAddadd1,10);
		}
	}
}

/*!
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
	if(screen_id==3&&control_id==3)//滑块控制
	{
		PowertempW[0] = value;
		SetMeterValue(3,2,PowertempW[0]);                          //调节功率
		SetTextValueInt32(3,4,PowertempW[0]);  
		//sprintf(PowertempW,"%ld", PowertempW[0]); //把整数转换为字符串
		W25QXX_Write((u8*)PowertempW,Poweradd,sizeof(PowertempW));
		SoftReset();
	}
	if(screen_id==4&&control_id==3)//滑块控制
	{
		VolumetempW[0] = value;
		SetMeterValue(4,2,value); //更新进度条数值
		AdjustVolume(value);                               //调节声音
		OpenBEEP(Beeppwmval);
		delay_ms(500);
		StopBEEP;
		delay_ms(100);
		OpenBEEP(Beeppwmval);
		delay_ms(500);
		StopBEEP;
		//sprintf(VolumetempW,"%ld",value); //把整数转换为字符串
		W25QXX_Write((u8*)VolumetempW,Volumeadd,sizeof(VolumetempW));
	}
}

/*!
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(uint16 screen_id, uint16 control_id, uint32 value)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(uint16 screen_id, uint16 control_id, uint8  item)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(uint16 screen_id, uint16 control_id)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
void NotifyReadFlash(uint8 status,uint8 *_data,uint16 length)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(uint8 status)
{
	//TODO: 添加用户代码
}

/*!
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
void NotifyReadRTC(uint8 year,uint8 month,uint8 week,uint8 day,uint8 hour,uint8 minute,uint8 second)
{
}


//设备重置
void SystemReset(void)
{
	W25QXX_Read(Powertemp,Poweradd,sizeof(Powertemp));   //重置功率
	if((Powertemp[0] == 0)||(Powertemp[0] == 0xff))
	{
		Powertemp[0] = 16;
	}
	SetSliderValue(3,3,Powertemp[0]);
	SetMeterValue(3,2,Powertemp[0]); //更新进度条数值
	SetTextValueInt32(3,4,Powertemp[0]);  //更新功率
	UHFInit(Powertemp[0]*10);

	W25QXX_Read(InNumtemp,InNumadd,sizeof(InNumtemp));   //重置进入人数
	sscanf(InNumtemp,"%ld",&InNum);             //把字符串转换为整数
	SetTextValueInt32(0,2,InNum);               //更新显示进馆人数
	SetTextValueInt32(2,2,InNum);               //更新显示进馆人数

	W25QXX_Read(OutNumtemp,OutNumadd,sizeof(OutNumtemp));   //重置出馆人数
	sscanf(OutNumtemp,"%ld",&OutNum);             //把字符串转换为整数
	SetTextValueInt32(0,3,OutNum);               //更新显示进馆人数
	SetTextValueInt32(2,3,OutNum);               //更新显示进馆人数

	W25QXX_Read(Volumetemp,Volumeadd,sizeof(Volumetemp));   //重置音量
	if(Volumetemp[0] == 0xff)
	{
		Volumetemp[0] = 50;
	}
	SetSliderValue(4,3,Volumetemp[0]);
	SetMeterValue(4,2,Volumetemp[0]); //更新进度条数值
//	AdjustVolume(Volumetemp[0]);

	W25QXX_Read(Door_Modetemp,Door_Modeadd,sizeof(Door_Modetemp));   //重置模式
	if(Door_Modetemp[0] == 0xff)
	{
		Door_Mode = Door_Modetemp[0] = 0;
	}
	else Door_Mode = Door_Modetemp[0];
	ModeReset(Door_Mode);
	
	W25QXX_Read(sInvertNumtemp,sInvertNum_Modeadd,sizeof(sInvertNumtemp));    //重置反转模式
	sscanf(sInvertNumtemp,"%ld",&sInvertNum); //把字符串转换为整数
	if(sInvertNum == 0xff)
	{
		sInvertNum = 1;
	}
	
	W25QXX_Read(SetPasswordTemp,SetPasswordadd,sizeof(SetPasswordTemp));    //重置密码
	if((SetPasswordTemp[0] == 0xff)|| (SetPasswordTemp[0] == 0x00))
	{
		memcpy(SetPasswordTemp,SetPassword,sizeof(SetPassword));
	}
	W25QXX_Read(PasswordLen,PasswordLenadd,sizeof(PasswordLen));    //重置密码长度
	


}
/****************************************************************************************************

函数原型:    	void SoftReset( void )
功能描述:	    软复位

输入参数:			无


输出参数:      无
返回值:        无
****************************************************************************************************/
void SoftReset( void )
{
#if DEBUG
	printf("进入软复位指令\r\n\r\n");
#endif
	__set_FAULTMASK(1);
	NVIC_SystemReset();
}



/****************************************************************************************************

函数原型:    	void ModeReset (u8 mode)
功能描述:	    mode 复位

输入参数:			模式


输出参数:      无
返回值:        无
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






