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

#include <stdio.h>
#include <stdlib.h>

/* --- Driver Includes (Hardware Layer) --- */
#include "keypad.h"
#include "buttons_exti.h"
#include "display_7seg.h"
#include "led_status.h"

/* --- Logic Includes (Application Layer) --- */
#include "system_events.h"
#include "mealy_fsm.h"

/* ========================================================================== */
/*  Helper: Human-readable state name for console output                      */
/* ========================================================================== */

static const char* StateToString(FSM_State_t state) {
    switch (state) {
        case STATE_LOCKED:   return "LOCKED";
        case STATE_UNLOCKED: return "UNLOCKED";
        case STATE_ALARM:    return "ALARM";
        default:             return "UNKNOWN";
    }
}

static const char* EventToString(SystemEvent_t type) {
    switch (type) {
        case EVENT_NONE:            return "NONE";
        case EVENT_KEY_PRESSED:     return "KEY_PRESSED";
        case EVENT_DOORBELL:        return "DOORBELL";
        case EVENT_EMERGENCY_RESET: return "EMERGENCY_RESET";
        case EVENT_LOCK_CMD:        return "LOCK_CMD";
        default:                    return "UNKNOWN";
    }
}

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
/*  FSM Output Callback Adapters                                              */
/*  Each callback wraps the real driver call with a printf so we can see       */
/*  exactly what the FSM is commanding the hardware to do.                    */
/* ========================================================================== */

static void CB_UpdateProgress(uint8_t count) {
    printf("  [LED] Progress bar: ");
    for (int i = 0; i < 4; i++) {
        printf("%s", (i < count) ? "■ " : "□ ");
    }
    printf("(%d/%d)\n", count, 4);
    LED_SetProgress(count);
}

static void CB_SetSuccess(bool state) {
    printf("  [LED] Success LED: %s\n", state ? "ON ✓" : "OFF");
    LED_SetSuccess(state);
}

static void CB_SetAlarm(bool state) {
    printf("  [LED] Alarm LED:   %s\n", state ? "ON !! ALARM !!" : "OFF");
    LED_SetAlarm(state);
}

static void CB_UpdateFailedAttempts(uint8_t count, bool blank) {
    if (blank) {
        printf("  [7SEG] Display: OFF\n");
    } else {
        printf("  [7SEG] Failed attempts: %d\n", count);
    }
    Display_7Seg_ShowNumber(count, blank);
}

static void CB_PulseDoorbell(void) {
    printf("  [BELL] Doorbell pulsed! DING-DONG!\n");
    LED_SetSuccess(true);
    LED_SetSuccess(false);
}

/* ========================================================================== */
/*  Keypad Polling Helper                                                     */
/*  On PC: also handles 'D'/'d' for doorbell and 'R'/'r' for reset since     */
/*  we don't have physical EXTI buttons.                                      */
/* ========================================================================== */

static void PollKeypad(void) {
    char key = Keypad_GetScannedKey();

    if (key == KEYPAD_NO_KEY_PRESSED) {
        return;
    }

    /* Quit simulator */
    if (key == 'Q' || key == 'q') {
        printf("\n[SYS] Quitting simulator. Goodbye!\n");
        exit(0);
    }

    printf("\n>> Key pressed: '%c'\n", key);

    /*
     * On PC, simulate the two EXTI buttons via keyboard:
     *   'D' or 'd' → Doorbell button (EXTI)
     *   'R' or 'r' → Emergency Reset button (EXTI)
     */
    if (key == 'D' || key == 'd') {
        printf("   -> Simulating DOORBELL EXTI interrupt\n");
        OnDoorbellPressed();
        return;
    }
    if (key == 'R' || key == 'r') {
        printf("   -> Simulating EMERGENCY RESET EXTI interrupt\n");
        OnEmergencyResetPressed();
        return;
    }

    /*
     * Classify the key into an event type:
     *   '*' → EVENT_LOCK_CMD   (used in STATE_UNLOCKED to re-lock)
     *   Everything else → EVENT_KEY_PRESSED (sequence input)
     */
    if (key == '*') {
        printf("   -> Event: LOCK_CMD\n");
        SystemEvents_Push(EVENT_LOCK_CMD, key);
    } else {
        printf("   -> Event: KEY_PRESSED (payload='%c')\n", key);
        SystemEvents_Push(EVENT_KEY_PRESSED, key);
    }
}

/* ========================================================================== */
/*  Welcome Banner                                                            */
/* ========================================================================== */

static void PrintBanner(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║         SECURE KEYPAD - Project 2 Simulator         ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Password: 1 2 3 4  (4 digits)                     ║\n");
    printf("║                                                      ║\n");
    printf("║  Controls:                                           ║\n");
    printf("║    0-9, A-C, #  → Sequence key input                ║\n");
    printf("║    *            → Lock command (re-lock when open)  ║\n");
    printf("║    D            → Doorbell button (EXTI)            ║\n");
    printf("║    R            → Emergency Reset (EXTI, hi-prio)   ║\n");
    printf("║    Q            → Quit simulator                    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("\n");
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

    /* ---- 6. Print welcome banner ---- */
    PrintBanner();
    printf("[FSM] Initial state: %s\n", StateToString(MealyFSM_GetCurrentState()));
    printf("[SYS] Waiting for keypad input...\n\n");

    /* ---- 7. Super-loop ---- */
    while (1) {
        /* 7a. Poll the keypad for new key presses and enqueue events */
        PollKeypad();

        /* 7b. Drain the event queue and feed each event to the FSM */
        Event_t evt;
        while (SystemEvents_Pop(&evt)) {
            FSM_State_t old_state = MealyFSM_GetCurrentState();

            printf("[FSM] Processing event: %s", EventToString(evt.type));
            if (evt.payload != '\0') {
                printf(" (payload='%c')", evt.payload);
            }
            printf("  |  State: %s", StateToString(old_state));

            FSM_State_t new_state = MealyFSM_ProcessEvent(evt);

            if (new_state != old_state) {
                printf("\n[FSM] *** STATE TRANSITION: %s -> %s ***\n",
                       StateToString(old_state), StateToString(new_state));
            } else {
                printf("  ->  %s (no change)\n", StateToString(new_state));
            }
            printf("\n");
        }

        /* 7c. Check for quit key */
        /* (handled naturally — 'Q'/'q' is just an invalid key press,
         *  but we add an explicit exit here for the PC simulator) */
    }

    /* Never reached */
    return 0;
}
