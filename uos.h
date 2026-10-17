/*
 * uOS - 8051 小型協同式多工核心
 *
 * 單一 CPU、單執行緒，但每個 task 有自己的堆疊，可以各自寫成 while(1)。
 * task 呼叫 os_delay() 時會把 CPU 讓給其他 task，等時間到再回來繼續執行。
 *
 * 硬體假設：石英振盪器 12 MHz，系統 tick 使用 Timer0。
 * 可用 Keil C51 或 SDCC 編譯。
 */
#ifndef UOS_H
#define UOS_H

#ifdef __SDCC
#include <8051.h>
#define SBIT(name, addr) __sbit __at(addr) name
#define IDATA __idata
#define ISR(n, bank) __interrupt(n) __using(bank)
#else
#include <reg51.h>
#define SBIT(name, addr) sbit name = addr
#define IDATA idata
#define ISR(n, bank) interrupt n using bank
#endif

#define OS_MAX_TASKS  3     /* 最多幾個 task */
#define OS_STACK_SIZE 16    /* 每個 task 的堆疊大小（bytes，放在 idata） */
#define OS_TICK_MS    10    /* 系統 tick 週期 */

/* 毫秒換算成 tick 數，例如 os_delay(OS_MS(500)) */
#define OS_MS(ms) ((unsigned int)((ms) / OS_TICK_MS))

typedef void (*os_task_t)(void);

void os_init(void);
void os_task_create(os_task_t task);
void os_start(void);                /* 開始排程，不會返回 */
void os_delay(unsigned int ticks);  /* 讓出 CPU，ticks 個 tick 後再回來 */
void os_yield(void);                /* 讓出 CPU，有機會就馬上回來 */

/* SDCC 要求中斷函式的宣告出現在 main() 所在的檔案，所以放在這裡 */
void os_tick_isr(void) ISR(1, 1);

#endif
