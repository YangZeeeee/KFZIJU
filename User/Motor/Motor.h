#ifndef _Motor_H
#define _Motor_H

#define Motor_GPIO					GPIOF
#define	Motor_Pul_Pin				GPIO_Pin_1
#define Motor_Dir_Pin				GPIO_Pin_0
#define Motor_En_Pin				GPIO_Pin_2

#define Motor_PulP					GPIO_WriteBit(Motor_GPIO,Motor_Pul_Pin,Bit_SET)
#define Motor_PulN					GPIO_WriteBit(Motor_GPIO,Motor_Pul_Pin,Bit_RESET)

#define Motor_DirP					GPIO_WriteBit(Motor_GPIO,Motor_Dir_Pin,Bit_SET)
#define Motor_DirN					GPIO_WriteBit(Motor_GPIO,Motor_Dir_Pin,Bit_RESET)

#define Motor_EnP						GPIO_WriteBit(Motor_GPIO,Motor_En_Pin,Bit_SET)
#define Motor_EnN						GPIO_WriteBit(Motor_GPIO,Motor_En_Pin,Bit_RESET)

void Motor_GPIO_Init(void);

#endif

