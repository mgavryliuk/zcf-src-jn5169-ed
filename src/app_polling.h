#ifndef APP_POLLING_H
#define APP_POLLING_H

#ifdef DEBUG_POLL
#define TRACE_POLL TRUE
#else
#define TRACE_POLL FALSE
#endif

#define POLL_REGULAR_TIME ZTIMER_TIME_MSEC(1000)
#define POLL_FAST_TIME ZTIMER_TIME_MSEC(200)

#define POLL_COMMISSIONING_CYCLES 100

typedef enum
{
    POLL_COMMISSIONING,
    POLL_REGULAR,
    POLL_FAST,
} tePollMode;

PUBLIC void APP_vStartPolling(tePollMode ePollMode);
PUBLIC void APP_cbTimerPoll(void *pvParam);
PUBLIC bool_t APP_bPollCanBeStopped(void);
PUBLIC void APP_vHandlePollConfirm(ZPS_tsAfPollConfEvent *psAfPollConfirmEvent);

#endif /* APP_POLLING_H */
