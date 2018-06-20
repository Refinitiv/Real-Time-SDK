/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>

#include "rtr/rsslMsg.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslRDM.h"

#include "rtr/rsslMessagePackage.h"
#include "rtr/decoderTools.h"
#include "rtr/encoderTools.h"
#include "rtr/retmacros.h"
#include "rtr/rsslhashfuncs.h"
#include "../rsslVersion.h"


#define __RSZRSSLMSG    sizeof(RsslMsg)

#if defined (_WIN32) || defined(WIN32)
#define snprintf _snprintf
#else
#include <strings.h>
#endif


RSSL_API void rsslQueryMessagesLibraryVersion(RsslLibraryVersionInfo *pVerInfo)
{
	RSSL_ASSERT(pVerInfo, Invalid parameters or parameters passed in as NULL);

	pVerInfo->productDate = rsslDeltaDate;
	pVerInfo->internalVersion = rsslVersion;
	pVerInfo->productVersion = rsslPackage;
}


RSSL_API const char* rsslMsgClassToString(RsslUInt8 msgClass)
{
	switch (msgClass)
	{
	case RSSL_MC_UPDATE:
		return "RSSL_MC_UPDATE";
	case RSSL_MC_REFRESH:
		return "RSSL_MC_REFRESH";
	case RSSL_MC_REQUEST:
		return "RSSL_MC_REQUEST";
	case RSSL_MC_STATUS:
		return "RSSL_MC_STATUS";
	case RSSL_MC_CLOSE:
		return "RSSL_MC_CLOSE";
	case RSSL_MC_ACK:
		return "RSSL_MC_ACK";
	case RSSL_MC_GENERIC:
		return "RSSL_MC_GENERIC";
	case RSSL_MC_POST:
		return "RSSL_MC_POST";
	}
	return 0;
}

RSSL_API const char* rsslMsgClassToOmmString(RsslUInt8 msgClass)
{
	switch (msgClass)
	{
	case RSSL_MC_UPDATE:
		return RSSL_OMMSTR_MC_UPDATE.data;
	case RSSL_MC_REFRESH:
		return RSSL_OMMSTR_MC_REFRESH.data;
	case RSSL_MC_REQUEST:
		return RSSL_OMMSTR_MC_REQUEST.data;
	case RSSL_MC_STATUS:
		return RSSL_OMMSTR_MC_STATUS.data;
	case RSSL_MC_CLOSE:
		return RSSL_OMMSTR_MC_CLOSE.data;
	case RSSL_MC_ACK:
		return RSSL_OMMSTR_MC_ACK.data;
	case RSSL_MC_GENERIC:
		return RSSL_OMMSTR_MC_GENERIC.data;
	case RSSL_MC_POST:
		return RSSL_OMMSTR_MC_POST.data;
	}
	return 0;
}


RSSL_API const char* rsslDomainTypeToString(RsslUInt8 domainType)
{
	switch(domainType)
	{
		case RSSL_DMT_LOGIN:
			return "RSSL_DMT_LOGIN";
		case RSSL_DMT_SOURCE:
			return "RSSL_DMT_SOURCE";
		case RSSL_DMT_DICTIONARY:
			return "RSSL_DMT_DICTIONARY";
		case RSSL_DMT_MARKET_PRICE:
			return "RSSL_DMT_MARKET_PRICE";
		case RSSL_DMT_MARKET_BY_ORDER:
			return "RSSL_DMT_MARKET_BY_ORDER";
		case RSSL_DMT_MARKET_BY_PRICE:
			return "RSSL_DMT_MARKET_BY_PRICE";
		case RSSL_DMT_MARKET_MAKER:	
			return "RSSL_DMT_MARKET_MAKER";
		case RSSL_DMT_SYMBOL_LIST:
			return "RSSL_DMT_SYMBOL_LIST";
		case RSSL_DMT_SERVICE_PROVIDER_STATUS:
			return "RSSL_DMT_SERVICE_PROVIDER_STATUS";
		case RSSL_DMT_HISTORY:
			return "RSSL_DMT_HISTORY";
		case RSSL_DMT_HEADLINE:
			return "RSSL_DMT_HEADLINE";
		case RSSL_DMT_STORY:
			return "RSSL_DMT_STORY";
		case RSSL_DMT_REPLAYHEADLINE:
			return "RSSL_DMT_REPLAYHEADLINE";
		case RSSL_DMT_REPLAYSTORY:
			return "RSSL_DMT_REPLAYSTORY";
		case RSSL_DMT_TRANSACTION:
			return "RSSL_DMT_TRANSACTION";
		case RSSL_DMT_YIELD_CURVE:
			return "RSSL_DMT_YIELD_CURVE";
		case RSSL_DMT_CONTRIBUTION:
			return "RSSL_DMT_CONTRIBUTION";
		case RSSL_DMT_PROVIDER_ADMIN:
			return "RSSL_DMT_PROVIDER_ADMIN";
		case RSSL_DMT_ANALYTICS:
			return "RSSL_DMT_ANALYTICS";
		case RSSL_DMT_REFERENCE:
			return "RSSL_DMT_REFERENCE";
		case RSSL_DMT_NEWS_TEXT_ANALYTICS:
			return "RSSL_DMT_NEWS_TEXT_ANALYTICS";
		case RSSL_DMT_ECONOMIC_INDICATOR:
			return "RSSL_DMT_ECONOMIC_INDICATOR";
		case RSSL_DMT_POLL:
			return "RSSL_DMT_POLL";
		case RSSL_DMT_FORECAST:
			return "RSSL_DMT_FORECAST";
		case RSSL_DMT_MARKET_BY_TIME:
			return "RSSL_DMT_MARKET_BY_TIME";
		case RSSL_DMT_SYSTEM:
			return "RSSL_DMT_SYSTEM";
		default:
			return "Unknown Domain Type";
	}
	return 0;

}

RSSL_API const char* rsslDomainTypeToOmmString(RsslUInt8 domainType)
{
	switch(domainType)
	{

		case RSSL_DMT_LOGIN:
			return RSSL_OMMSTR_DMT_LOGIN.data;
		case RSSL_DMT_SOURCE:
			return RSSL_OMMSTR_DMT_SOURCE.data;
		case RSSL_DMT_DICTIONARY:
			return RSSL_OMMSTR_DMT_DICTIONARY.data;
		case RSSL_DMT_MARKET_PRICE:
			return RSSL_OMMSTR_DMT_MARKET_PRICE.data;
		case RSSL_DMT_MARKET_BY_ORDER:
			return RSSL_OMMSTR_DMT_MARKET_BY_ORDER.data;
		case RSSL_DMT_MARKET_BY_PRICE:
			return RSSL_OMMSTR_DMT_MARKET_BY_PRICE.data;
		case RSSL_DMT_MARKET_MAKER:	
			return RSSL_OMMSTR_DMT_MARKET_MAKER.data;
		case RSSL_DMT_SYMBOL_LIST:
			return RSSL_OMMSTR_DMT_SYMBOL_LIST.data;
		case RSSL_DMT_SERVICE_PROVIDER_STATUS:
			return RSSL_OMMSTR_DMT_SERVICE_PROVIDER_STATUS.data;
		case RSSL_DMT_HISTORY:
			return RSSL_OMMSTR_DMT_HISTORY.data;
		case RSSL_DMT_HEADLINE:
			return RSSL_OMMSTR_DMT_HEADLINE.data;
		case RSSL_DMT_STORY:
			return RSSL_OMMSTR_DMT_STORY.data;
		case RSSL_DMT_REPLAYHEADLINE:
			return RSSL_OMMSTR_DMT_REPLAYHEADLINE.data;
		case RSSL_DMT_REPLAYSTORY:
			return RSSL_OMMSTR_DMT_REPLAYSTORY.data;
		case RSSL_DMT_TRANSACTION:
			return RSSL_OMMSTR_DMT_TRANSACTION.data;
		case RSSL_DMT_YIELD_CURVE:
			return RSSL_OMMSTR_DMT_YIELD_CURVE.data;
		case RSSL_DMT_CONTRIBUTION:
			return RSSL_OMMSTR_DMT_CONTRIBUTION.data;
		case RSSL_DMT_PROVIDER_ADMIN:
			return RSSL_OMMSTR_DMT_PROVIDER_ADMIN.data;
		case RSSL_DMT_ANALYTICS:
			return RSSL_OMMSTR_DMT_ANALYTICS.data;
		case RSSL_DMT_REFERENCE:
			return RSSL_OMMSTR_DMT_REFERENCE.data;
		case RSSL_DMT_NEWS_TEXT_ANALYTICS:
			return RSSL_OMMSTR_DMT_NEWS_TEXT_ANALYTICS.data;
		case RSSL_DMT_ECONOMIC_INDICATOR:
			return RSSL_OMMSTR_DMT_ECONOMIC_INDICATOR.data;
		case RSSL_DMT_POLL:
			return RSSL_OMMSTR_DMT_POLL.data;
		case RSSL_DMT_FORECAST:
			return RSSL_OMMSTR_DMT_FORECAST.data;
		case RSSL_DMT_MARKET_BY_TIME:
			return RSSL_OMMSTR_DMT_MARKET_BY_TIME.data;
		case RSSL_DMT_SYSTEM:
			return RSSL_OMMSTR_DMT_SYSTEM.data;
		default:
			return NULL;
	}
}

RSSL_API const char* rsslRDMInstrumentNameTypeToOmmString(RsslUInt8 type)
{
	switch(type)
	{
		case RDM_INSTRUMENT_NAME_TYPE_UNSPECIFIED: return RDM_OMMSTR_INSTRUMENT_NAME_TYPE_UNSPECIFIED.data;
		case RDM_INSTRUMENT_NAME_TYPE_RIC: return RDM_OMMSTR_INSTRUMENT_NAME_TYPE_RIC.data;
		case RDM_INSTRUMENT_NAME_TYPE_CONTRIBUTOR: return RDM_OMMSTR_INSTRUMENT_NAME_TYPE_CONTRIBUTOR.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMUpdateEventTypeToOmmString(RsslUInt8 type)
{
	switch(type)
	{
		case RDM_UPD_EVENT_TYPE_UNSPECIFIED: return RDM_OMMSTR_UPD_EVENT_TYPE_UNSPECIFIED.data;
		case RDM_UPD_EVENT_TYPE_QUOTE: return RDM_OMMSTR_UPD_EVENT_TYPE_QUOTE.data;
		case RDM_UPD_EVENT_TYPE_TRADE: return RDM_OMMSTR_UPD_EVENT_TYPE_TRADE.data;
		case RDM_UPD_EVENT_TYPE_NEWS_ALERT: return RDM_OMMSTR_UPD_EVENT_TYPE_NEWS_ALERT.data;
		case RDM_UPD_EVENT_TYPE_VOLUME_ALERT: return RDM_OMMSTR_UPD_EVENT_TYPE_VOLUME_ALERT.data;
		case RDM_UPD_EVENT_TYPE_ORDER_INDICATION: return RDM_OMMSTR_UPD_EVENT_TYPE_ORDER_INDICATION.data;
		case RDM_UPD_EVENT_TYPE_CLOSING_RUN: return RDM_OMMSTR_UPD_EVENT_TYPE_CLOSING_RUN.data;
		case RDM_UPD_EVENT_TYPE_CORRECTION: return RDM_OMMSTR_UPD_EVENT_TYPE_CORRECTION.data;
		case RDM_UPD_EVENT_TYPE_MARKET_DIGEST: return RDM_OMMSTR_UPD_EVENT_TYPE_MARKET_DIGEST.data;
		case RDM_UPD_EVENT_TYPE_QUOTES_TRADE: return RDM_OMMSTR_UPD_EVENT_TYPE_QUOTES_TRADE.data;
		case RDM_UPD_EVENT_TYPE_MULTIPLE: return RDM_OMMSTR_UPD_EVENT_TYPE_MULTIPLE.data;
		case RDM_UPD_EVENT_TYPE_VERIFY: return RDM_OMMSTR_UPD_EVENT_TYPE_VERIFY.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMViewTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_VIEW_TYPE_FIELD_ID_LIST: return RDM_OMMSTR_VIEW_TYPE_FIELD_ID_LIST.data;
		case RDM_VIEW_TYPE_ELEMENT_NAME_LIST: return RDM_OMMSTR_VIEW_TYPE_ELEMENT_NAME_LIST.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMLoginRoleTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_LOGIN_ROLE_CONS: return RDM_OMMSTR_LOGIN_ROLE_CONS.data;
		case RDM_LOGIN_ROLE_PROV: return RDM_OMMSTR_LOGIN_ROLE_PROV.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMLoginUserIdTypeToOmmString(RsslUInt8 type)
{
	switch(type)
	{
		case RDM_LOGIN_USER_NAME: return RDM_OMMSTR_LOGIN_USER_NAME.data;
		case RDM_LOGIN_USER_EMAIL_ADDRESS: return RDM_OMMSTR_LOGIN_USER_EMAIL_ADDRESS.data;
		case RDM_LOGIN_USER_TOKEN: return RDM_OMMSTR_LOGIN_USER_TOKEN.data;
		case RDM_LOGIN_USER_COOKIE: return RDM_OMMSTR_LOGIN_USER_COOKIE.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMLoginServerTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_LOGIN_SERVER_TYPE_ACTIVE: return RDM_OMMSTR_LOGIN_SERVER_TYPE_ACTIVE.data;
		case RDM_LOGIN_SERVER_TYPE_STANDBY: return RDM_OMMSTR_LOGIN_SERVER_TYPE_STANDBY.data;
		default: return NULL;
	}
}

RSSL_API RsslRet rsslRDMLoginBatchSupportFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt flags)
{
	const char *flagStrings[3 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RDM_LOGIN_BATCH_SUPPORT_REQUESTS) flagStrings[flagCount++] = RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REQUESTS.data;
	if (flags & RDM_LOGIN_BATCH_SUPPORT_REISSUES) flagStrings[flagCount++] = RDM_OMMSTR_LOGIN_BATCH_SUPPORT_REISSUES.data;
	if (flags & RDM_LOGIN_BATCH_SUPPORT_CLOSES) flagStrings[flagCount++] = RDM_OMMSTR_LOGIN_BATCH_SUPPORT_CLOSES.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API const char* rsslRDMDictionaryTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
        case RDM_DICTIONARY_FIELD_DEFINITIONS: return RDM_OMMSTR_DICTIONARY_FIELD_DEFINITIONS.data;
        case RDM_DICTIONARY_ENUM_TABLES: return RDM_OMMSTR_DICTIONARY_ENUM_TABLES.data;
        case RDM_DICTIONARY_RECORD_TEMPLATES: return RDM_OMMSTR_DICTIONARY_RECORD_TEMPLATES.data;
        case RDM_DICTIONARY_DISPLAY_TEMPLATES: return RDM_OMMSTR_DICTIONARY_DISPLAY_TEMPLATES.data;
        case RDM_DICTIONARY_DATA_DEFINITIONS: return RDM_OMMSTR_DICTIONARY_DATA_DEFINITIONS.data;
        case RDM_DICTIONARY_STYLE_SHEET: return RDM_OMMSTR_DICTIONARY_STYLE_SHEET.data;
        case RDM_DICTIONARY_REFERENCE: return RDM_OMMSTR_DICTIONARY_REFERENCE.data;
        case RDM_DICTIONARY_FIELD_SET_DEFINITION: return RDM_OMMSTR_DICTIONARY_FIELD_SET_DEFINITION.data;
        case RDM_DICTIONARY_ELEMENT_SET_DEFINITION: return RDM_OMMSTR_DICTIONARY_ELEMENT_SET_DEFINITION.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDictionaryVerbosityValueToOmmString(RsslUInt32 value)
{
	switch(value)
	{
		case RDM_DICTIONARY_INFO: return RDM_OMMSTR_DICTIONARY_INFO.data;
		case RDM_DICTIONARY_MINIMAL: return RDM_OMMSTR_DICTIONARY_MINIMAL.data;
		case RDM_DICTIONARY_NORMAL: return RDM_OMMSTR_DICTIONARY_NORMAL.data;
		case RDM_DICTIONARY_VERBOSE: return RDM_OMMSTR_DICTIONARY_VERBOSE.data;
		default: return NULL;
	}
}

RSSL_API RsslRet rsslRDMDirectoryServiceFilterFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt32 flags)
{
	const char *flagStrings[7 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RDM_DIRECTORY_SERVICE_INFO_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_INFO_FILTER.data;
	if (flags & RDM_DIRECTORY_SERVICE_STATE_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_STATE_FILTER.data;
	if (flags & RDM_DIRECTORY_SERVICE_GROUP_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_FILTER.data;
	if (flags & RDM_DIRECTORY_SERVICE_LOAD_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_FILTER.data;
	if (flags & RDM_DIRECTORY_SERVICE_DATA_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_DATA_FILTER.data;
	if (flags & RDM_DIRECTORY_SERVICE_LINK_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_LINK_FILTER.data;
	if (flags & RDM_DIRECTORY_SERVICE_SEQ_MCAST_FILTER) flagStrings[flagCount++] = RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_FILTER.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API const char* rsslRDMDirectoryServiceFilterIdToOmmString(RsslUInt id)
{
	switch(id)
	{
		case RDM_DIRECTORY_SERVICE_INFO_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_INFO_ID.data;
		case RDM_DIRECTORY_SERVICE_STATE_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_STATE_ID.data;
		case RDM_DIRECTORY_SERVICE_GROUP_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_GROUP_ID.data;
		case RDM_DIRECTORY_SERVICE_LOAD_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_LOAD_ID.data;
		case RDM_DIRECTORY_SERVICE_DATA_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_DATA_ID.data;
		case RDM_DIRECTORY_SERVICE_LINK_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_LINK_ID.data;
		case RDM_DIRECTORY_SERVICE_SEQ_MCAST_ID: return RDM_OMMSTR_DIRECTORY_SERVICE_SEQ_MCAST_ID.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDirectoryServiceStateToOmmString(RsslUInt serviceState)
{
	switch(serviceState)
	{
		case RDM_DIRECTORY_SERVICE_STATE_DOWN: return RDM_OMMSTR_DIRECTORY_SERVICE_STATE_DOWN.data;
		case RDM_DIRECTORY_SERVICE_STATE_UP: return RDM_OMMSTR_DIRECTORY_SERVICE_STATE_UP.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDirectorySourceMirroringModeToOmmString(RsslUInt mode)
{
	switch(mode)
	{
		case RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY: return RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_NO_STANDBY.data;
		case RDM_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY: return RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_ACTIVE_WITH_STANDBY.data;
		case RDM_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY: return RDM_OMMSTR_DIRECTORY_SOURCE_MIRROR_MODE_STANDBY.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDirectoryDataTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_DIRECTORY_DATA_TYPE_NONE: return RDM_OMMSTR_DIRECTORY_DATA_TYPE_NONE.data;
		case RDM_DIRECTORY_DATA_TYPE_TIME: return RDM_OMMSTR_DIRECTORY_DATA_TYPE_TIME.data;
		case RDM_DIRECTORY_DATA_TYPE_ALERT: return RDM_OMMSTR_DIRECTORY_DATA_TYPE_ALERT.data;
		case RDM_DIRECTORY_DATA_TYPE_HEADLINE: return RDM_OMMSTR_DIRECTORY_DATA_TYPE_HEADLINE.data;
		case RDM_DIRECTORY_DATA_TYPE_STATUS: return RDM_OMMSTR_DIRECTORY_DATA_TYPE_STATUS.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDirectoryLinkTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_DIRECTORY_LINK_TYPE_INTERACTIVE: return RDM_OMMSTR_DIRECTORY_LINK_TYPE_INTERACTIVE.data;
		case RDM_DIRECTORY_LINK_TYPE_BROADCAST: return RDM_OMMSTR_DIRECTORY_LINK_TYPE_BROADCAST.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDirectoryLinkCodeToOmmString(RsslUInt code)
{
	switch(code)
	{
		case RDM_DIRECTORY_LINK_CODE_NONE: return RDM_OMMSTR_DIRECTORY_LINK_CODE_NONE.data;
		case RDM_DIRECTORY_LINK_CODE_OK: return RDM_OMMSTR_DIRECTORY_LINK_CODE_OK.data;
		case RDM_DIRECTORY_LINK_CODE_RECOVERY_STARTED: return RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_STARTED.data;
		case RDM_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED: return RDM_OMMSTR_DIRECTORY_LINK_CODE_RECOVERY_COMPLETED.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMDirectoryLinkStateToOmmString(RsslUInt state)
{
	switch(state)
	{
		case RDM_DIRECTORY_LINK_STATE_DOWN: return RDM_OMMSTR_DIRECTORY_LINK_STATE_DOWN.data;
		case RDM_DIRECTORY_LINK_STATE_UP: return RDM_OMMSTR_DIRECTORY_LINK_STATE_UP.data;
		default: return NULL;
	}
}


RSSL_API RsslRet rsslRDMEnhancedSymbolListSupportFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt flags)
{
	const char *flagStrings[1 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RDM_SYMBOL_LIST_SUPPORT_DATA_STREAMS) flagStrings[flagCount++] = RDM_OMMSTR_SYMBOL_LIST_SUPPORT_DATA_STREAMS.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslRDMSymbolListDataStreamRequestFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt flags)
{
	const char *flagStrings[2 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RDM_SYMBOL_LIST_DATA_STREAMS) flagStrings[flagCount++] = RDM_OMMSTR_SYMBOL_LIST_DATA_STREAMS.data;
	if (flags & RDM_SYMBOL_LIST_DATA_SNAPSHOTS) flagStrings[flagCount++] = RDM_OMMSTR_SYMBOL_LIST_DATA_SNAPSHOTS.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API const char* rsslRDMCosFilterIdToOmmString(RsslUInt id)
{
	switch(id)
	{
		case RDM_COS_COMMON_PROPERTIES_ID: return RDM_OMMSTR_COS_COMMON_PROPERTIES_ID.data;
		case RDM_COS_AUTHENTICATION_ID: return RDM_OMMSTR_COS_AUTHENTICATION_ID.data;
		case RDM_COS_FLOW_CONTROL_ID: return RDM_OMMSTR_COS_FLOW_CONTROL_ID.data;
		case RDM_COS_DATA_INTEGRITY_ID: return RDM_OMMSTR_COS_DATA_INTEGRITY_ID.data;
		case RDM_COS_GUARANTEE_ID: return RDM_OMMSTR_COS_GUARANTEE_ID.data;
		default: return NULL;
	}
}

RSSL_API RsslRet rsslRDMCosFilterFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt32 flags)
{
	const char *flagStrings[5 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RDM_COS_COMMON_PROPERTIES_FLAG) flagStrings[flagCount++] = RDM_OMMSTR_COS_COMMON_PROPERTIES_FLAG.data;
	if (flags & RDM_COS_AUTHENTICATION_FLAG) flagStrings[flagCount++] = RDM_OMMSTR_COS_AUTHENTICATION_FLAG.data;
	if (flags & RDM_COS_FLOW_CONTROL_FLAG) flagStrings[flagCount++] = RDM_OMMSTR_COS_FLOW_CONTROL_FLAG.data;
	if (flags & RDM_COS_DATA_INTEGRITY_FLAG) flagStrings[flagCount++] = RDM_OMMSTR_COS_DATA_INTEGRITY_FLAG.data;
	if (flags & RDM_COS_GUARANTEE_FLAG) flagStrings[flagCount++] = RDM_OMMSTR_COS_GUARANTEE_FLAG.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API const char* rsslRDMCosAuthenticationTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_COS_AU_NOT_REQUIRED: return RDM_OMMSTR_COS_AU_NOT_REQUIRED.data;
		case RDM_COS_AU_OMM_LOGIN: return RDM_OMMSTR_COS_AU_OMM_LOGIN.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMCosFlowControlTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_COS_FC_NONE: return RDM_OMMSTR_COS_FC_NONE.data;
		case RDM_COS_FC_BIDIRECTIONAL: return RDM_OMMSTR_COS_FC_BIDIRECTIONAL.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMCosDataIntegrityTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_COS_DI_BEST_EFFORT: return RDM_OMMSTR_COS_DI_BEST_EFFORT.data;
		case RDM_COS_DI_RELIABLE: return RDM_OMMSTR_COS_DI_RELIABLE.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslRDMCosGuaranteeTypeToOmmString(RsslUInt type)
{
	switch(type)
	{
		case RDM_COS_GU_NONE: return RDM_OMMSTR_COS_GU_NONE.data;
		case RDM_COS_GU_PERSISTENT_QUEUE: return RDM_OMMSTR_COS_GU_PERSISTENT_QUEUE.data;
		default: return NULL;
	}
}

RSSL_API const char* rsslNakCodeToOmmString(RsslUInt8 code)
{
	switch(code)
	{
		case RSSL_NAKC_NONE: 
			return RSSL_OMMSTR_NAKC_NONE.data;
		case RSSL_NAKC_ACCESS_DENIED: 
			return RSSL_OMMSTR_NAKC_ACCESS_DENIED.data;
		case RSSL_NAKC_DENIED_BY_SRC: 
			return RSSL_OMMSTR_NAKC_DENIED_BY_SRC.data;
		case RSSL_NAKC_SOURCE_DOWN: 
			return RSSL_OMMSTR_NAKC_SOURCE_DOWN.data;
		case RSSL_NAKC_SOURCE_UNKNOWN: 
			return RSSL_OMMSTR_NAKC_SOURCE_UNKNOWN.data;
		case RSSL_NAKC_NO_RESOURCES: 
			return RSSL_OMMSTR_NAKC_NO_RESOURCES.data;
		case RSSL_NAKC_NO_RESPONSE: 
			return RSSL_OMMSTR_NAKC_NO_RESPONSE.data;
		case RSSL_NAKC_GATEWAY_DOWN: 
			return RSSL_OMMSTR_NAKC_GATEWAY_DOWN.data;
		case RSSL_NAKC_SYMBOL_UNKNOWN: 
			return RSSL_OMMSTR_NAKC_SYMBOL_UNKNOWN.data;
		case RSSL_NAKC_NOT_OPEN: 
			return RSSL_OMMSTR_NAKC_NOT_OPEN.data;
		case RSSL_NAKC_INVALID_CONTENT: 
			return RSSL_OMMSTR_NAKC_INVALID_CONTENT.data;
		default:
			return NULL;
	}
}

RSSL_API RsslRet rsslRequestFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[13 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_RQMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_RQMF_HAS_PRIORITY) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_HAS_PRIORITY.data;
	if (flags & RSSL_RQMF_STREAMING) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_STREAMING.data;
	if (flags & RSSL_RQMF_MSG_KEY_IN_UPDATES) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_MSG_KEY_IN_UPDATES.data;
	if (flags & RSSL_RQMF_CONF_INFO_IN_UPDATES) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_CONF_INFO_IN_UPDATES.data;
	if (flags & RSSL_RQMF_NO_REFRESH) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_NO_REFRESH.data;
	if (flags & RSSL_RQMF_HAS_QOS) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_HAS_QOS.data;
	if (flags & RSSL_RQMF_HAS_WORST_QOS) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_HAS_WORST_QOS.data;
	if (flags & RSSL_RQMF_PRIVATE_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_PRIVATE_STREAM.data;
	if (flags & RSSL_RQMF_PAUSE) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_PAUSE.data;
	if (flags & RSSL_RQMF_HAS_VIEW) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_HAS_VIEW.data;
	if (flags & RSSL_RQMF_HAS_BATCH) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_HAS_BATCH.data;
	if (flags & RSSL_RQMF_QUALIFIED_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_RQMF_QUALIFIED_STREAM.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslRefreshFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[14 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_RFMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_RFMF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_PERM_DATA.data;
	if (flags & RSSL_RFMF_HAS_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_MSG_KEY.data;
	if (flags & RSSL_RFMF_HAS_SEQ_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_SEQ_NUM.data;
	if (flags & RSSL_RFMF_SOLICITED) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_SOLICITED.data;
	if (flags & RSSL_RFMF_REFRESH_COMPLETE) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_REFRESH_COMPLETE.data;
	if (flags & RSSL_RFMF_HAS_QOS) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_QOS.data;
	if (flags & RSSL_RFMF_CLEAR_CACHE) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_CLEAR_CACHE.data;
	if (flags & RSSL_RFMF_DO_NOT_CACHE) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_DO_NOT_CACHE.data;
	if (flags & RSSL_RFMF_PRIVATE_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_PRIVATE_STREAM.data;
	if (flags & RSSL_RFMF_HAS_POST_USER_INFO) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_POST_USER_INFO.data;
	if (flags & RSSL_RFMF_HAS_PART_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_PART_NUM.data;
	if (flags & RSSL_RFMF_HAS_REQ_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_HAS_REQ_MSG_KEY.data;
	if (flags & RSSL_RFMF_QUALIFIED_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_RFMF_QUALIFIED_STREAM.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslStatusFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[10 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_STMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_STMF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_PERM_DATA.data;
	if (flags & RSSL_STMF_HAS_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_MSG_KEY.data;
	if (flags & RSSL_STMF_HAS_GROUP_ID) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_GROUP_ID.data;
	if (flags & RSSL_STMF_HAS_STATE) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_STATE.data;
	if (flags & RSSL_STMF_CLEAR_CACHE) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_CLEAR_CACHE.data;
	if (flags & RSSL_STMF_PRIVATE_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_PRIVATE_STREAM.data;
	if (flags & RSSL_STMF_HAS_POST_USER_INFO) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_POST_USER_INFO.data;
	if (flags & RSSL_STMF_HAS_REQ_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_HAS_REQ_MSG_KEY.data;
	if (flags & RSSL_STMF_QUALIFIED_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_STMF_QUALIFIED_STREAM.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslUpdateFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[10 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_UPMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_UPMF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_HAS_PERM_DATA.data;
	if (flags & RSSL_UPMF_HAS_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_HAS_MSG_KEY.data;
	if (flags & RSSL_UPMF_HAS_SEQ_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_HAS_SEQ_NUM.data;
	if (flags & RSSL_UPMF_HAS_CONF_INFO) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_HAS_CONF_INFO.data;
	if (flags & RSSL_UPMF_DO_NOT_CACHE) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_DO_NOT_CACHE.data;
	if (flags & RSSL_UPMF_DO_NOT_CONFLATE) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_DO_NOT_CONFLATE.data;
	if (flags & RSSL_UPMF_DO_NOT_RIPPLE) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_DO_NOT_RIPPLE.data;
	if (flags & RSSL_UPMF_HAS_POST_USER_INFO) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_HAS_POST_USER_INFO.data;
	if (flags & RSSL_UPMF_DISCARDABLE) flagStrings[flagCount++] = RSSL_OMMSTR_UPMF_DISCARDABLE.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslCloseFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[3 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_CLMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_CLMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_CLMF_ACK) flagStrings[flagCount++] = RSSL_OMMSTR_CLMF_ACK.data;
	if (flags & RSSL_CLMF_HAS_BATCH) flagStrings[flagCount++] = RSSL_OMMSTR_CLMF_HAS_BATCH.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslAckFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[7 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_AKMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_AKMF_HAS_TEXT) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_HAS_TEXT.data;
	if (flags & RSSL_AKMF_PRIVATE_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_PRIVATE_STREAM.data;
	if (flags & RSSL_AKMF_HAS_SEQ_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_HAS_SEQ_NUM.data;
	if (flags & RSSL_AKMF_HAS_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_HAS_MSG_KEY.data;
	if (flags & RSSL_AKMF_HAS_NAK_CODE) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_HAS_NAK_CODE.data;
	if (flags & RSSL_AKMF_QUALIFIED_STREAM) flagStrings[flagCount++] = RSSL_OMMSTR_AKMF_QUALIFIED_STREAM.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslGenericFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[8 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_GNMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_GNMF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_PERM_DATA.data;
	if (flags & RSSL_GNMF_HAS_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_MSG_KEY.data;
	if (flags & RSSL_GNMF_HAS_SEQ_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_SEQ_NUM.data;
	if (flags & RSSL_GNMF_MESSAGE_COMPLETE) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_MESSAGE_COMPLETE.data;
	if (flags & RSSL_GNMF_HAS_SECONDARY_SEQ_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_SECONDARY_SEQ_NUM.data;
	if (flags & RSSL_GNMF_HAS_PART_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_PART_NUM.data;
	if (flags & RSSL_GNMF_HAS_REQ_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_GNMF_HAS_REQ_MSG_KEY.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslPostFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[9 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_PSMF_HAS_EXTENDED_HEADER) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_EXTENDED_HEADER.data;
	if (flags & RSSL_PSMF_HAS_POST_ID) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_POST_ID.data;
	if (flags & RSSL_PSMF_HAS_MSG_KEY) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_MSG_KEY.data;
	if (flags & RSSL_PSMF_HAS_SEQ_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_SEQ_NUM.data;
	if (flags & RSSL_PSMF_POST_COMPLETE) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_POST_COMPLETE.data;
	if (flags & RSSL_PSMF_ACK) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_ACK.data;
	if (flags & RSSL_PSMF_HAS_PERM_DATA) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_PERM_DATA.data;
	if (flags & RSSL_PSMF_HAS_PART_NUM) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_PART_NUM.data;
	if (flags & RSSL_PSMF_HAS_POST_USER_RIGHTS) flagStrings[flagCount++] = RSSL_OMMSTR_PSMF_HAS_POST_USER_RIGHTS.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslPostUserRightsToOmmString(RsslBuffer *oBuffer, RsslUInt16 rights)
{
	const char *flagStrings[3 /* The max possible number of rights */];
	int flagCount = 0;

	if (rights & RSSL_PSUR_CREATE) flagStrings[flagCount++] = RSSL_OMMSTR_PSUR_CREATE.data;
	if (rights & RSSL_PSUR_DELETE) flagStrings[flagCount++] = RSSL_OMMSTR_PSUR_DELETE.data;
	if (rights & RSSL_PSUR_MODIFY_PERM) flagStrings[flagCount++] = RSSL_OMMSTR_PSUR_MODIFY_PERM.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}

RSSL_API RsslRet rsslMsgKeyFlagsToOmmString(RsslBuffer *oBuffer, RsslUInt16 flags)
{
	const char *flagStrings[6 /* The max possible number of flags */];
	int flagCount = 0;

	if (flags & RSSL_MKF_HAS_SERVICE_ID) flagStrings[flagCount++] = RSSL_OMMSTR_MKF_HAS_SERVICE_ID.data;
	if (flags & RSSL_MKF_HAS_NAME) flagStrings[flagCount++] = RSSL_OMMSTR_MKF_HAS_NAME.data;
	if (flags & RSSL_MKF_HAS_NAME_TYPE) flagStrings[flagCount++] = RSSL_OMMSTR_MKF_HAS_NAME_TYPE.data;
	if (flags & RSSL_MKF_HAS_FILTER) flagStrings[flagCount++] = RSSL_OMMSTR_MKF_HAS_FILTER.data;
	if (flags & RSSL_MKF_HAS_IDENTIFIER) flagStrings[flagCount++] = RSSL_OMMSTR_MKF_HAS_IDENTIFIER.data;
	if (flags & RSSL_MKF_HAS_ATTRIB) flagStrings[flagCount++] = RSSL_OMMSTR_MKF_HAS_ATTRIB.data;

	return _rsslFlagsToOmmString(oBuffer, flagStrings, flagCount);
}



RSSL_API RsslUInt8 rsslDomainTypeFromString(char *domainTypeString)
{
	if (strcmp(domainTypeString, "RSSL_DMT_LOGIN") == 0)
		return RSSL_DMT_LOGIN;
	else if (strcmp(domainTypeString, "RSSL_DMT_SOURCE") == 0)
		return RSSL_DMT_SOURCE;
	else if (strcmp(domainTypeString, "RSSL_DMT_DICTIONARY") == 0)
		return RSSL_DMT_DICTIONARY;
	else if (strcmp(domainTypeString, "RSSL_DMT_MARKET_PRICE") == 0)
		return RSSL_DMT_MARKET_PRICE;
	else if (strcmp(domainTypeString, "RSSL_DMT_MARKET_BY_ORDER") == 0)
		return RSSL_DMT_MARKET_BY_ORDER;
	else if (strcmp(domainTypeString, "RSSL_DMT_MARKET_BY_PRICE") == 0)
		return RSSL_DMT_MARKET_BY_PRICE;
	else if (strcmp(domainTypeString, "RSSL_DMT_MARKET_MAKER") == 0)
		return RSSL_DMT_MARKET_MAKER;
	else if (strcmp(domainTypeString, "RSSL_DMT_SYMBOL_LIST") == 0)
		return RSSL_DMT_SYMBOL_LIST;
	else if (strcmp(domainTypeString, "RSSL_DMT_SERVICE_PROVIDER_STATUS") == 0)
		return RSSL_DMT_SERVICE_PROVIDER_STATUS;
	else if (strcmp(domainTypeString, "RSSL_DMT_HISTORY") == 0)
		return RSSL_DMT_HISTORY;
	else if (strcmp(domainTypeString, "RSSL_DMT_TRANSACTION") == 0)
		return RSSL_DMT_TRANSACTION;
	else if (strcmp(domainTypeString, "RSSL_DMT_YIELD_CURVE") == 0)
		return RSSL_DMT_YIELD_CURVE;
	else if (strcmp(domainTypeString, "RSSL_DMT_CONTRIBUTION") == 0)
		return RSSL_DMT_CONTRIBUTION;
	else if (strcmp(domainTypeString, "RSSL_DMT_HEADLINE") == 0)
		return RSSL_DMT_HEADLINE;
	else if (strcmp(domainTypeString, "RSSL_DMT_STORY") == 0)
		return RSSL_DMT_STORY;
	else if (strcmp(domainTypeString, "RSSL_DMT_REPLAYHEADLINE") == 0)
		return RSSL_DMT_REPLAYHEADLINE;
	else if (strcmp(domainTypeString, "RSSL_DMT_REPLAYSTORY") == 0)
		return RSSL_DMT_REPLAYSTORY;
	else if (strcmp(domainTypeString, "RSSL_DMT_ANALYTICS") == 0)
		return RSSL_DMT_ANALYTICS;
	else if (strcmp(domainTypeString, "RSSL_DMT_REFERENCE") == 0)
		return RSSL_DMT_REFERENCE;
	else if (strcmp(domainTypeString, "RSSL_DMT_NEWS_TEXT_ANALYTICS") == 0)
		return RSSL_DMT_NEWS_TEXT_ANALYTICS;
	else if (strcmp(domainTypeString, "RSSL_DMT_ECONOMIC_INDICATOR") == 0)
		return RSSL_DMT_ECONOMIC_INDICATOR;
	else if (strcmp(domainTypeString, "RSSL_DMT_POLL") == 0)
		return RSSL_DMT_POLL;
	else if (strcmp(domainTypeString, "RSSL_DMT_FORECAST") == 0)
		return RSSL_DMT_FORECAST;
	else if (strcmp(domainTypeString, "RSSL_DMT_MARKET_BY_TIME") == 0)
		return RSSL_DMT_MARKET_BY_TIME;
	else if (strcmp(domainTypeString, "RSSL_DMT_SYSTEM") == 0)
		return RSSL_DMT_SYSTEM;
	else
		return 0;  //Domain Type is unknown or custom (user defined)
}


RsslBool _rsslValidateMsgKey(const RsslMsgKey * key)
{
	if (key == 0)
		return RSSL_TRUE;

	if ( (key->flags & RSSL_MKF_HAS_NAME) &&
		    key->name.length != 0 && 
		    key->name.data == 0 )
		return RSSL_FALSE;

	if ( (key->flags & RSSL_MKF_HAS_ATTRIB) &&
			( key->encAttrib.length == 0 || 
			  key->encAttrib.data   == 0 ) )
		return RSSL_FALSE;

	return RSSL_TRUE;
}


/* IMPORTANT: When new message classes are added, rsslCopyMsg, rsslValidateMsg, and rsslValidateEncodedMsgBuf have to modified as well */

RSSL_API RsslBool rsslValidateMsg(const RsslMsg *pMsg)
{
     if( pMsg == 0 )
        return RSSL_FALSE;

    if( pMsg->msgBase.encDataBody.length && pMsg->msgBase.encDataBody.data == 0 )
        return RSSL_FALSE;

    if( pMsg->msgBase.encMsgBuffer.length && pMsg->msgBase.encMsgBuffer.data == 0 )
        return RSSL_FALSE;

	if (!_rsslValidateMsgKey(rsslGetMsgKey((RsslMsg*)pMsg)))
		return RSSL_FALSE;

	if (!_rsslValidateMsgKey(rsslGetReqMsgKey((RsslMsg*)pMsg)))
		return RSSL_FALSE;

    switch( pMsg->msgBase.msgClass )
    {
        case RSSL_MC_UPDATE: 
            if( ( pMsg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA ) &&
                ( pMsg->updateMsg.permData.length == 0 ||
                  pMsg->updateMsg.permData.data   == 0 ) )
                return RSSL_FALSE;

            if( ( pMsg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->updateMsg.extendedHeader.length     == 0 ||
                  pMsg->updateMsg.extendedHeader.data       == 0 ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 

        case RSSL_MC_GENERIC: 
            if( ( pMsg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA ) &&
                ( pMsg->genericMsg.permData.length == 0 ||
                  pMsg->genericMsg.permData.data   == 0 ) )
                return RSSL_FALSE;

            if( ( pMsg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->genericMsg.extendedHeader.length     == 0 ||
                  pMsg->genericMsg.extendedHeader.data       == 0 ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 

		case RSSL_MC_REFRESH:			
             if( pMsg->refreshMsg.state.text.length != 0 && pMsg->refreshMsg.state.text.data == 0 ) 
                return RSSL_FALSE;

            if( ( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA ) &&
				( pMsg->refreshMsg.permData.length   == 0 ||
				  pMsg->refreshMsg.permData.data     == 0 ) )
                return RSSL_FALSE;

			if( pMsg->refreshMsg.groupId.length != 0 && pMsg->refreshMsg.groupId.data == 0 )
				return RSSL_FALSE;

            if( ( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->refreshMsg.extendedHeader.length   == 0 ||
                  pMsg->refreshMsg.extendedHeader.data     == 0 ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 

		case RSSL_MC_POST:			
             if( ( pMsg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA ) &&
				( pMsg->postMsg.permData.length   == 0 ||
				  pMsg->postMsg.permData.data     == 0 ) )
                return RSSL_FALSE;

            if( ( pMsg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->postMsg.extendedHeader.length   == 0 ||
                  pMsg->postMsg.extendedHeader.data     == 0 ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 

		case RSSL_MC_REQUEST: 
           if( ( pMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->requestMsg.extendedHeader.length   == 0 ||
                  pMsg->requestMsg.extendedHeader.data     == 0 ) )
                return RSSL_FALSE;

	        return RSSL_TRUE;

		case RSSL_MC_STATUS: 
            if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE ) &&
                  pMsg->statusMsg.state.text.length != 0 && pMsg->statusMsg.state.text.data == 0 ) 
                return RSSL_FALSE;

            if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA ) &&
                ( pMsg->statusMsg.permData.length == 0 ||
                  pMsg->statusMsg.permData.data   == 0 ) )
                return RSSL_FALSE;

			if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID ) &&
				( pMsg->statusMsg.groupId.length == 0 ||
				  pMsg->statusMsg.groupId.data == 0 ) )
				return RSSL_FALSE;

			if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->statusMsg.extendedHeader.length     == 0 ||
                  pMsg->statusMsg.extendedHeader.data       == 0 ) )
                return RSSL_FALSE;

            return RSSL_TRUE;

		case RSSL_MC_CLOSE: 
            if( ( pMsg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->closeMsg.extendedHeader.length     == 0 ||
                  pMsg->closeMsg.extendedHeader.data       == 0 ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 

		case RSSL_MC_ACK:
           if( ( pMsg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->ackMsg.extendedHeader.length   == 0 ||
                  pMsg->ackMsg.extendedHeader.data     == 0 ) )
                return RSSL_FALSE;

           if( ( pMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT ) &&
                ( pMsg->ackMsg.text.length   == 0 ||
                  pMsg->ackMsg.text.data     == 0 ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 

        default:
            return RSSL_FALSE; 
    }
}

RSSL_API RsslBool rsslValidateEncodedMsgBuf(const RsslMsg *pMsg)
{
	char *encMsgBufStart, *encMsgBufEnd;

	encMsgBufStart = pMsg->msgBase.encMsgBuffer.data;
	encMsgBufEnd = pMsg->msgBase.encMsgBuffer.data + pMsg->msgBase.encMsgBuffer.length;

    switch( pMsg->msgBase.msgClass )
    {
        case RSSL_MC_UPDATE: 
			if ( pMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY )
			{
				if( ( pMsg->updateMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->updateMsg.msgBase.msgKey.name.data < encMsgBufStart ||
					  pMsg->updateMsg.msgBase.msgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->updateMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->updateMsg.msgBase.msgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->updateMsg.msgBase.msgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

            if( ( pMsg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA ) &&
                ( pMsg->updateMsg.permData.data < encMsgBufStart ||
                  pMsg->updateMsg.permData.data > encMsgBufEnd ) )
                return RSSL_FALSE;

            if( ( pMsg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->updateMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->updateMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 
        case RSSL_MC_GENERIC: 
			if ( pMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY )
			{
				if( ( pMsg->genericMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->genericMsg.msgBase.msgKey.name.data < encMsgBufStart ||
					  pMsg->genericMsg.msgBase.msgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->genericMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->genericMsg.msgBase.msgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->genericMsg.msgBase.msgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

			if ( pMsg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY )
			{
				if( ( pMsg->genericMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->genericMsg.reqMsgKey.name.data < encMsgBufStart ||
					  pMsg->genericMsg.reqMsgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->genericMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->genericMsg.reqMsgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->genericMsg.reqMsgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

            if( ( pMsg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA ) &&
                ( pMsg->genericMsg.permData.data < encMsgBufStart ||
                  pMsg->genericMsg.permData.data > encMsgBufEnd ) )
                return RSSL_FALSE;

            if( ( pMsg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->genericMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->genericMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			return RSSL_TRUE;
		case RSSL_MC_REFRESH:			
			if ( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY )
			{
				if( ( pMsg->refreshMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->refreshMsg.msgBase.msgKey.name.data < encMsgBufStart ||
					  pMsg->refreshMsg.msgBase.msgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->refreshMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->refreshMsg.msgBase.msgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->refreshMsg.msgBase.msgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

			if ( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY )
			{
				if( ( pMsg->refreshMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->refreshMsg.reqMsgKey.name.data < encMsgBufStart ||
					  pMsg->refreshMsg.reqMsgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->refreshMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->refreshMsg.reqMsgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->refreshMsg.reqMsgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

            if( ( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA ) &&
                ( pMsg->refreshMsg.permData.data < encMsgBufStart ||
                  pMsg->refreshMsg.permData.data > encMsgBufEnd ) )
                return RSSL_FALSE;

            if( ( pMsg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->refreshMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->refreshMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			if( pMsg->refreshMsg.state.text.data < encMsgBufStart || pMsg->refreshMsg.state.text.data > encMsgBufEnd )
                return RSSL_FALSE;

			if( pMsg->refreshMsg.groupId.data < encMsgBufStart || pMsg->refreshMsg.groupId.data > encMsgBufEnd )
                return RSSL_FALSE;

			return RSSL_TRUE; 
		case RSSL_MC_STATUS: 
			if ( pMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY )
			{
				if( ( pMsg->statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->statusMsg.msgBase.msgKey.name.data < encMsgBufStart ||
					  pMsg->statusMsg.msgBase.msgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->statusMsg.msgBase.msgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->statusMsg.msgBase.msgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

			if ( pMsg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY )
			{
				if( ( pMsg->statusMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->statusMsg.reqMsgKey.name.data < encMsgBufStart ||
					  pMsg->statusMsg.reqMsgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->statusMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->statusMsg.reqMsgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->statusMsg.reqMsgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

            if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_STATE ) &&
                 ( pMsg->statusMsg.state.text.data < encMsgBufStart ||
				   pMsg->statusMsg.state.text.data > encMsgBufEnd ) ) 
                return RSSL_FALSE;

            if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA ) &&
                ( pMsg->statusMsg.permData.data < encMsgBufStart ||
                  pMsg->statusMsg.permData.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID ) &&
				( pMsg->statusMsg.groupId.data < encMsgBufStart ||
				  pMsg->statusMsg.groupId.data > encMsgBufEnd ) )
				return RSSL_FALSE;

			if( ( pMsg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->statusMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->statusMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

            return RSSL_TRUE;
		case RSSL_MC_POST:			
			if ( pMsg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY )
			{
				if( ( pMsg->postMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->postMsg.msgBase.msgKey.name.data < encMsgBufStart ||
					  pMsg->postMsg.msgBase.msgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->postMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->postMsg.msgBase.msgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->postMsg.msgBase.msgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

            if( ( pMsg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA ) &&
                ( pMsg->postMsg.permData.data < encMsgBufStart ||
                  pMsg->postMsg.permData.data > encMsgBufEnd ) )
                return RSSL_FALSE;

            if( ( pMsg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->postMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->postMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			return RSSL_TRUE;
		case RSSL_MC_REQUEST: 
           if( ( pMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->requestMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->requestMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

	        return RSSL_TRUE;
		case RSSL_MC_CLOSE: 
            if( ( pMsg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->closeMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->closeMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			return RSSL_TRUE; 
		case RSSL_MC_ACK:
			if ( pMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY )
			{
				if( ( pMsg->ackMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) &&
					( pMsg->ackMsg.msgBase.msgKey.name.data < encMsgBufStart ||
					  pMsg->ackMsg.msgBase.msgKey.name.data > encMsgBufEnd ) )
					return RSSL_FALSE;

				if( ( pMsg->ackMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB ) &&
					( pMsg->ackMsg.msgBase.msgKey.encAttrib.data < encMsgBufStart ||
					  pMsg->ackMsg.msgBase.msgKey.encAttrib.data > encMsgBufEnd ) )
					return RSSL_FALSE;
			}

			if( ( pMsg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER ) &&
                ( pMsg->ackMsg.extendedHeader.data < encMsgBufStart ||
                  pMsg->ackMsg.extendedHeader.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			if( ( pMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT ) &&
                ( pMsg->ackMsg.text.data < encMsgBufStart ||
                  pMsg->ackMsg.text.data > encMsgBufEnd ) )
                return RSSL_FALSE;

			return RSSL_TRUE;
        default:
            return RSSL_FALSE;
    }
}

RSSL_API RsslUInt32 rsslSizeOfMsg(const RsslMsg *pSrcMsg, RsslUInt32 copyMsgFlags)  
{
    RsslUInt32 mallocSize = __RSZRSSLMSG;

    const  RsslBuffer   *pStateTextBuffer;
    RsslUInt32			stateTextLength;

    const  RsslMsgKey   *pKey, *pReqKey;
    const  RsslBuffer   *pPermDataBuffer;
	const  RsslBuffer   *pKeyNameBuffer, *pReqKeyNameBuffer;
	const  RsslBuffer	*pGroupIdBuffer;
	RsslUInt32			keyNameLength, reqKeyNameLength;
    const  RsslBuffer   *pKeyOpaqueBuffer, *pReqKeyAttribBuffer;
    const  RsslBuffer   *pExtendedHeaderBuffer;
	const  RsslBuffer	*pNakTextBuffer;

    _RSSL_ASSERT( rsslValidateMsg( pSrcMsg ), Invalid message contents );

	/* optimize for update path */
	if (pSrcMsg->msgBase.msgClass == RSSL_MC_UPDATE)
	{
		if ((copyMsgFlags & RSSL_CMF_PERM_DATA) && (pSrcMsg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA))
			mallocSize += pSrcMsg->updateMsg.permData.length;

		if (pSrcMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY)
		{
			if ((copyMsgFlags & RSSL_CMF_KEY_NAME) && (pSrcMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME))
				mallocSize += pSrcMsg->msgBase.msgKey.name.length;

			if (( copyMsgFlags & RSSL_CMF_KEY_ATTRIB ) && (pSrcMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB))
				mallocSize += pSrcMsg->msgBase.msgKey.encAttrib.length;
		}

		if ((copyMsgFlags & RSSL_CMF_EXTENDED_HEADER) && (pSrcMsg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER))
			mallocSize += pSrcMsg->updateMsg.extendedHeader.length;

		      
	    if((copyMsgFlags & RSSL_CMF_DATA_BODY) && pSrcMsg->msgBase.encDataBody.length)
			mallocSize += pSrcMsg->msgBase.encDataBody.length;
    
		if((copyMsgFlags & RSSL_CMF_MSG_BUFFER) &&  pSrcMsg->msgBase.encMsgBuffer.length)
			mallocSize += pSrcMsg->msgBase.encMsgBuffer.length;

		return mallocSize;
	}

	if (pSrcMsg->msgBase.msgClass == RSSL_MC_GENERIC)
	{
		if ((copyMsgFlags & RSSL_CMF_PERM_DATA) && (pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA))
			mallocSize += pSrcMsg->genericMsg.permData.length;

		if (pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY)
		{
			if ((copyMsgFlags & RSSL_CMF_KEY_NAME) && (pSrcMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME))
				mallocSize += pSrcMsg->msgBase.msgKey.name.length;

			if (( copyMsgFlags & RSSL_CMF_KEY_ATTRIB ) && (pSrcMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB))
				mallocSize += pSrcMsg->msgBase.msgKey.encAttrib.length;
		}

		if (pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY)
		{
			if ((copyMsgFlags & RSSL_CMF_REQ_KEY_NAME) && (pSrcMsg->genericMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME))
				mallocSize += pSrcMsg->genericMsg.reqMsgKey.name.length;

			if (( copyMsgFlags & RSSL_CMF_REQ_KEY_ATTRIB ) && (pSrcMsg->genericMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB))
				mallocSize += pSrcMsg->genericMsg.reqMsgKey.encAttrib.length;
		}

		if ((copyMsgFlags & RSSL_CMF_EXTENDED_HEADER) && (pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER))
			mallocSize += pSrcMsg->genericMsg.extendedHeader.length;

		      
	    if((copyMsgFlags & RSSL_CMF_DATA_BODY) && pSrcMsg->msgBase.encDataBody.length)
			mallocSize += pSrcMsg->msgBase.encDataBody.length;
    
		if((copyMsgFlags & RSSL_CMF_MSG_BUFFER) &&  pSrcMsg->msgBase.encMsgBuffer.length)
			mallocSize += pSrcMsg->msgBase.encMsgBuffer.length;

		return mallocSize;
	}
    
	/* initialize variables for other paths */
    pStateTextBuffer = 0;
    stateTextLength = 0;

    pKey = 0;
	pReqKey = 0;
    pPermDataBuffer = 0;
	pKeyNameBuffer = 0;
	pReqKeyNameBuffer = 0;
	pGroupIdBuffer = 0;
	keyNameLength = 0;
	reqKeyNameLength = 0;
    pKeyOpaqueBuffer = 0;
    pReqKeyAttribBuffer = 0;
    pExtendedHeaderBuffer = 0;
	pNakTextBuffer = 0;

    switch( pSrcMsg->msgBase.msgClass )
    {
		case RSSL_MC_REFRESH:			
               pStateTextBuffer        = &pSrcMsg->refreshMsg.state.text; 

            if( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA ) 
                pPermDataBuffer          = &pSrcMsg->refreshMsg.permData;

			pGroupIdBuffer		    = &pSrcMsg->refreshMsg.groupId;
            
            if( ( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;

            if( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->refreshMsg.extendedHeader;
            
            if( ( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY ) )
				pReqKey = &pSrcMsg->refreshMsg.reqMsgKey;
			break; 

		case RSSL_MC_POST:			
			if(pSrcMsg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA)
				pPermDataBuffer = &pSrcMsg->postMsg.permData;
              
            if( ( pSrcMsg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;

            if( pSrcMsg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->postMsg.extendedHeader;
			break; 

		case RSSL_MC_REQUEST: 
            pKey = &pSrcMsg->msgBase.msgKey;

            if( pSrcMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->requestMsg.extendedHeader;
			break;

		case RSSL_MC_ACK: 
            if( pSrcMsg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->ackMsg.extendedHeader;
            
			if (pSrcMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				pNakTextBuffer = &pSrcMsg->ackMsg.text;

			if( ( pSrcMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;
			break; 

		case RSSL_MC_STATUS: 
            if( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_STATE )
                pStateTextBuffer        = &pSrcMsg->statusMsg.state.text;

			if (pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
				pGroupIdBuffer			= &pSrcMsg->statusMsg.groupId;

            if( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA ) 
                pPermDataBuffer          = &pSrcMsg->statusMsg.permData;

            if( ( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) )
                 pKey = &pSrcMsg->msgBase.msgKey;

			if( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->statusMsg.extendedHeader;

            if( ( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) )
                 pReqKey = &pSrcMsg->statusMsg.reqMsgKey;
			break;

		case RSSL_MC_CLOSE: 
			if( pSrcMsg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->closeMsg.extendedHeader;
			break; 

        default:
            _RSSL_ASSERT(0, Unexpected msgClass);
            return 0;
    }

    if( pKey )
    {
        if( pKey->flags & RSSL_MKF_HAS_NAME ) 
            pKeyNameBuffer        =  &pKey->name;

        if( pKey->flags & RSSL_MKF_HAS_ATTRIB ) 
            pKeyOpaqueBuffer        =  &pKey->encAttrib;
    }

    if( pReqKey )
    {
        if( pReqKey->flags & RSSL_MKF_HAS_NAME ) 
            pReqKeyNameBuffer        =  &pReqKey->name;

        if( pReqKey->flags & RSSL_MKF_HAS_ATTRIB ) 
            pReqKeyAttribBuffer        =  &pReqKey->encAttrib;
    }

    if( pStateTextBuffer && (copyMsgFlags & RSSL_CMF_STATE_TEXT) )
    {
        mallocSize += stateTextLength = ( pStateTextBuffer->length  ? 
                                          pStateTextBuffer->length : 
                                         ( pStateTextBuffer->data ? 
                                             (RsslUInt32)(strlen(pStateTextBuffer->data) + 1): 0 ) );
    }

    if( pPermDataBuffer && (copyMsgFlags & RSSL_CMF_PERM_DATA) )
        mallocSize += pPermDataBuffer->length;

	if( pGroupIdBuffer && (copyMsgFlags & RSSL_CMF_GROUP_ID) )
        mallocSize += pGroupIdBuffer->length;

    if( pKeyNameBuffer && (copyMsgFlags & RSSL_CMF_KEY_NAME))
	{
        mallocSize += keyNameLength = ( pKeyNameBuffer->length  ? 
                                          pKeyNameBuffer->length : 
                                         ( pKeyNameBuffer->data ? 
                                             (RsslUInt32)(strlen(pKeyNameBuffer->data) + 1): 0 ) );
	}

    if( pReqKeyNameBuffer && (copyMsgFlags & RSSL_CMF_REQ_KEY_NAME))
	{
        mallocSize += reqKeyNameLength = ( pReqKeyNameBuffer->length  ? 
                                          pReqKeyNameBuffer->length : 
                                         ( pReqKeyNameBuffer->data ? 
                                             (RsslUInt32)(strlen(pReqKeyNameBuffer->data) + 1): 0 ) );
	}

	if (pNakTextBuffer && (copyMsgFlags & RSSL_CMF_NAK_TEXT))
		mallocSize += pNakTextBuffer->length;
       
    if( pKeyOpaqueBuffer && ( copyMsgFlags & RSSL_CMF_KEY_ATTRIB )   )
        mallocSize += pKeyOpaqueBuffer->length;

    if( pReqKeyAttribBuffer && ( copyMsgFlags & RSSL_CMF_REQ_KEY_ATTRIB )   )
        mallocSize += pReqKeyAttribBuffer->length;

    if( pExtendedHeaderBuffer  && (copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
        mallocSize += pExtendedHeaderBuffer->length;

    if(  pSrcMsg->msgBase.encDataBody.length && (copyMsgFlags & RSSL_CMF_DATA_BODY))
    {
            mallocSize += pSrcMsg->msgBase.encDataBody.length;
    }

    if(  pSrcMsg->msgBase.encMsgBuffer.length && (copyMsgFlags & RSSL_CMF_MSG_BUFFER))
        mallocSize += pSrcMsg->msgBase.encMsgBuffer.length;
    
	return mallocSize;
}

RTR_C_ALWAYS_INLINE void _rsslMsgMemberCopyInPlace(RsslBuffer *dstBuf, const RsslBuffer* srcBuf, char *encDstMsg, char *encSrcMsg)
{
	if (srcBuf->data)
	{
		dstBuf->data = encDstMsg + (srcBuf->data - encSrcMsg);
	}
}

RsslMsg *rsslCopyMsgInLine(const RsslMsg *pSrcMsg, RsslBuffer *pCopyBuffer)
{
	RsslMsg *retmsg =  (RsslMsg*)pCopyBuffer->data;
	char *copyIter = pCopyBuffer->data;
	RsslUInt32 mask = 0;
	char *encMsgBodyData = pSrcMsg->msgBase.encMsgBuffer.data;
	char *copyEncMsg;

	switch (pSrcMsg->msgBase.msgClass)
	{
		case RSSL_MC_UPDATE:
		{
			RsslUpdateMsg *updateMsg = (RsslUpdateMsg *)pCopyBuffer->data; /* Target message */

			*updateMsg = (*pSrcMsg).updateMsg; /* Do the soft copy */
			copyIter += sizeof(RsslUpdateMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			updateMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&updateMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->updateMsg.flags;
			if (mask & RSSL_UPMF_HAS_MSG_KEY)
			{
				if (pSrcMsg->updateMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&updateMsg->msgBase.msgKey.name, &pSrcMsg->updateMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->updateMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&updateMsg->msgBase.msgKey.encAttrib, &pSrcMsg->updateMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_UPMF_HAS_PERM_DATA)
			{
				_rsslMsgMemberCopyInPlace(&updateMsg->permData, &pSrcMsg->updateMsg.permData, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_UPMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&updateMsg->extendedHeader, &pSrcMsg->updateMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}

			pCopyBuffer->length = sizeof(RsslUpdateMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_GENERIC:
		{
			RsslGenericMsg *genericMsg = (RsslGenericMsg *)pCopyBuffer->data; /* Target message */

			*genericMsg = (*pSrcMsg).genericMsg; /* Do the soft copy */
			copyIter += sizeof(RsslGenericMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			genericMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&genericMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->genericMsg.flags;
			if (mask & RSSL_GNMF_HAS_MSG_KEY)
			{
				if (pSrcMsg->genericMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&genericMsg->msgBase.msgKey.name, &pSrcMsg->genericMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->genericMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&genericMsg->msgBase.msgKey.encAttrib, &pSrcMsg->genericMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_GNMF_HAS_REQ_MSG_KEY)
			{
				if (pSrcMsg->genericMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&genericMsg->reqMsgKey.name, &pSrcMsg->genericMsg.reqMsgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->genericMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&genericMsg->reqMsgKey.encAttrib, &pSrcMsg->genericMsg.reqMsgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_GNMF_HAS_PERM_DATA)
			{
				_rsslMsgMemberCopyInPlace(&genericMsg->permData, &pSrcMsg->genericMsg.permData, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_GNMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&genericMsg->extendedHeader, &pSrcMsg->genericMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}

			pCopyBuffer->length = sizeof(RsslGenericMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_REFRESH:
		{
			RsslRefreshMsg *refreshMsg = (RsslRefreshMsg *)pCopyBuffer->data; /* Target message */

			*refreshMsg = (*pSrcMsg).refreshMsg; /* Do the soft copy */
			copyIter += sizeof(RsslRefreshMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			refreshMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&refreshMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->refreshMsg.flags;
			if (mask & RSSL_RFMF_HAS_MSG_KEY)
			{
				if (pSrcMsg->refreshMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&refreshMsg->msgBase.msgKey.name, &pSrcMsg->refreshMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->refreshMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&refreshMsg->msgBase.msgKey.encAttrib, &pSrcMsg->refreshMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_RFMF_HAS_REQ_MSG_KEY)
			{
				if (pSrcMsg->refreshMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&refreshMsg->reqMsgKey.name, &pSrcMsg->refreshMsg.reqMsgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->refreshMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&refreshMsg->reqMsgKey.encAttrib, &pSrcMsg->refreshMsg.reqMsgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_RFMF_HAS_PERM_DATA)
			{
				_rsslMsgMemberCopyInPlace(&refreshMsg->permData, &pSrcMsg->refreshMsg.permData, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_RFMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&refreshMsg->extendedHeader, &pSrcMsg->refreshMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}
			_rsslMsgMemberCopyInPlace(&refreshMsg->state.text, &pSrcMsg->refreshMsg.state.text, copyEncMsg, encMsgBodyData);
			_rsslMsgMemberCopyInPlace(&refreshMsg->groupId, &pSrcMsg->refreshMsg.groupId, copyEncMsg, encMsgBodyData);

			pCopyBuffer->length = sizeof(RsslRefreshMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_STATUS:
		{
			RsslStatusMsg *statusMsg = (RsslStatusMsg *)pCopyBuffer->data; /* Target message */

			*statusMsg = (*pSrcMsg).statusMsg; /* Do the soft copy */
			copyIter += sizeof(RsslStatusMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			statusMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&statusMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->statusMsg.flags;
			if (mask & RSSL_STMF_HAS_MSG_KEY)
			{
				if (pSrcMsg->statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&statusMsg->msgBase.msgKey.name, &pSrcMsg->statusMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->statusMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&statusMsg->msgBase.msgKey.encAttrib, &pSrcMsg->statusMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_STMF_HAS_REQ_MSG_KEY)
			{
				if (pSrcMsg->statusMsg.reqMsgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&statusMsg->reqMsgKey.name, &pSrcMsg->statusMsg.reqMsgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->statusMsg.reqMsgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&statusMsg->reqMsgKey.encAttrib, &pSrcMsg->statusMsg.reqMsgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_STMF_HAS_PERM_DATA)
			{
				_rsslMsgMemberCopyInPlace(&statusMsg->permData, &pSrcMsg->statusMsg.permData, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_STMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&statusMsg->extendedHeader, &pSrcMsg->statusMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_STMF_HAS_STATE)
			{
				_rsslMsgMemberCopyInPlace(&statusMsg->state.text, &pSrcMsg->statusMsg.state.text, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_STMF_HAS_GROUP_ID)
			{
				_rsslMsgMemberCopyInPlace(&statusMsg->groupId, &pSrcMsg->statusMsg.groupId, copyEncMsg, encMsgBodyData);
			}
			pCopyBuffer->length = sizeof(RsslStatusMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_POST:
		{
			RsslPostMsg *postMsg = (RsslPostMsg *)pCopyBuffer->data; /* Target message */

			*postMsg = (*pSrcMsg).postMsg; /* Do the soft copy */
			copyIter += sizeof(RsslPostMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			postMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&postMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->postMsg.flags;
			if (mask & RSSL_PSMF_HAS_MSG_KEY)
			{
				if (pSrcMsg->postMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&postMsg->msgBase.msgKey.name, &pSrcMsg->postMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->postMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&postMsg->msgBase.msgKey.encAttrib, &pSrcMsg->postMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_PSMF_HAS_PERM_DATA)
			{
				_rsslMsgMemberCopyInPlace(&postMsg->permData, &pSrcMsg->postMsg.permData, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_PSMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&postMsg->extendedHeader, &pSrcMsg->postMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}

			pCopyBuffer->length = sizeof(RsslPostMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_REQUEST:
		{
			RsslRequestMsg *requestMsg = (RsslRequestMsg *)pCopyBuffer->data; /* Target message */

			*requestMsg = (*pSrcMsg).requestMsg; /* Do the soft copy */
			copyIter += sizeof(RsslRequestMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			requestMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&requestMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->requestMsg.flags;
			if (pSrcMsg->requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
			{
				_rsslMsgMemberCopyInPlace(&requestMsg->msgBase.msgKey.name, &pSrcMsg->requestMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
			}
			if (pSrcMsg->requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
			{
				_rsslMsgMemberCopyInPlace(&requestMsg->msgBase.msgKey.encAttrib, &pSrcMsg->requestMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_RQMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&requestMsg->extendedHeader, &pSrcMsg->requestMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}

			pCopyBuffer->length = sizeof(RsslRequestMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_CLOSE:
		{
			RsslCloseMsg *closeMsg = (RsslCloseMsg *)pCopyBuffer->data; /* Target message */

			*closeMsg = (*pSrcMsg).closeMsg; /* Do the soft copy */
			copyIter += sizeof(RsslCloseMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			closeMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&closeMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->closeMsg.flags;
			if (mask & RSSL_CLMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&closeMsg->extendedHeader, &pSrcMsg->closeMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}

			pCopyBuffer->length = sizeof(RsslCloseMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		case RSSL_MC_ACK:
		{
			RsslAckMsg *ackMsg = (RsslAckMsg *)pCopyBuffer->data; /* Target message */

			*ackMsg = (*pSrcMsg).ackMsg; /* Do the soft copy */
			copyIter += sizeof(RsslAckMsg);

			MemCopyByInt(copyIter, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);

			// Remap pointers in all RsslBuffer elements
			// It should be sufficient to check if data != 0, but ....
			// Length is alredy set, no need to remap

			copyEncMsg = copyIter;
			ackMsg->msgBase.encMsgBuffer.data = copyIter;

			_rsslMsgMemberCopyInPlace(&ackMsg->msgBase.encDataBody, &pSrcMsg->msgBase.encDataBody, copyEncMsg, encMsgBodyData);

			mask = pSrcMsg->ackMsg.flags;
			if (mask & RSSL_AKMF_HAS_MSG_KEY)
			{
				if (pSrcMsg->ackMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_NAME) 
				{
					_rsslMsgMemberCopyInPlace(&ackMsg->msgBase.msgKey.name, &pSrcMsg->ackMsg.msgBase.msgKey.name, copyEncMsg, encMsgBodyData);
				}
				if (pSrcMsg->ackMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB) 
				{
					_rsslMsgMemberCopyInPlace(&ackMsg->msgBase.msgKey.encAttrib, &pSrcMsg->ackMsg.msgBase.msgKey.encAttrib, copyEncMsg, encMsgBodyData);
				}
			}
			if (mask & RSSL_AKMF_HAS_EXTENDED_HEADER)
			{
				_rsslMsgMemberCopyInPlace(&ackMsg->extendedHeader, &pSrcMsg->ackMsg.extendedHeader, copyEncMsg, encMsgBodyData);
			}
			if (mask & RSSL_AKMF_HAS_TEXT)
			{
				_rsslMsgMemberCopyInPlace(&ackMsg->text, &pSrcMsg->ackMsg.text, copyEncMsg, encMsgBodyData);
			}

			pCopyBuffer->length = sizeof(RsslAckMsg) + pSrcMsg->msgBase.encMsgBuffer.length;
			break;
		}
		default:
		{
			retmsg = NULL;
			break;
		}
	}

	return retmsg;
}

RSSL_API RsslMsg* rsslCopyMsg(const RsslMsg         *pSrcMsg,  
                      RsslUInt32          copyMsgFlags,
                      RsslUInt32          filterBlocks,
					  RsslBuffer		  *pCopyBuffer )
{
    size_t mallocSize = __RSZRSSLMSG + __RSZUI64;

    void   *mallocBuffer        = 0;
    char   *mallocBufferPos     = 0;
	char   *mallocBufferStartPos = 0;
    const  RsslBuffer   *pStateTextBuffer    = 0;
    RsslUInt32			stateTextLength      = 0;

    const  RsslMsgKey   *pKey                = 0;
    const  RsslMsgKey   *pReqKey             = 0;
    const  RsslBuffer   *pPermDataBuffer      = 0;
	const  RsslBuffer   *pKeyNameBuffer      = 0;
	const  RsslBuffer   *pReqKeyNameBuffer      = 0;
	const  RsslBuffer	*pGroupIdBuffer		 = 0;
	RsslUInt32			keyNameLength		 = 0;
	RsslUInt32			reqKeyNameLength	 = 0;
	const  RsslBuffer   *pKeyOpaqueBuffer    = 0;
	const  RsslBuffer   *pReqKeyAttribBuffer    = 0;
    const  RsslBuffer   *pExtendedHeaderBuffer = 0;
	const  RsslBuffer	*pNakTextBuffer	 = 0;
	RsslUInt32			copiedSize	= 0;

    RsslBuffer          filteredDataBody            = RSSL_INIT_BUFFER;

	int					filteredAllDataBody		= 0;

    _RSSL_ASSERT( rsslValidateMsg( pSrcMsg ), Invalid message contents );

	if (pCopyBuffer && (pSrcMsg->msgBase.encMsgBuffer.length != 0) &&
		(copyMsgFlags == RSSL_CMF_ALL_FLAGS) &&
		(filterBlocks == 0) && rsslValidateEncodedMsgBuf( pSrcMsg ))
	{
		return rsslCopyMsgInLine(pSrcMsg, pCopyBuffer);
	}

    switch( pSrcMsg->msgBase.msgClass )
    {
       case RSSL_MC_UPDATE: 
            if( pSrcMsg->updateMsg.flags & RSSL_UPMF_HAS_PERM_DATA ) 
                pPermDataBuffer          = &pSrcMsg->updateMsg.permData;

			if( ( pSrcMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;

            if( pSrcMsg->updateMsg.flags & RSSL_UPMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->updateMsg.extendedHeader;
			break; 

		case RSSL_MC_GENERIC: 
            if( pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_PERM_DATA ) 
                pPermDataBuffer          = &pSrcMsg->genericMsg.permData;

			if( ( pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;

			if( ( pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY ) )
                pReqKey = &pSrcMsg->genericMsg.reqMsgKey;

            if( pSrcMsg->genericMsg.flags & RSSL_GNMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->genericMsg.extendedHeader;
			break; 

		case RSSL_MC_REFRESH:			
            pStateTextBuffer        = &pSrcMsg->refreshMsg.state.text; 

            if( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_PERM_DATA ) 
                pPermDataBuffer          = &pSrcMsg->refreshMsg.permData;

			pGroupIdBuffer		    = &pSrcMsg->refreshMsg.groupId;
            
            if( ( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;

            if( ( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY ) )
                pReqKey = &pSrcMsg->refreshMsg.reqMsgKey;

            if( pSrcMsg->refreshMsg.flags & RSSL_RFMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->refreshMsg.extendedHeader;
			break; 

		case RSSL_MC_POST:			
			if( pSrcMsg->postMsg.flags & RSSL_PSMF_HAS_PERM_DATA ) 
                pPermDataBuffer = &pSrcMsg->postMsg.permData;           
                      
            if( ( pSrcMsg->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;

            if( pSrcMsg->postMsg.flags & RSSL_PSMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->postMsg.extendedHeader;
			break; 

		case RSSL_MC_REQUEST: 
            pKey = &pSrcMsg->msgBase.msgKey;

            if( pSrcMsg->requestMsg.flags & RSSL_RQMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->requestMsg.extendedHeader;
			break;

		case RSSL_MC_STATUS: 
            if( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_STATE )
                pStateTextBuffer        = &pSrcMsg->statusMsg.state.text;

			if (pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID)
				pGroupIdBuffer			= &pSrcMsg->statusMsg.groupId;

            if( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_PERM_DATA ) 
                pPermDataBuffer          = &pSrcMsg->statusMsg.permData;

            if( ( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) )
                 pKey = &pSrcMsg->msgBase.msgKey;

            if( ( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY ) )
                 pReqKey = &pSrcMsg->statusMsg.reqMsgKey;

            if( pSrcMsg->statusMsg.flags & RSSL_STMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->statusMsg.extendedHeader;
            break;

		case RSSL_MC_CLOSE: 
            if( pSrcMsg->closeMsg.flags & RSSL_CLMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->closeMsg.extendedHeader;
			break; 

		case RSSL_MC_ACK: 
            if( pSrcMsg->ackMsg.flags & RSSL_AKMF_HAS_EXTENDED_HEADER )
                pExtendedHeaderBuffer     = &pSrcMsg->ackMsg.extendedHeader;

			if( pSrcMsg->ackMsg.flags & RSSL_AKMF_HAS_TEXT)
				pNakTextBuffer	= &pSrcMsg->ackMsg.text;

            if( ( pSrcMsg->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY ) )
                pKey = &pSrcMsg->msgBase.msgKey;
			break; 

        default:
            _RSSL_ASSERT(0, Unexpected msgClass);
            return 0;
    }

    if( pKey )
    {
        if( pKey->flags & RSSL_MKF_HAS_NAME ) 
		{
            pKeyNameBuffer        =  &pKey->name;
            if (pKeyNameBuffer && ( copyMsgFlags & RSSL_CMF_KEY_NAME ))
                 keyNameLength = (pKeyNameBuffer->length ?
					pKeyNameBuffer->length : 
					( pKeyNameBuffer->data ?
					(RsslUInt32)(strlen(pKeyNameBuffer->data) + 1) : 0 ));
		}
		if( pKey->flags & RSSL_MKF_HAS_ATTRIB ) 
			pKeyOpaqueBuffer        =  &pKey->encAttrib;
    }

    if( pReqKey )
    {
        if( pReqKey->flags & RSSL_MKF_HAS_NAME ) 
		{
            pReqKeyNameBuffer        =  &pReqKey->name;
            if (pReqKeyNameBuffer && ( copyMsgFlags & RSSL_CMF_REQ_KEY_NAME ))
                 reqKeyNameLength = (pReqKeyNameBuffer->length ?
					pReqKeyNameBuffer->length : 
					( pReqKeyNameBuffer->data ?
					(RsslUInt32)(strlen(pReqKeyNameBuffer->data) + 1) : 0 ));
		}
		if( pReqKey->flags & RSSL_MKF_HAS_ATTRIB ) 
			pReqKeyAttribBuffer        =  &pReqKey->encAttrib;
    }

	if (pStateTextBuffer && (copyMsgFlags & RSSL_CMF_STATE_TEXT))
	{
		stateTextLength = (pStateTextBuffer->length ?
							pStateTextBuffer->length : 
							(pStateTextBuffer->data ? 
								(RsslUInt32)(strlen(pStateTextBuffer->data) + 1) : 0 ) );
	}

	if (!pCopyBuffer)
	{
		if( pStateTextBuffer && ( copyMsgFlags & RSSL_CMF_STATE_TEXT ) )
		{
			mallocSize += stateTextLength;
		}

		if( pPermDataBuffer && ( copyMsgFlags & RSSL_CMF_PERM_DATA ) )
			mallocSize += pPermDataBuffer->length;

		if( pGroupIdBuffer && ( copyMsgFlags & RSSL_CMF_GROUP_ID ) )
			mallocSize += pGroupIdBuffer->length;


		if( pKeyNameBuffer && ( copyMsgFlags & RSSL_CMF_KEY_NAME ) )
		{
			mallocSize += keyNameLength; 
		}

		if( pReqKeyNameBuffer && ( copyMsgFlags & RSSL_CMF_REQ_KEY_NAME ) )
		{
			mallocSize += reqKeyNameLength; 
		}

		if (pNakTextBuffer && (copyMsgFlags & RSSL_CMF_NAK_TEXT) )
			mallocSize += pNakTextBuffer->length;

		if( pKeyOpaqueBuffer && ( copyMsgFlags & RSSL_CMF_KEY_ATTRIB ) )
			mallocSize += pKeyOpaqueBuffer->length;

		if( pReqKeyAttribBuffer && ( copyMsgFlags & RSSL_CMF_REQ_KEY_ATTRIB ) )
			mallocSize += pReqKeyAttribBuffer->length;

		if( pExtendedHeaderBuffer && ( copyMsgFlags & RSSL_CMF_EXTENDED_HEADER ) )
			mallocSize += pExtendedHeaderBuffer->length;

		if( ( copyMsgFlags & RSSL_CMF_DATA_BODY ) && pSrcMsg->msgBase.encDataBody.length )
		{
				mallocSize += pSrcMsg->msgBase.encDataBody.length;
		}

		if( ( copyMsgFlags & RSSL_CMF_MSG_BUFFER ) && pSrcMsg->msgBase.encMsgBuffer.length )
			mallocSize += pSrcMsg->msgBase.encMsgBuffer.length;
	}
    
	if (pCopyBuffer)
	{
		mallocBuffer = pCopyBuffer->data;
	}
	else
	{
		if( ( mallocBuffer = malloc( mallocSize ) ) == NULL )
			return  NULL;

		 *( RsslUInt64 *)mallocBuffer = (RsslUInt64)copyMsgFlags;

	    mallocBuffer = (char *)mallocBuffer + __RSZUI64;
	}

    copiedSize = __RSZRSSLMSG;
    MemCopyByInt( mallocBuffer, pSrcMsg, copiedSize );
	
    mallocBufferPos = ( char *)mallocBuffer + __RSZRSSLMSG;
	mallocBufferStartPos = mallocBufferPos;

	if( pKeyNameBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_KEY_NAME )
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pKeyNameBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + keyNameLength)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pKeyNameBuffer->data, keyNameLength );
            mallocBufferPos += keyNameLength;
			copiedSize += keyNameLength;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pKeyNameBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pKeyNameBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

	if( pReqKeyNameBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_REQ_KEY_NAME )
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pReqKeyNameBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + reqKeyNameLength)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pReqKeyNameBuffer->data, reqKeyNameLength );
            mallocBufferPos += reqKeyNameLength;
			copiedSize += reqKeyNameLength;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pReqKeyNameBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pReqKeyNameBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

  if( pKeyOpaqueBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_KEY_ATTRIB ) 
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pKeyOpaqueBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pKeyOpaqueBuffer->length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pKeyOpaqueBuffer->data, pKeyOpaqueBuffer->length );
            mallocBufferPos += pKeyOpaqueBuffer->length;
			copiedSize += pKeyOpaqueBuffer->length;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pKeyOpaqueBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pKeyOpaqueBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

  if( pReqKeyAttribBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_REQ_KEY_ATTRIB ) 
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pReqKeyAttribBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pReqKeyAttribBuffer->length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pReqKeyAttribBuffer->data, pReqKeyAttribBuffer->length );
            mallocBufferPos += pReqKeyAttribBuffer->length;
			copiedSize += pReqKeyAttribBuffer->length;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pReqKeyAttribBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pReqKeyAttribBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

    if( pExtendedHeaderBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_EXTENDED_HEADER ) 
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pExtendedHeaderBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pExtendedHeaderBuffer->length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pExtendedHeaderBuffer->data, pExtendedHeaderBuffer->length );
            mallocBufferPos += pExtendedHeaderBuffer->length;
			copiedSize += pExtendedHeaderBuffer->length;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pExtendedHeaderBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pExtendedHeaderBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

    if( pSrcMsg->msgBase.encDataBody.length )
    {
        if( ( copyMsgFlags & RSSL_CMF_DATA_BODY  ) && !filteredAllDataBody )
        {
            ( ( RsslMsg *)mallocBuffer )->msgBase.encDataBody.data = mallocBufferPos;
            if( filteredDataBody.length )
            {
                ( ( RsslMsg *)mallocBuffer )->msgBase.encDataBody.length = filteredDataBody.length;
				if (pCopyBuffer && (((mallocBufferPos + filteredDataBody.length)-mallocBufferStartPos) > pCopyBuffer->length))
					return NULL;
                MemCopyByInt( mallocBufferPos, filteredDataBody.data, filteredDataBody.length);
                mallocBufferPos += filteredDataBody.length;
				copiedSize += filteredDataBody.length;
            }
            else
            {
                MemCopyByInt( mallocBufferPos, pSrcMsg->msgBase.encDataBody.data, pSrcMsg->msgBase.encDataBody.length);
                mallocBufferPos += pSrcMsg->msgBase.encDataBody.length;
				copiedSize += pSrcMsg->msgBase.encDataBody.length;
            }
        }
        else
        {
            ( ( RsslMsg *)mallocBuffer )->msgBase.encDataBody.length = 0;
            ( ( RsslMsg *)mallocBuffer )->msgBase.encDataBody.data = 0;
        }
    }

    if( pSrcMsg->msgBase.encMsgBuffer.length )
    {
        if( copyMsgFlags & RSSL_CMF_MSG_BUFFER ) 
        {
            ( ( RsslMsg *)mallocBuffer )->msgBase.encMsgBuffer.data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pSrcMsg->msgBase.encMsgBuffer.length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pSrcMsg->msgBase.encMsgBuffer.data, pSrcMsg->msgBase.encMsgBuffer.length);
            /* Uncomment if anything needs to be copied afterwards
            mallocBufferPos += pDomainHeaderBuffer->length; */
			mallocBufferPos += pSrcMsg->msgBase.encMsgBuffer.length;
			copiedSize += pSrcMsg->msgBase.encMsgBuffer.length;
        }
        else
        {
            ( ( RsslMsg *)mallocBuffer )->msgBase.encMsgBuffer.length = 0;
            ( ( RsslMsg *)mallocBuffer )->msgBase.encMsgBuffer.data   = 0;
        }
    }

    if( pKey )
    {
        RsslMsgKey *pResultKey = (RsslMsgKey *)((char*)mallocBuffer + ( (char*)pKey - (char*)pSrcMsg ) );

          
        if( filterBlocks )
            pResultKey->filter &= filterBlocks;
    }

    if( pStateTextBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_STATE_TEXT )
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pStateTextBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + stateTextLength)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pStateTextBuffer->data, stateTextLength );
            mallocBufferPos += stateTextLength;
			copiedSize += stateTextLength;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pStateTextBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pStateTextBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

    if( pPermDataBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_PERM_DATA )
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pPermDataBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pPermDataBuffer->length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pPermDataBuffer->data, pPermDataBuffer->length );
            mallocBufferPos += pPermDataBuffer->length;
			copiedSize += pPermDataBuffer->length;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pPermDataBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pPermDataBuffer - ( char* )pSrcMsg ) ) )->data = 0;
        }
    }

	if( pGroupIdBuffer )
    {
        if( copyMsgFlags & RSSL_CMF_GROUP_ID )
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pGroupIdBuffer - ( char* )pSrcMsg ) ) )->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pGroupIdBuffer->length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
            MemCopyByInt( mallocBufferPos, pGroupIdBuffer->data, pGroupIdBuffer->length );
            mallocBufferPos += pGroupIdBuffer->length;
			copiedSize += pGroupIdBuffer->length;
        }
        else
        {
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pGroupIdBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pGroupIdBuffer - ( char* )pSrcMsg ) ) )->data = 0;
        }
    }

	if (pNakTextBuffer)
	{
		if (copyMsgFlags & RSSL_CMF_NAK_TEXT)
		{
			((RsslBuffer*)((char*)mallocBuffer + ((char*)pNakTextBuffer - (char*)pSrcMsg)))->data = mallocBufferPos;
			if (pCopyBuffer && (((mallocBufferPos + pNakTextBuffer->length)-mallocBufferStartPos) > pCopyBuffer->length))
				return NULL;
			MemCopyByInt(mallocBufferPos, pNakTextBuffer->data, pNakTextBuffer->length);
			mallocBufferPos += pNakTextBuffer->length;
			copiedSize += pNakTextBuffer->length;
		}
		else
		{
			( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pNakTextBuffer - ( char* )pSrcMsg ) ) )->length = 0;
            ( ( RsslBuffer *)( ( char* )mallocBuffer + ( ( char* )pNakTextBuffer - ( char* )pSrcMsg ) ) )->data   = 0;
        }
    }

	if (pCopyBuffer)
	{
		/* set length properly */
		pCopyBuffer->length = copiedSize;
	}
		
	/* now unset flags for things we didnt copy */
    switch( ((RsslMsg*)mallocBuffer)->msgBase.msgClass )
    {
        case RSSL_MC_UPDATE: 
            if (!(copyMsgFlags & RSSL_CMF_PERM_DATA))
				((RsslMsg*)mallocBuffer)->updateMsg.flags &= ~RSSL_UPMF_HAS_PERM_DATA;

			if(  ((RsslMsg*)mallocBuffer)->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY ) 
            {
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->updateMsg.flags &= ~RSSL_UPMF_HAS_EXTENDED_HEADER;
			break; 
	
		case RSSL_MC_GENERIC: 
            if (!(copyMsgFlags & RSSL_CMF_PERM_DATA))
				((RsslMsg*)mallocBuffer)->genericMsg.flags &= ~RSSL_GNMF_HAS_PERM_DATA;

			if(  ((RsslMsg*)mallocBuffer)->genericMsg.flags & RSSL_GNMF_HAS_MSG_KEY ) 
            {
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

			if(  ((RsslMsg*)mallocBuffer)->genericMsg.flags & RSSL_GNMF_HAS_REQ_MSG_KEY ) 
            {
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->genericMsg.reqMsgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->genericMsg.reqMsgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->genericMsg.flags &= ~RSSL_GNMF_HAS_EXTENDED_HEADER;
			break; 

		case RSSL_MC_REFRESH:			
            if (!(copyMsgFlags & RSSL_CMF_PERM_DATA))
				((RsslMsg*)mallocBuffer)->refreshMsg.flags &= ~RSSL_RFMF_HAS_PERM_DATA;
               
            if(((RsslMsg*)mallocBuffer)->refreshMsg.flags & RSSL_RFMF_HAS_REQ_MSG_KEY )
			{
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->refreshMsg.reqMsgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->refreshMsg.reqMsgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

            if(((RsslMsg*)mallocBuffer)->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY )
			{
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->refreshMsg.flags &= ~RSSL_RFMF_HAS_EXTENDED_HEADER;
			break; 

		case RSSL_MC_POST:			
            if(!(copyMsgFlags & RSSL_CMF_PERM_DATA) )
				((RsslMsg*)mallocBuffer)->postMsg.flags &= ~RSSL_PSMF_HAS_PERM_DATA;
    
            if(((RsslMsg*)mallocBuffer)->postMsg.flags & RSSL_PSMF_HAS_MSG_KEY )
			{
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->postMsg.flags &= ~RSSL_PSMF_HAS_EXTENDED_HEADER;
			break; 

		case RSSL_MC_REQUEST: 
			if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
				((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

			if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
				((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
				
			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->requestMsg.flags &= ~RSSL_RQMF_HAS_EXTENDED_HEADER;
			break;

		case RSSL_MC_STATUS: 
            if(!(copyMsgFlags & RSSL_CMF_PERM_DATA) )
				((RsslMsg*)mallocBuffer)->statusMsg.flags &= ~RSSL_STMF_HAS_PERM_DATA;

			if (!(copyMsgFlags & RSSL_CMF_GROUP_ID))
				((RsslMsg*)mallocBuffer)->statusMsg.flags &= ~RSSL_STMF_HAS_GROUP_ID;

            if(  ((RsslMsg*)mallocBuffer)->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY )
            {
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

            if(  ((RsslMsg*)mallocBuffer)->statusMsg.flags & RSSL_STMF_HAS_REQ_MSG_KEY )
            {
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->statusMsg.reqMsgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->statusMsg.reqMsgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}

			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->statusMsg.flags &= ~RSSL_STMF_HAS_EXTENDED_HEADER;
			break;

		case RSSL_MC_CLOSE: 
			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->closeMsg.flags &= ~RSSL_CLMF_HAS_EXTENDED_HEADER;
			break; 

		case RSSL_MC_ACK: 
			if (!(copyMsgFlags & RSSL_CMF_EXTENDED_HEADER))
				((RsslMsg*)mallocBuffer)->ackMsg.flags &= ~RSSL_AKMF_HAS_EXTENDED_HEADER;

			if (!(copyMsgFlags & RSSL_CMF_NAK_TEXT))
				((RsslMsg*)mallocBuffer)->ackMsg.flags &= ~RSSL_AKMF_HAS_TEXT;

            if(((RsslMsg*)mallocBuffer)->ackMsg.flags & RSSL_AKMF_HAS_MSG_KEY )
			{
				if (!(copyMsgFlags & RSSL_CMF_KEY_ATTRIB))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_ATTRIB;

				if (!(copyMsgFlags & RSSL_CMF_KEY_NAME))
					((RsslMsg*)mallocBuffer)->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
			}
			break; 

		default:
		break;
	}
	
	return (RsslMsg *)( mallocBuffer );
}


RSSL_API void     rsslReleaseCopiedMsg(RsslMsg * pMsg)
{
    _RSSL_ASSERT( rsslValidateMsg( pMsg ), Invalid message contents );

    if( pMsg )
    {
        RsslUInt64 *pBufferStart = (RsslUInt64*)(( char *)pMsg - sizeof( RsslUInt64 ));

        _RSSL_ASSERT( *pBufferStart <= RSSL_CMF_ALL_FLAGS, Invalid buffer - may not be allocated by rsslCopyMsg );

        if( !(*pBufferStart & RSSL_CMF_STATE_TEXT ) )
        {
            RsslState *pState = (RsslState *) rsslGetState( pMsg );
            if( pState && pState->text.data )
                free( pState->text.data );
        }
    
        if( !(*pBufferStart & RSSL_CMF_PERM_DATA ) )
        {
            RsslBuffer *pPermData = (RsslBuffer *) rsslGetPermData( pMsg );
            if( pPermData && pPermData->length && pPermData->data )
                free( pPermData->data );
        }

		if ( !(*pBufferStart & RSSL_CMF_GROUP_ID ) )
		{
			RsslBuffer *pGroupId = (RsslBuffer *) rsslGetGroupId( pMsg );
			if ( pGroupId && pGroupId->length && pGroupId->data )
				free( pGroupId->data );
		}

		if ( !(*pBufferStart & RSSL_CMF_NAK_TEXT) && pMsg->ackMsg.text.data)
			free(pMsg->ackMsg.text.data);
			

        if( ( *pBufferStart & RSSL_CMF_KEY ) != RSSL_CMF_KEY )
        {
            RsslMsgKey *pKey = (RsslMsgKey *)rsslGetMsgKey( pMsg );
            if( pKey )
            {
                if( !( *pBufferStart & RSSL_CMF_KEY_NAME )  && pKey->name.data )
                    free( pKey->name.data );


                if( !( *pBufferStart & RSSL_CMF_KEY_ATTRIB ) && pKey->encAttrib.data )
                      free( pKey->encAttrib.data );
            }
        }

        if( ( *pBufferStart & RSSL_CMF_REQ_KEY ) != RSSL_CMF_REQ_KEY )
        {
            RsslMsgKey *pKey = (RsslMsgKey *)rsslGetReqMsgKey( pMsg );
            if( pKey )
            {
                if( !( *pBufferStart & RSSL_CMF_KEY_NAME )  && pKey->name.data )
                    free( pKey->name.data );


                if( !( *pBufferStart & RSSL_CMF_KEY_ATTRIB ) && pKey->encAttrib.data )
                      free( pKey->encAttrib.data );
            }
        }

        if( !( *pBufferStart & RSSL_CMF_EXTENDED_HEADER ) )
        {
            RsslBuffer *pExtendedHeader = (RsslBuffer *)rsslGetExtendedHeader( pMsg );
            if( pExtendedHeader && pExtendedHeader->data )
                free( pExtendedHeader->data );
        }

        if( !( *pBufferStart & RSSL_CMF_DATA_BODY ) &&  pMsg->msgBase.encDataBody.data )
            free(  pMsg->msgBase.encDataBody.data );

        if( !( *pBufferStart & RSSL_CMF_MSG_BUFFER ) &&  pMsg->msgBase.encMsgBuffer.data )
            free( pMsg->msgBase.encMsgBuffer.data );

        free( pBufferStart );
    }
}

RSSL_API RsslRet rsslAddFilterIdToFilter(const RsslUInt8 FilterId, RsslUInt32 *filter)
{
	RsslUInt32 FilterEntryFilter = 0;

	if (FilterId >= 32)
		return RSSL_RET_INVALID_DATA;

	FilterEntryFilter = 1 << (FilterId & 7);
	*filter |= FilterEntryFilter;	
	return RSSL_RET_SUCCESS;
}

RSSL_API RsslBool rsslCheckFilterForFilterId(const RsslUInt8 FilterId, const RsslUInt32 filter)
{
	RsslUInt32 FilterEntryFilter = 1 << (FilterId & 7);
	return ((filter & FilterEntryFilter) ? 1:0);	
}

RSSL_API RsslRet rsslCompareMsgKeys(const RsslMsgKey *key1, const RsslMsgKey *key2)
{
	/* if we dont have the name type flag set, if they are both set and both name types are the same,
	   or if one is set and the other isnt, but the name type on the one that is set is name type == 1
	   this should pass */
	if ((!(key1->flags & RSSL_MKF_HAS_NAME_TYPE) && !(key2->flags & RSSL_MKF_HAS_NAME_TYPE)) ||
		(((key1->flags & RSSL_MKF_HAS_NAME_TYPE) && (key2->flags & RSSL_MKF_HAS_NAME_TYPE)) && (key1->nameType == key2->nameType)) || 
		(((key1->flags & RSSL_MKF_HAS_NAME_TYPE) && (key1->nameType == 1)) && !(key2->flags & RSSL_MKF_HAS_NAME_TYPE))  || 
		(((key2->flags & RSSL_MKF_HAS_NAME_TYPE) && (key2->nameType == 1)) && !(key1->flags & RSSL_MKF_HAS_NAME_TYPE)))
	{}
	else
		return RSSL_RET_FAILURE;

	if ((key1->flags & RSSL_MKF_HAS_NAME) && (key2->flags & RSSL_MKF_HAS_NAME))
	{
		if (key1->name.length != key2->name.length)
			return RSSL_RET_FAILURE;
	
		if ( MemCmpByInt(key1->name.data, key2->name.data, key1->name.length) != 0)
			return RSSL_RET_FAILURE;
	}
	else if ((key1->flags & RSSL_MKF_HAS_NAME) || (key2->flags & RSSL_MKF_HAS_NAME))
		return RSSL_RET_FAILURE;

	if ((key1->flags & RSSL_MKF_HAS_SERVICE_ID) && (key2->flags & RSSL_MKF_HAS_SERVICE_ID))
	{
		if (key1->serviceId != key2->serviceId)
			return RSSL_RET_FAILURE;
	}
	else if ((key1->flags & RSSL_MKF_HAS_SERVICE_ID) || (key2->flags & RSSL_MKF_HAS_SERVICE_ID))
		return RSSL_RET_FAILURE;

	if ((key1->flags & RSSL_MKF_HAS_FILTER) && (key2->flags & RSSL_MKF_HAS_FILTER))
	{
		if (key1->filter != key2->filter)
			return RSSL_RET_FAILURE;
	}
	else if ((key1->flags & RSSL_MKF_HAS_FILTER) || (key2->flags & RSSL_MKF_HAS_FILTER))
		return RSSL_RET_FAILURE;
		
	if ((key1->flags & RSSL_MKF_HAS_IDENTIFIER) && (key2->flags & RSSL_MKF_HAS_IDENTIFIER))
	{
		if (key1->identifier != key2->identifier)
			return RSSL_RET_FAILURE;
	}
	else if ((key1->flags & RSSL_MKF_HAS_IDENTIFIER) || (key2->flags & RSSL_MKF_HAS_IDENTIFIER))
		return RSSL_RET_FAILURE;

	if ((key1->flags & RSSL_MKF_HAS_ATTRIB) && (key2->flags & RSSL_MKF_HAS_ATTRIB))
	{
		if (key1->attribContainerType != key2->attribContainerType)
			return RSSL_RET_FAILURE;

		if (key1->encAttrib.length != key2->encAttrib.length)
			return RSSL_RET_FAILURE;

		if ( MemCmpByInt(key1->encAttrib.data, key2->encAttrib.data, key1->encAttrib.length) != 0)
			return RSSL_RET_FAILURE;
	}
	else if ((key1->flags & RSSL_MKF_HAS_ATTRIB) || (key2->flags & RSSL_MKF_HAS_ATTRIB))
		return RSSL_RET_FAILURE;

	return RSSL_RET_SUCCESS;
}

RSSL_API RsslRet  rsslCopyMsgKey(RsslMsgKey* destKey, const RsslMsgKey* sourceKey)
{
	_RSSL_ASSERT(destKey, Invalid parameters or parameters passed in as NULL);
	_RSSL_ASSERT(sourceKey, Invalid parameters or parameters passed in as NULL);

	destKey->flags = sourceKey->flags;

	destKey->nameType = sourceKey->nameType;
	if ((sourceKey->flags & RSSL_MKF_HAS_NAME) && (sourceKey->name.length > 0))
	{
		if (destKey->name.length >= sourceKey->name.length)
		{
			if (destKey->name.data)
			{
				destKey->name.length = sourceKey->name.length;
				MemCopyByInt(destKey->name.data, sourceKey->name.data, destKey->name.length);
			}
			else
			{
				destKey->name.length = 0;
				return RSSL_RET_FAILURE;
			}
		}
		else
		{
			destKey->name.length = 0;
			return RSSL_RET_BUFFER_TOO_SMALL;
		}
	}
	else
	{
		destKey->name.length = 0;
	}

	destKey->serviceId = sourceKey->serviceId;

	destKey->filter = sourceKey->filter;

	destKey->identifier = sourceKey->identifier;

	destKey->attribContainerType = sourceKey->attribContainerType;
	if ((sourceKey->flags & RSSL_MKF_HAS_ATTRIB) && (sourceKey->encAttrib.length > 0))
	{
		if (destKey->encAttrib.length >= sourceKey->encAttrib.length)
		{
			if (destKey->encAttrib.data)
			{
				destKey->encAttrib.length = sourceKey->encAttrib.length;
				MemCopyByInt(destKey->encAttrib.data, sourceKey->encAttrib.data, destKey->encAttrib.length);
			}
			else
			{
				destKey->encAttrib.length = 0;
				return RSSL_RET_FAILURE;
			}
		}
		else
		{
			destKey->encAttrib.length = 0;
			return RSSL_RET_BUFFER_TOO_SMALL;
		}
	}
	else
	{
		destKey->encAttrib.length = 0;
	}

	return RSSL_RET_SUCCESS;

}	

RSSL_API RsslUInt32	rsslMsgKeyHash(const RsslMsgKey* pMsgKey)
{
	if (pMsgKey->flags & RSSL_MKF_HAS_NAME)
		return rsslPolyHash(pMsgKey->name.data, pMsgKey->name.length);

	return 0;
}
