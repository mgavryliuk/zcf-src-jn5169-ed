#ifndef APP_REPORTING_H
#define APP_REPORTING_H

#include "zcl.h"
#include "zcl_options.h"

#ifdef DEBUG_REPORT
#define TRACE_REPORT TRUE
#else
#define TRACE_REPORT FALSE
#endif

typedef struct
{
    uint16 u16ClusterID;
    bool_t bManufacturerSpecific;
    tsZCL_AttributeReportingConfigurationRecord sAttributeReportingConfigurationRecord;
} tsReports;

PUBLIC void vMakeSupportedAttributesReportable(void);

#endif /* APP_REPORTING_H */
