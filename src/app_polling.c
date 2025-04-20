#include <jendefs.h>
#include "dbg.h"
#include "pwrm.h"
#include "zps_apl.h"
#include "zps_apl_zdo.h"
#include "zcl_common.h"
#include "ZTimer.h"

#include "app_main.h"
#include "app_node.h"
#include "app_polling.h"

#define HAS_DATA_TO_POLL_DEBOUNCE_MASK 0b11111
#define HAS_DATA_TO_POLL_INITIAL_VALUE 0b01000

PRIVATE tePollMode ePollMode;
PRIVATE uint8 u8CommissioningCycles;
PRIVATE uint8 u8HasDataToPoll;

PUBLIC bool_t APP_bPollCanBeStopped(void)
{
    return u8HasDataToPoll == 0 && ePollMode == POLL_REGULAR;
}

PUBLIC void APP_vHandlePollConfirm(ZPS_tsAfPollConfEvent *psAfPollConfirmEvent)
{
    u8HasDataToPoll <<= 1;
    u8HasDataToPoll |= (psAfPollConfirmEvent->u8Status == 0) ? 1 : 0;
    u8HasDataToPoll &= HAS_DATA_TO_POLL_DEBOUNCE_MASK;
}

PUBLIC void APP_vStartPolling(tePollMode eNewPollMode)
{
    ZPS_eAplZdoPoll();
    u8HasDataToPoll = HAS_DATA_TO_POLL_INITIAL_VALUE;
    ePollMode = eNewPollMode;

    uint32 u32PollTime = POLL_FAST_TIME;
    if (ePollMode == POLL_REGULAR)
    {
        u32PollTime = POLL_REGULAR_TIME;
    }
    u8CommissioningCycles = POLL_COMMISSIONING_CYCLES;

    DBG_vPrintf(TRACE_POLL, "POLL: Start Polling. MODE: %d\n", ePollMode);
    ZTIMER_eStop(u8TimerPoll);
    ZTIMER_eStart(u8TimerPoll, u32PollTime);
}

PUBLIC void APP_cbTimerPoll(void *pvParam)
{
    ZPS_eAplZdoPoll();
    switch (ePollMode)
    {
    case POLL_COMMISSIONING:
        DBG_vPrintf(TRACE_POLL, "POLL: Commissioning mode. Cycles left: %d\n", u8CommissioningCycles);
        u8CommissioningCycles--;
        if (u8CommissioningCycles == 0)
        {
            DBG_vPrintf(TRACE_POLL, "POLL: Changing poll state to POLL_FAST\n");
            ePollMode = POLL_FAST;
        }
        ZTIMER_eStart(u8TimerPoll, POLL_FAST_TIME);
        break;

    case POLL_REGULAR:
        DBG_vPrintf(TRACE_POLL, "POLL: Regular mode\n");
        if (u8HasDataToPoll == HAS_DATA_TO_POLL_DEBOUNCE_MASK)
        {
            DBG_vPrintf(TRACE_POLL, "POLL: Switching to POLL_FAST mode\n");
            ePollMode = POLL_FAST;
            ZTIMER_eStart(u8TimerPoll, POLL_FAST_TIME);
            return;
        }

        ZTIMER_eStart(u8TimerPoll, POLL_REGULAR_TIME);
        break;

    case POLL_FAST:
        DBG_vPrintf(TRACE_POLL, "POLL: Fast mode\n");
        if (u8HasDataToPoll == 0)
        {
            DBG_vPrintf(TRACE_POLL, "POLL: Switching to POLL_REGULAR mode\n");
            ePollMode = POLL_REGULAR;
            ZTIMER_eStart(u8TimerPoll, POLL_REGULAR_TIME);
            return;
        }
        ZTIMER_eStart(u8TimerPoll, POLL_FAST_TIME);
        break;

    default:
        break;
    }
}
