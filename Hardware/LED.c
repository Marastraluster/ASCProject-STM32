#include "stm32f10x.h"                  // Device header
#include "Timer.h"

extern int dir;
extern int Led_speed;
const uint16_t LED_PINS[4] = {GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15};

void LED_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}


void LED_Update(void){
    static int8_t led_index=0;
    GPIO_ResetBits(GPIOB, LED_PINS[0]|LED_PINS[1]|LED_PINS[2]|LED_PINS[3]);
    switch (led_index){
        case 0:
            GPIO_ResetBits(GPIOB, LED_PINS[0]);
            GPIO_SetBits(GPIOB, LED_PINS[1]|LED_PINS[2]|LED_PINS[3]);
            break;
        case 1:
            GPIO_ResetBits(GPIOB, LED_PINS[1]);
            GPIO_SetBits(GPIOB, LED_PINS[0]|LED_PINS[2]|LED_PINS[3]);
            break;
        case 2:
            GPIO_ResetBits(GPIOB, LED_PINS[2]);
            GPIO_SetBits(GPIOB, LED_PINS[0]|LED_PINS[1]|LED_PINS[3]);
            break;
        case 3:
            GPIO_ResetBits(GPIOB, LED_PINS[3]);
            GPIO_SetBits(GPIOB, LED_PINS[0]|LED_PINS[1]|LED_PINS[2]);
            break;
    }
    led_index += (dir >= 0 ? 1 : -1);
    if (led_index > 3) led_index = 0;
    if (led_index < 0) led_index = 3;
}
extern void TIM2_SetInterval_ms(uint16_t ms);
void LED_SetSpeed(void){
    if(Led_speed==0)TIM2_SetInterval_ms(500);
    else if(Led_speed==1)TIM2_SetInterval_ms(1000);
    else if(Led_speed==2)TIM2_SetInterval_ms(200);
}
