#ifndef APP_LED_H
#define APP_LED_H

#include <jendefs.h>
#include "ZTimer.h"

#ifdef DEBUG_LED
#define TRACE_LED TRUE
#else
#define TRACE_LED FALSE
#endif

#define APP_LED_BLINK_INTERVAL ZTIMER_TIME_MSEC(100)

typedef enum
{
    BLINK_LED1,
    BLINK_LED2,
    BLINK_BOTH,
    BLINK_NONE,
} te_BlinkMode;

typedef struct
{
    uint32 u32Mask;
    uint8 u8Amount;
} ts_BlinkConfig;

PUBLIC void APP_vSetupLeds(void);
PUBLIC void APP_vBlinkLed(te_BlinkMode eBlinkMode, uint8 u8Amount);

#endif /* APP_LED_H */
