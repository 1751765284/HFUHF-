#include "beep.h"
#include "timer.h"
//#include "ulitity.h"


#define ADJUSTBEEP 1                      //�Ƿ�������BEEPģʽ
#define MaxV      0xff                     //�������
#define MinV      0                        //����
//#define BEEP(x)   ((x) ? (GPIO_ResetBits(GPIOC, GPIO_Pin_6)) : (GPIO_SetBits(GPIOC, GPIO_Pin_6)));

u16 Beeppwmval=0;                       
                      


#if  ADJUSTBEEP

//��ʼ��PB8Ϊ�����.��ʹ������ڵ�ʱ��		    
//��������ʼ��
void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��GPIOC�˿�ʱ��
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;				 //BEEP-->PC.6 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOC.6
 
 GPIO_ResetBits(GPIOC,GPIO_Pin_6);//���0���رշ��������

}


/*******************************************************************************
* ������  : AdjustVolume
* ����    : �����ն��ϻ�����룬 ��������
* ����    : getch
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void AdjustVolume(u32 Volume)
{
	OpenBEEP(Beeppwmval);
	//TIM_SetCompare1(TIM3,Beeppwmval);     //����PWMռ�ձ�
	if (Volume == 0)
	{
	//	StopBEEP;//���0���رշ��������
		Volume++;
	}
	 if(Volume >=1)
	{
	  Beeppwmval = 900-Volume*9;
		OpenBEEP(Beeppwmval);
		StopBEEP;
	}

}
#endif
