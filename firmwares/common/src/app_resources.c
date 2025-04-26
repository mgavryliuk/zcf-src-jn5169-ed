#include <jendefs.h>
#include "bdb_api.h"
#include "dbg.h"
#include "zps_apl_af.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "zps_gen.h"

#include "app_events.h"
#include "app_resources.h"
#include "app_zcl_tick.h"

PRIVATE ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + BDB_ZTIMER_STORAGE];

PRIVATE APP_tsEvent asAppEvent[APP_QUEUE_SIZE];
PRIVATE BDB_tsZpsAfEvent asBdbEvent[BDB_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMlmeVsDcfmInd asMacMlmeVsDcfmInd[MLME_QUEQUE_SIZE];
PRIVATE MAC_tsMcpsVsCfmData asMacMcpsDcfm[MCPS_DCFM_QUEUE_SIZE];
PRIVATE zps_tsTimeEvent asTimeEvent[TIMER_QUEUE_SIZE];

PUBLIC tszQueue APP_msgBdbEvents;
PUBLIC tszQueue APP_msgAppEvents;

PUBLIC uint8 u8TimerLedBlink;
PUBLIC uint8 u8TimerButtonScan;
PUBLIC uint8 u8TimerPoll;
PUBLIC uint8 u8TimerZclTick;

PRIVATE void APP_vInitTimers(void);
PRIVATE void APP_vInitQueues(void);

PUBLIC void APP_vInitResources(void)
{
    DBG_vPrintf(TRACE_RESOURCES, "APP RESOURCES: APP_vInitResources called\n");
    APP_vInitTimers();
    APP_vInitQueues();
    DBG_vPrintf(TRACE_RESOURCES, "APP RESOURCES: APP_vInitResources finished\n");
}

PRIVATE void APP_vInitTimers(void)
{
    DBG_vPrintf(TRACE_RESOURCES, "APP RESOURCES: APP_vInitTimers called\n");
    ZTIMER_eInit(asTimers, sizeof(asTimers) / sizeof(ZTIMER_tsTimer));
    ZTIMER_eOpen(&u8TimerLedBlink, APP_cbTimerLedBlink, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerButtonScan, APP_cbTimerButtonScan, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerPoll, APP_cbTimerPoll, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    ZTIMER_eOpen(&u8TimerZclTick, APP_cbTimerZclTick, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
    DBG_vPrintf(TRACE_RESOURCES, "APP RESOURCES: APP_vInitTimers finished\n");
}

PRIVATE void APP_vInitQueues(void)
{
    DBG_vPrintf(TRACE_RESOURCES, "APP RESOURCES: APP_vInitQueues called\n");
    ZQ_vQueueCreate(&APP_msgAppEvents, APP_QUEUE_SIZE, sizeof(APP_tsEvent), (uint8 *)asAppEvent);
    ZQ_vQueueCreate(&APP_msgBdbEvents, BDB_QUEUE_SIZE, sizeof(BDB_tsZpsAfEvent), (uint8 *)asBdbEvent);

    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd, MLME_QUEQUE_SIZE, sizeof(MAC_tsMlmeVsDcfmInd), (uint8 *)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd, MCPS_QUEUE_SIZE, sizeof(MAC_tsMcpsVsDcfmInd), (uint8 *)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfm, MCPS_DCFM_QUEUE_SIZE, sizeof(MAC_tsMcpsVsCfmData), (uint8 *)asMacMcpsDcfm);
    ZQ_vQueueCreate(&zps_TimeEvents, TIMER_QUEUE_SIZE, sizeof(zps_tsTimeEvent), (uint8 *)asTimeEvent);
    DBG_vPrintf(TRACE_RESOURCES, "APP RESOURCES: APP_vInitQueues finished\n");
}
