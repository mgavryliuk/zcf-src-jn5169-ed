#ifndef APP_INIT_H
#define APP_INIT_H

#include <jendefs.h>
#include "ZQueue.h"

#ifdef DEBUG_INIT
#define TRACE_INIT TRUE
#else
#define TRACE_INIT FALSE
#endif

PUBLIC void APP_vInitialise(tpfExtendedStatusCallBack pfExtendedStatusCallBack);

#endif /* APP_INIT_H */
