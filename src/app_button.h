#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "ButtonMode.h"

#ifdef DEBUG_BUTTON
#define TRACE_BUTTON TRUE
#else
#define TRACE_BUTTON FALSE
#endif

#define APP_BTN_LEFT_DIO (13)
#define APP_BTN_LEFT_MASK (1 << APP_BTN_LEFT_DIO)
#define APP_BTN_RIGHT_DIO (17)
#define APP_BTN_RIGHT_MASK (1 << APP_BTN_RIGHT_DIO)
#define APP_BTN_CTRL_MASK (APP_BTN_LEFT_MASK | APP_BTN_RIGHT_MASK)
#define APP_BTN_TIMER_MSEC ZTIMER_TIME_MSEC(10)
#define APP_BTN_DEBOUNCE_MASK 0b0111
#define APP_BTN_REGISTER_WINDOW_CYCLES 40     // 40 * APP_BTN_TIMER_MSEC = 400ms
#define APP_BTN_BOTH_REGISTER_CYCLES 10       // 100 * APP_BTN_TIMER_MSEC = 100ms
#define APP_BTN_LONG_PRESS_REGISTER_CYCLES 70 // 70 * APP_BTN_TIMER_MSEC = 700ms
#define APP_RESET_NETWORK_CYCLES 1000         // 1000 * APP_BTN_TIMER_MSEC = 10sec
#define APP_BTN_IDLE_CYCLES_MAX 100           // 100 * APP_BTN_TIMER_MSEC = 1sec

#define PDM_ID_BUTTON_MODE 0x9

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
    const uint8 u8ButtonDIO;  // button DIO number
    APP_teButtonState eState; // machineState state
    bool_t bPresssed;         // pressed or released
    uint16 u16Cycles;         // how many cycles last state preserve
    uint8 u8Debounce;
} APP_tsButtonState;

typedef struct
{
    APP_tsButtonState sLeftState;
    APP_tsButtonState sRightState;
    bool_t bBothPressedMode;
} APP_tsButtonTracker;

PUBLIC void APP_vConfigureButtons(void);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);
PUBLIC teCLD_ButtonMode getButtonMode(void);
PUBLIC void setButtonMode(teCLD_ButtonMode mode);
PUBLIC void resetButtonMode(void);

#endif /* APP_BUTTON_H */
