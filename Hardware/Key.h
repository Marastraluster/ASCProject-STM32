#ifndef __KEY_H__
#define __KEY_H__

void Key_Init(void);
void Key_Scan(void);     // 每1ms在 TIM3_IRQHandler 调用
int  Key_GetNum(void);   // 主循环/菜单调用，获取一次性事件

#endif
