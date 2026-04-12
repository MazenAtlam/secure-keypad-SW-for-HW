#include "keypad.h"
#include <stdbool.h>

/* --- Pseudo-HAL macros/functions --- */
#define HAL_GPIO_WritePin(port, pin, state)  ((void)0)
#define HAL_GPIO_ReadPin(port, pin)          (1) // assume unpressed (pull-up)
#define GPIO_PIN_SET                         1
#define GPIO_PIN_RESET                       0

static const char Keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void Keypad_Init(void) {
    /* 
     * Pseudo-HAL Initialization 
     * Configure Rows (e.g., Port A Pins 0-3) as Digital Outputs
     * Configure Cols (e.g., Port A Pins 4-7) as Digital Inputs with Pull-Up
     */
}

char Keypad_GetScannedKey(void) {
    // Simple polling matrix scan algorithm
    for (int row = 0; row < 4; row++) {
        // Set current row LOW, others HIGH
        for (int r = 0; r < 4; r++) {
            HAL_GPIO_WritePin(/*PORT*/0, r, (r == row) ? GPIO_PIN_RESET : GPIO_PIN_SET);
        }
        
        // Brief hardware delay for signal settling would go here
        
        // Read columns
        for (int col = 0; col < 4; col++) {
            if (HAL_GPIO_ReadPin(/*PORT*/0, col + 4) == GPIO_PIN_RESET) {
                // Key press detected
                // Simple debounce strategy: wait for release 
                while(HAL_GPIO_ReadPin(/*PORT*/0, col + 4) == GPIO_PIN_RESET);
                
                return Keymap[row][col];
            }
        }
    }
    return KEYPAD_NO_KEY_PRESSED;
}
