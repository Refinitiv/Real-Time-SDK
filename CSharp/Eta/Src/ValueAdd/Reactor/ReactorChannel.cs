/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Diagnostics;
using System.Net.Sockets;
using System.Text;

using LSEG.Eta.Common;
using LSEG.Eta.Codec;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Rdm;
using System.Runtime.CompilerServices;

namespace LSEG.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// Channel representing a connection handled by a <see cref="Reactor"/>
    /// </summary>
    sealed public class ReactorChannel : VaNode
    {
        private long m_InitializationTimeout;
        private long m_InitializationEndTimeMs;
        private PingHandler m_PingHandler = new PingHandler();
        private StringBuilder m_StringBuilder = new StringBuilder(100);

        /* Connection recovery information. */
        private const int NO_RECONNECT_LIMIT = -1;
        internal ReactorConnectOptions? ConnectOptions { get; private set; }
        private int m_ReconnectAttempts;
        private int m_ReconnectDelay;
        private int m_ListIndex;
        internal NotifierEvent NotifierEvent { get; private set; }

        private ReactorRole? m_Role;

        internal Watchlist? Watchlist { get; set; }

        /* Link for ReactorChannel queue */
        private ReactorChannel? _reactorChannelNext, _reactorChannelPrev;

        #region Session management
        private ReactorRestConnectOptions? m_RestConnectOptions;
        internal ReactorTokenSession? TokenSession { get; set; }

        internal List<ReactorServiceEndpointInfo> ServiceEndpointInfoList { get; private set; }

        internal LoginRequest? RDMLoginRequestRDP { get; set; }

        internal ReactorErrorInfo ReactorErrorInfo { get; set; } = new ReactorErrorInfo();
        #endregion

        internal TransportReturnCode ReadRet { get; set; } = TransportReturnCode.SUCCESS;

        internal class ReactorChannelLink : LSEG.Eta.ValueAdd.Common.VaDoubleLinkList<ReactorChannel>.ILink<ReactorChannel>
        {
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public ReactorChannel? GetPrev(ReactorChannel thisPrev) { return thisPrev._reactorChannelPrev; }
            
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void SetPrev(ReactorChannel? thisPrev, ReactorChannel? thatPrev) { thisPrev!._reactorChannelPrev = thatPrev; }
            
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public ReactorChannel? GetNext(ReactorChannel thisNext) { return thisNext._reactorChannelNext; }
            
            [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
            public void SetNext(ReactorChannel? thisNext, ReactorChannel? thatNext) { thisNext!._reactorChannelNext = thatNext; }
        }

        /* This is used to indicate the worker thread only whether the closed ack is sent from worker to Reactor */
        internal bool IsClosedAckSent = false;

        internal static readonly ReactorChannelLink REACTOR_CHANNEL_LINK = new();

        /// <summary>
        /// Gets the state of this <see cref="ReactorChannel"/>
        /// </summary>
        /// <value><see cref="State"/></value>
        public ReactorChannelState State { get; internal set; }

        /// <summary>
        /// Gets the <see cref="IChannel"/> associated with this <see cref="ReactorChannel"/>.
        /// </summary>
        /// <value><see cref="IChannel"/></value>
        public IChannel? Channel { get; internal set; }

        /// <summary>
        /// Gets the <see cref="IServer"/> associated with this <see cref="ReactorChannel"/>.
        /// </summary>
        public IServer? Server { get; internal set; }

        /// <summary>
        /// Gets the <see cref="Reactor"/> associated with this <see cref="ReactorChannel"/>.
        /// </summary>
        public Reactor? Reactor { get; internal set; }

        /// <summary>
        /// Gets the <see cref="System.Net.Sockets.Socket"/> associated with this <see cref="ReactorChannel"/>.
        /// </summary>
        public Socket? Socket { get; internal set; }

        /// <summary>
        /// Gets the old <see cref="System.Net.Sockets.Socket"/> associated with this <see cref="ReactorChannel"/>.
        /// </summary>
        public Socket? OldSocket { get; internal set; }

        /// <summary>
        /// Gets the user specified object associated with this ReactorChannel.
        /// </summary>
        public Object? UserSpecObj { get; internal set; }

        /// <summary>
        /// Role associated with this Channel. A deep copy is performed from a new value.
        /// </summary>
        internal ReactorRole? Role
        {
            get => m_Role;
            set
            {
                if (value == null)
                {
                    m_Role = null;
                    return;
                }
                // perform a deep copy *from* the provided value
                switch (value.Type)
                {
                    case ReactorRoleType.CONSUMER:
                        if (m_Role == null || m_Role.Type != ReactorRoleType.CONSUMER)
                        {
                            m_Role = new ConsumerRole();
                        }
                        ((ConsumerRole)m_Role).Copy((ConsumerRole)value);
                        break;
                    case ReactorRoleType.NIPROVIDER:
                        if (m_Role == null || m_Role.Type != ReactorRoleType.NIPROVIDER)
                        {
                            m_Role = new NIProviderRole();
                        }
                        ((NIProviderRole)m_Role).Copy((NIProviderRole)value);
                        break;
                    case ReactorRoleType.PROVIDER:
                        if (m_Role == null || m_Role.Type != ReactorRoleType.PROVIDER)
                        {
                            m_Role = new ProviderRole();
                        }
                        ((ProviderRole)m_Role).Copy((ProviderRole)value);
                        break;
                    default:
                        Debug.Assert(false);  // not supported
                        return;
                }
            }
        }

        /// <summary>
        /// When <see cref="ReactorChannel"/> becomes active for a client or server, this is
        /// populated with the protocolType associated with the content being sent on
        /// this connection. If the protocolType indicated by a server does not match
        /// the protocolType that a client specifies, the connection will be rejected.
        /// </summary>
        /// <remarks>
        /// The transport layer is data neutral and does not change nor depend on any
        /// information in content being distributed. This information is provided to help
        /// client and server applications manage the information they are communicating.
        /// </remarks>
        /// <value><see cref="Transports.ProtocolType"/></value>
        public Transports.ProtocolType ProtocolType
        {
            get
            {
                if (Channel != null)
                {
                    return Channel.ProtocolType;
                }

                return Transports.ProtocolType.RWF;
            }
        }

        /// <summary>
        /// Gets the name of the remote host to which a consumer or niprovider application
        /// is connected.
        /// </summary>
        public String? HostName
        {
            get
            {
                if (Channel is null)
                    return null;

                return Channel.HostName;
            }
        }

        /// <summary>
        /// Gets the port number of the remote host to which a consumer or niprovider application
        /// </summary>
        public int Port
        {
            get
            {
                if(Channel is null)
                    return 0;

                return Channel.Port;
            }
        }

        /// <summary>
        /// Gets the negotiated major version number that is associated with the content being sent
        /// on this connection.
        /// </summary>
        public int MajorVersion
        {
            get
            {
                if (Channel != null)
                    return Channel.MajorVersion;
                else
                    return Codec.Codec.MajorVersion();
            }

        }

        /// <summary>
        /// Gets the negotiated minor version number that is associated with the content being sent
        /// on this connection.
        /// </summary>
        public int MinorVersion
        {
            get
            {
                if (Channel != null)
                    return Channel.MinorVersion;
                else
                    return Codec.Codec.MinorVersion();
            }

        }

        /// <summary>
        /// Closes a reactor channel and removes it from the Reactor. May be called
        /// inside or outside of a callback function, however the channel should no
        /// longer be used afterwards.
        /// </summary>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating success or failure</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode Close(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorReturnCode retVal = ReactorReturnCode.SUCCESS;

            if (Reactor is null)
                return ReactorReturnCode.FAILURE;

            Reactor.ReactorLock.Enter();

            try
            {
                if (Reactor.IsShutdown)
                    retVal = Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                            "ReactorChannel.Close",
                            "Reactor is shutdown, close aborted.");

                if (State != ReactorChannelState.CLOSED)
                    retVal = Reactor.CloseChannel(this, out errorInfo);

                return retVal;
            }
            finally
            {
                Reactor.ReactorLock.Exit();
            }
        }

        /// <summary>
        /// Returns a String representation of this object.
        /// </summary>
        /// <returns>The string value</returns>
        public override string ToString()
        {
            m_StringBuilder.Length = 0;
            m_StringBuilder.AppendLine("ReactorChannel: ");

            if(Role != null)
            {
                m_StringBuilder.AppendLine(Role?.ToString());
            }
            else
            {
                m_StringBuilder.AppendLine("no Role defined.");
            }

            if(Channel != null)
            {
                m_StringBuilder.Append("Channel: ");
                m_StringBuilder.AppendLine(Channel?.ToString());
            }

            if(Server != null)
            {
                m_StringBuilder.Append("Server: ");
                m_StringBuilder.AppendLine(Server?.ToString());
            }

            if(UserSpecObj != null)
            {
                m_StringBuilder.Append("UserSpecObj: ");
                m_StringBuilder.AppendLine(UserSpecObj?.ToString());
            }

            return m_StringBuilder.ToString();
        }

        /// <summary>
        /// Populates information about the <see cref="ReactorChannel"/> into <see cref="ReactorChannelInfo"/>.
        /// </summary>
        /// <param name="info"><see cref="ReactorChannelInfo"/> structure to be populated with information</param>
        /// <param name="errorInfo"><see cref="ValueAdd.Reactor.ReactorErrorInfo"/> is set in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating sucess or failure.</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode Info(ReactorChannelInfo info, out ReactorErrorInfo? errorInfo)
        {
            if (Reactor == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "ReactorChannel.Info",
                    "Reactor is null");
            }
            if (Reactor.IsShutdown)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                        "ReactorChannel.Info",
                        "Reactor is shutdown, info aborted.");
            }

            if (Channel == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "ReactorChannel.Info",
                        "The channel is no longer available.");
            }

            Error transportError;
            if (Channel.Info(info.ChannelInfo, out transportError) != TransportReturnCode.SUCCESS)
            {
                Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "ReactorChannel.Info",
                        transportError.Text);
                errorInfo.Error = transportError;
                return ReactorReturnCode.FAILURE;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }


        /// <summary>
        /// Sends the given TransportBuffer to the channel.
        /// </summary>
        /// <param name="buffer">the buffer to send</param>
        /// <param name="submitOptions">options for how to send the message</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns>
        /// <see cref="ReactorReturnCode.SUCCESS"/> if submit succeeded or
        /// <see cref="ReactorReturnCode.WRITE_CALL_AGAIN"/> if the buffer cannot be written at this time or
        /// <see cref="ReactorReturnCode.FAILURE"/> if submit failed (refer to errorInfo for additional information)
        /// </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode Submit(ITransportBuffer buffer, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            if (Reactor == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "Reactor cannot be null");
            }
            else if (submitOptions == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "submitOptions cannot be null.");
            }

            Reactor.ReactorLock.Enter();

            try
            {

                if (Reactor.IsShutdown)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "ReactorChannel.Submit", "Reactor is shutdown, submit aborted.");
                }
                else if (State == ReactorChannelState.CLOSED)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "ReactorChannel is closed, submit aborted.");
                }

                return Reactor.SubmitChannel(this, buffer, submitOptions, out errorInfo);
            }
            finally
            {
                Reactor.ReactorLock.Exit();
            }
        }

        /// <summary>
        /// Sends a message to the channel.
        /// </summary>
        /// <param name="msg">the Codec <see cref="Msg"/> to send</param>
        /// <param name="submitOptions">options for how to send the message</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if submit succeeded,
        /// or <see cref="ReactorReturnCode.WRITE_CALL_AGAIN"/> if the message cannot be written at this time,
        /// or <see cref="ReactorReturnCode.NO_BUFFERS"/> if there are no more buffers to encode the message into,
        /// or <see cref="ReactorReturnCode.FAILURE"/> if submit failed (refer to errorInfo instance for additional information)</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode Submit(Msg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            if (Reactor == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "Reactor cannot be null");
            }
            else if (submitOptions == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "submitOptions cannot be null.");
            }

            Reactor.ReactorLock.Enter();

            try
            {
                if (Reactor.IsShutdown)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "ReactorChannel.Submit", "Reactor is shutdown, submit aborted.");
                }
                else if (State == ReactorChannelState.CLOSED)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "ReactorChannel is closed, submit aborted.");
                }

                if (Watchlist is null) // watchlist not enabled, submit normally
                {
                    return Reactor.SubmitChannel(this, msg, submitOptions, out errorInfo);
                }
                else // watchlist is enabled, submit via watchlist
                {
                    return Watchlist.SubmitMsg(msg, submitOptions, out errorInfo);
                }
            }
            finally
            {
                Reactor.ReactorLock.Exit();
            }
        }

        /// <summary>
        /// Sends a message to the channel.
        /// </summary>
        /// <param name="msg">the Codec <see cref="IMsg"/> to send</param>
        /// <param name="submitOptions">options for how to send the message</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if submit succeeded,
        /// or <see cref="ReactorReturnCode.WRITE_CALL_AGAIN"/> if the message cannot be written at this time,
        /// or <see cref="ReactorReturnCode.NO_BUFFERS"/> if there are no more buffers to encode the message into,
        /// or <see cref="ReactorReturnCode.FAILURE"/> if submit failed (refer to errorInfo instance for additional information)</returns>
        public ReactorReturnCode Submit(IMsg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            return Submit((Msg)msg, submitOptions, out errorInfo);
        }

        /// <summary>
        /// Sends an RDM message to the channel.
        /// </summary>
        /// <param name="rdmMsg">the RDM message to send</param>
        /// <param name="submitOptions">options for how to send the message</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode.SUCCESS"/> if submit succeeded,
        /// or <see cref="ReactorReturnCode.WRITE_CALL_AGAIN"/> if the message cannot be written at this time,
        /// or <see cref="ReactorReturnCode.NO_BUFFERS"/> if there are no more buffers to encode the message into,
        /// or <see cref="ReactorReturnCode.FAILURE"/> if submit failed (refer to errorInfo instance for additional information)</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ReactorReturnCode Submit(MsgBase rdmMsg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            if (Reactor == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "Reactor cannot be null");
            }
            else if (submitOptions == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "submitOptions cannot be null.");
            }

            Reactor.ReactorLock.Enter();

            try
            {
                if (Reactor.IsShutdown)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "ReactorChannel.Submit", "Reactor is shutdown, submit aborted.");
                }
                else if (State == ReactorChannelState.CLOSED)
                {
                    return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "ReactorChannel is closed, submit aborted.");
                }

                if (Watchlist is null) // watchlist not enabled, submit normally
                {
                    return Reactor.SubmitChannel(this, rdmMsg, submitOptions, out errorInfo);
                }
                else // watchlist is enabled, submit via watchlist
                {
                    return Watchlist.SubmitMsg(rdmMsg, submitOptions, out errorInfo);
                }
            }
            finally
            {
                Reactor.ReactorLock.Exit();
            }
        }

        /// <summary>
        /// Gets a buffer from the ReactorChannel for writing a message
        /// </summary>
        /// <param name="size">the size(in bytes) of the buffer to get</param>
        /// <param name="packedBuffer">indicates whether the buffer allows packing multiple messages</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns>the buffer for writing the message or null, if an error occurred (errorInfo will be populated with information)</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ITransportBuffer? GetBuffer(int size, bool packedBuffer, out ReactorErrorInfo? errorInfo)
        {
            if (Reactor == null)
            {
                Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.GetBuffer", "Reactor cannot be null");
                return null;
            }
            ITransportBuffer buffer = Channel!.GetBuffer(size, packedBuffer, out Error transportError);
            if (transportError != null)
            {
                Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.GetBuffer", transportError.Text);
            }
            else
            {
                errorInfo = null;
            }
            return buffer;
        }

        /// <summary>
        /// Packs current buffer to allow another message to be written
        /// </summary>
        /// <param name="buffer">the buffer to be packed</param>
        /// <param name="errorInfo"><see cref="ValueAdd.Reactor.ReactorErrorInfo"/> structure filled with error information in case of failure</param>
        /// <returns>value greater than 0 indicating the number of bytes left in the buffer in case of success,
        /// <see cref="ReactorReturnCode"/> value indicating the status operation otherwise. </returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ReactorReturnCode PackBuffer(ITransportBuffer buffer, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            if (Reactor == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.Submit", "Reactor cannot be null");
            }

            if (buffer == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "ReactorChannel.PackBuffer",
                        "TransportBuffer is null.");
            }

            if (Reactor.IsShutdown)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                        "ReactorChannel.PackBuffer",
                        "Reactor is shutdown, PackBuffer aborted.");
            }

            TransportReturnCode transportReturnCode = Channel!.PackBuffer(buffer, out Error error);
            if (transportReturnCode < TransportReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, (ReactorReturnCode)transportReturnCode,
                        "ReactorChannel.PackBuffer",
                        error?.Text ?? "");
            }

            return (ReactorReturnCode)transportReturnCode;
        }

        /// <summary>
        /// Returns an unwritten buffer to the ReactorChannel.
        /// </summary>
        /// <param name="buffer">the buffer to release</param>
        /// <param name="errorInfo"><see cref="ValueAdd.Reactor.ReactorErrorInfo"/> to be set in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating success or failure</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorReturnCode ReleaseBuffer(ITransportBuffer buffer, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            if (buffer is null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "ReactorChannel.ReleaseBuffer",
                        "TransportBuffer is null.");
            }

            if (Reactor is null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "ReactorChannel.ReleaseBuffer", "Reactor cannot be null");
            }

            TransportReturnCode retCode = Channel!.ReleaseBuffer(buffer, out Error error);

            if (retCode < TransportReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, (ReactorReturnCode)retCode,
                        "ReactorChannel.ReleaseBuffer",
                        error.Text);
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Changes some aspects of the ReactorChannel
        /// </summary>
        /// <param name="code">code indicating the option to change</param>
        /// <param name="value">value to change the option to</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> value indicating the status of the operation</returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        public ReactorReturnCode IOCtl(IOCtlCode code, int value, out ReactorErrorInfo? errorInfo)
        {
            if (Reactor == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "ReactorChannel.IOCtl", "Reactor cannot be null");
            }
            if (Reactor.IsShutdown)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                        "ReactorChannel.IOCtl",
                        "Reactor is shutdown, IOCtl aborted.");
            }

            if (Channel == null)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                        "ReactorChannel.IOCtl",
                        "Channel is shutdown, IOCtl aborted.");
            }

            TransportReturnCode ret = Channel.IOCtl(code, value, out Error error);

            if (ret < TransportReturnCode.SUCCESS)
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "ReactorChannel.IOCtl",
                        $"Channel.IOCtl failed, error: {error?.Text}");
            }
            else
            {
                errorInfo = null;
                return ReactorReturnCode.SUCCESS;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        internal IChannel? Reconnect(out Error? error)
        {
            m_ReconnectAttempts++;
             if (++m_ListIndex == ConnectOptions!.ConnectionList.Count)
            {
                m_ListIndex = 0;
            }

            ReactorConnectInfo reactorConnectInfo = ConnectOptions.ConnectionList[m_ListIndex];

            if(reactorConnectInfo.EnableSessionManagement)
            {
                if(RedoServiceDiscoveryForCurrentChannel())
                {
                    reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = string.Empty;
                    reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = string.Empty;
                    ResetCurrentChannelRetryCount();
                }

                if (Reactor!.SessionManagementStartup(TokenSession!, reactorConnectInfo, Role!, this, true,
                    out ReactorErrorInfo? reactorErrorInfo) != ReactorReturnCode.SUCCESS)
                {
                    error = new Error
                    {
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = reactorErrorInfo!.Error.Text
                    };
                }
                else
                {
                    error = null;
                }

                return null;
            }

            return Reconnect(reactorConnectInfo, out error);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        internal IChannel? ReconnectRDP(out Error? error)
        {
            ReactorConnectInfo reactorConnectInfo = ConnectOptions!.ConnectionList[m_ListIndex];
            ReactorErrorInfo? errorInfo;

            UserSpecObj = reactorConnectInfo.ConnectOptions.UserSpecObject;

            if(State == ReactorChannelState.RDP_RT_DONE)
            {
                ApplyAccessToken();

                if(Reactor.RequestServiceDiscovery(reactorConnectInfo))
                {
                    if(ApplyServiceDiscoveryEndpoint(out errorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        State = ReactorChannelState.DOWN;

                        error = new Error
                        {
                            ErrorId = TransportReturnCode.FAILURE,
                            Text = errorInfo?.Error.Text
                        };

                        return null;
                    }
                }

                return Reconnect(reactorConnectInfo, out error);
            }

            if(State == ReactorChannelState.RDP_RT_FAILED)
            {
                State = ReactorChannelState.DOWN; /* Waiting to retry with another channel info in the list. */

                error = new Error
                {
                    Text = ReactorErrorInfo.Error.Text,
                    ErrorId = TransportReturnCode.FAILURE,

                };

                return null;
            }

            error = null;
            return null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        private IChannel? Reconnect(ReactorConnectInfo reactorConnectInfo, out Error error)
        {
            IncreaseRetryCountForCurrentChannel();
            UserSpecObj = reactorConnectInfo.ConnectOptions.UserSpecObject;
            reactorConnectInfo.ConnectOptions.ChannelReadLocking = true;
            reactorConnectInfo.ConnectOptions.ChannelWriteLocking = true;

            // connect
            IChannel channel = Transport.Connect(reactorConnectInfo.ConnectOptions, out error);

            if (channel != null)
                InitializationTimeout(reactorConnectInfo.GetInitTimeout());

            return channel;
        }

        internal ReactorChannel()
        {
            NotifierEvent = new NotifierEvent
            {
                _RegisteredFlags = NotifierEventFlag.READ
            };

            ServiceEndpointInfoList = new List<ReactorServiceEndpointInfo>();

            Clear();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization)]
        internal void Clear()
        {
            State = ReactorChannelState.UNKNOWN;
            Reactor = null;
            Socket = null;
            Channel = null;
            Server = null;
            UserSpecObj = null;
            m_InitializationTimeout = 0;
            m_InitializationEndTimeMs = 0;
            FlushRequested = false;
            FlushAgain = false;
            m_PingHandler.Clear();
            m_ReconnectAttempts = 0;
            m_ReconnectDelay = 0;
            NextRecoveryTime = 0;
            ConnectOptions = null;
            m_ListIndex = 0;
            Role = null;
            m_RestConnectOptions = null;
            TokenSession = null;
            ServiceEndpointInfoList.Clear();
            RDMLoginRequestRDP = null;
            TokenSession = null;
            Watchlist = null;
            ReadRet = TransportReturnCode.SUCCESS;
            IsClosedAckSent = false;
        }

        /// <summary>
        /// Returns this object back to the pool. 
        /// </summary>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void ReturnToPool()
        {
            /* Releases user-specified object specified by users if any. */
            UserSpecObj = null;

            Reactor = null;
            Socket = null;
            Channel = null;
            Server = null;
            ConnectOptions = null;
            Role = null;
            TokenSession = null;
            RDMLoginRequestRDP = null;
            TokenSession = null;
            Watchlist?.Close();
            Watchlist = null;
            ReadRet = TransportReturnCode.SUCCESS;

            base.ReturnToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void InitializationTimeout(int timeout)
        {
            m_InitializationTimeout = timeout;
            m_InitializationEndTimeMs = (timeout * 1000) + ReactorUtil.GetCurrentTimeMilliSecond();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal long InitializationTimeout()
        {
            return m_InitializationTimeout;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal long InitializationEndTimeMs()
        {
            return m_InitializationEndTimeMs;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal PingHandler GetPingHandler()
        {
            return m_PingHandler;
        }

        /* Stores connection options for reconnection. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReactorConnectOptions(ReactorConnectOptions reactorConnectOptions)
        {
            if (ConnectOptions is null)
                ConnectOptions = new ReactorConnectOptions();

            reactorConnectOptions.Copy(ConnectOptions);

            foreach(var connectInfo in ConnectOptions.ConnectionList)
            {
                connectInfo.HostAndPortProvided = (!string.IsNullOrEmpty(connectInfo.ConnectOptions.UnifiedNetworkInfo.Address)
                    && !string.IsNullOrEmpty(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName));
                connectInfo.ReconnectAttempts = 0;
            }

            m_ReconnectDelay = 0;
            NextRecoveryTime = 0;
        }

        /* Determines the time at which reconnection should be attempted for this channel. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void CalculateNextReconnectTime()
        {
            if (m_ReconnectDelay < ConnectOptions!.GetReconnectMaxDelay())
            {
                if (m_ReconnectDelay != 0)
                {
                    m_ReconnectDelay *= 2;
                }
                else // set equal to reconnectMinDelay first time through
                {
                    m_ReconnectDelay = ConnectOptions.GetReconnectMinDelay();
                }

                if (m_ReconnectDelay > ConnectOptions.GetReconnectMaxDelay())
                {
                    m_ReconnectDelay = ConnectOptions.GetReconnectMaxDelay();
                }
            }

            NextRecoveryTime = ReactorUtil.GetCurrentTimeMilliSecond() + m_ReconnectDelay;
        }

        /* Resets info related to reconnection such as timers. Used when a channel is up. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ResetReconnectTimers()
        {
            m_ReconnectAttempts = 0;
            m_ReconnectDelay = 0;
            NextRecoveryTime = 0;
        }

        /* Returns whether this channel has reached its number of reconnect attempts. */
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool RecoveryAttemptLimitReached()
        {
            return (ConnectOptions!.GetReconnectAttemptLimit() != NO_RECONNECT_LIMIT &&
                    m_ReconnectAttempts >= ConnectOptions.GetReconnectAttemptLimit());
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorConnectInfo GetReactorConnectInfo()
        {
            return ConnectOptions!.ConnectionList![m_ListIndex];
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void SetChannel(IChannel? channel)
        {
            Channel = channel;

            if(channel != null)
            {
                Socket = channel.Socket;
                OldSocket = channel.Socket;
            }
            else
            {
                Socket = null;
                OldSocket= null;
            }
        }

        /// <summary>
        /// Returns the time at which to attempt to recover this channel.
        /// </summary>
        internal long NextRecoveryTime { get; private set; }

        /// <summary>
        /// Gets or set whether a FLUSH event is has been sent to the worker and is awaiting a FLUSH_DONE event.
        /// </summary>

        internal bool FlushRequested { get; set; }

        /// <summary>
        /// Gets or sets whether the Reactor should request more flushing when the FLUSH_DONE event arrives.
        /// </summary>
        internal bool FlushAgain { get; set; }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorRestConnectOptions GetRestConnectionOptions()
        {
            if(m_RestConnectOptions is null)
            {
                m_RestConnectOptions = new ReactorRestConnectOptions(Reactor!.m_ReactorOptions);
            }

            ReactorConnectInfo reactorConnectInfo = ConnectOptions!.ConnectionList![m_ListIndex];
            m_RestConnectOptions.ProxyOptions.ProxyHostName = reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName;
            m_RestConnectOptions.ProxyOptions.ProxyPort = reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyPort;
            m_RestConnectOptions.ProxyOptions.ProxyUserName = reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName;
            m_RestConnectOptions.ProxyOptions.ProxyPassword = reactorConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword;

            return m_RestConnectOptions;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorReturnCode ApplyServiceDiscoveryEndpoint(out ReactorErrorInfo? errorInfo)
        {
            ReactorConnectInfo reactorConnectInfo = ConnectOptions!.ConnectionList![m_ListIndex];
            string transportType = string.Empty;
            ReactorServiceEndpointInfo? selectedEndpoint = null;

            if (reactorConnectInfo.ConnectOptions.ConnectionType == ConnectionType.ENCRYPTED)
            {
                if (reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol == ConnectionType.SOCKET)
                {
                    transportType = ReactorRestClient.RDP_RT_TRANSPORT_PROTOCOL_TCP;
                }
                else
                {
                    transportType = ReactorRestClient.RDP_RT_TRANSPORT_PROTOCOL_WEBSOCKET;
                }
            }

            foreach (var endpointInfo in ServiceEndpointInfoList)
            {
                if (endpointInfo.LocationList.Count > 0 && reactorConnectInfo.Location is not null)
                {
                    if (endpointInfo.Transport.Equals(transportType) &&
                        endpointInfo.LocationList[0].StartsWith(reactorConnectInfo.Location))
                    {
                        // Try to find an endpoint which provides the maximum number of availability zones.
                        if (selectedEndpoint is null || selectedEndpoint.LocationList.Count < endpointInfo.LocationList.Count)
                        {
                            selectedEndpoint = endpointInfo;
                        }
                    }
                }
            }

            if(selectedEndpoint is not null)
            {
                reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address = selectedEndpoint.EndPoint;
                reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = selectedEndpoint.Port;
            }
            else
            {
                return Reactor.PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "ReactorChannel.ApplyServiceDiscoveryEndpoint()",
                    $"Could not find matching location: {reactorConnectInfo.Location} from RDP service discovery.");
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool EnableSessionManagement()
        {
            return ConnectOptions!.ConnectionList![m_ListIndex].EnableSessionManagement;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal IReactorAuthTokenEventCallback? ReactorAuthTokenEventCallback()
        {
            return ConnectOptions!.ConnectionList![m_ListIndex].ReactorAuthTokenEventCallback;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ApplyAccessToken()
        {
            /* Checks to ensure that the application specifies the Login request in the ConsumerRole as well */
            if(RDMLoginRequestRDP is not null && TokenSession is not null)
            {
                RDMLoginRequestRDP.UserNameType = Eta.Rdm.Login.UserIdTypes.AUTHN_TOKEN;
                RDMLoginRequestRDP.UserName.Data(TokenSession.ReactorAuthTokenInfo.AccessToken);

                // Don't send the password
                RDMLoginRequestRDP.Flags &= ~LoginRequestFlags.HAS_PASSWORD;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        bool RedoServiceDiscoveryForCurrentChannel()
        {
            if (ConnectOptions != null)
            {
                ReactorConnectInfo connectInfo = ConnectOptions.ConnectionList![m_ListIndex];

                return (connectInfo.ServiceDiscoveryRetryCount != 0
                        && !connectInfo.HostAndPortProvided
                        && connectInfo.ReconnectAttempts == connectInfo.ServiceDiscoveryRetryCount);
            }
            else
            {
                return false;
            }

        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void IncreaseRetryCountForCurrentChannel()
        {
            if (ConnectOptions != null)
            {
                ReactorConnectInfo connectInfo = ConnectOptions.ConnectionList![m_ListIndex];
                if (connectInfo.EnableSessionManagement && connectInfo.ServiceDiscoveryRetryCount != 0)
                {
                    connectInfo.ReconnectAttempts++;
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ResetCurrentChannelRetryCount()
        {
            if (ConnectOptions != null)
            {
                ReactorConnectInfo connectInfo = ConnectOptions.ConnectionList![m_ListIndex];
                connectInfo.ReconnectAttempts = 0;
            }
        }
    }
}
