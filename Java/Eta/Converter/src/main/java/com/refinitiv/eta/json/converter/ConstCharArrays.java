package com.refinitiv.eta.json.converter;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.rdm.UpdateEventTypes;

import static com.refinitiv.eta.codec.QosRates.*;
import static com.refinitiv.eta.codec.QosTimeliness.*;

class ConstCharArrays {

    public static final String blankStringConst = new String(new byte[] { 0x0 });
    public static final Buffer emptyBuffer = CodecFactory.createBuffer();
    static {
        emptyBuffer.data(blankStringConst);
    }

    public static final char[] nullBytes = {'n', 'u', 'l', 'l'};
    public static final String nullString = "null";
    public static final String trueString = "true";
    public static final String falseString = "false";

    public static final char[] digits = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    public static final char[] minInt = {'-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
    public static final char[] minLong = {'-', '9', '2', '2', '3', '3', '7', '2', '0', '3', '6', '8', '5', '4', '7', '7', '5', '8', '0', '8'};

    public static final String jsonDoublePositiveInfinityStr = "Inf";
    public static final String jsonDoubleNegativeInfinityStr = "-Inf";
    public static final String jsonDoubleNanStr = "NaN";

    public static final char[] inf = {'I', 'n', 'f'};
    public static final char[] infNeg = {'-', 'I', 'n', 'f'};
    public static final char[] nan = {'N', 'a', 'N'};

    public static final char[] safeQuote = {'\\', '\"'};
    public static final char[] safeBackslash = {'\\', '\\'};

    //JSON fields
    public static final String JSON_ID = "ID";
    public static final String JSON_TYPE = "Type";
    public static final String JSON_DOMAIN = "Domain";
    public static final String JSON_EXTHDR = "ExtHdr"; // {'E', 'x', 't', 'H', 'd', 'r'};

    public static final String JSON_DATA = "Data";
    public static final String JSON_KEY = "Key";
    public static final String JSON_REQKEY = "ReqKey"; //{'R', 'e', 'q', 'K', 'e', 'y'};
    public static final String JSON_KEY_NAME = "Name"; //{'N', 'a', 'm', 'e'};
    public static final String JSON_KEY_NAME_TYPE = "NameType"; //{'N', 'a', 'm', 'e', 'T', 'y', 'p', 'e'};
    public static final String JSON_KEY_FILTER = "Filter"; //{'F', 'i', 'l', 't', 'e', 'r'};
    public static final String JSON_KEY_IDENTIFIER = "Identifier"; //{'I', 'd', 'e', 'n', 't', 'i', 'f', 'i', 'e', 'r'};
    public static final String JSON_KEY_SERVICE = "Service"; //{'S', 'e', 'r', 'v', 'i', 'c', 'e'};
    public static final String JSON_ELEMENTS = "Elements";
    public static final String JSON_ELEMENTLIST = "ElementList";
    public static final String JSON_FILTERLIST = "FilterList";
    public static final String JSON_OPAQUE = "Opaque";
    public static final String JSON_XML = "Xml";
    public static final String JSON_JSON = "Json";
    public static final String JSON_NAME = "Name"; //{'N', 'a', 'm', 'e'};

    public static final String JSON_VIEW = "View"; // {'V', 'i', 'e', 'w'};
    public static final String JSON_VIEW_NAME_LIST = "NameList"; //{'N', 'a', 'm', 'e', 'L', 'i', 's', 't'};
    public static final String JSON_VIEW_FID_LIST = "FidList"; //{'F', 'i', 'd', 'L', 'i', 's', 't'};
    public static final String JSON_PRIVATE = "Private";
    public static final String JSON_VIEWTYPE = "ViewType"; //{'V', 'i', 'e', 'w', 'T', 'y', 'p', 'e'};
    public static final String JSON_STREAMING = "Streaming";
    public static final String JSON_PAUSE = "Pause";
    public static final String JSON_REFRESH = "Refresh";
    public static final String JSON_KEYINUPDATES = "KeyInUpdates";
    public static final String JSON_CONFINFOINUPDATES = "ConfInfoInUpdates"; //{'C', 'o', 'n', 'f', 'I', 'n', 'f', 'o', 'I', 'n', 'U', 'p', 'd', 'a', 't', 'e', 's'};
    public static final String JSON_QUALIFIED = "Qualified"; //{'Q', 'u', 'a', 'l', 'i', 'f', 'i', 'e', 'd'};
    public static final String JSON_COMPLETE = "Complete"; // {'C', 'o', 'm', 'p', 'l', 'e', 't', 'e'};
    public static final String JSON_DONOTCACHE = "DoNotCache"; // {'D', 'o', 'N', 'o', 't', 'C', 'a', 'c', 'h', 'e'};
    public static final String JSON_POSTUSERINFO = "PostUserInfo";//{'P', 'o', 's', 't', 'U', 's', 'e', 'r', 'I', 'n', 'f', 'o'};
    public static final String JSON_ADDRESS = "Address";
    public static final String JSON_USERID = "UserID";// {'U', 's', 'e', 'r', 'I', 'D'};
    public static final String JSON_PARTNUMBER = "PartNumber"; //{'P', 'a', 'r', 't', 'N', 'u', 'm', 'b', 'e', 'r'};
    public static final String JSON_CLEARCACHE = "ClearCache"; //{'C', 'l', 'e', 'a', 'r', 'C', 'a', 'c', 'h', 'e'};
    public static final String JSON_UPDATETYPE = "UpdateType"; //{'U', 'p', 'd', 'a', 't', 'e', 'T', 'y', 'p', 'e'};
    public static final String JSON_DONOTCONFLATE = "DoNotConflate"; //'D', 'o', 'N', 'o', 't', 'C', 'o', 'n', 'f', 'l', 'a', 't', 'e'};
    public static final String JSON_DONOTRIPPLE = "DoNotRipple"; // {'D', 'o', 'N', 'o', 't', 'R', 'i', 'p', 'p', 'l', 'e'};
    public static final String JSON_DISCARDABLE = "Discardable";//{'D', 'i', 's', 'c', 'a', 'r', 'd', 'a', 'b', 'l', 'e'};
    public static final String JSON_PERMDATA = "PermData";
    public static final String JSON_SEQNUM = "SeqNumber";
    public static final String JSON_SECSEQNUM = "SecSeqNumber";//{'S', 'e', 'c', 'S', 'e', 'q', 'N', 'u', 'm', 'b', 'e', 'r'};
    public static final String JSON_CONFINFO = "ConflationInfo";
    public static final String JSON_PROVIDER_DRIVEN = "ProviderDriven";

    public static final String JSON_QOS = "Qos";
    public static final String JSON_WORSTQOS = "WorstQos";
    public static final String JSON_TIMELINESS = "Timeliness";
    public static final String JSON_RATE = "Rate";
    public static final String JSON_DYNAMIC = "Dynamic";
    public static final String JSON_TIMEINFO = "TimeInfo";
    public static final String JSON_RATEINFO = "RateInfo";

    public static final String JSON_PRIORITY = "Priority";
    public static final String JSON_CLASS = "Class";
    public static final String JSON_COUNT = "Count";
    public static final String JSON_TIME = "Time";

    public static final String JSON_LENGTH = "Length";
    public static final String JSON_FIELDS = "Fields";
    public static final String JSON_FIELDLIST = "FieldList";

    public static final String JSON_STATE = "State";
    public static final String JSON_STREAM = "Stream";
    public static final String JSON_CODE = "Code";
    public static final String JSON_TEXT = "Text";
    public static final String JSON_ENTRIES = "Entries";
    public static final String JSON_COUNTHINT = "CountHint";
    public static final String JSON_ACTION = "Action";
    public static final String JSON_SET = "Set";
    public static final String JSON_CLEAR = "Clear";

    public static final String JSON_MAP = "Map";
    public static final String JSON_SUMMARY = "Summary";
    public static final String JSON_HASPERMDATA = "HasPermData"; //{'H', 'a', 's', 'P', 'e', 'r', 'm', 'D', 'a', 't', 'a'};
    public static final String JSON_KEYTYPE = "KeyType";
    public static final String JSON_KEYFIELDID = "KeyFieldID";
    public static final String JSON_SERIES = "Series";
    public static final String JSON_VECTOR = "Vector";

    public static final String JSON_DATATYPE = "DataType";//{'D', 'a', 't', 'a', 'T', 'y', 'p', 'e'};
    public static final String JSON_ACK = "Ack";
    public static final String JSON_POSTUSERRIGHTS = "PostUserRights"; //{'P', 'o', 's', 't', 'U', 's', 'e', 'r', 'R', 'i', 'g', 'h', 't', 's'};
    public static final String JSON_NONE = "None";
    public static final String JSON_CREATE = "Create";
    public static final String JSON_MODIFYPERM = "ModifyPerm";//{'M', 'o', 'd', 'i', 'f', 'y', 'P', 'e', 'r', 'm'};
    public static final String JSON_SOLICITED = "Solicited";
    public static final String JSON_POSTID = "PostID";

    public static final String JSON_ACKID = "AckID";
    public static final String JSON_NAKCODE = "NakCode";

    public static final String JSON_MESSAGE = "Message";

    public static final String JSON_INDEX = "Index";
    public static final String JSON_SUPPORTSORTING = "SupportSorting";
    public static final String PING = "Ping"; //{'P', 'i', 'n', 'g'};
    public static final String PONG = "Pong"; //{'P', 'o', 'n', 'g'};
    public static final String JSON_ERROR = "Error";

    public static final String JSON_STRING = "String"; //{'S', 't', 'r', 'i', 'n', 'g'};
    public static final String JSON_PRIMITIVE = "Primitive"; //{'P', 'r', 'i', 'm', 'i', 't', 'i', 'v', 'e'};
    public static final String JSON_DEBUG = "Debug"; //{'D', 'e', 'b', 'u', 'g'};
    public static final String JSON_FILE = "File";
    public static final String JSON_LINE = "Line";
    public static final String JSON_OFFSET = "Offset";

    public static final String JSON_RDM_OMMSTR_LOGIN_USER_AUTHENTICATION_TOKEN = "AuthnToken"; // {'A', 'u', 't', 'h', 'n', 'T', 'o', 'k', 'e', 'n'};

    public static final String STATE_CODE_STR_NONE = "None";
    public static final String STATE_CODE_STR_NOT_FOUND = "NotFound";
    public static final String STATE_CODE_STR_TIMEOUT = "Timeout";
    public static final String STATE_CODE_STR_NOT_ENTITLED = "NotEntitled";
    public static final String STATE_CODE_STR_INVALID_ARGUMENT = "InvalidArgument";
    public static final String STATE_CODE_STR_USAGE_ERROR = "UsageError";
    public static final String STATE_CODE_STR_PREEMPTED = "Preempted";
    public static final String STATE_CODE_STR_JIT_CONFLATION_STARTED = "JitConflationStarted";
    public static final String STATE_CODE_STR_REALTIME_RESUMED = "RealtimeResumed";
    public static final String STATE_CODE_STR_FAILOVER_STARTED = "FailoverStarted";
    public static final String STATE_CODE_STR_FAILOVER_COMPLETED = "FailoverCompleted";
    public static final String STATE_CODE_STR_GAP_DETECTED = "GapDetected";
    public static final String STATE_CODE_STR_NO_RESOURCES = "NoResources";
    public static final String STATE_CODE_STR_TOO_MANY_ITEMS = "TooManyItems";
    public static final String STATE_CODE_STR_ALREADY_OPEN = "AlreadyOpen";
    public static final String STATE_CODE_STR_SOURCE_UNKNOWN = "SourceUnknown";
    public static final String STATE_CODE_STR_NOT_OPEN = "NotOpen";

    public static final String STATE_CODE_STR_NON_UPDATING_ITEM = "NonUpdatingItem";
    public static final String STATE_CODE_STR_UNSUPPORTED_VIEW_TYPE = "UnsupportedViewType";
    public static final String STATE_CODE_STR_INVALID_VIEW = "InvalidView";
    public static final String STATE_CODE_STR_FULL_VIEW_PROVIDED = "FullViewProvided";
    public static final String STATE_CODE_STR_UNABLE_TO_REQUEST_AS_BATCH = "UnableToRequestAsBatch";

    public static final String STATE_CODE_STR_NO_BATCH_VIEW_SUPPORT_IN_REQ = "NoBatchViewSupportInReq";
    public static final String STATE_CODE_STR_EXCEEDED_MAX_MOUNTS_PER_USER = "ExceededMaxMountsPerUser";
    public static final String STATE_CODE_STR_ERROR = "Error";
    public static final String STATE_CODE_STR_DACS_DOWN = "DacsDown";
    public static final String STATE_CODE_STR_USER_UNKNOWN_TO_PERM_SYS = "UserUnknownToPermSys";
    public static final String STATE_CODE_STR_DACS_MAX_LOGINS_REACHED = "MaxLoginsReached";
    public static final String STATE_CODE_STR_DACS_USER_ACCESS_TO_APP_DENIED = "UserAccessToAppDenied";
    public static final String STATE_CODE_STR_GAP_FILL = "GapFill";
    public static final String STATE_CODE_STR_APP_AUTHORIZATION_FAILED = "AppAuthorizationFailed";
    public static final String[] stateCodesString = {
            STATE_CODE_STR_NONE,
            STATE_CODE_STR_NOT_FOUND,
            STATE_CODE_STR_TIMEOUT,
            STATE_CODE_STR_NOT_ENTITLED,
            STATE_CODE_STR_INVALID_ARGUMENT,
            STATE_CODE_STR_USAGE_ERROR,
            STATE_CODE_STR_PREEMPTED,
            STATE_CODE_STR_JIT_CONFLATION_STARTED,
            STATE_CODE_STR_REALTIME_RESUMED,
            STATE_CODE_STR_FAILOVER_STARTED,
            STATE_CODE_STR_FAILOVER_COMPLETED,
            STATE_CODE_STR_GAP_DETECTED,
            STATE_CODE_STR_NO_RESOURCES,
            STATE_CODE_STR_TOO_MANY_ITEMS,
            STATE_CODE_STR_ALREADY_OPEN,
            STATE_CODE_STR_SOURCE_UNKNOWN,
            STATE_CODE_STR_NOT_OPEN,
            "",
            "",
            STATE_CODE_STR_NON_UPDATING_ITEM,
            STATE_CODE_STR_UNSUPPORTED_VIEW_TYPE,
            STATE_CODE_STR_INVALID_VIEW,
            STATE_CODE_STR_FULL_VIEW_PROVIDED,
            STATE_CODE_STR_UNABLE_TO_REQUEST_AS_BATCH,
            "",
            "",
            STATE_CODE_STR_NO_BATCH_VIEW_SUPPORT_IN_REQ,
            STATE_CODE_STR_EXCEEDED_MAX_MOUNTS_PER_USER,
            STATE_CODE_STR_ERROR,
            STATE_CODE_STR_DACS_DOWN,
            STATE_CODE_STR_USER_UNKNOWN_TO_PERM_SYS,
            STATE_CODE_STR_DACS_MAX_LOGINS_REACHED,
            STATE_CODE_STR_DACS_USER_ACCESS_TO_APP_DENIED,
            STATE_CODE_STR_GAP_FILL,
            STATE_CODE_STR_APP_AUTHORIZATION_FAILED
    };

    //timeliness -> t9s
    public static final String QOS_T9S_STR_UNSPECIFIED = "Unspecified";
    public static final String QOS_T9S_STR_REALTIME = "Realtime";
    public static final String QOS_T9S_STR_DELAYED_BY_UNKNOWN = "DelayedUnknown";
    public static final String QOS_T9S_STR_DELAYED_BY_TIME_INFO = "Delayed";

    public static final String[] qosTimelinessStrings = {QOS_T9S_STR_UNSPECIFIED, QOS_T9S_STR_REALTIME, QOS_T9S_STR_DELAYED_BY_UNKNOWN, QOS_T9S_STR_DELAYED_BY_TIME_INFO};
    public static class JsonQosTimeliness {

        public static int ofValue(String timelinessStr, JsonConverterError error) {
            switch (timelinessStr) {
                case QOS_T9S_STR_UNSPECIFIED:
                    return com.refinitiv.eta.codec.QosTimeliness.UNSPECIFIED;
                case QOS_T9S_STR_REALTIME:
                    return REALTIME;
                case QOS_T9S_STR_DELAYED_BY_UNKNOWN:
                    return DELAYED_UNKNOWN;
                case QOS_T9S_STR_DELAYED_BY_TIME_INFO:
                    return DELAYED;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "QosTimeliness ='" + timelinessStr +"'");
                    return AbstractTypeConverter.DEFAULT_INT;
            }
        }
    }

    public static final String QOS_RATE_STR_UNSPECIFIED             = "Unspecified";
    public static final String QOS_RATE_STR_TICK_BY_TICK            = "TickByTick";
    public static final String QOS_RATE_STR_JUST_IN_TIME_CONFLATED  = "JitConflated";
    public static final String QOS_RATE_STR_CONFLATED_BY_RATE_INFO  = "TimeConflated";

    public static final String[] qosRateStrings = {QOS_RATE_STR_UNSPECIFIED, QOS_RATE_STR_TICK_BY_TICK, QOS_RATE_STR_JUST_IN_TIME_CONFLATED, QOS_RATE_STR_CONFLATED_BY_RATE_INFO};

    public static class JsonQosRate {
        public static int ofValue(String rateStr, JsonConverterError error) {
            switch (rateStr) {
                case QOS_RATE_STR_UNSPECIFIED:
                    return com.refinitiv.eta.codec.QosRates.UNSPECIFIED;
                case QOS_RATE_STR_TICK_BY_TICK:
                    return TICK_BY_TICK;
                case QOS_RATE_STR_JUST_IN_TIME_CONFLATED:
                    return JIT_CONFLATED;
                case QOS_RATE_STR_CONFLATED_BY_RATE_INFO:
                    return TIME_CONFLATED;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "QosRate ='" + rateStr +"'");
                    return AbstractTypeConverter.DEFAULT_INT;
            }
        }
    }

    public static final String NAME_TYPE_STR_UNSPECIFIED        = "Unspecified";
    public static final String NAME_TYPE_STR_RIC                = "Ric";
    public static final String NAME_TYPE_STR_CONTRIBUTOR        = "Contributor";
    public static final String LOGIN_USER_NAME                  = "Name";
    public static final String LOGIN_USER_EMAIL_ADDRESS         = "EmailAddress";
    public static final String LOGIN_USER_TOKEN                 = "Token";
    public static final String LOGIN_USER_COOKIE                = "Cookie";
    public static final String LOGIN_USER_AUTHENTICATION_TOKEN  = "AuthnToken";

    public static class JsonNameType {
        public static int ofValue(String nameTypeStr, JsonConverterError error) {
            switch (nameTypeStr) {
                case NAME_TYPE_STR_UNSPECIFIED:
                    return InstrumentNameTypes.UNSPECIFIED;
                case NAME_TYPE_STR_RIC:
                    return InstrumentNameTypes.RIC;
                case NAME_TYPE_STR_CONTRIBUTOR:
                    return InstrumentNameTypes.CONTRIBUTOR;
                case LOGIN_USER_NAME:
                    return Login.UserIdTypes.NAME;
                case LOGIN_USER_EMAIL_ADDRESS:
                    return Login.UserIdTypes.EMAIL_ADDRESS;
                case LOGIN_USER_TOKEN:
                    return Login.UserIdTypes.TOKEN;
                case LOGIN_USER_COOKIE:
                    return Login.UserIdTypes.COOKIE;
                case LOGIN_USER_AUTHENTICATION_TOKEN:
                    return Login.UserIdTypes.AUTHN_TOKEN;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "NameType ='" + nameTypeStr +"'");
                    //to avoid errors on NameType range defaulting to Unspecified
                    return InstrumentNameTypes.UNSPECIFIED;
            }
        }
    }

    public static final String DOMAIN_STR_LOGIN = "Login";
    public static final String DOMAIN_STR_SOURCE = "Source";
    public static final String DOMAIN_STR_DICTIONARY = "Dictionary";
    public static final String DOMAIN_STR_MARKET_PRICE = "MarketPrice";
    public static final String DOMAIN_STR_MARKET_BY_ORDER = "MarketByOrder";
    public static final String DOMAIN_STR_MARKET_BY_PRICE = "MarketByPrice";
    public static final String DOMAIN_STR_MARKET_MAKER = "MarketMaker";
    public static final String DOMAIN_STR_SYMBOL_LIST = "SymbolList";
    public static final String DOMAIN_STR_SERVICE_PROVIDER_STATUS = "ServiceProviderStatus";
    public static final String DOMAIN_STR_HISTORY = "History";
    public static final String DOMAIN_STR_HEADLINE = "Headline";
    public static final String DOMAIN_STR_STORY = "Story";
    public static final String DOMAIN_STR_REPLAYHEADLINE = "ReplayHeadline";
    public static final String DOMAIN_STR_REPLAYSTORY = "ReplayStory";
    public static final String DOMAIN_STR_TRANSACTION = "Transaction";
    public static final String DOMAIN_STR_YIELD_CURVE = "YieldCurve";
    public static final String DOMAIN_STR_CONTRIBUTION = "Contribution";
    public static final String DOMAIN_STR_PROVIDER_ADMIN = "ProviderAdmin";
    public static final String DOMAIN_STR_ANALYTICS = "Analytics";
    public static final String DOMAIN_STR_REFERENCE = "Reference";
    public static final String DOMAIN_STR_NEWS_TEXT_ANALYTICS = "NewsTextAnalytics";
    public static final String DOMAIN_STR_ECONOMIC_INDICATOR = "EconomicIndicator";
    public static final String DOMAIN_STR_POLL = "Poll";
    public static final String DOMAIN_STR_FORECAST = "Forecast";
    public static final String DOMAIN_STR_MARKET_BY_TIME = "MarketByTime";
    public static final String DOMAIN_STR_SYSTEM = "System";


    public static class JsonDomain {
        public static int ofValue(String domainStr, JsonConverterError error) {
            switch (domainStr) {
                case DOMAIN_STR_LOGIN:
                    return DomainTypes.LOGIN;
                case DOMAIN_STR_SOURCE:
                    return DomainTypes.SOURCE;
                case DOMAIN_STR_DICTIONARY:
                    return DomainTypes.DICTIONARY;
                case DOMAIN_STR_MARKET_PRICE:
                    return DomainTypes.MARKET_PRICE;
                case DOMAIN_STR_MARKET_BY_ORDER:
                    return DomainTypes.MARKET_BY_ORDER;
                case DOMAIN_STR_MARKET_BY_PRICE:
                    return DomainTypes.MARKET_BY_PRICE;
                case DOMAIN_STR_MARKET_MAKER:
                    return DomainTypes.MARKET_MAKER;
                case DOMAIN_STR_SYMBOL_LIST:
                    return DomainTypes.SYMBOL_LIST;
                case DOMAIN_STR_SERVICE_PROVIDER_STATUS:
                    return DomainTypes.SERVICE_PROVIDER_STATUS;
                case DOMAIN_STR_HISTORY:
                    return DomainTypes.HISTORY;
                case DOMAIN_STR_HEADLINE:
                    return DomainTypes.HEADLINE;
                case DOMAIN_STR_STORY:
                    return DomainTypes.STORY;
                case DOMAIN_STR_REPLAYHEADLINE:
                    return DomainTypes.REPLAYHEADLINE;
                case DOMAIN_STR_REPLAYSTORY:
                    return DomainTypes.REPLAYSTORY;
                case DOMAIN_STR_TRANSACTION:
                    return DomainTypes.TRANSACTION;
                case DOMAIN_STR_YIELD_CURVE:
                    return DomainTypes.YIELD_CURVE;
                case DOMAIN_STR_CONTRIBUTION:
                    return DomainTypes.CONTRIBUTION;
                case DOMAIN_STR_PROVIDER_ADMIN:
                    return DomainTypes.PROVIDER_ADMIN;
                case DOMAIN_STR_ANALYTICS:
                    return DomainTypes.ANALYTICS;
                case DOMAIN_STR_REFERENCE:
                    return DomainTypes.REFERENCE;
                case DOMAIN_STR_NEWS_TEXT_ANALYTICS:
                    return DomainTypes.NEWS_TEXT_ANALYTICS;
                case DOMAIN_STR_ECONOMIC_INDICATOR:
                    return DomainTypes.ECONOMIC_INDICATOR;
                case DOMAIN_STR_POLL:
                    return DomainTypes.POLL;
                case DOMAIN_STR_FORECAST:
                    return DomainTypes.FORECAST;
                case DOMAIN_STR_MARKET_BY_TIME:
                    return DomainTypes.MARKET_BY_TIME;
                case DOMAIN_STR_SYSTEM:
                    return DomainTypes.SYSTEM;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Domain '" + domainStr + "'");
                    //to avoid errors on domain range defaulting to Login
                    return DomainTypes.LOGIN;
            }
        }
    }

    public static class JsonStateCode {
        public static int ofValue(String codeStr, JsonConverterError error) {
            switch (codeStr) {
                case STATE_CODE_STR_ALREADY_OPEN:
                    return StateCodes.ALREADY_OPEN;
                case STATE_CODE_STR_APP_AUTHORIZATION_FAILED:
                    return StateCodes.APP_AUTHORIZATION_FAILED;
                case STATE_CODE_STR_DACS_DOWN:
                    return StateCodes.DACS_DOWN;
                case STATE_CODE_STR_ERROR:
                    return StateCodes.ERROR;
                case STATE_CODE_STR_EXCEEDED_MAX_MOUNTS_PER_USER:
                    return StateCodes.EXCEEDED_MAX_MOUNTS_PER_USER;
                case STATE_CODE_STR_FAILOVER_COMPLETED:
                    return StateCodes.FAILOVER_COMPLETED;
                case STATE_CODE_STR_FAILOVER_STARTED:
                    return StateCodes.FAILOVER_STARTED;
                case STATE_CODE_STR_FULL_VIEW_PROVIDED:
                    return StateCodes.FULL_VIEW_PROVIDED;
                case STATE_CODE_STR_GAP_DETECTED:
                    return StateCodes.GAP_DETECTED;
                case STATE_CODE_STR_GAP_FILL:
                    return StateCodes.GAP_FILL;
                case STATE_CODE_STR_INVALID_ARGUMENT:
                    return StateCodes.INVALID_ARGUMENT;
                case STATE_CODE_STR_INVALID_VIEW:
                    return StateCodes.INVALID_VIEW;
                case STATE_CODE_STR_JIT_CONFLATION_STARTED:
                    return StateCodes.JIT_CONFLATION_STARTED;
                case STATE_CODE_STR_DACS_MAX_LOGINS_REACHED:
                    return StateCodes.DACS_MAX_LOGINS_REACHED;
                case STATE_CODE_STR_NO_BATCH_VIEW_SUPPORT_IN_REQ:
                    return StateCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ;
                case STATE_CODE_STR_NONE:
                    return StateCodes.NONE;
                case STATE_CODE_STR_NON_UPDATING_ITEM:
                    return StateCodes.NON_UPDATING_ITEM;
                case STATE_CODE_STR_NO_RESOURCES:
                    return StateCodes.NO_RESOURCES;
                case STATE_CODE_STR_NOT_ENTITLED:
                    return StateCodes.NOT_ENTITLED;
                case STATE_CODE_STR_NOT_FOUND:
                    return StateCodes.NOT_FOUND;
                case STATE_CODE_STR_NOT_OPEN:
                    return StateCodes.NOT_OPEN;
                case STATE_CODE_STR_PREEMPTED:
                    return StateCodes.PREEMPTED;
                case STATE_CODE_STR_REALTIME_RESUMED:
                    return StateCodes.REALTIME_RESUMED;
                case STATE_CODE_STR_SOURCE_UNKNOWN:
                    return StateCodes.SOURCE_UNKNOWN;
                case STATE_CODE_STR_TIMEOUT:
                    return StateCodes.TIMEOUT;
                case STATE_CODE_STR_TOO_MANY_ITEMS:
                    return StateCodes.TOO_MANY_ITEMS;
                case STATE_CODE_STR_UNABLE_TO_REQUEST_AS_BATCH:
                    return StateCodes.UNABLE_TO_REQUEST_AS_BATCH;
                case STATE_CODE_STR_UNSUPPORTED_VIEW_TYPE:
                    return StateCodes.UNSUPPORTED_VIEW_TYPE;
                case STATE_CODE_STR_USAGE_ERROR:
                    return StateCodes.USAGE_ERROR;
                case STATE_CODE_STR_DACS_USER_ACCESS_TO_APP_DENIED:
                    return StateCodes.DACS_USER_ACCESS_TO_APP_DENIED;
                case STATE_CODE_STR_USER_UNKNOWN_TO_PERM_SYS:
                    return StateCodes.USER_UNKNOWN_TO_PERM_SYS;

                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "State code '" + codeStr + "'");
                    //to avoid errors on domain range defaulting to NONE
                    return StateCodes.NONE;
            }
        }
    }

    public static class JsonDataState {
        public static int ofValue(String codeStr, JsonConverterError error) {
            switch (codeStr) {
                case DATA_STATE_STR_OK:
                    return DataStates.OK;
                case DATA_STATE_STR_SUSPECT:
                    return DataStates.SUSPECT;
                case DATA_STATE_STR_NO_CHANGE:
                    return DataStates.NO_CHANGE;

                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "State code '" + codeStr + "'");
                    //to avoid errors on domain range defaulting to NO_CHANGE
                    return DataStates.NO_CHANGE;
            }
        }
    }

    public static class JsonStreamState {
        public static int ofValue(String codeStr, JsonConverterError error) {
            switch (codeStr) {
                case STREAM_STATE_STR_UNSPECIFIED:
                    return StreamStates.UNSPECIFIED;
                case STREAM_STATE_STR_OPEN:
                    return StreamStates.OPEN;
                case STREAM_STATE_STR_NON_STREAMING:
                    return StreamStates.NON_STREAMING;
                case STREAM_STATE_STR_CLOSED_RECOVER:
                    return StreamStates.CLOSED_RECOVER;
                case STREAM_STATE_STR_CLOSED:
                    return StreamStates.CLOSED;
                case STREAM_STATE_STR_REDIRECTED:
                    return StreamStates.REDIRECTED;

                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "State code '" + codeStr + "'");
                    //to avoid errors on domain range defaulting to UNSPECIFIED
                    return StreamStates.UNSPECIFIED;
            }
        }
    }

    public static final String NAKC_STR_NONE                = "None";
    public static final String NAKC_STR_ACCESS_DENIED       = "AccessDenied";
    public static final String NAKC_STR_DENIED_BY_SRC       = "DeniedBySrc";
    public static final String NAKC_STR_SOURCE_DOWN         = "SourceDown";
    public static final String NAKC_STR_SOURCE_UNKNOWN      = "SourceUnknown";
    public static final String NAKC_STR_NO_RESOURCES        = "NoResources";
    public static final String NAKC_STR_NO_RESPONSE         = "NoResponse";
    public static final String NAKC_STR_GATEWAY_DOWN        = "GatewayDown";
    public static final String NAKC_STR_SYMBOL_UNKNOWN      = "SymbolUnknown";
    public static final String NAKC_STR_NOT_OPEN            = "NotOpen";
    public static final String NAKC_STR_INVALID_CONTENT     = "InvalidContent";

    public static class JsonNackCode {
        public static int ofValue(String nackCodeStr, JsonConverterError error) {
            switch (nackCodeStr) {
                case NAKC_STR_NONE:
                    return NakCodes.NONE;
                case NAKC_STR_ACCESS_DENIED:
                    return NakCodes.ACCESS_DENIED;
                case NAKC_STR_DENIED_BY_SRC:
                    return NakCodes.DENIED_BY_SRC;
                case NAKC_STR_SOURCE_DOWN:
                    return NakCodes.SOURCE_DOWN;
                case NAKC_STR_SOURCE_UNKNOWN:
                    return NakCodes.SOURCE_UNKNOWN;
                case NAKC_STR_NO_RESOURCES:
                    return NakCodes.NO_RESOURCES;
                case NAKC_STR_NO_RESPONSE:
                    return NakCodes.NO_RESPONSE;
                case NAKC_STR_GATEWAY_DOWN:
                    return NakCodes.GATEWAY_DOWN;
                case NAKC_STR_SYMBOL_UNKNOWN:
                    return NakCodes.SYMBOL_UNKNOWN;
                case NAKC_STR_NOT_OPEN:
                    return NakCodes.NOT_OPEN;
                case NAKC_STR_INVALID_CONTENT:
                    return NakCodes.INVALID_CONTENT;
                default:
                    return NakCodes.NONE;
            }
        }
    }

    public static final String STREAM_STATE_STR_UNSPECIFIED         = "Unspecified";
    public static final String STREAM_STATE_STR_OPEN                = "Open";
    public static final String STREAM_STATE_STR_NON_STREAMING       = "NonStreaming";
    public static final String STREAM_STATE_STR_CLOSED_RECOVER      = "ClosedRecover";
    public static final String STREAM_STATE_STR_CLOSED              = "Closed";
    public static final String STREAM_STATE_STR_REDIRECTED          = "Redirected";
    public static final String[] streamStateStrings = {STREAM_STATE_STR_UNSPECIFIED, STREAM_STATE_STR_OPEN, STREAM_STATE_STR_NON_STREAMING, STREAM_STATE_STR_CLOSED_RECOVER, STREAM_STATE_STR_CLOSED, STREAM_STATE_STR_REDIRECTED};

    public static final String UPD_EVENT_TYPE_UNSPECIFIED      = "Unspecified";
    public static final String UPD_EVENT_TYPE_QUOTE            = "Quote";
    public static final String UPD_EVENT_TYPE_TRADE            = "Trade";
    public static final String UPD_EVENT_TYPE_NEWS_ALERT       = "NewsAlert";
    public static final String UPD_EVENT_TYPE_VOLUME_ALERT     = "VolumeAlert";
    public static final String UPD_EVENT_TYPE_ORDER_INDICATION = "OrderIndication";
    public static final String UPD_EVENT_TYPE_CLOSING_RUN      = "ClosingRun";
    public static final String UPD_EVENT_TYPE_CORRECTION       = "Correction";
    public static final String UPD_EVENT_TYPE_MARKET_DIGEST    = "MarketDigest";
    public static final String UPD_EVENT_TYPE_QUOTES_TRADE     = "QuotesTrade";
    public static final String UPD_EVENT_TYPE_MULTIPLE         = "Multiple";
    public static final String UPD_EVENT_TYPE_VERIFY           = "Verify";

    public static final String NAKC_NONE            = "None";
    public static final String NAKC_ACCESS_DENIED   = "AccessDenied";
    public static final String NAKC_DENIED_BY_SRC   = "DeniedBySrc";
    public static final String NAKC_SOURCE_DOWN     = "SourceDown";
    public static final String NAKC_SOURCE_UNKNOWN  = "SourceUnknown";
    public static final String NAKC_NO_RESOURCES    = "NoResources";
    public static final String NAKC_NO_RESPONSE     = "NoResponse";
    public static final String NAKC_GATEWAY_DOWN    = "GatewayDown";
    public static final String NAKC_SYMBOL_UNKNOWN  = "SymbolUnknown";
    public static final String NAKC_NOT_OPEN        = "NotOpen";
    public static final String NAKC_INVALID_CONTENT = "InvalidContent";

    public static final String DATA_STATE_STR_NO_CHANGE     = "NoChange";
    public static final String DATA_STATE_STR_OK            = "Ok";
    public static final String DATA_STATE_STR_SUSPECT       = "Suspect";
    public static final String[] dataStateStrings = {DATA_STATE_STR_NO_CHANGE, DATA_STATE_STR_OK, DATA_STATE_STR_SUSPECT};

    public static final String DATA_TYPE_STR_UNKNOWN            = "Unknown";
    public static final String DATA_TYPE_STR_INT                = "Int";
    public static final String DATA_TYPE_STR_UINT               = "UInt";
    public static final String DATA_TYPE_STR_FLOAT              = "Float";
    public static final String DATA_TYPE_STR_DOUBLE             = "Double";
    public static final String DATA_TYPE_STR_REAL               = "Real";
    public static final String DATA_TYPE_STR_DATE               = "Date";
    public static final String DATA_TYPE_STR_TIME               = "Time";
    public static final String DATA_TYPE_STR_DATE_TIME          = "DateTime";
    public static final String DATA_TYPE_STR_QOS                = "Qos";
    public static final String DATA_TYPE_STR_STATE              = "State";
    public static final String DATA_TYPE_STR_ENUM               = "Enum";
    public static final String DATA_TYPE_STR_ARRAY              = "Array";
    public static final String DATA_TYPE_STR_BUFFER             = "Buffer";
    public static final String DATA_TYPE_STR_ASCII_STRING       = "AsciiString";
    public static final String DATA_TYPE_STR_UTF_8_STRING       = "Utf8String";
    public static final String DATA_TYPE_STR_RMTES_STRING       = "RmtesString";
    public static final String DATA_TYPE_STR_INT_1              = "Int1";
    public static final String DATA_TYPE_STR_UINT_1             = "UInt1";
    public static final String DATA_TYPE_STR_INT_2              = "Int2";
    public static final String DATA_TYPE_STR_UINT_2             = "UInt2";
    public static final String DATA_TYPE_STR_INT_4              = "Int4";
    public static final String DATA_TYPE_STR_UINT_4             = "UInt4";
    public static final String DATA_TYPE_STR_INT_8              = "Int8";
    public static final String DATA_TYPE_STR_UINT_8             = "UInt8";
    public static final String DATA_TYPE_STR_FLOAT_4            = "Float4";
    public static final String DATA_TYPE_STR_DOUBLE_8           = "Double8";
    public static final String DATA_TYPE_STR_REAL_4_RB          = "Real4RB";
    public static final String DATA_TYPE_STR_REAL_8_RB          = "Real8RB";
    public static final String DATA_TYPE_STR_DATE_4             = "Date4";
    public static final String DATA_TYPE_STR_TIME_3             = "Time3";
    public static final String DATA_TYPE_STR_TIME_5             = "Time5";
    public static final String DATA_TYPE_STR_DATE_TIME_7        = "DateTime7";
    public static final String DATA_TYPE_STR_DATE_TIME_9        = "DateTime9";
    public static final String DATA_TYPE_STR_DATE_TIME_11       = "DateTime11";
    public static final String DATA_TYPE_STR_DATE_TIME_12       = "DateTime12";
    public static final String DATA_TYPE_STR_TIME_7             = "Time7";
    public static final String DATA_TYPE_STR_TIME_8             = "Time8";
    public static final String DATA_TYPE_STR_NO_DATA            = "NoData";
    public static final String DATA_TYPE_STR_OPAQUE             = "Opaque";
    public static final String DATA_TYPE_STR_XML                = "Xml";
    public static final String DATA_TYPE_STR_FIELD_LIST         = "FieldList";
    public static final String DATA_TYPE_STR_ELEMENT_LIST       = "ElementList";
    public static final String DATA_TYPE_STR_ANSI_PAGE          = "AnsiPage";
    public static final String DATA_TYPE_STR_FILTER_LIST        = "FilterList";
    public static final String DATA_TYPE_STR_VECTOR             = "Vector";
    public static final String DATA_TYPE_STR_MAP                = "Map";
    public static final String DATA_TYPE_STR_SERIES             = "Series";
    public static final String DATA_TYPE_STR_MSG                = "Msg";
    public static final String DATA_TYPE_STR_JSON               = "Json";
    public static final int INT_1_SHIFT = 44;
    public static final int OPAQUE_SHIFT = 87;
    public static final int NO_DATA_POSITION = 41;
    public static final String[] dataTypeStrings = {
            DATA_TYPE_STR_UNKNOWN, // i = 0, DataTypes = 0
            "",
            "",
            DATA_TYPE_STR_INT,
            DATA_TYPE_STR_UINT,
            DATA_TYPE_STR_FLOAT,
            DATA_TYPE_STR_DOUBLE,
            "",
            DATA_TYPE_STR_REAL,
            DATA_TYPE_STR_DATE,
            DATA_TYPE_STR_TIME,
            DATA_TYPE_STR_DATE_TIME,
            DATA_TYPE_STR_QOS,
            DATA_TYPE_STR_STATE,
            DATA_TYPE_STR_ENUM,
            DATA_TYPE_STR_ARRAY,
            DATA_TYPE_STR_BUFFER,
            DATA_TYPE_STR_ASCII_STRING,
            DATA_TYPE_STR_UTF_8_STRING,
            DATA_TYPE_STR_RMTES_STRING, //i = 19, DataTypes = 19
            DATA_TYPE_STR_INT_1, //i = 20, DataTypes = 64
            DATA_TYPE_STR_UINT_1,
            DATA_TYPE_STR_INT_2,
            DATA_TYPE_STR_UINT_2,
            DATA_TYPE_STR_INT_4,
            DATA_TYPE_STR_UINT_4,
            DATA_TYPE_STR_INT_8,
            DATA_TYPE_STR_UINT_8,
            DATA_TYPE_STR_FLOAT_4,
            DATA_TYPE_STR_DOUBLE_8,
            DATA_TYPE_STR_REAL_4_RB,
            DATA_TYPE_STR_REAL_8_RB,
            DATA_TYPE_STR_DATE_4,
            DATA_TYPE_STR_TIME_3,
            DATA_TYPE_STR_TIME_5,
            DATA_TYPE_STR_DATE_TIME_7,
            DATA_TYPE_STR_DATE_TIME_9,
            DATA_TYPE_STR_DATE_TIME_11,
            DATA_TYPE_STR_DATE_TIME_12,
            DATA_TYPE_STR_TIME_7,
            DATA_TYPE_STR_TIME_8, //i = 40, DataTypes = 84
            DATA_TYPE_STR_NO_DATA, //i = 41, DataTypes = 128
            "",
            DATA_TYPE_STR_OPAQUE, //i = 43, DataTypes = 130
            DATA_TYPE_STR_XML,
            DATA_TYPE_STR_FIELD_LIST,
            DATA_TYPE_STR_ELEMENT_LIST,
            DATA_TYPE_STR_ANSI_PAGE,
            DATA_TYPE_STR_FILTER_LIST,
            DATA_TYPE_STR_VECTOR,
            DATA_TYPE_STR_MAP,
            DATA_TYPE_STR_SERIES,
            "",
            "",
            DATA_TYPE_STR_MSG,
            DATA_TYPE_STR_JSON //i = 53, DataTypes = 142
    };

    public static final String UPD_EVENT_TYPE_STR_UNSPECIFIED       = "Unspecified";
    public static final String UPD_EVENT_TYPE_STR_QUOTE             = "Quote";
    public static final String UPD_EVENT_TYPE_STR_TRADE             = "Trade";
    public static final String UPD_EVENT_TYPE_STR_NEWS_ALERT        = "NewsAlert";
    public static final String UPD_EVENT_TYPE_STR_VOLUME_ALERT      = "VolumeAlert";
    public static final String UPD_EVENT_TYPE_STR_ORDER_INDICATION  = "OrderIndication";
    public static final String UPD_EVENT_TYPE_STR_CLOSING_RUN       = "ClosingRun";
    public static final String UPD_EVENT_TYPE_STR_CORRECTION        = "Correction";
    public static final String UPD_EVENT_TYPE_STR_MARKET_DIGEST     = "MarketDigest";
    public static final String UPD_EVENT_TYPE_STR_QUOTES_TRADE      = "QuotesTrade";
    public static final String UPD_EVENT_TYPE_STR_MULTIPLE          = "Multiple";
    public static final String UPD_EVENT_TYPE_STR_VERIFY            = "Verify";

    public static class JsonUpdateType {
        public static int ofValue(String updateTypeStr, JsonConverterError error) {
            switch (updateTypeStr) {
                case UPD_EVENT_TYPE_STR_UNSPECIFIED:
                    return UpdateEventTypes.UNSPECIFIED;
                case UPD_EVENT_TYPE_STR_QUOTE:
                    return UpdateEventTypes.QUOTE;
                case UPD_EVENT_TYPE_STR_TRADE:
                    return UpdateEventTypes.TRADE;
                case UPD_EVENT_TYPE_STR_NEWS_ALERT:
                    return UpdateEventTypes.NEWS_ALERT;
                case UPD_EVENT_TYPE_STR_VOLUME_ALERT:
                    return UpdateEventTypes.VOLUME_ALERT;
                case UPD_EVENT_TYPE_STR_ORDER_INDICATION:
                    return UpdateEventTypes.ORDER_INDICATION;
                case UPD_EVENT_TYPE_STR_CLOSING_RUN:
                    return UpdateEventTypes.CLOSING_RUN;
                case UPD_EVENT_TYPE_STR_CORRECTION:
                    return UpdateEventTypes.CORRECTION;
                case UPD_EVENT_TYPE_STR_MARKET_DIGEST:
                    return UpdateEventTypes.MARKET_DIGEST;
                case UPD_EVENT_TYPE_STR_QUOTES_TRADE:
                    return UpdateEventTypes.QUOTES_TRADE;
                case UPD_EVENT_TYPE_STR_MULTIPLE:
                    return UpdateEventTypes.MULTIPLE;
                case UPD_EVENT_TYPE_STR_VERIFY:
                    return UpdateEventTypes.VERIFY;
                default:
                    error.setError(JsonConverterErrorCodes.JSON_ERROR_UNEXPECTED_VALUE, "Update type '" + updateTypeStr + "'");
                    return UpdateEventTypes.UNSPECIFIED;
            }
        }
    }

    public static final String BATCH_REQUEST_NAME_STR   = ":ItemList";
    public static final String VIEW_TYPE_STR            = ":ViewType";
    public static final String VIEW_NAME_STR            = ":ViewData";

    public static final String ACTION_STR_UPDATE    = "Update";
    public static final String ACTION_STR_ADD       = "Add";
    public static final String ACTION_STR_CLEAR     = "Clear";
    public static final String ACTION_STR_INSERT    = "Insert";
    public static final String ACTION_STR_DELETE    = "Delete";
    public static final String ACTION_STR_SET       = "Set";

    public static final String[] entryActionStrings = {
            ACTION_STR_UPDATE,
            ACTION_STR_SET,
            ACTION_STR_CLEAR,
            ACTION_STR_INSERT,
            ACTION_STR_DELETE
    };

    public static final String[] mapEntryActionStrings = {
            ACTION_STR_UPDATE,
            ACTION_STR_ADD,
            ACTION_STR_DELETE
    };

    public static final String[] containerTypes = {
            DATA_TYPE_STR_OPAQUE,
            DATA_TYPE_STR_XML,
            JSON_FIELDS,
            JSON_ELEMENTS,
            DATA_TYPE_STR_ANSI_PAGE,
            DATA_TYPE_STR_FILTER_LIST,
            DATA_TYPE_STR_VECTOR,
            DATA_TYPE_STR_MAP,
            DATA_TYPE_STR_SERIES,
            DATA_TYPE_STR_MSG,
            DATA_TYPE_STR_JSON
    };

    public static final String MC_REQUEST   = "Request";
    public static final String MC_REFRESH   = "Refresh";
    public static final String MC_STATUS    = "Status";
    public static final String MC_UPDATE    = "Update";
    public static final String MC_CLOSE     = "Close";
    public static final String MC_ACK       = "Ack";
    public static final String MC_GENERIC   = "Generic";
    public static final String MC_POST      = "Post";

    public static final String MESSAGE_START = "{\"ID\":";
}
