#ifndef _LoadCode_H
#define _LoadCode_H

struct LoadCodeS
{
	unsigned int  LoadCodeTime;  		//检测时长 总时长为10s
	unsigned char LoadCodeTime_R;		//时长已到标志位
	unsigned char LoadCodeResult;		//载入代码检测结果 2 失败  1成功
	unsigned char LoadCodeStep;  		//载入代码流程。
	unsigned int  HintDisPlayTime;  //提示的显示时间。	
	unsigned char HintDisPlayFLag;  //提示的标志位 
};
extern struct LoadCodeS LoadCodeP;
void LoadCode_Process(void);
void LoadCodeP_Init(void);
void HintDisplay_Send(void);
void HintYinCang_Send(void);
#endif
