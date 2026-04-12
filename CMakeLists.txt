cmake_minimum_required(VERSION 3.10)
project(SecureKeypad C)

set(CMAKE_C_STANDARD 99)

# --- Include directory for all headers ---
include_directories(${CMAKE_SOURCE_DIR}/inc)

# --- Source files grouped by layer ---
set(DRIVER_SOURCES
    src/drivers/keypad.c
    src/drivers/buttons_exti.c
    src/drivers/display_7seg.c
    src/drivers/led_status.c
)

set(LOGIC_SOURCES
    src/logic/system_events.c
    src/logic/mealy_fsm.c
)

set(APP_SOURCES
    src/app/main.c
)

# --- Build the executable ---
add_executable(secure_keypad
    ${DRIVER_SOURCES}
    ${LOGIC_SOURCES}
    ${APP_SOURCES}
)

# --- Compiler warnings ---
target_compile_options(secure_keypad PRIVATE -Wall -Wextra -pedantic)
