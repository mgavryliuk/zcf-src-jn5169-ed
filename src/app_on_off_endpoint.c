#include <jendefs.h>
#include "dbg.h"
#include "OnOff.h"

#include "zps_gen.h"
#include "app_on_off_endpoint.h"

PRIVATE void APP_cbOnOffEndpointCallback(tsZCL_CallBackEvent *psEvent);

tsZHA_OnOffEndpoint tsOnOffLeftButtonEndpoint;
tsZHA_OnOffEndpoint tsOnOffRightButtonEndpoint;
tsZHA_OnOffEndpoint tsOnOffBothButtonsEndpoint;

PRIVATE void APP_vRegisterOnOffEndPoint(uint8 uEndPoint, tsZHA_OnOffEndpoint *tsEndpoint)
{
    teZCL_Status eZCL_Status = eCLD_OnOffCreateOnOff(
        &tsEndpoint->sClusterInstance.sOnOffClient,
        FALSE,
        &sCLD_OnOffClient,
        &tsEndpoint->sOnOffClientCluster,
        &au8OnOffClientAttributeControlBits[0],
        NULL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "Error: APP_vRegisterOnOffEndPoint:%d\r\n", eZCL_Status);
    }

    tsEndpoint->sEndPoint.u8EndPointNumber = WXKG07LM_ALT_BASIC_ENDPOINT;
    tsEndpoint->sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    tsEndpoint->sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    tsEndpoint->sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    tsEndpoint->sEndPoint.u16NumberOfClusters = sizeof(tsZHA_OnOffEndpointClusterInstances) / sizeof(tsZCL_ClusterInstance);
    tsEndpoint->sEndPoint.psClusterInstance = (tsZCL_ClusterInstance *)&tsEndpoint->sClusterInstance;
    tsEndpoint->sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    tsEndpoint->sEndPoint.pCallBackFunctions = &APP_cbOnOffEndpointCallback;

    eZCL_Status = eZCL_Register(&tsEndpoint->sEndPoint);
}

PUBLIC void APP_vRegisterOnOffEndPoints(void)
{
    APP_vRegisterOnOffEndPoint(WXKG07LM_ALT_LEFTBUTTON_ENDPOINT, &tsOnOffLeftButtonEndpoint);
    APP_vRegisterOnOffEndPoint(WXKG07LM_ALT_RIGHTBUTTON_ENDPOINT, &tsOnOffRightButtonEndpoint);
    APP_vRegisterOnOffEndPoint(WXKG07LM_ALT_BOTHBUTTONS_ENDPOINT, &tsOnOffBothButtonsEndpoint);
}

PRIVATE void APP_cbOnOffEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
    default:
        DBG_vPrintf(TRUE, "EP EVT: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\r\n", psEvent->eEventType);
        break;
    }
}
