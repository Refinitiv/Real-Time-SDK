///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


class OmmStateImpl extends DataImpl implements OmmState
{
	private final static String OPEN_STRING = "Open";
	private final static String NONSTREAMING_STRING = "NonStreaming";
	private final static String CLOSED_STRING = "CLOSED";
	private final static String CLOSEDRECOVER_STRING = "CLOSEDRecover";
	private final static String CLOSEDREDIRECTED_STRING = "CLOSEDRedirected";

	private final static String NOCHANGE_STRING = "NoChange";
	private final static String OK_STRING = "Ok";
	private final static String SUSPECT_STRING = "Suspect";

	private final static String NONE_STRING = "None";
	private final static String NOTFOUND_STRING = "NotFound";
	private final static String TIMEOUT_STRING = "Timeout";
	private final static String NOTAUTHORIZED_STRING = "NotAuthorized";
	private final static String INVALIDARGUMENT_STRING = "InvalidArgument";
	private final static String USAGEERROR_STRING = "UsageError";
	private final static String PREEMPTED_STRING = "Preempted";
	private final static String JUSTINTIMECONFLATIONSTARTED_STRING = "JustInTimeConflationStarted";
	private final static String TICKBYTICKRESUMED_STRING = "TickByTickResumed";
	private final static String FAILOVERSTARTED_STRING = "FailoverStarted";
	private final static String FAILOVERCOMPLETED_STRING = "FailoverCompleted";
	private final static String GAPDETECTED_STRING = "GapDetected";
	private final static String NORESOURCES_STRING = "NoResources";
	private final static String TOOMANYITEMS_STRING = "TooManyItems";
	private final static String ALREADYOPEN_STRING = "AlreadyOpen";
	private final static String SOURCEUNKNOWN_STRING = "SourceUnknown";
	private final static String NOTOPEN_STRING = "NotOpen";
	private final static String NONUPDATINGITEM_STRING = "NonUpdatingItem";
	private final static String UNSUPPORTEDVIEWTYPE_STRING = "UnsupportedViewType";
	private final static String INVALIDVIEW_STRING = "InvalidView";
	private final static String FULLVIEWPROVIDED_STRING = "FullViewProvided";
	private final static String UNABLETOREQUESTASBATCH_STRING = "UnableToRequestAsBatch";
	private final static String NOBATCHVIEWSUPPORTINREQ_STRING = "NoBatchViewSupportInReq";
	private final static String EXCEEDEDMAXMOUNTSPERUSER_STRING = "ExceededMaxMountsPerUser";
	private final static String ERROR_STRING = "Error";
	private final static String DACSDOWN_STRING = "DacsDown";
	private final static String USERUNKNOWNTOPERMSYS_STRING = "UserUnknownToPermSys";
	private final static String DACSMAXLOGINSREACHED_STRING = "DacsMaxLoginsReached";
	private final static String DACSUSERACCESSTOAPPDENIED_STRING = "DacsUserAccessToAppDenied";
	private final static String INVALIDFORMEDMSG_STRING = "InvalidFormedMsg";
	private final static String CHANNELUNAVAILABLE_STRING = "ChannelUnavailable";
	private final static String SERVICEUNAVAILABLE_STRING = "ServiceUnavailable";
	private final static String SERVICEDOWN_STRING = "ServiceDown";
	private final static String SERVICENOTACCEPTINGREQUESTS_STRING = "ServiceNotAcceptingRequests";
	private final static String LOGINCLOSED_STRING = "LoginCLOSED";
	private final static String DIRECTORYCLOSED_STRING = "DirectoryCLOSED";
	private final static String ITEMNOTFOUND_STRING = "ItemNotFound";
	private final static String DICTIONARYUNAVAILABLE_STRING = "DictionaryUnavailable";
	private final static String FIELDIDNOTFOUNDDICTIONARYUNAVAILABLE_STRING = "FieldIdNotFoundDictionaryUnavailable";
	private final static String ITEMREQUESTTIMEOUT_STRING = "ItemRequestTimeout";
	
	private final static String DEFAULTSS_STRING = "Unknown StreamState value ";
	private final static String DEFAULTDS_STRING = "Unknown DataState value ";
	private final static String DEFAULTSC_STRING = "Unknown StatusCode value ";
	
	private com.thomsonreuters.upa.codec.State _rsslState = com.thomsonreuters.upa.codec.CodecFactory.createState();
	
	@Override
	public int dataType()
	{
		return DataType.DataTypes.STATE;
	}

	@Override
	public String streamStateAsString()
	{
		switch (streamState())
		{
			case StreamState.OPEN :
				return OPEN_STRING;
			case StreamState.NON_STREAMING :
				return NONSTREAMING_STRING;
			case StreamState.CLOSED :
				return CLOSED_STRING;
			case StreamState.CLOSED_RECOVER :
				return CLOSEDRECOVER_STRING;
			case StreamState.CLOSED_REDIRECTED :
				return CLOSEDREDIRECTED_STRING;
			default :
				return DEFAULTSS_STRING + streamState();
		}
	}

	@Override
	public String dataStateAsString()
	{
		switch (dataState())
		{
			case DataState.NO_CHANGE :
				return NOCHANGE_STRING;
			case DataState.OK :
				return OK_STRING;
			case DataState.SUSPECT :
				return SUSPECT_STRING;
			default :
				return DEFAULTDS_STRING + dataState();
		}
	}

	@Override
	public String statusCodeAsString()
	{
		switch (statusCode())
		{
			case StatusCode.NONE :
				return NONE_STRING;
			case StatusCode.NOT_FOUND :
				return NOTFOUND_STRING;
			case StatusCode.TIMEOUT :
				return TIMEOUT_STRING;
			case StatusCode.NOT_AUTHORIZED :
				return NOTAUTHORIZED_STRING;
			case StatusCode.INVALID_ARGUMENT :
				return INVALIDARGUMENT_STRING;
			case StatusCode.USAGE_ERROR :
				return USAGEERROR_STRING;
			case StatusCode.PREEMPTED :
				return PREEMPTED_STRING;
			case StatusCode.JUST_IN_TIME_CONFLATION_STARTED :
				return JUSTINTIMECONFLATIONSTARTED_STRING;
			case StatusCode.TICK_BY_TICK_RESUMED :
				return TICKBYTICKRESUMED_STRING;
			case StatusCode.FAILOVER_STARTED :
				return FAILOVERSTARTED_STRING;
			case StatusCode.FAILOVER_COMPLETED :
				return FAILOVERCOMPLETED_STRING;
			case StatusCode.GAP_DETECTED :
				return GAPDETECTED_STRING;
			case StatusCode.NO_RESOURCES :
				return NORESOURCES_STRING;
			case StatusCode.TOO_MANY_ITEMS :
				return TOOMANYITEMS_STRING;
			case StatusCode.ALREADY_OPEN :
				return ALREADYOPEN_STRING;
			case StatusCode.SOURCE_UNKNOWN :
				return SOURCEUNKNOWN_STRING;
			case StatusCode.NOT_OPEN :
				return NOTOPEN_STRING;
			case StatusCode.NON_UPDATING_ITEM :
				return NONUPDATINGITEM_STRING;
			case StatusCode.UNSUPPORTED_VIEW_TYPE :
				return UNSUPPORTEDVIEWTYPE_STRING;
			case StatusCode.INVALID_VIEW :
				return INVALIDVIEW_STRING;
			case StatusCode.FULL_VIEW_PROVIDED :
				return FULLVIEWPROVIDED_STRING;
			case StatusCode.UNABLE_TO_REQUEST_AS_BATCH :
				return UNABLETOREQUESTASBATCH_STRING;
			case StatusCode.NO_BATCH_VIEW_SUPPORT_IN_REQ :
				return NOBATCHVIEWSUPPORTINREQ_STRING;
			case StatusCode.EXCEEDED_MAX_MOUNTS_PER_USER :
				return EXCEEDEDMAXMOUNTSPERUSER_STRING;
			case StatusCode.ERROR :
				return ERROR_STRING;
			case StatusCode.DACS_DOWN :
				return DACSDOWN_STRING;
			case StatusCode.USER_UNKNOWN_TO_PERM_SYS :
				return USERUNKNOWNTOPERMSYS_STRING;
			case StatusCode.DACS_MAX_LOGINS_REACHED :
				return DACSMAXLOGINSREACHED_STRING;
			case StatusCode.DACS_USER_ACCESS_TO_APP_DENIED :
				return DACSUSERACCESSTOAPPDENIED_STRING;
			case StatusCode.INVALID_FORMED_MSG :
				return INVALIDFORMEDMSG_STRING;
			case StatusCode.CHANNEL_UNAVAILABLE :
				return CHANNELUNAVAILABLE_STRING;
			case StatusCode.SERVICE_UNAVAILABLE :
				return SERVICEUNAVAILABLE_STRING;
			case StatusCode.SERVICE_DOWN :
				return SERVICEDOWN_STRING;
			case StatusCode.SERVICE_NOT_ACCEPTING_REQUESTS :
				return SERVICENOTACCEPTINGREQUESTS_STRING;
			case StatusCode.LOGIN_CLOSED :
				return LOGINCLOSED_STRING;
			case StatusCode.DIRECTORY_CLOSED :
				return DIRECTORYCLOSED_STRING;
			case StatusCode.ITEM_NOT_FOUND :
				return ITEMNOTFOUND_STRING;
			case StatusCode.DICTIONARY_UNAVAILABLE :
				return DICTIONARYUNAVAILABLE_STRING;
			case StatusCode.FIELD_ID_NOT_FOUND_DICTIONARY_UNAVAILABLE :
				return FIELDIDNOTFOUNDDICTIONARYUNAVAILABLE_STRING;
			case StatusCode.ITEM_REQUEST_TIMEOUT :
				return ITEMREQUESTTIMEOUT_STRING;
			default :
				return DEFAULTSC_STRING + statusCode();
		}
	}

	@Override
	public int streamState()
	{
		return _rsslState.streamState();
	}

	@Override
	public int dataState()
	{
		return _rsslState.dataState();
	}

	@Override
	public int statusCode()
	{
		return _rsslState.code();
	}

	@Override
	public String statusText()
	{
		if (_rsslState.text().length() == 0)
			return DataImpl.EMPTY_STRING;
		else
			return _rsslState.text().toString();
	}

	@Override
	public String toString()
	{
		if (DataCode.BLANK == code())
			return BLANK_STRING;
		else
		{
			_toString.setLength(0);
			_toString.append(com.thomsonreuters.upa.codec.StreamStates.info(_rsslState.streamState()))
			.append(" / ")
			.append(com.thomsonreuters.upa.codec.DataStates.info(_rsslState.dataState()))
			.append(" / ")
			.append(com.thomsonreuters.upa.codec.StateCodes.info(_rsslState.code()))
			.append(" / '")
			.append(statusText())
			.append("'");
			
			return _toString.toString();
		}
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_rsslBuffer = rsslBuffer;

		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS == _rsslState.decode(dIter))
			_dataCode = DataCode.NO_CODE;
		else
		{
			_dataCode = DataCode.BLANK;
			_rsslState.clear();
		}
	}
	
	void decode(com.thomsonreuters.upa.codec.State rsslState)
	{
		if (rsslState != null)
		{
			_dataCode = DataCode.NO_CODE;
			
			_rsslState.streamState(rsslState.streamState());
			_rsslState.dataState(rsslState.dataState());
			_rsslState.code(rsslState.code());
			_rsslState.text(rsslState.text());
		}
		else
		{
			_dataCode = DataCode.BLANK;

			_rsslState.code(com.thomsonreuters.upa.codec.StateCodes.NONE);
			_rsslState.dataState(com.thomsonreuters.upa.codec.DataStates.OK);
			_rsslState.streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
			_rsslState.text().clear();
		}
	}
}