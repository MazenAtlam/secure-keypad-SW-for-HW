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
/*  These short functions are registered with the EXTI driver so that when    */
/*  an interrupt fires, it pushes an event into the queue. This is the bridge */
/*  between the hardware ISR world and the event-driven logic world.          */
/* ========================================================================== */

static void OnDoorbellPressed(void) {
    /* ISR context: push event and return immediately */
    SystemEvents_Push(EVENT_DOORBELL, '\0');
}

static void OnEmergencyResetPressed(void) {
    /* ISR context: push event and return immediately (highest priority) */
    SystemEvents_Push(EVENT_EMERGENCY_RESET, '\0');
}

/* ========================================================================== */
/*  FSM Output Callback Adapters                                              */
/*  These functions map FSM output requests to actual hardware driver calls.  */
/*  The FSM itself has zero knowledge of which driver exists — it only calls  */
/*  function pointers. This is where we bind them.                            */
/* ========================================================================== */

static void CB_UpdateProgress(uint8_t count) {
    LED_SetProgress(count);
}

static void CB_SetSuccess(bool state) {
    LED_SetSuccess(state);
}

static void CB_SetAlarm(bool state) {
    LED_SetAlarm(state);
}

static void CB_UpdateFailedAttempts(uint8_t count, bool blank) {
    Display_7Seg_ShowNumber(count, blank);
}

static void CB_PulseDoorbell(void) {
    /*
     * Briefly pulse a doorbell indicator (e.g., flash an LED or buzzer).
     * For now we reuse the Success LED as a quick visual pulse.
     * On real hardware this would toggle a buzzer GPIO.
     */
    LED_SetSuccess(true);
    /* A short software delay would go here for visibility */
    LED_SetSuccess(false);
}

/* ========================================================================== */
/*  Keypad Polling Helper                                                     */
/*  Scans the keypad once per loop iteration and translates the raw key       */
/*  press into the appropriate system event before pushing it to the queue.   */
/* ========================================================================== */

static void PollKeypad(void) {
    char key = Keypad_GetScannedKey();

    if (key == KEYPAD_NO_KEY_PRESSED) {
        return; /* No key — nothing to do */
    }

    /*
     * Classify the key into an event type:
     *   '*' → EVENT_LOCK_CMD   (used in STATE_UNLOCKED to re-lock)
     *   '0'-'9', 'A'-'D', '#' → EVENT_KEY_PRESSED (sequence input)
     */
    if (key == '*') {
        SystemEvents_Push(EVENT_LOCK_CMD, key);
    } else {
        SystemEvents_Push(EVENT_KEY_PRESSED, key);
    }
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

    /* ---- 2. Initialize the event queue ---- */
    SystemEvents_Init();

    /* ---- 3. Register EXTI callbacks (ISR → Event Queue bridge) ---- */
    Buttons_EXTI_SetDoorbellCallback(OnDoorbellPressed);
    Buttons_EXTI_SetResetCallback(OnEmergencyResetPressed);

    /* ---- 4. Build the FSM callback table (FSM → Driver bridge) ---- */
    FSM_Callbacks_t fsm_hw = {
        .update_progress       = CB_UpdateProgress,
        .set_success           = CB_SetSuccess,
        .set_alarm             = CB_SetAlarm,
        .update_failed_attempts = CB_UpdateFailedAttempts,
        .pulse_doorbell        = CB_PulseDoorbell
    };

    /* ---- 5. Initialize the Mealy FSM (defaults to STATE_LOCKED) ---- */
    MealyFSM_Init(&fsm_hw);

    /* ---- 6. Super-loop ---- */
    while (1) {
        /* 6a. Poll the keypad for new key presses and enqueue events */
        PollKeypad();

        /* 6b. Drain the event queue and feed each event to the FSM */
        Event_t evt;
        while (SystemEvents_Pop(&evt)) {
            MealyFSM_ProcessEvent(evt);
        }

        /*
         * 6c. Optional: enter a low-power wait here (e.g., __WFI() on ARM)
         *     to reduce power consumption between polling cycles.
         */
    }

    /* Never reached */
    return 0;
}
