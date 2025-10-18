#include "stm32f10x.h"                  // Device header

#define DEBOUNCE_MS   20    // 去抖阈值
#define LONGPRESS_MS 1000   // 首次长按判定
#define REPEAT_MS     100   // 长按后重复间隔

/* 硬件初始化保持不变 */
void Key_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructurea;
    GPIO_InitStructurea.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_0;
    GPIO_InitStructurea.GPIO_Mode = GPIO_Mode_IPU;     // 上拉输入，按下为0
    GPIO_InitStructurea.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructurea);

    GPIO_InitTypeDef GPIO_InitStructurec;
    GPIO_InitStructurec.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructurec.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructurec.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructurec);
}

/* 供主循环获取一次性事件：0=无事件，1/2/3/4=对应键（短按一次或长按期间的每次重复） */
int Key_GetNum(void){
    extern volatile uint8_t g_key_event;
    uint8_t ev = g_key_event;
    g_key_event = 0;         // 取走即清
    return (int)ev;
}

/* 扫描与判定（在 TIM3_IRQHandler 每1ms调用） */
volatile uint8_t g_key_event = 0;  // 待取事件（单字节，主循环用 Key_GetNum 读取）

static uint8_t read_raw_key(void){
    // 只能返回一个键值：1=down(PA2), 2=up(PA4), 3=enter(PA0), 4=back(PC14)
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0) return 1;
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0) return 2;
    if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0) return 3;
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14) == 0) return 4;
    return 0;
}

void Key_Scan(void){
    static uint8_t  last_raw = 0;
    static uint8_t  stable   = 0;     // 当前去抖后的稳定键值（0/1/2/3/4）
    static uint8_t  prev_stable = 0;  // 上一次稳定值
    static uint16_t same_cnt = 0;     // 连续相同原始值的计数(ms)

    static uint16_t press_ms = 0;     // 当前按下持续时间(ms)
    static uint16_t rep_ms   = 0;     // 长按后的重复节拍(ms)
    static uint8_t  long_sent = 0;    // 是否已触发过长按

    uint8_t raw = read_raw_key();

    // 简单去抖：原始值连续一致满 DEBOUNCE_MS 才认为稳定
    if(raw == last_raw){
        if(same_cnt < 0xFFFF) same_cnt++;
    }else{
        same_cnt = 0;
        last_raw = raw;
    }

    // 达到去抖阈值，更新稳定值
    if(same_cnt >= DEBOUNCE_MS && raw != stable){
        prev_stable = stable;
        stable = raw;

        if(stable != 0){
            // 新按下：重置计时
            press_ms = 0;
            rep_ms   = 0;
            long_sent = 0;
        }else{
            // 刚刚松开：若未触发长按，则产生一次“短按”事件
            if(prev_stable != 0 && !long_sent && g_key_event == 0){
                g_key_event = prev_stable;
            }
        }
    }

    // 按住期间的长按/重复
    if(stable != 0){
        press_ms++;
        if(!long_sent){
            if(press_ms >= LONGPRESS_MS){
                if(g_key_event == 0){
                    g_key_event = stable; // 第一次长按事件
                    long_sent = 1;
                    rep_ms = 0;
                }
            }
        }else{
            rep_ms++;
            if(rep_ms >= REPEAT_MS){
                rep_ms = 0;
                if(g_key_event == 0){
                    g_key_event = stable; // 长按期间的重复事件
                }
            }
        }
    }
}
