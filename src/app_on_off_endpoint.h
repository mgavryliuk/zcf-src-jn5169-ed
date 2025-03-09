#ifndef APP_ON_OFF_EP_H
#define APP_ON_OFF_EP_H

#include "zcl.h"
#include "OnOff.h"
#include "MultistateInputBasic.h"

typedef struct
{
    tsZCL_ClusterInstance sOnOffClient;
    tsZCL_ClusterInstance sMultistateInputServer;
} tsZHA_OnOffEndpointClusterInstances __attribute__((aligned(4)));

typedef struct
{
    tsZCL_EndPointDefinition sEndPoint;
    tsZHA_OnOffEndpointClusterInstances sClusterInstance;

    tsCLD_OnOffClient sOnOffClientCluster;
    tsCLD_MultistateInputBasic sMultistateInputServerCluster;
} tsZHA_OnOffEndpoint;

PUBLIC void APP_vRegisterOnOffEndPoints(void);

#endif /* APP_ON_OFF_EP_H */
