#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

#define KEYPAD_NO_KEY_PRESSED '\0'

/* Initializes the keypad GPIOs */
void Keypad_Init(void);

/* Returns the currently pressed key (e.g., '1', 'A') or KEYPAD_NO_KEY_PRESSED */
char Keypad_GetScannedKey(void);

#endif // KEYPAD_H
