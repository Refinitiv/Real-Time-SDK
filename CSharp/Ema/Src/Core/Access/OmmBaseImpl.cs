/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.IO;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Microsoft.IdentityModel.Tokens;
using System.Runtime.CompilerServices;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// This is the base implementation class for OmmConsumer and OmmNiProvider
    /// </summary>
    /// <typeparam name="T"></typeparam>
    internal abstract class OmmBaseImpl<T> : IOmmCommonImpl, ITimeoutClient
    {
        internal class OmmImplState
        {
            public const long NOT_INITIALIZED = 0;
            public const long INITIALIZED = 1;
            public const long REACTOR_INITIALIZED = 2;
            public const long CHANNEL_DOWN = 3;
            public const long CHANNEL_UP = 4;
            public const long CHANNEL_UP_STREAM_NOT_OPEN = 5;
            public const long LOGIN_STREAM_OPEN_SUSPECT = 6;
            public const long LOGIN_STREAM_OPEN_OK = 7;
            public const long LOGIN_STREAM_CLOSED = 8;
            public const long DIRECTORY_STREAM_OPEN_SUSPECT = 9;
            public const long DIRECTORY_STREAM_OPEN_OK = 10;
        }

        private static int INSTANCE_ID = 0;
        private const int DISPATCH_LOOP_COUNT = 20;
        private const int TERMINATE_API_DISPATCHING_TIMEOUT = 5000;
        private OmmConsumerConfig.OperationModelMode operationModel = OmmConsumerConfig.OperationModelMode.API_DISPATCH;

        private bool m_receivedEvent;

        private StringBuilder stringBuilder = new StringBuilder();

        private List<Socket> registerSocketList = new();

        private List<Socket> socketReadList = new();

        private Thread? apiDispatching;
        private volatile bool apiThreadRunning;
        private EventSignal eventSignal = new();

        private EmaObjectManager m_EmaObjectManager = new EmaObjectManager();

        private bool m_LogDispatchError = true;

        public long ImplState = OmmImplState.NOT_INITIALIZED;

        protected bool m_EventTimeout;

        internal ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        #region Callback clients
        public ChannelCallbackClient<T>? ChannelCallbackClient { get; protected set; }
        public LoginCallbackClient<T>? LoginCallbackClient { get; protected set; }
        public DirectoryCallbackClient<T>? DirectoryCallbackClient { get; protected set; }
        public DictionaryCallbackClient<T>? DictionaryCallbackClient { get; protected set; }
        public ItemCallbackClient<T>? ItemCallbackClient { get; protected set; }

        public OAuthCallbackClientConsumer? OAuthCallbackClient { get; protected set; }
        #endregion

        #region Value Added classes
        internal Reactor? reactor;
        protected readonly ReactorOptions reactorOptions = new();
        protected readonly ReactorDispatchOptions reactorDispatchOptions = new();

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void ReceivedEvent()
        {
            m_receivedEvent = true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal ReactorSubmitOptions GetSubmitOptions()
        {
            m_SubmitOptions.Clear();           
            return m_SubmitOptions;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long RegisterClient(RequestMsg requestMsg, T client, object? closure)
        {
            if (CheckClient(client))
                return 0;

            return ItemCallbackClient!.RegisterClient(requestMsg, client, closure);
        }

        public virtual void Reissue(RequestMsg requestMsg, long handle)
        {
            try
            {
                UserLock.Enter();
                ItemCallbackClient!.Reissue(requestMsg, handle);
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public virtual void Submit(PostMsg postMsg, long handle)
        {
            try
            {
                UserLock.Enter();
                ItemCallbackClient!.Submit(postMsg, handle);
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public virtual void Submit(GenericMsg genericMsg, long handle)
        {
            try
            {
                UserLock.Enter();
                ItemCallbackClient!.Submit(genericMsg, handle);
            }
            finally
            {
                UserLock.Exit();
            }
        }

        protected readonly ReactorSubmitOptions reactorSubmitOptions = new();
        internal ReactorErrorInfo? reactorErrorInfo;
        #endregion

        #region abstract methods
        internal abstract void HandleInvalidUsage(string text, int errorCode);

        internal abstract void HandleInvalidHandle(long handle, string text);

        protected abstract bool HasErrorClient();

        protected abstract void HandleAdminDomains();

        protected abstract void NotifyErrorClient(OmmException ommException);

        internal abstract IOmmCommonImpl.ImpleType GetImplType();

        internal abstract long NextLongId();

        public virtual void Unregister(long handle)
        {
            try
            {
                UserLock.Enter();
                ItemCallbackClient!.UnregisterClient(handle);
            }
            finally
            {
                UserLock.Exit();
            }
        }
        #endregion

        /// <summary>
        /// The derived class can process the channel event as needed.
        /// </summary>
        /// <param name="evt"></param>
        public virtual void ProcessChannelEvent(ReactorChannelEvent evt) { }

        public MonitorWriteLocker UserLock { get; private set; } = new MonitorWriteLocker(new object());

        public MonitorWriteLocker DispatchLock { get; private set; } = new MonitorWriteLocker(new object());

        public TimeoutEventManager<T>? TimeoutEventManager { get; private set; }

        public ILoggerClient LoggerClient;

        internal OmmConsumerConfigImpl ConfigImpl { get; private set; }

        /* This is used for unit testing to skip generating an unique instance ID. */
        internal static bool GENERATE_INSTANCE_ID { get; set; } = true;

        public OmmBaseImpl(OmmConsumerConfigImpl configImpl)
        {
            // First, verify the configuration.  If there are exceptions, this will throw an OmmInvalidConfigurationException.
            configImpl.VerifyConfiguration();

            // Second, deep copy only what's necessary for the config using the copy constructor.
            // ConfigImpl can be used after this to generate the Reactor Role and connection list.
            ConfigImpl = new OmmConsumerConfigImpl(configImpl);

            // Generate an instance name from Consumer element name and an unique instance ID
            if (GENERATE_INSTANCE_ID)
                InstanceName = $"{ConfigImpl.ConsumerName}_{Interlocked.Increment(ref INSTANCE_ID)}";
            else
                InstanceName = $"{ConfigImpl.ConsumerName}";

            LoggerClient = new LoggerClient<T>(this);

            // Log all of the stored config log lines
            configImpl.ConfigErrorLog?.Log(LoggerClient, LoggerClient.Level);

            operationModel = ConfigImpl.DispatchModel;

            if (configImpl.DataDictionary() is not null)
            {
                ConfigImpl.DictionaryConfig.DataDictionary = configImpl.DataDictionary()!;

                if (LoggerClient.IsTraceEnabled)
                {
                    LoggerClient.Trace(InstanceName, "The user specified DataDictionary object is used for dictionary information. "
                        + "EMA ignores the DictionaryGroup configuration in either file and programmatic configuration database.");
                }
            }
        }

        public void Initialize()
        {
            try
            {
                Interlocked.Exchange(ref ImplState, OmmImplState.NOT_INITIALIZED);

                UserLock.Enter();

                if (eventSignal.InitEventSignal() != 0)
                {
                    GetStrBuilder().Append("Failed to initiate event signal for timeout in OmmBaseImpl (EventSingnal.InitEventSignal()).");

                    string errorText = stringBuilder.ToString();

                    throw new OmmInvalidUsageException(errorText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                if (LoggerClient.IsTraceEnabled)
                {
                    LoggerClient.Trace(InstanceName, DumpActiveConfig(ConfigImpl));
                }

                TimeoutEventManager = new TimeoutEventManager<T>(this, eventSignal);

                reactorOptions.UserSpecObj = this;
                ConfigImpl.PopulateReactorOptions(reactorOptions);

                reactor = Reactor.CreateReactor(reactorOptions, out ReactorErrorInfo? reactorErrInfo);

                if (reactorErrInfo != null)
                {
                    GetStrBuilder().Append("Failed to initialize OmmBaseImpl (Reactor.CreateReactor).")
                        .Append($" Error Id={reactorErrInfo.Error.ErrorId}")
                        .Append($" Internal sysError={reactorErrInfo.Error.SysError}")
                        .Append($" Error Location={reactorErrInfo.Location}")
                        .Append($" Error Text={reactorErrInfo.Error.Text}.");

                    string errorText = stringBuilder.ToString();

                    throw new OmmInvalidUsageException(errorText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }
                else
                {
                    if (LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, "Successfully created Reactor.");
                    }
                }

                Interlocked.Exchange(ref ImplState, OmmImplState.INITIALIZED);

                registerSocketList.Add(reactor!.EventSocket!);
                registerSocketList.Add(eventSignal.GetEventSignalSocket());

                m_EmaObjectManager.GrowAdminDomainItemPools<T>();

                HandleAdminDomains();

                if (operationModel == OmmConsumerConfig.OperationModelMode.API_DISPATCH)
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

        public virtual void ChannelInformation(ChannelInformation channelInformation) { }

        internal virtual void UnsetActiveRsslReactorChannel(ChannelInfo channelInfo)
        {
        }

        internal virtual void SetActiveReactorChannel(ChannelInfo channelInfo)
        {
        }

        public virtual void Uninitialize()
        {
            try
            {
                if (operationModel == OmmConsumerConfig.OperationModelMode.API_DISPATCH)
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
                                if (LoggerClient.IsErrorEnabled)
                                {
                                    LoggerClient.Error(InstanceName, "Failed to uninitialize OmmBaseImpl(apiDispatching.Join() timed out).");
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

                if (LoginCallbackClient != null)
                {
                    LoginCallbackClient.SendLoginClose();
                }

                if (ChannelCallbackClient != null)
                {
                    ChannelCallbackClient.CloseChannels();
                }

                ReactorErrorInfo? errorInfo = null;
                if (reactor?.Shutdown(out errorInfo) != ReactorReturnCode.SUCCESS)
                {
                    var strBuilder = GetStrBuilder();

                    strBuilder.Append("Failed to uninitialize OmmBaseImpl (Reactor.Shutdown).")
                        .Append($" Error Id={errorInfo?.Error.ErrorId}")
                        .Append($" Internal sysError={errorInfo?.Error.SysError}")
                        .Append($" Error Location={errorInfo?.Location}")
                        .Append($" Error Text={errorInfo?.Error.Text}.");

                    if (LoggerClient.IsErrorEnabled)
                    {
                        LoggerClient.Error(InstanceName, strBuilder.ToString());
                    }
                }

            }
            catch (Exception exp)
            {
                var strBuilder = GetStrBuilder();

                strBuilder.Append($"OmmBaseImpl Unintialize(), Exception occurred, exception={exp.Message}");

                if (LoggerClient.IsErrorEnabled)
                {
                    LoggerClient.Error(InstanceName, strBuilder.ToString());
                }
            }
            finally
            {
                Interlocked.Exchange(ref ImplState, OmmImplState.NOT_INITIALIZED);

                if (reactor != null)
                    m_EmaObjectManager.Free();

                UserLock.Exit();
                reactor = null;
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void CloseReactorChannel(ReactorChannel? reactorChannel)
        {
            if(reactorChannel == null)
            {
                return;
            }

            ChannelInfo? channelInfo = reactorChannel.UserSpecObj as ChannelInfo;
            if(reactorChannel.Reactor != null && reactorChannel.Close(out var errorInfo) != ReactorReturnCode.SUCCESS)
            {
                if(LoggerClient.IsErrorEnabled && errorInfo != null)
                {
                    StringBuilder strBuilder = GetStrBuilder();
                    strBuilder.Append($"Failed to close reactor channel (ReactorChannel).")
                        .AppendLine($" Channel= {errorInfo.Error.Channel?.GetHashCode()}")
                        .AppendLine($"Error Id {errorInfo.Error.ErrorId}")
                        .AppendLine($"Internal SysError {errorInfo.Error.SysError}")
                        .AppendLine($"Error Location {errorInfo.Location}")
                        .AppendLine($"Error Text {errorInfo.Error.Text}.");

                    LoggerClient.Error(InstanceName, strBuilder.ToString());
                }
            }

            ChannelCallbackClient!.RemoveChannel(channelInfo);
        }

        public string InstanceName { get; set; }

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
            stringBuilder.Clear();
            return stringBuilder;
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
        public int Dispatch(int dispatchTimeout)
        {
            if (operationModel == OmmConsumerConfig.OperationModelMode.USER_DISPATCH)
            {
                DispatchLock.Enter();

                if (ReactorDispatchLoop(dispatchTimeout, ConfigImpl.ConsumerConfig.MaxDispatchCountUserThread))
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
            long dispatchTimeout = ConfigImpl.ConsumerConfig.DispatchTimeoutApiThread;
            int maxDispatchCount = ConfigImpl.ConsumerConfig.MaxDispatchCountApiThread;

            while (apiThreadRunning)
            {
                ReactorDispatchLoop(dispatchTimeout, maxDispatchCount);
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool ReactorDispatchLoop(long timeOut, int count)
        {
            if(Interlocked.Read(ref ImplState) == OmmImplState.NOT_INITIALIZED)
            {
                UserLock.Enter();
                try
                {
                    if(LoggerClient.IsErrorEnabled && m_LogDispatchError)
                    {
                        m_LogDispatchError = false;
                        LoggerClient.Error(InstanceName, "Call to ReactorDispatchLoop() failed. The State is set to OmmImplState.NOT_INITIALIZED.");
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
            reactorDispatchOptions.SetMaxMessages(count);
            ReactorReturnCode reactorRetCode = ReactorReturnCode.SUCCESS;
            int loopCount;
            long startTime = EmaUtil.GetMicroseconds();
            long endTime;
            int selectTimeout = 0;

            long userTimeout = TimeoutEventManager!.CheckUserTimeoutExist();
            bool userTimeoutExist = false;

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
                if (ImplState >= OmmImplState.CHANNEL_UP)
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

                    if (selectTimeout < 0)
                    {
                        selectTimeout = 0;
                    }
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

                                reactorRetCode = reactor!.Dispatch(reactorDispatchOptions, out reactorErrorInfo);
                                ++loopCount;
                            }
                            finally
                            {
                                UserLock.Exit();
                            }

                        } while (reactorRetCode > ReactorReturnCode.SUCCESS && !m_receivedEvent && loopCount < DISPATCH_LOOP_COUNT);

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
                return true;
            }
            catch (ObjectDisposedException)
            {
                return true;
            }
            catch(ArgumentNullException)
            {
                return false;
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
        public void HandleTimeoutEvent()
        {
            m_EventTimeout = true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void HandleLoginReqTimeout()
        {
            long loginRequestTimeOut = ConfigImpl.ConsumerConfig.LoginRequestTimeOut;
            long dispatchTimeoutApiThread = ConfigImpl.ConsumerConfig.DispatchTimeoutApiThread;
            int maxDispatchCountApiThread = ConfigImpl.ConsumerConfig.MaxDispatchCountApiThread;
            if (loginRequestTimeOut == 0)
            {
                while (ImplState < OmmImplState.LOGIN_STREAM_OPEN_OK && ImplState != OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN)
                    ReactorDispatchLoop(dispatchTimeoutApiThread, maxDispatchCountApiThread);

                /* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
                if (ImplState == OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN)
                {
                    throw new OmmInvalidUsageException(LoginCallbackClient!.LoginFailureMsg, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_REJECTED);
                }
            }
            else
            {
                m_EventTimeout = false;
                TimeoutEvent timeoutEvent = TimeoutEventManager!.AddTimeoutEvent(loginRequestTimeOut * 1000, this);

                while (!m_EventTimeout && (ImplState < OmmImplState.LOGIN_STREAM_OPEN_OK) && (ImplState != OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN))
                {
                    ReactorDispatchLoop(dispatchTimeoutApiThread, maxDispatchCountApiThread);
                }

                if (m_EventTimeout)
                {
                    int initChannelIndex = ChannelCallbackClient!.InitialChannelConnectIndex;

                    ChannelInfo? channelInfo = ChannelCallbackClient!.GetChannelInfo(initChannelIndex);

                    ClientChannelConfig? channelConfig = channelInfo?.ChannelConfig;

                    StringBuilder strBuilder = GetStrBuilder();
                    strBuilder.Append($"login failed (timed out after waiting {loginRequestTimeOut} milliseconds)");
                    
                    if(channelConfig != null)
                    {
                        strBuilder.Append($" for {channelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address}:" +
                            $"{channelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName})");
                    }

                    string message = strBuilder.ToString();

                    if (LoggerClient.IsErrorEnabled)
                        LoggerClient.Error(InstanceName, message);

                    throw new OmmInvalidUsageException(message, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_TIME_OUT);
                }
                else if (ImplState == OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN) /* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
                {
                    timeoutEvent.Cancel();
                    throw new OmmInvalidUsageException(LoginCallbackClient!.LoginFailureMsg, OmmInvalidUsageException.ErrorCodes.LOGIN_REQUEST_REJECTED);
                }
                else
                    timeoutEvent.Cancel();
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        protected void ModifyIOCtl(IOCtlCode code, int val, ChannelInfo? activeChannelInfo)
        {
            if (activeChannelInfo == null
                || activeChannelInfo.ReactorChannel == null)
            {
                HandleInvalidUsage("No active channel to modify I/O option.",
                    OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                return;
            }

            Eta.Transports.IChannel channel = activeChannelInfo.ReactorChannel.Channel!;

            Eta.Transports.TransportReturnCode ret = channel.IOCtl((Eta.Transports.IOCtlCode)(int)code, val, out var error);

            if (code == IOCtlCode.MAX_NUM_BUFFERS || code == IOCtlCode.NUM_GUARANTEED_BUFFERS)
            {
                if ((int)ret != (int)val)
                {
                    ret = Eta.Transports.TransportReturnCode.FAILURE;
                }
                else
                {
                    ret = Eta.Transports.TransportReturnCode.SUCCESS;
                }
            }

            if (ret != Eta.Transports.TransportReturnCode.SUCCESS)
            {
                StringBuilder strBuilder = GetStrBuilder();

                strBuilder.Append("Failed to modify I/O option = ")
                    .Append(code).Append(". Reason: ")
                    .Append(ret.ToString())
                    .Append(". Error text: ")
                    .Append(error?.Text ?? string.Empty);

                HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.FAILURE);
            }
        }

        /// <summary>
        /// Checks whether the client is null.
        /// </summary>
        /// <param name="client">The passed in Client</param>
        /// <returns><c>true</c> if the client is not set;otherwise <c>false</c></returns>
        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        protected bool CheckClient(T client)
        {
            if (client == null)
            {
                HandleInvalidUsage("A derived class of IOmmConsumerClient is not set", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                return true;
            }

            return false;
        }

        private string DumpActiveConfig(OmmConsumerConfigImpl configImpl)
        {
            StringBuilder strBuilder = GetStrBuilder();

            strBuilder.Append($"Print out active configuration detail.{ILoggerClient.CR}")
                .Append($"ConfiguredName: {configImpl.ConsumerConfig.Name}{ILoggerClient.CR}")
                .Append($"InstanceName: {InstanceName}{ILoggerClient.CR}")
                .Append($"ItemCountHint: {configImpl.ConsumerConfig.ItemCountHint}{ILoggerClient.CR}")
                .Append($"ServiceCountHint: {configImpl.ConsumerConfig.ServiceCountHint}{ILoggerClient.CR}")
                .Append($"MaxDispatchCountApiThread: {configImpl.ConsumerConfig.MaxDispatchCountApiThread}{ILoggerClient.CR}")
                .Append($"MaxDispatchCountUserThread: {configImpl.ConsumerConfig.MaxDispatchCountUserThread}{ILoggerClient.CR}")
                .Append($"RequestTimeout: {configImpl.ConsumerConfig.RequestTimeout}{ILoggerClient.CR}")
                .Append($"XmlTraceToStdout: {configImpl.ConsumerConfig.XmlTraceToStdout}{ILoggerClient.CR}")
                .Append($"ObeyOpenWindow: {configImpl.ConsumerConfig.ObeyOpenWindow}{ILoggerClient.CR}")
                .Append($"PostAckTimeout: {configImpl.ConsumerConfig.PostAckTimeout}{ILoggerClient.CR}")
                .Append($"MaxOutstandingPosts: {configImpl.ConsumerConfig.MaxOutstandingPosts}{ILoggerClient.CR}")
                .Append($"DispatchMode: {configImpl.DispatchModel}{ILoggerClient.CR}")
                .Append($"ReconnectAttemptLimit: {configImpl.ConsumerConfig.ReconnectAttemptLimit}{ILoggerClient.CR}")
                .Append($"ReconnectMinDelay: {configImpl.ConsumerConfig.ReconnectMinDelay}{ILoggerClient.CR}")
                .Append($"ReconnectMaxDelay: {configImpl.ConsumerConfig.ReconnectMaxDelay}{ILoggerClient.CR}")
                .Append($"MsgKeyInUpdates: {configImpl.ConsumerConfig.MsgKeyInUpdates}{ILoggerClient.CR}")
                .Append($"DirectoryRequestTimeOut: {configImpl.ConsumerConfig.DirectoryRequestTimeOut}{ILoggerClient.CR}")
                .Append($"DictionaryRequestTimeOut: {configImpl.ConsumerConfig.DictionaryRequestTimeOut}{ILoggerClient.CR}")
                .Append($"RestRequestTimeOut: {configImpl.ConsumerConfig.RestRequestTimeOut}{ILoggerClient.CR}")
                .Append($"LoginRequestTimeOut: {configImpl.ConsumerConfig.LoginRequestTimeOut}");

            return strBuilder.ToString();
        }
    }
}
