#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include "ConfigurationCluster.h"
#include "app_led.h"
#include "zps_gen.h"

#ifdef DEBUG_BUTTON
#define TRACE_BUTTON TRUE
#else
#define TRACE_BUTTON FALSE
#endif

#define APP_BTN_TIMER_MSEC ZTIMER_TIME_MSEC(10)
#define APP_BTN_DEBOUNCE_MASK 0b0111
#define APP_BTN_REGISTER_WINDOW_CYCLES 40     // 40 * APP_BTN_TIMER_MSEC = 400ms
#define APP_BTN_LONG_PRESS_REGISTER_CYCLES 70 // 70 * APP_BTN_TIMER_MSEC = 700ms
#define APP_RESET_DEVICE_CYCLES 500           // 500 * APP_BTN_TIMER_MSEC = 5sec
#define APP_BTN_IDLE_CYCLES_MAX 100           // 100 * APP_BTN_TIMER_MSEC = 1sec

typedef enum
{
    IDLE = 0,
    SINGLE_CLICK,
    DOUBLE_CLICK,
    TRIPLE_CLICK,
    LONG_CLICK
} APP_teButtonState;

typedef enum
{
    BUTTON_TOGGLE_ACTION,
    BUTTON_MOMENTRAY_PRESSED_ACTION,
    BUTTON_MOMENTARY_RELEASED_ACTION,
    BUTTON_SINGLE_CLICK_ACTION,
    BUTTON_DOUBLE_CLICK_ACTION,
    BUTTON_TRIPLE_CLICK_ACTION,
    BUTTON_LONG_PRESSED_ACTION,
    BUTTON_LONG_RELEASED_ACTION,
    BUTTON_NONE_ACTION,
} teButtonAction;

typedef struct
{
    bool_t bPressed;          // pressed or released
    uint16 u16Cycles;         // how many cycles last state preserve
    uint8 u8Debounce;
} tsPressedState;

typedef struct
{
    const uint16 u16Endpoint;
    const te_BlinkMode eBlinkMode;
    const uint8 u8ButtonDIO;  // button DIO number
    APP_teButtonState eState; // machineState state
    tsPressedState sPressedState;
} tsButtonState;

typedef struct
{
    tsButtonState sBtn1State;
#ifdef TARGET_WXKG07LM
    tsButtonState sBtn2State;
#endif
} APP_tsButtonTracker;

PUBLIC void APP_vConfigureButtons(void);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);
PUBLIC teCLD_ButtonMode getButtonMode(void);
PUBLIC void setButtonMode(teCLD_ButtonMode mode);

#endif /* APP_BUTTON_H */
