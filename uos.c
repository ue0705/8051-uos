/*
 * uOS 核心實作
 *
 * 切換 task 的原理：
 *   8051 的 CALL 會把返回位址推進堆疊，RET 再從 SP 指的地方取回來。
 *   每個 task 有自己的一塊堆疊，os_switch() 先記下目前 task 的 SP，
 *   再把 SP 換成下一個 task 的 SP，最後的 RET 就會回到下一個 task
 *   上次呼叫 os_delay() 的地方繼續執行。
 *
 *   新建立的 task 堆疊底部預先放好 task 函式的位址，第一次被切換到時，
 *   RET 就會直接跳進 task 的開頭。
 *
 * 使用規則：
 *   1. task 裡要在 os_delay() 前後都保留的變數，請宣告成 static 或全域變數。
 *      8051 編譯器會把一般區域變數放在共用（overlay）的記憶體或暫存器，
 *      切換 task 後可能被別的 task 改掉。
 *   2. 會呼叫 os_delay() 的函式不要被兩個 task 共用（C51 函式預設不可重入）。
 *   3. task 不可以 return。
 */
#include "uos.h"

#define T0_RELOAD (65536 - OS_TICK_MS * 1000)   /* 12 MHz：1 個機器週期 = 1 us */

static unsigned char IDATA task_stack[OS_MAX_TASKS][OS_STACK_SIZE];
static unsigned char task_sp[OS_MAX_TASKS];
static volatile unsigned int task_delay[OS_MAX_TASKS];
static unsigned char task_count;
static unsigned char cur;

void os_init(void)
{
    task_count = 0;

    TMOD = (TMOD & 0xF0) | 0x01;   /* Timer0 Mode 1，不動 Timer1 的設定 */
    TH0 = T0_RELOAD >> 8;
    TL0 = T0_RELOAD & 0xFF;
}

void os_task_create(os_task_t task)
{
    unsigned int addr = (unsigned int)task;
    unsigned char IDATA *stk;

    if (task_count >= OS_MAX_TASKS)
        return;

    /* 堆疊往上長；RET 先取高位元組再取低位元組，所以低位元組放下面 */
    stk = task_stack[task_count];
    stk[0] = addr & 0xFF;
    stk[1] = addr >> 8;
    task_sp[task_count] = (unsigned char)&stk[1];
    task_delay[task_count] = 0;
    task_count++;
}

/*
 * 切到下一個可以執行的 task。
 * 從記下 SP 到換掉 SP 之間不能有任何函式呼叫或 push，
 * 所以這裡只寫最簡單的迴圈，也沒有區域變數。
 */
static void os_switch(void)
{
    task_sp[cur] = SP;

    for (;;) {
        if (++cur >= task_count)
            cur = 0;
        EA = 0;                     /* task_delay 是 16 位元，讀取時先擋住中斷 */
        if (task_delay[cur] == 0) {
            EA = 1;
            break;
        }
        EA = 1;
    }

    SP = task_sp[cur];
}   /* 這裡的 RET 會回到新 task */

void os_delay(unsigned int ticks)
{
    EA = 0;
    task_delay[cur] = ticks;
    EA = 1;
    os_switch();
}

void os_yield(void)
{
    os_switch();
}

void os_start(void)
{
    cur = 0;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
    SP = task_sp[0];
}   /* 這裡的 RET 會跳進第一個 task，main 的堆疊從此不再使用 */

/* 系統 tick：每 OS_TICK_MS 進來一次，把所有 task 的等待時間減 1 */
void os_tick_isr(void) ISR(1, 1)
{
    unsigned char i;

    TH0 = T0_RELOAD >> 8;
    TL0 = T0_RELOAD & 0xFF;

    for (i = 0; i < task_count; i++) {
        if (task_delay[i])
            task_delay[i]--;
    }
}
