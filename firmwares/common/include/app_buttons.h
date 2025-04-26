#ifndef APP_BUTTON_H
#define APP_BUTTON_H

// #include "app_led.h"
#include "zps_gen.h"
#include "device_config.h"

#ifdef DEBUG_BUTTON
#define TRACE_BUTTON TRUE
#else
#define TRACE_BUTTON FALSE
#endif

#ifndef APP_RESET_DEVICE_CYCLES
#define APP_RESET_DEVICE_CYCLES 500 // 500 * BUTTON_SCAN_TIME_MSEC = 5 sec
#endif

#define APP_BTN_DEBOUNCE_MASK 0b0111
#define APP_BTN_REGISTER_WINDOW_CYCLES 40     // 40 * BUTTON_SCAN_TIME_MSEC = 400ms
#define APP_BTN_LONG_PRESS_REGISTER_CYCLES 70 // 70 * BUTTON_SCAN_TIME_MSEC = 700ms
#define APP_BTN_IDLE_CYCLES_MAX 100           // 100 * BUTTON_SCAN_TIME_MSEC = 1sec

typedef enum
{
    IDLE = 0,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    LONG_CLICK
} APP_teButtonState;

typedef struct
{
    bool_t bPressed;
    uint16 u16Cycles;
    uint8 u8Debounce;
    APP_teButtonState eState;
    union
    {
        const tsEndpointButtonConfig *sEndpointBtnConfig;
        const tsSystemButtonConfig *sResetBtnConfig;
    } u;
} tsButtonState;

PUBLIC void APP_vConfigureButtons(void);

#endif /* APP_BUTTON_H */
