#ifndef APP_NODE_H
#define APP_NODE_H

#define PDM_ID_APP_END_DEVICE       0x8

PUBLIC void APP_vInitialiseNode(void);

typedef enum
{
    E_STARTUP,
    E_RUNNING
} teNodeState;

#endif /* APP_NODE_H */
