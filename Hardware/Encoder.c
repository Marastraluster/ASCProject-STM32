#include "stm32f10x.h"
#include "Encoder.h"

/* A=PB0, B=PB1 */
volatile int encoder_delta = 0;

static inline int ENC_A(void){ return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0); }
static inline int ENC_B(void){ return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1); }

void Encoder_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitTypeDef io;
    io.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
    io.GPIO_Mode  = GPIO_Mode_IPU;   // 公共端接GND；若公共端接VCC，请改为 GPIO_Mode_IPD
    io.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &io);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);

    EXTI_InitTypeDef ex;
    EXTI_StructInit(&ex);
    ex.EXTI_Mode    = EXTI_Mode_Interrupt;
    // 先用单边沿做连通性测试，成功后再改 Falling+Rising
    ex.EXTI_Trigger = EXTI_Trigger_Rising;

    ex.EXTI_Line    = EXTI_Line0; ex.EXTI_LineCmd = ENABLE; EXTI_Init(&ex);
    ex.EXTI_Line    = EXTI_Line1; ex.EXTI_LineCmd = ENABLE; EXTI_Init(&ex);

    // 关键：初始化后清一次挂起位，避免“刚上电就挂起”把第一次中断吃掉
    EXTI_ClearITPendingBit(EXTI_Line0 | EXTI_Line1);

    // 显式设置分组，避免优先级配置被忽略
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitTypeDef n;
    n.NVIC_IRQChannel = EXTI0_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 1;
    n.NVIC_IRQChannelSubPriority = 1;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);

    n.NVIC_IRQChannel = EXTI1_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 1;
    n.NVIC_IRQChannelSubPriority = 2;
    n.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&n);
}

/* IRQ 内最小处理：判断方向，累计增量 */
void Encoder_EXTI_Handler(uint8_t line){
    int a = ENC_A();
    int b = ENC_B();
    if(line == 0){
        encoder_delta += (a != b) ? +1 : -1;
    }else{
        encoder_delta += (a == b) ? +1 : -1;
    }
}

int Encoder_FetchDelta(void){
    int d;
    __disable_irq();
    d = encoder_delta;
    encoder_delta = 0;
    __enable_irq();
    return d;
}

void Encoder_Clear(void){
    __disable_irq();
    encoder_delta = 0;
    __enable_irq();
}