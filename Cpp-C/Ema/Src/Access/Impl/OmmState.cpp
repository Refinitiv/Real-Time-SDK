/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmState.h"
#include "OmmStateDecoder.h"
#include "Utilities.h"
#include "ExceptionTranslator.h"
#include <new>

using namespace thomsonreuters::ema::access;

const EmaString OpenString( "Open" );
const EmaString NonStreamingString( "NonStreaming" );
const EmaString ClosedString( "Closed" );
const EmaString ClosedRecoverString( "ClosedRecover" );
const EmaString ClosedRedirectedString( "ClosedRedirected" );
EmaString TempSSString;

const EmaString NoChangeString( "NoChange" );
const EmaString OkString( "Ok" );
const EmaString SuspectString( "Suspect" );
EmaString TempDSString;

const EmaString NoneString( "None" );
const EmaString NotFoundString( "NotFound" );
const EmaString TimeoutString( "Timeout" );
const EmaString NotAuthorizedString( "NotAuthorized" );
const EmaString InvalidArgumentString( "InvalidArgument" );
const EmaString UsageErrorString( "UsageError" );
const EmaString PreemptedString( "Preempted" );
const EmaString JustInTimeConflationStartedString( "JustInTimeConflationStarted" );
const EmaString TickByTickResumedString( "TickByTickResumed" );
const EmaString FailoverStartedString( "FailoverStarted" );
const EmaString FailoverCompletedString( "FailoverCompleted" );
const EmaString GapDetectedString( "GapDetected" );
const EmaString NoResourcesString( "NoResources" );
const EmaString TooManyItemsString( "TooManyItems" );
const EmaString AlreadyOpenString( "AlreadyOpen" );
const EmaString SourceUnknownString( "SourceUnknown" );
const EmaString NotOpenString( "NotOpen" );
const EmaString NonUpdatingItemString( "NonUpdatingItem" );
const EmaString UnsupportedViewTypeString( "UnsupportedViewType" );
const EmaString InvalidViewString( "InvalidView" );
const EmaString FullViewProvidedString( "FullViewProvided" );
const EmaString UnableToRequestAsBatchString( "UnableToRequestAsBatch" );
const EmaString NoBatchViewSupportInReqString( "NoBatchViewSupportInReq" );
const EmaString ExceededMaxMountsPerUserString( "ExceededMaxMountsPerUser" );
const EmaString ErrorString( "Error" );
const EmaString DacsDownString( "DacsDown" );
const EmaString UserUnknownToPermSysString( "UserUnknownToPermSys" );
const EmaString DacsMaxLoginsReachedString( "DacsMaxLoginsReached" );
const EmaString DacsUserAccessToAppDeniedString( "DacsUserAccessToAppDenied" );
const EmaString GapFillString( "GapFill" );
const EmaString AppAuthorizationFailedString( "AppAuthorizationFailed" );
EmaString TempSCString;

OmmState::OmmState() :
 _pDecoder( new ( _space ) OmmStateDecoder() )
{
}

OmmState::~OmmState()
{
	_pDecoder->~OmmStateDecoder();
}

const EmaString& OmmState::getStreamStateAsString() const
{
	switch ( getStreamState() )
	{
	case OpenEnum :
		return OpenString;
	case NonStreamingEnum :
		return NonStreamingString;
	case ClosedEnum :
		return ClosedString;
	case ClosedRecoverEnum :
		return ClosedRecoverString;
	case ClosedRedirectedEnum :
		return ClosedRedirectedString;
	default :
		return TempSSString.set( "Unknown StreamState value " ).append( (Int64)getStreamState() );
	}
}

const EmaString& OmmState::getDataStateAsString() const
{
	switch ( getDataState() )
	{
	case NoChangeEnum :
		return NoChangeString;
	case OkEnum :
		return OkString;
	case SuspectEnum :
		return SuspectString;
	default :
		return TempDSString.set( "Unknown DataState value " ).append( (Int64)getDataState() );
	}
}

const EmaString& OmmState::getStatusCodeAsString() const
{
	switch ( (RsslUInt16)getStatusCode() )
	{
	case NoneEnum :
		return NoneString;
	case NotFoundEnum :
		return NotFoundString;
	case TimeoutEnum :
		return TimeoutString;
	case NotAuthorizedEnum :
		return NotAuthorizedString;
	case InvalidArgumentEnum :
		return InvalidArgumentString;
	case UsageErrorEnum :
		return UsageErrorString;
	case PreemptedEnum :
		return PreemptedString;
	case JustInTimeConflationStartedEnum :
		return JustInTimeConflationStartedString;
	case TickByTickResumedEnum :
		return TickByTickResumedString;
	case FailoverStartedEnum :
		return FailoverStartedString;
	case FailoverCompletedEnum :
		return FailoverCompletedString;
	case GapDetectedEnum :
		return GapDetectedString;
	case NoResourcesEnum :
		return NoResourcesString;
	case TooManyItemsEnum :
		return TooManyItemsString;
	case AlreadyOpenEnum :
		return AlreadyOpenString;
	case SourceUnknownEnum :
		return SourceUnknownString;
	case NotOpenEnum :
		return NotOpenString;
	case NonUpdatingItemEnum :
		return NonUpdatingItemString;
	case UnsupportedViewTypeEnum :
		return UnsupportedViewTypeString;
	case InvalidViewEnum :
		return InvalidViewString;
	case FullViewProvidedEnum :
		return FullViewProvidedString;
	case UnableToRequestAsBatchEnum :
		return UnableToRequestAsBatchString;
	case NoBatchViewSupportInReqEnum :
		return NoBatchViewSupportInReqString;
	case ExceededMaxMountsPerUserEnum :
		return ExceededMaxMountsPerUserString;
	case ErrorEnum :
		return ErrorString;
	case DacsDownEnum :
		return DacsDownString;
	case UserUnknownToPermSysEnum :
		return UserUnknownToPermSysString;
	case DacsMaxLoginsReachedEnum :
		return DacsMaxLoginsReachedString;
	case DacsUserAccessToAppDeniedEnum :
		return DacsUserAccessToAppDeniedString;
	case GapFillEnum:
		return GapFillString;
	case AppAuthorizationFailedEnum:
		return AppAuthorizationFailedString;
	default :
		return TempSCString.set( "Unknown StatusCode value " ).append( (Int64)getStatusCode() );
	}
}

DataType::DataTypeEnum OmmState::getDataType() const
{
	return DataType::StateEnum;
}

Data::DataCode OmmState::getCode() const
{
	return _pDecoder->getCode();
}

OmmState::StreamState OmmState::getStreamState() const
{
	return _pDecoder->getStreamState();
}

OmmState::DataState OmmState::getDataState() const
{
	return _pDecoder->getDataState();
}

UInt8 OmmState::getStatusCode() const
{
	return _pDecoder->getStatusCode();
}

const EmaString& OmmState::getStatusText() const
{
	return _pDecoder->getStatusText();
}

const EmaString& OmmState::toString() const
{
	return _pDecoder->toString();
}

const EmaString& OmmState::toString( UInt64 ) const
{
	return _pDecoder->toString();
}

const EmaBuffer& OmmState::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& OmmState::getDecoder()
{
	return *_pDecoder;
}

bool OmmState::hasDecoder() const
{
	return true;
}

const Encoder& OmmState::getEncoder() const
{
	return *static_cast<const Encoder*>( 0 );
}

bool OmmState::hasEncoder() const
{
	return false;
}
