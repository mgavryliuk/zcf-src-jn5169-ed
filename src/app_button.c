#include <jendefs.h>
#include "dbg.h"
#include "portmacro.h"
#include "pwrm.h"

#include "app_button.h"
#include "app_common.h"
#include "app_led.h"


PUBLIC void APP_vConfigureButtons(void)
{
    DBG_vPrintf(TRACE_APP, "APP: Configuring buttons. Mask: %08x\n", APP_BTN_CTRL_MASK);
    vAHI_DioSetDirection(APP_BTN_CTRL_MASK, 0);
    vAHI_DioSetPullup(APP_BTN_CTRL_MASK, 0);
    vAHI_DioInterruptEdge(0, APP_BTN_CTRL_MASK);
    vAHI_DioWakeEnable(APP_BTN_CTRL_MASK, 0);
    APP_cbTimerButtonScan(NULL);
}


PUBLIC void APP_vHandleBtnInterrupts(void) {
    uint32 u32DioStatus = u32AHI_DioWakeStatus();
    if(u32DioStatus & APP_BTN_CTRL_MASK)
    {
        if (u32DioStatus & APP_BTN_LEFT_MASK) {
            DBG_vPrintf(TRACE_APP, "APP:  Button interrupt - Left\n");
            APP_vBlinkLed(BLINK_LEFT, 1);
        }
        if (u32DioStatus & APP_BTN_RIGHT_MASK) {
            DBG_vPrintf(TRACE_APP, "APP: Button interrupt - Right\n");
            APP_vBlinkLed(BLINK_RIGHT, 1);
        }
        // TODO: handle button scan and blink proper led based on what btn was clicked
        APP_cbTimerButtonScan(NULL);
    }
}

PUBLIC void APP_cbTimerButtonScan(void *pvParam) {
    DBG_vPrintf(TRACE_APP, "Scan btn callback\n");
}
