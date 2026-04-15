#include "mealy_fsm.h"

#define MAX_PASSWORD_LENGTH 4
#define MAX_FAILED_ATTEMPTS 3

/* Target valid sequence */
static const char VALID_PASSWORD[MAX_PASSWORD_LENGTH] = {'1', '2', '3', '4'};

/* FSM Context variables */
static FSM_State_t CurrentState = STATE_LOCKED;
static char InputBuffer[MAX_PASSWORD_LENGTH];
static uint8_t InputCount = 0;
static uint8_t FailedAttempts = 0;

/* Registered output actions */
static FSM_Callbacks_t hw_outputs;

void MealyFSM_Init(FSM_Callbacks_t *callbacks) {
    if (callbacks) {
        hw_outputs = *callbacks; // Bind external hardware logic
    }

    CurrentState = STATE_LOCKED;
    InputCount = 0;
    FailedAttempts = 0;
    
    // Explicit initialization outputs
    if (hw_outputs.update_progress) hw_outputs.update_progress(0);
    if (hw_outputs.set_success) hw_outputs.set_success(false);
    if (hw_outputs.set_alarm) hw_outputs.set_alarm(false);
    if (hw_outputs.set_locked) hw_outputs.set_locked(true);
    if (hw_outputs.update_failed_attempts) hw_outputs.update_failed_attempts(FailedAttempts, false);
}

FSM_State_t MealyFSM_GetCurrentState(void) {
    return CurrentState;
}

FSM_State_t MealyFSM_ProcessEvent(Event_t event) {
    FSM_State_t NextState = CurrentState; // Default to remain in current state

    /*
     * EMERGENCY RESET — Global override, handled BEFORE the state switch.
     * This mirrors a high-priority EXTI interrupt: it forcefully yanks the
     * system out of ANY state (LOCKED, UNLOCKED, or ALARM), clears all
     * history (failed attempts, partial sequence), resets every indicator,
     * and transitions unconditionally to STATE_LOCKED.
     */
    if (event.type == EVENT_EMERGENCY_RESET) {
        FailedAttempts = 0;
        InputCount = 0;

        if (hw_outputs.set_alarm)             hw_outputs.set_alarm(false);
        if (hw_outputs.set_success)           hw_outputs.set_success(false);
        if (hw_outputs.update_progress)       hw_outputs.update_progress(0);
        if (hw_outputs.set_locked)            hw_outputs.set_locked(true);
        if (hw_outputs.update_failed_attempts) hw_outputs.update_failed_attempts(0, false);

        CurrentState = STATE_LOCKED;
        return STATE_LOCKED;
    }

    /*
     * DOORBELL EVENT — Global override, handled BEFORE the state switch.
     * Triggers the buzzer output action regardless of whether the system 
     * is in STATE_LOCKED, STATE_UNLOCKED, or STATE_ALARM. 
     */
    if (event.type == EVENT_DOORBELL) {
        // MEALY OUTPUT: Doorbell trigger in any state
        if (hw_outputs.pulse_doorbell) hw_outputs.pulse_doorbell();
        return CurrentState; // Doorbell does not cause a state transition
    }

    switch (CurrentState) {
        
        case STATE_LOCKED:
            if (event.type == EVENT_KEY_PRESSED) {
                // Buffer the new input
                InputBuffer[InputCount] = event.payload;
                InputCount++;
                
                // MEALY OUTPUT: Update progress LED immediately based on input
                if (hw_outputs.update_progress) hw_outputs.update_progress(InputCount);
                
                if (InputCount == MAX_PASSWORD_LENGTH) {
                    bool isValid = true;
                    for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
                        if (InputBuffer[i] != VALID_PASSWORD[i]) {
                            isValid = false;
                            break;
                        }
                    }
                    
                    if (isValid) {
                        // MEALY OUTPUT: Valid sequence triggers unlock setup
                        FailedAttempts = 0;
                        if (hw_outputs.update_progress) hw_outputs.update_progress(0);
                        if (hw_outputs.set_success) hw_outputs.set_success(true);
                        if (hw_outputs.set_locked) hw_outputs.set_locked(false);
                        if (hw_outputs.update_failed_attempts) hw_outputs.update_failed_attempts(FailedAttempts, false);
                        
                        NextState = STATE_UNLOCKED; // Transition
                    } else {
                        // MEALY OUTPUT: Invalid sequence actions
                        FailedAttempts++;
                        if (hw_outputs.update_progress) hw_outputs.update_progress(0);
                        if (hw_outputs.update_failed_attempts) hw_outputs.update_failed_attempts(FailedAttempts, false);
                        
                        if (FailedAttempts >= MAX_FAILED_ATTEMPTS) {
                            // MEALY OUTPUT: Max thresholds reached
                            if (hw_outputs.set_alarm) hw_outputs.set_alarm(true);
                            if (hw_outputs.set_locked) hw_outputs.set_locked(false);
                            NextState = STATE_ALARM; // Transition
                        }
                    }
                    InputCount = 0; // Prepare buffer for next attempts
                }
            } 
            break;
            
        case STATE_UNLOCKED:
            if (event.type == EVENT_LOCK_CMD) {
                // MEALY OUTPUT: Action to physical locked status
                if (hw_outputs.set_success) hw_outputs.set_success(false);
                if (hw_outputs.set_locked) hw_outputs.set_locked(true);
                if (hw_outputs.update_progress) hw_outputs.update_progress(0);
                InputCount = 0;
                
                NextState = STATE_LOCKED; // Transition
            }
            // Sequence keys are explicitly ignored
            break;
            
        case STATE_ALARM:
            // Only EVENT_EMERGENCY_RESET exits this state (handled above)
            // All other events explicitly ignored
            break;
            
        default:
            NextState = STATE_LOCKED; // System safefail
            break;
    }
    
    CurrentState = NextState; // Set state based on evaluated transition logic
    return NextState;
}
