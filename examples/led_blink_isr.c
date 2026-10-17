/*
 * 8051 三顆 LED 各自獨立閃爍（同時進行）
 *
 *   LED1 (P1.0) 每 0.5 秒切換一次（亮 0.5 秒、滅 0.5 秒）
 *   LED2 (P1.1) 每 1   秒切換一次（亮 1 秒、  滅 1 秒）
 *   LED3 (P1.2) 每 2   秒切換一次（亮 2 秒、  滅 2 秒）
 *
 * 硬體假設：
 *   - 石英振盪器 12 MHz（1 個機器週期 = 1 us）
 *   - LED 為低電位點亮（P1.x -> 限流電阻 -> LED -> VCC），大多數實驗板都是這樣接
 *     閃爍只是切換狀態，高電位點亮的接法也一樣能用，只差開機時先亮還是先滅
 *
 * 計時方式：Timer0 Mode 1（16 位元）中斷，每 50 ms 進一次中斷
 *   50 ms = 50000 個機器週期 -> 初值 = 65536 - 50000 = 15536 = 0x3CB0
 *   每顆 LED 有自己的計數器，數到各自的次數就切換一次
 *
 * 可用 Keil C51 或 SDCC 編譯：
 *   sdcc led_blink.c && packihx led_blink.ihx > led_blink.hex
 */

#ifdef __SDCC
#include <8051.h>
#define SBIT(name, addr) __sbit __at(addr) name
#define ISR(n) __interrupt(n)
#else
#include <reg51.h>
#define SBIT(name, addr) sbit name = addr
#define ISR(n) interrupt n
#endif

SBIT(LED1, 0x90);   /* P1.0 */
SBIT(LED2, 0x91);   /* P1.1 */
SBIT(LED3, 0x92);   /* P1.2 */

#define LED_ON  0
#define LED_OFF 1

#define T0_RELOAD_H 0x3C   /* 65536 - 50000 = 0x3CB0 */
#define T0_RELOAD_L 0xB0

/* 每顆 LED 切換一次需要幾個 50 ms */
#define LED1_TICKS 10      /* 0.5 秒 */
#define LED2_TICKS 20      /* 1 秒 */
#define LED3_TICKS 40      /* 2 秒 */

static unsigned char cnt1, cnt2, cnt3;

/* Timer0 溢位中斷，每 50 ms 一次 */
void timer0_isr(void) ISR(1)
{
    TH0 = T0_RELOAD_H;
    TL0 = T0_RELOAD_L;

    if (++cnt1 >= LED1_TICKS) {
        cnt1 = 0;
        LED1 = !LED1;
    }
    if (++cnt2 >= LED2_TICKS) {
        cnt2 = 0;
        LED2 = !LED2;
    }
    if (++cnt3 >= LED3_TICKS) {
        cnt3 = 0;
        LED3 = !LED3;
    }
}

void main(void)
{
    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;

    TMOD = (TMOD & 0xF0) | 0x01;   /* Timer0 Mode 1，不動 Timer1 的設定 */
    TH0 = T0_RELOAD_H;
    TL0 = T0_RELOAD_L;
    ET0 = 1;                       /* 開 Timer0 中斷 */
    EA = 1;                        /* 開總中斷 */
    TR0 = 1;                       /* 啟動 Timer0 */

    while (1) {
        /* LED 全由中斷處理，主程式可以做其他事 */
    }
}
