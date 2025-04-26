#include <jendefs.h>
#include "dbg.h"
#include "zcl.h"
#include "zps_apl_af.h"

#include "app_resources.h"
#include "app_zcl_tick.h"

PUBLIC void APP_cbTimerZclTick(void *pvParam)
{

    DBG_vPrintf(TRACE_ZCL_TICK, "ZCL TICK: APP_cbTimerZclTick\n");
    ZPS_tsAfEvent sStackEvent;
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = &sStackEvent;
    sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
    vZCL_EventHandler(&sCallBackEvent);
    ZTIMER_eStart(u8TimerZclTick, ZCL_TICK_TIME_MSEC);
}
