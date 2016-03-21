/****************************
					 Y(Roll)
     ˳ʱ��ת | ��ʱ��ת
      motor1  |  motor4
       ����   |   ����
    --------------------X(Pitch)          
     ��ʱ��ת | ˳ʱ��ת 
      motor2  |  motor3
       ����   |   ���� 
              |
****************************/

#include "control.h"
#include "telecontrol.h"

volatile u16 motor[5];																		//������
vs16 throttle = 0;																				//����ֵ
vu8 fly_state = 0;																				//�������״̬

volatile PID PID_Roll_Angle,PID_Pitch_Angle,PID_Yaw_Angle;//�⻷���Ƕ�PID��
volatile PID PID_Roll_Rate,PID_Pitch_Rate,PID_Yaw_Rate;		//�ڻ������ٶ�PID��
volatile PID PID_Height;																	//�߶�
volatile S_FLOAT_XYZ Exp_Angle;														//�����Ƕ�
volatile S_FLOAT_XYZ MPU6500_Gyro_Last;										//��һ�μ�¼������������

extern u8 unlock_flag;//������־λ��Ϊ0δ������Ϊ1����	
extern float angle_z;

volatile s32 exp_height;
vu8 exp_height_flag = 1;
extern s32 height;
extern u8 rc_channel_5__status;	//ң��ͨ����״̬��Ϊ0�������棬Ϊ1��������
extern u8 rc_channel_6__status;	//ң��ͨ����״̬��Ϊ0�������棬Ϊ1��������

void Inner_Loop_Control(void);
void Outter_Loop_Control(void);
void Height_Control(void);

/*
 * ��������Control
 * ����  ��������̬��Ϣ����������
 * ����  ��Now_Angle����ǰ��̬��Exp_Angle��������̬
 * ���  ����
 */ 
void Control(void)
{	
	Outter_Loop_Control();
	Inner_Loop_Control();
	Height_Control();	
	if((throttle > LAUNCH_THROTTLE) && (unlock_flag) && (Is_WFLY_Connected()))//�����Ŵ���100�������ˣ���ң�����������������
	{
		motor[1] = ADD_THROTTLE + throttle - PID_Roll_Rate.Out - PID_Pitch_Rate.Out - PID_Yaw_Rate.Out + PID_Height.Out;
		motor[2] = ADD_THROTTLE + throttle - PID_Roll_Rate.Out + PID_Pitch_Rate.Out + PID_Yaw_Rate.Out + PID_Height.Out;
		motor[3] = ADD_THROTTLE + throttle + PID_Roll_Rate.Out + PID_Pitch_Rate.Out - PID_Yaw_Rate.Out + PID_Height.Out;
		motor[4] = ADD_THROTTLE + throttle + PID_Roll_Rate.Out - PID_Pitch_Rate.Out + PID_Yaw_Rate.Out + PID_Height.Out;
		
		fly_state = 1;
	}
	else
	{
			motor[1]  = 0;
			motor[2]  = 0;
			motor[3]  = 0;
			motor[4]  = 0;
			
		fly_state = 0;
	}

	Motor_PWM_Update(motor[1] ,motor[2] ,motor[3] ,motor[4]);

}

/*
 * ��������Outter_Loop_Control
 * ����  ���⻷���ƺ���
 * ����  ����
 * ���  ����
 */  
void Outter_Loop_Control(void)
{
	//������+�����ǣ����߷����෴��ʵ��Ϊƫ��
	float roll_angle_err = Exp_Angle.Y + Angle.Y;
	float pitch_angle_err = Exp_Angle.X + Angle.X;
	float yaw_angle_err = Exp_Angle.Z + angle_z;
	
	PID_Roll_Angle.Pout = PID_Roll_Angle.P * roll_angle_err;
	PID_Pitch_Angle.Pout = PID_Pitch_Angle.P * pitch_angle_err;
	PID_Yaw_Angle.Pout = PID_Yaw_Angle.P * yaw_angle_err;		
	
	PID_Pitch_Angle.Dout = PID_Pitch_Angle.D * MPU6500_Gyro.X;	
	PID_Roll_Angle.Dout = PID_Roll_Angle.D * MPU6500_Gyro.Y;
	PID_Yaw_Angle.Dout = PID_Yaw_Angle.D * MPU6500_Gyro.Z;
	
	PID_Roll_Angle.Iout += (PID_Roll_Angle.I * roll_angle_err);
	PID_Pitch_Angle.Iout += (PID_Pitch_Angle.I * pitch_angle_err);
	//�����޷�
	if(PID_Roll_Angle.Iout > 30)	PID_Roll_Angle.Iout = 30;
	else if(PID_Roll_Angle.Iout < -30)	PID_Roll_Angle.Iout = -30;
	if(PID_Pitch_Angle.Iout > 30) PID_Pitch_Angle.Iout = 30;
	else if(PID_Pitch_Angle.Iout < -30) PID_Pitch_Angle.Iout = -30;
	
	//�⻷�������Ϊ�ڻ����룬������ֵ
	PID_Roll_Angle.Out = PID_Roll_Angle.Pout + PID_Roll_Angle.Iout + PID_Roll_Angle.Dout;
	PID_Pitch_Angle.Out = PID_Pitch_Angle.Pout + PID_Pitch_Angle.Iout + PID_Pitch_Angle.Dout;
	PID_Yaw_Angle.Out = PID_Yaw_Angle.Pout + PID_Yaw_Angle.Iout + PID_Yaw_Angle.Dout;
}


/*
 * ��������Inner_Loop_Control
 * ����  ���ڻ����ƺ���
 * ����  ����
 * ���  ����
 */  
void Inner_Loop_Control(void)
{ 
	static float roll_rate_err = 0,pitch_rate_err = 0,yaw_rate_err = 0;
	
	roll_rate_err = PID_Roll_Angle.Out + MPU6500_Gyro.Y * 0.0610351f;
	pitch_rate_err = PID_Pitch_Angle.Out + MPU6500_Gyro.X * 0.0610351f;
	yaw_rate_err = PID_Yaw_Angle.Out + MPU6500_Gyro.Z * 0.0610351f;
	
	PID_Roll_Rate.Pout = PID_Roll_Rate.P  * roll_rate_err;
	PID_Pitch_Rate.Pout= PID_Pitch_Rate.P * pitch_rate_err;
	PID_Yaw_Rate.Pout  = PID_Yaw_Rate.P * yaw_rate_err;
	
	PID_Roll_Rate.Dout = PID_Roll_Rate.D * (MPU6500_Gyro.Y - MPU6500_Gyro_Last.Y);//��������
	PID_Pitch_Rate.Dout = PID_Pitch_Rate.D * (MPU6500_Gyro.X - MPU6500_Gyro_Last.X);
	PID_Yaw_Rate.Dout = PID_Yaw_Rate.D * (MPU6500_Gyro.Z - MPU6500_Gyro_Last.Z);
	
	PID_Roll_Rate.Iout += (PID_Roll_Rate.I * roll_rate_err);
	PID_Pitch_Rate.Iout += (PID_Pitch_Rate.I * pitch_rate_err);
	
	if(PID_Roll_Rate.Iout > 30) PID_Roll_Rate.Iout = 30;   
	else if(PID_Roll_Rate.Iout < -30) PID_Roll_Rate.Iout = -30;
	if(PID_Pitch_Rate.Iout > 100)	PID_Pitch_Rate.Iout = 30;
	else if(PID_Pitch_Rate.Iout < -30)	PID_Pitch_Rate.Iout = -30;
	
	PID_Roll_Rate.Out = PID_Roll_Rate.Pout + PID_Roll_Rate.Dout + PID_Roll_Rate.Iout;
	PID_Pitch_Rate.Out = PID_Pitch_Rate.Pout + PID_Pitch_Rate.Dout + PID_Roll_Rate.Iout;
  PID_Yaw_Rate.Out = PID_Yaw_Rate.Pout + PID_Yaw_Rate.Dout;
	
	//�洢���μ�¼�������ǽ�����
	MPU6500_Gyro_Last.Y = MPU6500_Gyro.Y;
	MPU6500_Gyro_Last.X = MPU6500_Gyro.X;
	MPU6500_Gyro_Last.Z = MPU6500_Gyro.Z;
}

/*
 * ��������Height_Control
 * ����  ���߶ȿ��ƺ���
 * ����  ����
 * ���  ����
 */  
void Height_Control(void)
{
	static s32 height_err = 0;	//�߶�ƫ��
	static s32	height_Last = 0;
	if(rc_channel_5__status)	//���붨��ģʽ,ע�����ݸ��µ�Ƶ��ֻ��50hz
	{ 
		if(exp_height_flag == 1)
		{
			exp_height = height;	//��¼�߶�����ֵ
			exp_height_flag = 0;
		}
		
		height_err = exp_height - height;
		if((height_err > 500) || (height_err < -500)) 
		{
			height_err = 0;//ƫ�����500cm������Ϊ���ݴ���ϵͳ����������ģʽʧЧ
			PID_Height.Out = 0;
		}
		else
		{
			PID_Height.Pout = PID_Height.P * height_err;
			PID_Height.Dout = PID_Height.D * (height - height_Last);//��������
			height_Last = height;//�洢���θ߶�ֵ
			//PID_Height.Dout=PID_Height.D*Vz; //����ĸ���Ƶ��Ϊ50hz
		   //��Ϊ�߶ȵĸ���Ƶ��Ҫ��Ķ࣬��������Ҫ����һ��
			PID_Height.Out = PID_Height.Pout + PID_Height.Iout -PID_Height.Dout;
		}			
		
	}
	else
	{
	   PID_Height.Out=0;
		 PID_Height.Pout=0;
		 PID_Height.Dout=0;
     exp_height_flag = 1;
	}
}

/****************************
������������ѣ���С����˳���
���Ǳ�������֣�����ٰ�΢�ּ�
�����񵴺�Ƶ������������Ҫ�Ŵ�
����Ư���ƴ��壬����������С��
����ƫ��ظ���������ʱ�����½�
���߲������ڳ�������ʱ���ټӳ�
������Ƶ�ʿ죬�Ȱ�΢�ֽ�����
���������������΢��ʱ��Ӧ�ӳ�
����������������ǰ�ߺ��4��1
һ��������������������������
****************************/

/*
 * ��������PID_Init
 * ����  ��PID������ʼ������
 * ����  ����
 * ���  ����
 */ 
void PID_Init(void)
{
//�⻷���ǶȻ�	
	PID_Roll_Angle.P = 8;
	PID_Roll_Angle.I = 0.001;
	PID_Roll_Angle.D = 0;
	
	PID_Pitch_Angle.P = 8;
	PID_Pitch_Angle.I = 0.001;
	PID_Pitch_Angle.D = 0;
	
	PID_Yaw_Angle.P = 10;
	PID_Yaw_Angle.I = 0;
	PID_Yaw_Angle.D = 0;
		
//�ڻ������ٶȻ�	
	PID_Roll_Rate.P = 3.3;
	PID_Roll_Rate.I = 0;
	PID_Roll_Rate.D = 0.2;
	
	PID_Pitch_Rate.P = 3.3;
	PID_Pitch_Rate.I = 0;
	PID_Pitch_Rate.D = 0.2;
	
	PID_Yaw_Rate.P = 5;
	PID_Yaw_Rate.I = 0;
	PID_Yaw_Rate.D = 0.2;

//�߶�
  PID_Height.P = 2;
	PID_Height.I = 0;
	PID_Height.D = 1;

//��ʼ������
	PID_Roll_Angle.Pout = 0;
	PID_Roll_Angle.Iout = 0;
	PID_Roll_Angle.Dout = 0;
	
	PID_Pitch_Angle.Pout = 0;
	PID_Pitch_Angle.Iout = 0;
	PID_Pitch_Angle.Dout = 0;
	
	PID_Yaw_Angle.Pout = 0;
	PID_Yaw_Angle.Iout = 0;
	PID_Yaw_Angle.Dout = 0;
	
	PID_Roll_Rate.Pout = 0;
	PID_Roll_Rate.Iout = 0;
	PID_Roll_Rate.Dout = 0;
	
	PID_Pitch_Rate.Pout = 0;
	PID_Pitch_Rate.Iout = 0;
	PID_Pitch_Rate.Dout = 0;
	
	PID_Yaw_Rate.Pout = 0;
	PID_Yaw_Rate.Iout = 0;
	PID_Yaw_Rate.Dout = 0;
	
	PID_Height.Pout = 0;
	PID_Height.Iout = 0;
	PID_Height.Dout = 0;
}
												
						

