#include <jendefs.h>
#include "zcl.h"
#include "ButtonMode.h"
#include "zcl_options.h"
#include "zcl_customcommand.h"

const tsZCL_AttributeDefinition asCLD_ButtonModeClusterAttributeDefinitions[] = {
    {E_CLD_BUTTON_MODE_ATTR_ID_MODE_TYPE, (E_ZCL_AF_RD | E_ZCL_AF_RP | E_ZCL_AF_WR | E_ZCL_AF_MS), E_ZCL_ENUM8, (uint32)(&((tsCLD_ButtonMode *)(0))->eMode), 0},
};

tsZCL_ClusterDefinition sCLD_ButtonMode = {
    GENERAL_CLUSTER_ID_BUTTON_MODE,
    TRUE,
    E_ZCL_SECURITY_NETWORK,
    (sizeof(asCLD_ButtonModeClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition)),
    (tsZCL_AttributeDefinition *)asCLD_ButtonModeClusterAttributeDefinitions,
    NULL};

uint8 au8ButtonModeAttributeControlBits[(sizeof(asCLD_ButtonModeClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition))];

PUBLIC teZCL_Status eCLD_ButtonModeCreateButtonMode(
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
        ((tsCLD_ButtonMode *)psClusterInstance->pvEndPointSharedStructPtr)->eMode = E_CLD_BUTTON_MODE_TOGGLE;
    }
    vZCL_SetDefaultReporting(psClusterInstance);

    return E_ZCL_SUCCESS;
}
