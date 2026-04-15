#include "keypad.h"
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/* Keymap for 4x4 Keypad */
static const char Keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* Rows: PD5, PD6, PD7, PD8 */
static const uint16_t ROW_PINS[4] = {GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8};
/* Columns: PD0, PD1, PD2, PD3 */
static const uint16_t COL_PINS[4] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};

void Keypad_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Configure Rows (PD5-PD8) as Outputs */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure Columns (PD0-PD3) as Inputs with Pull-Up */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Set all rows HIGH initially (idle state) */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_SET);
}

char Keypad_GetScannedKey(void) {
    for (int r = 0; r < 4; r++) {
        /* Drive current row LOW */
        HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_RESET);
        
        /* Check all columns */
        for (int c = 0; c < 4; c++) {
            if (HAL_GPIO_ReadPin(GPIOD, COL_PINS[c]) == GPIO_PIN_RESET) {
                /* Wait for key release to prevent multiple events for one press */
                while(HAL_GPIO_ReadPin(GPIOD, COL_PINS[c]) == GPIO_PIN_RESET);
                
                /* Restore row HIGH */
                HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_SET);
                return Keymap[r][c];
            }
        }
        
        /* Restore row HIGH */
        HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_SET);
    }
    return KEYPAD_NO_KEY_PRESSED;
}
