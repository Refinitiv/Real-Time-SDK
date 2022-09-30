/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Common.Interfaces;
using Refinitiv.Eta.Codec;
using Refinitiv.Eta.Common;
using Refinitiv.Eta.Rdm;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.ValueAdd.Common;
using Refinitiv.Eta.ValueAdd.Rdm;
using System.Net.Sockets;
using System.Text;
using Buffer = Refinitiv.Eta.Codec.Buffer;

namespace Refinitiv.Eta.ValueAdd.Reactor
{
    /// <summary>
    /// The Reactor. Applications create Reactor objects by calling <see cref="Reactor.CreateReactor(ReactorOptions, out ReactorErrorInfo?)"/>,
    /// create connections by calling <see cref="Reactor.Connect(ReactorConnectOptions, ReactorRole, out ReactorErrorInfo?)"/> and process events 
    /// by calling <see cref="ReactorChannel.Dispatch(ReactorDispatchOptions, out ReactorErrorInfo?)"/>/<see cref="Reactor.Dispatch(ReactorDispatchOptions, out ReactorErrorInfo?)"/>
    /// </summary>
    public class Reactor
    {
        internal const int DEFAULT_INIT_EVENT_POOLS = 10;

        internal ReaderWriterLockSlim ReactorLock { get; set; } = 
            new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion);

        private VaDoubleLinkList<ReactorChannel> m_ReactorChannelQueue = new VaDoubleLinkList<ReactorChannel>();
        private bool m_ReactorActive = false;
        internal ReactorOptions m_ReactorOptions = new ReactorOptions();
        private ReactorWorker? m_ReactorWorker;
        private ReactorEventQueue? m_ReactorEventQueue;
        private ReactorPool m_ReactorPool = new ReactorPool();

        private LoginMsg m_LoginMsg = new();
        private DirectoryMsg m_DirectoryMsg = new DirectoryMsg();
        private DictionaryMsg m_DictionaryMsg = new DictionaryMsg();

        private DecodeIterator m_DecodeIterator = new DecodeIterator();
        private EncodeIterator m_EncodeIterator = new EncodeIterator();
        private Msg m_Msg = new Msg();

        private ReactorSubmitOptions m_ReactorSubmitOptions = new();

        private ReactorChannelInfo m_ReactorChannelInfo = new ReactorChannelInfo();

        private StringBuilder m_XmlString = new StringBuilder();
        private XmlTraceDump m_XmlTraceDump = new XmlTraceDump();

        private IDictionary<Msg, ITransportBuffer> m_SubmitMsgMap = new Dictionary<Msg, ITransportBuffer>();
        private IDictionary<MsgBase, ITransportBuffer> m_SubmitRdmMsgMap = new Dictionary<MsgBase, ITransportBuffer>();

        internal ReactorRestClient? m_ReactorRestClient;

        private WriteArgs m_WriteArgs = new WriteArgs();

        public Socket? EventSocket { get; private set; }
        public object? UserSpecObj { get; private set; }

        internal int ChannelCount { get => m_ReactorChannelQueue.Count(); }

        private ReactorOAuthCredential m_ReactorOAuthCredential = new ReactorOAuthCredential();

        private ReactorTokenSession? m_TokenSessionRenewalCallback;

        private Reactor(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            if(InitializeTransport(out errorInfo) != ReactorReturnCode.SUCCESS )
            {
                return;
            }

            if (InitializeReactor(out errorInfo) != ReactorReturnCode.SUCCESS)
            {
                return;
            }

            m_ReactorOptions.Copy(options);
            m_ReactorRestClient = new ReactorRestClient(this);
            m_ReactorActive = true;
        }


        /// <summary>
        /// Create a <see cref="Reactor"/>.
        /// </summary>
        /// <param name="options">The options</param>
        /// <param name="errorInfo"></param>
        /// <returns>A <see cref="Reactor"/> object or <c>null</c>. If <c>null</c>, check errorInfo for
        /// additional information regarding the failure.
        /// </returns>
        public static Reactor? CreateReactor(ReactorOptions options, out ReactorErrorInfo? errorInfo)
        {
            if (options is null)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.CreateReactor", "options was null and cannot continue.");
                return null;
            }

            Reactor reactor = new Reactor(options, out errorInfo);
            
            if(errorInfo is null)
            {
                reactor.UserSpecObj = options.UserSpecObj;
                return reactor;
            }

            return null;
        }

        /// <summary>
        /// Adds a client-side channel to the Reactor. Once the channel is initialized,
        /// the <see cref="IReactorChannelEventCallback.ReactorChannelEventCallback(ReactorChannelEvent)"/>
        /// will receive an event indicating that the channel is up.
        /// </summary>
        /// <param name="reactorConnectOptions">options for this connection</param>
        /// <param name="role">role of this connection</param>
        /// <param name="errorInfo">error structure to be set in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating success or failure</returns>
        public ReactorReturnCode Connect(ReactorConnectOptions reactorConnectOptions, ReactorRole role, 
            out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ReactorReturnCode retVal = ReactorReturnCode.SUCCESS;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "Reactor.Connect",
                        "Reactor is not active, aborting.");
                }
                else if (reactorConnectOptions is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "reactorConnectOptions cannot be null, aborting.");
                }
                else if (role is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "ReaCtor.connect", "role cannot be null, aborting.");
                }
                else if (role.ChannelEventCallback is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.connect",
                            "role must have a channelEventCallback defined, aborting.");
                }
                else if (role.DefaultMsgCallback == null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "role must have a DefaultMsgCallback defined, aborting.");
                }
                else if (role.Type == ReactorRoleType.CONSUMER)
                {
                    if (((ConsumerRole)role).RdmDirectoryRequest != null && ((ConsumerRole)role).RdmLoginRequest == null)
                    {
                        return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "Must specify an LoginRequest if specifying an DirectoryRequest, aborting.");
                    }

                    if (((ConsumerRole)role).DictionaryDownloadMode == DictionaryDownloadMode.FIRST_AVAILABLE
                        && ((ConsumerRole)role).RdmDirectoryRequest == null)
                    {
                        return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                "Reactor.Connect",
                                "Must specify an RdmDirectoryRequest if specifying a dictionary download, aborting.");
                    }
                }
                else if (role.Type == ReactorRoleType.NIPROVIDER)
                {
                    if (((NIProviderRole)role).RdmDirectoryRefresh != null && ((NIProviderRole)role).RdmLoginRequest == null)
                    {
                        return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                "Reactor.Connect",
                                "Must specify an LoginRequest if specifying an DirectoryRequest, aborting.");
                    }
                }
                else if (role.Type == ReactorRoleType.PROVIDER)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "role must be Consumer or NIProvider Role, aborting.");
                }

                if (reactorConnectOptions.ConnectionList.Count == 0)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "ReactorConnectOptions.ConnectionList must have at least one ReactorConnectInfo, aborting.");
                }
                else if (reactorConnectOptions.ConnectionList[0].GetInitTimeout() < 1)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "ReactorConnectOptions.InitTimeout must be greater than zero, aborting.");
                }
                else if (reactorConnectOptions.ConnectionList[0].ConnectOptions.Blocking == true)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "ReactorConnectOptions.ConnectOptions.Blocking must be false, aborting.");
                }

                ReactorChannel reactorChannel = m_ReactorPool.CreateReactorChannel();
                reactorChannel.Reactor = this;
                reactorChannel.UserSpecObj = reactorConnectOptions.ConnectionList[0].ConnectOptions.UserSpecObject;
                reactorChannel.InitializationTimeout(reactorConnectOptions.ConnectionList[0].GetInitTimeout());
                reactorChannel.ReactorConnectOptions(reactorConnectOptions);

                bool sendAuthTokenEvent = false;

                if (CheckEnableSessionManagement(reactorConnectOptions))
                {
                    SetupRestClient();

                    ReactorOAuthCredential? oAuthCredential = RetrieveOAuthCredentialFromRole(role, out errorInfo);

                    if (oAuthCredential is null)
                    {
                        return errorInfo!.Code;
                    }

                    reactorChannel.TokenSession = new ReactorTokenSession(this, oAuthCredential);
                }

                if(reactorConnectOptions.ConnectionList[0].EnableSessionManagement)
                {
                    if(SessionManagementStartup(reactorChannel.TokenSession!, reactorConnectOptions.ConnectionList[0], role,
                        reactorChannel, false, out errorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        reactorChannel.ReturnToPool();
                        return errorInfo!.Code;
                    }

                    reactorChannel.ApplyAccessToken();

                    reactorChannel.TokenSession!.OriginalExpiresIn = reactorChannel.TokenSession.ReactorAuthTokenInfo.ExpiresIn;

                    sendAuthTokenEvent = true;

                    /* Clears OAuth sensitive information if the callback is specified */
                    if (reactorChannel.TokenSession.ReactorOAuthCredential.ReactorOAuthCredentialEventCallback != null)
                    {
                        reactorChannel.TokenSession.ReactorOAuthCredential.ClientSecret.Clear();
                    }
                }

                reactorChannel.State = ReactorChannelState.INITIALIZING;
                reactorChannel.Role = role;

                // Add it to the initChannelQueue.
                m_ReactorChannelQueue.PushBack(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

                // enable channel read/write locking for reactor since it's multi-threaded with worker thread
                ConnectOptions connectOptions = reactorChannel.GetReactorConnectInfo().ConnectOptions;
                connectOptions.ChannelReadLocking = true;
                connectOptions.ChannelWriteLocking = true;

                if(sendAuthTokenEvent)
                {
                    errorInfo = new ReactorErrorInfo();
                    if(SendAuthTokenEventCallback(reactorChannel, reactorChannel.TokenSession!.ReactorAuthTokenInfo, errorInfo) != ReactorCallbackReturnCode.SUCCESS)
                    {
                        return ReactorReturnCode.FAILURE;
                    }
                    
                    if(!reactorChannel.TokenSession!.InTokenSessionQueue)
                    {
                        /* This is used for unit testing to override the expires in seconds. */
                        if (ReactorTokenSession.ExpiresInTest != -1)
                        {
                            reactorChannel.TokenSession!.ReactorAuthTokenInfo.ExpiresIn = ReactorTokenSession.ExpiresInTest;
                        }

                        if (SendReactorImplEvent(ReactorEventImpl.ImplType.TOKEN_MGNT, reactorChannel) != ReactorReturnCode.SUCCESS)
                        {
                            reactorChannel.ReturnToPool();
                            m_ReactorChannelQueue.Remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
                            return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.Connect",
                                "SendReactorImplEvent() failed");
                        }
                    }
                }

                // call Transport.connect to create a new Channel
                IChannel channel = Transport.Connect(connectOptions, out Error error);

                reactorChannel.SetChannel(channel);

                if (channel is null)
                {
                    if (reactorChannel.Server is null && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                    {
                        reactorChannel.State = ReactorChannelState.DOWN_RECONNECTING;

                        // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                        SendAndHandleChannelEventCallback("Reactor.Connect",
                                ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING,
                                reactorChannel, errorInfo);
                    }
                    else // server channel or no more retries
                    {
                        reactorChannel.State = ReactorChannelState.DOWN;

                        // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                        SendAndHandleChannelEventCallback("Reactor.Connect",
                                ReactorChannelEventType.CHANNEL_DOWN,
                                reactorChannel, errorInfo);
                    }
                }
                // send a ReactorImplEvent to the Worker to initialize this channel.
                else if (SendReactorImplEvent(ReactorEventImpl.ImplType.CHANNEL_INIT, reactorChannel) != ReactorReturnCode.SUCCESS)
                {
                    // SendReactorImplEvent() failed, send channel down
                    reactorChannel.State = ReactorChannelState.DOWN;
                    SendAndHandleChannelEventCallback("Reactor.Connect",
                            ReactorChannelEventType.CHANNEL_DOWN,
                            reactorChannel, errorInfo);

                    reactorChannel.ReturnToPool();
                    m_ReactorChannelQueue.Remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
                    return PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE,
                            "Reactor.Connect",
                            "SendReactorImplEvent() failed");
                }
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            return retVal;
        }

        /// <summary>
        /// Adds a server-side channel to the Reactor. Once the channel is initialized,
        /// the <see cref="IReactorChannelEventCallback.ReactorChannelEventCallback(ReactorChannelEvent)"/>
        /// will receive an event indicating that the channel is up.
        /// </summary>
        /// <param name="server">server that is accepting this connection (a server can be created with
        /// <see cref="Transport.Bind(BindOptions, out Error)"/></param>
        /// <param name="reactorAcceptOptions">options for accepting this connection</param>
        /// <param name="role">role of this connection</param>
        /// <param name="errorInfo">error structure to be set in the event of failure</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating success or failure</returns>
        public ReactorReturnCode Accept(IServer server, ReactorAcceptOptions reactorAcceptOptions, ReactorRole role, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                            "Reactor.Accept", "Reactor is not active, aborting.");
                }
                else if (reactorAcceptOptions is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Accept",
                            "reactorAcceptOptions cannot be null, aborting.");
                }
                else if (role is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Accept", "role cannot be null, aborting.");
                }
                else if (role.ChannelEventCallback is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Accept",
                            "role must have a channelEventCallback defined, aborting.");
                }
                else if (role.DefaultMsgCallback == null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Accept",
                            "role must have a DefaultMsgCallback defined, aborting.");
                }
                else if (role.Type != ReactorRoleType.PROVIDER)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Accept",
                            "role must be Provider Role, aborting.");
                }

                if (reactorAcceptOptions.GetInitTimeout() < 1)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.accept",
                            "ReactorAcceptOptions.timeout must be greater than zero, aborting.");
                }

                // create a ReactorChannel and add it to the initChannelQueue.
                ReactorChannel reactorChannel = m_ReactorPool.CreateReactorChannel();
                reactorChannel.State = ReactorChannelState.INITIALIZING;
                reactorChannel.Role = role;
                reactorChannel.Reactor = this;
                reactorChannel.InitializationTimeout(reactorAcceptOptions.GetInitTimeout());
                reactorChannel.Server = server;
                m_ReactorChannelQueue.PushBack(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

                // enable channel read/write locking for reactor since it's multi-threaded with worker thread
                reactorAcceptOptions.AcceptOptions.ChannelReadLocking = true;
                reactorAcceptOptions.AcceptOptions.ChannelWriteLocking = true;

                // call Server.accept to accept a new Channel
                IChannel channel = server.Accept(reactorAcceptOptions.AcceptOptions,
                        out Error error);
                if (channel != null)
                {
                    reactorChannel.SetChannel(channel);
                    reactorChannel.UserSpecObj = reactorAcceptOptions.AcceptOptions.UserSpecObject;
                }
                else
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                            "Reactor.Accept", "Server.Accept() failed, error="
                                              + error?.Text);
                }

                // send a ReactorImplEvent to the Worker to initialize this channel.
                if (SendReactorImplEvent(ReactorEventImpl.ImplType.CHANNEL_INIT, reactorChannel) != ReactorReturnCode.SUCCESS)
                {
                    // SendReactorImplEvent() failed, send channel down
                    reactorChannel.State = ReactorChannelState.DOWN;
                    SendAndHandleChannelEventCallback("Reactor.Accept",
                            ReactorChannelEventType.CHANNEL_DOWN,
                            reactorChannel, errorInfo);

                    reactorChannel.ReturnToPool();
                    m_ReactorChannelQueue.Remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
                    return PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE,
                            "Reactor.Accept",
                            "SendReactorImplEvent() failed");
                }
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }


            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode SendAndHandleChannelEventCallback(string location, ReactorChannelEventType eventType, 
            ReactorChannel reactorChannel, ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode retVal = SendChannelEventCallback(eventType, reactorChannel, errorInfo);

            // check return code from callback.
            if (retVal == ReactorCallbackReturnCode.FAILURE)
            {
                PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        location,
                        "ReactorCallbackReturnCode.FAILURE was returned from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
                Shutdown(out errorInfo);
                return ReactorReturnCode.FAILURE;
            }
            else if (retVal == ReactorCallbackReturnCode.RAISE)
            {
                // RAISE is not a valid return code for the
                // reactorChannelEventCallback.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, location, "ReactorCallbackReturnCode.RAISE is not a valid return code from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
                Shutdown(out errorInfo);
                return ReactorReturnCode.FAILURE;

            }
            else if (retVal != ReactorCallbackReturnCode.SUCCESS)
            {
                // retval is not a valid ReactorReturnCodes.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, location, "retval of "
                                                    + retVal + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
                Shutdown(out errorInfo);
                return ReactorReturnCode.FAILURE;
            }

            if (eventType == ReactorChannelEventType.CHANNEL_DOWN || eventType == ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING)
            {
                if (reactorChannel.State != ReactorChannelState.CLOSED)
                {
                    SendReactorImplEvent(ReactorEventImpl.ImplType.CHANNEL_DOWN, reactorChannel);
                }
            }

            if (eventType == ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING)
            {
                if (reactorChannel.Role!.Type == ReactorRoleType.CONSUMER)
                {
                    ((ConsumerRole)reactorChannel.Role).ReceivedFieldDictionaryResp = false;
                    ((ConsumerRole)reactorChannel.Role).ReceivedEnumDictionaryResp = false;
                }
                else if (reactorChannel.Role!.Type == ReactorRoleType.NIPROVIDER)
                {
                    ((NIProviderRole)reactorChannel.Role).ReceivedFieldDictionaryResp = false;
                    ((NIProviderRole)reactorChannel.Role).ReceivedEnumDictionaryResp = false;
                }
            }

            return (ReactorReturnCode)retVal;
        }

        private ReactorCallbackReturnCode SendLoginMsgCallback(ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg, LoginMsg loginMsg)
        {
            if (reactorChannel.Role == null)
                return ReactorCallbackReturnCode.FAILURE;

            ReactorCallbackReturnCode retval;
            IRDMLoginMsgCallback? callback = null;

            switch (reactorChannel.Role)
            {
                case ConsumerRole consumerRole:
                    callback = consumerRole.LoginMsgCallback;
                    break;
                case ProviderRole providerRole:
                    callback = providerRole.LoginMsgCallback;
                    break;
                case NIProviderRole niProviderRole:
                    callback = niProviderRole.LoginMsgCallback;
                    break;
                default:
                    break;
            }

            if (callback != null)
            {
                RDMLoginMsgEvent rdmLoginMsgEvent = m_ReactorPool.CreateReactorRDMLoginMsgEventImpl();
                rdmLoginMsgEvent.Clear();
                rdmLoginMsgEvent.LoginMsg = loginMsg;
                rdmLoginMsgEvent.TransportBuffer = transportBuffer;
                rdmLoginMsgEvent.ReactorChannel = reactorChannel;
                rdmLoginMsgEvent.Msg = msg;

                retval = callback.RdmLoginMsgCallback(rdmLoginMsgEvent);
                rdmLoginMsgEvent.ReturnToPool();
            }
            else
            {
                // callback is undefined, raise it to defaultMsgCallback.
                retval = ReactorCallbackReturnCode.RAISE;
            }

            return retval;
        }

        // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
        private ReactorCallbackReturnCode SendAndHandleLoginMsgCallback(string location, ReactorChannel reactorChannel,
            ITransportBuffer transportBuffer, Msg msg, LoginMsg loginMsg, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode retval = SendLoginMsgCallback(reactorChannel, transportBuffer, msg, loginMsg /*, null*/);

            // check return code from callback.
            if (retval == ReactorCallbackReturnCode.FAILURE)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, location,
                        "ReactorCallbackReturnCodes.FAILURE was returned from RdmLoginMsgCallback(). This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }
            else if (retval != ReactorCallbackReturnCode.RAISE
                && retval != ReactorCallbackReturnCode.SUCCESS)
            {
                // retval is not a valid ReactorReturnCodes.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, location,
                    $"retval of {retval} was returned from RdmLoginMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }

            errorInfo = null;
            return retval;
        }

        /// <summary>
        /// Sends call results information back to the application.
        /// </summary>
        /// <param name="eventType"></param>
        /// <param name="reactorChannel">reactor channel associated with the current connection</param>
        /// <param name="errorInfo">optional error information (when the call resulted in an error)</param>
        /// <returns></returns>
        private ReactorCallbackReturnCode SendChannelEventCallback(ReactorChannelEventType eventType, ReactorChannel reactorChannel,
            ReactorErrorInfo? errorInfo)
        {
            ReactorChannelEvent reactorChannelEvent = m_ReactorPool.CreateReactorChannelEvent();
            reactorChannelEvent.ReactorChannel = reactorChannel;
            reactorChannelEvent.EventType = eventType;

            if (errorInfo != null)
            {
                PopulateErrorInfo(reactorChannelEvent.ReactorErrorInfo, errorInfo.Code, errorInfo.Location!, errorInfo.Error.Text);
            }

            ReactorCallbackReturnCode retVal = reactorChannel.Role!.ChannelEventCallback!.ReactorChannelEventCallback(reactorChannelEvent);
            reactorChannelEvent.ReturnToPool();

            return retVal;
        }

        /// <summary>
        /// Process events and messages from the Reactor.
        /// </summary>
        ///
        /// <remarks>
        /// <para>These are passed to the calling application via the the callback methods associated with
        /// each of the channels.</para>
        ///
        /// <para>
        /// Depending on whether reactorDispatchOptions.ReactorChannel is set, it either handles
        /// events from the reactor's channel and the specified channel only, or the reactor and
        /// all channels.
        /// </para>
        /// </remarks>
        ///
        /// <param name="reactorDispatchOptions">options for how to dispatch</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns>a positive value if dispatching succeeded and there are more messages to process or
        /// <see cref="ReactorReturnCode.SUCCESS"/> if dispatching succeeded and there are no more messages
        /// to process or <see cref="ReactorReturnCode.FAILURE"/>, if dispatching failed (refer to errorInfo
        /// for additional information)</returns>
        public ReactorReturnCode Dispatch(ReactorDispatchOptions reactorDispatchOptions, out ReactorErrorInfo? errorInfo)
        {
            int maxMessages = reactorDispatchOptions.GetMaxMessages();
            int msgCount = 0;
            ReactorReturnCode retVal = ReactorReturnCode.SUCCESS;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "Reactor.Dispatch",
                        "Reactor is not active, aborting.");
                }

                // handle Reactor's channel before individual channels in any case
                while (msgCount < maxMessages && m_ReactorEventQueue!.GetEventQueueSize() > 0)
                {
                    msgCount++;
                    if ((retVal = ProcessReactorEventImpl(out errorInfo)) < ReactorReturnCode.SUCCESS)
                        return retVal;
                }

                if (msgCount == maxMessages)
                {
                    errorInfo = null;
                    /* return the number of events to be dispatch if any */
                    return (ReactorReturnCode)m_ReactorEventQueue!.GetEventQueueSize();
                }

                int reactorChannelCount = 0;

                /* Either handle just one specified channel, or round robin through all channels */
                ReactorChannel? reactorChannel = reactorDispatchOptions.ReactorChannel
                    ?? m_ReactorChannelQueue.Start(ReactorChannel.REACTOR_CHANNEL_LINK);

                do
                {
                    if (reactorChannel == null)
                        break;

                    reactorChannelCount++;
                    if (!IsReactorChannelReady(reactorChannel) || reactorChannel.Socket is null)
                        continue;

                    bool isReadReady = false;

                    try
                    {
                        isReadReady = reactorChannel.Socket.Poll(0, SelectMode.SelectRead) || 
                            reactorChannel.ReadRet > TransportReturnCode.SUCCESS;
                    }
                    catch (Exception)
                    {
                        /* The Socket is no longer valid */
                    }

                    if (isReadReady)
                    {
                        retVal++;
                    }

                    while (isReadReady && msgCount < maxMessages && retVal > ReactorReturnCode.SUCCESS)
                    {
                        if ((retVal = PerformChannelRead(reactorChannel, reactorDispatchOptions.ReadArgs, out errorInfo)) < ReactorReturnCode.SUCCESS)
                        {
                            if (reactorChannel.State != ReactorChannelState.CLOSED &&
                                reactorChannel.State != ReactorChannelState.DOWN_RECONNECTING)
                            {
                                return retVal;
                            }
                            else
                            {
                                // return success since close or reconnecting is not an error
                                retVal = ReactorReturnCode.SUCCESS;
                            }
                        }

                        // only increment msgCount if byts are actally read
                        if (reactorDispatchOptions.ReadArgs.UncompressedBytesRead > 0)
                        {
                            msgCount++;
                        }
                    }

                    /* Don't Roundrobin when dispatching per ReactorChannel. */
                    if (reactorDispatchOptions.ReactorChannel != null)
                        break;

                    if (msgCount == maxMessages || reactorChannelCount == m_ReactorChannelQueue.Count())
                    {
                        // update retval
                        retVal = (int)(reactorChannelCount < m_ReactorChannelQueue.Count()
                            ? (ReactorReturnCode.SUCCESS + 1) // not all channels were dispatched
                            : ReactorReturnCode.SUCCESS)      // all channels were dispatched
                            + retVal;
                        break; // break from the for loop
                    }
                }
                while ((reactorChannel = m_ReactorChannelQueue.Forth(ReactorChannel.REACTOR_CHANNEL_LINK)) != null);
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            errorInfo = null;
            return retVal;
        }

        /// <summary>
        /// Queries EDP-RT service discovery to get service endpoint information.
        /// </summary>
        /// <param name="options"></param>
        /// <param name="errorInfo"></param>
        /// <returns></returns>
        public ReactorReturnCode QueryServiceDiscovery(ReactorServiceDiscoveryOptions serviceDiscoveryOptions, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN,
                            "Reactor.QueryServiceDiscovery", "Reactor is not active, aborting.");
                }

                if (serviceDiscoveryOptions is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.QueryServiceDiscovery",
                            "Reactor.QueryServiceDiscovery(): options cannot be null, aborting.");
                }

                if (serviceDiscoveryOptions.ReactorServiceEndpointEventCallback is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.QueryServiceDiscovery",
                            "Reactor.QueryServiceDiscovery(): ReactorServiceEndpointEventCallback cannot be null, aborting.");
                }

                if (serviceDiscoveryOptions.ClientId is null || serviceDiscoveryOptions.ClientId.Length == 0)
                {
                    return PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.PARAMETER_INVALID,
                            "Reactor.QueryServiceDiscovery",
                            "Required parameter ClientId is not set");
                }

                if (serviceDiscoveryOptions.ClientSecret is null || serviceDiscoveryOptions.ClientSecret.Length == 0)
                {
                    return PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.PARAMETER_INVALID,
                            "Reactor.QueryServiceDiscovery",
                            "Required parameter ClientSecret is not set");
                }

                ReactorRestConnectOptions connOptions = new ReactorRestConnectOptions(m_ReactorOptions);

                switch (serviceDiscoveryOptions.Transport)
                {
                    case ReactorDiscoveryTransportProtocol.RD_TP_INIT:
                        break;
                    case ReactorDiscoveryTransportProtocol.RD_TP_TCP:
                        connOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
                        break;
                    case ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET:
                        connOptions.Transport = ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET;
                        break;
                    default:

                        PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_OUT_OF_RANGE, "Reactor.QueryServiceDiscovery",
                                "Reactor.QueryServiceDiscovery(): Invalid transport protocol type " + serviceDiscoveryOptions.Transport);
                        return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;
                }

                switch (serviceDiscoveryOptions.DataFormat)
                {
                    case ReactorDiscoveryDataFormatProtocol.RD_DP_INIT:
                        break;
                    case ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2:
                        connOptions.DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2;
                        break;
                    case ReactorDiscoveryDataFormatProtocol.RD_DP_RWF:
                        connOptions.DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_RWF;
                        break;
                    default:

                        PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_OUT_OF_RANGE, "Reactor.QueryServiceDiscovery",
                                "Reactor.QueryServiceDiscovery(): Invalid dataformat protocol type " + serviceDiscoveryOptions.DataFormat);
                        return ReactorReturnCode.PARAMETER_OUT_OF_RANGE;
                }

                SetupRestClient();

                ReactorAuthTokenInfo authTokenInfo = new ReactorAuthTokenInfo();
                connOptions.ApplyServiceDiscoveryOptions(serviceDiscoveryOptions);

                m_ReactorOAuthCredential.Clear();

                m_ReactorOAuthCredential.ClientId.Data(new ByteBuffer(serviceDiscoveryOptions.ClientId.Length));
                serviceDiscoveryOptions.ClientId.Copy(m_ReactorOAuthCredential.ClientId);

                m_ReactorOAuthCredential.ClientSecret.Data(new ByteBuffer(serviceDiscoveryOptions.ClientSecret.Length));
                serviceDiscoveryOptions.ClientSecret.Copy(m_ReactorOAuthCredential.ClientSecret);

                if (m_ReactorRestClient!.SendTokenRequest(m_ReactorOAuthCredential, connOptions, authTokenInfo, 
                    out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    SendQueryServiceDiscoveryEvent(serviceDiscoveryOptions, null, errorInfo);
                    return ReactorReturnCode.SUCCESS;
                }

                List<ReactorServiceEndpointInfo> endpointInfos = new List<ReactorServiceEndpointInfo>();

                if (m_ReactorRestClient!.SendServiceDirectoryRequest(connOptions, authTokenInfo, endpointInfos, out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    SendQueryServiceDiscoveryEvent(serviceDiscoveryOptions, null, errorInfo);
                }
                else
                {
                    SendQueryServiceDiscoveryEvent(serviceDiscoveryOptions, endpointInfos, errorInfo);
                }
            }
            finally
            { 
                ReactorLock.ExitWriteLock();
            }

            return ReactorReturnCode.SUCCESS;
        }



        /// <summary>
        /// Submit OAuth credential renewal with sensitive information.
        /// </summary>
        /// <param name="renewalOptions">The <see cref="ReactorOAuthCredentialRenewalOptions"/> to configure renewal options</param>
        /// <param name="oAuthCredentialRenewal">The <see cref="ReactorOAuthCredentialRenewal"/> to provide credential renewal information</param>
        /// <param name="errorInfo">error structure to be populated in the event of failure</param>
        /// <returns><see cref="ReactorReturnCodes"/> indicating success or failure</returns>
        public ReactorReturnCode SubmitOAuthCredentialRenewal(ReactorOAuthCredentialRenewalOptions renewalOptions, 
            ReactorOAuthCredentialRenewal oAuthCredentialRenewal, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "Reactor.SubmitOAuthCredentialRenewal",
                        "Reactor is not active, aborting.");
                }

                if(renewalOptions is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.SubmitOAuthCredentialRenewal",
                        "renewalOptions cannot be null, aborting.");
                }

                if(oAuthCredentialRenewal is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.SubmitOAuthCredentialRenewal",
                        "oAuthCredentialRenewal cannot be null, aborting.");
                }

                if(m_TokenSessionRenewalCallback is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.SubmitOAuthCredentialRenewal",
                        "This function must be called in the IReactorOAuthCredentialEventCallback.ReactorOAuthCredentialEventCallback() method, aborting.");
                }

                if(oAuthCredentialRenewal.ClientSecret is null || oAuthCredentialRenewal.ClientSecret.IsBlank)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.SubmitOAuthCredentialRenewal",
                        "ReactorOAuthCredentialRenewal.ClientSecret not provided, aborting.");
                }

                switch(renewalOptions.RenewalModes)
                {
                    case ReactorOAuthCredentialRenewalModes.CLIENT_SECRET:
                        break;
                    default:
                        return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.SubmitOAuthCredentialRenewal",
                            $"Invalid ReactorOAuthCredentialRenewalOptions.RenewalModes({renewalOptions.RenewalModes}), aborting.");
                }

                m_TokenSessionRenewalCallback.SendAuthRequestWithSensitiveInfo(oAuthCredentialRenewal.ClientSecret.ToString());
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            return ReactorReturnCode.SUCCESS;
        }

        /// <summary>
        /// Returns whether or not the Reactor is shutdown.
        /// </summary>
        /// <returns><c>true</c> if the Reactor is shutdown; otherwise <c>false</c></returns>
        public bool IsShutdown
        {
            get => !m_ReactorActive;
        }

        /// <summary>
        /// Shuts down and cleans up a Reactor. Stops the ETA Reactor if necessary and
        /// sends ReactorChannelEvents to all active channels indicating that they are down.
        /// Once this call is made, the Reactor is destroyed and no further calls should
        /// be made with it.
        /// </summary>
        /// <param name="errorInfo">Error structure to be populated in the event of an error</param>
        /// <returns><see cref="ReactorReturnCode"/> indicating success or failure</returns>
        public ReactorReturnCode Shutdown(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ReactorReturnCode retVal = ReactorReturnCode.SUCCESS;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                    return retVal;

                /* Send CHANNEL_DOWN to worker and application for all reactorChannels. */
                for (ReactorChannel? reactorChannel = m_ReactorChannelQueue.Start(ReactorChannel.REACTOR_CHANNEL_LINK);
                reactorChannel != null;
                reactorChannel = m_ReactorChannelQueue.Forth(ReactorChannel.REACTOR_CHANNEL_LINK))
                {
                    if (reactorChannel is null || reactorChannel.State == ReactorChannelState.CLOSED)
                        continue;

                    SendChannelEventCallback(ReactorChannelEventType.CHANNEL_DOWN, reactorChannel, errorInfo);

                    if(reactorChannel.State != ReactorChannelState.CLOSED)
                    {
                        CloseChannel(reactorChannel, out errorInfo);
                    }
                }

                m_ReactorChannelQueue.Clear();

                SendReactorImplEvent(ReactorEventImpl.ImplType.SHUTDOWN, null);

                /* Waits until the worker thread exits properly */
                do
                {
                    Thread.Sleep(5000);
                } while (m_ReactorWorker!.IsWorkerThreadStarted);

                TransportReturnCode transportReturnCode = Transport.Uninitialize();
                if (transportReturnCode != TransportReturnCode.SUCCESS)
                {
                    retVal = PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.Shutdown", 
                        "Failed to uninitilize the transport library.");
                }

                m_ReactorWorker = null;

                if (m_ReactorEventQueue != null)
                {
                    m_ReactorEventQueue.UninitReactorEventQueue();
                    m_ReactorEventQueue = null;
                }

                if(m_ReactorRestClient is not null)
                {
                    m_ReactorRestClient.Dispose();
                    m_ReactorRestClient = null;
                }
            }
            finally
            {
                m_ReactorActive = false;
                ReactorLock.ExitWriteLock();
            }

            return retVal;
        }

        private void SetupRestClient()
        {
            if (m_ReactorRestClient is null)
            {
                m_ReactorRestClient = new ReactorRestClient(this);
            }
        }

        private ReactorReturnCode InitializeTransport(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true,
            };

            if (Transport.Initialize(initArgs, out Error error) != TransportReturnCode.SUCCESS)
            {
                return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "Reactor.initializeTransport", error);
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode InitializeReactor(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            m_ReactorEventQueue = new ReactorEventQueue();

            if (m_ReactorEventQueue.InitReactorEventQueue() != ReactorReturnCode.SUCCESS)
            {
                return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.Constructor", "Failed to initilize event queue.");
            }

            EventSocket = m_ReactorEventQueue.GetEventQueueSocket();

            /* Initialize event pools */
            m_ReactorPool.InitReactorChannelEventPool(DEFAULT_INIT_EVENT_POOLS);
            m_ReactorPool.InitReactorEventImplPool(DEFAULT_INIT_EVENT_POOLS);
            m_ReactorPool.InitReactorChannelPool(DEFAULT_INIT_EVENT_POOLS);
            m_ReactorPool.InitReactorMsgEventImplPool(DEFAULT_INIT_EVENT_POOLS);
            m_ReactorPool.InitReactorRDMLoginMsgEventImplPool(DEFAULT_INIT_EVENT_POOLS);
            m_ReactorPool.InitReactorRDMDirectoryMsgEventImplPool(DEFAULT_INIT_EVENT_POOLS);
            m_ReactorPool.InitReactorRDMDictionaryMsgEventImplPool(DEFAULT_INIT_EVENT_POOLS);

            m_ReactorWorker = new ReactorWorker(this);

            if (m_ReactorWorker.InitReactorWorker(out errorInfo) != ReactorReturnCode.SUCCESS)
            {
                return errorInfo!.Code;
            }

            return ReactorReturnCode.SUCCESS;
        }

        internal static ReactorReturnCode PopulateErrorInfo(out ReactorErrorInfo? errorInfo, ReactorReturnCode reactorReturnCode, 
            string location, Error error)
        {
            errorInfo = new ReactorErrorInfo
            {
                Code = reactorReturnCode,
                Location = location,
                Error = error,
            };

            return reactorReturnCode;
        }

        internal static ReactorReturnCode PopulateErrorInfo(out ReactorErrorInfo errorInfo, ReactorReturnCode reactorReturnCode,
            string location, string text)
        {
            errorInfo = new ReactorErrorInfo
            {
                Code = reactorReturnCode,
                Location = location,
                Error = new Error
                {
                    Text = text
                }
            };

            return reactorReturnCode;
        }

        internal static ReactorReturnCode PopulateErrorInfo(ReactorErrorInfo errorInfo, ReactorReturnCode reactorReturnCode,
            string location, string text)
        {
            errorInfo.Code = reactorReturnCode;
            errorInfo.Location = location;
            errorInfo.Error.Text = text;

            return reactorReturnCode;
        }

        internal ReactorReturnCode CloseChannel(ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ReactorLock.EnterWriteLock();

            try
            {
                if(reactorChannel is null)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.CloseChannel",
                        "reactorChannel cannot be null");
                }
                else if (m_ReactorActive == false)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "Reactor.CloseChannel",
                        "Reactor is shutdown, CloseChannel ignored");
                }
                else if (reactorChannel.State == ReactorChannelState.CLOSED)
                {
                    return ReactorReturnCode.SUCCESS;
                }

                // set the ReactorChannel's state to CLOSED.
                // and remove it from the queue.
                reactorChannel.State = ReactorChannelState.CLOSED;

                m_ReactorChannelQueue.Remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

                if(SendReactorImplEvent(ReactorEventImpl.ImplType.CHANNEL_CLOSE, reactorChannel) != ReactorReturnCode.SUCCESS)
                {
                    // SendReactorImplEvent() failed, send channel down
                    reactorChannel.State = ReactorChannelState.DOWN;
                    SendAndHandleChannelEventCallback("Reactor.CloseChannel",
                            ReactorChannelEventType.CHANNEL_DOWN,
                            reactorChannel, errorInfo);
                    return PopulateErrorInfo(out errorInfo,
                            ReactorReturnCode.FAILURE,
                            "Reactor.CloseChannel",
                            "SendReactorImplEvent() failed");
                }
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            return ReactorReturnCode.SUCCESS;
        }

        internal ReactorPool GetReactorPool()
        {
            return m_ReactorPool;
        }

        internal ReactorEventQueue GetReactorEventQueue()
        {
            return m_ReactorEventQueue!;
        }


        internal ReactorReturnCode SubmitChannel(ReactorChannel reactorChannel, ITransportBuffer buffer, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode = ReactorReturnCode.SUCCESS;
            ITransportBuffer writeBuffer = buffer;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!IsReactorChannelReady(reactorChannel))
                {
                    reactorReturnCode = ReactorReturnCode.FAILURE;
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.SubmitChannel", "ReactorChannel is closed, aborting.");
                }
                else
                {
                    if (m_ReactorOptions.XmlTracing)
                    {
                        Error dumpError;
                        m_XmlString.Length = 0;
                        m_XmlString
                                .Append("\n<!-- Outgoing Reactor message -->\n")
                                .Append("<!-- ").Append(reactorChannel?.Channel?.ToString()).Append(" -->\n")
                                .Append("<!-- ").Append(System.DateTime.Now.ToString()).Append(" -->\n");
                        if (m_XmlTraceDump.DumpBuffer(reactorChannel?.Channel, (int)reactorChannel!.Channel!.ProtocolType, writeBuffer, null, m_XmlString, out dumpError) == TransportReturnCode.SUCCESS)
                        {
                            Console.WriteLine(m_XmlString);
                        }
                        else
                        {
                            Console.WriteLine($"Failed to dump message buffer: {dumpError!.Text}");
                        }
                    }

                    TransportReturnCode transportReturnCode = reactorChannel.Channel!.Write(writeBuffer, submitOptions.WriteArgs, out Error transportError);
                    if (transportReturnCode > TransportReturnCode.SUCCESS
                        || transportReturnCode == TransportReturnCode.WRITE_FLUSH_FAILED
                        || transportReturnCode == TransportReturnCode.WRITE_CALL_AGAIN)
                    {
                        if (SendFlushRequest(reactorChannel, "Reactor.SubmitChannel", out errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            return ReactorReturnCode.FAILURE;
                        }

                        if (transportReturnCode != TransportReturnCode.WRITE_CALL_AGAIN)
                        {
                            reactorReturnCode = ReactorReturnCode.SUCCESS;
                        }
                        else
                        {
                            reactorReturnCode = ReactorReturnCode.WRITE_CALL_AGAIN;
                        }
                    }
                    else if (transportReturnCode < TransportReturnCode.SUCCESS)
                    {
                        PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                "Reactor.SubmitChannel",
                                $"channel write failure chnl={reactorChannel.Channel}, errorId={transportError.ErrorId}, errorText={transportError.Text}");

                        reactorReturnCode = ReactorReturnCode.FAILURE;
                    }
                    else
                    {
                        reactorChannel.FlushAgain = false;
                    }
                }
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            // update ping handler for message sent
            if (reactorReturnCode == ReactorReturnCode.SUCCESS)
            {
                reactorChannel.GetPingHandler().SentMsg();
            }

            errorInfo = null;
            return reactorReturnCode;
        }

        internal ReactorReturnCode SubmitChannel(ReactorChannel reactorChannel, Msg msg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode = ReactorReturnCode.SUCCESS;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!IsReactorChannelReady(reactorChannel))
                {
                    reactorReturnCode = ReactorReturnCode.FAILURE;
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.SubmitChannel", "ReactorChannel is closed, aborting.");
                }
                else
                {
                    // check first if write for Msg is pending
                    if (m_SubmitMsgMap.Count > 0 && m_SubmitMsgMap.ContainsKey(msg))
                    {
                        ITransportBuffer writeBuffer = m_SubmitMsgMap[msg];
                        reactorReturnCode = SubmitChannel(reactorChannel, writeBuffer, submitOptions, out errorInfo);
                        if (reactorReturnCode != ReactorReturnCode.WRITE_CALL_AGAIN)
                        {
                            m_SubmitMsgMap.Remove(msg);
                        }
                        return reactorReturnCode;
                    }
                    // Msg not pending - proceed
                    int msgSize = EncodedMsgSize(msg);
                    while (true) // try to get buffer and encode until success or error
                    {
                        Error transportError;
                        ITransportBuffer writeBuffer = reactorChannel.Channel!.GetBuffer(msgSize, false, out transportError);

                        if (writeBuffer != null)
                        {
                            m_EncodeIterator.Clear();
                            m_EncodeIterator.SetBufferAndRWFVersion(writeBuffer, reactorChannel.Channel.MajorVersion, reactorChannel.Channel.MinorVersion);
                            CodecReturnCode codecReturnCode = msg.Encode(m_EncodeIterator);
                            if (codecReturnCode == CodecReturnCode.SUCCESS)
                            {
                                reactorReturnCode = SubmitChannel(reactorChannel, writeBuffer, submitOptions, out errorInfo);
                                // add to pending Msg write map if return code is WRITE_CALL_AGAIN
                                if (reactorReturnCode == ReactorReturnCode.WRITE_CALL_AGAIN)
                                {
                                    m_SubmitMsgMap.Add(msg, writeBuffer);
                                }
                                break;
                            }
                            else if (codecReturnCode == CodecReturnCode.BUFFER_TOO_SMALL) // resize buffer and try again
                            {
                                // release buffer that's too small
                                reactorChannel.Channel.ReleaseBuffer(writeBuffer, out transportError);
                                // resize
                                msgSize *= 2;
                                continue;
                            }
                            else // encoding failure
                            {
                                // release buffer that caused encoding failure
                                reactorChannel.Channel.ReleaseBuffer(writeBuffer, out transportError);

                                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.SubmitChannel",
                                    $"message encoding failure chnl={reactorChannel.Channel}, errorId={codecReturnCode.GetAsString()}");

                                reactorReturnCode = ReactorReturnCode.FAILURE;
                                break;
                            }
                        }
                        else // return NO_BUFFERS
                        {
                            if (SendFlushRequest(reactorChannel, "Reactor.SubmitChannel", out errorInfo) != ReactorReturnCode.SUCCESS)
                            {
                                return ReactorReturnCode.FAILURE;
                            }

                            PopulateErrorInfo(out errorInfo, ReactorReturnCode.NO_BUFFERS, "Reactor.SubmitChannel",
                                    $"channel out of buffers chnl={reactorChannel.Channel}, errorId={transportError!.ErrorId}, errorText={transportError!.Text}");

                            reactorReturnCode = ReactorReturnCode.NO_BUFFERS;
                            break;
                        }
                    }
                }
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            return reactorReturnCode;
        }

        public ReactorReturnCode SubmitChannel(ReactorChannel reactorChannel, MsgBase rdmMsg, ReactorSubmitOptions submitOptions, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode = ReactorReturnCode.SUCCESS;
            CodecReturnCode codecReturnCode;

            ReactorLock.EnterWriteLock();

            try
            {
                if (!m_ReactorActive)
                {
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.SHUTDOWN, "Reactor.SubmitChannel", "Reactor is not active, aborting.");
                }
                if (!IsReactorChannelReady(reactorChannel))
                {
                    reactorReturnCode = ReactorReturnCode.FAILURE;
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.SubmitChannel", "ReactorChannel is closed, aborting.");
                }
                else
                {
                    // check first if write for Msg is pending
                    if (m_SubmitRdmMsgMap.Count > 0 && m_SubmitRdmMsgMap.ContainsKey(rdmMsg))
                    {
                        ITransportBuffer writeBuffer = m_SubmitRdmMsgMap[rdmMsg];
                        reactorReturnCode = SubmitChannel(reactorChannel, writeBuffer, submitOptions, out errorInfo);
                        if (reactorReturnCode != ReactorReturnCode.WRITE_CALL_AGAIN)
                        {
                            m_SubmitRdmMsgMap.Remove(rdmMsg);
                        }
                        return reactorReturnCode;
                    }
                    // Msg not pending - proceed
                    int bufferSize;
                    if ((bufferSize = GetMaxFragmentSize(reactorChannel, out errorInfo)) < 0)
                    {
                        return (ReactorReturnCode)bufferSize;
                    }

                    while (true) // try to get buffer and encode until success or error
                    {
                        Error transportError;
                        ITransportBuffer writeBuffer = reactorChannel.Channel!.GetBuffer(bufferSize, false, out transportError);

                        if (writeBuffer != null)
                        {
                            m_EncodeIterator.Clear();
                            m_EncodeIterator.SetBufferAndRWFVersion(writeBuffer, reactorChannel.Channel.MajorVersion, reactorChannel.Channel.MinorVersion);
                            codecReturnCode = rdmMsg.Encode(m_EncodeIterator);
                            if (codecReturnCode == CodecReturnCode.SUCCESS)
                            {
                                reactorReturnCode = SubmitChannel(reactorChannel, writeBuffer, submitOptions, out errorInfo);
                                // add to pending Msg write map if return code is WRITE_CALL_AGAIN
                                if (reactorReturnCode == ReactorReturnCode.WRITE_CALL_AGAIN)
                                {
                                    m_SubmitRdmMsgMap.Add(rdmMsg, writeBuffer);
                                }
                                break;
                            }
                            else if (codecReturnCode == CodecReturnCode.BUFFER_TOO_SMALL) // resize buffer and try again
                            {
                                // release buffer that's too small
                                reactorChannel.Channel.ReleaseBuffer(writeBuffer, out transportError);
                                // resize
                                bufferSize *= 2;
                                continue;
                            }
                            else // encoding failure
                            {
                                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, "Reactor.SubmitChannel",
                                    $"message encoding failure chnl={reactorChannel.Channel}, errorId={codecReturnCode.GetAsString()}");
                                // release buffer that caused encoding failure
                                reactorChannel.Channel.ReleaseBuffer(writeBuffer, out transportError);

                                reactorReturnCode = ReactorReturnCode.FAILURE;
                                break;
                            }
                        }
                        else // return NO_BUFFERS
                        {
                            if (SendFlushRequest(reactorChannel, "Reactor.SubmitChannel", out errorInfo) != ReactorReturnCode.SUCCESS)
                            {
                                return ReactorReturnCode.FAILURE;
                            }

                            PopulateErrorInfo(out errorInfo, ReactorReturnCode.NO_BUFFERS, "Reactor.SubmitChannel",
                                    $"channel out of buffers chnl={reactorChannel.Channel}, errorId={transportError!.ErrorId}, errorText={transportError!.Text}");

                            reactorReturnCode = ReactorReturnCode.NO_BUFFERS;
                            break;
                        }
                    }
                }
            }
            finally
            {
                ReactorLock.ExitWriteLock();
            }

            errorInfo = null;
            return reactorReturnCode;
        }

        private ReactorReturnCode PerformChannelRead(ReactorChannel reactorChannel, ReadArgs readArgs, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ITransportBuffer msgBuf = reactorChannel.Channel!.Read(readArgs, out Error error);

            reactorChannel.ReadRet = readArgs.ReadRetVal;

            if (msgBuf != null)
            {
                if (m_ReactorOptions.XmlTracing)
                {
                    m_XmlString.Length = 0;
                    m_XmlString
                            .Append("\n<!-- Incoming Reactor message -->\n")
                    .Append("<!-- ").Append(reactorChannel.Channel).Append(" -->\n")
                    .Append("<!-- ").Append(System.DateTime.Now).Append(" -->\n");
                    m_XmlTraceDump.DumpBuffer(reactorChannel.Channel, (int)reactorChannel.Channel.ProtocolType, msgBuf, null, m_XmlString, out error);
                    Console.WriteLine(m_XmlString);
                }

                reactorChannel.GetPingHandler().ReceivedMsg();

                // inspect the message and dispatch it to the application.
                ReactorReturnCode retval = ProcessRwfMessage(msgBuf, null, reactorChannel, out errorInfo);
                if (retval != ReactorReturnCode.SUCCESS)
                {
                    return retval;
                }
            }
            else
            {
                if (readArgs.ReadRetVal == TransportReturnCode.FAILURE)
                {
                    if (reactorChannel.Server is null && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                    {
                        reactorChannel.State = ReactorChannelState.DOWN_RECONNECTING;
                    }
                    else // server channel or no more retries
                    {
                        reactorChannel.State = ReactorChannelState.DOWN;
                    }

                    if (reactorChannel.Server is null && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                    {
                        // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                        SendAndHandleChannelEventCallback("Reactor.PerformChannelRead",
                                ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING,
                                reactorChannel, errorInfo);
                    }
                    else // server channel or no more retries
                    {
                        // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                        SendAndHandleChannelEventCallback("Reactor.PerformChannelRead",
                                ReactorChannelEventType.CHANNEL_DOWN,
                                reactorChannel, errorInfo);
                    }

                }
                else if (readArgs.ReadRetVal == TransportReturnCode.READ_FD_CHANGE)
                {
                    reactorChannel.SetChannel(reactorChannel.Channel);

                    // send FD_CHANGE WorkerEvent to Worker.
                    if (SendReactorImplEvent(ReactorEventImpl.ImplType.FD_CHANGE, reactorChannel) != ReactorReturnCode.SUCCESS)
                    {
                        // sendWorkerEvent() failed, send channel down
                        reactorChannel.State = ReactorChannelState.DOWN;
                        SendAndHandleChannelEventCallback("Reactor.PerformChannelRead",
                                ReactorChannelEventType.CHANNEL_DOWN,
                                reactorChannel, errorInfo);

                        errorInfo = new ReactorErrorInfo();

                        return PopulateErrorInfo(errorInfo,
                                ReactorReturnCode.FAILURE,
                                "Reactor.PerformChannelRead",
                                "sendWorkerEvent() failed");
                    }

                    // send FD_CHANGE to user app via reactorChannelEventCallback.
                    SendAndHandleChannelEventCallback("Reactor.PerformChannelRead",
                            ReactorChannelEventType.FD_CHANGE,
                            reactorChannel, errorInfo);
                }
                else if (readArgs.ReadRetVal == TransportReturnCode.READ_PING)
                {
                    /* PingHandler.m_ReceivedRemoteMsg is set to true from the following call as well */
                    reactorChannel.GetPingHandler().ReceivedPing();
                }
            }

            if (readArgs.ReadRetVal > 0)
            {
                return (ReactorReturnCode)readArgs.ReadRetVal;
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode ProcessRwfMessage(ITransportBuffer transportBuffer, Buffer? buffer, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            // inspect the message and dispatch it to the application.
            m_DecodeIterator.Clear();

            if (buffer is not null)
            {
                m_DecodeIterator.SetBufferAndRWFVersion(buffer, reactorChannel.Channel!.MajorVersion, reactorChannel.Channel.MinorVersion);
            }
            else
            {
                m_DecodeIterator.SetBufferAndRWFVersion(transportBuffer, reactorChannel.Channel!.MajorVersion, reactorChannel.Channel.MinorVersion);
            }

            m_Msg.Clear();
            CodecReturnCode codecReturnCode = m_Msg.Decode(m_DecodeIterator);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    "Reactor.PerformChannelRead",
                    $"initial decode of msg failed: {codecReturnCode.GetAsString()}");
            }

            return ProcessChannelMessage(reactorChannel, m_DecodeIterator, m_Msg, transportBuffer, out errorInfo);
        }

        ReactorReturnCode ProcessChannelMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, ITransportBuffer transportBuffer, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode reactorReturnCode;

            switch (msg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    reactorReturnCode = ProcessLoginMessage(reactorChannel, dIter, msg, transportBuffer, out errorInfo);
                    break;
                case (int)DomainType.SOURCE:
                    reactorReturnCode = ProcessDirectoryMessage(reactorChannel, dIter, msg, transportBuffer, out errorInfo);
                    break;
                case (int)DomainType.DICTIONARY:
                    reactorReturnCode = ProcessDictionaryMessage(reactorChannel, dIter, msg, transportBuffer, out errorInfo);
                    break;
                default:
                    reactorReturnCode = (SendAndHandleDefaultMsgCallback("Reactor.ProcessChannelMessage", reactorChannel, transportBuffer, msg, out errorInfo)
                                         == ReactorCallbackReturnCode.SUCCESS)
                        ? ReactorReturnCode.SUCCESS
                        : ReactorReturnCode.FAILURE;
                    break;
            }

            return reactorReturnCode;
        }

        private void SendQueryServiceDiscoveryEvent(ReactorServiceDiscoveryOptions options,
            List<ReactorServiceEndpointInfo>? serviceEndPointInfoList, ReactorErrorInfo? errorInfo)
        {
            ReactorServiceEndpointEvent serviceEnpointEvent = m_ReactorPool.CreateReactorServiceEndpointEvent();
            ReactorErrorInfo holder = serviceEnpointEvent.ReactorErrorInfo;

            if(serviceEndPointInfoList != null)
            {
                serviceEnpointEvent.ServiceEndpointInfoList = serviceEndPointInfoList;
            }
            else
            {
                serviceEnpointEvent.ReactorErrorInfo = errorInfo!;
            }

            serviceEnpointEvent.UserSpecObject = options.UserSpecObject;
            options.ReactorServiceEndpointEventCallback!.ReactorServiceEndpointEventCallback(serviceEnpointEvent);
            serviceEnpointEvent.ReactorErrorInfo = holder;
            serviceEnpointEvent.ReturnToPool();
        }

        private ReactorReturnCode ProcessReactorEventImpl(out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            ReactorEventImpl? eventImpl = (ReactorEventImpl?)m_ReactorEventQueue!.GetEventFromQueue();
            if (eventImpl is null)
                return ReactorReturnCode.SUCCESS;

            ReactorEventImpl.ImplType eventType = eventImpl.EventImplType;
            ReactorChannel? reactorChannel = eventImpl.ReactorChannel;
            ReactorReturnCode ret = ReactorReturnCode.SUCCESS;
            ReactorTokenSession? tokenSession;

            switch (eventType)
            {
                case ReactorEventImpl.ImplType.FLUSH_DONE:
                    reactorChannel!.FlushRequested = false;
                    if (reactorChannel.FlushAgain)
                    {
                        /* Channel wrote a message since its last flush request, request flush again
                         * in case that message was not flushed. */
                        if (SendFlushRequest(reactorChannel, "Reactor.ProcessReactorEventImpl", out errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            eventImpl.ReturnToPool();
                            return ReactorReturnCode.FAILURE;
                        }
                    }
                    break;
                case ReactorEventImpl.ImplType.CHANNEL_UP:
                    ProcessChannelUp(eventImpl, out errorInfo);
                    break;
                case ReactorEventImpl.ImplType.CHANNEL_DOWN:
                    if (reactorChannel!.Server is null && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                    {
                        PopulateErrorInfo(out errorInfo, ReactorReturnCode.SUCCESS,
                                "Reactor.ProcessReactorEventImpl",
                                "client channel has connection attempts left");
                    }
                    else // server channel or no more retries
                    {
                        PopulateErrorInfo(out errorInfo, ReactorReturnCode.SUCCESS,
                                "Reactor.ProcessReactorEventImpl",
                                "either a server channel is down or no more retries");
                    }
                    ProcessChannelDown(eventImpl, errorInfo);
                    break;
                case ReactorEventImpl.ImplType.CHANNEL_CLOSE_ACK:
                    /* Worker is done with channel. Safe to release it. */
                    reactorChannel!.ReturnToPool();
                    break;

                case ReactorEventImpl.ImplType.WARNING:
                    /* Override the error code as this is not a failure */
                    PopulateErrorInfo(out errorInfo, ReactorReturnCode.SUCCESS,
                            "Reactor.ProcessReactorEventImpl",
                            "received a Warning, not a failure");

                    SendChannelEventCallback(ReactorChannelEventType.WARNING, reactorChannel!, errorInfo);
                    break;
                case ReactorEventImpl.ImplType.SHUTDOWN:
                    errorInfo = new ReactorErrorInfo();
                    ProcessWorkerShutdown(eventImpl, "Reactor.ProcessReactorEventImpl", errorInfo);
                    break;
                case ReactorEventImpl.ImplType.TOKEN_MGNT:

                    tokenSession = eventImpl.TokenSession;
                    if(tokenSession is not null)
                    {
                        SendAuthTokenEventCallback(reactorChannel!, tokenSession.ReactorAuthTokenInfo, eventImpl.ReactorErrorInfo);
                    }

                    break;
                case ReactorEventImpl.ImplType.TOKEN_CREDENTIAL_RENEWAL:
                    tokenSession = eventImpl.TokenSession;
                    if (tokenSession is not null)
                    {
                        m_TokenSessionRenewalCallback = tokenSession;
                        SendOAuthCredentialEventCallback(tokenSession, eventImpl.ReactorErrorInfo);
                        m_TokenSessionRenewalCallback = null;
                    }
                    break;
                default:
                    eventImpl.ReturnToPool();
                    return PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        "Reactor.ProcessReactorEventImpl",
                        "received an unexpected WorkerEventType of " + eventType);
            }

            eventImpl.ReturnToPool();
            return ret;
        }

        /* Request that the Worker start flushing this channel.  */
        private ReactorReturnCode SendFlushRequest(ReactorChannel reactorChannel, string location, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;

            if (reactorChannel.FlushRequested)
                reactorChannel.FlushAgain = true; /* Flush already in progress; wait till FLUSH_DONE is received, then request again. */
            else
            {
                if (SendReactorImplEvent(ReactorEventImpl.ImplType.FLUSH, reactorChannel) != ReactorReturnCode.SUCCESS)
                {
                    // SendReactorImplEvent() failed, send channel down
                    var retCode = PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, location,
                            "SendReactorImplEvent() failed while requesting flush");
                    reactorChannel.State = ReactorChannelState.DOWN;

                    SendAndHandleChannelEventCallback(location,
                            ReactorChannelEventType.CHANNEL_DOWN,
                            reactorChannel, errorInfo);

                    return retCode;
                }

                reactorChannel.FlushAgain = false;
                reactorChannel.FlushRequested = true;
            }

            return ReactorReturnCode.SUCCESS;
        }

        private void ProcessChannelDown(ReactorEventImpl eventImpl, ReactorErrorInfo errorInfo)
        {
            ReactorChannel reactorChannel = eventImpl.ReactorChannel!;

            if (reactorChannel.State != ReactorChannelState.CLOSED)
            {
                if (reactorChannel.Server is null && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                {
                    // send CHANNEL_DOWN_RECONNECTING
                    reactorChannel.State = ReactorChannelState.DOWN_RECONNECTING;

                    // send channel_down to user app via reactorChannelEventCallback.
                    SendAndHandleChannelEventCallback("Reactor.ProcessChannelDown",
                            ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING,
                            reactorChannel, errorInfo);
                }
                else // server channel or no more retries
                {
                    // send CHANNEL_DOWN since server channels are not recovered
                    reactorChannel.State = ReactorChannelState.DOWN;

                    // send channel_down to user app via reactorChannelEventCallback.
                    SendAndHandleChannelEventCallback("Reactor.ProcessChannelDown",
                        ReactorChannelEventType.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
                }
            }
        }

        private void ProcessChannelUp(ReactorEventImpl eventImpl, out ReactorErrorInfo? errorInfo)
        {
            errorInfo = null;
            ReactorChannel reactorChannel = eventImpl.ReactorChannel!;
            ReactorRole reactorRole = reactorChannel.Role!;

            if (reactorChannel.State == ReactorChannelState.CLOSED|| reactorChannel.State == ReactorChannelState.DOWN)
            {
                return;
            }

            // update ReactorChannel's state to ACTIVE.
            reactorChannel.State = ReactorChannelState.UP;

            // If channel has no watchlist, consider connection established and reset the
            // reconnect timer.
            reactorChannel.ResetReconnectTimers();

            // send channel_up to user app via reactorChannelEventCallback.
            if (SendAndHandleChannelEventCallback("Reactor.ProcessChannelUp",
                    ReactorChannelEventType.CHANNEL_UP, reactorChannel,
                    errorInfo) != ReactorReturnCode.SUCCESS)
            {
                return;
            }

            // check role and start sending predefined messages (if specified).
            // if none are specified, send channel_ready.
            switch (reactorRole)
            {
                case ConsumerRole consumerRole:
                    {
                        if (reactorChannel.State == ReactorChannelState.CLOSED || reactorChannel.State == ReactorChannelState.DOWN)
                        {
                            return;
                        }

                        LoginRequest? loginRequest = null;

                        if(reactorChannel.EnableSessionManagement())
                        {
                            loginRequest = reactorChannel.RDMLoginRequestRDP;
                        }
                        else
                        {
                            loginRequest = consumerRole.RdmLoginRequest;
                        }

                        if (loginRequest != null)
                        {
                            EncodeAndWriteLoginRequest(loginRequest, reactorChannel, out errorInfo);
                        }
                        else
                        {
                            // no rdmLoginRequest defined, so just send CHANNEL_READY
                            reactorChannel.State = ReactorChannelState.READY;
                            if (SendAndHandleChannelEventCallback("Reactor.ProcessChannelReady", ReactorChannelEventType.CHANNEL_READY,
                                                                  reactorChannel, errorInfo)
                                != ReactorReturnCode.SUCCESS)
                            {
                                return;
                            }
                        }
                        break;
                    }
                case NIProviderRole niProviderRole:
                    {
                        if (reactorChannel.State == ReactorChannelState.CLOSED || reactorChannel.State == ReactorChannelState.DOWN)
                            return;

                        // multi-credential authentication: LoginRequest provided for the Channel overrides "global" LoginRequest
                        // specified in the Role
                        LoginRequest? loginRequest = niProviderRole.RdmLoginRequest;

                        if (loginRequest != null)
                        {
                            // a rdmLoginRequest was specified, send it out.
                            EncodeAndWriteLoginRequest(loginRequest, reactorChannel, out errorInfo);
                        }
                        else
                        {
                            // no rdmLoginRequest defined, so just send CHANNEL_READY
                            reactorChannel.State = ReactorChannelState.READY;
                            if (SendAndHandleChannelEventCallback("Reactor.ProcessChannelUp", ReactorChannelEventType.CHANNEL_READY,
                                                                  reactorChannel, errorInfo)
                                != ReactorReturnCode.SUCCESS)
                            {
                                return;
                            }
                        }
                        break;
                    }
                case ProviderRole:
                    {
                        // send CHANNEL_READY
                        reactorChannel.State = ReactorChannelState.READY;
                        if (SendAndHandleChannelEventCallback("Reactor.ProcessChannelUp", ReactorChannelEventType.CHANNEL_READY,
                                                              reactorChannel, errorInfo)
                            != ReactorReturnCode.SUCCESS)
                        {
                            return;
                        }
                        break;
                    }
            }
        }

        private void EncodeAndWriteLoginRequest(LoginRequest loginRequest, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the login request
            IChannel? channel = reactorChannel.Channel;
            if (channel == null)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                  "Reactor.EncodeAndWriteLoginRequest",
                                  "Failed to obtain an action channel");
                return;
            }

            int bufSize;
            if ((bufSize = GetMaxFragmentSize(reactorChannel, out errorInfo)) < 0)
                return;

            ITransportBuffer msgBuf = channel.GetBuffer(bufSize, false, out var bufferError);
            if (msgBuf == null)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                                  "Reactor.EncodeAndWriteLoginRequest",
                                  $"Failed to obtain a TransportBuffer, reason={bufferError?.Text}");
                return;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

            CodecReturnCode retval = loginRequest.Encode(m_EncodeIterator);
            if (retval != CodecReturnCode.SUCCESS)
            {
                // set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo)
                reactorChannel.State = ReactorChannelState.DOWN;
                SendAndHandleChannelEventCallback("Reactor.EncodeAndWriteLoginRequest",
                                                  ReactorChannelEventType.CHANNEL_DOWN,
                                                  reactorChannel, errorInfo);
                PopulateErrorInfo(out errorInfo,
                                  ReactorReturnCode.FAILURE,
                                  "Reactor.encodeAndWriteLoginRequest",
                                  $"Encoding of login request failed: <{retval}>");
                return;
            }

            if (m_ReactorOptions.XmlTracing) {
                m_XmlString.Length = 0;
                m_XmlString
                    .Append("\n<!-- Outgoing Reactor message -->\n")
                    .Append("<!-- ").Append(reactorChannel.Channel).Append(" -->\n")
                    .Append("<!-- ").Append(System.DateTime.Now).Append(" -->\n");
                if (m_XmlTraceDump.DumpBuffer(reactorChannel.Channel,
                    (int)reactorChannel.Channel!.ProtocolType, msgBuf, null, m_XmlString, out var dumpError)
                        == TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine(m_XmlString);
                }
                else
                {
                    Console.WriteLine($"Failed to dump message buffer: {dumpError?.Text}");
                }
            }

            var writeStatus = channel.Write(msgBuf, m_WriteArgs, out var writeError);

            // TODO: implement real code
            // Aggregate number of bytes written
            //if(m_ReactorOptions.WriteStatSet == true)
            //{
            //    ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(),_writeArgs.bytesWritten()));
            //    ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(),_writeArgs.uncompressedBytesWritten()));
            //}

            if (writeStatus > TransportReturnCode.SUCCESS)
            {
                SendFlushRequest(reactorChannel, "Reactor.EncodeAndWriteLoginRequest", out errorInfo);
            }
            else if (writeStatus < TransportReturnCode.SUCCESS)
            {
                // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
                // for user application.
                // also, set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo))
                if (reactorChannel.Server == null 
                    && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                {
                    if(errorInfo is null)
                    {
                        errorInfo = new ReactorErrorInfo();
                    }

                    errorInfo.Error = writeError;
                    reactorChannel.State = ReactorChannelState.DOWN_RECONNECTING;
                    SendAndHandleChannelEventCallback("Reactor.EncodeAndWriteLoginRequest",
                                                      ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING,
                                                      reactorChannel, errorInfo);
                }
                else // server channel or no more retries
                {
                    if (errorInfo is null)
                    {
                        errorInfo = new ReactorErrorInfo();
                    }

                    errorInfo.Error = writeError;

                    reactorChannel.State = ReactorChannelState.DOWN;
                    SendAndHandleChannelEventCallback("Reactor.EncodeAndWriteLoginRequest",
                                                      ReactorChannelEventType.CHANNEL_DOWN,
                                                      reactorChannel, errorInfo);
                }
                PopulateErrorInfo(errorInfo,
                                  ReactorReturnCode.FAILURE,
                                  "Reactor.EncodeAndWriteLoginRequest",
                                  $"Channel.write failed to write login request: <{writeStatus}> error={errorInfo.Error.Text}");
            }
            else
                reactorChannel.FlushAgain = false;
        }

        ///
        ///
        /// <param name="msg">generic msg which should be decoded and handled.</param>
        /// <param name="reactorChannel">channel connection between client and server.</param>
        /// <param name="errorInfo">Warning/Error information buffer.</param>
        /// <param name="decodeIterator">An iterator for decoding the RWF content.</param>
        /// 
        /// <returns>true when message must be proceeded. Returns false if result of this method should be ignored.
        /// For instance: when <see cref="LoginMsg.LoginMsgType"/> is <see cref="LoginMsgType.RTT"/> and RTT messaging
        /// is not supported by a <see cref="ConsumerRole"/></returns> 
        private bool ProceedLoginGenericMsg(ReactorChannel reactorChannel, DecodeIterator decodeIterator,
                                           Msg msg, out ReactorErrorInfo? errorInfo)
        {
            LoginMsg loginGenericMsg = m_LoginMsg;
            if (DataTypes.ELEMENT_LIST == msg.ContainerType)
            {
                loginGenericMsg.LoginMsgType = LoginMsgType.RTT;
                switch (reactorChannel.Role)
                {
                    case ProviderRole:
                        break;

                    case ConsumerRole consumerRole:
                        if (consumerRole.RTTEnabled)
                        {
                            ReturnBackRTTMessage(msg, reactorChannel, out errorInfo);
                            break;
                        }
                        else
                        {
                            errorInfo = null;
                            return false;
                        }
                    default:
                        {
                            /* return false when it is not enabled for consumer or when it is NIProvider
                             * for preventing further handling */
                            errorInfo = null;
                            return false;
                        }
                }
            }
            else
            {
                loginGenericMsg.LoginMsgType = LoginMsgType.CONSUMER_CONNECTION_STATUS;
            }
            loginGenericMsg.Decode(decodeIterator, msg);
            errorInfo = null;
            return true;
        }


        private void ReturnBackRTTMessage(Msg msg, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            m_ReactorSubmitOptions.Clear();
            ReactorReturnCode retval = SubmitChannel(reactorChannel, msg, m_ReactorSubmitOptions, out errorInfo);

            if (retval != ReactorReturnCode.SUCCESS)
            {
                PopulateErrorInfo(out errorInfo,
                        ReactorReturnCode.FAILURE,
                        "Reactor.ReturnBackRTTMessage",
                        $"Reactor.SubmitChannel failed to return back login RTT message: <{retval}> error={errorInfo?.Error.Text}");
            }
        }


        private ReactorReturnCode ProcessLoginMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg,
            ITransportBuffer transportBuffer, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode retval = ReactorReturnCode.SUCCESS;
            LoginMsg loginMsg = new();
            errorInfo = null;

            //loginMsg.Clear();
            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    loginMsg.LoginMsgType = LoginMsgType.REQUEST;
                    loginMsg.LoginRequest!.Decode(dIter, msg);
                    break;
                case MsgClasses.REFRESH:
                    loginMsg.LoginMsgType = LoginMsgType.REFRESH;
                    loginMsg.LoginRefresh!.Decode(dIter, msg);
                    break;
                case MsgClasses.STATUS:
                    loginMsg.LoginMsgType = LoginMsgType.STATUS;
                    loginMsg.LoginStatus!.Decode(dIter, msg);
                    break;
                case MsgClasses.CLOSE:
                    loginMsg.LoginMsgType = LoginMsgType.CLOSE;
                    loginMsg.LoginClose!.Decode(dIter, msg);
                    break;
                case MsgClasses.GENERIC:
                    if (!ProceedLoginGenericMsg(reactorChannel, dIter, msg, out errorInfo))
                        return ReactorReturnCode.SUCCESS;
                    loginMsg = m_LoginMsg;
                    break;
                default:
                    break;
            }

            if (retval != ReactorReturnCode.FAILURE)
            {
                var callbackStatus = SendAndHandleLoginMsgCallback("Reactor.ProcessLoginMessage", reactorChannel, transportBuffer, msg,
                    loginMsg, out errorInfo);

                if (callbackStatus == ReactorCallbackReturnCode.RAISE)
                    callbackStatus = SendAndHandleDefaultMsgCallback("Reactor.ProcessLoginMessage", reactorChannel, transportBuffer, msg,
                        out errorInfo);

                if (callbackStatus == ReactorCallbackReturnCode.SUCCESS)
                {
                    /*
                     * check if this is a reactorChannel's role is CONSUMER, a Login REFRESH, if the reactorChannel State is UP,
                     * and that the loginRefresh's state was OK.
                     * If all this is true, check if a directoryRequest is populated. If so, send the directoryRequest. if not, change the
                     * reactorChannel state to READY.
                     */
                    if (reactorChannel.State == ReactorChannelState.UP
                        && reactorChannel.Role is ConsumerRole consumerRole
                        && consumerRole.RdmLoginRequest != null
                        && msg.StreamId == consumerRole.RdmLoginRequest.StreamId
                        && loginMsg.LoginMsgType == LoginMsgType.REFRESH
                        && loginMsg.LoginRefresh!.State.StreamState() == StreamStates.OPEN
                        && loginMsg.LoginRefresh!.State.DataState() == DataStates.OK)
                    {
                        DirectoryRequest? directoryRequest = consumerRole.RdmDirectoryRequest;
                        if (directoryRequest != null)
                        {
                             // a rdmDirectoryRequest was specified, send it out.
                             EncodeAndWriteDirectoryRequest(directoryRequest, reactorChannel, out errorInfo);
                        }
                        else
                        {
                            // no rdmDirectoryRequest defined, so just send CHANNEL_READY
                            reactorChannel.State = ReactorChannelState.READY;
                            if ((retval = SendAndHandleChannelEventCallback("Reactor.ProcessLoginMessage",
                                ReactorChannelEventType.CHANNEL_READY, reactorChannel, errorInfo))
                                    != ReactorReturnCode.SUCCESS)
                            {
                                return retval;
                            }
                        }
                    }

                    /*
                     * check if this is a reactorChannel's role is NIPROVIDER, a Login REFRESH, if the reactorChannel State is UP,
                     * and that the loginRefresh's state was OK.
                     * If all this is true, check if a directoryRefresh is populated. If so, send the directoryRefresh. if not, change the
                     * reactorChannel state to READY.
                     */
                    if (reactorChannel.State == ReactorChannelState.UP
                        && reactorChannel.Role is NIProviderRole niProviderRole
                        && niProviderRole.RdmLoginRequest != null
                        && msg.StreamId == niProviderRole.RdmLoginRequest.StreamId
                        && loginMsg.LoginMsgType == LoginMsgType.REFRESH
                        && loginMsg.LoginRefresh!.State.StreamState() == StreamStates.OPEN
                        && loginMsg.LoginRefresh!.State.DataState() == DataStates.OK)
                    {

                        DirectoryRefresh? directoryRefresh = niProviderRole.RdmDirectoryRefresh;
                        if (directoryRefresh != null)
                        {
                            // a rdmDirectoryRefresh was specified, send it out.
                            EncodeAndWriteDirectoryRefresh(directoryRefresh, reactorChannel, out errorInfo);
                        }

                        // send CHANNEL_READY
                        reactorChannel.State = ReactorChannelState.READY;
                        if ((retval = SendAndHandleChannelEventCallback("Reactor.ProcessLoginMessage",
                            ReactorChannelEventType.CHANNEL_READY, reactorChannel, errorInfo))
                                != ReactorReturnCode.SUCCESS)
                        {
                            return retval;
                        }
                    }
                }
            }

            return retval;
        }

        private void ProcessWorkerShutdown(ReactorEventImpl eventImpl, string location, ReactorErrorInfo errorInfo)
        {
            PopulateErrorInfo(errorInfo, ReactorReturnCode.FAILURE, location, "Worker has shutdown, "
                                                                               + eventImpl.ReactorErrorInfo.ToString());
        }

        internal ReactorReturnCode SendReactorImplEvent(ReactorEventImpl.ImplType eventType, ReactorChannel? reactorChannel)
        {
            ReactorReturnCode retCode = ReactorReturnCode.SUCCESS;

            ReactorEventImpl reactorEventImpl = m_ReactorPool.CreateReactorEventImpl();
            reactorEventImpl.EventImplType = eventType;
            reactorEventImpl.ReactorChannel = reactorChannel;
            reactorEventImpl.TokenSession = reactorChannel is not null ? reactorChannel.TokenSession: null;
            if (m_ReactorWorker != null)
            {
                m_ReactorWorker.WorkerEventQueue.PutEventToQueue(reactorEventImpl);
            }
            else
            {
                retCode = ReactorReturnCode.FAILURE;
            }

            return retCode;
        }

        private bool IsReactorChannelReady(ReactorChannel reactorChannel)
        {
            return reactorChannel.State == ReactorChannelState.UP ||
                reactorChannel.State == ReactorChannelState.READY;
        }

        private ReactorReturnCode ProcessDirectoryMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, ITransportBuffer transportBuffer, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode callbackReturnCode;

            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.REQUEST;
                    m_DirectoryMsg.DirectoryRequest!.Decode(dIter, msg);
                    break;
                case MsgClasses.REFRESH:
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;
                    m_DirectoryMsg.DirectoryRefresh!.Decode(dIter, msg);
                    break;
                case MsgClasses.STATUS:
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.STATUS;
                    m_DirectoryMsg.DirectoryStatus!.Decode(dIter, msg);
                    break;
                case MsgClasses.CLOSE:
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.CLOSE;
                    m_DirectoryMsg.DirectoryClose!.Decode(dIter, msg);
                    break;
                case MsgClasses.GENERIC:
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.CONSUMER_STATUS;
                    m_DirectoryMsg.DirectoryConsumerStatus!.Decode(dIter, msg);
                    break;
                case MsgClasses.UPDATE:
                    m_DirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;
                    m_DirectoryMsg.DirectoryUpdate!.Decode(dIter, msg);
                    break;
                default:
                    break;
            }

            callbackReturnCode = SendAndHandleDirectoryMsgCallback("Reactor.ProcessDirectoryMessage", reactorChannel, transportBuffer, msg, m_DirectoryMsg, out errorInfo);

            if (callbackReturnCode == ReactorCallbackReturnCode.RAISE)
            {
                callbackReturnCode = SendAndHandleDefaultMsgCallback("Reactor.ProcessDirectoryMessage", reactorChannel, transportBuffer, msg, out errorInfo);
            }

            if (callbackReturnCode == ReactorCallbackReturnCode.SUCCESS)
            {
                // Check if this is a reactorChannel's role is CONSUMER, a Directory REFRESH, and if the reactorChannel State is UP.
                // If all this is true, check DictionaryDownloadMode is FIRST_AVAILABLE.
                ReactorRole? reactorRole = reactorChannel.Role;
                if (reactorChannel.State == ReactorChannelState.UP
                    && msg.StreamId == ((ConsumerRole)reactorRole!).RdmDirectoryRequest!.StreamId
                    && reactorChannel.Role is ConsumerRole consumerRole
                    && m_DirectoryMsg.DirectoryMsgType == DirectoryMsgType.REFRESH)
                {
                    if (consumerRole.DictionaryDownloadMode == DictionaryDownloadMode.FIRST_AVAILABLE)
                    {
                        DirectoryRefresh directoryRefresh = m_DirectoryMsg.DirectoryRefresh!;
                        int serviceId = 0;
                        bool hasFieldDictionary = false;
                        bool hasEnumTypeDictionary = false;
                        // find first directory message service that has RWFFld and RWFEnum available
                        foreach (Service service in directoryRefresh.ServiceList)
                        {
                            if (service.HasInfo)
                            {
                                foreach (string dictionaryName in service.Info.DictionariesProvidedList)
                                {
                                    if (dictionaryName.Equals(consumerRole.FieldDictionaryName.ToString()))
                                        hasFieldDictionary = true;

                                    if (dictionaryName.Equals(consumerRole.EnumTypeDictionaryName.ToString()))
                                        hasEnumTypeDictionary = true;

                                    if (hasFieldDictionary && hasEnumTypeDictionary)
                                    {
                                        serviceId = service.ServiceId;
                                        break;
                                    }
                                }
                            }

                            // send field and enum type dictionary requests
                            if (hasFieldDictionary && hasEnumTypeDictionary)
                            {
                                DictionaryRequest dictionaryRequest;

                                consumerRole.InitDefaultRDMFieldDictionaryRequest();
                                dictionaryRequest = consumerRole.RdmFieldDictionaryRequest!;
                                dictionaryRequest.ServiceId = serviceId;
                                EncodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, out errorInfo);

                                consumerRole.InitDefaultRDMEnumDictionaryRequest();
                                dictionaryRequest = consumerRole.RdmEnumTypeDictionaryRequest!;
                                dictionaryRequest.ServiceId = serviceId;
                                EncodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, out errorInfo);

                                break;
                            }
                        }

                        // check if dictionary download not supported by the provider
                        if (!hasFieldDictionary || !hasEnumTypeDictionary)
                        {
                            Console.WriteLine("Dictionary download not supported by the indicated provider");
                        }
                    }
                    else
                    {
                        // dictionaryDownloadMode is NONE, so just send CHANNEL_READY
                        reactorChannel.State = ReactorChannelState.READY;
                        return SendAndHandleChannelEventCallback("Reactor.ProcessDirectoryMessage", ReactorChannelEventType.CHANNEL_READY, reactorChannel, errorInfo);
                    }
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        private ReactorReturnCode ProcessDictionaryMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, ITransportBuffer transportBuffer, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode callbackReturnCode;
            DictionaryRefresh? dictionaryRefresh = null;

            switch (msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    m_DictionaryMsg.DictionaryMsgType = DictionaryMsgType.REQUEST;
                    DictionaryRequest dictionaryRequest = m_DictionaryMsg.DictionaryRequest!;
                    dictionaryRequest.Decode(dIter, msg);
                    break;
                case MsgClasses.REFRESH:
                    m_DictionaryMsg.DictionaryMsgType = DictionaryMsgType.REFRESH;
                    dictionaryRefresh = m_DictionaryMsg.DictionaryRefresh!;
                    dictionaryRefresh.Decode(dIter, msg);
                    break;
                case MsgClasses.STATUS:
                    m_DictionaryMsg.DictionaryMsgType = DictionaryMsgType.STATUS;
                    DictionaryStatus dictionaryStatus = m_DictionaryMsg.DictionaryStatus!;
                    dictionaryStatus.Decode(dIter, msg);
                    break;
                case MsgClasses.CLOSE:
                    m_DictionaryMsg.DictionaryMsgType = DictionaryMsgType.CLOSE;
                    DictionaryClose dictionaryClose = m_DictionaryMsg.DictionaryClose!;
                    dictionaryClose.Decode(dIter, msg);
                    break;
                default:
                    break;
            }

            callbackReturnCode = SendAndHandleDictionaryMsgCallback("Reactor.ProcessDictionaryMessage", reactorChannel, transportBuffer, msg, m_DictionaryMsg, out errorInfo);

            if (callbackReturnCode == ReactorCallbackReturnCode.RAISE)
            {
                callbackReturnCode = SendAndHandleDefaultMsgCallback("Reactor.ProcessDictionaryMessage", reactorChannel, transportBuffer, msg, out errorInfo);
            }

            if (callbackReturnCode == ReactorCallbackReturnCode.SUCCESS)
            {
                bool receivedFieldDictResponse = false;
                bool receivedEnumTypeResponse = false;

                // check if this is a reactorChannel's role is CONSUMER, a Dictionary REFRESH,
                // reactorChannel State is UP, and dictionaryDownloadMode is FIRST_AVAILABLE.
                // If all this is true, close dictionary stream for this refresh.
                if (reactorChannel.State == ReactorChannelState.UP
                    && reactorChannel.Role is ConsumerRole consumerRole
                    && m_DictionaryMsg.DictionaryMsgType == DictionaryMsgType.REFRESH
                    && consumerRole.DictionaryDownloadMode == DictionaryDownloadMode.FIRST_AVAILABLE)
                {
                    // field dictionary
                    if (msg.StreamId == consumerRole.RdmFieldDictionaryRequest!.StreamId
                        && dictionaryRefresh != null
                        && dictionaryRefresh.RefreshComplete)
                    {
                        // Close stream so its streamID is free for the user. When connecting to an ADS,
                        // there won't be any further messages on this stream -- the consumer will be
                        // disconnected if the dictionary version is changed.
                        consumerRole.ReceivedFieldDictionaryResp = true;
                        EncodeAndWriteDictionaryClose(consumerRole.FieldDictionaryClose()!, reactorChannel, out errorInfo);
                    }

                    // enum type dictionary
                    if (msg.StreamId == consumerRole.RdmEnumTypeDictionaryRequest!.StreamId
                        && dictionaryRefresh != null
                        && dictionaryRefresh.RefreshComplete)
                    {
                        // Close stream so its streamID is free for the user.  When connecting to an ADS,  there won't be any further messages on this stream --
                        // the consumer will be disconnected if the dictionary version is changed.
                        consumerRole.ReceivedEnumDictionaryResp = true;
                        EncodeAndWriteDictionaryClose(consumerRole.EnumDictionaryClose()!, reactorChannel, out errorInfo);
                    }
                    receivedFieldDictResponse = consumerRole.ReceivedFieldDictionaryResp;
                    receivedEnumTypeResponse = consumerRole.ReceivedEnumDictionaryResp;
                }
                else if (reactorChannel.State == ReactorChannelState.UP
                    && reactorChannel.Role is NIProviderRole niProviderRole
                    && m_DictionaryMsg.DictionaryMsgType == DictionaryMsgType.REFRESH
                    && niProviderRole.DictionaryDownloadMode == DictionaryDownloadMode.FIRST_AVAILABLE)
                {
                    // field dictionary
                    if (msg.StreamId == niProviderRole.RdmFieldDictionaryRequest!.StreamId && dictionaryRefresh != null && dictionaryRefresh.RefreshComplete)
                    {
                        // Close stream so its streamID is free for the user. When connecting to an ADS,
                        // there won't be any further messages on this stream -- the consumer will be
                        // disconnected if the dictionary version is changed.
                        niProviderRole.ReceivedFieldDictionaryResp = true;
                        EncodeAndWriteDictionaryClose(niProviderRole.RdmFieldDictionaryClose!, reactorChannel, out errorInfo);
                    }

                    // enum type dictionary
                    if (msg.StreamId == niProviderRole.RdmEnumDictionaryRequest!.StreamId
                        && dictionaryRefresh != null
                        && dictionaryRefresh.RefreshComplete)
                    {
                        // Close stream so its streamID is free for the user.  When connecting to an ADS,  there won't be any further messages on this stream --
                        // the consumer will be disconnected if the dictionary version is changed.
                        niProviderRole.ReceivedEnumDictionaryResp = true;
                        EncodeAndWriteDictionaryClose(niProviderRole.RdmEnumDictionaryClose!, reactorChannel, out errorInfo);
                    }

                    receivedFieldDictResponse = niProviderRole.ReceivedFieldDictionaryResp;
                    receivedEnumTypeResponse = niProviderRole.ReceivedEnumDictionaryResp;
                }

                // if both field and enum type refreshes received, send CHANNEL_READY
                if (receivedFieldDictResponse && receivedEnumTypeResponse)
                {
                    reactorChannel.State = ReactorChannelState.READY;
                    return SendAndHandleChannelEventCallback("Reactor.ProcessDictionaryMessage", ReactorChannelEventType.CHANNEL_READY, reactorChannel, errorInfo);
                }
            }

            return ReactorReturnCode.SUCCESS;
        }

        private void EncodeAndWriteDictionaryRequest(DictionaryRequest dictionaryRequest, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            EncodeAndWriteMessage(dictionaryRequest, reactorChannel, "Reactor.EncodeAndWriteDictionaryRequest", out errorInfo);
        }

        private void EncodeAndWriteDictionaryClose(DictionaryClose dictionaryClose, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            EncodeAndWriteMessage(dictionaryClose, reactorChannel, "Reactor.EncodeAndWriteDictionaryClose", out errorInfo);
        }

        private void EncodeAndWriteDirectoryRequest(DirectoryRequest directoryRequest, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            EncodeAndWriteMessage(directoryRequest, reactorChannel, "Reactor.EncodeAndWriteDirectoryRequest", out errorInfo);
        }

        private void EncodeAndWriteDirectoryRefresh(DirectoryRefresh directoryRefresh, ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            EncodeAndWriteMessage(directoryRefresh, reactorChannel, "Reactor.EncodeAndWriteDirectoryRefresh", out errorInfo);
        }

        private void EncodeAndWriteMessage(MsgBase message, ReactorChannel reactorChannel, string location, out ReactorErrorInfo? errorInfo)
        {
            // get a buffer for the dictionary request
            IChannel channel = reactorChannel.Channel!;

            int bufSize;
            if ((bufSize = GetMaxFragmentSize(reactorChannel, out errorInfo)) < 0)
            {
                return;
            }

            Error transpError;
            ITransportBuffer msgBuf = channel.GetBuffer(bufSize, false, out transpError);
            if (msgBuf == null)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        location,
                        $"Failed to obtain a TransportBuffer, reason={transpError!.Text}");
                errorInfo!.Error = transpError;
                return;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion);

            CodecReturnCode codecReturnCode = message.Encode(m_EncodeIterator);
            if (codecReturnCode != CodecReturnCode.SUCCESS)
            {
                // set reactorChannel.State to ReactorChannelState.DOWN and notify the application via ReactorChannelCallback(CHANNEL_DOWN, errorInfo)
                reactorChannel.State = ReactorChannelState.DOWN;
                SendAndHandleChannelEventCallback(location, ReactorChannelEventType.CHANNEL_DOWN, reactorChannel, errorInfo);
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    location,
                    $"Message encoding failed: <{codecReturnCode.GetAsString()}>");
                return;
            }

            Error transportError;
            if (m_ReactorOptions.XmlTracing)
            {
                m_XmlString.Length = 0;
                m_XmlString
                    .Append("\n<!-- Outgoing Reactor message -->\n")
                    .Append("<!-- ").Append(reactorChannel.Channel!.ToString()).Append(" -->\n")
                    .Append("<!-- ").Append(new System.DateTime().ToString()).Append(" -->\n");

                TransportReturnCode dumpReturnCode = m_XmlTraceDump.DumpBuffer(reactorChannel.Channel,
                    (int)reactorChannel.Channel.ProtocolType,
                    msgBuf, null,
                    m_XmlString, out transportError);

                if (dumpReturnCode < TransportReturnCode.SUCCESS)
                {
                    Console.WriteLine($"Failed to dump data to xml: {transportError!.Text}");
                }
                else
                {
                    Console.WriteLine(m_XmlString);
                }
            }

            TransportReturnCode transportReturnCode = channel.Write(msgBuf, m_WriteArgs, out transportError);

            /* // TODO: Aggregate number of bytes written
            if (m_ReactorOptions.WriteStatSet == true)
            {
                ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
                ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
            } */

            if (transportReturnCode > TransportReturnCode.SUCCESS)
            {
                SendFlushRequest(reactorChannel, location, out errorInfo);
            }
            else if (transportReturnCode < TransportReturnCode.SUCCESS)
            {
                // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
                // for user application.
                // also, set reactorChannel.state(State.DOWN) and notify the application (via ReactorChannelCallback(CHANNEL_DOWN, errorInfo))
                if (reactorChannel.Server == null && !reactorChannel.RecoveryAttemptLimitReached()) // client channel
                {
                    reactorChannel.State = ReactorChannelState.DOWN_RECONNECTING;
                    SendAndHandleChannelEventCallback(location,
                            ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING,
                            reactorChannel, errorInfo);
                }
                else // server channel or no more retries
                {
                    reactorChannel.State = ReactorChannelState.DOWN;
                    SendAndHandleChannelEventCallback(location,
                            ReactorChannelEventType.CHANNEL_DOWN,
                            reactorChannel, errorInfo);
                }
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                        location,
                        $"Channel.Write failed to write message: <{transportReturnCode.ToString()}>, error={transportError!.Text}");
            }
            else
            {
                reactorChannel.FlushAgain = false;
            }
        }

        private ReactorCallbackReturnCode SendDirectoryMsgCallback(ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg)
        {
            ReactorCallbackReturnCode retval;
            IDirectoryMsgCallback? callback = null;

            switch (reactorChannel.Role!.Type)
            {
                case ReactorRoleType.CONSUMER:
                    callback = ((ConsumerRole)reactorChannel.Role).DirectoryMsgCallback;
                    break;
                case ReactorRoleType.PROVIDER:
                    callback = ((ProviderRole)reactorChannel.Role).DirectoryMsgCallback;
                    break;
                case ReactorRoleType.NIPROVIDER:
                    // no directory callback for NIProvider.
                    break;
                default:
                    break;
            }

            if (callback != null)
            {
                RDMDirectoryMsgEvent rdmDirectoryMsgEvent = m_ReactorPool.CreateReactorRDMDirectoryMsgEventImpl();
                rdmDirectoryMsgEvent.Clear();
                rdmDirectoryMsgEvent.ReactorChannel = reactorChannel;
                rdmDirectoryMsgEvent.TransportBuffer = transportBuffer;
                rdmDirectoryMsgEvent.Msg = msg;
                rdmDirectoryMsgEvent.DirectoryMsg = directoryMsg;

                retval = callback.RdmDirectoryMsgCallback(rdmDirectoryMsgEvent);
                rdmDirectoryMsgEvent.ReturnToPool();
            }
            else
            {
                // callback is undefined, raise it to DefaultMsgCallback.
                retval = ReactorCallbackReturnCode.RAISE;
            }

            return retval;
        }

        private ReactorCallbackReturnCode SendAndHandleDirectoryMsgCallback(string location, ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode retval = SendDirectoryMsgCallback(reactorChannel, transportBuffer, msg, directoryMsg);

            // check return code from callback.
            if (retval == ReactorCallbackReturnCode.FAILURE)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    location,
                    "ReactorCallbackReturnCode.FAILURE was returned from RdmDirectoryMsgCallback(). This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }
            else if (retval != ReactorCallbackReturnCode.RAISE && retval != ReactorCallbackReturnCode.SUCCESS)
            {
                // retval is not a valid ReactorReturnCodes.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    location,
                    $"Return value of {retval} was returned from RdmDirectoryMsgCallback() and is not a valid ReactorCallbackReturnCode. " +
                    $"This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }

            errorInfo = null;
            return retval;
        }

        private ReactorCallbackReturnCode SendDictionaryMsgCallback(ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg)
        {
            ReactorCallbackReturnCode callbackReturnCode;
            IDictionaryMsgCallback? callback = null;

            switch (reactorChannel.Role!.Type)
            {
                case ReactorRoleType.CONSUMER:
                    callback = ((ConsumerRole)reactorChannel.Role).DictionaryMsgCallback;
                    break;
                case ReactorRoleType.PROVIDER:
                    callback = ((ProviderRole)reactorChannel.Role).DictionaryMsgCallback;
                    break;
                case ReactorRoleType.NIPROVIDER:
                    callback = ((NIProviderRole)reactorChannel.Role).DictionaryMsgCallback;
                    break;
                default:
                    break;
            }

            if (callback != null)
            {
                RDMDictionaryMsgEvent rdmDictionaryMsgEvent = m_ReactorPool.CreateReactorRDMDictionaryMsgEventImpl();
                rdmDictionaryMsgEvent.ReactorChannel = reactorChannel;
                rdmDictionaryMsgEvent.TransportBuffer = transportBuffer;
                rdmDictionaryMsgEvent.Msg = msg;
                rdmDictionaryMsgEvent.DictionaryMsg = dictionaryMsg;

                callbackReturnCode = callback.RdmDictionaryMsgCallback(rdmDictionaryMsgEvent);
                rdmDictionaryMsgEvent.ReturnToPool();
            }
            else
            {
                // callback is undefined, raise it to defaultMsgCallback.
                callbackReturnCode = ReactorCallbackReturnCode.RAISE;
            }

            return callbackReturnCode;
        }

        private ReactorCallbackReturnCode SendAndHandleDictionaryMsgCallback(string location, ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode callbackReturnCode = SendDictionaryMsgCallback(reactorChannel, transportBuffer, msg, dictionaryMsg);

            // check return code from callback.
            if (callbackReturnCode == ReactorCallbackReturnCode.FAILURE)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    location,
                    "ReactorCallbackReturnCode.FAILURE was returned from DictionaryMsgCallback(). This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }
            else if (callbackReturnCode != ReactorCallbackReturnCode.RAISE && callbackReturnCode != ReactorCallbackReturnCode.SUCCESS)
            {
                // return value is not a valid ReactorReturnCodes.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE,
                    location,
                    $"return value of {callbackReturnCode} was returned from DictionaryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }

            errorInfo = null;
            return callbackReturnCode;
        }

        private ReactorCallbackReturnCode SendDefaultMsgCallback(ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg)
        {
            ReactorMsgEvent reactorMsgEvent = m_ReactorPool.CreateReactorMsgEventImpl();
            reactorMsgEvent.ReactorChannel = reactorChannel;
            reactorMsgEvent.TransportBuffer = transportBuffer;
            reactorMsgEvent.Msg = msg;

            ReactorCallbackReturnCode callbackReturnCode = reactorChannel.Role!.DefaultMsgCallback!.DefaultMsgCallback(reactorMsgEvent);
            reactorMsgEvent.ReturnToPool();

            return callbackReturnCode;
        }

        private ReactorCallbackReturnCode SendAndHandleDefaultMsgCallback(string location, ReactorChannel reactorChannel, ITransportBuffer transportBuffer, Msg msg, out ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode callbackReturnCode = SendDefaultMsgCallback(reactorChannel, transportBuffer, msg);

            // check return code from callback.
            if (callbackReturnCode == ReactorCallbackReturnCode.FAILURE)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, 
                    location,
                        "ReactorCallbackReturnCode.FAILURE was returned from DefaultMsgCallback. This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }
            else if (callbackReturnCode == ReactorCallbackReturnCode.RAISE)
            {
                // RAISE is not a valid return code for the DefaultMsgCallback.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, 
                    location, 
                    "ReactorCallbackReturnCode.RAISE is not a valid return code from DefaultMsgCallback. This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }
            else if (callbackReturnCode != ReactorCallbackReturnCode.SUCCESS)
            {
                // return value is not a valid ReactorCallbackReturnCode.
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, 
                    location, 
                    $"return value of {callbackReturnCode} is not a valid ReactorCallbackReturnCode. This caused the Reactor to shutdown.");
                Shutdown(out _);
                return ReactorCallbackReturnCode.FAILURE;
            }

            errorInfo = null;
            return callbackReturnCode;
        }

        private int GetMaxFragmentSize(ReactorChannel reactorChannel, out ReactorErrorInfo? errorInfo)
        {
            ReactorReturnCode ret;
            m_ReactorChannelInfo.Clear();
            if ((ret = reactorChannel.Info(m_ReactorChannelInfo, out errorInfo)) < ReactorReturnCode.SUCCESS)
            {
                return (int)ret;
            }
            return m_ReactorChannelInfo.ChannelInfo.MaxFragmentSize;
        }

        private int EncodedMsgSize(Msg msg)
        {
            int msgSize = 128;
            IMsgKey key = msg.MsgKey;

            msgSize += msg.EncodedDataBody.Length;

            if (key != null)
            {
                if (key.CheckHasName())
                    msgSize += key.Name.Length;

                if (key.CheckHasAttrib())
                    msgSize += key.EncodedAttrib.Length;
            }

            return msgSize;
        }

        // Checks whether the session management is enabled.
        private static bool CheckEnableSessionManagement(ReactorConnectOptions reactorConnectOptions)
        {
            foreach(var connectInfo in reactorConnectOptions.ConnectionList)
            {
                if(connectInfo.EnableSessionManagement)
                {
                    return true;
                }
            }

            return false;
        }

        private static ReactorOAuthCredential? RetrieveOAuthCredentialFromRole(ReactorRole role, out ReactorErrorInfo? errorInfo)
        {
            ReactorOAuthCredential? oauthCredential = null;
            ReactorOAuthCredential oauthCredentialOut;

            if (role.Type == ReactorRoleType.CONSUMER)
            {
                oauthCredential = ((ConsumerRole)role).ReactorOAuthCredential!;
            }
            else if (role.Type == ReactorRoleType.NIPROVIDER)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE, "Reactor.RetrieveOAuthCredentialFromRole",
                        "The session management supports only on the ReactorRoleType.CONSUMER type.");
                return null;
            }

            if (oauthCredential is null)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE, "Reactor.RetrieveOAuthCredentialFromRole",
                        "There is no user credential available for enabling session management.");
                return null;
            }

            if(oauthCredential.ClientId.Length == 0)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE, "Reactor.RetrieveOAuthCredentialFromRole",
                       "Failed to copy OAuth credential for enabling the session management; OAuth client ID does not exist.");
                return null;
            }

            if (oauthCredential.ClientSecret.Length == 0)
            {
                PopulateErrorInfo(out errorInfo, ReactorReturnCode.INVALID_USAGE, "Reactor.RetrieveOAuthCredentialFromRole",
                       "Failed to copy OAuth credential for enabling the session management; OAuth client secret does not exist.");
                return null;
            }

            oauthCredentialOut = new ReactorOAuthCredential();

            oauthCredential.Copy(oauthCredentialOut);

            errorInfo = null;
            return oauthCredentialOut;
        }

        internal static bool RequestServiceDiscovery(ReactorConnectInfo connectInfo)
        {
            if(string.IsNullOrEmpty(connectInfo.ConnectOptions.UnifiedNetworkInfo.Address) &&
                string.IsNullOrEmpty(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName))
            {
                return true;
            }

            return false;
        }

        internal ReactorReturnCode SessionManagementStartup(ReactorTokenSession tokenSession, ReactorConnectInfo reactorConnectInfo,
            ReactorRole role, ReactorChannel reactorChannel, bool isAsysncReq, out ReactorErrorInfo? errorInfo)
        {
            LoginRequest? loginRequest = null;

            if(role.Type == ReactorRoleType.CONSUMER)
            {
                loginRequest = ((ConsumerRole)role).RdmLoginRequest;
            }

            if( (loginRequest is not null) && (reactorChannel.RDMLoginRequestRDP is null) )
            {
                reactorChannel.RDMLoginRequestRDP = new LoginRequest();
                loginRequest.Copy(reactorChannel.RDMLoginRequestRDP);
                reactorChannel.RDMLoginRequestRDP.UserNameType = Login.UserIdTypes.AUTHN_TOKEN;

                // Don't send the password if any.
                reactorChannel.RDMLoginRequestRDP.Flags &= ~LoginRequestFlags.HAS_PASSWORD;
            }

            if(RequestServiceDiscovery(reactorConnectInfo))
            {
                switch(reactorConnectInfo.ConnectOptions.ConnectionType)
                {
                    case ConnectionType.ENCRYPTED:
                        {
                            if(reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol == ConnectionType.SOCKET)
                            {
                                reactorChannel.GetRestConnectionOptions().Transport = ReactorDiscoveryTransportProtocol.RD_TP_TCP;
                                reactorChannel.GetRestConnectionOptions().DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_RWF;
                            }
                            else
                            {
                                reactorChannel.GetRestConnectionOptions().Transport = ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET;
                                reactorChannel.GetRestConnectionOptions().DataFormat = ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2;
                            }
                        }
                        break;
                    default:
                        return PopulateErrorInfo(out errorInfo, ReactorReturnCode.PARAMETER_INVALID, "Reactor.Connect",
                            "Reactor.Connect(): Invalid connection type: " +
                            reactorConnectInfo.ConnectOptions.ConnectionType +
                            " for requesting EDP-RT service discovery.");
                }
            }


            try
            {
                tokenSession.Lock();
                reactorChannel.State = ReactorChannelState.RDP_RT;
                tokenSession.SetProxyInfo(reactorConnectInfo);
                tokenSession.ReactorChannel= reactorChannel;

                if(tokenSession.SessionMgntState == ReactorTokenSession.SessionState.STOP_QUERYING_SERVICE_DISCOVERY ||
                    tokenSession.SessionMgntState == ReactorTokenSession.SessionState.STOP_TOKEN_REQUEST ||
                    tokenSession.SessionMgntState == ReactorTokenSession.SessionState.REQ_AUTH_TOKEN_USING_CLIENT_CRED ||
                    tokenSession.SessionMgntState == ReactorTokenSession.SessionState.QUERYING_SERVICE_DISCOVERY)
                {
                    errorInfo = null;
                    return ReactorReturnCode.SUCCESS;
                }

                if (!tokenSession.HasAccessToken())
                {
                    if (isAsysncReq)
                    {
                        tokenSession.HandleTokenRequest();

                        errorInfo = null;
                        return ReactorReturnCode.SUCCESS; // Waits for async response from the token service.
                    }
                    else
                    {
                        if (m_ReactorRestClient!.SendTokenRequest(tokenSession.ReactorOAuthCredential, tokenSession.ReactorRestConnectOptions,
                            tokenSession.ReactorAuthTokenInfo, out errorInfo) != ReactorReturnCode.SUCCESS)
                        {
                            return errorInfo!.Code;
                        }
                    }
                }
            }
            finally
            {
                tokenSession.Unlock();
            }

            if(RequestServiceDiscovery(reactorConnectInfo))
            {
                if (isAsysncReq)
                {
                    m_ReactorRestClient!.SendServiceDirectoryRequestAsync(tokenSession.ReactorRestConnectOptions, tokenSession.ReactorAuthTokenInfo,
                        tokenSession);

                    errorInfo = null;
                    return ReactorReturnCode.SUCCESS; // Waits for async response from the service discovery.
                }
                else
                {
                    if (m_ReactorRestClient!.SendServiceDirectoryRequest(tokenSession.ReactorRestConnectOptions, tokenSession.ReactorAuthTokenInfo,
                        reactorChannel.ServiceEndpointInfoList, out errorInfo) != ReactorReturnCode.SUCCESS)
                    {
                        return errorInfo!.Code;
                    }
                }

                if(reactorChannel.ApplyServiceDiscoveryEndpoint(out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    return errorInfo!.Code;
                }
                else
                {
                    reactorChannel.State = ReactorChannelState.RDP_RT_DONE;
                }
                
            }
            else
            {
                reactorChannel.State = ReactorChannelState.RDP_RT_DONE;
            }

            errorInfo = null;
            return ReactorReturnCode.SUCCESS;
        }

        internal ReactorCallbackReturnCode SendAuthTokenEventCallback(ReactorChannel reactorChannel, ReactorAuthTokenInfo authTokenInfo, 
            ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode retVal = ReactorCallbackReturnCode.SUCCESS;
            IReactorAuthTokenEventCallback? callback = reactorChannel.ReactorAuthTokenEventCallback();

            if(callback is not null && reactorChannel.EnableSessionManagement())
            {
                ReactorAuthTokenEvent? reactorAuthTokenEvent = new ReactorAuthTokenEvent();
                reactorAuthTokenEvent.ReactorChannel = reactorChannel;
                reactorAuthTokenEvent.ReactorAuthTokenInfo = authTokenInfo;

                if (errorInfo != null && errorInfo.Code != ReactorReturnCode.SUCCESS)
                {
                    reactorAuthTokenEvent.ReactorErrorInfo = errorInfo;
                }

                retVal = callback.ReactorAuthTokenEventCallback(reactorAuthTokenEvent);

                if (retVal != ReactorCallbackReturnCode.SUCCESS)
                {
                    PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, $"Reactor.SendAuthTokenEventCallback",
                        $"Callback return value of {retVal} causing the Reactor to shutdown.");

                    Shutdown(out _);
                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            return retVal;
        }

        internal ReactorCallbackReturnCode SendOAuthCredentialEventCallback(ReactorTokenSession tokenSession, ReactorErrorInfo? errorInfo)
        {
            ReactorCallbackReturnCode retVal = ReactorCallbackReturnCode.SUCCESS;
            ReactorOAuthCredential oAuthCredential = tokenSession.ReactorOAuthCredential;
            IReactorOAuthCredentialEventCallback? callback = oAuthCredential.ReactorOAuthCredentialEventCallback;

            if (callback is not null)
            {
                ReactorOAuthCredentialEvent? reactorOAuthCredentialEvent = new ReactorOAuthCredentialEvent();
                reactorOAuthCredentialEvent.ReactorOAuthCredentialRenewal = tokenSession.ReactorOAuthCredentialRenewal;
                reactorOAuthCredentialEvent.ReactorChannel = tokenSession.ReactorChannel;
                reactorOAuthCredentialEvent.UserSpecObj = oAuthCredential.UserSpecObj;
                reactorOAuthCredentialEvent.Reactor = this;

                if (errorInfo != null && errorInfo.Code != ReactorReturnCode.SUCCESS)
                {
                    reactorOAuthCredentialEvent.ReactorErrorInfo = errorInfo;
                }

                retVal = callback.ReactorOAuthCredentialEventCallback(reactorOAuthCredentialEvent);

                if (retVal != ReactorCallbackReturnCode.SUCCESS)
                {
                    PopulateErrorInfo(out errorInfo, ReactorReturnCode.FAILURE, $"Reactor.SendOAuthCredentialEventCallback",
                        $"Callback return value of {retVal} causing the Reactor to shutdown.");

                    Shutdown(out _);
                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            return retVal;
        }

        internal void SendAuthTokenEvent(ReactorChannel reactorChannel, ReactorTokenSession tokenSession, ReactorErrorInfo? reactorErrorInfo)
        {
            ReactorEventImpl reactorEventImpl = m_ReactorPool.CreateReactorEventImpl();
            reactorEventImpl.EventImplType = ReactorEventImpl.ImplType.TOKEN_MGNT;
            reactorEventImpl.ReactorChannel = reactorChannel;
            reactorEventImpl.TokenSession = tokenSession;

            if(reactorErrorInfo != null)
            {
                PopulateErrorInfo(reactorEventImpl.ReactorErrorInfo, reactorErrorInfo.Code, reactorErrorInfo.Location!, reactorErrorInfo.Error.Text);
            }
            
            m_ReactorEventQueue!.PutEventToQueue(reactorEventImpl);
        }

        internal void SendCredentialRenewalEvent(ReactorChannel reactorChannel, ReactorTokenSession tokenSession, ReactorErrorInfo? reactorErrorInfo)
        {
            ReactorEventImpl reactorEventImpl = m_ReactorPool.CreateReactorEventImpl();
            reactorEventImpl.EventImplType = ReactorEventImpl.ImplType.TOKEN_CREDENTIAL_RENEWAL;
            reactorEventImpl.ReactorChannel = reactorChannel;
            reactorEventImpl.TokenSession = tokenSession;

            if (reactorErrorInfo != null)
            {
                PopulateErrorInfo(reactorEventImpl.ReactorErrorInfo, reactorErrorInfo.Code, reactorErrorInfo.Location!, reactorErrorInfo.Error.Text);
            }

            m_ReactorEventQueue!.PutEventToQueue(reactorEventImpl);
        }
        
        internal void SendChannelWarningEvent(ReactorChannel reactorChannel, ReactorErrorInfo reactorErrorInfo)
        {
            ReactorEventImpl reactorEventImpl = m_ReactorPool.CreateReactorEventImpl();
            reactorEventImpl.EventImplType = ReactorEventImpl.ImplType.WARNING;
            reactorEventImpl.ReactorChannel = reactorChannel;

            if (reactorErrorInfo != null)
            {
                PopulateErrorInfo(reactorEventImpl.ReactorErrorInfo, reactorErrorInfo.Code, reactorErrorInfo.Location!, reactorErrorInfo.Error.Text);
            }

            m_ReactorEventQueue!.PutEventToQueue(reactorEventImpl);
        }
    }
}
