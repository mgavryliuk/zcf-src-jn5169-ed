#ifndef APP_NODE_H
#define APP_NODE_H

#ifdef DEBUG_NODE
#define TRACE_NODE TRUE
#else
#define TRACE_NODE FALSE
#endif

#define PDM_ID_APP_END_DEVICE 0x8

#include <jendefs.h>

typedef enum
{
    E_NO_NETWORK,
    E_JOINED,
    E_JOINING
} teNodeState;

PUBLIC void APP_vInitialiseNode(void);
PUBLIC bool_t bNodeJoined(void);
PUBLIC teNodeState eGetNodeState(void);
PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent);
PUBLIC void APP_vFactoryResetRecords(void);

#endif /* APP_NODE_H */
