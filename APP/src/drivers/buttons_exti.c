#include "buttons_exti.h"
#include <stddef.h>
#include "stm32f4xx_hal.h"

static Button_Callback_t DoorBell_Callback = NULL;
static Button_Callback_t Reset_Callback = NULL;

void Buttons_EXTI_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Doorbell EXTI: PA0 */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Emergency Reset EXTI: PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* NVIC Priorities: Emergency Reset (PC13) is higher priority than Doorbell (PA0) */
    
    /* EXTI line 0 (PA0) */
    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    
    /* EXTI line 15:10 (PC13) */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0); // Highest priority
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Buttons_EXTI_SetDoorbellCallback(Button_Callback_t cb) {
    DoorBell_Callback = cb;
}

void Buttons_EXTI_SetResetCallback(Button_Callback_t cb) {
    Reset_Callback = cb;
}

/* ISR Handlers */
void EXTI0_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

/* Callbacks requested by HAL */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_0) {
        if (DoorBell_Callback != NULL) {
            DoorBell_Callback();
        }
    } else if (GPIO_Pin == GPIO_PIN_13) {
        if (Reset_Callback != NULL) {
            Reset_Callback();
        }
    }
}
