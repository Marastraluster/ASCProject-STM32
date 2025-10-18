#include "stm32f10x.h"
#include "OLED.h"
#include "Key.h"
#include "Encoder.h"

extern int dir;          // 1 正向，-1 反向
extern int Led_speed;    // 0=500ms,1=1000ms,2=200ms
extern void LED_SetSpeed(void);

/* PID 参数（显示一位小数） */
static double kp = 0.8, ki = 0.1, kd = 0.5;

/* 页面绘制 */
static void draw_main(uint8_t row){
    OLED_Clear();
    OLED_ShowString(1, 1, row==1?">":" "); OLED_ShowString(1, 2, " LED Control");
    OLED_ShowString(2, 1, row==2?">":" "); OLED_ShowString(2, 2, " PID");
    OLED_ShowString(3, 1, row==3?">":" "); OLED_ShowString(3, 2, " Image");
    OLED_ShowString(4, 1, row==4?">":" "); OLED_ShowString(4, 2, " Angle");
}
static void draw_led(uint8_t row, uint8_t editing){
    OLED_Clear();
    OLED_ShowString(1, 1, "LED Control");
    if(editing) OLED_ShowString(1, 15, "E");

    OLED_ShowString(2, 1, row==1?">":" "); OLED_ShowString(2, 2, " LED_speed");
    OLED_ShowNum(2, 13, (uint32_t)Led_speed, 1);

    OLED_ShowString(3, 1, row==2?">":" "); OLED_ShowString(3, 2, " LED_dir");
    OLED_ShowNum(3, 13, (dir>=0)?0:1, 1);
}
static void draw_pid(uint8_t row, uint8_t editing){
    OLED_Clear();
    OLED_ShowString(1, 1, "PID");
    if(editing) OLED_ShowString(1, 15, "E");

    OLED_ShowString(2, 1, row==1?">":" "); OLED_ShowString(2, 2, " kp");
    OLED_ShowFloat(2, 11, kp, 1);

    OLED_ShowString(3, 1, row==2?">":" "); OLED_ShowString(3, 2, " ki");
    OLED_ShowFloat(3, 11, ki, 1);

    OLED_ShowString(4, 1, row==3?">":" "); OLED_ShowString(4, 2, " kd");
    OLED_ShowFloat(4, 11, kd, 1);
}
static void draw_image(uint8_t row, uint8_t editing){
    (void)editing;
    OLED_Clear();
    OLED_ShowString(1, 1, "Image");
    OLED_ShowString(2, 1, row==1?">":" "); OLED_ShowString(2, 2, " Image");
}
static void draw_angle(uint8_t row, uint8_t editing){
    (void)editing;
    OLED_Clear();
    OLED_ShowString(1, 1, "Angle");
    OLED_ShowString(2, 1, row==1?">":" "); OLED_ShowString(2, 2, " Angle");
}

/* 非阻塞菜单状态机：在主循环频繁调用 */
void Menu_ShowMain(void){
    // 页面：0=主菜单,1=LED,2=PID,3=Image,4=Angle
    static uint8_t page = 0;
    static uint8_t row_main = 1;
    static uint8_t row_led  = 1;
    static uint8_t row_pid  = 1;
    static uint8_t row_img  = 1;
    static uint8_t row_ang  = 1;
    static uint8_t editing  = 0;
    static uint8_t need_redraw = 1;

    if(need_redraw){
        switch(page){
            case 0: draw_main(row_main); break;
            case 1: draw_led(row_led, editing); break;
            case 2: draw_pid(row_pid, editing); break;
            case 3: draw_image(row_img, editing); break;
            case 4: draw_angle(row_ang, editing); break;
        }
        need_redraw = 0;
    }

    // 在 PID 编辑态时，随时读编码器增量（每脉冲步长 0.1）
    if(page==2 && editing){
        int d = Encoder_FetchDelta();
        if(d){
            double step = 0.1 * (double)d;
            if(row_pid==1) kp += step;
            else if(row_pid==2) ki += step;
            else kd += step;
            if(kp<0) kp=0; if(ki<0) ki=0; if(kd<0) kd=0;
            need_redraw = 1;
        }
    }

    int k = Key_GetNum();     // 0=无事件；1=down(下) 2=up(上) 3=enter 4=back
    if(k == 0) return;

    if(page == 0){ // 主菜单
        if(k==1){ if(++row_main>4) row_main=1; need_redraw=1; }
        else if(k==2){ if(--row_main<1) row_main=4; need_redraw=1; }
        else if(k==3){ page=row_main; editing=0; need_redraw=1; }
    }else if(page == 1){ // LED
        if(!editing){
            if(k==1){ if(++row_led>2) row_led=1; need_redraw=1; }
            else if(k==2){ if(--row_led<1) row_led=2; need_redraw=1; }
            else if(k==3){ editing=1; need_redraw=1; }
            else if(k==4){ page=0; need_redraw=1; }
        }else{
            if(row_led==1){ // LED_speed: 0->1->2->0
                if(k==1 || k==2){
                    Led_speed = (Led_speed + 1) % 3;
                    LED_SetSpeed(); // 即时生效
                    need_redraw = 1;
                }else if(k==3 || k==4){ editing=0; need_redraw=1; }
            }else{ // LED_dir: 0/1 切换
                if(k==1 || k==2){
                    dir = -dir;
                    need_redraw = 1;
                }else if(k==3 || k==4){ editing=0; need_redraw=1; }
            }
        }
    }else if(page == 2){ // PID
        if(!editing){
            if(k==1){ if(++row_pid>3) row_pid=1; need_redraw=1; }
            else if(k==2){ if(--row_pid<1) row_pid=3; need_redraw=1; }
            else if(k==3){ editing=1; need_redraw=1; }   // 进入编辑态，右上角 E
            else if(k==4){ page=0; need_redraw=1; }
        }else{
            // 编辑态：上下键步长 0.1（短按一次，长按1s后每100ms重复事件）
            double step = 0.0;
            if(k==1) step = +0.1;
            else if(k==2) step = -0.1;
            else if(k==3 || k==4){ editing=0; need_redraw=1; return; }

            if(step!=0.0){
                if(row_pid==1) kp += step;
                else if(row_pid==2) ki += step;
                else kd += step;
                if(kp<0) kp=0; if(ki<0) ki=0; if(kd<0) kd=0;
                need_redraw = 1;
            }
        }
    }else if(page == 3){ // Image
        if(k==3 || k==4){ page=0; need_redraw=1; }
    }else if(page == 4){ // Angle
        if(k==3 || k==4){ page=0; need_redraw=1; }
    }
}