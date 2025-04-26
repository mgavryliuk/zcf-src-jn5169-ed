#include <jendefs.h>
#include <string.h>
#include "dbg.h"
#include "zcl.h"
#include "zcl_options.h"
#include "Basic.h"
#include "Identify.h"
#include "PDM.h"
#include "PowerConfiguration.h"

#include "device_config.h"
#include "app_basic_endpoint.h"
#include "ConfigurationCluster.h"

PRIVATE tsZHA_BasicEndpoint sBasicEndpoint;

PRIVATE void cbBasicEndpointCallback(tsZCL_CallBackEvent *psEvent);

PUBLIC void APP_vRegisterBasicEndPoint(void)
{
    teZCL_Status eZCL_Status = eCLD_BasicCreateBasic(
        &sBasicEndpoint.sClusterInstance.sBasicServer,
        TRUE,
        &sCLD_Basic,
        &sBasicEndpoint.sBasicServerCluster,
        &au8BasicClusterAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_BasicCreateBasic failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_IdentifyCreateIdentify(
        &sBasicEndpoint.sClusterInstance.sIdentifyServer,
        TRUE,
        &sCLD_Identify,
        &sBasicEndpoint.sIdentifyServerCluster,
        &au8IdentifyAttributeControlBits[0],
        &sBasicEndpoint.sIdentifyServerCustomDataStructure);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_IdentifyCreateIdentify failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_PowerConfigurationCreatePowerConfiguration(
        &sBasicEndpoint.sClusterInstance.sPowerConfigurationServer,
        TRUE,
        &sCLD_PowerConfiguration,
        &sBasicEndpoint.sPowerConfigurationCluster,
        &au8PowerConfigurationAttributeControlBits[0]);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eCLD_PowerConfigurationCreatePowerConfiguration failed with status: %d\n", eZCL_Status);
    }

    eZCL_Status = eCLD_ConfigurationCreateConfiguration(
        &sBasicEndpoint.sClusterInstance.sConfigurationServer,
        TRUE,
        &sCLD_Configuration,
        &sBasicEndpoint.sConfigurationCluster,
        &au8ConfigurationAttributeControlBits[0]);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Configuring Custom Button Mode cluster status: %d\n", eZCL_Status);

    sBasicEndpoint.sEndPoint.u8EndPointNumber = sDeviceConfig.u8BasicEndpoint;
    sBasicEndpoint.sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    sBasicEndpoint.sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    sBasicEndpoint.sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    sBasicEndpoint.sEndPoint.u16NumberOfClusters = sizeof(tsZHA_BasicEndpointClusterInstances) / sizeof(tsZCL_ClusterInstance);
    sBasicEndpoint.sEndPoint.psClusterInstance = (tsZCL_ClusterInstance *)&sBasicEndpoint.sClusterInstance;
    sBasicEndpoint.sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    sBasicEndpoint.sEndPoint.pCallBackFunctions = &cbBasicEndpointCallback;

    eZCL_Status = eZCL_Register(&sBasicEndpoint.sEndPoint);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eZCL_Register failed with status: %d\n", eZCL_Status);
    }

    memcpy(sBasicEndpoint.sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBasicEndpoint.sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBasicEndpoint.sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(sBasicEndpoint.sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);

    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: Configuring voltage attribute to be reportable\n");
    eZCL_Status = eZCL_SetReportableFlag(sDeviceConfig.u8BasicEndpoint, GENERAL_CLUSTER_ID_POWER_CONFIGURATION, TRUE, FALSE, E_CLD_PWRCFG_ATTR_ID_BATTERY_VOLTAGE);
    DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP: eZCL_SetReportableFlag status: %d\n", eZCL_Status);
}

PRIVATE void cbBasicEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    switch (psEvent->eEventType)
    {
    case E_ZCL_CBET_CHECK_ATTRIBUTE_RANGE:
        if (clusterId == GENERAL_CLUSTER_ID_CONFIGURATION)
        {
            vHandleConfigurationClusterAttrsRange(psEvent);
        }
        break;

    case E_ZCL_CBET_WRITE_INDIVIDUAL_ATTRIBUTE:
        if (clusterId == GENERAL_CLUSTER_ID_CONFIGURATION)
        {
            vHandleConfigurationClusterAttrsWrite(psEvent, &sBasicEndpoint.sConfigurationCluster);
        }
        break;

    default:
        DBG_vPrintf(TRACE_BASIC_EP, "BASIC EP CB: Event - (%d)\n", psEvent->eEventType);
        break;
    }
}
