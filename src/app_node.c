#include "jendefs.h"
#include "dbg.h"
#include "zps_apl_af.h"
#include "bdb_api.h"
#include "zcl.h"
#include "pdum_gen.h"
#include "PDM.h"
#include "app_main.h"
#include "zps_gen.h"
#include "app_basic_endpoint.h"
#include "app_node.h"

PRIVATE teNodeState eNodeState;

PRIVATE void APP_ZCL_vInitialise(void);
PRIVATE void APP_vBdbInit(void);

PUBLIC void APP_vInitialiseNode(void)
{
    uint16 u16ByteRead;
    eNodeState = E_STARTUP;
    PDM_eReadDataFromRecord(PDM_ID_APP_END_DEVICE,
                            &eNodeState,
                            sizeof(teNodeState),
                            &u16ByteRead);
    // ZPS_u32MacSetTxBuffers(4);
    ZPS_eAplAfInit();
    APP_ZCL_vInitialise();
    APP_vBdbInit();
}

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent) {}

PRIVATE void APP_ZCL_vInitialise(void)
{
    teZCL_Status eZCL_Status;

    /* Initialise ZLL */
    eZCL_Status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "\nErr: eZLO_Initialise:%d\n", eZCL_Status);
    }

    APP_vRegisterBasicEndPoint();
    // TODO: register all other endpoiints
}

PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_NONE:
            break;
        case BDB_EVENT_ZPSAF:
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_ZPSAF\n");
            // vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_INIT_SUCCESS\n");
            break;

        case BDB_EVENT_REJOIN_FAILURE:
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_REJOIN_FAILURE\n");
            // bBDBJoinFailed = TRUE;
            break;

        case BDB_EVENT_REJOIN_SUCCESS:
            // vStartPolling();
            // eNodeState = E_RUNNING;
            // bBDBJoinFailed = FALSE;
            DBG_vPrintf(TRUE,"APP: BDB_EVENT_REJOIN_SUCCESS\n");
            break;


        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRUE,"APP: NwkSteering Success 0x%016llx\n",ZPS_psAplAibGetAib()->u64ApsUseExtendedPanid);
            // ZPS_vSaveAllZpsRecords();
            // PDM_eSaveRecordData(PDM_ID_APP_END_DEVICE,
            //                     &eNodeState,
            //                     sizeof(teNodeState));
            // vStartPolling();
            break;

        case BDB_EVENT_APP_START_POLLING:
            DBG_vPrintf(TRUE,"APP: Start polling\n");
            // vStartPolling();
            break;

        default:
            DBG_vPrintf(1, "BDB event callback: evt %d\n", psBdbEvent->eEventType);
            break;
    }
}

PRIVATE void APP_vBdbInit(void)
{
    BDB_tsInitArgs sInitArgs;

    // sBDB.sAttrib.bbdbNodeIsOnANetwork = ((eNodeState >= E_RUNNING)? (TRUE): (FALSE));
    sBDB.sAttrib.bbdbNodeIsOnANetwork = FALSE;
    if(sBDB.sAttrib.bbdbNodeIsOnANetwork)
    {
        // DBG_vPrintf(TRACE_APP, "APP: NFN Start\n");
        // bFailToJoin = TRUE;
        // ZTIMER_eStart(u8TimerPoll, ZTIMER_TIME_MSEC(1000) );
    }
    else
    {
        // DBG_vPrintf(TRACE_APP, "APP: FN Start\n");
        // u8KeepAliveTime = KEEP_ALIVE_FACTORY_NEW;
    }
    // ZTIMER_eStart(u8TimerPoll, ZTIMER_TIME_MSEC(1000) );
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    BDB_vInit(&sInitArgs);
}
