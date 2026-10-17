/*
 * 8051 三顆 LED 輪流閃爍
 *
 *   LED1 (P1.0) 亮 0.5 秒 -> LED2 (P1.1) 亮 1 秒 -> LED3 (P1.2) 亮 2 秒 -> 重複
 *
 * 硬體假設：
 *   - 石英振盪器 12 MHz（1 個機器週期 = 1 us）
 *   - LED 為低電位點亮（P1.x -> 限流電阻 -> LED -> VCC），大多數實驗板都是這樣接
 *     若你的 LED 是高電位點亮，把 LED_ON / LED_OFF 對調即可
 *
 * 計時方式：Timer0 Mode 1（16 位元），每 50 ms 溢位一次，數 N 次得到所需時間
 *   50 ms = 50000 個機器週期 -> 初值 = 65536 - 50000 = 15536 = 0x3CB0
 *
 * 可用 Keil C51 或 SDCC 編譯：
 *   sdcc led_blink.c && packihx led_blink.ihx > led_blink.hex
 */

#ifdef __SDCC
#include <8051.h>
#define SBIT(name, addr) __sbit __at(addr) name
#else
#include <reg51.h>
#define SBIT(name, addr) sbit name = addr
#endif

SBIT(LED1, 0x90);   /* P1.0 */
SBIT(LED2, 0x91);   /* P1.1 */
SBIT(LED3, 0x92);   /* P1.2 */

#define LED_ON  0
#define LED_OFF 1

#define T0_RELOAD_H 0x3C   /* 65536 - 50000 = 0x3CB0 */
#define T0_RELOAD_L 0xB0

/* 延遲 ticks 個 50 ms */
static void delay_50ms(unsigned int ticks)
{
    while (ticks--) {
        TH0 = T0_RELOAD_H;
        TL0 = T0_RELOAD_L;
        TF0 = 0;
        TR0 = 1;
        while (!TF0)
            ;
        TR0 = 0;
    }
}

void main(void)
{
    TMOD = (TMOD & 0xF0) | 0x01;   /* Timer0 Mode 1，不動 Timer1 的設定 */

    LED1 = LED_OFF;
    LED2 = LED_OFF;
    LED3 = LED_OFF;

    while (1) {
        LED1 = LED_ON;
        delay_50ms(10);            /* 0.5 秒 */
        LED1 = LED_OFF;

        LED2 = LED_ON;
        delay_50ms(20);            /* 1 秒 */
        LED2 = LED_OFF;

        LED3 = LED_ON;
        delay_50ms(40);            /* 2 秒 */
        LED3 = LED_OFF;
    }
}
