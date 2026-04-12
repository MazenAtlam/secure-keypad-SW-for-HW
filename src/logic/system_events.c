#include "system_events.h"

/* 
 * Pseudo-HAL macros for atomic operations (ISR-safe queue) 
 * In a real application (e.g. ARM Cortex-M), these would map to __disable_irq() and __enable_irq()
 */
#define DISABLE_INTERRUPTS()   ((void)0) 
#define ENABLE_INTERRUPTS()    ((void)0) 

/* Circular buffer size for the event queue */
#define EVENT_QUEUE_SIZE 16

static Event_t EventQueue[EVENT_QUEUE_SIZE];
static uint8_t head = 0;
static uint8_t tail = 0;

void SystemEvents_Init(void) {
    head = 0;
    tail = 0;
}

bool SystemEvents_Push(SystemEvent_t type, char payload) {
    bool success = false;
    
    // Critical Section Start
    DISABLE_INTERRUPTS();
    
    uint8_t next_head = (head + 1) % EVENT_QUEUE_SIZE;
    
    if (next_head != tail) { // Queue is not full
        EventQueue[head].type = type;
        EventQueue[head].payload = payload;
        head = next_head;
        success = true;
    }
    
    // Critical Section End
    ENABLE_INTERRUPTS();
    
    return success;
}

bool SystemEvents_Pop(Event_t *out_event) {
    bool has_event = false;
    
    // Critical Section Start
    DISABLE_INTERRUPTS();
    
    if (head != tail) { // Queue is not empty
        *out_event = EventQueue[tail];
        tail = (tail + 1) % EVENT_QUEUE_SIZE;
        has_event = true;
    } else {
        out_event->type = EVENT_NONE;
        out_event->payload = '\0';
    }
    
    // Critical Section End
    ENABLE_INTERRUPTS();
    
    return has_event;
}
