#include "Basic.h"
#include "Identify.h"
#include "PowerConfiguration.h"
#include "zcl.h"
#include "dbg.h"
#include <string.h>
#include "app_basic_endpoint.h"
#include "zps_gen.h"
#include "zcl_options.h"

tsZHA_BasicEndpoint tsBasicEndpoint;

PRIVATE void APP_BasicEndpointCallback(tsZCL_CallBackEvent *psEvent);

PUBLIC void APP_vRegisterBasicEndPoint(void)
{
    teZCL_Status eZCL_Status = eCLD_BasicCreateBasic(
        &tsBasicEndpoint.sClusterInstance.sBasicServer,
        TRUE,
        &sCLD_Basic,
        &tsBasicEndpoint.sBasicServerCluster,
        &au8BasicClusterAttributeControlBits[0]
    );
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "Error: eCLD_BasicCreateBasic:%d\r\n", eZCL_Status);
    }

    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ManufacturerName, "NXP", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ModelIdentifier, "BDB-EndDevice", CLD_BAS_MODEL_ID_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8DateCode, "20150212", CLD_BAS_DATE_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);

    eZCL_Status = eCLD_IdentifyCreateIdentify(
        &tsBasicEndpoint.sClusterInstance.sIdentifyServer,
        TRUE,
        &sCLD_Identify,
        &tsBasicEndpoint.sIdentifyServerCluster,
        &au8BasicClusterAttributeControlBits[0],
        &tsBasicEndpoint.sIdentifyServerCustomDataStructure
    );
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "Error: eCLD_IdentifyCreateIdentify:%d\r\n", eZCL_Status);
    }

    eZCL_Status = eCLD_PowerConfigurationCreatePowerConfiguration(
        &tsBasicEndpoint.sClusterInstance.sPowerConfigurationServer,
        TRUE,
        &sCLD_PowerConfiguration,
        &tsBasicEndpoint.sPowerConfigurationCluster,
        &au8BasicClusterAttributeControlBits[0]
    );
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "Error: eCLD_PowerConfigurationCreatePowerConfiguration:%d\r\n", eZCL_Status);
    }

    tsBasicEndpoint.sEndPoint.u8EndPointNumber = WXKG07LM_ALT_BASIC_ENDPOINT;
    tsBasicEndpoint.sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    tsBasicEndpoint.sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    tsBasicEndpoint.sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    tsBasicEndpoint.sEndPoint.u16NumberOfClusters = sizeof(tsZHA_BasicEndpointClusterInstances) / sizeof(tsZCL_ClusterInstance);
    tsBasicEndpoint.sEndPoint.psClusterInstance = (tsZCL_ClusterInstance*)&tsBasicEndpoint.sClusterInstance;
    tsBasicEndpoint.sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    tsBasicEndpoint.sEndPoint.pCallBackFunctions = APP_BasicEndpointCallback;

    eZCL_Status = eZCL_Register(&tsBasicEndpoint.sEndPoint);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRUE, "Error: eZCL_Register:%d\r\n", eZCL_Status);
    }
}

PRIVATE void APP_BasicEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
        case E_ZCL_CBET_UNHANDLED_EVENT:
        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
        case E_ZCL_CBET_READ_REQUEST:
        case E_ZCL_CBET_DEFAULT_RESPONSE:
        case E_ZCL_CBET_ERROR:
        case E_ZCL_CBET_TIMER:
        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRUE, "EP EVT:No action\r\n");
            break;

        case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
            DBG_vPrintf(TRUE, " Read Attrib Rsp %d %02x\n", psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
                *((uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            DBG_vPrintf(TRUE, "EP EVT: Custom %04x\r\n", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);
            // APP_vHandleClusterCustomCommands(psEvent);
            break;

        case E_ZCL_CBET_CLUSTER_UPDATE:
            DBG_vPrintf(TRUE, "Update Id %04x\n", psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
            // APP_vHandleClusterUpdate(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "EP EVT: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\r\n", psEvent->eEventType);
            break;
    }
}
