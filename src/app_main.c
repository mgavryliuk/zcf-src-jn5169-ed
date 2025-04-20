#include "dbg.h"
#include "bdb_api.h"
#include "mac_vs_sap.h"
#include "pdum_gen.h"
#include "pwrm.h"
#include "zps_apl_af.h"
#include "zps_gen.h"
#include "AppHardwareApi.h"
#include "PDM.h"

#include "app_basic_endpoint.h"
#include "app_battery.h"
#include "app_button.h"
#include "app_events.h"
#include "app_led.h"
#include "app_main.h"
#include "app_node.h"
#include "app_polling.h"

#ifdef DBG_ENABLE
PRIVATE uint32 logSleepAttempt = 0;
#endif

PRIVATE pwrm_tsWakeTimerEvent wakeStruct;
PRIVATE bool_t bActivityScheduled = FALSE;
PUBLIC tszQueue APP_msgBdbEvents;
PUBLIC tszQueue APP_msgAppEvents;

PRIVATE ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + BDB_ZTIMER_STORAGE];

PRIVATE APP_tsEvent asAppEvent[APP_QUEUE_SIZE];
PRIVATE BDB_tsZpsAfEvent asBdbEvent[BDB_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMlmeVsDcfmInd asMacMlmeVsDcfmInd[MLME_QUEQUE_SIZE];
PRIVATE MAC_tsMcpsVsCfmData asMacMcpsDcfm[MCPS_DCFM_QUEUE_SIZE];
PRIVATE zps_tsTimeEvent asTimeEvent[TIMER_QUEUE_SIZE];

PUBLIC uint8 u8LedBlinkTimer;
PUBLIC uint8 u8TimerButtonScan;
PUBLIC uint8 u8TimerPoll;
PUBLIC uint8 u8TimerTick;

extern void zps_taskZPS(void);

PUBLIC void APP_vInitResources(void)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Init timers and queues\n");
    ZTIMER_eInit(asTimers, sizeof(asTimers) / sizeof(ZTIMER_tsTimer));

    ZTIMER_eOpen(&u8LedBlinkTimer, APP_cbBlinkLed, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerButtonScan, APP_cbTimerButtonScan, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerPoll, APP_cbTimerPoll, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerTick, APP_cbTimerZclTick, NULL, ZTIMER_FLAG_PREVENT_SLEEP);

    ZQ_vQueueCreate(&APP_msgAppEvents, APP_QUEUE_SIZE, sizeof(APP_tsEvent), (uint8 *)asAppEvent);
    ZQ_vQueueCreate(&APP_msgBdbEvents, BDB_QUEUE_SIZE, sizeof(BDB_tsZpsAfEvent), (uint8 *)asBdbEvent);
    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd, MLME_QUEQUE_SIZE, sizeof(MAC_tsMlmeVsDcfmInd), (uint8 *)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd, MCPS_QUEUE_SIZE, sizeof(MAC_tsMcpsVsDcfmInd), (uint8 *)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfm, MCPS_DCFM_QUEUE_SIZE, sizeof(MAC_tsMcpsVsCfmData), (uint8 *)asMacMcpsDcfm);
    ZQ_vQueueCreate(&zps_TimeEvents, TIMER_QUEUE_SIZE, sizeof(zps_tsTimeEvent), (uint8 *)asTimeEvent);
}

PRIVATE void vfExtendedStatusCallBack(ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: vfExtendedStatusCallBack - Extended status 0x%02x\n", eExtendedStatus);
}

PUBLIC void APP_vInitialise(void)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Init PDM\n");
    PDM_eInitialise(0);
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Set PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON)\n");
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Init PDUM\n");
    PDUM_vInit();
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Set extended status callback\n");
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Calling APP_vInitialiseNode\n");
    APP_vInitialiseNode();

    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Calling APP_vSetupLeds\n");
    APP_vSetupLeds();
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: Calling APP_vConfigureButtons\n");
    APP_vConfigureButtons();
}

PRIVATE uint8 u8NumberOfTimersTaskTimers(void)
{
    uint8 u8NumberOfRunningTimers = 0;

    if (ZTIMER_eGetState(u8LedBlinkTimer) == E_ZTIMER_STATE_RUNNING)
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

    if (ZTIMER_eGetState(u8TimerTick) == E_ZTIMER_STATE_RUNNING)
    {
        u8NumberOfRunningTimers++;
    }

    if (ZTIMER_eGetState(u8TimerPoll) == E_ZTIMER_STATE_RUNNING)
    {
        u8NumberOfRunningTimers++;
    }

    return u8NumberOfRunningTimers;
}

PRIVATE void APP_vWakeCallBack(void)
{
    DBG_vPrintf(TRACE_MAIN, "APP MAIN: wake callback triggered\n");
    bActivityScheduled = FALSE;
}

PRIVATE void vAttemptToSleep(void)
{
#ifdef DBG_ENABLE
    logSleepAttempt++;
    if (logSleepAttempt == 3200 * 10)
    {
        DBG_vPrintf(TRUE, "APP Sleep Handler: Activity Count = %d\n", PWRM_u16GetActivityCount());
        DBG_vPrintf(TRUE, "APP Sleep Handler: Task Timers = %d\n", u8NumberOfTimersTaskTimers());
        logSleepAttempt = 0;
    }
#endif

    if (ZTIMER_eGetState(u8TimerPoll) == E_ZTIMER_STATE_RUNNING && !APP_bPollCanBeStopped()) {
        return;
    }

    if ((PWRM_u16GetActivityCount() == u8NumberOfNonSleepPreventingTimers()) && (u8NumberOfTimersTaskTimers() == 0))
    {
        ZTIMER_eStop(u8TimerTick);
        ZTIMER_eStop(u8TimerPoll);

        DBG_vPrintf(TRUE, "APP Sleep Handler: Activity Count = %d\n", PWRM_u16GetActivityCount());
        DBG_vPrintf(TRUE, "APP Sleep Handler: Task Timers = %d\n", u8NumberOfTimersTaskTimers());

        bool_t bDeepSleep = bNodeJoined() ? FALSE : TRUE;
        if (bDeepSleep)
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
                PWRM_teStatus u8Status = PWRM_eScheduleActivity(&wakeStruct, MAXIMUM_TIME_TO_SLEEP_SEC * 1000 * 32, APP_vWakeCallBack);
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

PUBLIC void APP_vMainLoop(void)
{
    while (TRUE)
    {
        zps_taskZPS();
        bdb_taskBDB();
        ZTIMER_vTask();
        APP_processEvents();
        vAHI_WatchdogRestart();
        vAttemptToSleep();
        PWRM_vManagePower();
    }
}
