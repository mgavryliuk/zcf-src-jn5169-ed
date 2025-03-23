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
PUBLIC teNodeState eGetNodeState(void);
PUBLIC bool_t bNodeJoined(void);

#endif /* APP_NODE_H */
