#include <jendefs.h>

#include "bdb_api.h"
#include "dbg.h"
#include "pwrm.h"
#include "zps_apl_af.h"
#include "PDM.h"
#include "ZQueue.h"
#include "ZTimer.h"

#include "pdum_gen.h"

#include "app_init.h"
#include "app_led.h"
#include "app_node.h"
#include "app_battery.h"
#include "app_buttons.h"

PUBLIC void APP_vInitialise(tpfExtendedStatusCallBack pfExtendedStatusCallBack)
{
    DBG_vPrintf(TRACE_INIT, "APP INIT: APP_vInitialise called\n");
    PDM_eInitialise(0);
    DBG_vPrintf(TRACE_INIT, "APP INIT: Set PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON)\n");
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);
    DBG_vPrintf(TRACE_INIT, "APP INIT: Init PDUM\n");
    PDUM_vInit();
    DBG_vPrintf(TRACE_INIT, "APP INIT: Set extended status callback\n");
    ZPS_vExtendedStatusSetCallback(pfExtendedStatusCallBack);
    DBG_vPrintf(TRACE_INIT, "APP INIT: Calling APP_vInitialiseNode\n");
    APP_vInitialiseNode();

    DBG_vPrintf(TRACE_INIT, "APP INIT: Calling APP_vMeasureRemainingBattery\n");
    APP_vMeasureRemainingBattery();

    DBG_vPrintf(TRACE_INIT, "APP INIT: Calling APP_vSetupLeds\n");
    APP_vSetupLeds();
    DBG_vPrintf(TRACE_INIT, "APP INIT: Calling APP_vConfigureButtons\n");
    APP_vConfigureButtons();
}
