#ifndef _rtr_rdm_domains_h
#define _rtr_rdm_domains_h

#ifdef __cplusplus
extern "C" {
#endif


#include "rtr/rsslTypes.h"


/**
 *	@addtogroup RSSLWFDomain
 *	@{
 */
 

/** 
 * @brief Domain Type enumeration values, see Usage Guide for domain model definitions (DMT = DomainType)
 * This will be extended as new message types are defined and implemented
 * @see RsslMsgBase
 */
typedef enum {
	RSSL_DMT_LOGIN						= 1,    /*!< (1) Login Message */
	RSSL_DMT_SOURCE						= 4,	/*!< (4) Source Message */
	RSSL_DMT_DICTIONARY					= 5,    /*!< (5) Dictionary Message */
	RSSL_DMT_MARKET_PRICE				= 6,    /*!< (6) Market Price Message */
	RSSL_DMT_MARKET_BY_ORDER			= 7,	/*!< (7) Market by Order/Order Book Message */
	RSSL_DMT_MARKET_BY_PRICE			= 8,	/*!< (8) Market by Price/Market Depth Message */
	RSSL_DMT_MARKET_MAKER				= 9,	/*!< (9) Market Maker Message */
	RSSL_DMT_SYMBOL_LIST				= 10,	/*!< (10) Symbol List Messages */
	RSSL_DMT_SERVICE_PROVIDER_STATUS	= 11,	/*!< (11) Service Provider Status domain, used with Elektron Pulse */
	RSSL_DMT_HISTORY					= 12,   /*!< (12) History Message */
	RSSL_DMT_HEADLINE					= 13,   /*!< (13) Headline Message */
	RSSL_DMT_STORY						= 14,	/*!< (14) Story Message */
	RSSL_DMT_REPLAYHEADLINE				= 15,	/*!< (15) Replay Headline Message */
	RSSL_DMT_REPLAYSTORY				= 16,	/*!< (16) Replay Story Message */
	RSSL_DMT_TRANSACTION				= 17,	/*!< (17) Transaction Message */
	RSSL_DMT_YIELD_CURVE				= 22,	/*!< (22) Yield Curve */
	RSSL_DMT_CONTRIBUTION				= 27,	/*!< (27) Contribution domain for user-stream contributions */
	RSSL_DMT_PROVIDER_ADMIN				= 29,
	RSSL_DMT_ANALYTICS					= 30,	/*!< (30) Analitics content */
	RSSL_DMT_REFERENCE					= 31,	/*!< (31) Reference content */
	RSSL_DMT_NEWS_TEXT_ANALYTICS		= 33,	/*!< (33) News Text Analytics domain for machine readable news content */
	RSSL_DMT_ECONOMIC_INDICATOR			= 34,	/*!< (34) Economic Indicator content */
	RSSL_DMT_POLL						= 35,   /*!< (35) Poll content */
	RSSL_DMT_SYSTEM						= 127,  /*!< (127) System domain for use with domain netural content (e.g. tunnel stream creation) */
	RSSL_DMT_MAX_RESERVED				= 127,	/*!< (127) Maximum reserved message type value */
	RSSL_DMT_MAX_VALUE					= 255	/*!< (255) Maximum value for a message type */
} RsslDomainTypes;

 
/**
 *	@defgroup DomainCommon TRDM Common Enumerations
 *	@brief	The TRDM Common Enumerations are used across several Thomson Reuters Domain Models.
 *	@{
 */


typedef enum
{
	RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED		= 0,	/*!< (0) Symbology is not specified or not applicable */
	RDM_INSTRUMENT_NAME_TYPE_RIC				= 1,	/*!< (1) Reuters Instrument Code */
	RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR		= 2,	/*!< (2) Contributor identification information for user-stream contributions (RSSL_DMT_CONTRIBUTION) */
	RDM_INSTRUMENT_NAME_TYPE_MAX_RESERVED		= 127	/*!< (127) Maximum reserved Quote Symbology */
} RDMInstrumentNameTypes;


typedef enum
{
	RDM_UPD_EVENT_TYPE_UNSPECIFIED		= 0,	/*!< (0) Unspecified Update Event */
	RDM_UPD_EVENT_TYPE_QUOTE			= 1,	/*!< (1) Update Event Quote */
	RDM_UPD_EVENT_TYPE_TRADE			= 2,	/*!< (2) Update Event Trade */
	RDM_UPD_EVENT_TYPE_NEWS_ALERT		= 3,	/*!< (3) Update Event News Alert */
	RDM_UPD_EVENT_TYPE_VOLUME_ALERT		= 4,	/*!< (4) Update Event Volume Alert */
	RDM_UPD_EVENT_TYPE_ORDER_INDICATION	= 5,	/*!< (5) Update Event Order Indication */
	RDM_UPD_EVENT_TYPE_CLOSING_RUN		= 6,	/*!< (6) Update Event Closing Run */
	RDM_UPD_EVENT_TYPE_CORRECTION		= 7,	/*!< (7) Update Event Correction */
	RDM_UPD_EVENT_TYPE_MARKET_DIGEST	= 8,	/*!< (8) Update Event Market Digest */
	RDM_UPD_EVENT_TYPE_QUOTES_TRADE		= 9,	/*!< (9) Update Event Quotes followed by a Trade */
	RDM_UPD_EVENT_TYPE_MULTIPLE			= 10,	/*!< (10) Update Event with filtering and conflation applied */
	RDM_UPD_EVENT_TYPE_VERIFY			= 11,	/*!< (11) Fields may have changed */
	RDM_UPD_EVENT_TYPE_MAX_RESERVED		= 127	/*!< (127) Maximum reserved update event type */
} RDMUpdateEventTypes;


typedef enum {
	RDM_VIEW_TYPE_FIELD_ID_LIST				= 1,	/*!< (1) View Data contains a list of Field IDs */
	RDM_VIEW_TYPE_ELEMENT_NAME_LIST			= 2		/*!< (2) View Data contains a list of Element Names */
} RDMViewTypes;

/**
 *	@}
 */

/**
 *	@defgroup DomainLogin TRDM Login Domain
 *	@brief The Login Domain is used to register a user with the system.
 *	@{
 */


typedef enum {
    RDM_LOGIN_ROLE_CONS           = 0, /*!< (0) Application logs in as a consumer */
    RDM_LOGIN_ROLE_PROV           = 1  /*!< (1) Application logs in as a provider */
} RDMLoginRoleTypes;


typedef enum {
	RDM_LOGIN_USER_NAME				= 1,	/*!< (1) Name */
	RDM_LOGIN_USER_EMAIL_ADDRESS	= 2,	/*!< (2) Email address */
	RDM_LOGIN_USER_TOKEN			= 3,	/*!< (3) User Token, typically a Triple A (AAA) token. */
	RDM_LOGIN_USER_COOKIE			= 4		/*!< (4) User information is specified in a cookie.  */
} RDMLoginUserIdTypes;

typedef enum {
	RDM_LOGIN_SERVER_TYPE_ACTIVE	= 0,	/*!< (0) Active Server */
	RDM_LOGIN_SERVER_TYPE_STANDBY	= 1		/*!< (1) Standby Server */
} RDMLoginServerTypes;

typedef enum
{            
	RDM_LOGIN_BATCH_NONE				= 0x0,  /*!< (0x0) Provider does not support batching. */
	RDM_LOGIN_BATCH_SUPPORT_REQUESTS	= 0x1,  /*!< (0x1) Provider supports batch requests. */
	RDM_LOGIN_BATCH_SUPPORT_REISSUES	= 0x2,  /*!< (0x2) Provider supports batch reissue requests. */
	RDM_LOGIN_BATCH_SUPPORT_CLOSES		= 0x4   /*!< (0x4) Provider supports batch closes. */
} RDMLoginBatchSupportFlags;

/**
 *	@}
 */
 


/**
 *	@defgroup DomainDictionary TRDM Dictionary Domain
 *	@brief The Dictionary Domain is used transmit the RDM Field Dictionary across the wire.
 *	@{
 */

/**
 * @brief Enumerations describing the Type of a particular dictionary.
 * These values are associated with the "Type" tag of a dictionary, found in the associated file or summary data.
 * Functions for loading or decoding these dictionaries will look for this information and use it to verify that the correct type of dictionary is being interpreted.
 * @see RsslDataDictionary
 */
typedef enum
{
        RDM_DICTIONARY_FIELD_DEFINITIONS        = 1,  /*!< (1) Field Dictionary type, typically referring to an RDMFieldDictionary */
        RDM_DICTIONARY_ENUM_TABLES              = 2,  /*!< (2) Enumeration Dictionary type, typically referring to an enumtype.def */
        RDM_DICTIONARY_RECORD_TEMPLATES         = 3,  /*!< (3) Record template type, typically referring to a template to help with caching of data - can be referred to by fieldListNum or elemListNum */
        RDM_DICTIONARY_DISPLAY_TEMPLATES        = 4,  /*!< (4) Display template type, typically provides information about displaying data (e.g. position on screen, etc) */
        RDM_DICTIONARY_DATA_DEFINITIONS         = 5,  /*!< (5) Set Data Definition type, contains data definitions that would apply globally to any messages sent or received from the provider of the dictionary */
        RDM_DICTIONARY_STYLE_SHEET              = 6,  /*!< (6) Style sheet type, can be used to send style information */
        RDM_DICTIONARY_REFERENCE                = 7,  /*!< (7) Dictionary reference type, additional dictionary information */
        RDM_DICTIONARY_FIELD_SET_DEFINITION     = 8,  /*!< (8) Global Field Set Definition type */
        RDM_DICTIONARY_ELEMENT_SET_DEFINITION   = 9   /*!< (9) Global Element Set Definition type */
} RDMDictionaryTypes;

/**
 * @brief Enumerations describing how much information about a particular dictionary is desired.
 * These values are typically set in an RsslRequestMsg MsgKey filter when the request for the dictionary is made.
 * See the UPA RDM Usage Guide for details.
 * @see RsslDataDictionary, RsslMsgKey
 */
typedef enum
{
    RDM_DICTIONARY_INFO				= 0x00,  /*!< (0x00) "Dictionary Info" Verbosity, no data - version information only */
    RDM_DICTIONARY_MINIMAL          = 0x03,  /*!< (0x03) "Minimal" Verbosity, e.g. Cache + ShortName */
    RDM_DICTIONARY_NORMAL           = 0x07,  /*!< (0x07) "Normal" Verbosity, e.g. all but description */
    RDM_DICTIONARY_VERBOSE          = 0x0F   /*!< (0x0F) "Verbose" Verbosity, e.g. all with description */
} RDMDictionaryVerbosityValues;

/**
 *	@defgroup RSSLWFDict RDMFieldDictionary and enumtype.def Dictionary loading, encoding, and decoding helpers. 
 *	@brief Utility structures and functions that can be used to handle the loading, encoding, and decoding of the RDMFieldDictionary and enumtype.def dictionary.  These also allow for performant field entry lookups.  
 *	@{
 *	@}
 */


/**
 *	@}
 */
 
/**
 *	@defgroup DomainDirectory TRDM Directory Domain
 *	@brief The Directory Domain is used to communicate information about services available on the system.
 *	@{
 */


typedef enum {
	RDM_DIRECTORY_SERVICE_INFO_FILTER				= 0x00000001,   /*!< (0x00000001) Source Info Filter Mask */
	RDM_DIRECTORY_SERVICE_STATE_FILTER				= 0x00000002,   /*!< (0x00000002) Source State Filter Mask */
	RDM_DIRECTORY_SERVICE_GROUP_FILTER				= 0x00000004,	/*!< (0x00000004) Source Group Filter Mask */
	RDM_DIRECTORY_SERVICE_LOAD_FILTER				= 0x00000008,	/*!< (0x00000008) Source Load Filter Mask*/
	RDM_DIRECTORY_SERVICE_DATA_FILTER				= 0x00000010,	/*!< (0x00000010) Source Data Filter Mask */
	RDM_DIRECTORY_SERVICE_LINK_FILTER				= 0x00000020,	/*!< (0x00000020) Source Communication Link Information */
	RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER				= 0x00000040	/*!< (0x00000040) Source Sequenced Multicast Information */
} RDMDirectoryServiceFilterFlags;

typedef enum {
	RDM_DIRECTORY_SERVICE_INFO_ID				= 1,    /*!< (1) Service Info Filter ID */
	RDM_DIRECTORY_SERVICE_STATE_ID				= 2,    /*!< (2) Source State Filter ID */
	RDM_DIRECTORY_SERVICE_GROUP_ID				= 3,	/*!< (3) Source Group Filter ID */
	RDM_DIRECTORY_SERVICE_LOAD_ID				= 4,	/*!< (4) Source Load Filter ID */
	RDM_DIRECTORY_SERVICE_DATA_ID				= 5,	/*!< (5) Source Data Filter ID */
	RDM_DIRECTORY_SERVICE_LINK_ID				= 6,	/*!< (6) Communication Link Filter ID */
	RDM_DIRECTORY_SERVICE_SEQ_MCAST_ID			= 7	/*!< (7) Sequenced Multicast Filter ID */
} RDMDirectoryServiceFilterIds;

typedef enum {
	RDM_DIRECTORY_SERVICE_STATE_DOWN			= 0,	/*!< (0) Service state down */
	RDM_DIRECTORY_SERVICE_STATE_UP				= 1		/*!< (1) Service state up */
} RDMDirectoryServiceStates;

typedef enum {
	RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY		= 0,	/*!< (0) Indicates the upstream provider is the active and there is no standby provider */
	RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY	= 1,	/*!< (1) Indicates the upstream provider is the active and there is a standby provider */
	RDM_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY				= 2 	/*!< (2) Indicates the upstream provider is a standby */
}  RDMDirectorySourceMirroringMode;

typedef enum {
	RDM_DIRECTORY_DATA_TYPE_MIN_RESERVED	= 0,		/*!< (0) Minimum reserved Data Type */
	RDM_DIRECTORY_DATA_TYPE_NONE			= 0,		/*!< (0) None */
	RDM_DIRECTORY_DATA_TYPE_TIME			= 1,		/*!< (1) Time */
	RDM_DIRECTORY_DATA_TYPE_ALERT			= 2,		/*!< (2) Alert */
	RDM_DIRECTORY_DATA_TYPE_HEADLINE		= 3,		/*!< (3) Headline */
	RDM_DIRECTORY_DATA_TYPE_STATUS			= 4,		/*!< (4) Status */
	RDM_DIRECTORY_DATA_TYPE_MAX_RESERVED	= 1023		/*!< (1023) Maximum reserved Data Type */
} RDMDirectoryDataTypes;


typedef enum {
	RDM_DIRECTORY_LINK_TYPE_INTERACTIVE		= 1,		/*!< (1) Interactive */
	RDM_DIRECTORY_LINK_TYPE_BROADCAST		= 2			/*!< (2) Broadcast */
} RDMDirectoryLinkTypes;

typedef enum {
	RDM_DIRECTORY_LINK_CODE_NONE				= 0,	/*!< (0) None */
	RDM_DIRECTORY_LINK_CODE_OK					= 1,	/*!< (1) Ok */
	RDM_DIRECTORY_LINK_CODE_RECOVERY_STARTED	= 2,	/*!< (2) Recovery Started */
	RDM_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED	= 3		/*!< (3) Recovery Completed */
} RDMDirectoryLinkCodes;
/**
 *	@}
 */

/**
 * @brief Enhanced Symbol List behavior support flags.
 */
typedef enum
{
	RDM_SYMBOL_LIST_SUPPORT_NAMES_ONLY		= 0x0,	/*!< (0x0) Supports names only, no additional functionality. */
	RDM_SYMBOL_LIST_SUPPORT_DATA_STREAMS	= 0x1	/*!< (0x1) Supports symbol list data streams. */
} RDMEnhancedSymbolListSupportFlags;
 
/**
 * @brief Symbol List requestable behavior flags.
 */
typedef enum
{
	RDM_SYMBOL_LIST_NAMES_ONLY		= 0x0,	/*!< (0x0) Request names only. */
	RDM_SYMBOL_LIST_DATA_STREAMS 	= 0x1,	/*!< (0x1) Request symbol list data streams. */
	RDM_SYMBOL_LIST_DATA_SNAPSHOTS 	= 0x2	/*!< (0x2) Request symbol list data snapshots. */
} RDMSymbolListDataStreamRequestFlags;

/**
 *	@defgroup ClassOfService  Class of Service Elements
 *	@brief Class of Service elements are used for negotiating features of a qualified stream.
 *	@{
 */

/**
  * @brief Class of Service property filter IDs.
  */
typedef enum
{
	RDM_COS_COMMON_PROPERTIES_ID	= 1,	/*!< (1) Class of Service Common Properties Filter ID */
	RDM_COS_AUTHENTICATION_ID		= 2,	/*!< (2) Class of Service Authentication Filter ID */
	RDM_COS_FLOW_CONTROL_ID			= 3,	/*!< (3) Class of Service Flow Control Filter ID */
	RDM_COS_DATA_INTEGRITY_ID		= 4,	/*!< (4) Class of Service Data Integrity Filter ID */
	RDM_COS_GUARANTEE_ID			= 5		/*!< (5) Class of Service Guarantee Filter ID */
} RDMClassOfServiceFilterIDs;

/**
  * @brief Class of Service property filter flags.
  */
typedef enum
{
	RDM_COS_COMMON_PROPERTIES_FLAG	= 0x01,	/*!< (0x01) Class of Service Common Properties Filter Flag */
	RDM_COS_AUTHENTICATION_FLAG		= 0x02,	/*!< (0x02) Class of Service Authentication Filter Flag */
	RDM_COS_FLOW_CONTROL_FLAG		= 0x04,	/*!< (0x04) Class of Service Flow Control Filter Flag */
	RDM_COS_DATA_INTEGRITY_FLAG		= 0x08,	/*!< (0x08) Class of Service Data Integrity Filter Flag */
	RDM_COS_GUARANTEE_FLAG			= 0x10 	/*!< (0x10) Class of Service Guarantee Filter Flag */
} RDMClassOfServiceFilterFlags;

/**
  * @brief Class of Service authentication types.
  */
typedef enum
{
	RDM_COS_AU_NOT_REQUIRED	= 0, /*!< (0) Authentication is not required. */
	RDM_COS_AU_OMM_LOGIN	= 1  /*!< (1) A login message is exchanged to authenticate the stream. */
} RDMClassOfServiceAuthenticationType;

/**
  * @brief Class of Service flow control types.
  */
typedef enum
{
	RDM_COS_FC_NONE				= 0, /*!< (0) None */
	RDM_COS_FC_BIDIRECTIONAL	= 1  /*!< (1) Flow control is performed in both directions. */
} RDMClassOfServiceFlowControlType;

/**
  * @brief Class of Service data integrity types.
  */
typedef enum {
	RDM_COS_DI_BEST_EFFORT		= 0, /*!< (0) Delivery of messages is best-effort. */
	RDM_COS_DI_RELIABLE			= 1  /*!< (1) Messages are reliably delivered. */
} RDMClassOfServiceDataIntegrityType;

/**
  * @brief Class of Service guarantee types.
  */
typedef enum
{
	RDM_COS_GU_NONE				= 0, /*!< (0) None */
	RDM_COS_GU_PERSISTENT_QUEUE	= 1  /*!< (1) Messages for queue streams are persisted for guaranteed delivery. */
} RDMClassOfServiceGuaranteeType;

/**
 * @}
 */


// RDMUser - Well known Element Names
static const RsslBuffer RSSL_ENAME_APPID = { 13 , (char*)"ApplicationId" };
static const RsslBuffer RSSL_ENAME_APPNAME = { 15 , (char*)"ApplicationName" };
static const RsslBuffer RSSL_ENAME_APPAUTH_TOKEN = { 29 , (char*)"ApplicationAuthorizationToken" };
static const RsslBuffer RSSL_ENAME_POSITION = { 8 , (char*)"Position" };
static const RsslBuffer RSSL_ENAME_PASSWORD = { 8 , (char*)"Password" };
static const RsslBuffer RSSL_ENAME_PROV_PERM_PROF = { 24 , (char*)"ProvidePermissionProfile" };
static const RsslBuffer RSSL_ENAME_PROV_PERM_EXP = { 28 , (char*)"ProvidePermissionExpressions" };
static const RsslBuffer RSSL_ENAME_ALLOW_SUSPECT_DATA = { 16 , (char*)"AllowSuspectData" };
static const RsslBuffer RSSL_ENAME_SINGLE_OPEN = { 10 , (char*)"SingleOpen" };
static const RsslBuffer RSSL_ENAME_SUPPORT_PR = { 18 , (char*)"SupportPauseResume" };
static const RsslBuffer RSSL_ENAME_SUPPORT_OPR = { 27 , (char*)"SupportOptimizedPauseResume" };
static const RsslBuffer RSSL_ENAME_SUPPORT_POST = { 14 , (char*)"SupportOMMPost" };
static const RsslBuffer RSSL_ENAME_SUPPORT_BATCH = { 20 , (char*)"SupportBatchRequests" };
static const RsslBuffer RSSL_ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = { 33 , (char*)"SupportProviderDictionaryDownload" };
static const RsslBuffer RSSL_ENAME_SUPPORT_ENH_SL = { 25 , (char*)"SupportEnhancedSymbolList" };
static const RsslBuffer RSSL_ENAME_SUPPORT_VIEW = { 19 , (char*)"SupportViewRequests" };
static const RsslBuffer RSSL_ENAME_INST_ID = { 10 , (char*)"InstanceId" };
static const RsslBuffer RSSL_ENAME_ROLE = { 4 , (char*)"Role" };
static const RsslBuffer RSSL_ENAME_PERSISTENT_MOUNT = { 15 , (char*)"PersistentMount" };
static const RsslBuffer RSSL_ENAME_SEQUENCE_RETRY_INTERVAL = { 21 , (char*)"SequenceRetryInterval" };
static const RsslBuffer RSSL_ENAME_UPDATE_BUFFER_LIMIT = { 17 , (char*)"UpdateBufferLimit" };
static const RsslBuffer RSSL_ENAME_SEQUENCE_NUMBER_RECOVERY = { 22 , (char*)"SequenceNumberRecovery" };

//Warm Standby - Well known Element Names
static const RsslBuffer RSSL_ENAME_SUPPORT_STANDBY = { 14 , (char*)"SupportStandby" };
static const RsslBuffer RSSL_ENAME_WARMSTANDBY_INFO = { 15 , (char*)"WarmStandbyInfo" };
static const RsslBuffer RSSL_ENAME_WARMSTANDBY_MODE = { 15 , (char*)"WarmStandbyMode" };
static const RsslBuffer RSSL_ENAME_CONS_CONN_STATUS = { 24 , (char*)"ConsumerConnectionStatus" };

//Connection Load Balancing - Well known Element Names
static const RsslBuffer RSSL_ENAME_DOWNLOAD_CON_CONFIG = { 24 , (char*)"DownloadConnectionConfig" };
static const RsslBuffer RSSL_ENAME_CONNECTION_CONFIG = { 16 , (char*)"ConnectionConfig" };
static const RsslBuffer RSSL_ENAME_NUM_STANDBY_SERVERS = { 17 , (char*)"NumStandbyServers" };
static const RsslBuffer RSSL_ENAME_HOSTNAME = { 8 , (char*)"Hostname" };
static const RsslBuffer RSSL_ENAME_PORT = { 4 , (char*)"Port" };
static const RsslBuffer RSSL_ENAME_SERVER_TYPE = { 10 , (char*)"ServerType" };
static const RsslBuffer RSSL_ENAME_SYSTEM_ID = { 8 , (char*)"SystemID" };

// RDMService - Well known Element Names
static const RsslBuffer RSSL_ENAME_NAME = { 4 , (char*)"Name" };
static const RsslBuffer RSSL_ENAME_VENDOR = { 6 , (char*)"Vendor" };
static const RsslBuffer RSSL_ENAME_IS_SOURCE = { 8 , (char*)"IsSource" };
static const RsslBuffer RSSL_ENAME_CAPABILITIES = { 12 , (char*)"Capabilities" };
static const RsslBuffer RSSL_ENAME_DICTIONARYS_PROVIDED = { 20 , (char*)"DictionariesProvided" };
static const RsslBuffer RSSL_ENAME_DICTIONARYS_USED = { 16 , (char*)"DictionariesUsed" };
static const RsslBuffer RSSL_ENAME_DICTIONARIES_PROVIDED = { 20 , (char*)"DictionariesProvided" };
static const RsslBuffer RSSL_ENAME_DICTIONARIES_USED = { 16 , (char*)"DictionariesUsed" };
static const RsslBuffer RSSL_ENAME_QOS = { 3 , (char*)"QoS" };
static const RsslBuffer RSSL_ENAME_SUPPS_QOS_RANGE = { 16 , (char*)"SupportsQoSRange" };
static const RsslBuffer RSSL_ENAME_ITEM_LIST = { 8 , (char*)"ItemList" };
static const RsslBuffer RSSL_ENAME_SUPPS_OOB_SNAPSHOTS = { 26 , (char*)"SupportsOutOfBandSnapshots" };
static const RsslBuffer RSSL_ENAME_ACCEPTING_CONS_STATUS = { 23 , (char*)"AcceptingConsumerStatus" };
static const RsslBuffer RSSL_ENAME_SOURCE_MIROR_MODE = { 19 , (char*)"SourceMirroringMode" };
static const RsslBuffer RSSL_ENAME_CONS_STATUS = { 14 , (char*)"ConsumerStatus" };
static const RsslBuffer RSSL_ENAME_SVC_STATE = { 12 , (char*)"ServiceState" };
static const RsslBuffer RSSL_ENAME_ACCEPTING_REQS = { 17 , (char*)"AcceptingRequests" };
static const RsslBuffer RSSL_ENAME_STATUS = { 6 , (char*)"Status" };
static const RsslBuffer RSSL_ENAME_GROUP = { 5 , (char*)"Group" };
static const RsslBuffer RSSL_ENAME_MERG_TO_GRP = { 13 , (char*)"MergedToGroup" };
static const RsslBuffer RSSL_ENAME_OPEN_LIMIT = { 9 , (char*)"OpenLimit" };
static const RsslBuffer RSSL_ENAME_OPEN_WINDOW = { 10 , (char*)"OpenWindow" };
static const RsslBuffer RSSL_ENAME_LOAD_FACT = { 10 , (char*)"LoadFactor" };
static const RsslBuffer RSSL_ENAME_TYPE = { 4 , (char*)"Type" };
static const RsslBuffer RSSL_ENAME_DATA = { 4 , (char*)"Data" };
static const RsslBuffer RSSL_ENAME_LINK_STATE = { 9 , (char*)"LinkState" };
static const RsslBuffer RSSL_ENAME_LINK_CODE = { 8 , (char*)"LinkCode" };
static const RsslBuffer RSSL_ENAME_TEXT = { 4 , (char*)"Text" };
static const RsslBuffer RSSL_ENAME_VERSION = { 7 , (char*)"Version" };
static const RsslBuffer RSSL_ENAME_REFERENCE_DATA_SERVER_HOST   = { 23, (char*)"ReferenceDataServerHost" };
static const RsslBuffer RSSL_ENAME_REFERENCE_DATA_SERVER_PORT   = { 23, (char*)"ReferenceDataServerPort" };
static const RsslBuffer RSSL_ENAME_SNAPSHOT_SERVER_HOST         = { 18, (char*)"SnapshotServerHost" };
static const RsslBuffer RSSL_ENAME_SNAPSHOT_SERVER_PORT         = { 18, (char*)"SnapshotServerPort" };
static const RsslBuffer RSSL_ENAME_GAP_RECOVERY_SERVER_HOST     = { 21, (char*)"GapRecoveryServerHost" };
static const RsslBuffer RSSL_ENAME_GAP_RECOVERY_SERVER_PORT     = { 21, (char*)"GapRecoveryServerPort" };
static const RsslBuffer RSSL_ENAME_STREAMING_MCAST_CHANNELS	= { 26, (char*)"StreamingMulticastChannels" };
static const RsslBuffer RSSL_ENAME_GAP_MCAST_CHANNELS		= { 20, (char*)"GapMulticastChannels" };
static const RsslBuffer RSSL_ENAME_ADDRESS                      = {  7, (char*)"Address" };
static const RsslBuffer RSSL_ENAME_DOMAIN                       = {  6, (char*)"Domain" };
static const RsslBuffer RSSL_ENAME_MULTICAST_GROUP              = { 14, (char*)"MulticastGroup" };

// Dictionary - Well known element names
static const RsslBuffer RSSL_ENAME_DICTIONARY_ID = { 12, (char*)"DictionaryId" };
static const RsslBuffer	RSSL_ENAME_DICT_TYPE = { 4, (char*)"Type" };
static const RsslBuffer	RSSL_ENAME_DICT_VERSION = { 7, (char*)"Version" };
static const RsslBuffer RSSL_ENAME_FIELD_NAME = {4, (char*)"NAME"};
static const RsslBuffer RSSL_ENAME_FIELD_ID = {3, (char*)"FID"};
static const RsslBuffer RSSL_ENAME_FIELD_RIPPLETO = {8, (char*)"RIPPLETO"};
static const RsslBuffer	RSSL_ENAME_FIELD_TYPE = { 4, (char*)"TYPE" };
static const RsslBuffer RSSL_ENAME_FIELD_LENGTH = {6, (char*)"LENGTH"};
static const RsslBuffer RSSL_ENAME_FIELD_RWFTYPE = {7, (char*)"RWFTYPE"};
static const RsslBuffer RSSL_ENAME_FIELD_RWFLEN = {6, (char*)"RWFLEN"};
static const RsslBuffer RSSL_ENAME_FIELD_ENUMLENGTH = {10, (char*)"ENUMLENGTH"};
static const RsslBuffer RSSL_ENAME_FIELD_LONGNAME = {8, (char*)"LONGNAME"};

// EnumType names
static const RsslBuffer RSSL_ENAME_ENUM_FIDS = {4, (char*)"FIDS"};
static const RsslBuffer RSSL_ENAME_ENUM_VALUE = {5, (char*)"VALUE"};
static const RsslBuffer RSSL_ENAME_ENUM_DISPLAY = {7, (char*)"DISPLAY"};
static const RsslBuffer RSSL_ENAME_ENUM_MEANING = {7, (char*)"MEANING"};

// Enum Type Dictionary Tags
static const RsslBuffer RSSL_ENAME_ENUM_RT_VERSION = {10, (char*)"RT_Version"};
static const RsslBuffer RSSL_ENAME_ENUM_DT_VERSION = {10, (char*)"DT_Version"};

// Set Definition Dictionary Tags
static const RsslBuffer RSSL_ENAME_SETDEF_NUMENTRIES = {10, (char*)"NUMENTRIES"};
static const RsslBuffer RSSL_ENAME_SETDEF_FIDS		 = {4, (char*)"FIDS"};
static const RsslBuffer RSSL_ENAME_SETDEF_TYPES		 = {5, (char*)"TYPES"};
static const RsslBuffer RSSL_ENAME_SETDEF_NAMES		 = {5, (char*)"NAMES"};

// Request Message Payload - Well known Element Names
// Because these span domains, they are namespaced
// <namespace>:<element name>
// Thomson Reuters claims empty namespace (e.g. :ItemList is TR namespace)
// Customers can define and namespace using other values as they need 
static const RsslBuffer RSSL_ENAME_BATCH_ITEM_LIST = { 9 , (char*)":ItemList" };
static const RsslBuffer RSSL_ENAME_VIEW_TYPE = { 9 , (char*)":ViewType" };
static const RsslBuffer RSSL_ENAME_VIEW_DATA = { 9 , (char*)":ViewData" };
static const RsslBuffer RSSL_ENAME_SYMBOL_LIST_BEHAVIORS = { 20 , (char*)":SymbolListBehaviors" };

// Symbol List Behaviors - Well known Element Names
static const RsslBuffer RSSL_ENAME_DATA_STREAMS = { 12 , (char*)":DataStreams" };
static const RsslBuffer RSSL_ENAME_CHANNEL_ID	= {  9, (char*)"ChannelId" };
static const RsslBuffer RSSL_ENAME_GAP_CHANNEL_ID		= { 12, (char*)"GapChannelId" };
static const RsslBuffer RSSL_ENAME_STREAMING_CHANNEL_ID		= { 18, (char*)"StreamingChannelId" };

// Batch Reissue/Batch Close Behaviors - Well known Element Name(s)
// Used for processing Batch requests with StreamID List(s)
static const RsslBuffer RSSL_ENAME_BATCH_STREAMID_LIST = { 13, (char *)":StreamIdList" };

/**
  * @addtogroup ClassOfService
  * @{
  */

// Class of Service Properties - Well known Element Names
static const RsslBuffer RSSL_ENAME_COS_MAX_MSG_SIZE = { 11, (char*)":MaxMsgSize" };
static const RsslBuffer RSSL_ENAME_COS_MAX_FRAGMENT_SIZE = { 16 , (char*)":MaxFragmentSize" };
static const RsslBuffer RSSL_ENAME_COS_SUPPS_FRAGMENTATION = { 22 , (char*)":SupportsFragmentation" };
static const RsslBuffer RSSL_ENAME_COS_PROT_TYPE = { 13 , (char*)":ProtocolType" };
static const RsslBuffer RSSL_ENAME_COS_PROT_MAJOR_VERSION = { 21, (char*)":ProtocolMajorVersion" };
static const RsslBuffer RSSL_ENAME_COS_PROT_MINOR_VERSION = { 21, (char*)":ProtocolMinorVersion" };
static const RsslBuffer RSSL_ENAME_COS_STREAM_VERSION = { 14, (char*)":StreamVersion" };
static const RsslBuffer RSSL_ENAME_COS_TYPE = { 5, (char*)":Type" };
static const RsslBuffer RSSL_ENAME_COS_RECV_WINDOW_SIZE = { 15, (char*)":RecvWindowSize" };

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

/**
 *	@}
 */



#endif

