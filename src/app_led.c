#include <jendefs.h>
#include "dbg.h"
#include "AppHardwareApi.h"

#include "app_common.h"
#include "app_led.h"

PUBLIC void APP_vSetupLeds()
{
    DBG_vPrintf(TRACE_APP, "APP: Configure LEDs control MASK: %u\n", APP_LED_CTRL_MASK);
    vAHI_DioSetDirection(0, APP_LED_CTRL_MASK);
}

PUBLIC void APP_vBlinkLed() {
    uint32 u32CurrentState = u32AHI_DioReadInput();
    vAHI_DioSetOutput(u32CurrentState^APP_LED_CTRL_MASK, u32CurrentState&APP_LED_CTRL_MASK);
}

PUBLIC void APP_cbBlinkLed(void *pvParam) {
    DBG_vPrintf(TRACE_APP, "APP: Blink Led callback\n");
    // uint32 u32CurrentState = u32AHI_DioReadInput();
    // vAHI_DioSetOutput(u32CurrentState, u32LedPin);
}
