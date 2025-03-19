#ifndef APP_NODE_H
#define APP_NODE_H

#define PDM_ID_APP_END_DEVICE 0x8

typedef enum
{
    E_NO_NETWORK,
    E_JOINED,
    E_JOINING,
    E_JOIN_FAILED
} teNodeState;

PUBLIC void APP_vInitialiseNode(void);
PUBLIC void APP_vFactoryResetRecords(void);
PUBLIC void APP_vFactoryResetRecordsAndJoin(void);
PUBLIC teNodeState eGetNodeState();
PUBLIC bool_t bNodeIsRunning();

#endif /* APP_NODE_H */
