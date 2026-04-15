#ifndef DISPLAY_7SEG_H
#define DISPLAY_7SEG_H

#include <stdint.h>
#include <stdbool.h>

/* Initializes 7-segment display GPIOs */
void Display_7Seg_Init(void);

/* 
 * Displays a number on the 7-segment display (0-10).
 * 10 is displayed as the hexadecimal character 'A'.
 * If blank == true, turns off all segments. 
 */
void Display_7Seg_ShowNumber(uint8_t number, bool blank);

#endif // DISPLAY_7SEG_H
