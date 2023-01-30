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
	IndicLigh_Current,//指示灯电流
//	Voltage3_3,//电压3.3V检测
	Power_Current,//上电电流
//	ValueOpen_Current,//阀开启电流
////	ValueLoad_Current,//阀负载电流
	Standby_Current, //待机电流
	Trans_Power,//电源ADC值 - 学习结果 - 发射功率
//	PowerADC_Value,//电源ADC值
	Display_Send,
//	GanYinOpen_Result,//感应开启电流
	//GanyinReady_Time,//感应就绪时间
	Ganyin_Distance,//感应距离
	Restart_Motor,   //一次检验完让电机重新归位
	Sleep_Current, //休眠电流
};


//整个结构体的数据
struct ParaData
{
	unsigned long Power_Current_Test;//上电电流 = 驱动电路使能电流
	unsigned long Value_Current_Test;//阀负载电流
	unsigned long Light_Current_Test;//指示灯电流
	unsigned long Standby_Current_Test;//待机电流
	unsigned char Shoot_Power_Test;//发射功率
	unsigned int  Power_ADC_Test;//电压ADC
	unsigned char Learn_Test;   //学习结果
	unsigned char OPA1_Test;
	unsigned char OPA0_Test;
	unsigned int GanDistance_Test;  //感应距离
};


extern struct ParaData TestPara;

struct Step
{
	unsigned char Test_Press; //步骤流程。
	unsigned int Test_Time;  //时长超过十秒钟后 则显示超时。
	unsigned char Test_Flag;  //用于判断此时是否是在测试的时候
	unsigned char Test_Start; //此位为1时开始测试  为0时暂停测试流程。
	unsigned char Test_ReadFlag; //保存参数读取参数值
	unsigned char Test_ReadFlag_Press;//读取保存参数流程
	unsigned char Test_ReadRec_Flag; // 读取到单个文本的标志位 跟前面整个保存参数标志位要区分开
	unsigned char Motor_Step;
	unsigned int Motor_ReTime;
	unsigned int Motor_ReFlag;
};
extern struct Step  TestStep;

void Data_Analy(void);
void Start_Testing(void);
void Read_TextPara(void);


#endif
