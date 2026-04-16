#include "led_status.h"
#include "stm32f4xx_hal.h"

#define MAX_PROGRESS 4

void LED_Status_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* PA4 (Doorbell LED migrated from PA2 Buzzer) */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB0 (Locked), PB1 (Success), PB2 (Alarm) */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PD13, PD12, PD11, PD10 (Progress LEDs) */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void LED_SetProgress(uint8_t count) {
    if (count > MAX_PROGRESS) {
         count = MAX_PROGRESS;
    }
    
    // Progress LEDs: PD13 (Seq 1), PD12 (Seq 2), PD11 (Seq 3), PD10 (Seq 4)
    uint16_t prog_pins[4] = {GPIO_PIN_13, GPIO_PIN_12, GPIO_PIN_11, GPIO_PIN_10};
    
    for (int i = 0; i < MAX_PROGRESS; i++) {
        HAL_GPIO_WritePin(GPIOD, prog_pins[i], (i < count) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void LED_SetSuccess(bool state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_SetAlarm(bool state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_SetLocked(bool state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint32_t doorbell_counter = 0;
static bool doorbell_active = false;

void LED_PulseDoorbell(void) {
    /* Trigger the Doorbell LED (PA4) immediately and set threshold counter */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    /* FIX: Massively increased threshold to bridge gap in superloop pacing */
    doorbell_counter = 1000000;
    doorbell_active = true;
}

void LED_Doorbell_Update(void) {
    /* Non-blocking decrement checking for doorbell timeout */
    if (doorbell_active) {
        if (doorbell_counter > 0) {
            doorbell_counter--;
        }
        
        if (doorbell_counter == 0) {
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
            doorbell_active = false;
        }
    }
}
