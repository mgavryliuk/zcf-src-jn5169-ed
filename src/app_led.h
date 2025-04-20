#ifndef APP_LED_H
#define APP_LED_H

#include "ZTimer.h"

#ifdef DEBUG_LED
#define TRACE_LED TRUE
#else
#define TRACE_LED FALSE
#endif

#define APP_LED_LEFT_DIO (10)
#define APP_LED_LEFT_MASK (1UL << APP_LED_LEFT_DIO)
#define APP_LED_RIGHT_DIO (11)
#define APP_LED_RIGHT_MASK (1UL << APP_LED_RIGHT_DIO)
#define APP_LEDS_CTRL_MASK (APP_LED_LEFT_MASK | APP_LED_RIGHT_MASK)
#define APP_LED_BLINK_INTERVAL ZTIMER_TIME_MSEC(100)

typedef enum
{
    BLINK_LEFT,
    BLINK_RIGHT,
    BLINK_BOTH
} te_BlinkMode;

typedef struct
{
    uint32 u32Mask;
    uint8 u8Amount;
} ts_BlinkConfig;

PUBLIC void APP_vSetupLeds(void);
PUBLIC void APP_vBlinkLed(te_BlinkMode eBlinkMode, uint8 u8Amount);
PUBLIC void APP_cbBlinkLed(void *pvParam);

#endif /* APP_LED_H */
