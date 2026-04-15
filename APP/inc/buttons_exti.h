#ifndef BUTTONS_EXTI_H
#define BUTTONS_EXTI_H

#include <stdbool.h>

/* Callback function type for decoupled interrupt handling */
typedef void (*Button_Callback_t)(void);

/* Initializes EXTI pins for Doorbell and Reset buttons */
void Buttons_EXTI_Init(void);

/* Register callbacks so drivers remain decoupled from App logic */
void Buttons_EXTI_SetDoorbellCallback(Button_Callback_t cb);
void Buttons_EXTI_SetResetCallback(Button_Callback_t cb);

#endif // BUTTONS_EXTI_H
