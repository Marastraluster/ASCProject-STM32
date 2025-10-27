#include "stm32f10x.h"                  // Device header
#include "Motor.h"
#include "OLED.h"
#include "Key.h"
#include "Serial.h"


int main(void){
	OLED_Init();
	Motor_Init();
	Key_Init();
	Serial_Init();
	
	while (1){
		
	}
}
