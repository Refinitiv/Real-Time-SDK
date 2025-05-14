/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;

namespace LSEG.Ema.Access;

/// <summary>
/// OmmState represents State information in Omm.
/// </summary>
/// <remarks>
/// OmmState is used to represent state of item, item group and service.<br/>
/// OmmState encapsulates stream state, data state, status code and status text information.<br/>
/// OmmState is a read only class.<br/>
/// This class is used for extraction of OmmState info only.<br/>
/// All methods in this class are single threaded.<br/>
/// </remarks>
/// <seealso cref="Data"/>
/// <seealso cref="EmaBuffer"/>
public sealed class OmmState : Data
{

    #region Public members

    /// <summary>
    /// StreamState represents item stream state.
    /// </summary>
    public static class StreamStates
    {
        /// <summary>
        /// Indicates the stream is opened and will incur interest after the final refresh.
        /// </summary>
        public const int OPEN = 1;

        /// <summary>
        /// Indicates the item will not incur interest after	the final refresh.
        /// </summary>
        public const int NON_STREAMING = 2;

        /// <summary>
        /// Indicates the stream is closed, typically unobtainable or
        /// identity indeterminable due to a comms outage.<br/>
        /// The item may be available in the future.
        /// </summary>
        public const int CLOSED_RECOVER = 3;

        /// <summary>
        /// Indicates the stream is closed.
        /// </summary>
        public const int CLOSED = 4;

        /// <summary>
        /// Indicates the stream is closed and has been renamed.<br/>
        /// The stream is available with another name.<br/>
        /// This stream state is only valid for refresh messages.<br/>
        /// The new item name is in the Name get accessor methods.
        /// </summary>
        public const int CLOSED_REDIRECTED = 5;
    }

    /// <summary>
    /// DataState represents item data state.
    /// </summary>
    public static class DataStates
    {
        /// <summary>
        /// Indicates the health of the data item did not change.
        /// </summary>
        public const int NO_CHANGE = 0;

        /// <summary>
        /// Indicates the entire data item is healthy.
        /// </summary>
        public const int OK = 1;

        /// <summary>
        /// Indicates the health of some or all of the item's data is stale or unknown.
        /// </summary>
        public const int SUSPECT = 2;
    }

    /// <summary>
    /// StatusCode represents status code.
    /// </summary>
    public static class StatusCodes
    {
        /// <summary> None </summary>
        public const int NONE = 0;

        /// <summary> Not Found </summary>
        public const int NOT_FOUND = 1;

        /// <summary> Timeout </summary>
        public const int TIMEOUT = 2;

        /// <summary> Not Authorized </summary>
        public const int NOT_AUTHORIZED = 3;

        /// <summary> Invalid Argument </summary>
        public const int INVALID_ARGUMENT = 4;

        /// <summary> Usage Error </summary>
        public const int USAGE_ERROR = 5;

        /// <summary> Pre-empted </summary>
        public const int PREEMPTED = 6;

        /// <summary> Just In Time Filtering Started </summary>
        public const int JUST_IN_TIME_CONFLATION_STARTED = 7;

        /// <summary> Tick By Tick Resumed </summary>
        public const int TICK_BY_TICK_RESUMED = 8;

        /// <summary> Fail-over Started </summary>
        public const int FAILOVER_STARTED = 9;

        /// <summary> Fail-over Completed </summary>
        public const int FAILOVER_COMPLETED = 10;

        /// <summary> Gap Detected </summary>
        public const int GAP_DETECTED = 11;

        /// <summary> No Resources </summary>
        public const int NO_RESOURCES = 12;

        /// <summary> Too Many Items </summary>
        public const int TOO_MANY_ITEMS = 13;

        /// <summary> Already Open </summary>
        public const int ALREADY_OPEN = 14;

        /// <summary> Source Unknown </summary>
        public const int SOURCE_UNKNOWN = 15;

        /// <summary> Not Open </summary>
        public const int NOT_OPEN = 16;

        /// <summary> Non Updating Item </summary>
        public const int NON_UPDATING_ITEM = 19;

        /// <summary> Unsupported View Type </summary>
        public const int UNSUPPORTED_VIEW_TYPE = 20;

        /// <summary> Invalid View </summary>
        public const int INVALID_VIEW = 21;

        /// <summary> Full View Provided </summary>
        public const int FULL_VIEW_PROVIDED = 22;

        /// <summary> Unable To Request As Batch </summary>
        public const int UNABLE_TO_REQUEST_AS_BATCH = 23;

        /// <summary> Request does not support batch or view </summary>
        public const int NO_BATCH_VIEW_SUPPORT_IN_REQ = 26;

        /// <summary> Exceeded maximum number of mounts per user </summary>
        public const int EXCEEDED_MAX_MOUNTS_PER_USER = 27;

        /// <summary> Internal error from sender </summary>
        public const int ERROR = 28;

        /// <summary> Connection to DACS down, users are not allowed to connect </summary>
        public const int DACS_DOWN = 29;

        /// <summary> User unknown to permissioning system, it could be DACS, AAA or EED </summary>
        public const int USER_UNKNOWN_TO_PERM_SYS = 30;

        /// <summary> Maximum logins reached </summary>
        public const int DACS_MAX_LOGINS_REACHED = 31;

        /// <summary> User is not allowed to use application </summary>
        public const int DACS_USER_ACCESS_TO_APP_DENIED = 32;

        /// <summary> Content is intended to fill a recognized gap </summary>
        public const int GAP_FILL = 34;

        /// <summary> Application Authorization Failed </summary>
        public const int APP_AUTHORIZATION_FAILED = 35;
    }

    /// <summary>
    /// Gets stream state.
    /// </summary>
    public int StreamState { get => m_State.StreamState(); }

    /// <summary>
    /// Gets data state.
    /// </summary>
    public int DataState { get => m_State.DataState(); }

    /// <summary>
    /// Gets status code.
    /// </summary>
    public int StatusCode { get => m_State.Code(); }

    /// <summary>
    /// Returns the StreamState value as a string format.
    /// </summary>
    /// <returns>string representation of this object's StreamState</returns>
    public string StreamStateAsString()
    {
        return StreamState switch
        {
            StreamStates.OPEN => OPEN_STRING,
            StreamStates.NON_STREAMING => NONSTREAMING_STRING,
            StreamStates.CLOSED => CLOSED_STRING,
            StreamStates.CLOSED_RECOVER => CLOSEDRECOVER_STRING,
            StreamStates.CLOSED_REDIRECTED => CLOSEDREDIRECTED_STRING,
            _ => DEFAULTSS_STRING + StreamState
        };
    }

    /// <summary>
    /// Returns the DataState value as a string format.
    /// </summary>
    /// <returns>string representation of this object's DataState</returns>
    public string DataStateAsString()
    {
        return DataState switch
        {
            DataStates.NO_CHANGE => NOCHANGE_STRING,
            DataStates.OK => OK_STRING,
            DataStates.SUSPECT => SUSPECT_STRING,
            _ => DEFAULTDS_STRING + DataState
        };
    }

    /// <summary>
    /// Returns the StatusCode value as a string format.
    /// </summary>
    /// <returns>string representation of this object's StatusCode</returns>
    public string StatusCodeAsString()
    {
        return StatusCode switch
        {
            StatusCodes.NONE => NONE_STRING,
            StatusCodes.NOT_FOUND => NOTFOUND_STRING,
            StatusCodes.TIMEOUT => TIMEOUT_STRING,
            StatusCodes.NOT_AUTHORIZED => NOTAUTHORIZED_STRING,
            StatusCodes.INVALID_ARGUMENT => INVALIDARGUMENT_STRING,
            StatusCodes.USAGE_ERROR => USAGEERROR_STRING,
            StatusCodes.PREEMPTED => PREEMPTED_STRING,
            StatusCodes.JUST_IN_TIME_CONFLATION_STARTED => JUSTINTIMECONFLATIONSTARTED_STRING,
            StatusCodes.TICK_BY_TICK_RESUMED => TICKBYTICKRESUMED_STRING,
            StatusCodes.FAILOVER_STARTED => FAILOVERSTARTED_STRING,
            StatusCodes.FAILOVER_COMPLETED => FAILOVERCOMPLETED_STRING,
            StatusCodes.GAP_DETECTED => GAPDETECTED_STRING,
            StatusCodes.NO_RESOURCES => NORESOURCES_STRING,
            StatusCodes.TOO_MANY_ITEMS => TOOMANYITEMS_STRING,
            StatusCodes.ALREADY_OPEN => ALREADYOPEN_STRING,
            StatusCodes.SOURCE_UNKNOWN => SOURCEUNKNOWN_STRING,
            StatusCodes.NOT_OPEN => NOTOPEN_STRING,
            StatusCodes.NON_UPDATING_ITEM => NONUPDATINGITEM_STRING,
            StatusCodes.UNSUPPORTED_VIEW_TYPE => UNSUPPORTEDVIEWTYPE_STRING,
            StatusCodes.INVALID_VIEW => INVALIDVIEW_STRING,
            StatusCodes.FULL_VIEW_PROVIDED => FULLVIEWPROVIDED_STRING,
            StatusCodes.UNABLE_TO_REQUEST_AS_BATCH => UNABLETOREQUESTASBATCH_STRING,
            StatusCodes.NO_BATCH_VIEW_SUPPORT_IN_REQ => NOBATCHVIEWSUPPORTINREQ_STRING,
            StatusCodes.EXCEEDED_MAX_MOUNTS_PER_USER => EXCEEDEDMAXMOUNTSPERUSER_STRING,
            StatusCodes.ERROR => ERROR_STRING,
            StatusCodes.DACS_DOWN => DACSDOWN_STRING,
            StatusCodes.USER_UNKNOWN_TO_PERM_SYS => USERUNKNOWNTOPERMSYS_STRING,
            StatusCodes.DACS_MAX_LOGINS_REACHED => DACSMAXLOGINSREACHED_STRING,
            StatusCodes.DACS_USER_ACCESS_TO_APP_DENIED => DACSUSERACCESSTOAPPDENIED_STRING,
            StatusCodes.GAP_FILL => GAPFILL_STRING,
            StatusCodes.APP_AUTHORIZATION_FAILED => APPAUTHORIZATIONFAILED_STRING,
            _ => DEFAULTSC_STRING + StatusCode
        };
    }

    /// <summary>
    /// Gets status text.
    /// </summary>
    public string StatusText
    {
        get
        {
            if (m_State.Text().Length == 0)
                return string.Empty;
            else
                return m_State.Text().ToString();
        }
    }

    /// <summary>
    /// Provides string representation of the current instance
    /// </summary>
    /// <returns>string representing current <see cref="OmmState"/> object.</returns>
    public override string ToString()
    {
        if (DataCode.BLANK == Code)
            return BLANK_STRING;
        else
        {
            m_ToString.Clear();
            m_ToString.Append(Eta.Codec.StreamStates.Info(m_State.StreamState()))
                .Append(" / ")
                .Append(Eta.Codec.DataStates.Info(m_State.DataState()))
                .Append(" / ")
                .Append(Eta.Codec.StateCodes.Info(m_State.Code()))
                .Append(" / '")
                .Append(StatusText)
                .Append("'");

            return m_ToString.ToString();
        }
    }

    #endregion

    #region Implementation details

    internal OmmState() 
    { 
        ReturnToPoolInternal = () => m_objectManager.ReturnToPool(this);
        DecodePrimitiveType = DecodeOmmState;
        m_dataType = Access.DataType.DataTypes.STATE;
    }

    internal CodecReturnCode DecodeOmmState(DecodeIterator dIter)
    {

        if (CodecReturnCode.SUCCESS == m_State.Decode(dIter))
            Code = DataCode.NO_CODE;
        else
        {
            Code = DataCode.BLANK;
            m_State.Clear();
        }

        return CodecReturnCode.SUCCESS;
    }

    internal void Decode(Eta.Codec.State rsslState)
    {
        if (rsslState != null)
        {
            Code = DataCode.NO_CODE;

            m_State.StreamState(rsslState.StreamState());
            m_State.DataState(rsslState.DataState());
            m_State.Code(rsslState.Code());
            m_State.Text(rsslState.Text());
        }
        else
        {
            Code = DataCode.BLANK;

            m_State.Code(Eta.Codec.StateCodes.NONE);
            m_State.DataState(Eta.Codec.DataStates.OK);
            m_State.StreamState(Eta.Codec.StreamStates.OPEN);
            m_State.Text().Clear();
        }
    }

    internal override string ToString(int indent)
    {
        return ToString();
    }

    /* This is used to override the data state from the ETA State */
    internal void SetDataState(int dataState)
    {
        m_State.DataState(dataState);
    }

    private LSEG.Eta.Codec.State m_State = new();

    internal const string OPEN_STRING = "Open";
    internal const string NONSTREAMING_STRING = "NonStreaming";
    internal const string CLOSED_STRING = "Closed";
    internal const string CLOSEDRECOVER_STRING = "ClosedRecover";
    internal const string CLOSEDREDIRECTED_STRING = "ClosedRedirected";

    internal const string NOCHANGE_STRING = "NoChange";
    internal const string OK_STRING = "Ok";
    internal const string SUSPECT_STRING = "Suspect";

    internal const string NONE_STRING = "None";
    internal const string NOTFOUND_STRING = "NotFound";
    internal const string TIMEOUT_STRING = "Timeout";
    internal const string NOTAUTHORIZED_STRING = "NotAuthorized";
    internal const string INVALIDARGUMENT_STRING = "InvalidArgument";
    internal const string USAGEERROR_STRING = "UsageError";
    internal const string PREEMPTED_STRING = "Preempted";
    internal const string JUSTINTIMECONFLATIONSTARTED_STRING = "JustInTimeConflationStarted";
    internal const string TICKBYTICKRESUMED_STRING = "TickByTickResumed";
    internal const string FAILOVERSTARTED_STRING = "FailoverStarted";
    internal const string FAILOVERCOMPLETED_STRING = "FailoverCompleted";
    internal const string GAPDETECTED_STRING = "GapDetected";
    internal const string NORESOURCES_STRING = "NoResources";
    internal const string TOOMANYITEMS_STRING = "TooManyItems";
    internal const string ALREADYOPEN_STRING = "AlreadyOpen";
    internal const string SOURCEUNKNOWN_STRING = "SourceUnknown";
    internal const string NOTOPEN_STRING = "NotOpen";
    internal const string NONUPDATINGITEM_STRING = "NonUpdatingItem";
    internal const string UNSUPPORTEDVIEWTYPE_STRING = "UnsupportedViewType";
    internal const string INVALIDVIEW_STRING = "InvalidView";
    internal const string FULLVIEWPROVIDED_STRING = "FullViewProvided";
    internal const string UNABLETOREQUESTASBATCH_STRING = "UnableToRequestAsBatch";
    internal const string NOBATCHVIEWSUPPORTINREQ_STRING = "NoBatchViewSupportInReq";
    internal const string EXCEEDEDMAXMOUNTSPERUSER_STRING = "ExceededMaxMountsPerUser";
    internal const string ERROR_STRING = "Error";
    internal const string DACSDOWN_STRING = "DacsDown";
    internal const string USERUNKNOWNTOPERMSYS_STRING = "UserUnknownToPermSys";
    internal const string DACSMAXLOGINSREACHED_STRING = "DacsMaxLoginsReached";
    internal const string DACSUSERACCESSTOAPPDENIED_STRING = "DacsUserAccessToAppDenied";
    internal const string GAPFILL_STRING = "GapFill";
    internal const string APPAUTHORIZATIONFAILED_STRING = "AppAuthorizationFailed";

    private const string DEFAULTSS_STRING = "Unknown StreamState value ";
    private const string DEFAULTDS_STRING = "Unknown DataState value ";
    private const string DEFAULTSC_STRING = "Unknown StatusCode value ";

    #endregion
}
