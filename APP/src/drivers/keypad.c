#include "keypad.h"
#include <stdbool.h>

/*
 * PC SIMULATION MODE:
 * Instead of scanning a physical 4x4 matrix via GPIO, we read from the
 * PC keyboard using _kbhit() / _getch() from <conio.h>.
 * This allows testing the full FSM logic on a desktop.
 *
 * On real hardware, replace this block with the GPIO scanning algorithm.
 */
#include <conio.h>

/* Keymap is not needed for PC simulation — we return the raw character */

void Keypad_Init(void) {
    /* No GPIO configuration needed for PC simulation */
}

char Keypad_GetScannedKey(void) {
    /*
     * Non-blocking check: _kbhit() returns non-zero if a key is waiting.
     * _getch() reads the key without echoing it to the console.
     * This mirrors the non-blocking behavior of a real keypad scan.
     */
    if (_kbhit()) {
        return (char)_getch();
    }
    return KEYPAD_NO_KEY_PRESSED;
}
