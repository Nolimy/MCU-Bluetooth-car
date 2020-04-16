#include "reg52.h"

#define uchar unsigned char
#define uint unsigned int
//�ܽŶ���
sbit IN1=P1^2;
sbit IN2=P1^3;
sbit IN3=P1^4;
sbit IN4=P1^5;
sbit IRIN=P3^2;
uchar cmd=0;//���յ�ָ��
bit rec_flag=0;
#define right_go{IN3=0,IN4=1;}//OUT4 OUT2Ϊ+ OUT3 OUT1Ϊ�� 4��3������ 2��1������
#define right_back{IN3=1;IN4=0;}
#define right_stop{IN3=0;IN4=0;}
#define left_go{IN2=1,IN1=0;}
#define left_back{IN2=0;IN1=1;}
#define left_stop{IN2=0;IN1=0;}

uchar lr[6];//�������
uchar t;//�ж��߼�1���߼�0ʱ�ļ�ʱ����

void delay(uint x)
{
	while(x--);
}

void forword()//ǰ��
{
	left_go;
	right_go;
}

void back()//����
{
	left_back;
	right_back;
}

void turnleft()//��ת�� ��ʱ���ֺ� ����ǰ��
{
	left_back;
	right_go;
}

void turnright()//��ת�� ��ʱ����ǰ�� ���ֺ�
{
	left_go;
	right_back;
}

void stop()//ɲ��
{
	left_stop;
	right_stop;
}

void UartInit()//���ڳ�ʼ������
{
	SM0=0;				//�����п��ƼĴ���������Ϊ������ʽ1
	SM1=1;
	REN=1;				//�����ڽ���	
	TMOD=0X20;			//���ö�ʱ��1��������ʽ2
	PCON=0X80;			//SMODΪ0�����ʲ��ӱ�
	TH1=0XF3;			//���ò����ʳ�ʼֵ4800,�����
	TL1=0XF3;
	ES=1;				//�򿪴����ж�
	EA=1;				//�����ж�
	TR1=1;				//�򿪶�ʱ��1
	IT0=1;				//�������
	EX0=1;
	IRIN=1;
}

void main()
{
	UartInit();
	while(1)
	/*{
		turnleft();//������
	}*/
	{
		if(rec_flag==1)
		{
			switch(cmd)
			{
				case 'f':
					forword();
					cmd=0;
					rec_flag=0;
					break;
				case 'b':
					back();
					cmd=0;
					rec_flag=0;
					break;
				case 'l':
					turnleft();
					cmd=0;
					rec_flag=0;
					break;
				case 'r':
					turnright();
					cmd=0;
					rec_flag=0;
					break;
				case 'F':
					forword();
					break;
				case 'B':
					back();
					break;
				case 'L':
					turnleft();
					break;
				case 'R':
					turnright();
					break;
				default:
					stop();
					break;
			}
				
		}
		switch(lr[2])
		{
			case 0x18:
				forword();
				//lr[2]=0;
				break;
			case 0x52:
				back();
				//lr[2]=0;
				break;
			case 0x08:
				turnleft();
				//lr[2]=0;
				break;
			case 0x5a:
				turnright();
				//lr[2]=0;
				break;
			default:
				stop();
		}	
	}
}
void UART() interrupt 4
{
	if(RI==1)
	{
		RI=0;
		cmd=SBUF;
		SBUF=cmd;
		rec_flag=1;
	}
}

void Int0() interrupt 0 //�������ģ��
{
	uchar k,j;
	uint r;
	t=0;
	delay(700);
	if(IRIN==0)//��ʱ�����ź���ȻΪ�͵�ƽ ˵�����յ�����ȷ��������
	{
		r=1000;//10ms��ʱ
		while((IRIN==0)&&(r>0))//�ȴ�9ms��������͵�ƽ��ȥ
		{
			delay(1);
			r--;
		}
		if(IRIN==1)//9ms��תΪ�ߵ�ƽ
		{
			r=500;
			while((IRIN==1)&&(r>0))
			{
				delay(1);
				r--;
			}
			for(k=0;k<4;k++)
			{
				for(j=0;j<8;j++)
				{
					r=60;
					while((IRIN==0)&&(r>0))//�ȴ�����λǰ���560us�͵�ƽ��ȥ
					{
						delay(1);
						r--;
					}
					r=500;
					while((IRIN==1)&&(r>0))//����ߵ�ƽ���� ȷ������ֵ
					{
						delay(10);
						t++;
						r--;
						if(t>30)//�ߵ�ƽ���� �����˳�
						{
							//EX0=1;
							return;
						}
					}
					lr[k]>>=1;//���յĵ�k���ֽ����λ�ƶ�
					if(t>=8)//���յ��߼�1
					{
						lr[k]|=0x80;//�����ֽڵ����λ��1
					}
					t=0;//��������ʱ��������
				}
			}
		}
		if(lr[2]!=~lr[3])
			return;//��⵽����ԭ��ͷ��벻�� ��������
	}
}