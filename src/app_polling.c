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

PRIVATE tePollMode eState;
PRIVATE uint8 u8FastCycles = 0;
PRIVATE uint8 u8RegularCycles = POLL_REGULAR_CYCLES;

PUBLIC void APP_vStopPolling(void)
{
    DBG_vPrintf(TRACE_POLL, "POLL: Stop Polling\n");
    ZTIMER_eStop(u8TimerPoll);
}

PUBLIC void APP_vStartPolling(tePollMode ePollMode)
{
    DBG_vPrintf(TRACE_POLL, "POLL: Start Polling. MODE: %d\n", ePollMode);
    uint32 u32PollTime = POLL_REGULAR_TIME;
    eState = ePollMode;
    u8RegularCycles = POLL_REGULAR_CYCLES;
    u8FastCycles = POLL_FAST_CYCLES;

    if (eState == POLL_FAST)
    {
        u32PollTime = POLL_FAST_TIME;
    }
    ZTIMER_eStop(u8TimerPoll);
    ZTIMER_eStart(u8TimerPoll, u32PollTime);
}

PUBLIC void APP_cbTimerPoll(void *pvParam)
{
    if (bNodeIsRunning())
    {
        ZPS_eAplZdoPoll();
        switch (eState)
        {
        case POLL_REGULAR:
            u8RegularCycles--;
            DBG_vPrintf(TRACE_POLL, "POLL: REGULAR MODE. Regular keep alive cycles left: %d\n", u8RegularCycles);
            if (u8RegularCycles == 0)
            {
                DBG_vPrintf(TRACE_POLL, "POLL: No poll cycles left. Do not restart timer\n");
                return;
            }
            ZTIMER_eStart(u8TimerPoll, POLL_REGULAR_TIME);
            break;

        case POLL_FAST:
            u8FastCycles--;
            DBG_vPrintf(TRACE_POLL, "POLL: FAST MODE. Fast keep alive cycles left: %d. Regular keep alive cycles left: %d\n", u8FastCycles, u8RegularCycles);
            if (u8FastCycles == 0)
            {
                DBG_vPrintf(TRACE_POLL, "POLL: Changing poll state to POLL_REGULAR\n");
                eState = POLL_REGULAR;
            }
            ZTIMER_eStart(u8TimerPoll, (eState == POLL_FAST) ? POLL_FAST_TIME : POLL_REGULAR_TIME);
            break;

        default:
            break;
        }
    }
    else
    {
        DBG_vPrintf(TRACE_POLL, "POLL: Cannot poll. Node is not in running state\n");
    }
}
