#include "keypad.h"
#include <stdbool.h>
#include "stm32f4xx_hal.h"

/* Keymap for 4x3 Keypad */
static const char Keymap[4][3] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

/* Rows: PD5, PD6, PD7, PD8 */
static const uint16_t ROW_PINS[4] = {GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8};
/* Columns: Flipped Y-Axis software mapping (PD2, PD1, PD0) to match hardware wiring inversions */
static const uint16_t COL_PINS[3] = {GPIO_PIN_2, GPIO_PIN_1, GPIO_PIN_0};

/* State tracking for edge detection (true = currently held down, false = released) */
static bool key_states[4][3] = {0};

void Keypad_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Configure Rows (PD5-PD8) as Outputs */
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Configure Columns (PD0-PD2) as Inputs with Pull-Up */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Set all rows HIGH initially (idle state) */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_SET);
}

char Keypad_GetScannedKey(void) {
    char pressed_key = KEYPAD_NO_KEY_PRESSED;
    
    for (int r = 0; r < 4; r++) {
        /* Drive current row LOW */
        HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_RESET);
        
        /* FIX: Capacitive smearing delay to let the voltage physically settle LOW */
        for (volatile int delay = 0; delay < 200; delay++);
        
        /* Check all columns */
        for (int c = 0; c < 3; c++) {
            bool is_pressed = (HAL_GPIO_ReadPin(GPIOD, COL_PINS[c]) == GPIO_PIN_RESET);
            
            if (is_pressed && !key_states[r][c]) {
                /* Falling edge detected: Key just pressed */
                pressed_key = Keymap[r][c];
                key_states[r][c] = true;
                /* FIX: Short-circuit. Restore row and return immediately to prevent Shadow Presses */
                HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_SET);
                return pressed_key;
            } else if (!is_pressed && key_states[r][c]) {
                /* Rising edge detected: Key released */
                key_states[r][c] = false;
            }
        }
        
        /* Restore row HIGH */
        HAL_GPIO_WritePin(GPIOD, ROW_PINS[r], GPIO_PIN_SET);
    }
    
    return pressed_key; 
}
