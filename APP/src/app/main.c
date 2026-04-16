/*
 * main.c — Application Integration (Secure Keypad Project 2)
 * 
 * PURPOSE:
 *   This is the ONLY file that bridges the hardware driver layer and the
 *   pure logic layer. It initializes all peripherals, wires driver functions
 *   into the FSM callback structure, registers ISR event callbacks, and runs
 *   the infinite super-loop that fetches events and feeds them to the Mealy FSM.
 *
 * ARCHITECTURE:
 *   Drivers (src/drivers)  -->  Events (src/logic/system_events)  -->  FSM (src/logic/mealy_fsm)
 *        ^                                                                    |
 *        |______________ FSM_Callbacks_t (output actions) ____________________|
 *
 *   main.c is the glue that connects these layers at initialization time.
 */

#include "stm32f4xx_hal.h"

/* --- Driver Includes (Hardware Layer) --- */
#include "keypad.h"
#include "buttons_exti.h"
#include "display_7seg.h"
#include "led_status.h"

/* --- Logic Includes (Application Layer) --- */
#include "system_events.h"
#include "mealy_fsm.h"

/* ========================================================================== */
/*  EXTI Callback Adapters                                                    */
/* ========================================================================== */

static void OnDoorbellPressed(void) {
    SystemEvents_Push(EVENT_DOORBELL, '\0');
}

static void OnEmergencyResetPressed(void) {
    SystemEvents_Push(EVENT_EMERGENCY_RESET, '\0');
}

/* ========================================================================== */
/*  main()                                                                    */
/* ========================================================================== */

int main(void) {
    /* ---- 1. Initialize all hardware peripherals ---- */
    Keypad_Init();
    Buttons_EXTI_Init();
    Display_7Seg_Init();
    LED_Status_Init();

    /* ---- 3. Initialize the event queue ---- */
    SystemEvents_Init();

    /* ---- 4. Register EXTI callbacks (ISR → Event Queue bridge) ---- */
    Buttons_EXTI_SetDoorbellCallback(OnDoorbellPressed);
    Buttons_EXTI_SetResetCallback(OnEmergencyResetPressed);

    /* ---- 5. Build the FSM callback table (FSM → Driver bridge) ---- */
    FSM_Callbacks_t fsm_hw = {
        .update_progress       = LED_SetProgress,
        .set_success           = LED_SetSuccess,
        .set_alarm             = LED_SetAlarm,
        .set_locked            = LED_SetLocked,
        .update_failed_attempts = Display_7Seg_ShowNumber,
        .pulse_doorbell        = LED_PulseDoorbell
    };

    /* ---- 6. Initialize the Mealy FSM (defaults to STATE_LOCKED) ---- */
    MealyFSM_Init(&fsm_hw);

    /* ---- 7. Super-loop ---- */
    while (1) {
        /* 7a. Poll the keypad for new key presses and enqueue events */
        char key = Keypad_GetScannedKey();
        if (key != KEYPAD_NO_KEY_PRESSED) {
            /* Classify the key into an event type */
            if (key == '*') {
                /* FIX: Push specific lock command */
                SystemEvents_Push(EVENT_LOCK_CMD, key);
            } else if (key == '#') {
                /* FIX: Push specific clear command */
                SystemEvents_Push(EVENT_CLEAR_CMD, key);
            } else {
                /* Only standard sequence digits (0-9) pass here */
                SystemEvents_Push(EVENT_KEY_PRESSED, key);
            }
        }

        /* 7b. Drain the event queue and feed each event to the FSM */
        Event_t evt;
        while (SystemEvents_Pop(&evt)) {
            MealyFSM_ProcessEvent(evt);
        }

        /* 7c. Update non-blocking drivers */
        LED_Doorbell_Update();
        Display_7Seg_Update();
    }

    /* Never reached */
    return 0;
}
