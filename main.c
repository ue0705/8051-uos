/*
 * 三顆 LED 各是一個 task，各自用 os_delay() 延遲：
 *
 *   task_led1：LED1 (P1.0) 每 0.5 秒切換一次
 *   task_led2：LED2 (P1.1) 每 1   秒切換一次
 *   task_led3：LED3 (P1.2) 每 2   秒切換一次
 *
 * LED 為低電位點亮（P1.x -> 限流電阻 -> LED -> VCC）。
 */
#include "uos.h"

SBIT(LED1, 0x90);   /* P1.0 */
SBIT(LED2, 0x91);   /* P1.1 */
SBIT(LED3, 0x92);   /* P1.2 */

static void task_led1(void)
{
    while (1) {
        LED1 = !LED1;
        os_delay(OS_MS(500));
    }
}

static void task_led2(void)
{
    while (1) {
        LED2 = !LED2;
        os_delay(OS_MS(1000));
    }
}

static void task_led3(void)
{
    while (1) {
        LED3 = !LED3;
        os_delay(OS_MS(2000));
    }
}

void main(void)
{
    os_init();
    os_task_create(task_led1);
    os_task_create(task_led2);
    os_task_create(task_led3);
    os_start();
}
