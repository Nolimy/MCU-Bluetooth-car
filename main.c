#include "reg52.h"

#define uchar unsigned char
#define uint unsigned int
//管脚定义
sbit IN1=P1^2;
sbit IN2=P1^3;
sbit IN3=P1^4;
sbit IN4=P1^5;
sbit IRIN=P3^2;
uchar cmd=0;//接收的指令
bit rec_flag=0;
#define right_go{IN3=0,IN4=1;}//OUT4 OUT2为+ OUT3 OUT1为— 4和3控制右 2和1控制左
#define right_back{IN3=1;IN4=0;}
#define right_stop{IN3=0;IN4=0;}
#define left_go{IN2=1,IN1=0;}
#define left_back{IN2=0;IN1=1;}
#define left_stop{IN2=0;IN1=0;}

uchar lr[6];//红外接收
uchar t;//判断逻辑1和逻辑0时的计时变量

void delay(uint x)
{
	while(x--);
}

void forword()//前进
{
	left_go;
	right_go;
}

void back()//后退
{
	left_back;
	right_back;
}

void turnleft()//左转弯 此时左轮后动 右轮前动
{
	left_back;
	right_go;
}

void turnright()//右转弯 此时左轮前动 右轮后动
{
	left_go;
	right_back;
}

void stop()//刹车
{
	left_stop;
	right_stop;
}

void UartInit()//串口初始化函数
{
	SM0=0;				//串口行控制寄存器，设置为工作方式1
	SM1=1;
	REN=1;				//允许串口接收	
	TMOD=0X20;			//设置定时器1，工作方式2
	PCON=0X80;			//SMOD为0波特率不加倍
	TH1=0XF3;			//设置波特率初始值4800,无误差
	TL1=0XF3;
	ES=1;				//打开串口中断
	EA=1;				//打开总中断
	TR1=1;				//打开定时器1
	IT0=1;				//红外接收
	EX0=1;
	IRIN=1;
}

void main()
{
	UartInit();
	while(1)
	/*{
		turnleft();//测试用
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

void Int0() interrupt 0 //红外接收模块
{
	uchar k,j;
	uint r;
	t=0;
	delay(700);
	if(IRIN==0)//延时接受信号仍然为低电平 说明接收到了正确的引导码
	{
		r=1000;//10ms延时
		while((IRIN==0)&&(r>0))//等待9ms的引导码低电平过去
		{
			delay(1);
			r--;
		}
		if(IRIN==1)//9ms后反转为高电平
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
					while((IRIN==0)&&(r>0))//等待数据位前面的560us低电平过去
					{
						delay(1);
						r--;
					}
					r=500;
					while((IRIN==1)&&(r>0))//计算高电平长度 确定其数值
					{
						delay(10);
						t++;
						r--;
						if(t>30)//高电平过长 出错退出
						{
							//EX0=1;
							return;
						}
					}
					lr[k]>>=1;//接收的第k个字节向低位移动
					if(t>=8)//接收到逻辑1
					{
						lr[k]|=0x80;//接收字节的最高位置1
					}
					t=0;//接收完后计时变量清零
				}
			}
		}
		if(lr[2]!=~lr[3])
			return;//检测到数码原码和反码不符 数据清零
	}
}