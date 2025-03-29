#ifndef APP_NODE_H
#define APP_NODE_H

#include "ZTimer.h"

#ifdef DEBUG_NODE
#define TRACE_NODE TRUE
#else
#define TRACE_NODE FALSE
#endif

#ifdef DEBUG_NODE_BDB
#define TRACE_NODE_BDB TRUE
#else
#define TRACE_NODE_BDB FALSE
#endif

#ifdef DEBUG_NODE_AF_CB
#define TRACE_NODE_AF_CB TRUE
#else
#define TRACE_NODE_AF_CB FALSE
#endif

#ifdef DEBUG_NODE_ZDO
#define TRACE_NODE_ZDO TRUE
#else
#define TRACE_NODE_ZDO FALSE
#endif

#define PDM_ID_APP_END_DEVICE 0x8

typedef enum
{
    E_NO_NETWORK,
    E_JOINED,
    E_JOINING
} teNodeState;

PUBLIC void APP_vInitialiseNode(void);
PUBLIC void APP_vFactoryResetRecords(void);
PUBLIC teNodeState eGetNodeState(void);
PUBLIC bool_t bNodeJoined(void);
PUBLIC void APP_cbTimerZclTick(void *pvParam);

#endif /* APP_NODE_H */
