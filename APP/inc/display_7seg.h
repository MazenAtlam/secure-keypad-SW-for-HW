#ifndef DISPLAY_7SEG_H
#define DISPLAY_7SEG_H

#include <stdint.h>
#include <stdbool.h>

/* Initializes 7-segment display GPIOs */
void Display_7Seg_Init(void);

/* 
 * Caches a number to be multiplexed onto the 7-segment display (0-99).
 * If blank == true, turns off all segments. 
 */
void Display_7Seg_ShowNumber(uint8_t number, bool blank);

/* Multiplexing engine to be called in the superloop */
void Display_7Seg_Update(void);

#endif // DISPLAY_7SEG_H
