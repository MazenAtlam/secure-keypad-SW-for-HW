#include "display_7seg.h"
#include "stm32f4xx_hal.h"

/* 7-Segment common cathode encoding for digits 0-9 and 'A' (10) */
static const uint8_t Seg_Encoding[11] = {
    0x3F, // 0 = A,B,C,D,E,F
    0x06, // 1 = B,C
    0x5B, // 2 = A,B,D,E,G
    0x4F, // 3 = A,B,C,D,G
    0x66, // 4 = B,C,F,G
    0x6D, // 5 = A,C,D,F,G
    0x7D, // 6 = A,C,D,E,F,G
    0x07, // 7 = A,B,C
    0x7F, // 8 = ALL
    0x6F, // 9 = A,B,C,D,F,G
    0x77  // 10 = A (A,B,C,E,F,G)
};

void Display_7Seg_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure PC0 - PC6 as Outputs */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Display_7Seg_ShowNumber(uint8_t number, bool blank) {
    if (blank || number > 10) {
        // Turn off all segments
        for (int i = 0; i < 7; i++) {
            HAL_GPIO_WritePin(GPIOC, (1 << i), GPIO_PIN_RESET);
        }
        return;
    }
    
    // Output code to GPIO pins
    uint8_t code = Seg_Encoding[number];
    for (int i = 0; i < 7; i++) {
        HAL_GPIO_WritePin(GPIOC, (1 << i), (code & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}
