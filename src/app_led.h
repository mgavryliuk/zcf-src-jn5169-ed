#ifndef APP_LED_H
#define APP_LED_H

#include "ZTimer.h"

#ifdef DEBUG_LED
#define TRACE_LED TRUE
#else
#define TRACE_LED FALSE
#endif

#if defined(TARGET_WXKG06LM) || defined(TARGET_WXKG07LM)
    #define APP_LED1_DIO (10)
    #define APP_LED1_MASK (1UL << APP_LED1_DIO)
    #define APP_LED2_DIO (11)
    #define APP_LED2_MASK (1UL << APP_LED2_DIO)
    #define APP_LEDS_CTRL_MASK (APP_LED1_MASK | APP_LED2_MASK)
#elif defined(TARGET_WXKG11LM)
    #define APP_LED1_DIO (11)
    #define APP_LED1_MASK (1UL << APP_LED1_DIO)
    #define APP_LEDS_CTRL_MASK (APP_LED1_MASK)
#else
    #error "Unknown target device"
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
PUBLIC void APP_cbBlinkLed(void *pvParam);

#endif /* APP_LED_H */
