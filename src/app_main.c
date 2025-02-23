#include "dbg.h"
#include "ZQueue.h"
#include "ZTimer.h"
#include "bdb_api.h"
#include "mac_vs_sap.h"
#include "zps_apl_af.h"
#include "zps_gen.h"
#include "AppHardwareApi.h"
#include "pwrm.h"
#include "pdum_gen.h"
#include "PDM.h"
#include "bdb_api.h"
#include "app_main.h"
#include "app_node.h"

PUBLIC tszQueue APP_msgBdbEvents;

PRIVATE ZTIMER_tsTimer asTimers[APP_ZTIMER_STORAGE + BDB_ZTIMER_STORAGE];

PRIVATE BDB_tsZpsAfEvent    asBdbEvent[BDB_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMlmeVsDcfmInd asMacMlmeVsDcfmInd[MLME_QUEQUE_SIZE];
PRIVATE MAC_tsMcpsVsCfmData asMacMcpsDcfm[MCPS_DCFM_QUEUE_SIZE];
PRIVATE zps_tsTimeEvent     asTimeEvent[TIMER_QUEUE_SIZE];

extern void zps_taskZPS(void);
extern void PWRM_vManagePower(void);

PUBLIC void APP_vInitResources(void) 
{
    ZTIMER_eInit(asTimers, sizeof(asTimers) / sizeof(ZTIMER_tsTimer));

    ZQ_vQueueCreate(&APP_msgBdbEvents,    BDB_QUEUE_SIZE,       sizeof(BDB_tsZpsAfEvent),    (uint8*)asBdbEvent);
    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd,  MLME_QUEQUE_SIZE,     sizeof(MAC_tsMlmeVsDcfmInd), (uint8*)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd,  MCPS_QUEUE_SIZE,      sizeof(MAC_tsMcpsVsDcfmInd), (uint8*)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfm,     MCPS_DCFM_QUEUE_SIZE, sizeof(MAC_tsMcpsVsCfmData), (uint8*)asMacMcpsDcfm);
    ZQ_vQueueCreate(&zps_TimeEvents,      TIMER_QUEUE_SIZE,     sizeof(zps_tsTimeEvent),     (uint8*)asTimeEvent);
}

PRIVATE void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRUE, "ERROR: Extended status 0x%02x\n", eExtendedStatus);
}

PUBLIC void APP_vInitialise(void)
{
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);
    PDM_eInitialise(0);
    PDUM_vInit();
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);
    APP_vInitialiseNode();
}

PUBLIC void APP_vMainLoop(void)
{
    while (TRUE)
    {
        zps_taskZPS();
        bdb_taskBDB();
        ZTIMER_vTask();
        // APP_taskEndDevice();

        vAHI_WatchdogRestart();
        PWRM_vManagePower();
    }
}
