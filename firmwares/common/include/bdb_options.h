#ifndef BDB_OPTIONS_INCLUDED
#define BDB_OPTIONS_INCLUDED

#if defined __cplusplus
extern "C"
{
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#if (SINGLE_CHANNEL < 11 || SINGLE_CHANNEL > 26)
#define BDB_PRIMARY_CHANNEL_SET (0x02108800)                             /* bdbPrimaryChannelSet e.g. (0x02108800) */
#define BDB_SECONDARY_CHANNEL_SET (0x07FFF800 ^ BDB_PRIMARY_CHANNEL_SET) /* bdbSecondaryChannelSet e.g. (0x07FFF800 ^ BDB_PRIMARY_CHANNEL_SET) */
#else
#warning Single channel only!
#define BDB_PRIMARY_CHANNEL_SET (1 << SINGLE_CHANNEL) /* bdbPrimaryChannelSet e.g. (0x02108800)  */
#define BDB_SECONDARY_CHANNEL_SET (0)                 /* bdbSecondaryChannelSet e.g. (0x07FFF800 ^ BDB_PRIMARY_CHANNEL_SET) */
#endif

#define BDBC_IMP_MAX_REJOIN_CYCLES (10)

/* BDB Constants used by all nodes
   Note - Must not change for final app */
#define BDBC_MAX_SAME_NETWORK_RETRY_ATTEMPTS (10) /* bdbcMaxSameNetworkRetryAttempts */
#define BDBC_MIN_COMMISSIONING_TIME (180)         /* bdbcMinCommissioningTime */
#define BDBC_REC_SAME_NETWORK_RETRY_ATTEMPTS (3)  /* bdbcRecSameNetworkRetryAttempts */
#define BDBC_TC_LINK_KEY_EXCHANGE_TIMEOUT (5)     /* bdbcTCLinkKeyExchangeTimeout */

/* BDB Attribute values */
#define BDB_COMMISSIONING_GROUP_ID (0xFFFF)                          /* bdbCommissioningGroupID */
#define BDB_COMMISSIONING_MODE (BDB_COMMISSIONING_MODE_NWK_STEERING) /* bdbCommissioningMode */
#define BDB_COMMISSIONING_STATUS (0x00)                              /* bdbCommissioningStatus */
#define BDB_JOINING_NODE_EUI64 (0x0000000000000000)                  /* bdbJoiningNodeEui64 */
                                                                     /* bdbJoiningNodeNewTCLinkKey */
#ifndef BDB_JOIN_USES_INSTALL_CODE_KEY
#define BDB_JOIN_USES_INSTALL_CODE_KEY (FALSE) /* bdbJoinUsesInstallCodeKey */
                                               /* bdbNodeCommissioningCapability - This will be set according to BDB features in Makefile */
                                               /* bdbNodeIsOnANetwork - Application is responsible to set this before BDB initialisation */
#endif
#define BDB_NODE_JOIN_LINK_KEY_TYPE (0x00) /* bdbNodeJoinLinkKeyType */

#define BDB_SCAN_DURATION (0x04) /* bdbScanDuration */

#define BDB_TC_LINK_KEY_EXCHANGE_ATTEMPTS (0x00)     /* bdbTCLinkKeyExchangeAttempts */
#define BDB_TC_LINK_KEY_EXCHANGE_ATTEMPTS_MAX (0x03) /* bdbTCLinkKeyExchangeAttemptsMax */
#define BDB_TC_LINK_KEY_EXCHANGE_METHOD (0x00)       /* bdbTCLinkKeyExchangeMethod */
#define BDB_TRUST_CENTER_NODE_JOIN_TIMEOUT (0x0F)    /* bdbTrustCenterNodeJoinTimeout */
#define BDB_TRUST_CENTER_REQUIRE_KEYEXCHANGE (TRUE)  /* bdbTrustCenterRequireKeyExchange */

   /****************************************************************************/
   /***        Type Definitions                                              ***/
   /****************************************************************************/

   /****************************************************************************/
   /***        Exported Functions                                            ***/
   /****************************************************************************/

   /****************************************************************************/
   /***        Exported Variables                                            ***/
   /****************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* BDB_OPTIONS_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
