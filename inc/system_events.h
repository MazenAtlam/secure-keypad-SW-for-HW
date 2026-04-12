#ifndef SYSTEM_EVENTS_H
#define SYSTEM_EVENTS_H

#include <stdint.h>
#include <stdbool.h>

/* Enumeration of all possible system events */
typedef enum {
    EVENT_NONE = 0,
    EVENT_KEY_PRESSED,     // Indicates a sequence digit was pressed
    EVENT_DOORBELL,        // Triggered by the doorbell EXTI
    EVENT_EMERGENCY_RESET, // Triggered by the emergency reset EXTI
    EVENT_LOCK_CMD         // Triggered when a specific lock key (e.g. '*') is pressed
} SystemEvent_t;

/* Event structure containing the event type and optional auxiliary data */
typedef struct {
    SystemEvent_t type;
    char payload;          // Useful for EVENT_KEY_PRESSED to hold the actual key character
} Event_t;

/* Initializes the event queue mechanism */
void SystemEvents_Init(void);

/* 
 * Pushes an event into the queue. 
 * Thread-safe / ISR-safe.
 * Returns true if successful, false if the queue is full.
 */
bool SystemEvents_Push(SystemEvent_t type, char payload);

/* 
 * Pops an event from the queue.
 * Thread-safe / ISR-safe.
 * Returns true if an event was fetched, false if the queue is empty.
 */
bool SystemEvents_Pop(Event_t *out_event);

#endif // SYSTEM_EVENTS_H
