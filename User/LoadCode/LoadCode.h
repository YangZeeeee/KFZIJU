#ifndef _LoadCode_H
#define _LoadCode_H

struct LoadCodeS
{
	unsigned int  LoadCodeTime;  		//���ʱ�� ��ʱ��Ϊ10s
	unsigned char LoadCodeTime_R;		//ʱ���ѵ���־λ
	unsigned char LoadCodeResult;		//����������� 2 ʧ��  1�ɹ�
	unsigned char LoadCodeStep;  		//����������̡�
	unsigned int  HintDisPlayTime;  //��ʾ����ʾʱ�䡣	
	unsigned char HintDisPlayFLag;  //��ʾ�ı�־λ 
};
extern struct LoadCodeS LoadCodeP;
void LoadCode_Process(void);
void LoadCodeP_Init(void);
void HintDisplay_Send(void);
void HintYinCang_Send(void);
#endif
