#include "dbg.h"
#include "zcl.h"
#include "PowerConfiguration.h"

#include "app_common.h"
#include "zcl_options.h"
#include "app_reporting.h"
#include "ConfigurationCluster.h"

tsReports asDefaultReports[ZCL_NUMBER_OF_REPORTS] = {
    {
        GENERAL_CLUSTER_ID_POWER_CONFIGURATION,
        FALSE,
        {
            0,
            E_ZCL_UINT8,
            E_CLD_PWRCFG_ATTR_ID_BATTERY_VOLTAGE,
            ZCL_SYSTEM_MIN_REPORT_INTERVAL,
            ZCL_SYSTEM_MAX_REPORT_INTERVAL,
            0,
            {1},
        },
    },
    {
        GENERAL_CLUSTER_ID_POWER_CONFIGURATION,
        FALSE,
        {
            0,
            E_ZCL_UINT8,
            E_CLD_PWRCFG_ATTR_ID_BATTERY_PERCENTAGE_REMAINING,
            ZCL_SYSTEM_MIN_REPORT_INTERVAL,
            ZCL_SYSTEM_MAX_REPORT_INTERVAL,
            0,
            {1},
        },
    },
};

PUBLIC void vMakeSupportedAttributesReportable(void)
{
    uint16 u16AttributeEnum;
    uint16 u16ClusterId;
    bool_t bManufacturerSpecific;
    teZCL_Status eStatus;
    int i;

    tsZCL_AttributeReportingConfigurationRecord *psAttributeReportingConfigurationRecord;

    for (i = 0; i < ZCL_NUMBER_OF_REPORTS; i++)
    {
        u16AttributeEnum = asDefaultReports[i].sAttributeReportingConfigurationRecord.u16AttributeEnum;
        u16ClusterId = asDefaultReports[i].u16ClusterID;
        bManufacturerSpecific = asDefaultReports[i].bManufacturerSpecific;
        psAttributeReportingConfigurationRecord = &(asDefaultReports[i].sAttributeReportingConfigurationRecord);
        DBG_vPrintf(TRACE_REPORT, "REPORT: Cluster %04x Attribute %04x Min %d Max %d IntV %d Direct %d Change %d ManuSpecific %d\n",
                    u16ClusterId,
                    u16AttributeEnum,
                    asDefaultReports[i].sAttributeReportingConfigurationRecord.u16MinimumReportingInterval,
                    asDefaultReports[i].sAttributeReportingConfigurationRecord.u16MaximumReportingInterval,
                    asDefaultReports[i].sAttributeReportingConfigurationRecord.u16TimeoutPeriodField,
                    asDefaultReports[i].sAttributeReportingConfigurationRecord.u8DirectionIsReceived,
                    asDefaultReports[i].sAttributeReportingConfigurationRecord.uAttributeReportableChange.zint8ReportableChange,
                    bManufacturerSpecific);
        eStatus = eZCL_CreateLocalReport(BASIC_ENDPOINT, u16ClusterId, bManufacturerSpecific, TRUE, psAttributeReportingConfigurationRecord);
        if (eStatus != E_ZCL_SUCCESS)
        {
            DBG_vPrintf(TRACE_REPORT, "REPORT: eZCL_CreateLocalReport failed with status %d\n", eStatus);
        }
    }
}
