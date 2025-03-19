#include <jendefs.h>
#include <string.h>
#include "dbg.h"
#include "zcl.h"
#include "zcl_options.h"
#include "zps_gen.h"
#include "Basic.h"
#include "Identify.h"
#include "PowerConfiguration.h"

#include "app_basic_endpoint.h"
#include "app_battery.h"

PRIVATE void APP_cbBasicEndpointCallback(tsZCL_CallBackEvent *psEvent);

tsZHA_BasicEndpoint tsBasicEndpoint;

PRIVATE void syncBatteryPercentageRemaining(void)
{
    tsBatteryData sBatteryData = APP_u8GetBatteryPercentageRemaining();
    tsBasicEndpoint.sPowerConfigurationCluster.u8BatteryPercentageRemaining = sBatteryData.u8BatteryPercentageRemaining * 2;
    tsBasicEndpoint.sPowerConfigurationCluster.u8BatteryVoltage = sBatteryData.u8Voltage;
}

PUBLIC void APP_vRegisterBasicEndPoint(void)
{
    teZCL_Status eZCL_Status = eCLD_BasicCreateBasic(
        &tsBasicEndpoint.sClusterInstance.sBasicServer,
        TRUE,
        &sCLD_Basic,
        &tsBasicEndpoint.sBasicServerCluster,
        &au8BasicClusterAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_BasicCreateBasic failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_IdentifyCreateIdentify(
        &tsBasicEndpoint.sClusterInstance.sIdentifyServer,
        TRUE,
        &sCLD_Identify,
        &tsBasicEndpoint.sIdentifyServerCluster,
        &au8IdentifyAttributeControlBits[0],
        &tsBasicEndpoint.sIdentifyServerCustomDataStructure);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_IdentifyCreateIdentify failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_PowerConfigurationCreatePowerConfiguration(
        &tsBasicEndpoint.sClusterInstance.sPowerConfigurationServer,
        TRUE,
        &sCLD_PowerConfiguration,
        &tsBasicEndpoint.sPowerConfigurationCluster,
        &au8PowerConfigurationAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_PowerConfigurationCreatePowerConfiguration failed with status: %d\n", eZCL_Status);
    }

    tsBasicEndpoint.sEndPoint.u8EndPointNumber = WXKG07LM_ALT_BASIC_ENDPOINT;
    tsBasicEndpoint.sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    tsBasicEndpoint.sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    tsBasicEndpoint.sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    tsBasicEndpoint.sEndPoint.u16NumberOfClusters = sizeof(tsZHA_BasicEndpointClusterInstances) / sizeof(tsZCL_ClusterInstance);
    tsBasicEndpoint.sEndPoint.psClusterInstance = (tsZCL_ClusterInstance *)&tsBasicEndpoint.sClusterInstance;
    tsBasicEndpoint.sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    tsBasicEndpoint.sEndPoint.pCallBackFunctions = &APP_cbBasicEndpointCallback;

    eZCL_Status = eZCL_Register(&tsBasicEndpoint.sEndPoint);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eZCL_Register failed with status: %d\n", eZCL_Status);
    }

    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ManufacturerName, "HomeMade", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8ModelIdentifier, "lumi.remote.b286acn02.alt", CLD_BAS_MODEL_ID_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8DateCode, "20251105", CLD_BAS_DATE_SIZE);
    memcpy(tsBasicEndpoint.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);
    syncBatteryPercentageRemaining();
}

PRIVATE void APP_cbBasicEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
    case E_ZCL_CBET_UNHANDLED_EVENT:
    case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
    case E_ZCL_CBET_READ_REQUEST:
        break;
    case E_ZCL_CBET_DEFAULT_RESPONSE:
    case E_ZCL_CBET_ERROR:
    case E_ZCL_CBET_TIMER:
    case E_ZCL_CBET_ZIGBEE_EVENT:
        DBG_vPrintf(TRACE_BASIC_EP, "APP_cbBasicEndpointCallback: No action\n");
        break;

    case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
        DBG_vPrintf(TRACE_BASIC_EP, "APP_cbBasicEndpointCallback: Read Attrib Rsp %d %02x\n",
                    psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
                    *((uint8 *)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
        break;

    case E_ZCL_CBET_CLUSTER_CUSTOM:
        DBG_vPrintf(TRACE_BASIC_EP, "APP_cbBasicEndpointCallback: Custom %04x\n", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);
        break;

    case E_ZCL_CBET_CLUSTER_UPDATE:
        DBG_vPrintf(TRACE_BASIC_EP, "APP_cbBasicEndpointCallback: Update Id %04x\n", psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
        break;

    default:
        DBG_vPrintf(TRACE_BASIC_EP, "APP_cbBasicEndpointCallback: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\n", psEvent->eEventType);
        break;
    }
}

PUBLIC void APP_vSendPowerConfigurationClusterReport(void)
{
    syncBatteryPercentageRemaining();
    teZCL_Status eStatus;
    PDUM_thAPduInstance myPDUM_thAPduInstance;
    tsZCL_Address sDestinationAddress;
    sDestinationAddress.uAddress.u16DestinationAddress = 0x0000;
    sDestinationAddress.eAddressMode = E_ZCL_AM_SHORT;
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Sending battery report\n");
    myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    if (myPDUM_thAPduInstance == PDUM_INVALID_HANDLE)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: PDUM_INVALID_HANDLE\n");
    }
    eStatus = eZCL_ReportAttribute(
        &sDestinationAddress,
        GENERAL_CLUSTER_ID_POWER_CONFIGURATION,
        E_CLD_PWRCFG_ATTR_ID_BATTERY_PERCENTAGE_REMAINING,
        WXKG07LM_ALT_BASIC_ENDPOINT,
        1,
        myPDUM_thAPduInstance);
    if (E_ZCL_SUCCESS != eStatus)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Sending battery report failed with status: %d\n", eStatus);
    }
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Battery report sent successfully\n");
}
