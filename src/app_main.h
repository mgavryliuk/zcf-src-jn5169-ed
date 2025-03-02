#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "bdb_api.h"

#define APP_ZTIMER_STORAGE          3
#define BDB_QUEUE_SIZE              3
#define TIMER_QUEUE_SIZE            8
#define MLME_QUEQUE_SIZE            8
#define MCPS_QUEUE_SIZE             20
#define MCPS_DCFM_QUEUE_SIZE 		5

PUBLIC void APP_vInitResources(void);
PUBLIC void APP_vInitialise(void);
PUBLIC void APP_vMainLoop(void);

extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfm;
extern PUBLIC tszQueue zps_TimeEvents;

extern PUBLIC tszQueue APP_msgBdbEvents;

extern PUBLIC uint8 u8LedBlinkTimer;

#endif /* APP_MAIN_H */
