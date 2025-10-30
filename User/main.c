#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "OLED.h"
#include "Key.h"
#include "Serial.h"
#include "string.h"
#include "Timer.h"
#include "Delay.h"




uint8_t Speed=0;

int main(void){
	OLED_Init();
	Motor_Init();
	Key_Init();
	Serial_Init();
	Timer_Init();
	while (1){
		
	}
}
void TIM1_UP_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		//PID控制代码放这里

		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
