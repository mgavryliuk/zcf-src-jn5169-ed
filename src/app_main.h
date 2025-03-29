#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "ZQueue.h"

#ifdef DEBUG_MAIN
#define TRACE_MAIN TRUE
#else
#define TRACE_MAIN FALSE
#endif

#define APP_ZTIMER_STORAGE 4
#define BDB_QUEUE_SIZE 3
#define TIMER_QUEUE_SIZE 8
#define MLME_QUEQUE_SIZE 8
#define MCPS_QUEUE_SIZE 20
#define MCPS_DCFM_QUEUE_SIZE 5

#define APP_QUEUE_SIZE 10
#define MAXIMUM_TIME_TO_SLEEP_SEC (60 * 60)
#define ZCL_TICK_TIME ZTIMER_TIME_MSEC(1000)

extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfm;
extern PUBLIC tszQueue zps_TimeEvents;

extern PUBLIC tszQueue APP_msgAppEvents;
extern PUBLIC tszQueue APP_msgBdbEvents;

extern PUBLIC uint8 u8LedBlinkTimer;
extern PUBLIC uint8 u8TimerButtonScan;
extern PUBLIC uint8 u8TimerPoll;
extern PUBLIC uint8 u8TimerTick;

PUBLIC void APP_vInitResources(void);
PUBLIC void APP_vInitialise(void);
PUBLIC void APP_vMainLoop(void);

#endif /* APP_MAIN_H */
