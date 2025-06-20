/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|       Copyright (C) 2015-2021,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __RSSL_RDM_DOMAINS_H
#define __RSSL_RDM_DOMAINS_H

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
	RSSL_DMT_SERVICE_PROVIDER_STATUS	= 11,	/*!< (11) Service Provider Status domain, used with LSEG Real-Time Pulse */
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
	RSSL_DMT_FORECAST					= 36,	/*!< (36) Forecast content */
	RSSL_DMT_MARKET_BY_TIME				= 37,	/*!< Market By Time content */
	RSSL_DMT_SYSTEM						= 127,  /*!< (127) System domain for use with domain netural content (e.g. tunnel stream creation) */
	RSSL_DMT_MAX_RESERVED				= 127,	/*!< (127) Maximum reserved message type value */
	RSSL_DMT_MAX_VALUE					= 255	/*!< (255) Maximum value for a message type */
} RsslDomainTypes;

/** 
 * @brief General OMM strings associated with the different domain types.
 * @see RsslDomainTypes, rsslDomainTypeToOmmString
 */

static const RsslBuffer RSSL_OMMSTR_DMT_LOGIN = { 5, (char*)"Login" };
static const RsslBuffer RSSL_OMMSTR_DMT_SOURCE = { 6, (char*)"Source" };
static const RsslBuffer RSSL_OMMSTR_DMT_DICTIONARY = { 10, (char*)"Dictionary" };
static const RsslBuffer RSSL_OMMSTR_DMT_MARKET_PRICE = { 11, (char*)"MarketPrice" };
static const RsslBuffer RSSL_OMMSTR_DMT_MARKET_BY_ORDER = { 13, (char*)"MarketByOrder" };
static const RsslBuffer RSSL_OMMSTR_DMT_MARKET_BY_PRICE = { 13, (char*)"MarketByPrice" };
static const RsslBuffer RSSL_OMMSTR_DMT_MARKET_MAKER = { 11, (char*)"MarketMaker" };
static const RsslBuffer RSSL_OMMSTR_DMT_SYMBOL_LIST = { 10, (char*)"SymbolList" };
static const RsslBuffer RSSL_OMMSTR_DMT_SERVICE_PROVIDER_STATUS = { 21, (char*)"ServiceProviderStatus" };
static const RsslBuffer RSSL_OMMSTR_DMT_HISTORY = { 7, (char*)"History" };
static const RsslBuffer RSSL_OMMSTR_DMT_HEADLINE = { 8, (char*)"Headline" };
static const RsslBuffer RSSL_OMMSTR_DMT_STORY = { 5, (char*)"Story" };
static const RsslBuffer RSSL_OMMSTR_DMT_REPLAYHEADLINE = { 14, (char*)"ReplayHeadline" };
static const RsslBuffer RSSL_OMMSTR_DMT_REPLAYSTORY = { 11, (char*)"ReplayStory" };
static const RsslBuffer RSSL_OMMSTR_DMT_TRANSACTION = { 11, (char*)"Transaction" };
static const RsslBuffer RSSL_OMMSTR_DMT_YIELD_CURVE = { 10, (char*)"YieldCurve" };
static const RsslBuffer RSSL_OMMSTR_DMT_CONTRIBUTION = { 12, (char*)"Contribution" };
static const RsslBuffer RSSL_OMMSTR_DMT_PROVIDER_ADMIN = { 13, (char*)"ProviderAdmin" };
static const RsslBuffer RSSL_OMMSTR_DMT_ANALYTICS = { 9, (char*)"Analytics" };
static const RsslBuffer RSSL_OMMSTR_DMT_REFERENCE = { 9, (char*)"Reference" };
static const RsslBuffer RSSL_OMMSTR_DMT_NEWS_TEXT_ANALYTICS = { 17, (char*)"NewsTextAnalytics" };
static const RsslBuffer RSSL_OMMSTR_DMT_ECONOMIC_INDICATOR = { 17, (char*)"EconomicIndicator" };
static const RsslBuffer RSSL_OMMSTR_DMT_POLL = { 4, (char*)"Poll" };
static const RsslBuffer RSSL_OMMSTR_DMT_FORECAST = { 8, (char*)"Forecast" };
static const RsslBuffer RSSL_OMMSTR_DMT_MARKET_BY_TIME = { 12, (char*)"MarketByTime" };
static const RsslBuffer RSSL_OMMSTR_DMT_SYSTEM = { 6, (char*)"System" };

 
/**
 *	@defgroup DomainCommon TRDM Common Enumerations
 *	@brief	The TRDM Common Enumerations are used across several LSEG Domain Models.
 *	@{
 */

typedef enum
{
	RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED		= 0,	/*!< (0) Symbology is not specified or not applicable */
	RDM_INSTRUMENT_NAME_TYPE_RIC				= 1,	/*!< (1) Instrument Code */
	RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR		= 2,	/*!< (2) Contributor identification information for user-stream contributions (RSSL_DMT_CONTRIBUTION) */
	RDM_INSTRUMENT_NAME_TYPE_MAX_RESERVED		= 127	/*!< (127) Maximum reserved Quote Symbology */
} RDMInstrumentNameTypes;

/** 
 * @brief General OMM strings associated with the different instrument name types.
 * @see RDMInstrumentNameTypes, rsslRDMInstrumentNameTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_INSTRUMENT_NAME_TYPE_UNSPECIFIED = { 11, (char*)"Unspecified" };
static const RsslBuffer RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC = { 3, (char*)"Ric" };
static const RsslBuffer RDM_OMMSTR_INSTRUMENT_NAME_TYPE_CONTRIBUTOR = { 11, (char*)"Contributor" };

/** 
 * @brief Converts the provided instrument name type enumeration to a general OMM string.
 *
 * @param type Instrument name type to translate to string
 * @return Null terminated character string containing the name of the name type
 * @see RDMInstrumentNameTypes
 */
RSSL_API const char* rsslRDMInstrumentNameTypeToOmmString(RsslUInt8 type);


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

/** 
 * @brief General OMM strings associated with the different update event types.
 * @see RDMUpdateEventTypes, rsslRDMUpdateEventTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_UNSPECIFIED = { 11, (char*)"Unspecified" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_QUOTE = { 5, (char*)"Quote" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_TRADE = { 5, (char*)"Trade" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_NEWS_ALERT = { 9, (char*)"NewsAlert" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_VOLUME_ALERT = { 11, (char*)"VolumeAlert" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_ORDER_INDICATION = { 15, (char*)"OrderIndication" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_CLOSING_RUN = { 10, (char*)"ClosingRun" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_CORRECTION = { 10, (char*)"Correction" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_MARKET_DIGEST = { 12, (char*)"MarketDigest" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_QUOTES_TRADE = { 11, (char*)"QuotesTrade" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_MULTIPLE = { 8, (char*)"Multiple" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_TYPE_VERIFY = { 6, (char*)"Verify" };

/** 
 * @brief Converts the provided update event type enumeration to a general OMM string.
 *
 * @param type update event type to translate to string
 * @return Null terminated character string containing the name of the update event type
 * @see RDMUpdateEventTypes
 */
RSSL_API const char* rsslRDMUpdateEventTypeToOmmString(RsslUInt8 type);

typedef enum {
	RDM_VIEW_TYPE_FIELD_ID_LIST				= 1,	/*!< (1) View Data contains a list of Field IDs */
	RDM_VIEW_TYPE_ELEMENT_NAME_LIST			= 2		/*!< (2) View Data contains a list of Element Names */
} RDMViewTypes;

/** 
 * @brief General OMM strings associated with the different view types.
 * @see RDMViewTypes, rsslRDMViewTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_VIEW_TYPE_FIELD_ID_LIST = { 11, (char*)"FieldIDList" };
static const RsslBuffer RDM_OMMSTR_VIEW_TYPE_ELEMENT_NAME_LIST = { 15, (char*)"ElementNameList" };

/** 
 * @brief Converts the provided view type enumeration to a general OMM string.
 *
 * @param type View type to translate to string
 * @return Null terminated character string containing the name of the view type
 * @see RDMViewTypes
 */
RSSL_API const char* rsslRDMViewTypeToOmmString(RsslUInt type);


typedef enum {
	RDM_UPD_EVENT_FILTER_TYPE_NONE = 0,  /*!< (0) No Update event type specified */
	RDM_UPD_EVENT_FILTER_TYPE_UNSPECIFIED        = 0x001,    /*!< (0x001) Unspecified Update Event */
	RDM_UPD_EVENT_FILTER_TYPE_QUOTE = 0x002,    /*!< (0x002) Update Event Quote */
	RDM_UPD_EVENT_FILTER_TYPE_TRADE = 0x004,    /*!< (0x004) Update Event Trade */
	RDM_UPD_EVENT_FILTER_TYPE_NEWS_ALERT = 0x008,    /*!< (0x008) Update Event News Alert */
	RDM_UPD_EVENT_FILTER_TYPE_VOLUME_ALERT = 0x010,    /*!< (0x010) Update Event Volume Alert */
	RDM_UPD_EVENT_FILTER_TYPE_ORDER_INDICATION = 0x020,    /*!< (0x020) Update Event Order Indication */
	RDM_UPD_EVENT_FILTER_TYPE_CLOSING_RUN = 0x040,    /*!< (0x040) Update Event Closing Run */
	RDM_UPD_EVENT_FILTER_TYPE_CORRECTION = 0x080,    /*!< (0x080) Update Event Correction */
	RDM_UPD_EVENT_FILTER_TYPE_MARKET_DIGEST = 0x100,    /*!< (0x100) Update Event Market Digest */
	RDM_UPD_EVENT_FILTER_TYPE_QUOTES_TRADE = 0x200,    /*!< (0x200) Update Event Quotes followed by a Trade */
	RDM_UPD_EVENT_FILTER_TYPE_MULTIPLE = 0x400,    /*!< (0x400) Update Event with filtering and conflation applied */
	RDM_UPD_EVENT_FILTER_TYPE_VERIFY = 0x800,    /*!< (0x800) Fields may have changed */
} RDMUpdateEventFilter;

#define DEFAULT_UPDATE_TYPE_FILTER 65533  /*!< This value is service info (first nibble) plus of all filters UPD_EVENT_FILTER_TYPE except UPD_EVENT_FILTER_TYPE_QUOTE */

/**
 * @brief General OMM strings associated with the different update event filter types.
 * @see RDMUpdateEventFilter, rsslRDMUpdateTypeFilterToOmmString
 */
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_UNSPECIFIED = { 11, (char*)"Unspecified" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_QUOTE = { 5, (char*)"Quote" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_TRADE = { 5, (char*)"Trade" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_NEWS_ALERT = { 9, (char*)"NewsAlert" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_VOLUME_ALERT = { 11, (char*)"VolumeAlert" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_ORDER_INDICATION = { 15, (char*)"OrderIndication" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_CLOSING_RUN = { 10, (char*)"ClosingRun" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_CORRECTION = { 10, (char*)"Correction" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_MARKET_DIGEST = { 12, (char*)"MarketDigest" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_QUOTES_TRADE = { 11, (char*)"QuotesTrade" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_MULTIPLE = { 8, (char*)"Multiple" };
static const RsslBuffer RDM_OMMSTR_UPD_EVENT_FILTER_TYPE_VERIFY = { 6, (char*)"Verify" };

/** 
 * @brief Converts the provided update type filter enumeration to a general OMM string.
 *
 * @param update type filter to translate to string
 * @return Null terminated character string containing the name of the update type filter
 * @see RDMUpdateEventFilter
 */
RSSL_API const char* rsslRDMUpdateEventFilterTypeToOmmString(RsslUInt type);

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

/** 
 * @brief General OMM strings associated with the different login role types.
 * @see RDMLoginRoleTypes, rsslRDMLoginRoleTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_LOGIN_ROLE_CONS = { 4, (char*)"Cons" };
static const RsslBuffer RDM_OMMSTR_LOGIN_ROLE_PROV = { 4, (char*)"Prov" };

/** 
 * @brief Converts the provided role type enumeration to a general OMM string.
 *
 * @param type Role type to translate to string
 * @return Null terminated character string containing the name of the role type
 * @see RDMLoginRoleTypes
 */
RSSL_API const char* rsslRDMLoginRoleTypeToOmmString(RsslUInt type);

typedef enum {
	RDM_LOGIN_USER_NAME				= 1,	/*!< (1) Name */
	RDM_LOGIN_USER_EMAIL_ADDRESS	= 2,	/*!< (2) Email address */
	RDM_LOGIN_USER_TOKEN			= 3,	/*!< (3) User Token, typically a Triple A (AAA) token. */
	RDM_LOGIN_USER_COOKIE			= 4,	/*!< (4) User information is specified in a cookie.  */
	RDM_LOGIN_USER_AUTHN_TOKEN 		= 5 	/*!< (5) String defining User Authentication Token */
} RDMLoginUserIdTypes;


typedef enum {
	RDM_LOGIN_RTT_ELEMENT = 2		/*!< (2) RTT Information encoded in an element list */
}RDMLoginRTTFormat;

/** 
 * @brief General OMM strings associated with the different user ID types.
 * @see RDMLoginUserIdTypes, rsslRDMLoginUserIdTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_LOGIN_USER_NAME = { 4, (char*)"Name" };
static const RsslBuffer RDM_OMMSTR_LOGIN_USER_EMAIL_ADDRESS = { 12, (char*)"EmailAddress" };
static const RsslBuffer RDM_OMMSTR_LOGIN_USER_TOKEN = { 5, (char*)"Token" };
static const RsslBuffer RDM_OMMSTR_LOGIN_USER_COOKIE = { 6, (char*)"Cookie" };

/** 
 * @brief Converts the provided user ID type enumeration to a general OMM string.
 *
 * @param type User ID type to translate to string
 * @return Null terminated character string containing the name of the user ID type
 * @see RDMLoginUserIdTypes
 */
RSSL_API const char* rsslRDMLoginUserIdTypeToOmmString(RsslUInt8 type);

typedef enum {
	RDM_LOGIN_SERVER_TYPE_ACTIVE	= 0,	/*!< (0) Active Server */
	RDM_LOGIN_SERVER_TYPE_STANDBY	= 1		/*!< (1) Standby Server */
} RDMLoginServerTypes;

typedef enum {
    RDM_DIRECTORY_SERVICE_TYPE_ACTIVE = 0,	/*!< (0) Active Directory Service */
    RDM_DIRECTORY_SERVICE_TYPE_STANDBY = 1		/*!< (1) Standby Directory Service */
} RDMDirectoryServiceTypes;


/** 
 * @brief General OMM strings associated with the different server types.
 * @see RDMLoginServerTypes, rsslRDMLoginServerTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_LOGIN_SERVER_TYPE_ACTIVE = { 6, (char*)"Active" };
static const RsslBuffer RDM_OMMSTR_LOGIN_SERVER_TYPE_STANDBY = { 7, (char*)"Standby" };

/** 
 * @brief Converts the provided server type enumeration to a general OMM string.
 *
 * @param type Server type to translate to string
 * @return Null terminated character string containing the name of the server type
 * @see RDMLoginServerTypes
 */
RSSL_API const char* rsslRDMLoginServerTypeToOmmString(RsslUInt type);

typedef enum
{            
	RDM_LOGIN_BATCH_NONE				= 0x0,  /*!< (0x0) Provider does not support batching. */
	RDM_LOGIN_BATCH_SUPPORT_REQUESTS	= 0x1,  /*!< (0x1) Provider supports batch requests. */
	RDM_LOGIN_BATCH_SUPPORT_REISSUES	= 0x2,  /*!< (0x2) Provider supports batch reissue requests. */
	RDM_LOGIN_BATCH_SUPPORT_CLOSES		= 0x4   /*!< (0x4) Provider supports batch closes. */
} RDMLoginBatchSupportFlags;

/** 
 * @brief General OMM strings associated with the different batch support flags.
 * @see RDMLoginBatchSupportFlags, rsslRDMLoginBatchSupportFlagsToOmmString
 */
static const RsslBuffer RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REQUESTS = { 8, (char*)"Requests" };
static const RsslBuffer RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REISSUES = { 8, (char*)"Reissues" };
static const RsslBuffer RDM_OMMSTR_LOGIN_BATCH_SUPPORT_CLOSES = { 6, (char*)"Closes" };

/**
 * @brief Provide general OMM string representation of RDMLoginBatchSupportFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RDMLoginBatchSupportFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RDMLoginBatchSupportFlags
 */
RSSL_API RsslRet rsslRDMLoginBatchSupportFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt flags);

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
 * @brief General OMM strings associated with the different dictionary types.
 * @see RDMDictionaryTypes, rsslRDMDictionaryTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DICTIONARY_FIELD_DEFINITIONS = { 16, (char*)"FieldDefinitions" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_ENUM_TABLES = { 10, (char*)"EnumTables" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_RECORD_TEMPLATES = { 15, (char*)"RecordTemplates" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_DISPLAY_TEMPLATES = { 16, (char*)"DisplayTemplates" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_DATA_DEFINITIONS = { 15, (char*)"DataDefinitions" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_STYLE_SHEET = { 10, (char*)"StyleSheet" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_REFERENCE = { 9, (char*)"Reference" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_FIELD_SET_DEFINITION = { 18, (char*)"FieldSetDefinition" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_ELEMENT_SET_DEFINITION = { 20, (char*)"ElementSetDefinition" };

/** 
 * @brief Converts the provided dictionary type enumeration to a general OMM string.
 *
 * @param type Dictionary type to translate to string
 * @return Null terminated character string containing the name of the dictionary type
 * @see RDMDictionaryTypes
 */
RSSL_API const char* rsslRDMDictionaryTypeToOmmString(RsslUInt type);

/**
 * @brief Enumerations describing how much information about a particular dictionary is desired.
 * These values are typically set in an RsslRequestMsg MsgKey filter when the request for the dictionary is made.
 * See the RDM Usage Guide for details.
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
 * @brief General OMM strings associated with the different dictionary verbosities.
 * @see RDMDictionaryVerbosityValues, rsslRDMDictionaryVerbosityValueToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DICTIONARY_INFO = { 4, (char*)"Info" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_MINIMAL = { 7, (char*)"Minimal" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_NORMAL = { 6, (char*)"Normal" };
static const RsslBuffer RDM_OMMSTR_DICTIONARY_VERBOSE = { 7, (char*)"Verbose" };

/** 
 * @brief Converts the provided verbosity enumeration to a general OMM string.
 *
 * @param value Verbosity value to translate to string
 * @return Null terminated character string containing the name of the verbosity
 * @see RDMDictionaryVerbosityValues
 */
RSSL_API const char* rsslRDMDictionaryVerbosityValueToOmmString(RsslUInt32 value);

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
	RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER			= 0x00000040	/*!< (0x00000040) Source Sequenced Multicast Information */
} RDMDirectoryServiceFilterFlags;

/** 
 * @brief General OMM strings associated with the different service filter flags.
 * @see RDMDirectoryServiceFilterFlags, rsslRDMDirectoryServiceFilterFlagsToOmmString
 */

static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_INFO_FILTER = { 4, (char*)"Info" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_STATE_FILTER = { 5, (char*)"State" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_FILTER = { 5, (char*)"Group" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_FILTER = { 4, (char*)"Load" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_DATA_FILTER = { 4, (char*)"Data" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_LINK_FILTER = { 4, (char*)"Link" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_FILTER = { 8, (char*)"SeqMcast" };

/**
 * @brief Provide general OMM string representation of RDMDirectoryServiceFilterFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RDMDirectoryServiceFilterFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RDMDirectoryServiceFilterFlags
 */
RSSL_API RsslRet rsslRDMDirectoryServiceFilterFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt32 flags);

typedef enum {
	RDM_DIRECTORY_SERVICE_INFO_ID				= 1,    /*!< (1) Service Info Filter ID */
	RDM_DIRECTORY_SERVICE_STATE_ID				= 2,    /*!< (2) Source State Filter ID */
	RDM_DIRECTORY_SERVICE_GROUP_ID				= 3,	/*!< (3) Source Group Filter ID */
	RDM_DIRECTORY_SERVICE_LOAD_ID				= 4,	/*!< (4) Source Load Filter ID */
	RDM_DIRECTORY_SERVICE_DATA_ID				= 5,	/*!< (5) Source Data Filter ID */
	RDM_DIRECTORY_SERVICE_LINK_ID				= 6,	/*!< (6) Communication Link Filter ID */
	RDM_DIRECTORY_SERVICE_SEQ_MCAST_ID			= 7		/*!< (7) Sequenced Multicast Filter ID */
} RDMDirectoryServiceFilterIds;

/** 
 * @brief General OMM strings associated with the different service filter IDs.
 * @see RDMDirectoryServiceFilterIds, rsslRDMDirectoryServiceFilterIdToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_INFO_ID = { 4, (char*)"Info" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_STATE_ID = { 5, (char*)"State" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_ID = { 5, (char*)"Group" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_ID = { 4, (char*)"Load" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_DATA_ID = { 4, (char*)"Data" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_LINK_ID = { 4, (char*)"Link" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_ID = { 8, (char*)"SeqMcast" };

/** 
 * @brief Converts the provided service filter ID enumeration to a general OMM string.
 *
 * @param id Service filter ID to translate to string
 * @return Null terminated character string containing the name of the service filter ID 
 * @see RDMDirectoryServiceFilterIds
 */
RSSL_API const char* rsslRDMDirectoryServiceFilterIdToOmmString(RsslUInt id);


typedef enum {
	RDM_DIRECTORY_SERVICE_STATE_DOWN			= 0,	/*!< (0) Service state down */
	RDM_DIRECTORY_SERVICE_STATE_UP				= 1		/*!< (1) Service state up */
} RDMDirectoryServiceStates;

/** 
 * @brief General OMM strings associated with the different service states.
 * @see RDMDirectoryServiceStates, rsslRDMDirectoryServiceStateToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_STATE_DOWN = { 4, (char*)"Down" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SERVICE_STATE_UP = { 2, (char*)"Up" };

/** 
 * @brief Converts the provided service state enumeration to a general OMM string.
 *
 * @param state Service state to translate to string
 * @return Null terminated character string containing the name of the service state
 * @see RDMDirectoryServiceStates
 */
RSSL_API const char* rsslRDMDirectoryServiceStateToOmmString(RsslUInt serviceState);

typedef enum {
	RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY		= 0,	/*!< (0) Indicates the upstream provider is the active and there is no standby provider */
	RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY	= 1,	/*!< (1) Indicates the upstream provider is the active and there is a standby provider */
	RDM_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY				= 2 	/*!< (2) Indicates the upstream provider is a standby */
}  RDMDirectorySourceMirroringMode;

/** 
 * @brief Converts the provided source mirroring mode enumeration to a general OMM string.
 *
 * @param mode Source mirroring mode to translate to string
 * @return Null terminated character string containing the name of the source mirroring mode
 * @see RDMDirectorySourceMirroringMode
 */
RSSL_API const char* rsslRDMDirectorySourceMirroringModeToOmmString(RsslUInt mode);

/** 
 * @brief General OMM strings associated with the different source mirroring modes.
 * @see RDMDirectorySourceMirroringMode, rsslRDMDirectorySourceMirroringModeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY = { 15, (char*)"ActiveNoStandby" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY = { 17, (char*)"ActiveWithStandby" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY = { 7, (char*)"Standby" };

typedef enum {
	RDM_DIRECTORY_DATA_TYPE_MIN_RESERVED	= 0,		/*!< (0) Minimum reserved Data Type */
	RDM_DIRECTORY_DATA_TYPE_NONE			= 0,		/*!< (0) None */
	RDM_DIRECTORY_DATA_TYPE_TIME			= 1,		/*!< (1) Time */
	RDM_DIRECTORY_DATA_TYPE_ALERT			= 2,		/*!< (2) Alert */
	RDM_DIRECTORY_DATA_TYPE_HEADLINE		= 3,		/*!< (3) Headline */
	RDM_DIRECTORY_DATA_TYPE_STATUS			= 4,		/*!< (4) Status */
	RDM_DIRECTORY_DATA_TYPE_MAX_RESERVED	= 1023		/*!< (1023) Maximum reserved Data Type */
} RDMDirectoryDataTypes;

/** 
 * @brief General OMM strings associated with the different directory data types.
 * @see RDMDirectoryDataTypes, rsslRDMDirectoryDataTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_DATA_TYPE_NONE = { 4, (char*)"None" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_DATA_TYPE_TIME = { 4, (char*)"Time" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_DATA_TYPE_ALERT = { 5, (char*)"Alert" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_DATA_TYPE_HEADLINE = { 8, (char*)"Headline" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_DATA_TYPE_STATUS = { 6, (char*)"Status" };

/** 
 * @brief Converts the provided directory data type enumeration to a general OMM string.
 *
 * @param type Data type to translate to string
 * @return Null terminated character string containing the name of the data type
 * @see RDMDirectoryDataTypes
 */
RSSL_API const char* rsslRDMDirectoryDataTypeToOmmString(RsslUInt type);

typedef enum {
	RDM_DIRECTORY_LINK_TYPE_INTERACTIVE		= 1,		/*!< (1) Interactive */
	RDM_DIRECTORY_LINK_TYPE_BROADCAST		= 2			/*!< (2) Broadcast */
} RDMDirectoryLinkTypes;

/** 
 * @brief General OMM strings associated with the different directory link types.
 * @see RDMDirectoryLinkTypes, rsslRDMDirectoryLinkTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_TYPE_INTERACTIVE = { 11, (char*)"Interactive" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_TYPE_BROADCAST = { 9, (char*)"Broadcast" };

/** 
 * @brief Converts the provided directory link type enumeration to a general OMM string.
 *
 * @param type Link type to translate to string
 * @return Null terminated character string containing the name of the link type
 * @see RDMDirectoryLinkTypes
 */
RSSL_API const char* rsslRDMDirectoryLinkTypeToOmmString(RsslUInt type);

typedef enum {
	RDM_DIRECTORY_LINK_STATE_DOWN		= 0,		/*!< (0) Down */
	RDM_DIRECTORY_LINK_STATE_UP			= 1			/*!< (1) Up */
} RDMDirectoryLinkStates;

/** 
 * @brief General OMM strings associated with the different directory link states.
 * @see RDMDirectoryLinkStates, rsslRDMDirectoryLinkStateToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_STATE_DOWN = { 4, (char*)"Down" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_STATE_UP = { 2, (char*)"Up" };

/** 
 * @brief Converts the provided directory link state enumeration to a general OMM string.
 *
 * @param state Link state to translate to string
 * @return Null terminated character string containing the name of the link state
 * @see RDMDirectoryLinkStates
 */
RSSL_API const char* rsslRDMDirectoryLinkStateToOmmString(RsslUInt state);

typedef enum {
	RDM_DIRECTORY_LINK_CODE_NONE				= 0,	/*!< (0) None */
	RDM_DIRECTORY_LINK_CODE_OK					= 1,	/*!< (1) Ok */
	RDM_DIRECTORY_LINK_CODE_RECOVERY_STARTED	= 2,	/*!< (2) Recovery Started */
	RDM_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED	= 3		/*!< (3) Recovery Completed */
} RDMDirectoryLinkCodes;

/** 
 * @brief General OMM strings associated with the different directory link codes.
 * @see RDMDirectoryLinkCodes, rsslRDMDirectoryLinkCodeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_CODE_NONE = { 4, (char*)"None" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_CODE_OK = { 2, (char*)"Ok" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_STARTED = { 15, (char*)"RecoveryStarted" };
static const RsslBuffer RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED = { 17, (char*)"RecoveryCompleted" };

/** 
 * @brief Converts the provided directory link code enumeration to a general OMM string.
 *
 * @param code Link code to translate to string
 * @return Null terminated character string containing the name of the link code
 * @see RDMDirectoryLinkCodes
 */
RSSL_API const char* rsslRDMDirectoryLinkCodeToOmmString(RsslUInt code);

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
 * @brief General OMM strings associated with the different enhanced symbol list behavior support flags.
 * @see RDMEnhancedSymbolListSupportFlags, rsslRDMEnhancedSymbolListSupportFlagsToOmmString
 */
static const RsslBuffer RDM_OMMSTR_SYMBOL_LIST_SUPPORT_DATA_STREAMS = { 11, (char*)"DataStreams" };

/**
 * @brief Provide general OMM string representation of RDMEnhancedSymbolListSupportFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RDMEnhancedSymbolListSupportFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RDMEnhancedSymbolListSupportFlags
 */
RSSL_API RsslRet rsslRDMEnhancedSymbolListSupportFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt flags);
 
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
 * @brief General OMM strings associated with the different symbol list requestable behavior flags.
 * @see RDMSymbolListDataStreamRequestFlags, rsslRDMSymbolListDataStreamRequestFlagsToOmmString
 */
static const RsslBuffer RDM_OMMSTR_SYMBOL_LIST_DATA_STREAMS = { 11, (char*)"DataStreams" };
static const RsslBuffer RDM_OMMSTR_SYMBOL_LIST_DATA_SNAPSHOTS = { 13, (char*)"DataSnapshots" };

/**
 * @brief Provide general OMM string representation of RDMSymbolListDataStreamRequestFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RDMSymbolListDataStreamRequestFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RDMSymbolListDataStreamRequestFlags
 */
RSSL_API RsslRet rsslRDMSymbolListDataStreamRequestFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt flags);

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
 * @brief General OMM strings associated with the different class of service filter IDs.
 * @see RDMClassOfServiceFilterIDs, rsslRDMCosFilterIdToOmmString
 */
static const RsslBuffer RDM_OMMSTR_COS_COMMON_PROPERTIES_ID = { 16, (char*)"CommonProperties" };
static const RsslBuffer RDM_OMMSTR_COS_AUTHENTICATION_ID = { 14, (char*)"Authentication" };
static const RsslBuffer RDM_OMMSTR_COS_FLOW_CONTROL_ID = { 11, (char*)"FlowControl" };
static const RsslBuffer RDM_OMMSTR_COS_DATA_INTEGRITY_ID = { 13, (char*)"DataIntegrity" };
static const RsslBuffer RDM_OMMSTR_COS_GUARANTEE_ID = { 9, (char*)"Guarantee" };

/** 
 * @brief Converts the provided class-of-service filter ID enumeration to a general OMM string.
 *
 * @param id Class-of-service filter ID to translate to string
 * @return Null terminated character string containing the name of the ID
 * @see RDMClassOfServiceFilterIDs
 */
RSSL_API const char* rsslRDMCosFilterIdToOmmString(RsslUInt id);

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
 * @brief General OMM strings associated with the different class of service filter flags.
 * @see RDMClassOfServiceFilterFlags, rsslRDMCosFilterFlagsToOmmString
 */
static const RsslBuffer RDM_OMMSTR_COS_COMMON_PROPERTIES_FLAG = { 16, (char*)"CommonProperties" };
static const RsslBuffer RDM_OMMSTR_COS_AUTHENTICATION_FLAG = { 14, (char*)"Authentication" };
static const RsslBuffer RDM_OMMSTR_COS_FLOW_CONTROL_FLAG = { 11, (char*)"FlowControl" };
static const RsslBuffer RDM_OMMSTR_COS_DATA_INTEGRITY_FLAG = { 13, (char*)"DataIntegrity" };
static const RsslBuffer RDM_OMMSTR_COS_GUARANTEE_FLAG = { 9, (char*)"Guarantee" };

/**
 * @brief Provide general OMM string representation of RDMClassOfServiceFilterFlags
 * If multiple flags are set, they will be separated by a '|' delimiter.
 * Unrecognized flags will be ignored.
 * @param oBuffer RsslBuffer to populate with string.  RsslBuffer::data should point to memory to convert into where RsslBuffer::length indicates the number of bytes available in RsslBuffer::data.
 * @param flags RDMClassOfServiceFilterFlags value
 * @return RsslRet ::RSSL_RET_SUCCESS if successful, ::RSSL_RET_BUFFER_TOO_SMALL if the buffer did not have enough space.
 * @see RDMClassOfServiceFilterFlags
 */
RSSL_API RsslRet rsslRDMCosFilterFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt32 flags);

/**
  * @brief Class of Service authentication types.
  */
typedef enum
{
	RDM_COS_AU_NOT_REQUIRED	= 0, /*!< (0) Authentication is not required. */
	RDM_COS_AU_OMM_LOGIN	= 1  /*!< (1) A login message is exchanged to authenticate the stream. */
} RDMClassOfServiceAuthenticationType;

/** 
 * @brief General OMM strings associated with the different class of service authentication types.
 * @see RDMClassOfServiceAuthenticationType, rsslRDMCosAuthenticationTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_COS_AU_NOT_REQUIRED = { 11, (char*)"NotRequired" };
static const RsslBuffer RDM_OMMSTR_COS_AU_OMM_LOGIN = { 8, (char*)"OmmLogin" };

/** 
 * @brief Converts the provided class-of-service authentication type enumeration to a general OMM string.
 *
 * @param type Class-of-service authentication type to translate to string
 * @return Null terminated character string containing the name of the type
 * @see RDMClassOfServiceAuthenticationType
 */
RSSL_API const char* rsslRDMCosAuthenticationTypeToOmmString(RsslUInt type);

/**
  * @brief Class of Service flow control types.
  */
typedef enum
{
	RDM_COS_FC_NONE				= 0, /*!< (0) None */
	RDM_COS_FC_BIDIRECTIONAL	= 1  /*!< (1) Flow control is performed in both directions. */
} RDMClassOfServiceFlowControlType;

/** 
 * @brief General OMM strings associated with the different class of service flow control types.
 * @see RDMClassOfServiceFlowControlType, rsslRDMCosFlowControlTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_COS_FC_NONE = { 4, (char*)"None" };
static const RsslBuffer RDM_OMMSTR_COS_FC_BIDIRECTIONAL = { 13, (char*)"Bidirectional" };

/** 
 * @brief Converts the provided class-of-service flow control type enumeration to a general OMM string.
 *
 * @param type Class-of-service flow control type to translate to string
 * @return Null terminated character string containing the name of the type
 * @see RDMClassOfServiceFlowControlType
 */
RSSL_API const char* rsslRDMCosFlowControlTypeToOmmString(RsslUInt type);

/**
  * @brief Class of Service data integrity types.
  */
typedef enum {
	RDM_COS_DI_BEST_EFFORT		= 0, /*!< (0) Delivery of messages is best-effort. */
	RDM_COS_DI_RELIABLE			= 1  /*!< (1) Messages are reliably delivered. */
} RDMClassOfServiceDataIntegrityType;

/** 
 * @brief General OMM strings associated with the different class of service data integrity types.
 * @see RDMClassOfServiceDataIntegrityType, rsslRDMCosDataIntegrityTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_COS_DI_BEST_EFFORT = { 10, (char*)"BestEffort" };
static const RsslBuffer RDM_OMMSTR_COS_DI_RELIABLE = { 8, (char*)"Reliable" };

/** 
 * @brief Converts the provided class-of-service data integrity type enumeration to a general OMM string.
 *
 * @param type Class-of-service data integrity type to translate to string
 * @return Null terminated character string containing the name of the type
 * @see RDMClassOfServiceDataIntegrityType
 */
RSSL_API const char* rsslRDMCosDataIntegrityTypeToOmmString(RsslUInt type);

/**
  * @brief Class of Service guarantee types.
  */
typedef enum
{
	RDM_COS_GU_NONE				= 0, /*!< (0) None */
	RDM_COS_GU_PERSISTENT_QUEUE	= 1  /*!< (1) Messages for queue streams are persisted for guaranteed delivery. */
} RDMClassOfServiceGuaranteeType;

/** 
 * @brief General OMM strings associated with the different class of service guarantee types.
 * @see RDMClassOfServiceGuaranteeType, rsslRDMCosGuaranteeTypeToOmmString
 */
static const RsslBuffer RDM_OMMSTR_COS_GU_NONE = { 4, (char*)"None" };
static const RsslBuffer RDM_OMMSTR_COS_GU_PERSISTENT_QUEUE = { 15, (char*)"PersistentQueue" };

/** 
 * @brief Converts the provided class-of-service guarantee type enumeration to a general OMM string.
 *
 * @param type Class-of-service guarantee type to translate to string
 * @return Null terminated character string containing the name of the type
 * @see RDMClassOfServiceGuaranteeType
 */
RSSL_API const char* rsslRDMCosGuaranteeTypeToOmmString(RsslUInt type);

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
static const RsslBuffer RSSL_ENAME_AUTHN_TOKEN = { 19, (char*)"AuthenticationToken" };
static const RsslBuffer RSSL_ENAME_AUTHN_EXTENDED = { 22, (char*)"AuthenticationExtended" };
static const RsslBuffer RSSL_ENAME_AUTHN_TT_REISSUE = { 23, (char*)"AuthenticationTTReissue" };
static const RsslBuffer RSSL_ENAME_AUTHN_EXTENDED_RESP = { 26, (char*)"AuthenticationExtendedResp" };
static const RsslBuffer RSSL_ENAME_AUTHN_ERROR_CODE = { 23, (char*)"AuthenticationErrorCode" };
static const RsslBuffer RSSL_ENAME_AUTHN_ERROR_TEXT = { 23, (char*)"AuthenticationErrorText" };
static const RsslBuffer RSSL_ENAME_UPDATE_TYPE_FILTER = { 16 , (char*)"UpdateTypeFilter" };
static const RsslBuffer RSSL_ENAME_NEGATIVE_UPDATE_TYPE_FILTER = { 24, (char*)"NegativeUpdateTypeFilter" };

//Round Trip Time - well known element names
static const RsslBuffer RSSL_ENAME_RTT = { 16 , (char*)"RoundTripLatency" };
static const RsslBuffer RSSL_ENAME_RTT_TICKS = { 5 , (char*)"Ticks" };
static const RsslBuffer RSSL_ENAME_RTT_TCP_RETRANS = { 10 , (char*)"TcpRetrans" };

//Warm Standby - Well known Element Names
static const RsslBuffer RSSL_ENAME_SUPPORT_STANDBY = { 14 , (char*)"SupportStandby" };
static const RsslBuffer RSSL_ENAME_WARMSTANDBY_INFO = { 15 , (char*)"WarmStandbyInfo" };
static const RsslBuffer RSSL_ENAME_WARMSTANDBY_MODE = { 15 , (char*)"WarmStandbyMode" };
static const RsslBuffer RSSL_ENAME_CONS_CONN_STATUS = { 24 , (char*)"ConsumerConnectionStatus" };
static const RsslBuffer RSSL_ENAME_SUPPORT_STANDBY_MODE = { 18 , (char*)"SupportStandbyMode" };

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
/* SOURCE_MIROR deprecated, use SOURCE_MIRROR */
static const RsslBuffer RSSL_ENAME_SOURCE_MIROR_MODE = { 19 , (char*)"SourceMirroringMode" };
static const RsslBuffer RSSL_ENAME_SOURCE_MIRROR_MODE = { 19 , (char*)"SourceMirroringMode" };
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
// LSEG claims empty namespace (e.g. :ItemList is Refinitv namespace)
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

