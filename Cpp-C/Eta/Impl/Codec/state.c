/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "rtr/rsslState.h"
#include "rtr/rsslRetCodes.h"
#include "rtr/rsslDataUtils.h"

RSSL_API const char* rsslStreamStateInfo(RsslUInt8 code)
{
	switch (code)
	{
	case RSSL_STREAM_UNSPECIFIED:		return "Unspecified";
	case RSSL_STREAM_OPEN:				return "Open";
	case RSSL_STREAM_NON_STREAMING:		return "Non-streaming";
	case RSSL_STREAM_CLOSED_RECOVER:	return "Closed, Recoverable";
	case RSSL_STREAM_CLOSED:			return "Closed";
	case RSSL_STREAM_REDIRECTED:		return "Redirected";
	default:							return "Unknown Stream State";
	}
}

RSSL_API const char* rsslDataStateInfo(RsslUInt8 code)
{
	switch (code)
	{
	case RSSL_DATA_NO_CHANGE:		return "No Change";
	case RSSL_DATA_OK:				return "Ok";
	case RSSL_DATA_SUSPECT:			return "Suspect";
	default:						return "Unknown Data State";
	}
}

RSSL_API const char* rsslStateCodeInfo(RsslUInt8 code)
{
	switch (code)
	{
	case RSSL_SC_NONE: return "None";
	case RSSL_SC_NOT_FOUND:		return "Not found";
	case RSSL_SC_TIMEOUT: return "Timeout";
	case RSSL_SC_NOT_ENTITLED: return "Not entitled";
	case RSSL_SC_INVALID_ARGUMENT: return "Invalid argument";
	case RSSL_SC_USAGE_ERROR: return "Usage error";
	case RSSL_SC_PREEMPTED: return "Preempted";
	case RSSL_SC_JIT_CONFLATION_STARTED: return "JIT conflation started";
	case RSSL_SC_REALTIME_RESUMED: return "Realtime resumed";
	case RSSL_SC_FAILOVER_STARTED: return "Failover started";
	case RSSL_SC_FAILOVER_COMPLETED: return "Failover completed";
	case RSSL_SC_GAP_DETECTED: return "Gap detected";
	case RSSL_SC_NO_RESOURCES: return "No resources";
	case RSSL_SC_TOO_MANY_ITEMS: return "Too many items";
	case RSSL_SC_ALREADY_OPEN: return "Already open";
	case RSSL_SC_SOURCE_UNKNOWN: return "Source unknown";
	case RSSL_SC_NOT_OPEN: return "Not open";
	case RSSL_SC_NON_UPDATING_ITEM: return "Non-updating item";
	case RSSL_SC_UNSUPPORTED_VIEW_TYPE: return "Unsupported view type";
	case RSSL_SC_INVALID_VIEW: return "Invalid view";
	case RSSL_SC_FULL_VIEW_PROVIDED: return "Full view provided";
	case RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH: return "Unable to request as batch";
	case RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ: return "Batch and/or View not supported on request";
	case RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER: return "Login rejected, exceeded maximum number of mounts per user";
	case RSSL_SC_ERROR: return "Internal error from sender";
	case RSSL_SC_DACS_DOWN: return "A21: Connection to DACS down, users are not allowed to connect";
	case RSSL_SC_USER_UNKNOWN_TO_PERM_SYS: return "User unknown to permissioning system, it could be DACS, AAA or EED";
	case RSSL_SC_DACS_MAX_LOGINS_REACHED: return "Maximum logins reached";
	case RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED: return "Application is denied access to system";
	default:						return "";
	}
}

RSSL_API RsslRet rsslStateToString(RsslBuffer *oBuffer, RsslState *pState)
{
	int length = 0;

	length = snprintf(oBuffer->data, oBuffer->length, "State: %s/%s/%s - text: \"%.*s\"",
		rsslStreamStateInfo(pState->streamState),
		rsslDataStateInfo(pState->dataState),
		rsslStateCodeInfo(pState->code),
		pState->text.length, pState->text.data);
	if (length >= 0 && length < (int)oBuffer->length)
	{
		oBuffer->length = length;
		return RSSL_RET_SUCCESS;
	}
	else
	{
		return RSSL_RET_FAILURE;
	}
}

RSSL_API const char* rsslStreamStateToString(RsslUInt8 code)
{
    switch (code)
    {
    case RSSL_STREAM_UNSPECIFIED:       return "RSSL_STREAM_UNSPECIFIED";
    case RSSL_STREAM_OPEN:              return "RSSL_STREAM_OPEN";
    case RSSL_STREAM_NON_STREAMING:     return "RSSL_STREAM_NON_STREAMING";
    case RSSL_STREAM_CLOSED_RECOVER:    return "RSSL_STREAM_CLOSED_RECOVER";
    case RSSL_STREAM_CLOSED:            return "RSSL_STREAM_CLOSED";
    case RSSL_STREAM_REDIRECTED:        return "RSSL_STREAM_REDIRECTED";
    default:                            return "Unknown Stream State";
    }
}

RSSL_API const char* rsslStreamStateToOmmString(RsslUInt8 code)
{
    switch (code)
    {
    case RSSL_STREAM_UNSPECIFIED:       return RSSL_OMMSTR_STREAM_UNSPECIFIED.data;
    case RSSL_STREAM_OPEN:              return RSSL_OMMSTR_STREAM_OPEN.data;
    case RSSL_STREAM_NON_STREAMING:     return RSSL_OMMSTR_STREAM_NON_STREAMING.data;
    case RSSL_STREAM_CLOSED_RECOVER:    return RSSL_OMMSTR_STREAM_CLOSED_RECOVER.data;
    case RSSL_STREAM_CLOSED:            return RSSL_OMMSTR_STREAM_CLOSED.data;
    case RSSL_STREAM_REDIRECTED:        return RSSL_OMMSTR_STREAM_REDIRECTED.data;
    default:                            return NULL;
    }
}



RSSL_API const char* rsslDataStateToString(RsslUInt8 code)
{
   switch (code)
    {
	case RSSL_DATA_NO_CHANGE:		return "RSSL_DATA_NO_CHANGE";
    case RSSL_DATA_OK:              return "RSSL_DATA_OK";
    case RSSL_DATA_SUSPECT:         return "RSSL_DATA_SUSPECT";
    default:                        return "Unknown Data State";
    }
}

RSSL_API const char* rsslDataStateToOmmString(RsslUInt8 code)
{
   switch (code)
    {
	case RSSL_DATA_NO_CHANGE:		return RSSL_OMMSTR_DATA_NO_CHANGE.data;
    case RSSL_DATA_OK:              return RSSL_OMMSTR_DATA_OK.data;
    case RSSL_DATA_SUSPECT:         return RSSL_OMMSTR_DATA_SUSPECT.data;
    default:                        return NULL;
    }
}

RSSL_API const char* rsslStateCodeToString(RsslUInt8 code)
{
	switch (code)
	{
	case	RSSL_SC_NONE: return "RSSL_SC_NONE";
	case	RSSL_SC_NOT_FOUND: return "RSSL_SC_NOT_FOUND";
	case	RSSL_SC_TIMEOUT: return "RSSL_SC_TIMEOUT";
	case	RSSL_SC_NOT_ENTITLED: return "RSSL_SC_NOT_ENTITLED";
	case	RSSL_SC_INVALID_ARGUMENT: return "RSSL_SC_INVALID_ARGUMENT";
	case	RSSL_SC_USAGE_ERROR: return "RSSL_SC_USAGE_ERROR";
	case	RSSL_SC_PREEMPTED: return "RSSL_SC_PREEMPTED";
	case	RSSL_SC_JIT_CONFLATION_STARTED: return "RSSL_SC_JIT_CONFLATION_STARTED";
	case	RSSL_SC_REALTIME_RESUMED: return "RSSL_SC_REALTIME_RESUMED";
	case	RSSL_SC_FAILOVER_STARTED: return "RSSL_SC_FAILOVER_STARTED";
	case	RSSL_SC_FAILOVER_COMPLETED: return "RSSL_SC_FAILOVER_COMPLETED";
	case	RSSL_SC_GAP_DETECTED: return "RSSL_SC_GAP_DETECTED";
	case	RSSL_SC_NO_RESOURCES: return "RSSL_SC_NO_RESOURCES";
	case	RSSL_SC_TOO_MANY_ITEMS: return "RSSL_SC_TOO_MANY_ITEMS";
	case	RSSL_SC_ALREADY_OPEN:	return "RSSL_SC_ALREADY_OPEN";
	case	RSSL_SC_SOURCE_UNKNOWN: return "RSSL_SC_SOURCE_UNKNOWN";
	case	RSSL_SC_NOT_OPEN: return "RSSL_SC_NOT_OPEN";
	case	RSSL_SC_NON_UPDATING_ITEM: return "RSSL_SC_NON_UPDATING_ITEM";
	case	RSSL_SC_UNSUPPORTED_VIEW_TYPE: return "RSSL_SC_UNSUPPORTED_VIEW_TYPE";
	case	RSSL_SC_INVALID_VIEW: return "RSSL_SC_INVALID_VIEW";
	case	RSSL_SC_FULL_VIEW_PROVIDED: return "RSSL_SC_FULL_VIEW_PROVIDED";
	case	RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH: return "RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH";
	case	RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ: return "RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ";
	case	RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER: return "RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER";
	case	RSSL_SC_ERROR: return "RSSL_SC_ERROR";
	case	RSSL_SC_DACS_DOWN: return "RSSL_SC_DACS_DOWN";
	case	RSSL_SC_USER_UNKNOWN_TO_PERM_SYS: return "RSSL_SC_USER_UNKNOWN_TO_PERM_SYS";
	case	RSSL_SC_DACS_MAX_LOGINS_REACHED: return "RSSL_SC_DACS_MAX_LOGINS_REACHED";
	case	RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED: return "RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED";
	case	RSSL_SC_GAP_FILL: return "RSSL_SC_GAP_FILL";
	case	RSSL_SC_APP_AUTHORIZATION_FAILED: return "RSSL_SC_APP_AUTHORIZATION_FAILED";
    default:                        return "Unknown State Code";
    }
}

RSSL_API const char* rsslStateCodeToOmmString(RsslUInt8 code)
{
	switch (code)
	{
	case	RSSL_SC_NONE: return RSSL_OMMSTR_SC_NONE.data;
	case	RSSL_SC_NOT_FOUND: return RSSL_OMMSTR_SC_NOT_FOUND.data;
	case	RSSL_SC_TIMEOUT: return RSSL_OMMSTR_SC_TIMEOUT.data;
	case	RSSL_SC_NOT_ENTITLED: return RSSL_OMMSTR_SC_NOT_ENTITLED.data;
	case	RSSL_SC_INVALID_ARGUMENT: return RSSL_OMMSTR_SC_INVALID_ARGUMENT.data;
	case	RSSL_SC_USAGE_ERROR: return RSSL_OMMSTR_SC_USAGE_ERROR.data;
	case	RSSL_SC_PREEMPTED: return RSSL_OMMSTR_SC_PREEMPTED.data;
	case	RSSL_SC_JIT_CONFLATION_STARTED: return RSSL_OMMSTR_SC_JIT_CONFLATION_STARTED.data;
	case	RSSL_SC_REALTIME_RESUMED: return RSSL_OMMSTR_SC_REALTIME_RESUMED.data;
	case	RSSL_SC_FAILOVER_STARTED: return RSSL_OMMSTR_SC_FAILOVER_STARTED.data;
	case	RSSL_SC_FAILOVER_COMPLETED: return RSSL_OMMSTR_SC_FAILOVER_COMPLETED.data;
	case	RSSL_SC_GAP_DETECTED: return RSSL_OMMSTR_SC_GAP_DETECTED.data;
	case	RSSL_SC_NO_RESOURCES: return RSSL_OMMSTR_SC_NO_RESOURCES.data;
	case	RSSL_SC_TOO_MANY_ITEMS: return RSSL_OMMSTR_SC_TOO_MANY_ITEMS.data;
	case	RSSL_SC_ALREADY_OPEN:	return RSSL_OMMSTR_SC_ALREADY_OPEN.data;
	case	RSSL_SC_SOURCE_UNKNOWN: return RSSL_OMMSTR_SC_SOURCE_UNKNOWN.data;
	case	RSSL_SC_NOT_OPEN: return RSSL_OMMSTR_SC_NOT_OPEN.data;
	case	RSSL_SC_NON_UPDATING_ITEM: return RSSL_OMMSTR_SC_NON_UPDATING_ITEM.data;
	case	RSSL_SC_UNSUPPORTED_VIEW_TYPE: return RSSL_OMMSTR_SC_UNSUPPORTED_VIEW_TYPE.data;
	case	RSSL_SC_INVALID_VIEW: return RSSL_OMMSTR_SC_INVALID_VIEW.data;
	case	RSSL_SC_FULL_VIEW_PROVIDED: return RSSL_OMMSTR_SC_FULL_VIEW_PROVIDED.data;
	case	RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH: return RSSL_OMMSTR_SC_UNABLE_TO_REQUEST_AS_BATCH.data;
	case	RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ: return RSSL_OMMSTR_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ.data;
	case	RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER: return RSSL_OMMSTR_SC_EXCEEDED_MAX_MOUNTS_PER_USER.data;
	case	RSSL_SC_ERROR: return RSSL_OMMSTR_SC_ERROR.data;
	case	RSSL_SC_DACS_DOWN: return RSSL_OMMSTR_SC_DACS_DOWN.data;
	case	RSSL_SC_USER_UNKNOWN_TO_PERM_SYS: return RSSL_OMMSTR_SC_USER_UNKNOWN_TO_PERM_SYS.data;
	case	RSSL_SC_DACS_MAX_LOGINS_REACHED: return RSSL_OMMSTR_SC_DACS_MAX_LOGINS_REACHED.data;
	case	RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED: return RSSL_OMMSTR_SC_DACS_USER_ACCESS_TO_APP_DENIED.data;
	case	RSSL_SC_GAP_FILL: return RSSL_OMMSTR_SC_GAP_FILL.data;
	case	RSSL_SC_APP_AUTHORIZATION_FAILED: return RSSL_OMMSTR_SC_APP_AUTHORIZATION_FAILED.data;
    default:                        return NULL;
    }
}


RSSL_API const char* rsslStateCodeDescription(RsslUInt8 code){
	switch (code)
	{
	case	RSSL_SC_NONE: return "None";
	case	RSSL_SC_NOT_FOUND: return "Item not found";
	case	RSSL_SC_TIMEOUT: return "A timeout has occurred";
	case	RSSL_SC_NOT_ENTITLED: return "Not entitled";
	case	RSSL_SC_INVALID_ARGUMENT: return "Invalid argument provided";
	case	RSSL_SC_USAGE_ERROR: return "General usage error";
	case	RSSL_SC_PREEMPTED: return "Item has been preempted";
	case	RSSL_SC_JIT_CONFLATION_STARTED: return "Just in time conflation has started";
	case	RSSL_SC_REALTIME_RESUMED: return "Realtime content delivery has resumed";
	case	RSSL_SC_FAILOVER_STARTED: return "Failover has begun";
	case	RSSL_SC_FAILOVER_COMPLETED: return "Failover has been completed";
	case	RSSL_SC_GAP_DETECTED: return "A gap has been detected in the content stream";
	case	RSSL_SC_NO_RESOURCES: return "No resources available to process request";
	case	RSSL_SC_TOO_MANY_ITEMS: return "Too many items are currently open";
	case	RSSL_SC_ALREADY_OPEN:	return "Item is already open";
	case	RSSL_SC_SOURCE_UNKNOWN: return "Source is unknown";
	case	RSSL_SC_NOT_OPEN: return "Item is not open";
	case	RSSL_SC_NON_UPDATING_ITEM: return "Item is non-updating";
	case	RSSL_SC_UNSUPPORTED_VIEW_TYPE: return "Unsupported view type requested";
	case	RSSL_SC_INVALID_VIEW: return "Invalid veiw requested";
	case	RSSL_SC_FULL_VIEW_PROVIDED: return "Full view provided for content";
	case	RSSL_SC_UNABLE_TO_REQUEST_AS_BATCH: return "Unable to request stream with batch information";
	case	RSSL_SC_NO_BATCH_VIEW_SUPPORT_IN_REQ: return "Batch and/or View not supported on request";
	case	RSSL_SC_EXCEEDED_MAX_MOUNTS_PER_USER: return "Login rejected, exceeded maximum number of mounts per user";
	case	RSSL_SC_ERROR: return "Internal error from sender";
	case	RSSL_SC_DACS_DOWN: return "A21: Connection to DACS down, users are not allowed to connect";
	case	RSSL_SC_USER_UNKNOWN_TO_PERM_SYS: return "User unknown to permissioning system, it could be DACS, AAA or EED";
	case	RSSL_SC_DACS_MAX_LOGINS_REACHED: return "Maximum logins reached";
	case	RSSL_SC_DACS_USER_ACCESS_TO_APP_DENIED: return "Application is denied access to system";
	case	RSSL_SC_GAP_FILL: return "This content is intended to fill a recognized gap";
	case	RSSL_SC_APP_AUTHORIZATION_FAILED: return "The signed application was not able to be authorized by the upstream component";
    default:                        return "Unknown State Code";
    }
}
