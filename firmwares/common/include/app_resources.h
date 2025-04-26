#ifndef APP_RESOURCES_H
#define APP_RESOURCES_H

#include <jendefs.h>
#include "ZQueue.h"

#ifdef DEBUG_RESOURCES
#define TRACE_RESOURCES TRUE
#else
#define TRACE_RESOURCES FALSE
#endif

#ifndef APP_ZTIMER_STORAGE
#define APP_ZTIMER_STORAGE 4
#endif

#define BDB_QUEUE_SIZE 5
#define TIMER_QUEUE_SIZE 8
#define MLME_QUEQUE_SIZE 8
#define MCPS_QUEUE_SIZE 20
#define MCPS_DCFM_QUEUE_SIZE 5

#define APP_QUEUE_SIZE 10

#define ZCL_TICK_TIME_MSEC ZTIMER_TIME_MSEC(1000)
#define BUTTON_SCAN_TIME_MSEC ZTIMER_TIME_MSEC(10)

extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfm;
extern PUBLIC tszQueue zps_TimeEvents;

extern PUBLIC tszQueue APP_msgBdbEvents;
extern PUBLIC tszQueue APP_msgAppEvents;

extern PUBLIC uint8 u8TimerLedBlink;
extern PUBLIC uint8 u8TimerButtonScan;
extern PUBLIC uint8 u8TimerPoll;
extern PUBLIC uint8 u8TimerZclTick;

PUBLIC void APP_vInitResources(void);
PUBLIC void APP_cbTimerZclTick(void *pvParam);
PUBLIC void APP_cbTimerPoll(void *pvParam);
PUBLIC void APP_cbTimerButtonScan(void *pvParam);
PUBLIC void APP_cbTimerLedBlink(void *pvParam);

#endif /* APP_RESOURCES_H */
