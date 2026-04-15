#ifndef MEALY_FSM_H
#define MEALY_FSM_H

#include "system_events.h"
#include <stdint.h>
#include <stdbool.h>

/* States for the Secure Keypad */
typedef enum {
    STATE_LOCKED,
    STATE_UNLOCKED,
    STATE_ALARM
} FSM_State_t;

/* 
 * Callback interface to strictly decouple the pure FSM logic 
 * from the hardware peripheral drivers (Singularity Principle).
 */
typedef struct {
    void (*update_progress)(uint8_t count);
    void (*set_success)(bool state);
    void (*set_alarm)(bool state);
    void (*update_failed_attempts)(uint8_t count, bool blank);
    void (*pulse_doorbell)(void);
} FSM_Callbacks_t;

/* Initializes the hardware callbacks and sets FSM to default STATE_LOCKED */
void MealyFSM_Init(FSM_Callbacks_t *callbacks);

/* 
 * Processes a single event against the CurrentState.
 * Triggered outputs and NextState are strictly evaluated (Mealy mechanism).
 * Returns the NextState.
 */
FSM_State_t MealyFSM_ProcessEvent(Event_t event);

/* Returns the current FSM state without triggering transitions */
FSM_State_t MealyFSM_GetCurrentState(void);

#endif // MEALY_FSM_H
