#include <jendefs.h>
#include "dbg.h"
#include "dbg_uart.h"
#include "string.h"
#include "portmacro.h"
#include "pwrm.h"

#include "app_common.h"
#include "app_buttons.h"
#include "app_main.h"

static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

void vAppRegisterPWRMCallbacks(void){
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
}

PUBLIC void waitForXTAL(void) {
    while (bAHI_GetClkSource() == TRUE);
    bAHI_SetClockRate(3);
}

PUBLIC void APP_vSetUpHardware(void)
{
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();
}

PUBLIC void vAppMain(void)
{
    waitForXTAL();
    
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "vAppMain: Start\n");
    APP_vSetUpHardware();
    APP_vInitResources();
    APP_vInitialise();
    BDB_vStart();
    // Do this only on specific button click
    // BDB_eNsStartNwkSteering();
    APP_vScheduleActivity();
    DBG_vPrintf(TRACE_APP, "vAppMain: Entering main loop...\n");
    APP_vMainLoop();
}

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRACE_APP, "PWRM_CALLBACK(PreSleep): Callback triggered\n");
    vAppApiSaveMacSettings();
    ZTIMER_vSleep();
    DBG_vUartFlush();
    vAHI_UartDisable(E_AHI_UART_0);
}

PWRM_CALLBACK(Wakeup)
{
    waitForXTAL();
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_APP, "\nPWRM_CALLBACK(Wakeup): Callback triggered\n");
    vMAC_RestoreSettings();
    APP_vSetUpHardware();
    ZTIMER_vWake();
    /* Activate the SleepTask, that would start the SW timer and polling would continue * */
    // APP_vStartUpHW();
}
