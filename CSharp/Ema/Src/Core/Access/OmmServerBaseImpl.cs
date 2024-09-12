/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Reactor;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal abstract class OmmServerBaseImpl : IOmmCommonImpl, ITimeoutClient
    {
        internal class OmmImplState
        {
            public const long NOT_INITIALIZED = 0;
            public const long INITIALIZED = 1;
            public const long REACTOR_INITIALIZED = 2;
            public const long CHANNEL_DOWN = 3;
        }

        private static int INSTANCE_ID = 0;
        private const int DISPATCH_LOOP_COUNT = 20;
        private const int TERMINATE_API_DISPATCHING_TIMEOUT = 5000;
        private const int MIN_TIME_FOR_SELECT = 1000; // 1 millisecond in microseconds
        private OmmIProviderConfig.OperationModelMode m_OperationModel = OmmIProviderConfig.OperationModelMode.API_DISPATCH;

        protected long DispatchTimeoutApiThread;
        protected int MaxDispatchCountApiThread;
        protected int MaxDispatchCountUserThread;

        private bool m_receivedEvent;

        internal readonly StringBuilder m_StringBuilder = new (2048);
        private readonly StringBuilder m_DispatchStringBuilder = new(1024);

        private readonly List<Socket> registerSocketList = new();

        private readonly List<Socket> socketReadList = new();

        private Thread? apiDispatching;
        private volatile bool apiThreadRunning;
        private readonly EventSignal eventSignal = new();

        protected EmaObjectManager m_EmaObjectManager = new ();

        private bool m_LogDispatchError = true;

        private readonly ILoggerClient m_LoggerClient;

        private readonly EmaObjectManager m_ObjectManager = new();

        private readonly RequestMsg m_EmaRequestMsg;
        private readonly RefreshMsg m_EmaRefreshMsg;
        private readonly StatusMsg m_EmaStatusMsg;
        private readonly GenericMsg m_EmaGenericMsg;
        private readonly PostMsg m_EmaPostMsg;

        private readonly IRequestMsg m_RequestMsg = new Eta.Codec.Msg();

        protected bool m_EventTimeout;

        private MonitorWriteLocker UserLock { get; set; } = new MonitorWriteLocker(new object());
        private MonitorWriteLocker DispatchLock { get; set; } = new MonitorWriteLocker(new object());

        private ProviderRole m_ProviderRole = new ();

        private BindOptions m_BindOption;

        private ReactorSubmitOptions m_SubmitOptions = new();

        protected IServer m_Server;
        public virtual OmmProvider Provider { get; }

        #region internal members

        /// <summary>
        /// This instance is reused to encapsulate parameter data for <see cref="IOmmProviderClient"/>
        /// callbacks for the supplied <see cref="OmmProviderClient"/>.
        /// </summary>
        internal OmmEventImpl<IOmmProviderEvent> OmmProviderEvent { get; private set; } = new OmmEventImpl<IOmmProviderEvent>();

        internal IOmmProviderClient OmmProviderClient { get; private set; }

        internal IOmmProviderErrorClient? OmmProviderErrorClient { get; private set; }

        internal object? Closure { get; private set; }

        internal ServerPool ServerPool { get; private set; }

        internal LinkedList<ReactorChannel> ConnectedChannelList { get; private set; } = new LinkedList<ReactorChannel>();

        #endregion

        #region provider handlers
        internal ServerChannelHandler ServerChannelHandler { get; private set; }
        internal LoginHandler LoginHandler { get; private set; }
        internal DictionaryHandler DictionaryHandler {get; private set; }

        internal DirectoryHandler DirectoryHandler { get; private set; }

        //internal DictionaryHandler

        internal MarketItemHandler MarketItemHandler { get; private set; }

        internal OmmIProviderConfigImpl ConfigImpl { get; private set; }

        internal ItemCallbackClient<IOmmProviderClient> ItemCallbackClient { get; private set; }

        internal Dictionary<long, ItemInfo> ItemInfoMap = new Dictionary<long, ItemInfo>();
        #endregion

        #region abstract methods

        internal abstract DirectoryServiceStore GetDirectoryServiceStore();
        internal abstract void ProcessChannelEvent(ReactorChannelEvent evt);
        #endregion

        protected Reactor? m_Reactor;
        protected readonly ReactorOptions m_ReactorOptions = new();
        protected readonly ReactorDispatchOptions m_ReactorDispatchOptions = new();
        protected ReactorErrorInfo? m_ReactorErrorInfo;
        protected ReactorAcceptOptions m_ReactorAcceptOptions = new();

        public string InstanceName { get; set; }

        public IOmmCommonImpl.ImpleType BaseType => IOmmCommonImpl.ImpleType.IPROVIDER;

        public TimeoutEventManager? TimeoutEventManager { get; private set; }

        internal long ImplState = OmmImplState.NOT_INITIALIZED;

#pragma warning disable CS8618
        public OmmServerBaseImpl(OmmIProviderConfigImpl configImpl, IOmmProviderClient provierClient, object? closure)
#pragma warning restore CS8618
        {
            // First, verify the configuration.  If there are exceptions, this will throw an OmmInvalidConfigurationException.
            configImpl.VerifyConfiguration();

            // Second, deep copy only what's necessary for the config using the copy constructor.
            // ConfigImpl can be used after this to generate the Reactor Role and connection list.
            ConfigImpl = new OmmIProviderConfigImpl(configImpl);

            InstanceName = $"{ConfigImpl.IProviderName}_{Interlocked.Increment(ref INSTANCE_ID)}";

            m_LoggerClient = new LoggerClient<IOmmProviderClient>(this);
            OmmProviderClient = provierClient;
            Closure = closure;

            OmmProviderEvent = new OmmEventImpl<IOmmProviderEvent>();

            m_OperationModel = (OmmIProviderConfig.OperationModelMode)ConfigImpl.DispatchModel;

            ServerPool = new ServerPool(this);
            ServerPool.Initialize(1000, 1000);

            m_EmaRequestMsg = new RequestMsg(m_EmaObjectManager);
            m_EmaRefreshMsg = new RefreshMsg(m_EmaObjectManager);
            m_EmaStatusMsg = new StatusMsg(m_EmaObjectManager);
            m_EmaGenericMsg = new GenericMsg(m_EmaObjectManager);
            m_EmaPostMsg = new PostMsg(m_EmaObjectManager);

            // Set m_BindOption to the ConfigImpl.ServerConfig.BindOptions object.
            m_BindOption = ConfigImpl.GenerateBindOptions();

            Initialize();
        }

#pragma warning disable CS8618
        public OmmServerBaseImpl(OmmIProviderConfigImpl configImpl, IOmmProviderClient provierClient, IOmmProviderErrorClient providerErrorClient, object? closure)
#pragma warning restore CS8618
        {
            // First, verify the configuration.  If there are exceptions, this will throw an OmmInvalidConfigurationException.
            configImpl.VerifyConfiguration();

            // Second, deep copy only what's necessary for the config using the copy constructor.
            // ConfigImpl can be used after this to generate the Reactor Role and connection list.
            ConfigImpl = new OmmIProviderConfigImpl(configImpl);

            InstanceName = $"{ConfigImpl.IProviderName}_{Interlocked.Increment(ref INSTANCE_ID)}";

            m_LoggerClient = new LoggerClient<IOmmProviderClient>(this);
            OmmProviderClient = provierClient;
            OmmProviderErrorClient = providerErrorClient;
            Closure = closure;

            OmmProviderEvent = new OmmEventImpl<IOmmProviderEvent>();

            m_OperationModel = (OmmIProviderConfig.OperationModelMode)ConfigImpl.DispatchModel;

            ServerPool = new ServerPool(this);
            ServerPool.Initialize(1000, 1000);

            m_EmaRequestMsg = new RequestMsg(m_EmaObjectManager);
            m_EmaRefreshMsg = new RefreshMsg(m_EmaObjectManager);
            m_EmaStatusMsg = new StatusMsg(m_EmaObjectManager);
            m_EmaGenericMsg = new GenericMsg(m_EmaObjectManager);
            m_EmaPostMsg = new PostMsg(m_EmaObjectManager);

            // Set m_BindOption to the ConfigImpl.ServerConfig.BindOptions object.
            m_BindOption = ConfigImpl.GenerateBindOptions();

            Initialize();
        }

        public void Initialize()
        {
            try
            {
                Interlocked.Exchange(ref ImplState, OmmImplState.NOT_INITIALIZED);

                UserLock.Enter();

                if (eventSignal.InitEventSignal() != 0)
                {
                    GetStrBuilder().Append("Failed to initiate event signal for timeout in OmmServerBaseImpl (EventSingnal.InitEventSignal()).");

                    string errorText = m_StringBuilder.ToString();

                    throw new OmmInvalidUsageException(errorText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                TimeoutEventManager = new TimeoutEventManager(this, eventSignal);

                m_ReactorOptions.UserSpecObj = this;

                if (m_LoggerClient.IsTraceEnabled)
                {
                    m_LoggerClient.Trace(InstanceName, DumpActiveConfig(ConfigImpl));
                }

                m_ReactorOptions.XmlTracing = ConfigImpl.IProviderConfig.XmlTraceToStdout;
                m_ReactorOptions.XmlTraceToFile = ConfigImpl.IProviderConfig.XmlTraceToFile;
                m_ReactorOptions.XmlTraceMaxFileSize = ConfigImpl.IProviderConfig.XmlTraceMaxFileSize;
                m_ReactorOptions.XmlTraceFileName = ConfigImpl.IProviderConfig.XmlTraceFileName;
                m_ReactorOptions.XmlTraceToMultipleFiles = ConfigImpl.IProviderConfig.XmlTraceToMultipleFiles;
                m_ReactorOptions.XmlTraceWrite = ConfigImpl.IProviderConfig.XmlTraceWrite;
                m_ReactorOptions.XmlTraceRead = ConfigImpl.IProviderConfig.XmlTraceRead;
                m_ReactorOptions.XmlTracePing = ConfigImpl.IProviderConfig.XmlTracePing;

                DispatchTimeoutApiThread = ConfigImpl.IProviderConfig.DispatchTimeoutApiThread;
                MaxDispatchCountApiThread = ConfigImpl.IProviderConfig.MaxDispatchCountApiThread;
                MaxDispatchCountUserThread = ConfigImpl.IProviderConfig.MaxDispatchCountUserThread;

                m_Reactor = Reactor.CreateReactor(m_ReactorOptions, out ReactorErrorInfo? reactorErrInfo);

                if (reactorErrInfo != null)
                {
                    GetStrBuilder().Append("Failed to initialize OmmServerBaseImpl (Reactor.CreateReactor).")
                        .Append($" Error Id={reactorErrInfo.Error.ErrorId}")
                        .Append($" Internal sysError={reactorErrInfo.Error.SysError}")
                        .Append($" Error Location={reactorErrInfo.Location}")
                        .Append($" Error Text={reactorErrInfo.Error.Text}.");

                    string errorText = m_StringBuilder.ToString();

                    throw new OmmInvalidUsageException(errorText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }
                else
                {
                    if (m_LoggerClient.IsTraceEnabled)
                    {
                        m_LoggerClient.Trace(InstanceName, "Successfully created Reactor.");
                    }
                }

                ItemCallbackClient = new ItemCallbackClientProvider(this);
                ItemCallbackClient.Initialize();

                ServerChannelHandler = new ServerChannelHandler(this);
                LoginHandler = new LoginHandler(this);
                LoginHandler.Initialize();

                DirectoryHandler = new DirectoryHandler(this);
                DirectoryHandler.Initialize();

                MarketItemHandler = new MarketItemHandler(this);
                MarketItemHandler.Initialize();

                DictionaryHandler = new DictionaryHandler(this);
                DictionaryHandler.Initialize();

                m_ProviderRole.ChannelEventCallback = ServerChannelHandler;
                m_ProviderRole.LoginMsgCallback = LoginHandler;
                m_ProviderRole.DictionaryMsgCallback = DictionaryHandler;
                m_ProviderRole.DirectoryMsgCallback = DirectoryHandler;
                m_ProviderRole.DefaultMsgCallback = MarketItemHandler;

                // m_BindOption is taken directly from the config
                m_Server = Transport.Bind(m_BindOption, out Error error);

                if(m_Server == null)
                {
                    GetStrBuilder().Append("Failed to initialize OmmServerBaseImpl (Transport.Bind).")
                        .Append($" Error Id={error.ErrorId}")
                        .Append($" Internal sysError={error.SysError}")
                        .Append($" Error Text={error.Text}.");

                    string errorText = m_StringBuilder.ToString();

                    if(m_LoggerClient.IsErrorEnabled)
                    {
                        m_LoggerClient.Error(InstanceName, errorText);
                    }

                    throw new OmmInvalidUsageException(errorText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                if (m_LoggerClient.IsTraceEnabled)
                {
                    m_LoggerClient.Trace(InstanceName, $"Provider bound on port = {m_Server.PortNumber}.");
                }

                Interlocked.Exchange(ref ImplState, OmmImplState.INITIALIZED);

                registerSocketList.Add(m_Reactor!.EventSocket!);
                registerSocketList.Add(eventSignal.GetEventSignalSocket());
                registerSocketList.Add(m_Server.Socket);

                m_EmaObjectManager.GrowAdminDomainItemPools<IOmmProviderClient>();

                if (m_OperationModel == OmmIProviderConfig.OperationModelMode.API_DISPATCH)
                {
                    apiThreadRunning = true;
                    apiDispatching = new Thread(new ThreadStart(Run));
                    apiDispatching.Start();
                }
            }
            catch (OmmException ommException)
            {
                Uninitialize();

                if (HasErrorClient())
                {
                    NotifyErrorClient(ommException);
                }
                else
                {
                    throw;
                }
            }
            finally
            {
                if (UserLock.Locked)
                {
                    UserLock.Exit();
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int Dispatch(int dispatchTimeout)
        {
            if (m_OperationModel == OmmIProviderConfig.OperationModelMode.USER_DISPATCH)
            {
                DispatchLock.Enter();

                if (ReactorDispatchLoop(dispatchTimeout, MaxDispatchCountUserThread))
                {
                    DispatchLock.Exit();
                    return DispatchReturn.DISPATCHED;
                }
                else
                {
                    DispatchLock.Exit();
                    return DispatchReturn.TIMEOUT;

                }
            }

            return DispatchReturn.TIMEOUT;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void Run()
        {
            while (apiThreadRunning)
            {
                ReactorDispatchLoop(DispatchTimeoutApiThread, MaxDispatchCountApiThread);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool ReactorDispatchLoop(long timeOut, int count)
        {
            if (Interlocked.Read(ref ImplState) == OmmImplState.NOT_INITIALIZED)
            {
                UserLock.Enter();
                try
                {
                    if (m_LoggerClient.IsErrorEnabled && m_LogDispatchError)
                    {
                        m_LogDispatchError = false;
                        m_LoggerClient.Error(InstanceName, "Call to ReactorDispatchLoop() failed. The State is set to OmmImplState.NOT_INITIALIZED.");
                    }
                }
                finally
                {
                    UserLock.Exit();
                }

                return false;
            }

            bool waitInfinity = timeOut == DispatchTimeout.INFINITE_WAIT;
            m_receivedEvent = false;
            m_ReactorDispatchOptions.SetMaxMessages(count);
            ReactorReturnCode reactorRetCode = ReactorReturnCode.SUCCESS;
            int loopCount;
            long startTime = EmaUtil.GetMicroseconds();
            long endTime;
            int selectTimeout = 0;

            long userTimeout = TimeoutEventManager!.CheckUserTimeoutExist();
            bool userTimeoutExist = false;
            ReactorErrorInfo? reactorErrorInfo;

            if (userTimeout >= 0)
            {
                if (timeOut > 0 && timeOut < userTimeout)
                {
                    userTimeoutExist = false;
                }
                else
                {
                    userTimeoutExist = true;
                    timeOut = userTimeout;
                }
            }

            try
            {
                endTime = EmaUtil.GetMicroseconds();

                if (timeOut > 0)
                {
                    timeOut -= endTime - startTime;
                    if (timeOut <= 0)
                    {
                        if (userTimeoutExist)
                        {
                            TimeoutEventManager.Execute();
                        }
                    }
                }

                selectTimeout = (int)timeOut;

                if (selectTimeout < MIN_TIME_FOR_SELECT)
                {
                    selectTimeout = MIN_TIME_FOR_SELECT;
                }

                while (Interlocked.Read(ref ImplState) != OmmImplState.NOT_INITIALIZED)
                {
                    UpdateReadSocketList();

                    Socket.Select(socketReadList, null, null, selectTimeout);
                    if (socketReadList.Count > 0)
                    {
                        if (socketReadList.Contains(TimeoutEventManager.TimeoutSignal.GetEventSignalSocket()))
                        {
                            TimeoutEventManager.TimeoutSignal.ResetEventSignal();
                        }

                        /* Checks the Server's Socket to accept the client's connection. */
                        if(socketReadList.Contains(m_Server.Socket))
                        {
                            m_ReactorAcceptOptions.Clear();
                            ClientSession clientSession = ServerPool.GetClientSession(this);
                            m_ReactorAcceptOptions.AcceptOptions.UserSpecObject = clientSession;
                            m_ReactorAcceptOptions.AcceptOptions.NakMount = false;
                            m_ReactorAcceptOptions.SetInitTimeout(ConfigImpl.ServerConfig.InitializationTimeout);

                            if (m_Reactor?.Accept(m_Server, m_ReactorAcceptOptions, m_ProviderRole, out m_ReactorErrorInfo) != ReactorReturnCode.SUCCESS )
                            {
                                if(m_LoggerClient.IsErrorEnabled)
                                {
                                    m_DispatchStringBuilder.Clear();
                                    m_DispatchStringBuilder.Append($"Failed to initialize OmmServerBaseImpl (Reactor.Accept).")
                                        .Append($" Error Id= {m_ReactorErrorInfo?.Error.ErrorId}")
                                        .Append($" Internal SysError= {m_ReactorErrorInfo?.Error.SysError}")
                                        .Append($" Error Location= {m_ReactorErrorInfo?.Location}")
                                        .Append($" Error Text= {m_ReactorErrorInfo?.Error.Text}");

                                    m_LoggerClient.Error(InstanceName, m_DispatchStringBuilder.ToString());
                                }

                                clientSession.ReturnToPool();

                                return false;
                            }
                        }

                        loopCount = 0;
                        do
                        {
                            UserLock.Enter();

                            try
                            {
                                if (Interlocked.Read(ref ImplState) == OmmImplState.NOT_INITIALIZED)
                                {
                                    return false;
                                }

                                reactorRetCode = m_Reactor!.Dispatch(m_ReactorDispatchOptions, out reactorErrorInfo);
                                ++loopCount;
                            }
                            finally
                            {
                                UserLock.Exit();
                            }

                        } while (reactorRetCode > ReactorReturnCode.SUCCESS && !m_receivedEvent && loopCount < DISPATCH_LOOP_COUNT);

                        if (reactorRetCode < ReactorReturnCode.SUCCESS)
                        {
                            StringBuilder strBuilder = new(1024);
                            strBuilder.Append($"Call to Reactor.Dispatch() failed.")
                                .AppendLine($"Reactor Return Code {reactorRetCode}")
                                .AppendLine($"Error Id {reactorErrorInfo?.Error.ErrorId}")
                                .AppendLine($"Internal SysError {reactorErrorInfo?.Error.SysError}")
                                .AppendLine($"Error Location {reactorErrorInfo?.Location}")
                                .AppendLine($"Error Text {reactorErrorInfo?.Error.Text}.");

                            UserLock.Enter();

                            try
                            {
                                if (HasErrorClient())
                                {
                                    OmmProviderErrorClient?.OnDispatchError(strBuilder.ToString(), (int)reactorRetCode);
                                }
                            }
                            finally
                            {
                                UserLock.Exit();
                            }

                            if (m_LoggerClient.IsErrorEnabled)
                            {
                                m_LoggerClient.Error(InstanceName, strBuilder.ToString());
                            }
                        }

                        if (m_receivedEvent) return true;

                        TimeoutEventManager.Execute();

                        if (m_receivedEvent) return true;
                    }
                    else if (socketReadList.Count == 0)
                    {
                        TimeoutEventManager.Execute();

                        if (m_receivedEvent) return true;
                    }

                    endTime = EmaUtil.GetMicroseconds();

                    if (timeOut > 0)
                    {
                        timeOut -= (endTime - startTime);
                        if (timeOut < 0) return false;
                    }
                    else if (!waitInfinity)
                    {
                        return false;
                    }
                }

                return false;
            }
            catch (SocketException)
            {
                registerSocketList.RemoveAll(e => (e.SafeHandle.IsClosed || e.SafeHandle.IsInvalid));

                return true;
            }
            catch (ObjectDisposedException)
            {
                registerSocketList.RemoveAll(e => (e.SafeHandle.IsClosed || e.SafeHandle.IsInvalid));

                return true;
            }
            catch (ArgumentNullException)
            {
                return false;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void RemoveConnectedChannel(ReactorChannel reactorChannel, ClientSession clientSession)
        {
            if(clientSession.Node != null)
            {
                ConnectedChannelList.Remove(clientSession.Node);

                clientSession.Node = null;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void AddConnectedChannel(ReactorChannel reactorChannel, ClientSession clientSession)
        {
            if (clientSession.Node != null)
                ConnectedChannelList.Remove(clientSession.Node);

            var node = ConnectedChannelList.AddLast(reactorChannel);
            clientSession.Node = node;
        }

        private string DumpActiveConfig(OmmIProviderConfigImpl configImpl)
        {
            StringBuilder strBuilder = GetStrBuilder();

            strBuilder.Append($"Print out active configuration detail.{ILoggerClient.CR}")
                   .Append($"ConfiguredName: {configImpl.IProviderConfig.Name}{ILoggerClient.CR}")
                   .Append($"InstanceName: {InstanceName}{ILoggerClient.CR}")
                   .Append($"ItemCountHint: {configImpl.IProviderConfig.ItemCountHint}{ILoggerClient.CR}")
                   .Append($"ServiceCountHint: {configImpl.IProviderConfig.ServiceCountHint}{ILoggerClient.CR}")
                   .Append($"MaxDispatchCountApiThread: {configImpl.IProviderConfig.MaxDispatchCountApiThread}{ILoggerClient.CR}")
                   .Append($"MaxDispatchCountUserThread: {configImpl.IProviderConfig.MaxDispatchCountUserThread}{ILoggerClient.CR}")
                   .Append($"DispatchTimeoutApiThread: {configImpl.IProviderConfig.DispatchTimeoutApiThread}{ILoggerClient.CR}")
                   .Append($"DispatchMode: {configImpl.DispatchModel}{ILoggerClient.CR}")
                   .Append($"RequestTimeout: {configImpl.IProviderConfig.RequestTimeout}{ILoggerClient.CR}")
                   .Append($"XmlTraceToStdout: {configImpl.IProviderConfig.XmlTraceToStdout}{ILoggerClient.CR}")
                   .Append($"XmlTraceToFile: {configImpl.IProviderConfig.XmlTraceToFile}{ILoggerClient.CR}")
                   .Append($"XmlTraceMaxFileSize: {configImpl.IProviderConfig.XmlTraceMaxFileSize}{ILoggerClient.CR}")
                   .Append($"XmlTraceFileName: {configImpl.IProviderConfig.XmlTraceFileName}{ILoggerClient.CR}")
                   .Append($"XmlTraceToMultipleFiles: {configImpl.IProviderConfig.XmlTraceToMultipleFiles}{ILoggerClient.CR}")
                   .Append($"XmlTraceWrite: {configImpl.IProviderConfig.XmlTraceWrite}{ILoggerClient.CR}")
                   .Append($"XmlTraceRead: {configImpl.IProviderConfig.XmlTraceRead}{ILoggerClient.CR}")
                   .Append($"XmlTracePing: {configImpl.IProviderConfig.XmlTracePing}{ILoggerClient.CR}")
                   .Append($"AcceptMessageWithoutBeingLogin: {configImpl.IProviderConfig.AcceptMessageWithoutBeingLogin}{ILoggerClient.CR}")
                   .Append($"AcceptMessageWithoutAcceptingRequests: {configImpl.IProviderConfig.AcceptMessageWithoutAcceptingRequests}{ILoggerClient.CR}")
                   .Append($"AcceptDirMessageWithoutMinFilters: {configImpl.IProviderConfig.AcceptDirMessageWithoutMinFilters}{ILoggerClient.CR}")
                   .Append($"AcceptMessageWithoutQosInRange: {configImpl.IProviderConfig.AcceptMessageWithoutQosInRange}{ILoggerClient.CR}")
                   .Append($"AcceptMessageSameKeyButDiffStream: {configImpl.IProviderConfig.AcceptMessageSameKeyButDiffStream}{ILoggerClient.CR}")
                   .Append($"AcceptMessageThatChangesService: {configImpl.IProviderConfig.AcceptMessageThatChangesService}{ILoggerClient.CR}")
                   .Append($"EnforceAckIDValidation: {configImpl.IProviderConfig.EnforceAckIDValidation}{ILoggerClient.CR}")
                   .Append($"DirectoryAdminControl: {configImpl.AdminControlDirectory}{ILoggerClient.CR}")
                   .Append($"DictionaryAdminControl: {configImpl.AdminControlDictionary}{ILoggerClient.CR}")
                   .Append($"RefreshFirstRequired: {configImpl.IProviderConfig.RefreshFirstRequired}{ILoggerClient.CR}")
                   .Append($"MaxFieldDictFragmentSize: {configImpl.IProviderConfig.FieldDictionaryFragmentSize}{ILoggerClient.CR}")
                   .Append($"MaxEnumTypeFragmentSize: {configImpl.IProviderConfig.EnumTypeFragmentSize}{ILoggerClient.CR}");

            return strBuilder.ToString();
        }

        internal int RequestTimeout()
        {
            return Utilities.Convert_ulong_int(ConfigImpl.IProviderConfig.RequestTimeout);
        }

        public virtual void ChannelInformation(ChannelInformation channelInformation)
        {
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void EventReceived()
        {
            m_receivedEvent = true;
        }


        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public EmaObjectManager GetEmaObjManager()
        {
            return m_EmaObjectManager;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public StringBuilder GetStrBuilder()
        {
            m_StringBuilder.Clear();
            return m_StringBuilder;
        }


        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public MonitorWriteLocker GetUserLocker()
        {
            return UserLock;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void SetOmmImplState(long implState)
        {
            Interlocked.Exchange(ref ImplState, implState);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ILoggerClient GetLoggerClient()
        {
            return m_LoggerClient;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorSubmitOptions GetSubmitOptions()
        {
            m_SubmitOptions.Clear();
            if (ConfigImpl.ServerConfig.DirectWrite)
            {
                m_SubmitOptions.WriteArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
            }

            return m_SubmitOptions;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void HandleTimeoutEvent()
        {
            m_EventTimeout = true;
        }

        /// <summary>
        /// Get ETA Request message.
        /// </summary>
        /// <returns></returns>
        public IRequestMsg GetRequestMsg()
        {
            m_RequestMsg.Clear();

            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            return m_RequestMsg;
        }

        public RequestMsg RequestMsg()
        {
            m_EmaRequestMsg.ClearRequest_Decode();
            return m_EmaRequestMsg;
        }

        public RefreshMsg RefreshMsg()
        {
            m_EmaRefreshMsg.ClearRefresh_Decode();
            return m_EmaRefreshMsg;
        }

        public StatusMsg StatusMsg()
        {
            m_EmaStatusMsg.ClearStatus_Decode();
            return m_EmaStatusMsg;
        }

        public GenericMsg GenericMsg()
        {
            m_EmaGenericMsg.ClearMsg_Decode();
            return m_EmaGenericMsg;
        }

        public PostMsg PostMsg()
        {
            m_EmaPostMsg.ClearMsg_Decode();
            return m_EmaPostMsg;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void AddItemInfo(ItemInfo itemInfo)
        {
            GetUserLocker().Enter();

            ItemInfoMap.TryAdd(itemInfo.Handle, itemInfo);
            itemInfo.ClientSession!.AddItemInfo(itemInfo);

            if (GetLoggerClient().IsTraceEnabled)
            {
                m_StringBuilder.Length = 0;
                m_StringBuilder.Append("Added ItemInfo ").Append(itemInfo.Handle).Append(" to ItemInfoMap").Append(ILoggerClient.CR)
                .Append("Client handle ").Append(itemInfo.ClientSession.ClientHandle);
                GetLoggerClient().Trace(InstanceName, m_StringBuilder.ToString());
            }

            GetUserLocker().Exit();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void RemoveItemInfo(ItemInfo itemInfo, bool removeItemGroup)
        {
            GetUserLocker().Enter();

            try
            {
                ItemInfoMap.Remove(itemInfo.Handle);
                itemInfo.ClientSession!.RemoveItemInfo(itemInfo);

                if (removeItemGroup && itemInfo.HasItemGroup)
                {
                    RemoveItemGroup(itemInfo);
                }

                if (GetLoggerClient().IsTraceEnabled)
                {
                    StringBuilder temp = GetStrBuilder();
                    temp.Append("Removed ItemInfo ").Append(itemInfo.Handle).Append(" from ItemInfoMap").Append(ILoggerClient.CR)
                    .Append("Client handle ").Append(itemInfo.ClientSession.ClientHandle);
                    GetLoggerClient().Trace(InstanceName, temp.ToString());
                }

                itemInfo.ReturnToPool();
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ItemInfo? GetItemInfo(long handle)
        {
            GetUserLocker().Enter();
            ItemInfoMap.TryGetValue(handle, out ItemInfo? itemInfo);
            GetUserLocker().Exit();

            return itemInfo;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void HandleInvalidUsage(string text, int errorCode)
        {
            if (OmmProviderErrorClient != null)
            {
                OmmProviderErrorClient.OnInvalidUsage(text, errorCode);
            }
            else
            {
                throw new OmmInvalidUsageException(text, errorCode);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void HandleInvalidHandle(long handle, string text)
        {
            if (OmmProviderErrorClient != null)
            {
                OmmProviderErrorClient.OnInvalidHandle(handle, text);
            }
            else
            {
                throw new OmmInvalidHandleException(handle, text);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        protected  bool HasErrorClient()
        {
            return OmmProviderErrorClient != null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        protected void NotifyErrorClient(OmmException ommException)
        {
            if (OmmProviderErrorClient != null)
            {
                switch (ommException.Type)
                {
                    case OmmException.ExceptionType.OmmInvalidHandleException:
                        {
                            OmmProviderErrorClient.OnInvalidHandle(((OmmInvalidHandleException)ommException).Handle, ommException.Message);
                            break;
                        }
                    case OmmException.ExceptionType.OmmInvalidUsageException:
                        {
                            OmmProviderErrorClient.OnInvalidUsage(ommException.Message, ((OmmInvalidUsageException)ommException).ErrorCode);
                            break;
                        }
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Uninitialize()
        {
            try
            {
                if (m_OperationModel == OmmIProviderConfig.OperationModelMode.API_DISPATCH)
                {
                    lock (this)
                    {
                        if (apiDispatching != null && apiThreadRunning)
                        {
                            apiThreadRunning = false;
                            m_receivedEvent = true;
                            TimeoutEventManager?.CleanupEventSignal();
                            if (apiDispatching.Join(TERMINATE_API_DISPATCHING_TIMEOUT) == false) /* Waits for API dispatching thread to exit */
                            {
                                if (m_LoggerClient.IsErrorEnabled)
                                {
                                    m_LoggerClient.Error(InstanceName, "Failed to uninitialize OmmServerBaseImpl(apiDispatching.Join() timed out).");
                                }
                            }
                        }
                    }

                    UserLock.Enter();
                }
                else
                {

                    TimeoutEventManager?.CleanupEventSignal();
                    UserLock.Enter();
                    m_receivedEvent = true;
                }

                if (Interlocked.Read(ref ImplState) == OmmImplState.NOT_INITIALIZED)
                {
                    return;
                }

                ReactorErrorInfo? errorInfo = null;
                if (m_Reactor?.Shutdown(out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    if (m_LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = GetStrBuilder();

                        strBuilder.Append("Failed to uninitialize OmmServerBaseImpl (Reactor.Shutdown).")
                            .Append($" Error Id={errorInfo?.Error.ErrorId}")
                            .Append($" Internal sysError={errorInfo?.Error.SysError}")
                            .Append($" Error Location={errorInfo?.Location}")
                            .Append($" Error Text={errorInfo?.Error.Text}.");
                        m_LoggerClient.Error(InstanceName, strBuilder.ToString());
                    }
                }

                if (ServerChannelHandler != null)
                {
                    ServerChannelHandler.CloseActiveSessions();
                }

                if(m_Server != null && m_Server.State == Eta.Transports.ChannelState.ACTIVE
                    && (TransportReturnCode.SUCCESS != m_Server.Close(out Error error)))
                {
                    if (m_LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = GetStrBuilder();
                        strBuilder.Append($"IServer.Close() failed while uninitializing OmmServerBaseImpl.")
                            .Append($" Internal sysError='{error.SysError}'").Append($" Error text='{error.Text}'");

                        m_LoggerClient.Error(InstanceName, strBuilder.ToString());
                    }
                }
            }
            catch (Exception exp)
            {
                var strBuilder = GetStrBuilder();

                strBuilder.Append($"OmmServerBaseImpl Unintialize(), Exception occurred, exception={exp.Message}");

                if (m_LoggerClient.IsErrorEnabled)
                {
                    m_LoggerClient.Error(InstanceName, strBuilder.ToString());
                }
            }
            finally
            {
                Interlocked.Exchange(ref ImplState, OmmImplState.NOT_INITIALIZED);

                if (m_Reactor != null)
                    m_EmaObjectManager.Free();

                UserLock.Exit();
                m_LoggerClient.Cleanup();
                m_Reactor = null;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private void UpdateReadSocketList()
        {
            socketReadList.Clear();
            socketReadList.AddRange(registerSocketList);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void RegisterSocket(Socket socket)
        {
            if (socket == null)
                return;

            int index = registerSocketList.IndexOf(socket);

            /* not found */
            if (index == -1)
            {
                if (!socket.SafeHandle.IsClosed)
                {
                    registerSocketList.Add(socket);
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void UnregisterSocket(Socket socket)
        {
            if (socket == null)
                return;

            int index = registerSocketList.IndexOf(socket);
            if (index != -1)
            {
                registerSocketList.RemoveAt(index);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void RemoveItemGroup(ItemInfo itemInfo)
        {
            if (itemInfo.ClientSession == null)
                return;

            UserLock.Enter();

            try
            {
                itemInfo.ClientSession.ServiceGroupIdToItemInfoMap.TryGetValue(itemInfo.ServiceId, out var groupIdToItemInfoList);

                if (groupIdToItemInfoList != null)
                {
                    groupIdToItemInfoList.TryGetValue(itemInfo.ItemGroup, out var itemInfoList);

                    if (itemInfoList != null)
                    {
                        itemInfoList.Remove(itemInfo);

                        if (itemInfoList.Count == 0)
                        {
                            groupIdToItemInfoList.Remove(itemInfo.ItemGroup);
                        }
                    }
                }

                itemInfo.Flags &= ~ItemInfo.ItemInfoFlags.ITEM_GROUP;
            }
            finally
            {
                UserLock.Exit();
            }
        }

        internal void AddItemGroup(ItemInfo itemInfo, Buffer groupId)
        {
            UserLock.Enter();

            try
            {
                if (itemInfo.ClientSession!.ServiceGroupIdToItemInfoMap.TryGetValue(itemInfo.ServiceId, out var groupIdToItemInfoList))
                {
                    if (groupIdToItemInfoList.TryGetValue(groupId, out var itemInfoList))
                    {
                        itemInfoList.Add(itemInfo);
                    }
                    else
                    {
                        itemInfoList = new List<ItemInfo>(1000);
                        itemInfoList.Add(itemInfo);

                        groupIdToItemInfoList[groupId] = itemInfoList;
                    }
                }
                else
                {
                    groupIdToItemInfoList = new Dictionary<Buffer, List<ItemInfo>>();
                    var itemInfoList = new List<ItemInfo>(1000);
                    itemInfoList.Add(itemInfo);

                    groupIdToItemInfoList[groupId] = itemInfoList;
                    itemInfo.ClientSession!.ServiceGroupIdToItemInfoMap[itemInfo.ServiceId] = groupIdToItemInfoList;
                }
            }
            finally
            {
                UserLock.Exit();
            }
        }
    }
}
