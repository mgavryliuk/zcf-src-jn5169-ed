#include <jendefs.h>
#include "dbg.h"
#include "dbg_uart.h"
#include "portmacro.h"
#include "pwrm.h"
#include "bdb_start.h"

#include "app_main.h"
#include "app_events.h"
#include "app_init.h"
#include "app_node.h"
#include "app_polling.h"
#include "app_resources.h"

#ifdef DBG_ENABLE
PRIVATE uint32 logSleepAttempt = 0;
#endif

PRIVATE pwrm_tsWakeTimerEvent wakeStruct;
PRIVATE bool_t bActivityScheduled = FALSE;

PRIVATE void vWaitForXTAL(void);
PRIVATE void vInitDebugUART(void);
PRIVATE void vSetUpHardware(void);
PRIVATE void vAPConfigure(void);
PRIVATE void vWakeCallBack(void);
PRIVATE uint8 u8NumberOfTimersTaskTimers(void);
PRIVATE uint8 u8NumberOfNonSleepPreventingTimers(void);
PRIVATE void vAttemptToSleep(void);
PRIVATE void vfExtendedStatusCallBack(ZPS_teExtendedStatus eExtendedStatus);
PRIVATE void vMainLoop(void);

extern void zps_taskZPS(void);

static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

PUBLIC void vAppRegisterPWRMCallbacks(void)
{
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);
}

/****************************************************************************
 *
 * NAME: vAppMain
 *
 * DESCRIPTION:
 * Main application entry point function.
 * Initializes hardware, starts network stack, and enters main processing loop.
 * Also called after waking from DEEP sleep.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vAppMain(void)
{
    vWaitForXTAL();
    vInitDebugUART();
    vSetUpHardware();
    vAPConfigure();
    APP_vInitResources();
    APP_vInitialise(vfExtendedStatusCallBack);
    BDB_vStart();
    vMainLoop();
}

PUBLIC void APP_vOnWakeTimer1(void)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: APP_vOnWakeTimer1 callback triggered\n");
    PWRM_vWakeInterruptCallback();
}

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRUE, "PWRM_CALLBACK: PreSleep callback triggered\n");
    if (bNodeJoined())
    {
        DBG_vPrintf(TRUE, "PWRM_CALLBACK: Node is joined. Saving MAC settings\n");
        vAppApiSaveMacSettings();
    }
    ZTIMER_vSleep();
    DBG_vUartFlush();
    vAHI_UartDisable(E_AHI_UART_0);
}

PWRM_CALLBACK(Wakeup)
{
    vWaitForXTAL();
    vInitDebugUART();
    DBG_vPrintf(TRUE, "PWRM_CALLBACK: Wakeup callback triggered\n");
    vSetUpHardware();
    vAPConfigure();
    vMAC_RestoreSettings();
    ZTIMER_vWake();
    APP_vSendWakeUpEvent();
}

PRIVATE void vWaitForXTAL(void)
{
    while (bAHI_GetClkSource() == TRUE)
        ;
    bAHI_SetClockRate(3);
}

PRIVATE void vInitDebugUART(void)
{
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);
    DBG_vPrintf(TRACE_MAIN, "\nAPP MAIN: Debug UART initialized!\n");
}

PRIVATE void vSetUpHardware(void)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: vSetUpHardware\n");
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: vSetUpHardware finished!\n");
}

/****************************************************************************
 *
 * NAME: vAPConfigure
 *
 * DESCRIPTION:
 * Configures the Analogue domain which supplies power to the ADC.
 * The domain is switched on when this function is called to configure
 * the analogue peripherals. Note that this domain is always unpowered
 * during sleep mode.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vAPConfigure(void)
{

    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Configuing ADC\n");
    vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE,
                     E_AHI_AP_INT_DISABLE,
                     E_AHI_AP_SAMPLE_2,
                     E_AHI_AP_CLOCKDIV_500KHZ,
                     E_AHI_AP_INTREF);
    while (!bAHI_APRegulatorEnabled())
        ;
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: ADC configured!\n");
}

PRIVATE void vWakeCallBack(void)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: wake callback triggered\n");
    bActivityScheduled = FALSE;
}

PRIVATE uint8 u8NumberOfTimersTaskTimers(void)
{
    uint8 u8NumberOfRunningTimers = 0;

    if (ZTIMER_eGetState(u8TimerLedBlink) == E_ZTIMER_STATE_RUNNING)
    {
        u8NumberOfRunningTimers++;
    }

    if (ZTIMER_eGetState(u8TimerButtonScan) == E_ZTIMER_STATE_RUNNING)
    {
        u8NumberOfRunningTimers++;
    }

    return u8NumberOfRunningTimers;
}

PRIVATE uint8 u8NumberOfNonSleepPreventingTimers(void)
{
    uint8 u8NumberOfRunningTimers = 0;

    if (ZTIMER_eGetState(u8TimerZclTick) == E_ZTIMER_STATE_RUNNING)
    {
        u8NumberOfRunningTimers++;
    }

    if (ZTIMER_eGetState(u8TimerPoll) == E_ZTIMER_STATE_RUNNING)
    {
        u8NumberOfRunningTimers++;
    }

    return u8NumberOfRunningTimers;
}

/****************************************************************************
 *
 * NAME: vAttemptToSleep
 *
 * DESCRIPTION:
 * Attempts to put the device into sleep mode if conditions allow.
 * Checks activity count and running timers to determine if sleep is possible.
 * If sleep is possible, stops timers and configures appropriate sleep mode:
 * - DEEP_SLEEP if node not joined
 * - OSCON_RAMON sleep with wake timer if node joined
 *
 * PARAMETERS: None
 *
 * RETURNS: None
 *
 ****************************************************************************/
PRIVATE void vAttemptToSleep(void)
{
#ifdef DBG_ENABLE
    logSleepAttempt++;
    if (logSleepAttempt == 3200 * 10)
    {
        DBG_vPrintf(TRUE, "APP MAIN: Activity Count = %d\n", PWRM_u16GetActivityCount());
        DBG_vPrintf(TRUE, "APP MAIN: Task Timers = %d\n", u8NumberOfTimersTaskTimers());
        logSleepAttempt = 0;
    }
#endif

    if (ZTIMER_eGetState(u8TimerPoll) == E_ZTIMER_STATE_RUNNING && !APP_bPollCanBeStopped())
    {
        return;
    }

    if ((PWRM_u16GetActivityCount() == u8NumberOfNonSleepPreventingTimers()) && (u8NumberOfTimersTaskTimers() == 0))
    {
        ZTIMER_eStop(u8TimerZclTick);
        ZTIMER_eStop(u8TimerPoll);

        DBG_vPrintf(TRUE, "APP MAIN: Activity Count = %d\n", PWRM_u16GetActivityCount());
        DBG_vPrintf(TRUE, "APP MAIN: Task Timers = %d\n", u8NumberOfTimersTaskTimers());

        if (!bNodeJoined())
        {
            PWRM_vInit(E_AHI_SLEEP_DEEP);
            DBG_vPrintf(TRACE_MAIN, "APP MAIN: Node is not running. Setting E_AHI_SLEEP_DEEP sleep mode\n");
        }
        else
        {
            if (bActivityScheduled == FALSE)
            {
                PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);
                DBG_vPrintf(TRACE_MAIN, "APP MAIN: Going to E_AHI_SLEEP_OSCON_RAMON sleep for %d seconds\n", MAXIMUM_TIME_TO_SLEEP_SEC);
                PWRM_teStatus u8Status = PWRM_eScheduleActivity(&wakeStruct, MAXIMUM_TIME_TO_SLEEP_SEC * 1000 * 32, vWakeCallBack);
                bActivityScheduled = TRUE;
                DBG_vPrintf(TRACE_MAIN, "APP MAIN: PWRM_eScheduleActivity status: %d\n", u8Status);
            }
            else
            {
                DBG_vPrintf(TRACE_MAIN, "APP MAIN: PWRM_eScheduleActivity is already running\n");
            }
        }
    }
}

PRIVATE void vfExtendedStatusCallBack(ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: vfExtendedStatusCallBack - Extended status 0x%02x\n", eExtendedStatus);
}

PRIVATE void vMainLoop(void)
{
    while (TRUE)
    {
        zps_taskZPS();
        bdb_taskBDB();
        ZTIMER_vTask();
        APP_vProcessEvents();
        vAHI_WatchdogRestart();
        vAttemptToSleep();
        PWRM_vManagePower();
    }
}
