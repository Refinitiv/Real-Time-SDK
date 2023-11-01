/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */


using System;

namespace LSEG.Ema.Rdm
{
    /// <summary>
    /// This file contains RDM constants and definitions.
    /// </summary>
    /// <remarks>
    /// Refinitiv Domain Models. The EMA RDM package consists of a header file containing
    /// the Refinitiv Domain Model specific definitions that can be used by the EMA applications.<br/>
    /// The definitions in this file will be made extensible in the future.
    /// </remarks>
    public sealed class EmaRdm
    {
        ///<summary>
        ///Values used for Domain Types <br/>
        ///Domain Types also known as RDM message model types. They describe message domain.<br/>
        ///Values 0-127 are reserved.<br/>
        ///</summary>

        /// <summary>
        /// Login Message Model Type
        /// </summary>
        public const int MMT_LOGIN = 1;

        /// <summary>
        /// Source Message Model Type
        /// </summary>
        public const int MMT_DIRECTORY = 4;

        /// <summary>
        /// Dictionary Message Model Type
        /// </summary>
        public const int MMT_DICTIONARY = 5;

        /// <summary>
        /// MarketPrice Message Model Type
        /// </summary>
        public const int MMT_MARKET_PRICE = 6;

        /// <summary>
        /// Market by Order/Order Book Model Message Type
        /// </summary>
        public const int MMT_MARKET_BY_ORDER = 7;

        /// <summary>
        /// Market by Price/Market Depth Model Message Type
        /// </summary>
        public const int MMT_MARKET_BY_PRICE = 8;

        /// <summary>
        /// Market Maker Message Model Type
        /// </summary>
        public const int MMT_MARKET_MAKER = 9;

        /// <summary>
        /// Symbol List Messages Model Type
        /// </summary>
        public const int MMT_SYMBOL_LIST = 10;

        /// <summary>
        /// Service Provider Status Message Model Type
        /// </summary>
        public const int MMT_SERVICE_PROVIDER_STATUS = 11;

        /// <summary>
        /// History Message Model Type
        /// </summary>
        public const int MMT_HISTORY = 12;

        /// <summary>
        /// Headline Message Model Type
        /// </summary>
        public const int MMT_HEADLINE = 13;

        /// <summary>
        /// Story Message Model Type
        /// </summary>
        public const int MMT_STORY = 14;

        /// <summary>
        /// Replay Headline Message Model Type
        /// </summary>
        public const int MMT_REPLAYHEADLINE = 15;

        /// <summary>
        /// Replay Story Message Model Type
        /// </summary>
        public const int MMT_REPLAYSTORY = 16;

        /// <summary>
        /// Transaction Message Model Type
        /// </summary>
        public const int MMT_TRANSACTION = 17;

        /// <summary>
        /// Yield Curve Message Model Type
        /// </summary>
        public const int MMT_YIELD_CURVE = 22;

        /// <summary>
        /// Contribution Message Model Type
        /// </summary>
        public const int MMT_CONTRIBUTION = 27;

        /// <summary>
        /// Provider AdminMessage Model Type
        /// </summary>
        public const int MMT_PROVIDER_ADMIN = 29;

        /// <summary>
        /// Analytics content Message Model Type
        /// </summary>
        public const int MMT_ANALYTICS = 30;

        /// <summary>
        /// Reference content Message Model Type
        /// </summary>
        public const int MMT_REFERENCE = 31;

        /// <summary>
        /// News Text Analytics domain for machine readable news content
        /// </summary>
        public const int MMT_NEWS_TEXT_ANALYTICS = 33;

        /// <summary>
        /// System domain for use with domain neutral content (e.g. tunnel stream creation)
        /// </summary>
        public const int MMT_SYSTEM = 127;

        /// <summary>
        /// Maximum reserved message model type value
        /// </summary>
        public const int MMT_MAX_RESERVED = 127;

        /// <summary>
        /// Maximum message model type value
        /// </summary>
        public const int MMT_MAX_VALUE = 255;

        ///<summary>
        ///User Name Types<br/>
        ///User name types that define specify interpretation of the string containing user name.<br/>
        ///User name types are used on login domain messages<br/>
        ///</summary>

        /// <summary>
        /// String defining User Name
        /// </summary>
        public const int USER_NAME = 1;

        /// <summary>
        /// String defining Email address
        /// </summary>
        public const int USER_EMAIL_ADDRESS = 2;

        /// <summary>
        /// String defining User Token
        /// </summary>
        public const int USER_TOKEN = 3;

        /// <summary>
        /// String defining User Cookie
        /// </summary>
        public const int USER_COOKIE = 4;

        /// <summary>
        /// String defining User Authentication
        /// </summary>
        public const int USER_AUTH_TOKEN = 5;

        ///<summary>
        ///Login Roles <br/>
        ///Login role specifies if a given client is a consumer or provider.<br/>
        ///Login roles are used on login domain request messages<br/>
        /// </summary>

        /// <summary>
        /// log in as consumer
        /// </summary>
        public const int LOGIN_ROLE_CONS = 0;

        /// <summary>
        /// log in as provider
        /// </summary>
        public const int LOGIN_ROLE_PROV = 1;

        ///<summary>
        ///Dictionary Verbosity <br/>
        ///Dictionary verbosity defines how much information and description is contained in a dictionary. <br/>
        ///Dictionary verbosity is used on dictionary domain messages. <br/>
        /// </summary>

        /// <summary>
        /// "Dictionary Info" Verbosity, no data - version information only
        /// </summary>
        public const int DICTIONARY_INFO = 0x00;

        /// <summary>
        /// "Minimal" Verbosity, e.g. Cache + ShortName
        /// </summary>
        public const int DICTIONARY_MINIMAL = 0x03;

        /// <summary>
        /// "Normal" Verbosity, e.g. all but description
        /// </summary>
        public const int DICTIONARY_NORMAL = 0x07;

        /// <summary>
        /// "Verbose" Verbosity, e.g. all with description
        /// </summary>
        public const int DICTIONARY_VERBOSE = 0x0F;

        ///<summary>
        ///Dictionary Type<br/>
        ///Dictionary type defines type of a dictionary.<br/>
        ///Dictionary type is used on dictionary domain messages.<br/>
        ///</summary>

        /// <summary>
        /// Unspecified dictionary type
        /// </summary>
        public const int DICTIONARY_UNSPECIFIED = 0;

        /// <summary>
        /// Field Definition dictionary
        /// </summary>
        public const int DICTIONARY_FIELD_DEFINITIONS = 1;

        /// <summary>
        /// Enumeration dictionary
        /// </summary>
        public const int DICTIONARY_ENUM_TABLES = 2;

        /// <summary>
        /// Record template dictionary
        /// </summary>
        public const int DICTIONARY_RECORD_TEMPLATES = 3;

        /// <summary>
        /// Display template dictionary
        /// </summary>
        public const int DICTIONARY_DISPLAY_TEMPLATES = 4;

        /// <summary>
        /// DataDef dictionary
        /// </summary>
        public const int DICTIONARY_DATA_DEFINITIONS = 5;

        /// <summary>
        /// Style sheet
        /// </summary>
        public const int DICTIONARY_STYLE_SHEET = 6;

        /// <summary>
        /// Reference dictionary
        /// </summary>
        public const int DICTIONARY_REFERENCE = 7;

        ///<summary>
        ///Service Filter <br/>
        ///Service filter specifies which service filters are requested and provided. <br/>
        ///Service filter is used on directory domain messages. <br/>
        ///</summary>

        /// <summary>
        /// Service Info Filter
        /// </summary>
        public const int SERVICE_INFO_FILTER = 0x00000001;

        /// <summary>
        /// Service State Filter
        /// </summary>
        public const int SERVICE_STATE_FILTER = 0x00000002;

        /// <summary>
        /// Service Group Filter
        /// </summary>
        public const int SERVICE_GROUP_FILTER = 0x00000004;

        /// <summary>
        /// Service Load Filter
        /// </summary>
        public const int SERVICE_LOAD_FILTER = 0x00000008;

        /// <summary>
        /// Service Data Filter
        /// </summary>
        public const int SERVICE_DATA_FILTER = 0x00000010;

        /// <summary>
        /// Service Symbol List Filter
        /// </summary>
        public const int SERVICE_LINK_FILTER = 0x00000020;

        ///<summary>
        ///Service Filter Id <br/>
        ///Service filter Id describes identity of a service filter. <br/>
        ///Service filter Id is used on directory domain messages. <br/>
        ///</summary>

        /// <summary>
        /// Service Info Filter Id
        /// </summary>
        public const int SERVICE_INFO_ID = 1;

        /// <summary>
        /// Service State Filter Id
        /// </summary>
        public const int SERVICE_STATE_ID = 2;

        /// <summary>
        /// Service Group Filter Id
        /// </summary>
        public const int SERVICE_GROUP_ID = 3;

        /// <summary>
        /// Service Load Filter Id
        /// </summary>
        public const int SERVICE_LOAD_ID = 4;

        /// <summary>
        /// Service Data Filter Id
        /// </summary>
        public const int SERVICE_DATA_ID = 5;

        /// <summary>
        /// Service Symbol List Filter Id
        /// </summary>
        public const int SERVICE_LINK_ID = 6;

        ///<summary>
        ///Service Accepting Requests <br/>
        ///Service accepting requests describes if a given service accepts item requests.<br/>
        /// </summary>

        /// <summary>
        /// Service up, Service accepting requests, or link up
        /// </summary>
        public const int SERVICE_YES = 1;

        /// <summary>
        /// Service down, Service not accepting requests, or link down
        /// </summary>
        public const int SERVICE_NO = 0;

        ///<summary>
        ///Service state
        /// </summary>

        /// <summary>
        /// Service up
        /// </summary>
        public const int SERVICE_UP = 1;

        /// <summary>
        /// Service down
        /// </summary>
        public const int SERVICE_DOWN = 0;

        ///<summary>
        ///Service Link Types <br/>
        ///Service link type describes type of a link from which a service is sourced. <br/>
        /// </summary>

        /// <summary>
        /// Link type is interactive
        /// </summary>
        public const int SERVICE_LINK_INTERACTIVE = 1;

        /// <summary>
        /// Link type is broadcast
        /// </summary>
        public const int SERVICE_LINK_BROADCAST = 2;

        ///<summary>
        ///Service Link Codes <br/>
        ///Service link code provides more info about a link from which a service is sourced. <br/>
        /// </summary>

        /// <summary>
        /// No information is available
        /// </summary>
        public const int SERVICE_LINK_CODE_NONE = 0;

        /// <summary>
        /// Link is ok
        /// </summary>
        public const int SERVICE_LINK_CODE_OK = 1;

        /// <summary>
        /// Link has started to recover
        /// </summary>
        public const int SERVICE_LINK_CODE_RECOVERY_STARTED = 2;

        /// <summary>
        /// Link has completed recovery
        /// </summary>
        public const int SERVICE_LINK_CODE_RECOVERY_COMPLETED = 3;

        ///<summary>
        ///Service Data Filter Data Type <br/>
        ///Service Data Filter data type specifies type of data present in the service data filter. <br/>
        /// </summary>

        /// <summary>
        /// Service Data is a time
        /// </summary>
        public const int SERVICE_DATA_TIME = 1;

        /// <summary>
        /// Service Data is an alert
        /// </summary>
        public const int SERVICE_DATA_ALERT = 2;

        /// <summary>
        /// Service Data is a headline
        /// </summary>
        public const int SERVICE_DATA_HEADLINE = 3;

        /// <summary>
        /// Service Data is a status
        /// </summary>
        public const int SERVICE_DATA_STATUS = 4;

        ///<summary>
        ///Instrument Name Types <br/>
        ///Instrument name types specify type of item name. <br/>
        ///Instrument name types are used on market domain messages (e.g. MarketPrice domain) <br/>
        /// </summary>

        /// <summary>
        /// Symbology is not specified or not applicable
        /// </summary>
        public const int INSTRUMENT_NAME_UNSPECIFIED = 0;

        /// <summary>
        /// Reuters Instrument Code
        /// </summary>
        public const int INSTRUMENT_NAME_RIC = 1;

        /// <summary>
        /// Contribution Instrument Code
        /// </summary>
        public const int INSTRUMENT_NAME_CONTRIBUTOR = 2;

        /// <summary>
        /// Maximum reserved Name Type
        /// </summary>
        public const int INSTRUMENT_NAME_MAX_RESERVED = 127;

        ///<summary>
        ///Instrument Update Type Numbers<br/>
        ///Instrument update type number specifies type of updates.<br/>
        /// </summary>

        /// <summary>
        /// Not specified
        /// </summary>
        public const int INSTRUMENT_UPDATE_UNSPECIFIED = 0;

        /// <summary>
        /// Quote
        /// </summary>
        public const int INSTRUMENT_UPDATE_QUOTE = 1;

        /// <summary>
        /// Trade
        /// </summary>
        public const int INSTRUMENT_UPDATE_TRADE = 2;

        /// <summary>
        /// News Alert
        /// </summary>
        public const int INSTRUMENT_UPDATE_NEWS_ALERT = 3;

        /// <summary>
        /// Volume Alert
        /// </summary>
        public const int INSTRUMENT_UPDATE_VOLUME_ALERT = 4;

        /// <summary>
        /// Order Indication
        /// </summary>
        public const int INSTRUMENT_UPDATE_ORDER_INDICATION = 5;

        /// <summary>
        /// Closing Run
        /// </summary>
        public const int INSTRUMENT_UPDATE_CLOSING_RUN = 6;

        /// <summary>
        /// Correction
        /// </summary>
        public const int INSTRUMENT_UPDATE_CORRECTION = 7;

        /// <summary>
        /// Official information from the exchange
        /// </summary>
        public const int INSTRUMENT_UPDATE_MARKET_DIGEST = 8;

        /// <summary>
        /// One or more conflated quotes followed by a trade
        /// </summary>
        public const int INSTRUMENT_UPDATE_QUOTES_TRADE = 9;

        /// <summary>
        /// Update with other filtering and conflation applied
        /// </summary>
        public const int INSTRUMENT_UPDATE_MULTIPLE = 10;

        /// <summary>
        /// Fields may have changed
        /// </summary>
        public const int INSTRUMENT_UPDATE_VERIFY = 11;

        ///<summary>
        ///View Type Numbers<br/>
        ///View type number specifies how view information is encoded.<br/>
        ///</summary>

        /// <summary>
        /// View data is array of field ids
        /// </summary>
        public const int VT_FIELD_ID_LIST = 1;

        /// <summary>
        /// View data is array of element names
        /// </summary>
        public const int VT_ELEMENT_NAME_LIST = 2;

        ///<summary>
        ///Support Batch Request Type <br/>
        ///Support batch request type describes what batch request is supported. <br/>
        ///A provider specifies what type of batch requesting it supports. <br/>
        /// </summary>

        /// <summary>
        /// Support batch request
        /// </summary>
        public const int SUPPORT_BATCH_REQUEST = 0x001;

        /// <summary>
        /// Support batch reissue
        /// </summary>
        public const int SUPPORT_BATCH_REISSUE = 0x002;

        /// <summary>
        /// Support batch close
        /// </summary>
        public const int SUPPORT_BATCH_CLOSE = 0x004;

        ///<summary>
        ///Support Enhanced SymbolList Type <br/>
        ///Support enhanced symbolList type describes what enhanced symbolList request is supported. <br/>
        ///A provider specifies what type of enhanced symbolList requesting it supports. <br/>
        /// </summary>

        /// <summary>
        /// Supports Symbol List names only.
        /// </summary>
        public const int SUPPORT_SYMBOL_LIST_NAMES_ONLY = 0x000;

        /// <summary>
        /// Supports Symbol List data streams.
        /// </summary>
        public const int SUPPORT_SYMBOL_LIST_DATA_STREAMS = 0x001;

        /// <summary>
        /// Enhanced SymbolList Request Type <br/>
        /// Enhanced symbolList request type describes type of symbolList request. <br/>
        /// A consumer specifies what type of enhanced symbolList request it wants. <br/>
        /// </summary>

        /// <summary>
        /// Requesting for names only and no data.
        /// </summary>
        public const int SYMBOL_LIST_NAMES_ONLY = 0x000;

        /// <summary>
        /// Requesting for streaming behavior of datastreams.
        /// </summary>
        public const int SYMBOL_LIST_DATA_STREAMS = 0x001;

        /// <summary>
        /// Requesting for snapshot behavior of datastreams
        /// </summary>
        public const int SYMBOL_LIST_DATA_SNAPSHOTS = 0x002;

        /// <summary>
        /// ApplicationId element name
        /// </summary>
        public const string ENAME_APP_ID = "ApplicationId";

        /// <summary>
        /// ApplicationName element name
        /// </summary>
        public const string ENAME_APP_NAME = "ApplicationName";

        /// <summary>
        /// ApplicationAuthorizationToken element name
        /// </summary>
        public const string ENAME_APPAUTH_TOKEN = "ApplicationAuthorizationToken";

        /// <summary>
        /// Position element name
        /// </summary>
        public const string ENAME_POSITION = "Position";

        /// <summary>
        /// Password element name
        /// </summary>
        public const string ENAME_PASSWORD = "Password";

        /// <summary>
        /// Pause element name
        /// </summary>
        public const string ENAME_PAUSE = "Pause";

        /// <summary>
        /// ProvidePermissionProfile element name
        /// </summary>
        public const string ENAME_PROV_PERM_PROF = "ProvidePermissionProfile";

        /// <summary>
        /// ProvidePermissionExpressions element name
        /// </summary>
        public const string ENAME_PROV_PERM_EXP = "ProvidePermissionExpressions";

        /// <summary>
        /// AllowSuspectData element name
        /// </summary>
        public const string ENAME_ALLOW_SUSPECT_DATA = "AllowSuspectData";

        /// <summary>
        /// SingleOpen element name
        /// </summary>
        public const string ENAME_SINGLE_OPEN = "SingleOpen";

        /// <summary>
        /// InstanceId element name
        /// </summary>
        public const string ENAME_INST_ID = "InstanceId";

        /// <summary>
        /// Role element name
        /// </summary>
        public const string ENAME_ROLE = "Role";

        /// <summary>
        /// SupportPauseResume element name
        /// </summary>
        public const string ENAME_SUPPORT_PR = "SupportPauseResume";

        /// <summary>
        /// SupportOptimizedPauseResume element name
        /// </summary>
        public const string ENAME_SUPPORT_OPR = "SupportOptimizedPauseResume";

        /// <summary>
        /// SupportOMMPost element name
        /// </summary>
        public const string ENAME_SUPPORT_POST = "SupportOMMPost";

        /// <summary>
        /// SupportBatchRequests element name
        /// </summary>
        public const string ENAME_SUPPORT_BATCH = "SupportBatchRequests";

        /// <summary>
        /// SupportViewRequests element name
        /// </summary>
        public const string ENAME_SUPPORT_VIEW = "SupportViewRequests";

        /// <summary>
        /// SupportEnhancedSymbolList element name
        /// </summary>
        public const string ENAME_SUPPORT_ENH_SYMBOL_LIST = "SupportEnhancedSymbolList";

        /// <summary>
        /// SupportProviderDictionaryDownload element name
        /// </summary>
        public const string ENAME_SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD = "SupportProviderDictionaryDownload";

        /// <summary>
        /// SequenceRetryInterval element name
        /// </summary>
        public const string ENAME_SEQUENCE_RETRY_INTERVAL = "SequenceRetryInterval";

        /// <summary>
        /// UpdateBufferLimit element name
        /// </summary>
        public const string ENAME_UPDATE_BUFFER_LIMIT = "UpdateBufferLimit";

        /// <summary>
        /// SequenceNumberRecovery element name
        /// </summary>
        public const string ENAME_SEQUENCE_NUMBER_RECOVERY = "SequenceNumberRecovery";

        /// <summary>
        /// AuthenticationToken element name
        /// </summary>
        public const string ENAME_AUTHN_TOKEN = "AuthenticationToken";

        /// <summary>
        /// AuthenticationExtended element name
        /// </summary>
        public const string ENAME_AUTHN_EXTENDED = "AuthenticationExtended";

        /// <summary>
        /// AuthenticationExtendedResp element name
        /// </summary>
        public const string ENAME_AUTHN_EXTENDED_RESP = "AuthenticationExtendedResp";

        /// <summary>
        /// AuthenticationTTReissue element name
        /// </summary>
        public const string ENAME_AUTHN_TT_REISSUE = "AuthenticationTTReissue";

        /// <summary>
        /// AuthenticationErrorCode element name
        /// </summary>
        public const string ENAME_AUTHN_ERRORCODE = "AuthenticationErrorCode";

        /// <summary>
        /// AuthenticationErrorText element name
        /// </summary>
        public const string ENAME_AUTHN_ERRORTEXT = "AuthenticationErrorText";

        /// <summary>
        /// UserName element name
        /// </summary>
        public const string ENAME_USERNAME = "UserName";

        /// <summary>
        /// UserNameType element name
        /// </summary>
        public const string ENAME_USERNAME_TYPE = "UserNameType";

        /// <summary>
        /// SequenceNumber element name
        /// </summary>
        public const string ENAME_SEQ_NUM = "SequenceNumber";

        /// <summary>
        /// State element name
        /// </summary>
        public const string ENAME_STATE = "State";

        /// <summary>
        /// Solicited element name
        /// </summary>
        public const string ENAME_SOLICITED = "Solicited";

        ///<summary>
        ///Well known source directory names.<br/>
        /// </summary>

        /// <summary>
        /// Name element name
        /// </summary>
        public const string ENAME_NAME = "Name";

        /// <summary>
        /// ServiceID element name
        /// </summary>
        public const string ENAME_SERVICE_ID = "ServiceID";

        /// <summary>
        /// Vendor element name
        /// </summary>
        public const string ENAME_VENDOR = "Vendor";

        /// <summary>
        /// IsSource element name
        /// </summary>
        public const string ENAME_IS_SOURCE = "IsSource";

        /// <summary>
        /// Capabilities element name
        /// </summary>
        public const string ENAME_CAPABILITIES = "Capabilities";

        /// <summary>
        /// DictionariesProvided element name
        /// </summary>
        public const string ENAME_DICTIONARYS_PROVIDED = "DictionariesProvided";

        /// <summary>
        /// DictionariesUsed element name
        /// </summary>
        public const string ENAME_DICTIONARYS_USED = "DictionariesUsed";

        /// <summary>
        /// QoS element name
        /// </summary>
        public const string ENAME_QOS = "QoS";

        /// <summary>
        /// SupportsQoSRange element name
        /// </summary>
        public const string ENAME_SUPPS_QOS_RANGE = "SupportsQoSRange";

        /// <summary>
        /// ItemList element name
        /// </summary>
        public const string ENAME_ITEM_LIST = "ItemList";

        /// <summary>
        /// SupportsOutOfBandSnapshots element name
        /// </summary>
        public const string ENAME_SUPPS_OOB_SNAPSHOTS = "SupportsOutOfBandSnapshots";

        /// <summary>
        /// AcceptingConsumerStatus element name
        /// </summary>
        public const string ENAME_ACCEPTING_CONS_STATUS = "AcceptingConsumerStatus";

        /// <summary>
        /// SourceMirroringMode element name
        /// </summary>
        public const string ENAME_CONS_SOURCE_MIROR_MODE = "SourceMirroringMode";

        /// <summary>
        /// ConsumerStatus element name
        /// </summary>
        public const string ENAME_CONS_STATUS = "ConsumerStatus";

        /// <summary>
        /// ServiceState element name
        /// </summary>
        public const string ENAME_SVC_STATE = "ServiceState";

        /// <summary>
        /// AcceptingRequests element name
        /// </summary>
        public const string ENAME_ACCEPTING_REQS = "AcceptingRequests";

        /// <summary>
        /// Status element name
        /// </summary>
        public const string ENAME_STATUS = "Status";

        /// <summary>
        /// Group element name
        /// </summary>
        public const string ENAME_GROUP = "Group";

        /// <summary>
        /// MergedToGroup element name
        /// </summary>
        public const string ENAME_MERG_TO_GRP = "MergedToGroup";

        /// <summary>
        /// OpenLimit element name
        /// </summary>
        public const string ENAME_OPEN_LIMIT = "OpenLimit";

        /// <summary>
        /// OpenWindow element name
        /// </summary>
        public const string ENAME_OPEN_WINDOW = "OpenWindow";

        /// <summary>
        /// LoadFactor element name
        /// </summary>
        public const string ENAME_LOAD_FACT = "LoadFactor";

        /// <summary>
        /// Type element name
        /// </summary>
        public const string ENAME_TYPE = "Type";

        /// <summary>
        /// Data element name
        /// </summary>
        public const string ENAME_DATA = "Data";

        /// <summary>
        /// LinkState element name
        /// </summary>
        public const string ENAME_LINK_STATE = "LinkState";

        /// <summary>
        /// LinkCode element name
        /// </summary>
        public const string ENAME_LINK_CODE = "LinkCode";

        /// <summary>
        /// Ticks element name
        /// </summary>
        public const string ENAME_TICKS = "Ticks";

        /// <summary>
        /// RoundTripLatency element name
        /// </summary>
        public const string ENAME_LATENCY = "RoundTripLatency";

        /// <summary>
        /// TcpRetrans element name
        /// </summary>
        public const string ENAME_TCP_RETRANS = "TcpRetrans";

        ///<summary>
        ///Well known server configuration names.<br/>
        ///</summary>

        /// <summary>
        /// SupportStandby element name
        /// </summary>
        public const string ENAME_SUPPORT_STANDBY = "SupportStandby";

        /// <summary>
        /// WarmStandbyInfo element name
        /// </summary>
        public const string ENAME_WARMSTANDBY_INFO = "WarmStandbyInfo";

        /// <summary>
        /// WarmStandbyMode element name
        /// </summary>
        public const string ENAME_WARMSTANDBY_MODE = "WarmStandbyMode";

        /// <summary>
        /// ConsumerConnectionStatus element name
        /// </summary>
        public const string ENAME_CONS_CONN_STATUS = "ConsumerConnectionStatus";

        /// <summary>
        /// DownloadConnectionConfig element name
        /// </summary>
        public const string ENAME_DOWNLOAD_CON_CONFIG = "DownloadConnectionConfig";

        /// <summary>
        /// ConnectionConfig element name
        /// </summary>
        public const string ENAME_CONNECTION_CONFIG = "ConnectionConfig";

        /// <summary>
        /// NumStandbyServers element name
        /// </summary>
        public const string ENAME_NUM_STANDBY_SERVERS = "NumStandbyServers";

        /// <summary>
        /// Hostname element name
        /// </summary>
        public const string ENAME_HOSTNAME = "Hostname";

        /// <summary>
        /// Port element name
        /// </summary>
        public const string ENAME_PORT = "Port";

        /// <summary>
        /// ServerType element name
        /// </summary>
        public const string ENAME_SERVER_TYPE = "ServerType";

        /// <summary>
        /// SystemID element name
        /// </summary>
        public const string ENAME_SYSTEM_ID = "SystemID";

        ///<summary>
        ///Well known dictionary names.<br/>
        ///</summary>

        /// <summary>
        /// DictionaryId element name
        /// </summary>
        public const string ENAME_DICTIONARY_ID = "DictionaryId";

        /// <summary>
        /// Type element name
        /// </summary>
        public const string ENAME_DICT_TYPE = "Type";

        /// <summary>
        /// Version element name
        /// </summary>
        public const string ENAME_DICT_VERSION = "Version";

        /// <summary>
        /// NAME element name
        /// </summary>
        public const string ENAME_FIELD_NAME = "NAME";

        /// <summary>
        /// FID element name
        /// </summary>
        public const string ENAME_FIELD_ID = "FID";

        /// <summary>
        /// RIPPLETO element name
        /// </summary>
        public const string ENAME_FIELD_RIPPLETO = "RIPPLETO";

        /// <summary>
        /// TYPE element name
        /// </summary>
        public const string ENAME_FIELD_TYPE = "TYPE";

        /// <summary>
        /// LENGTH element name
        /// </summary>
        public const string ENAME_FIELD_LENGTH = "LENGTH";

        /// <summary>
        /// RWFTYPE element name
        /// </summary>
        public const string ENAME_FIELD_RWFTYPE = "RWFTYPE";

        /// <summary>
        /// RWFLEN element name
        /// </summary>
        public const string ENAME_FIELD_RWFLENGTH = "RWFLEN";

        /// <summary>
        /// ENUMLENGTH element name
        /// </summary>
        public const string ENAME_FIELD_ENUMLENGTH = "ENUMLENGTH";

        /// <summary>
        /// LONGNAME element name
        /// </summary>
        public const string ENAME_FIELD_LONGNAME = "LONGNAME";

        /// <summary>
        /// RT_Version element name
        /// </summary>
        public const string ENAME_ENUM_RT_VERSION = "RT_Version";

        /// <summary>
        /// DT_Version element name
        /// </summary>
        public const string ENAME_ENUM_DT_VERSION = "DT_Version";

        /// <summary>
        /// FIDS element name
        /// </summary>
        public const string ENAME_ENUM_FIDS = "FIDS";

        /// <summary>
        /// FID element name
        /// </summary>
        public const string ENAME_ENUM_FID = "FID";

        /// <summary>
        /// VALUES element name
        /// </summary>
        public const string ENAME_ENUM_VALUES = "VALUES";

        /// <summary>
        /// VALUE element name
        /// </summary>
        public const string ENAME_ENUM_VALUE = "VALUE";

        /// <summary>
        /// DISPLAYS element name
        /// </summary>
        public const string ENAME_ENUM_DISPLAYS = "DISPLAYS";

        /// <summary>
        /// DISPLAY element name
        /// </summary>
        public const string ENAME_ENUM_DISPLAY = "DISPLAY";

        /// <summary>
        /// MEANINGS element name
        /// </summary>
        public const string ENAME_ENUM_MEANINGS = "MEANINGS";

        /// <summary>
        /// MEANING element name
        /// </summary>
        public const string ENAME_ENUM_MEANING = "MEANING";

        /// <summary>
        /// Text element name
        /// </summary>
        public const string ENAME_TEXT = "Text";

        /// <summary>
        /// Version element name
        /// </summary>
        public const string ENAME_VERSION = "Version";

        ///<summary>
        ///Well known batch and view names.<br/>
        ///</summary>

        /// <summary>
        /// :ItemList element name
        /// </summary>
        public const string ENAME_BATCH_ITEM_LIST = ":ItemList";

        /// <summary>
        /// :ViewType element name
        /// </summary>
        public const string ENAME_VIEW_TYPE = ":ViewType";

        /// <summary>
        /// :ViewData element name
        /// </summary>
        public const string ENAME_VIEW_DATA = ":ViewData";

        /// <summary>
        /// :SymbolListBehaviors
        /// </summary>
        public const string ENAME_SYMBOL_LIST_BEHAVIORS = ":SymbolListBehaviors";

        /// <summary>
        /// :DataStreams element name
        /// </summary>
        public const string ENAME_DATA_STREAMS = ":DataStreams";
    }
}
