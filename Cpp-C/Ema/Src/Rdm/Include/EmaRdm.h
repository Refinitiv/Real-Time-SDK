/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_rdm_EmaRdm_h
#define __thomsonreuters_ema_rdm_EmaRdm_h

/**
	@file EmaRdm.h "Rdm/Include/EmaRdm.h"
	@brief EmaRdm.h file contains RDM constants and definitions.

	Reuters Domain Models. The EMA RDM package consists of a header file containing
	the Reuters Domain Model specific definitions that can be used by the EMA applications.
	The definitions in this file will be made extensible in the future.
*/

#include "Access/Include/EmaString.h"

/**
	@namespace thomsonreuters
	@brief The thomsonreuters namespace conatins all interfaces defined by Thomson Reuters.
*/
namespace thomsonreuters {

/**
	@namespace ema
	@brief The ema namespace contains all interfaces and definitions specified for use with EMA.
*/
namespace ema {

/**
	@namespace rdm
	@brief The rdm namespace contains all RDM definitions and constants.
*/
namespace rdm {

 
///@name Domain Types
//@{
/** Domain Types also known as RDM message model types describe message domain.
	\remark Values 0-127 are reserved.
*/
static const thomsonreuters::ema::access::UInt8 MMT_LOGIN					= 1;	/*!< Login Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_DIRECTORY				= 4;	/*!< Source Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_DICTIONARY				= 5;	/*!< Dictionary Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_MARKET_PRICE			= 6;	/*!< MarketPrice Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_MARKET_BY_ORDER			= 7;	/*!< Market by Order/Order Book Model Message Type */
static const thomsonreuters::ema::access::UInt8 MMT_MARKET_BY_PRICE			= 8;	/*!< Market by Price/Market Depth Model Message Type */
static const thomsonreuters::ema::access::UInt8 MMT_MARKET_MAKER			= 9;	/*!< Market Maker Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_SYMBOL_LIST				= 10;	/*!< Symbol List Messages Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_SERVICE_PROVIDER_STATUS = 11;	/*!< Service Provider Status Message Model Type*/
static const thomsonreuters::ema::access::UInt8 MMT_HISTORY					= 12;	/*!< History Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_HEADLINE				= 13;	/*!< Headline Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_STORY					= 14;	/*!< Story Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_REPLAYHEADLINE			= 15;	/*!< Replay Headline Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_REPLAYSTORY				= 16;	/*!< Replay Story Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_TRANSACTION				= 17;	/*!< Transaction Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_YIELD_CURVE				= 22;	/*!< Yield Curve Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_CONTRIBUTION			= 27;	/*!< Contribution Message Model Type */
static const thomsonreuters::ema::access::UInt8 MMT_PROVIDER_ADMIN			= 29;	
static const thomsonreuters::ema::access::UInt8 MMT_ANALYTICS				= 30;	/*!< Analytics content */
static const thomsonreuters::ema::access::UInt8 MMT_REFERENCE				= 31;	/*!< Reference content */
static const thomsonreuters::ema::access::UInt8 MMT_NEWS_TEXT_ANALYTICS		= 33;	/*!< News Text Analytics domain for machine readable news content */
static const thomsonreuters::ema::access::UInt8 MMT_SYSTEM					= 127;	/*!< System domain for use with domain netural content (e.g. tunnel stream creation) */
static const thomsonreuters::ema::access::UInt8 MMT_MAX_RESERVED			= 127;	/*!< Maximum reserved message model type value */
static const thomsonreuters::ema::access::UInt8 MMT_MAX_VALUE				= 255;	/*!< Maximum message model type value */
//@}

///@name User Name Types
//@{
/** User name types define specify interpretation of the string containing user name.
	User name types are used on login domain messages
*/
static const thomsonreuters::ema::access::UInt8 USER_NAME	        =	1;	/*!< String defining User Name */
static const thomsonreuters::ema::access::UInt8 USER_EMAIL_ADDRESS	=	2;	/*!< String defining Email address */
static const thomsonreuters::ema::access::UInt8 USER_TOKEN			=	3;	/*!< String defining User Token */
//@}

///@name Login Roles
//@{
/** Login role specifies if a given client is a consumer or provider.
	Login roles are used on login domain request messages
*/
static const thomsonreuters::ema::access::UInt32 LOGIN_ROLE_CONS     = 0;    /*!< log in as consumer */
static const thomsonreuters::ema::access::UInt32 LOGIN_ROLE_PROV     = 1;    /*!< log in as provider */
//@}

///@name Dictionary Verbosity
//@{
/** Dictionary verbosity defines how much information and description is
	contained in a dictionary.
	Dictionary verbosity is used on dictionary domain messages.
*/
static const thomsonreuters::ema::access::UInt32 DICTIONARY_INFO      = 0x00;	/*!< "Dictionary Info" Verbosity, no data - version information only */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_MINIMAL   = 0x03;   /*!< "Minimal" Verbosity, e.g. Cache + ShortName */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_NORMAL    = 0x07;   /*!< "Normal" Verbosity, e.g. all but description */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_VERBOSE   = 0x0F;   /*!< "Verbose" Verbosity, e.g. all with description */
//@}

///@name Dictionary Type
//@{
/** Dictionary type defines type of a dictionary.
	Dictionary type is used on dictionary domain messages.
*/
static const thomsonreuters::ema::access::UInt32 DICTIONARY_UNSPECIFIED			= 0; /*!< Unspecified dictionary type */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_FIELD_DEFINITIONS	= 1; /*!< Field Definition dictionary */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_ENUM_TABLES			= 2; /*!< Enumeration dictionary */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_RECORD_TEMPLATES	= 3; /*!< Record template dictionary */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_DISPLAY_TEMPLATES	= 4; /*!< Display template dictionary */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_DATA_DEFINITIONS	= 5; /*!< DataDef dictionary */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_STYLE_SHEET			= 6; /*!< Style sheet */
static const thomsonreuters::ema::access::UInt32 DICTIONARY_REFERENCE			= 7; /*!< Reference dictionary */
//@}

///@name Service Filter
//@{
/** Service filter specifies which service filters are requested and provided.
	Service filter is used on directory domain messages.
*/
static const thomsonreuters::ema::access::UInt32 SERVICE_INFO_FILTER     = 0x00000001;  /*!< Service Info Filter */
static const thomsonreuters::ema::access::UInt32 SERVICE_STATE_FILTER    = 0x00000002;  /*!< Service State Filter */
static const thomsonreuters::ema::access::UInt32 SERVICE_GROUP_FILTER    = 0x00000004;  /*!< Service Group Filter */
static const thomsonreuters::ema::access::UInt32 SERVICE_LOAD_FILTER     = 0x00000008;  /*!< Service Load Filter */
static const thomsonreuters::ema::access::UInt32 SERVICE_DATA_FILTER     = 0x00000010;  /*!< Service Data Filter */
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_FILTER     = 0x00000020;	/*!< Service Symbol List Filter */
//@}

///@name Service Filter Id
//@{
/** Service filter Id describes identity of a service filter.
	Service filter Id is used on directory domain messages.
*/
static const thomsonreuters::ema::access::UInt8 SERVICE_INFO_ID			= 1; /*!< Service Info Filter */
static const thomsonreuters::ema::access::UInt8 SERVICE_STATE_ID		= 2; /*!< Service State Filter */
static const thomsonreuters::ema::access::UInt8 SERVICE_GROUP_ID		= 3; /*!< Service Group Filter */
static const thomsonreuters::ema::access::UInt8 SERVICE_LOAD_ID			= 4; /*!< Service Load Filter */
static const thomsonreuters::ema::access::UInt8 SERVICE_DATA_ID			= 5; /*!< Service Data Filter */
static const thomsonreuters::ema::access::UInt8 SERVICE_LINK_ID			= 6;/*!< Service Symbol List Filter */
//@}

///@name Service Accepting Requests
//@{
/** Service accepting requests describes if a given service accepts item requests.
*/
static const thomsonreuters::ema::access::UInt32 SERVICE_YES = 1;	/*!< Service up, Service accepting requests, or link up */
static const thomsonreuters::ema::access::UInt32 SERVICE_NO	 = 0;	/*!< Service down, Service not accepting requests, or link down */
//@}

///@name Service Link Types
//@{
/** Service link type describes type of a link from which a service is sourced.
*/
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_INTERACTIVE	= 1;	/*!< Link type is interactive */
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_BROADCAST		= 2;	/*!< Link type is broadcast */
//@}

///@name Service Link Codes
//@{
/** Service link code provides more info about a link from which a service is sourced.
*/
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_CODE_NONE				  = 0;	/*!< No information is available */
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_CODE_OK				  = 1;	/*!< Link is ok */
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_CODE_RECOVERY_STARTED   = 2;	/*!< Link has started to recover */
static const thomsonreuters::ema::access::UInt32 SERVICE_LINK_CODE_RECOVERY_COMPLETED = 3;	/*!< Link has completed recovery */
//@}

///@name Service Data Filter Data Type
//@{
/** Service Data Filter data type specifies type of data present in the service data filter.
*/
static const thomsonreuters::ema::access::UInt32 SERVICE_DATA_TIME		= 1;	/*!< Data is a time */
static const thomsonreuters::ema::access::UInt32 SERVICE_DATA_ALERT		= 2;	/*!< Data is an alert */
static const thomsonreuters::ema::access::UInt32 SERVICE_DATA_HEADLINE	= 3;	/*!< Data is a headline */
static const thomsonreuters::ema::access::UInt32 SERVICE_DATA_STATUS	= 4;	/*!< Data is a status */
//@}

///@name Instrument Name Types
//@{
/** Instrument name types specify type of item name.
	Instrument name types are used on market doamin messages (e.g., MarketPrice domain)
*/
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_NAME_UNSPECIFIED     = 0;		/*!< Symbology is not specified or not applicable */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_NAME_RIC             = 1;		/*!< Reuters Instrument Code */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_NAME_CONTRIBUTOR     = 2;		/*!< Contribution Instrument Code */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_NAME_MAX_RESERVED    = 127;		/*!< Maximum reserved Name Type*/
//@}

///@name Instrument Update Type Numbers
//@{
/** Instrument update type number specifies type of updates
*/
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_UNSPECIFIED		= 0;		/*!< Not specified */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_QUOTE				= 1;		/*!< Quote */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_TRADE				= 2;		/*!< Trade */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_NEWS_ALERT		= 3;		/*!< News Alert */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_VOLUME_ALERT		= 4;		/*!< Volume Alert */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_ORDER_INDICATION	= 5;		/*!< Order Indication */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_CLOSING_RUN		= 6;		/*!< Closing Run */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_CORRECTION		= 7;		/*!< Correction */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_MARKET_DIGEST		= 8;		/*!< Official information from the exchange */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_QUOTES_TRADE		= 9;		/*!< One or more conflated quotes followed by a trade */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_MULTIPLE			= 10;		/*!< Update with other filtering and conflation applied */
static const thomsonreuters::ema::access::UInt8 INSTRUMENT_UPDATE_VERIFY			= 11;		/*!< Fields may have changed */
//@}

///@name View Type Numbers
//@{
/** View type number specifies how view information is encoded.
*/
static const thomsonreuters::ema::access::UInt32 VT_FIELD_ID_LIST       = 1;	/*!< View data is array of field ids  */
static const thomsonreuters::ema::access::UInt32 VT_ELEMENT_NAME_LIST   = 2;	/*!< View data is array of element names */
//@}

///@name Support Batch Request Type
//@{
/** Support batch request type describes what batch request is supported.
	A provider specifies what type of batch requesting it does support.
*/
static const thomsonreuters::ema::access::UInt32 SUPPORT_BATCH_REQUEST      = 0x001;	/*!< Support batch request */
static const thomsonreuters::ema::access::UInt32 SUPPORT_BATCH_REISSUE      = 0x002;	/*!< Support batch reissue */
static const thomsonreuters::ema::access::UInt32 SUPPORT_BATCH_CLOSE		= 0x004;	/*!< Support batch close */
//@}

///@name Support Enhanced SymbolList Type
//@{
/** Support enhanced symbolList type describes what enhanced symbolList request is supported.
	A provider specifies what type of enhanced symbolList requesting it does support.
*/
static const thomsonreuters::ema::access::UInt32 SUPPORT_SYMBOL_LIST_NAMES_ONLY		 = 0x000;	/*!< Supports Symbol List names only. */
static const thomsonreuters::ema::access::UInt32 SUPPORT_SYMBOL_LIST_DATA_STREAMS    = 0x001;	/*!< Supports Symbol List data streams. */
//@}

///@name Enhanced SymbolList Request Type
//@{
/** Enhanced symbolList request type describes type of symbolList request.
	A consumer specifies what type of enhanced symbolList request it wants.
*/
static const thomsonreuters::ema::access::UInt32 SYMBOL_LIST_NAMES_ONLY 	   = 0x000;	/*!< Requesting for names only and no data. */
static const thomsonreuters::ema::access::UInt32 SYMBOL_LIST_DATA_STREAMS	   = 0x001;	/*!< Requesting for streaming behavior of datastreams. */
static const thomsonreuters::ema::access::UInt32 SYMBOL_LIST_DATA_SNAPSHOTS    = 0x002;	/*!< Requesting for snapshot behavior of datastreams. */
//@}

///@name Login Attribute Names
//@{
/** Well known login attribute names.
*/
static const  thomsonreuters::ema::access::EmaString ENAME_APP_ID( "ApplicationId", 13 );
static const  thomsonreuters::ema::access::EmaString ENAME_APP_NAME( "ApplicationName", 15 );
static const  thomsonreuters::ema::access::EmaString ENAME_APPAUTH_TOKEN ( "ApplicationAuthorizationToken", 29 );
static const  thomsonreuters::ema::access::EmaString ENAME_POSITION( "Position", 8 );
static const  thomsonreuters::ema::access::EmaString ENAME_PASSWORD( "Password", 8 );
static const  thomsonreuters::ema::access::EmaString ENAME_PROV_PERM_PROF( "ProvidePermissionProfile", 24 );
static const  thomsonreuters::ema::access::EmaString ENAME_PROV_PERM_EXP( "ProvidePermissionExpressions", 28 );
static const  thomsonreuters::ema::access::EmaString ENAME_ALLOW_SUSPECT_DATA( "AllowSuspectData", 16 );
static const  thomsonreuters::ema::access::EmaString ENAME_SINGLE_OPEN( "SingleOpen", 10 );
static const  thomsonreuters::ema::access::EmaString ENAME_INST_ID( "InstanceId", 10 );
static const  thomsonreuters::ema::access::EmaString ENAME_ROLE( "Role", 4 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_PR( "SupportPauseResume", 18 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_OPR( "SupportOptimizedPauseResume", 27 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_POST( "SupportOMMPost", 14 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_BATCH( "SupportBatchRequests", 20 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_VIEW( "SupportViewRequests", 19 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_ENH_SYMBOL_LIST( "SupportEnhancedSymbolList", 25 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD( "SupportProviderDictionaryDownload", 33 );
static const  thomsonreuters::ema::access::EmaString ENAME_SEQUENCE_RETRY_INTERVAL( "SequenceRetryInterval", 21 );
static const  thomsonreuters::ema::access::EmaString ENAME_UPDATE_BUFFER_LIMIT( "UpdateBufferLimit", 17 );
static const  thomsonreuters::ema::access::EmaString ENAME_SEQUENCE_NUMBER_RECOVERY( "SequenceNumberRecovery", 22 );
//@}

///@name Source Directory Names
//@{
/** Well known source directory names.
*/
static const  thomsonreuters::ema::access::EmaString ENAME_NAME( "Name", 4 );
static const  thomsonreuters::ema::access::EmaString ENAME_SERVICE_ID( "ServiceID", 9 );
static const  thomsonreuters::ema::access::EmaString ENAME_VENDOR( "Vendor", 6 );
static const  thomsonreuters::ema::access::EmaString ENAME_IS_SOURCE( "IsSource", 8 );
static const  thomsonreuters::ema::access::EmaString ENAME_CAPABILITIES( "Capabilities", 12 );
static const  thomsonreuters::ema::access::EmaString ENAME_DICTIONARYS_PROVIDED( "DictionariesProvided", 20 );
static const  thomsonreuters::ema::access::EmaString ENAME_DICTIONARYS_USED( "DictionariesUsed", 16 );
static const  thomsonreuters::ema::access::EmaString ENAME_QOS( "QoS", 3 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPS_QOS_RANGE( "SupportsQoSRange", 16 );
static const  thomsonreuters::ema::access::EmaString ENAME_ITEM_LIST( "ItemList", 8 );
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPS_OOB_SNAPSHOTS( "SupportsOutOfBandSnapshots", 26 );
static const  thomsonreuters::ema::access::EmaString ENAME_ACCEPTING_CONS_STATUS( "AcceptingConsumerStatus", 23 );
static const  thomsonreuters::ema::access::EmaString ENAME_CONS_SOURCE_MIROR_MODE( "SourceMirroringMode", 19 );
static const  thomsonreuters::ema::access::EmaString ENAME_CONS_STATUS( "ConsumerStatus", 14 );
static const  thomsonreuters::ema::access::EmaString ENAME_SVC_STATE( "ServiceState", 12 );
static const  thomsonreuters::ema::access::EmaString ENAME_ACCEPTING_REQS( "AcceptingRequests", 17 );
static const  thomsonreuters::ema::access::EmaString ENAME_STATUS( "Status", 6 );
static const  thomsonreuters::ema::access::EmaString ENAME_GROUP( "Group", 5 );
static const  thomsonreuters::ema::access::EmaString ENAME_MERG_TO_GRP( "MergedToGroup", 13 );
static const  thomsonreuters::ema::access::EmaString ENAME_OPEN_LIMIT( "OpenLimit", 9 );
static const  thomsonreuters::ema::access::EmaString ENAME_OPEN_WINDOW( "OpenWindow", 10 );
static const  thomsonreuters::ema::access::EmaString ENAME_LOAD_FACT( "LoadFactor", 10 );
static const  thomsonreuters::ema::access::EmaString ENAME_TYPE( "Type", 4 );
static const  thomsonreuters::ema::access::EmaString ENAME_DATA( "Data", 4 );
static const  thomsonreuters::ema::access::EmaString ENAME_LINK_STATE( "LinkState", 9 );
static const  thomsonreuters::ema::access::EmaString ENAME_LINK_CODE( "LinkCode", 8 );
//@}

///@name Server Configuration Names
//@{
/** Well known server configuration names.
*/
static const  thomsonreuters::ema::access::EmaString ENAME_SUPPORT_STANDBY( "SupportStandby", 14 );
static const  thomsonreuters::ema::access::EmaString ENAME_WARMSTANDBY_INFO( "WarmStandbyInfo", 15 );
static const  thomsonreuters::ema::access::EmaString ENAME_WARMSTANDBY_MODE( "WarmStandbyMode", 15 );
static const  thomsonreuters::ema::access::EmaString ENAME_CONS_CONN_STATUS( "ConsumerConnectionStatus", 24 );
static const  thomsonreuters::ema::access::EmaString ENAME_DOWNLOAD_CON_CONFIG( "DownloadConnectionConfig", 24 );
static const  thomsonreuters::ema::access::EmaString ENAME_CONNECTION_CONFIG( "ConnectionConfig", 16 );
static const  thomsonreuters::ema::access::EmaString ENAME_NUM_STANDBY_SERVERS( "NumStandbyServers", 17 );
static const  thomsonreuters::ema::access::EmaString ENAME_HOSTNAME( "Hostname", 8 );
static const  thomsonreuters::ema::access::EmaString ENAME_PORT( "Port", 4 );
static const  thomsonreuters::ema::access::EmaString ENAME_SERVER_TYPE( "ServerType", 10 );
static const  thomsonreuters::ema::access::EmaString ENAME_SYSTEM_ID( "SystemID", 8 );
//@}

///@name Dictionary Names
//@{
/** Well known dictionary names.
*/
static const thomsonreuters::ema::access::EmaString ENAME_DICTIONARY_ID( "DictionaryId", 12 );
static const thomsonreuters::ema::access::EmaString ENAME_DICT_TYPE( "Type", 4 );
static const thomsonreuters::ema::access::EmaString ENAME_DICT_VERSION( "Version", 7 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_NAME( "NAME", 4 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_ID( "FID", 3 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_RIPPLETO( "RIPPLETO", 8 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_TYPE( "TYPE", 4 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_LENGTH( "LENGTH", 6 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_RWFTYPE( "RWFTYPE", 7 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_RWFLENGTH( "RWFLEN", 6  );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_ENUMLENGTH( "ENUMLENGTH", 10 );
static const thomsonreuters::ema::access::EmaString ENAME_FIELD_LONGNAME( "LONGNAME", 8 );

static const thomsonreuters::ema::access::EmaString ENAME_ENUM_RT_VERSION( "RT_Version", 10 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_DT_VERSION( "DT_Version", 10 );

static const thomsonreuters::ema::access::EmaString ENAME_ENUM_FIDS( "FIDS", 4 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_FID( "FID", 3 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_VALUES( "VALUES", 6 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_VALUE( "VALUE", 5 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_DISPLAYS( "DISPLAYS", 8 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_DISPLAY( "DISPLAY", 7 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_MEANINGS( "MEANINGS", 8 );
static const thomsonreuters::ema::access::EmaString ENAME_ENUM_MEANING( "MEANING", 7 );

static const  thomsonreuters::ema::access::EmaString ENAME_TEXT( "Text", 4 );
static const  thomsonreuters::ema::access::EmaString ENAME_VERSION( "Version", 7 );
//@}

///@name Batch and View Names
//@{
/** Well known batch and view names.
*/
static const  thomsonreuters::ema::access::EmaString ENAME_BATCH_ITEM_LIST( ":ItemList", 9 );
static const  thomsonreuters::ema::access::EmaString ENAME_VIEW_TYPE( ":ViewType", 9 );
static const  thomsonreuters::ema::access::EmaString ENAME_VIEW_DATA( ":ViewData", 9 );
static const  thomsonreuters::ema::access::EmaString ENAME_SYMBOL_LIST_BEHAVIORS( ":SymbolListBehaviors", 20 );
static const  thomsonreuters::ema::access::EmaString ENAME_DATA_STREAMS( ":DataStreams", 12 );
//@}

}

}

}

#endif // __thomsonreuters_ema_rdm_EmaRdm_h
