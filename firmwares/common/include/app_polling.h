#ifndef APP_POLLING_H
#define APP_POLLING_H

#ifdef DEBUG_POLL
#define TRACE_POLL TRUE
#else
#define TRACE_POLL FALSE
#endif

#define POLL_REGULAR_TIME ZTIMER_TIME_MSEC(1000)
#define POLL_FAST_TIME ZTIMER_TIME_MSEC(200)

#define HAS_DATA_TO_POLL_DEBOUNCE_MASK 0b111
#define HAS_DATA_TO_POLL_INITIAL_VALUE 0b010
#define POLL_COMMISSIONING_CYCLES 100

#include <jendefs.h>
#include "zps_apl_af.h"

typedef enum
{
    POLL_COMMISSIONING,
    POLL_REGULAR,
    POLL_FAST,
} tePollMode;

PUBLIC void APP_vStartPolling(tePollMode ePollMode);
PUBLIC bool_t APP_bPollCanBeStopped(void);
PUBLIC void APP_vHandlePollConfirm(ZPS_tsAfPollConfEvent *psAfPollConfirmEvent);

#endif /* APP_POLLING_H */
