///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.rdm;

/**
 * This file contains RDM constants and definitions.
 *
 * <p>Thomson Reuters Domain Models.
 * <br>This file contains the Thomson Reuters Domain Model specific definitions
 * that can be used by the EMA applications.
 * <br>The definitions in this file will be made extensible in the future.</p>
 */
public class EmaRdm
{
    private EmaRdm()
    {
        throw new AssertionError();
    }
    
    
	//Values used for Domain Types
	//Domain Types also known as RDM message model types. They describe message domain.
	//Values 0-127 are reserved.
      /**
       * Login Message Model Type
       */
	  public static final int MMT_LOGIN						= 1;
	  
	  /**
	   * Source Message Model Type
	   */
	  public static final int MMT_DIRECTORY					= 4;
	  
	  /**
	   * Dictionary Message Model Type
	   */
	  public static final int MMT_DICTIONARY				= 5;
	  
	  /**
	   * MarketPrice Message Model Type
	   */
	  public static final int MMT_MARKET_PRICE				= 6;
	  
	  /**
	   * Market by Order/Order Book Model Message Type
	   */
	  public static final int MMT_MARKET_BY_ORDER			= 7;
	  
	  /**
	   * Market by Price/Market Depth Model Message Type
	   */
	  public static final int MMT_MARKET_BY_PRICE			= 8;
	  
	  /**
	   * Market Maker Message Model Type
	   */
	  public static final int MMT_MARKET_MAKER				= 9;
	  
	  /**
	   * Symbol List Messages Model Type
	   */
	  public static final int MMT_SYMBOL_LIST				= 10;
	  
	  /**
	   * Service Provider Status Message Model Type
	   */
	  public static final int MMT_SERVICE_PROVIDER_STATUS 	= 11;
	  
	  /**
	   * History Message Model Type
	   */
	  public static final int MMT_HISTORY					= 12;
	  
	  /**
	   * Headline Message Model Type
	   */
	  public static final int MMT_HEADLINE					= 13;
	  
	  /**
	   * Story Message Model Type
	   */
	  public static final int MMT_STORY						= 14;
	  
	  /**
	   * Replay Headline Message Model Type
	   */
	  public static final int MMT_REPLAYHEADLINE			= 15;
	  
	  /**
	   * Replay Story Message Model Type
	   */
	  public static final int MMT_REPLAYSTORY				= 16;
	  
	  /**
	   * Transaction Message Model Type
	   */
	  public static final int MMT_TRANSACTION				= 17;
	  
	  /**
	   * Yield Curve Message Model Type
	   */
	  public static final int MMT_YIELD_CURVE				= 22;
	  
	  /**
	   * Contribution Message Model Type
	   */
	  public static final int MMT_CONTRIBUTION				= 27;
	  
	  /**
	   * Provider AdminMessage Model Type
	   */
	  public static final int MMT_PROVIDER_ADMIN			= 29;	
	  
	  /**
	   * Analytics content Message Model Type
	   */
	  public static final int MMT_ANALYTICS					= 30;
	  
	  /**
	   * Reference content Message Model Type
	   */
	  public static final int MMT_REFERENCE					= 31;
	  
	  /**
	   * News Text Analytics domain for machine readable news content 
	   */
	  public static final int MMT_NEWS_TEXT_ANALYTICS		= 33;
	  
	  /**
	   * System domain for use with domain neutral content (e.g. tunnel stream creation)
	   */
	  public static final int MMT_SYSTEM					= 127;
	  
	  /**
	   * Maximum reserved message model type value
	   */
	  public static final int MMT_MAX_RESERVED				= 127;
	  
	  /**
	   * Maximum message model type value
	   */
	  public static final int MMT_MAX_VALUE					= 255;
	
	  
	//User Name Types
	//User name types that define specify interpretation of the string containing user name.
	//User name types are used on login domain messages
	  /**
	   * String defining User Name
	   */
	  public static final int USER_NAME				=	1;
	  
	  /**
	   * String defining Email address
	   */
	  public static final int USER_EMAIL_ADDRESS	=	2;
	  
	  /**
	   * String defining User Token
	   */
	  public static final int USER_TOKEN			=	3;

	
	//Login Roles
	//Login role specifies if a given client is a consumer or provider.
	//Login roles are used on login domain request messages
	  /**
	   * log in as consumer
	   */
	  public static final int LOGIN_ROLE_CONS     = 0;
	  
	  /**
	   * log in as provider
	   */
	  public static final int LOGIN_ROLE_PROV     = 1;
	
	  
	//Dictionary Verbosity
	//Dictionary verbosity defines how much information and description is contained in a dictionary.
	//Dictionary verbosity is used on dictionary domain messages.
	  /**
	   * "Dictionary Info" Verbosity, no data - version information only
	   */
	  public static final int DICTIONARY_INFO      = 0x00;
	  
	  /**
	   * "Minimal" Verbosity, e.g. Cache + ShortName
	   */
	  public static final int DICTIONARY_MINIMAL   = 0x03;
	  
	  /**
	   * "Normal" Verbosity, e.g. all but description
	   */
	  public static final int DICTIONARY_NORMAL    = 0x07;
	  
	  /**
	   * "Verbose" Verbosity, e.g. all with description
	   */
	  public static final int DICTIONARY_VERBOSE   = 0x0F;
	
	  
	//Dictionary Type
	//Dictionary type defines type of a dictionary.
	//Dictionary type is used on dictionary domain messages.
	  /**
	   * Unspecified dictionary type
	   */
	  public static final int DICTIONARY_UNSPECIFIED		= 0;
	  
	  /**
	   * Field Definition dictionary
	   */
	  public static final int DICTIONARY_FIELD_DEFINITIONS	= 1;
	  
	  /**
	   * Enumeration dictionary
	   */
	  public static final int DICTIONARY_ENUM_TABLES		= 2;
	  
	  /**
	   * Record template dictionary
	   */
	  public static final int DICTIONARY_RECORD_TEMPLATES	= 3;
	  
	  /**
	   * Display template dictionary
	   */
	  public static final int DICTIONARY_DISPLAY_TEMPLATES	= 4;
	  
	  /**
	   * DataDef dictionary
	   */
	  public static final int DICTIONARY_DATA_DEFINITIONS	= 5;
	  
	  /**
	   * Style sheet
	   */
	  public static final int DICTIONARY_STYLE_SHEET		= 6;
	  
	  /**
	   * Reference dictionary
	   */
	  public static final int DICTIONARY_REFERENCE			= 7;
	
	  
	//Service Filter
	//Service filter specifies which service filters are requested and provided.
	//Service filter is used on directory domain messages.
	  /**
	   * Service Info Filter
	   */
	  public static final int SERVICE_INFO_FILTER     = 0x00000001;
	  
	  /**
	   * Service State Filter
	   */
	  public static final int SERVICE_STATE_FILTER    = 0x00000002;
	  
	  /**
	   * Service Group Filter
	   */
	  public static final int SERVICE_GROUP_FILTER    = 0x00000004;
	  
	  /**
	   * Service Load Filter
	   */
	  public static final int SERVICE_LOAD_FILTER     = 0x00000008;
	  
	  /**
	   * Service Data Filter
	   */
	  public static final int SERVICE_DATA_FILTER     = 0x00000010;
	  
	  /**
	   * Service Symbol List Filter
	   */
	  public static final int SERVICE_LINK_FILTER     = 0x00000020;
	
	  
	//Service Filter Id
	//Service filter Id describes identity of a service filter.
	//Service filter Id is used on directory domain messages.
	  /**
	   * Service Info Filter Id
	   */
	  public static final int SERVICE_INFO_ID			= 1;
	  
	  /**
	   * Service State Filter Id
	   */
	  public static final int SERVICE_STATE_ID			= 2;
	  
	  /**
	   * Service Group Filter Id
	   */
	  public static final int SERVICE_GROUP_ID			= 3;
	  
	  /**
	   * Service Load Filter Id
	   */
	  public static final int SERVICE_LOAD_ID			= 4;
	  
	  /**
	   * Service Data Filter Id
	   */
	  public static final int SERVICE_DATA_ID			= 5;
	  
	  /**
	   * Service Symbol List Filter Id
	   */
	  public static final int SERVICE_LINK_ID			= 6;
	
	  
	//Service Accepting Requests
	//Service accepting requests describes if a given service accepts item requests.
	  /**
	   * Service up, Service accepting requests, or link up
	   */
	  public static final int SERVICE_YES	= 1;
	  
	  /**
	   * Service down, Service not accepting requests, or link down
	   */
	  public static final int SERVICE_NO	= 0;
	  
	 
	//Service state
	  /**
	   * Service up
	   */
	  public static final int SERVICE_UP = 1;
	  
	  /**
	   * Service down
	   */
	  public static final int SERVICE_DOWN = 0;
	  
	  
	//Service Link Types
	//Service link type describes type of a link from which a service is sourced.
	  /**
	   * Link type is interactive
	   */
	  public static final int SERVICE_LINK_INTERACTIVE	 = 1;
	  
	  /**
	   * Link type is broadcast
	   */
	  public static final int SERVICE_LINK_BROADCAST	 = 2;
	
	  
	//Service Link Codes
	//Service link code provides more info about a link from which a service is sourced.
	  /**
	   * No information is available
	   */
	  public static final int SERVICE_LINK_CODE_NONE				= 0;
	  
	  /**
	   * Link is ok
	   */
	  public static final int SERVICE_LINK_CODE_OK				    = 1;
	  
	  /**
	   * Link has started to recover
	   */
	  public static final int SERVICE_LINK_CODE_RECOVERY_STARTED    = 2;
	  
	  /**
	   * Link has completed recovery
	   */
	  public static final int SERVICE_LINK_CODE_RECOVERY_COMPLETED  = 3;
	
	  
	//Service Data Filter Data Type
	//Service Data Filter data type specifies type of data present in the service data filter.
	  /**
	   * Service Data is a time
	   */
	  public static final int SERVICE_DATA_TIME		= 1;
	  
	  /**
	   * Service Data is an alert
	   */
	  public static final int SERVICE_DATA_ALERT	= 2;
	  
	  /**
	   * Service Data is a headline
	   */
	  public static final int SERVICE_DATA_HEADLINE	= 3;
	  
	  /**
	   * Service Data is a status
	   */
	  public static final int SERVICE_DATA_STATUS	= 4;
	
	  
	//Instrument Name Types
	//Instrument name types specify type of item name.
	//Instrument name types are used on market domain messages (e.g. MarketPrice domain)
	  /**
	   * Symbology is not specified or not applicable
	   */
	  public static final int INSTRUMENT_NAME_UNSPECIFIED     = 0;
	  
	  /**
	   * Reuters Instrument Code
	   */
	  public static final int INSTRUMENT_NAME_RIC             = 1;
	  
	  /**
	   * Contribution Instrument Code
	   */
	  public static final int INSTRUMENT_NAME_CONTRIBUTOR     = 2;
	  
	  /**
	   * Maximum reserved Name Type
	   */
	  public static final int INSTRUMENT_NAME_MAX_RESERVED    = 127;
	
	  
	//Instrument Update Type Numbers
	//Instrument update type number specifies type of updates.
	  /**
	   * Not specified
	   */
	  public static final int INSTRUMENT_UPDATE_UNSPECIFIED			= 0;
	  
	  /**
	   * Quote
	   */
	  public static final int INSTRUMENT_UPDATE_QUOTE				= 1;
	  
	  /**
	   * Trade
	   */
	  public static final int INSTRUMENT_UPDATE_TRADE				= 2;
	  
	  /**
	   * News Alert
	   */
	  public static final int INSTRUMENT_UPDATE_NEWS_ALERT			= 3;
	  
	  /**
	   * Volume Alert
	   */
	  public static final int INSTRUMENT_UPDATE_VOLUME_ALERT		= 4;
	  
	  /**
	   * Order Indication
	   */
	  public static final int INSTRUMENT_UPDATE_ORDER_INDICATION	= 5;
	  
	  /**
	   * Closing Run
	   */
	  public static final int INSTRUMENT_UPDATE_CLOSING_RUN			= 6;
	  
	  /**
	   * Correction
	   */
	  public static final int INSTRUMENT_UPDATE_CORRECTION			= 7;
	  
	  /**
	   * Official information from the exchange
	   */
	  public static final int INSTRUMENT_UPDATE_MARKET_DIGEST		= 8;
	  
	  /**
	   * One or more conflated quotes followed by a trade
	   */
	  public static final int INSTRUMENT_UPDATE_QUOTES_TRADE		= 9;
	  
	  /**
	   * Update with other filtering and conflation applied
	   */
	  public static final int INSTRUMENT_UPDATE_MULTIPLE			= 10;
	  
	  /**
	   * Fields may have changed
	   */
	  public static final int INSTRUMENT_UPDATE_VERIFY				= 11;
	
	  
	//View Type Numbers
	//View type number specifies how view information is encoded.
	  /**
	   * View data is array of field ids
	   */
	  public static final int VT_FIELD_ID_LIST       = 1;
	  
	  /**
	   * View data is array of element names
	   */
	  public static final int VT_ELEMENT_NAME_LIST   = 2;
	
	  
	//Support Batch Request Type
	//Support batch request type describes what batch request is supported.
	//A provider specifies what type of batch requesting it supports.
	  /**
	   * Support batch request
	   */
	  public static final int SUPPORT_BATCH_REQUEST		= 0x001;
	  
	  /**
	   * Support batch reissue
	   */
	  public static final int SUPPORT_BATCH_REISSUE     = 0x002;
	  
	  /**
	   * Support batch close
	   */
	  public static final int SUPPORT_BATCH_CLOSE		= 0x004;
	
	  
	//Support Enhanced SymbolList Type
	//Support enhanced symbolList type describes what enhanced symbolList request is supported.
	//A provider specifies what type of enhanced symbolList requesting it supports.
	  /**
	   * Supports Symbol List names only.
	   */
	  public static final int SUPPORT_SYMBOL_LIST_NAMES_ONLY		= 0x000;
	  
	  /**
	   * Supports Symbol List data streams.
	   */
	  public static final int SUPPORT_SYMBOL_LIST_DATA_STREAMS    	= 0x001;
	
	  
	//Enhanced SymbolList Request Type
	//Enhanced symbolList request type describes type of symbolList request.
	//A consumer specifies what type of enhanced symbolList request it wants.
	  /**
	   * Requesting for names only and no data.
	   */
	  public static final int SYMBOL_LIST_NAMES_ONLY		= 0x000;
	  
	  /**
	   * Requesting for streaming behavior of datastreams.
	   */
	  public static final int SYMBOL_LIST_DATA_STREAMS	   	= 0x001;
	  
	  /**
	   * Requesting for snapshot behavior of datastreams
	   */
	  public static final int SYMBOL_LIST_DATA_SNAPSHOTS   	= 0x002;
	
	  
	  public static final String ENAME_APP_ID = "ApplicationId";
	  public static final String ENAME_APP_NAME = "ApplicationName";
	  public static final String ENAME_APPAUTH_TOKEN  = "ApplicationAuthorizationToken";
	  public static final String ENAME_POSITION = "Position";
	  public static final String ENAME_PASSWORD = "Password";
	  public static final String ENAME_PROV_PERM_PROF = "ProvidePermissionProfile";
	  public static final String ENAME_PROV_PERM_EXP = "ProvidePermissionExpressions";
	  public static final String ENAME_ALLOW_SUSPECT_DATA = "AllowSuspectData";
	  public static final String ENAME_SINGLE_OPEN = "SingleOpen";
	  public static final String ENAME_INST_ID = "InstanceId";
	  public static final String ENAME_ROLE = "Role";
	  public static final String ENAME_SUPPORT_PR = "SupportPauseResume";
	  public static final String ENAME_SUPPORT_OPR = "SupportOptimizedPauseResume";
	  public static final String ENAME_SUPPORT_POST = "SupportOMMPost";
	  public static final String ENAME_SUPPORT_BATCH = "SupportBatchRequests";
	  public static final String ENAME_SUPPORT_VIEW = "SupportViewRequests";
	  public static final String ENAME_SUPPORT_ENH_SYMBOL_LIST = "SupportEnhancedSymbolList";
	  public static final String ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = "SupportProviderDictionaryDownload";
	  public static final String ENAME_SEQUENCE_RETRY_INTERVAL = "SequenceRetryInterval";
	  public static final String ENAME_UPDATE_BUFFER_LIMIT = "UpdateBufferLimit";
	  public static final String ENAME_SEQUENCE_NUMBER_RECOVERY = "SequenceNumberRecovery";

	  
	//Well known source directory names.
	  public static final String ENAME_NAME = "Name";
	  public static final String ENAME_SERVICE_ID = "ServiceID";
	  public static final String ENAME_VENDOR = "Vendor";
	  public static final String ENAME_IS_SOURCE = "IsSource";
	  public static final String ENAME_CAPABILITIES = "Capabilities";
	  public static final String ENAME_DICTIONARYS_PROVIDED = "DictionariesProvided";
	  public static final String ENAME_DICTIONARYS_USED = "DictionariesUsed";
	  public static final String ENAME_QOS = "QoS";
	  public static final String ENAME_SUPPS_QOS_RANGE = "SupportsQoSRange";
	  public static final String ENAME_ITEM_LIST = "ItemList";
	  public static final String ENAME_SUPPS_OOB_SNAPSHOTS = "SupportsOutOfBandSnapshots";
	  public static final String ENAME_ACCEPTING_CONS_STATUS = "AcceptingConsumerStatus";
	  public static final String ENAME_CONS_SOURCE_MIROR_MODE = "SourceMirroringMode";
	  public static final String ENAME_CONS_STATUS = "ConsumerStatus";
	  public static final String ENAME_SVC_STATE = "ServiceState";
	  public static final String ENAME_ACCEPTING_REQS = "AcceptingRequests";
	  public static final String ENAME_STATUS = "Status";
	  public static final String ENAME_GROUP = "Group";
	  public static final String ENAME_MERG_TO_GRP = "MergedToGroup";
	  public static final String ENAME_OPEN_LIMIT = "OpenLimit";
	  public static final String ENAME_OPEN_WINDOW = "OpenWindow";
	  public static final String ENAME_LOAD_FACT = "LoadFactor";
	  public static final String ENAME_TYPE = "Type";
	  public static final String ENAME_DATA = "Data";
	  public static final String ENAME_LINK_STATE = "LinkState";
	  public static final String ENAME_LINK_CODE = "LinkCode";

	  
	//Well known server configuration names.
	  public static final String ENAME_SUPPORT_STANDBY = "SupportStandby";
	  public static final String ENAME_WARMSTANDBY_INFO = "WarmStandbyInfo";
	  public static final String ENAME_WARMSTANDBY_MODE = "WarmStandbyMode";
	  public static final String ENAME_CONS_CONN_STATUS = "ConsumerConnectionStatus";
	  public static final String ENAME_DOWNLOAD_CON_CONFIG = "DownloadConnectionConfig";
	  public static final String ENAME_CONNECTION_CONFIG = "ConnectionConfig";
	  public static final String ENAME_NUM_STANDBY_SERVERS = "NumStandbyServers";
	  public static final String ENAME_HOSTNAME = "Hostname";
	  public static final String ENAME_PORT = "Port";
	  public static final String ENAME_SERVER_TYPE = "ServerType";
	  public static final String ENAME_SYSTEM_ID = "SystemID";

	  
	//Well known dictionary names.
	  public static final String ENAME_DICTIONARY_ID = "DictionaryId";
	  public static final String ENAME_DICT_TYPE = "Type";
	  public static final String ENAME_DICT_VERSION = "Version";
	  public static final String ENAME_FIELD_NAME = "NAME";
	  public static final String ENAME_FIELD_ID = "FID";
	  public static final String ENAME_FIELD_RIPPLETO = "RIPPLETO";
	  public static final String ENAME_FIELD_TYPE = "TYPE";
	  public static final String ENAME_FIELD_LENGTH = "LENGTH";
	  public static final String ENAME_FIELD_RWFTYPE = "RWFTYPE";
	  public static final String ENAME_FIELD_RWFLENGTH = "RWFLEN";
	  public static final String ENAME_FIELD_ENUMLENGTH = "ENUMLENGTH";
	  public static final String ENAME_FIELD_LONGNAME = "LONGNAME";

	  public static final String ENAME_ENUM_RT_VERSION = "RT_Version";
	  public static final String ENAME_ENUM_DT_VERSION = "DT_Version";

	  public static final String ENAME_ENUM_FIDS = "FIDS";
	  public static final String ENAME_ENUM_FID = "FID";
	  public static final String ENAME_ENUM_VALUES = "VALUES";
	  public static final String ENAME_ENUM_VALUE = "VALUE";
	  public static final String ENAME_ENUM_DISPLAYS = "DISPLAYS";
	  public static final String ENAME_ENUM_DISPLAY = "DISPLAY";
	  public static final String ENAME_ENUM_MEANINGS = "MEANINGS";
	  public static final String ENAME_ENUM_MEANING = "MEANING";

	  public static final String ENAME_TEXT = "Text";
	  public static final String ENAME_VERSION = "Version";

	  
	//Well known batch and view names.
	  public static final String ENAME_BATCH_ITEM_LIST = ":ItemList";
	  public static final String ENAME_VIEW_TYPE = ":ViewType";
	  public static final String ENAME_VIEW_DATA = ":ViewData";
	  public static final String ENAME_SYMBOL_LIST_BEHAVIORS = ":SymbolListBehaviors";
	  public static final String ENAME_DATA_STREAMS = ":DataStreams";
}