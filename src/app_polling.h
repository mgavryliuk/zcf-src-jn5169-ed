#ifndef APP_POLLING_H
#define APP_POLLING_H

#ifdef DEBUG_POLL
#define TRACE_POLL TRUE
#else
#define TRACE_POLL FALSE
#endif

#define POLL_REGULAR_TIME ZTIMER_TIME_MSEC(2000)
#define POLL_FAST_TIME ZTIMER_TIME_MSEC(200)

#define POLL_REGULAR_CYCLES 5
#define POLL_FAST_CYCLES 50

typedef enum
{
    POLL_REGULAR,
    POLL_FAST,
} tePollMode;

PUBLIC void APP_vStartPolling(tePollMode ePollMode);
PUBLIC void APP_vStopPolling(void);
PUBLIC void APP_cbTimerPoll(void *pvParam);

#endif /* APP_POLLING_H */
