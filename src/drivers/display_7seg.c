#include "display_7seg.h"

/* Pseudo-HAL Macros */
#define HAL_GPIO_WritePin(port, pin, state) ((void)0)
#define GPIO_PIN_SET   1
#define GPIO_PIN_RESET 0

/* 7-Segment common cathode encoding for digits 0-9 */
static const uint8_t Seg_Encoding[10] = {
    0x3F, // 0 = A,B,C,D,E,F
    0x06, // 1 = B,C
    0x5B, // 2 = A,B,D,E,G
    0x4F, // 3 = A,B,C,D,G
    0x66, // 4 = B,C,F,G
    0x6D, // 5 = A,C,D,F,G
    0x7D, // 6 = A,C,D,E,F,G
    0x07, // 7 = A,B,C
    0x7F, // 8 = ALL
    0x6F  // 9 = A,B,C,D,F,G
};

void Display_7Seg_Init(void) {
    /*
     * Pseudo-HAL Configuration
     * Configure 7 GPIO pins (e.g. PORTB 0-6 corresponding to segments A-G) as Outputs
     */
}

void Display_7Seg_ShowNumber(uint8_t number, bool blank) {
    if (blank || number > 9) {
        // Turn off all segments
        for (int i = 0; i < 7; i++) {
            HAL_GPIO_WritePin(/*PORT*/1, /*PIN*/i, GPIO_PIN_RESET);
        }
        return;
    }
    
    // Output code to GPIO pins
    uint8_t code = Seg_Encoding[number];
    (void)code; /* Suppress -Wunused-variable when using pseudo-HAL stubs */
    for (int i = 0; i < 7; i++) {
        HAL_GPIO_WritePin(/*PORT*/1, /*PIN*/i, (code & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
