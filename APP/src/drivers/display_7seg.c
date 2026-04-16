#include "display_7seg.h"
#include "stm32f4xx_hal.h"

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

static uint8_t current_number = 0;
static bool is_blank = false;

void Display_7Seg_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure PC0 - PC8 as Outputs */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |
                          GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Display_7Seg_ShowNumber(uint8_t number, bool blank) {
    /* Cache the state for the multiplexing engine */
    current_number = number;
    is_blank = blank;
}

static void Output_Segments(uint8_t digit_value) {
    uint8_t code = Seg_Encoding[digit_value];
    for (int i = 0; i < 7; i++) {
        HAL_GPIO_WritePin(GPIOC, (1 << i), (code & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void Display_7Seg_Update(void) {
    if (is_blank) {
        // Turn off all segments and digit enablers
        for (int i = 0; i < 7; i++) {
            HAL_GPIO_WritePin(GPIOC, (1 << i), GPIO_PIN_RESET);
        }
        /* FIX: Common Cathode - SET common pins to HIGH to turn OFF */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_SET);
        return;
    }

    uint8_t tens = (current_number / 10) % 10;
    uint8_t units = current_number % 10;

    /* 1 & 2. Turn OFF both digit enable pins to prevent ghosting (SET = OFF) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_SET);
    
    /* 3. Output the segment code for the Tens digit to PC0-PC6 */
    Output_Segments(tens);
    
    /* 4. Turn ON the Digit 1 enable pin (PC7) (RESET = ON) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    
    /* 5. Brief software delay loop */
    for (volatile uint32_t i = 0; i < 500; i++);
    
    /* 6. Turn OFF Digit 1 (PC7) (SET = OFF) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
    
    /* 7. Output the segment code for the Units digit to PC0-PC6 */
    Output_Segments(units);
    
    /* 8. Turn ON the Digit 2 enable pin (PC8) (RESET = ON) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
    
    /* 9. Brief software delay loop */
    for (volatile uint32_t i = 0; i < 500; i++);
    
    /* 10. Turn OFF Digit 2 (PC8) (SET = OFF) */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
}
