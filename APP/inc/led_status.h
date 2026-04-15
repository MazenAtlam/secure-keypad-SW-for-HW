#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <stdint.h>
#include <stdbool.h>

void LED_Status_Init(void);

/* Sets the progress LED bar based on sequence count (e.g. 0 to 4) */
void LED_SetProgress(uint8_t count);

/* Sets the Success LED state */
void LED_SetSuccess(bool state);

/* Sets the Alarm LED state */
void LED_SetAlarm(bool state);

/* Sets the Locked LED state (Blue) */
void LED_SetLocked(bool state);

/* Hardware pulse for the doorbell buzzer */
void LED_PulseDoorbell(void);

#endif // LED_STATUS_H
