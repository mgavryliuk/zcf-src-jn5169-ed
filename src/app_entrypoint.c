#include <jendefs.h>
#include "dbg.h"
#include "dbg_uart.h"
#include "portmacro.h"
#include "pwrm.h"
#include "bdb_start.h"

#include "app_entrypoint.h"
#include "app_basic_endpoint.h"
#include "app_button.h"
#include "app_main.h"
#include "app_node.h"
#include "app_polling.h"

static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

void vAppRegisterPWRMCallbacks(void)
{
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
}

PUBLIC void vISR_SystemController(void)
{
    uint8 u8WakeInt = u8AHI_WakeTimerFiredStatus();
    uint32 u32DioStatus = u32AHI_DioInterruptStatus();
    if (u32DioStatus & APP_BTN_CTRL_MASK)
    {
        DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: vISR_SystemController called. Some buttong pressed. DIO Status: %x04\n", u32DioStatus);
        vAHI_DioInterruptEnable(0, APP_BTN_CTRL_MASK);
        ZTIMER_eStop(u8TimerButtonScan);
        ZTIMER_eStart(u8TimerButtonScan, APP_BTN_TIMER_MSEC);
    }

    if (u8WakeInt & E_AHI_WAKE_TIMER_MASK_1)
    {
        DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: vISR_SystemController called. Wake Timer 1 Interrupt\n");
        PWRM_vWakeInterruptCallback();
    }
}

PUBLIC void waitForXTAL(void)
{
    while (bAHI_GetClkSource() == TRUE)
        ;
    bAHI_SetClockRate(3);
}

PUBLIC void APP_vSetUpHardware(void)
{
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: APP_vSetUpHardware\n");
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: APP_vSetUpHardware finished!\n");
}

PRIVATE void APP_vConfigureADC(void)
{

    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: Configuing ADC\n");
    vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE,
                     E_AHI_AP_INT_DISABLE,
                     E_AHI_AP_SAMPLE_2,
                     E_AHI_AP_CLOCKDIV_500KHZ,
                     E_AHI_AP_INTREF);
    while (!bAHI_APRegulatorEnabled())
        ;
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: ADC configured!\n");
}

PRIVATE void initDebugUART(void)
{
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_ENTRYPOINT, "\nAPP ENTRY: Debug UART initialized!\n");
}

PUBLIC void vAppMain(void)
{
    waitForXTAL();
    initDebugUART();
    APP_vSetUpHardware();
    APP_vConfigureADC();
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: Calling APP_vInitResources\n");
    APP_vInitResources();
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: Calling APP_vInitialise\n");
    APP_vInitialise();
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: Calling BDB_vStart\n");
    BDB_vStart();
    DBG_vPrintf(TRACE_ENTRYPOINT, "APP ENTRY: Entering main loop...\n");
    APP_vMainLoop();
}

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRACE_ENTRYPOINT, "PWRM_CALLBACK(PreSleep): Callback triggered\n");
    if (bNodeJoined())
    {
        DBG_vPrintf(TRACE_ENTRYPOINT, "PWRM_CALLBACK(PreSleep): Saving MAC settings\n");
        vAppApiSaveMacSettings();
    }
    ZTIMER_vSleep();
    DBG_vUartFlush();
    vAHI_UartDisable(E_AHI_UART_0);
}

PWRM_CALLBACK(Wakeup)
{
    waitForXTAL();
    initDebugUART();
    DBG_vPrintf(TRACE_ENTRYPOINT, "PWRM_CALLBACK(Wakeup): Callback triggered. Restoring MAC settings\n");
    vMAC_RestoreSettings();
    APP_vSetUpHardware();
    APP_vConfigureADC();
    ZTIMER_vWake();

    APP_vStartPolling(POLL_FAST);
    // sendBasicEndpointReports();
}
