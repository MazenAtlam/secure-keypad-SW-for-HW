#include "led_status.h"

/* Pseudo-HAL Macros */
#define HAL_GPIO_WritePin(port, pin, state) ((void)0)
#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

/* Pin Definitions */
#define PIN_SUCCESS 0
#define PIN_ALARM   1
#define PROG_PIN_START 2
#define MAX_PROGRESS 4

void LED_Status_Init(void) {
    /* 
     * Pseudo-HAL Configuration
     * Configure GPIO pins for Success, Alarm, and Progress LEDs as Output
     */
}

void LED_SetProgress(uint8_t count) {
    // Limits
    if (count > MAX_PROGRESS) {
         count = MAX_PROGRESS;
    }
    
    // Light up 'count' number of LEDs consecutively
    for (int i = 0; i < MAX_PROGRESS; i++) {
        HAL_GPIO_WritePin(/*PORT*/2, PROG_PIN_START + i, (i < count) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void LED_SetSuccess(bool state) {
    HAL_GPIO_WritePin(/*PORT*/2, PIN_SUCCESS, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_SetAlarm(bool state) {
    HAL_GPIO_WritePin(/*PORT*/2, PIN_ALARM, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
