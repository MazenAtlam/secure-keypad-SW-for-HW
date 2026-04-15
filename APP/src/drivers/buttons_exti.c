#include "buttons_exti.h"
#include <stddef.h>

/* --- Pseudo-HAL for Interrupts --- */
#define HAL_EXTI_ClearFlag(pin) ((void)0)
#define PIN_DOORBELL 0
#define PIN_RESET    1

static Button_Callback_t DoorBell_Callback = NULL;
static Button_Callback_t Reset_Callback = NULL;

void Buttons_EXTI_Init(void) {
    /* 
     * Pseudo-HAL Configuration 
     * - Configure Doorbell pin as EXTI input (falling edge trigger)
     * - Configure Emergency Reset pin as EXTI input (falling edge trigger)
     * - Assign IRQ priorities giving Emergency Reset a higher priority
     */
}

void Buttons_EXTI_SetDoorbellCallback(Button_Callback_t cb) {
    DoorBell_Callback = cb;
}

void Buttons_EXTI_SetResetCallback(Button_Callback_t cb) {
    Reset_Callback = cb;
}

/* Pseudo-ISR for Doorbell */
void EXTI_DoorBell_IRQHandler(void) {
    HAL_EXTI_ClearFlag(PIN_DOORBELL);
    
    if (DoorBell_Callback != NULL) {
        DoorBell_Callback();
    }
}

/* Pseudo-ISR for Emergency Reset */
void EXTI_Reset_IRQHandler(void) {
    HAL_EXTI_ClearFlag(PIN_RESET);
    
    if (Reset_Callback != NULL) {
        Reset_Callback();
    }
}
