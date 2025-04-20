#include <jendefs.h>
#include "zcl.h"
#include "ConfigurationCluster.h"
#include "zcl_options.h"
#include "zcl_customcommand.h"

const tsZCL_AttributeDefinition asCLD_ConfigurationClusterAttributeDefinitions[] = {
    {E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE, (E_ZCL_AF_RD | E_ZCL_AF_WR | E_ZCL_AF_MS), E_ZCL_ENUM8, (uint32)(&((tsCLD_Configuration *)(0))->eButtonMode), 0},
    {E_CLD_PREVENT_RESET_ATTR_ID_MODE_TYPE, (E_ZCL_AF_RD | E_ZCL_AF_WR | E_ZCL_AF_MS), E_ZCL_BOOL, (uint32)(&((tsCLD_Configuration *)(0))->bPreventReset), 0},
};

tsZCL_ClusterDefinition sCLD_Configuration = {
    GENERAL_CLUSTER_ID_CONFIGURATION,
    TRUE,
    E_ZCL_SECURITY_NETWORK,
    (sizeof(asCLD_ConfigurationClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition)),
    (tsZCL_AttributeDefinition *)asCLD_ConfigurationClusterAttributeDefinitions,
    NULL};

uint8 au8ConfigurationAttributeControlBits[(sizeof(asCLD_ConfigurationClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition))];

PUBLIC teZCL_Status eCLD_ConfigurationCreateConfiguration(
    tsZCL_ClusterInstance *psClusterInstance,
    bool_t bIsServer,
    tsZCL_ClusterDefinition *psClusterDefinition,
    void *pvEndPointSharedStructPtr,
    uint8 *pu8AttributeControlBits)
{
    vZCL_InitializeClusterInstance(
        psClusterInstance,
        bIsServer,
        psClusterDefinition,
        pvEndPointSharedStructPtr,
        pu8AttributeControlBits,
        NULL,
        NULL);

    if (pvEndPointSharedStructPtr != NULL)
    {
        ((tsCLD_Configuration *)psClusterInstance->pvEndPointSharedStructPtr)->eButtonMode = E_CLD_BUTTON_MODE_TOGGLE;
        ((tsCLD_Configuration *)psClusterInstance->pvEndPointSharedStructPtr)->bPreventReset = FALSE;
    }
    vZCL_SetDefaultReporting(psClusterInstance);
    return E_ZCL_SUCCESS;
}
