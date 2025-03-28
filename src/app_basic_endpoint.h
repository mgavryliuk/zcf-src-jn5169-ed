#ifndef APP_BASIC_ENDPOINT_H
#define APP_BASIC_ENDPOINT_H

#include "zcl.h"
#include "Basic.h"
#include "Identify.h"
#include "PowerConfiguration.h"
#include "ButtonMode.h"

#ifdef DEBUG_BASIC_EP
#define TRACE_BASIC_EP TRUE
#else
#define TRACE_BASIC_EP FALSE
#endif

typedef struct
{
    tsZCL_ClusterInstance sBasicServer;
    tsZCL_ClusterInstance sIdentifyServer;
    tsZCL_ClusterInstance sPowerConfigurationServer;
    tsZCL_ClusterInstance sButtonModeServer;
} tsZHA_BasicEndpointClusterInstances __attribute__((aligned(4)));

typedef struct
{
    tsZCL_EndPointDefinition sEndPoint;
    tsZHA_BasicEndpointClusterInstances sClusterInstance;

    tsCLD_Basic sBasicServerCluster;

    tsCLD_Identify sIdentifyServerCluster;
    tsCLD_IdentifyCustomDataStructure sIdentifyServerCustomDataStructure;

    tsCLD_PowerConfiguration sPowerConfigurationCluster;

    tsCLD_ButtonMode sButtonModeCluster;
} tsZHA_BasicEndpoint;

PUBLIC void APP_vRegisterBasicEndPoint(void);
PUBLIC void sendBasicEndpointReports(void);

#endif /* APP_BASIC_ENDPOINT_H */
