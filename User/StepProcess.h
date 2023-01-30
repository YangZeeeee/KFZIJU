#ifndef  _StepProcess_H
#define  _StepProcess_H
#define Test_Mum	 11
#define Color_Red  0xf800
#define Color_White 0xffff

#define Send_Num    		5
#define Send_Wait_Time	3000

struct Com_Para
{
	unsigned char Com_Num;
	unsigned char Com_Flag;
	unsigned int  Com_time;
	unsigned char Com_Step;
	unsigned long Com_Value;
};

extern struct Com_Para ComP;


	
enum 
{
	Test_Init = 0,
	HW_Learn,
	ReadData_Send,
	IndicLigh_Current,//ָʾ�Ƶ���
//	Voltage3_3,//��ѹ3.3V���
	Power_Current,//�ϵ����
//	ValueOpen_Current,//����������
////	ValueLoad_Current,//�����ص���
	Standby_Current, //��������
	Trans_Power,//��ԴADCֵ - ѧϰ��� - ���书��
//	PowerADC_Value,//��ԴADCֵ
	Display_Send,
//	GanYinOpen_Result,//��Ӧ��������
	//GanyinReady_Time,//��Ӧ����ʱ��
	Ganyin_Distance,//��Ӧ����
	Restart_Motor,   //һ�μ������õ�����¹�λ
	Sleep_Current, //���ߵ���
};


//�����ṹ�������
struct ParaData
{
	unsigned long Power_Current_Test;//�ϵ���� = ������·ʹ�ܵ���
	unsigned long Value_Current_Test;//�����ص���
	unsigned long Light_Current_Test;//ָʾ�Ƶ���
	unsigned long Standby_Current_Test;//��������
	unsigned char Shoot_Power_Test;//���书��
	unsigned int  Power_ADC_Test;//��ѹADC
	unsigned char Learn_Test;   //ѧϰ���
	unsigned char OPA1_Test;
	unsigned char OPA0_Test;
	unsigned int GanDistance_Test;  //��Ӧ����
};


extern struct ParaData TestPara;

struct Step
{
	unsigned char Test_Press; //�������̡�
	unsigned int Test_Time;  //ʱ������ʮ���Ӻ� ����ʾ��ʱ��
	unsigned char Test_Flag;  //�����жϴ�ʱ�Ƿ����ڲ��Ե�ʱ��
	unsigned char Test_Start; //��λΪ1ʱ��ʼ����  Ϊ0ʱ��ͣ�������̡�
	unsigned char Test_ReadFlag; //���������ȡ����ֵ
	unsigned char Test_ReadFlag_Press;//��ȡ�����������
	unsigned char Test_ReadRec_Flag; // ��ȡ�������ı��ı�־λ ��ǰ���������������־λҪ���ֿ�
	unsigned char Motor_Step;
	unsigned int Motor_ReTime;
	unsigned int Motor_ReFlag;
};
extern struct Step  TestStep;

void Data_Analy(void);
void Start_Testing(void);
void Read_TextPara(void);


#endif
