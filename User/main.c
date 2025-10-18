#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "Menu.h"
#include "Delay.h"
#include "Encoder.h"


int KeyNum = 0;
int EnterFlag_LED_Speed = 0;
int EnterFlag_LED_Dir = 0;
int EnterFlag_PID = 0;
int dir = 1;
int Led_speed = 0;
int Encoder_speed=0;
volatile uint32_t enc_irq_cnt0 = 0, enc_irq_cnt1 = 0;

int main(void){
	OLED_Init();
	LED_Init();
	Timer2_Init();
	Timer3_Init();
	Key_Init();
	Encoder_Init();
	LED_SetSpeed();
	while (1){
		Menu_ShowMain();
	}
}


void TIM2_IRQHandler(void){
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET){
		LED_Update();
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	}
}
void TIM3_IRQHandler(void){
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET){
		Key_Scan(); // 每1ms扫描一次
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
	}
}
void EXTI0_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line0) == SET){
        enc_irq_cnt0++;                 // 计数调试
        Encoder_EXTI_Handler(0);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
void EXTI1_IRQHandler(void){
    if(EXTI_GetITStatus(EXTI_Line1) == SET){
        enc_irq_cnt1++;                 // 计数调试
        Encoder_EXTI_Handler(1);
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}
