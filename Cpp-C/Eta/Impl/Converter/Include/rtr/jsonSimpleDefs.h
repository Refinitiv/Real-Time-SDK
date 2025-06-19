/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtr_jsonSimpleDefs
#define __rtr_jsonSimpleDefs

static const RsslBuffer JSON_ID = { 2, (char*)"ID"};
static const RsslBuffer JSON_TYPE = { 4, (char*)"Type"};
static const RsslBuffer JSON_DOMAIN = { 6, (char*)"Domain"};
static const RsslBuffer JSON_EXTHDR = { 6, (char*)"ExtHdr"};

static const RsslBuffer JSON_DATA = { 4, (char*)"Data"};
static const RsslBuffer JSON_KEY = { 3, (char*)"Key"};
static const RsslBuffer JSON_REQKEY = { 6, (char*)"ReqKey"};
static const RsslBuffer JSON_KEY_NAME = { 4, (char*)"Name"};
static const RsslBuffer JSON_KEY_NAME_TYPE = { 8, (char*)"NameType"};
static const RsslBuffer JSON_KEY_FILTER = { 6, (char*)"Filter"};
static const RsslBuffer JSON_KEY_IDENTIFIER = { 10, (char*)"Identifier"};
static const RsslBuffer JSON_KEY_SERVICE = { 7, (char*)"Service"};
static const RsslBuffer JSON_ELEMENTS = { 8, (char*)"Elements"};
static const RsslBuffer JSON_FILTERLIST = { 10, (char*)"FilterList"};
static const RsslBuffer JSON_OPAQUE = { 6, (char*)"Opaque"};
static const RsslBuffer JSON_XML = { 3, (char*)"Xml"};
static const RsslBuffer JSON_NAME = { 4, (char*)"Name"};

static const RsslBuffer JSON_VIEW = { 4, (char*)"View"};
static const RsslBuffer JSON_VIEW_NAME_LIST = { 8, (char*)"NameList"};
static const RsslBuffer JSON_VIEW_FID_LIST = { 7, (char*)"FidList"};
static const RsslBuffer JSON_PRIVATE = { 7, (char*)"Private"};
static const RsslBuffer JSON_VIEWTYPE = { 8, (char*)"ViewType"};
static const RsslBuffer JSON_STREAMING = { 9, (char*)"Streaming"};
static const RsslBuffer JSON_PAUSE = { 5, (char*)"Pause"};
static const RsslBuffer JSON_REFRESH = { 7, (char*)"Refresh"};
static const RsslBuffer JSON_KEYINUPDATES = { 12, (char*)"KeyInUpdates"};
static const RsslBuffer JSON_CONFINFOINUPDATES = { 17, (char*)"ConfInfoInUpdates"};
static const RsslBuffer JSON_QUALIFIED = { 9, (char*)"Qualified"};
static const RsslBuffer JSON_COMPLETE = { 8, (char*)"Complete"};
static const RsslBuffer JSON_DONOTCACHE = { 10, (char*)"DoNotCache"};
static const RsslBuffer JSON_POSTUSERINFO = { 12, (char*)"PostUserInfo"};
static const RsslBuffer JSON_ADDRESS = { 7, (char*)"Address"};
static const RsslBuffer JSON_USERID = { 6, (char*)"UserID"};
static const RsslBuffer JSON_PARTNUMBER = { 10, (char*)"PartNumber"};
static const RsslBuffer JSON_CLEARCACHE = { 10, (char*)"ClearCache"};
static const RsslBuffer JSON_UPDATETYPE = { 10, (char*)"UpdateType"};
static const RsslBuffer JSON_DONOTCONFLATE = { 13, (char*)"DoNotConflate"};
static const RsslBuffer JSON_DONOTRIPPLE = { 11, (char*)"DoNotRipple"};
static const RsslBuffer JSON_DISCARDABLE = { 11, (char*)"Discardable"};
static const RsslBuffer JSON_PERMDATA = { 8, (char*)"PermData"};
static const RsslBuffer JSON_SEQNUM   = { 9, (char*)"SeqNumber"};
static const RsslBuffer JSON_SECSEQNUM   = { 12, (char*)"SecSeqNumber"};
static const RsslBuffer JSON_CONFINFO = { 14, (char*)"ConflationInfo"};

static const RsslBuffer JSON_QOS = { 3, (char*)"Qos"};
static const RsslBuffer JSON_WORSTQOS = { 8, (char*)"WorstQos"};
static const RsslBuffer JSON_TIMELINESS = { 10, (char*)"Timeliness"};
static const RsslBuffer JSON_RATE = { 4, (char*)"Rate"};
static const RsslBuffer JSON_DYNAMIC = { 7, (char*)"Dynamic"};
static const RsslBuffer JSON_TIMEINFO = { 8, (char*)"TimeInfo"};
static const RsslBuffer JSON_RATEINFO = { 8, (char*)"RateInfo"};

static const RsslBuffer JSON_PRIORITY = { 8, (char*)"Priority"};
static const RsslBuffer JSON_CLASS = { 5, (char*)"Class"};
static const RsslBuffer JSON_COUNT = { 5, (char*)"Count"};
static const RsslBuffer JSON_TIME = { 4, (char*)"Time"};

static const RsslBuffer JSON_LENGTH = { 6, (char*)"Length"};
static const RsslBuffer JSON_FIELDS = { 6, (char*)"Fields"};

static const RsslBuffer JSON_STATE = { 5, (char*)"State"};
static const RsslBuffer JSON_STREAM = { 6, (char*)"Stream"};
static const RsslBuffer JSON_CODE = { 4, (char*)"Code"};
static const RsslBuffer JSON_TEXT = { 4, (char*)"Text"};

static const RsslBuffer JSON_ENTRIES = { 7, (char*)"Entries"};
static const RsslBuffer JSON_COUNTHINT = { 9, (char*)"CountHint"};
static const RsslBuffer JSON_ACTION = { 6, (char*)"Action"};
static const RsslBuffer JSON_SET = { 3, (char*)"Set"};
static const RsslBuffer JSON_CLEAR = { 5, (char*)"Clear"};

static const RsslBuffer JSON_MAP = { 3, (char*)"Map"};
static const RsslBuffer JSON_SUMMARY = { 7, (char*)"Summary"};
static const RsslBuffer JSON_HASPERMDATA = { 11, (char*)"HasPermData"};
static const RsslBuffer JSON_KEYTYPE = { 7, (char*)"KeyType"};
static const RsslBuffer JSON_KEYFIELDID = { 10, (char*)"KeyFieldID"};

static const RsslBuffer JSON_SERIES = { 6, (char*)"Series"};
static const RsslBuffer JSON_VECTOR = { 6, (char*)"Vector"};

static const RsslBuffer JSON_DATATYPE = { 8, (char*)"DataType"};
static const RsslBuffer JSON_ACK = { 3, (char*)"Ack"};
static const RsslBuffer JSON_POSTUSERRIGHTS = { 14, (char*)"PostUserRights"};
static const RsslBuffer JSON_NONE = { 4, (char*)"None"};
static const RsslBuffer JSON_CREATE = { 6, (char*)"Create"};
static const RsslBuffer JSON_MODIFYPERM = { 10, (char*)"ModifyPerm"};
static const RsslBuffer JSON_SOLICITED = { 9, (char*)"Solicited"};
static const RsslBuffer JSON_POSTID = { 6, (char*)"PostID"};

static const RsslBuffer JSON_ACKID = { 5, (char*)"AckID"};
static const RsslBuffer JSON_NAKCODE = { 7, (char*)"NakCode"};

static const RsslBuffer JSON_MESSAGE = { 7, (char*)"Message"};

static const RsslBuffer JSON_INDEX = { 5, (char*)"Index"};
static const RsslBuffer JSON_SUPPORTSORTING = { 14, (char*)"SupportSorting"};
static const RsslBuffer JSON_PING = { 4, (char*)"Ping"};
static const RsslBuffer JSON_PONG = { 4, (char*)"Pong"};
static const RsslBuffer JSON_ERROR = { 5, (char*)"Error"};

static const RsslBuffer JSON_STRING = { 6, (char*)"String"};
static const RsslBuffer JSON_PRIMITIVE = { 9, (char*)"Primitive"};
static const RsslBuffer JSON_DEBUG = { 5, (char*)"Debug"};
static const RsslBuffer JSON_FILE = { 4, (char*)"File"};
static const RsslBuffer JSON_LINE = { 4, (char*)"Line"};
static const RsslBuffer JSON_OFFSET = { 6, (char*)"Offset"};

static const RsslBuffer RDM_OMMSTR_LOGIN_USER_AUTHENTICATION_TOKEN = {10,(char*)"AuthnToken"};
#endif
