﻿/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Codec;
using System.Text;
using System.Collections.Generic;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Transports;
using System.Threading;

namespace LSEG.Ema.Access
{
    internal sealed class ChannelInfo
    {
        internal ChannelInfo? ParentChannel { get; set; }

        public ClientChannelConfig ChannelConfig { get; private set; }
        public Reactor Reactor { get; private set; }
        public ReactorChannel? ReactorChannel { get; set; }
        public DataDictionary? DataDictionary { get; set; }

        public ChannelInfo(ClientChannelConfig config, Reactor reactor)
        {
            ChannelConfig = config;
            Reactor = reactor;
        }
    }

    internal sealed class ChannelCallbackClient<T> : IReactorChannelEventCallback
    {
        private static readonly string CLIENT_NAME = "ChannelCallbackClient";
        private List<ChannelInfo> channelList = new();
        private OmmBaseImpl<T> baseImpl;
        private Reactor reactor;
        private ReactorRole? reactorRole;
        private ReactorConnectOptions? reactorConnOptions;
        private ReactorChannelInfo m_ReactorChannelInfo = new();
        bool initialChannelReadyEventReceived;
        private static readonly ProxyOptions m_ProxyOptions = new();
        internal int InitialChannelConnectIndex = 0; /* This is used to keep track the channel index for the initial connection only */

        public ChannelCallbackClient(OmmBaseImpl<T> baseImpl, Reactor reactor)
        {
            this.baseImpl = baseImpl;
            this.reactor = reactor;

            if(baseImpl.LoggerClient.IsTraceEnabled)
            {
                baseImpl.LoggerClient.Trace(CLIENT_NAME, "Created ChannelCallbackClient");
            }
        }

        // EmaConfigImpl as third parameter.
        public void InitializeConsumerRole(IReactorOAuthCredentialEventCallback? credentialCallback = null)
        {
            // Generate the role based on the ConfigImpl.
            ConsumerRole consumerRole = baseImpl.ConfigImpl.GenerateConsumerRole();

            LoginRequest loginReq = consumerRole.RdmLoginRequest!;
            loginReq.HasRole = true;
            loginReq.Role = Eta.Rdm.Login.RoleTypes.CONS;
            consumerRole.RdmDirectoryRequest = baseImpl.DirectoryCallbackClient!.DirectoryRequest;
            consumerRole.DictionaryDownloadMode = DictionaryDownloadMode.NONE;
            consumerRole.LoginMsgCallback = baseImpl.LoginCallbackClient;
            consumerRole.DirectoryMsgCallback = baseImpl.DirectoryCallbackClient;
            consumerRole.DictionaryMsgCallback = baseImpl.DictionaryCallbackClient;
            consumerRole.ChannelEventCallback = baseImpl.ChannelCallbackClient;
            consumerRole.DefaultMsgCallback = baseImpl.ItemCallbackClient;

            // configure OAuth credential
            if (consumerRole.ReactorOAuthCredential != null)
                consumerRole.ReactorOAuthCredential.ReactorOAuthCredentialEventCallback = credentialCallback;

            // finished configuring OAuth credential

            ConsumerWatchlistOptions watchlistOptions = consumerRole.WatchlistOptions;
            watchlistOptions.ChannelOpenEventCallback = this;

            reactorRole = consumerRole;

            InitializeReactor();
        }

        public void InitializeEmaManagerItemPools()
        {
            if (reactorRole != null && reactorRole is ConsumerRole)
            {
                baseImpl.GetEmaObjManager().GrowSingleItemPool<T>((int)((ConsumerRole)reactorRole).WatchlistOptions.ItemCountHint);
            }
        }

        private void InitializeReactor()
        {
            string channelNames = "";
            reactorConnOptions = baseImpl.ConfigImpl.GenerateReactorConnectOpts();
            StringBuilder? stringBuilder = null;
            int supportedConnectionTypeChannelCount = 1;
            string channelParams;

            if (baseImpl.LoggerClient.IsTraceEnabled)
            {
                stringBuilder = new StringBuilder(2048);
                if (baseImpl.ConfigImpl.ConsumerConfig.ChannelSet.Count > 1)
                {
                    stringBuilder.Append("Attempt to connect using the following list");
                }
                else
                {
                    stringBuilder.Append("Attempt to connect using");
                }
            }

            StringBuilder errorStrUnsupportedConnectionType = new StringBuilder();
            errorStrUnsupportedConnectionType.Append("Unsupported connection type. Passed in type is ");
            int channelCount = 1;

            foreach (string channelName in baseImpl.ConfigImpl.ConsumerConfig.ChannelSet)
            {
                ChannelInfo channelInfo = new ChannelInfo(baseImpl.ConfigImpl.ClientChannelConfigMap[channelName], reactor);

                // If proxy options were set by functions, override them for all connections.
                if (string.IsNullOrEmpty(baseImpl.ConfigImpl.ProxyHost) == false)
                {
                    channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyHostName = baseImpl.ConfigImpl.ProxyHost;
                }

                if (string.IsNullOrEmpty(baseImpl.ConfigImpl.ProxyPort) == false)
                {
                    channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPort = baseImpl.ConfigImpl.ProxyPort;
                }

                if (string.IsNullOrEmpty(baseImpl.ConfigImpl.ProxyUserName) == false)
                {
                    channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyUserName = baseImpl.ConfigImpl.ProxyUserName;
                }

                if (string.IsNullOrEmpty(baseImpl.ConfigImpl.ProxyPassword) == false)
                {
                    channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions.ProxyPassword = baseImpl.ConfigImpl.ProxyPassword;
                }


                if (channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ConnectionType == Eta.Transports.ConnectionType.SOCKET ||
                    channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ConnectionType == Eta.Transports.ConnectionType.ENCRYPTED)
                {
                    /* Checks whether any proxy options is set. */
                    if (!m_ProxyOptions.Equals(channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions)
                        && baseImpl.LoggerClient.IsTraceEnabled)
                    {
                        StringBuilder strBuilder = baseImpl.GetStrBuilder();
                        strBuilder.Append($"Successfully set proxy options{ILoggerClient.CR}")
                        .Append($"Channel name {channelName}{ILoggerClient.CR}")
                        .Append($"Instance Name {baseImpl.InstanceName}{ILoggerClient.CR}")
                        .Append($"with the following proxy configurations: " +
                        $"{channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ProxyOptions}{ILoggerClient.CR}");

                        baseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                    }

                    if(channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ConnectionType == Eta.Transports.ConnectionType.ENCRYPTED)
                    {
                        /* Overrides the encryption parameters from OmmConsumerConfig */

                        if(baseImpl.ConfigImpl.SetEncryptedProtocolFlags)
                        {
                            channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags = 
                                (EncryptionProtocolFlags)baseImpl.ConfigImpl.EncryptedTLSProtocolFlags;
                        }

                        if(baseImpl.ConfigImpl.CipherSuites != null)
                        {
                            channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.EncryptionOpts.TlsCipherSuites =
                                baseImpl.ConfigImpl.CipherSuites;
                        }
                    }

                    // Set the channelInfo on the ETA userSpecObject, this will
                    channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.UserSpecObject = channelInfo;
                    channelList.Add(channelInfo);
                    channelNames += channelName;

                    if (baseImpl.LoggerClient.IsTraceEnabled && stringBuilder != null)
                    {
                        channelParams = ChannelParametersToString(reactorConnOptions, channelInfo.ChannelConfig);
                        stringBuilder.AppendLine().Append($"\t{supportedConnectionTypeChannelCount}] ").Append(channelParams);
                        if (supportedConnectionTypeChannelCount == baseImpl.ConfigImpl.ConsumerConfig.ChannelSet.Count)
                        {
                            baseImpl.LoggerClient.Trace(CLIENT_NAME, stringBuilder.ToString());
                        }
                        else
                        {
                            ++supportedConnectionTypeChannelCount;
                        }
                    }
                }
                else
                {
                    errorStrUnsupportedConnectionType.Append($"\t{channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.ConnectionType}")
                        .Append($" for {channelName}");
                    if (channelCount < baseImpl.ConfigImpl.ConsumerConfig.ChannelSet.Count - 1)
                        errorStrUnsupportedConnectionType.Append(", ");
                }

                ++channelCount;
            }

            if (supportedConnectionTypeChannelCount > 0)
            {
                if (reactor.Connect(reactorConnOptions, reactorRole!, out var errorInfo) < ReactorReturnCode.SUCCESS)
                {
                    Error error = errorInfo!.Error;
                    StringBuilder strBuilder = baseImpl.GetStrBuilder();
                    strBuilder.AppendLine($"Failed to add RsslChannel(s) to RsslReactor. Channel name(s) {channelNames}")
                        .AppendLine($"Instance Name {baseImpl.InstanceName}").AppendLine($"Reactor {reactor.GetHashCode()}")
                        .AppendLine($"Channel {error.Channel}").AppendLine($"Error Id {error.ErrorId}")
                        .AppendLine($"System error {error.SysError}").AppendLine($"Error location {errorInfo.Location}")
                        .AppendLine($"Error text {error.Text}");

                    throw new OmmInvalidUsageException(strBuilder.ToString(), (int)errorInfo.Code);
                }

                Interlocked.Exchange(ref baseImpl.ImplState, OmmBaseImpl<T>.OmmImplState.CHANNEL_DOWN);

                if (baseImpl.LoggerClient.IsTraceEnabled)
                {
                    StringBuilder strBuilder = baseImpl.GetStrBuilder();
                    strBuilder.AppendLine($"Successfully created a Reactor and Channel(s)")
                        .AppendLine($"\tChannel name(s) {channelNames}")
                        .AppendLine($"\tInstance name {baseImpl.InstanceName}");

                    baseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                }
            }
            else
            {
                if (baseImpl.LoggerClient.IsErrorEnabled)
                {
                    baseImpl.LoggerClient.Error(CLIENT_NAME, errorStrUnsupportedConnectionType.ToString());
                }

                throw new OmmInvalidUsageException(errorStrUnsupportedConnectionType.ToString(),
                    OmmInvalidUsageException.ErrorCodes.UNSUPPORTED_CHANNEL_TYPE);
            }
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            baseImpl.EventReceived();
            ReactorChannel reactorChannel = evt.ReactorChannel!;
            ChannelInfo? channelInfo = (ChannelInfo?)reactorChannel.UserSpecObj;

            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_OPENED:
                    {
                        InitialChannelConnectIndex = 0;

                        if (baseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelOpened event on channel {channelInfo?.ChannelConfig.Name}")
                                .Append($"\tInstance Name {baseImpl.InstanceName}");

                            baseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_UP:
                    {
                        baseImpl.RegisterSocket(reactorChannel.Socket!);

                        baseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP);

                        m_ReactorChannelInfo.Clear();

                        SetRsslReactorChannel(reactorChannel, m_ReactorChannelInfo, out _);

                        if(baseImpl.LoggerClient.IsInfoEnabled && m_ReactorChannelInfo.ChannelInfo.ComponentInfoList != null)
                        {
                            int count = m_ReactorChannelInfo.ChannelInfo.ComponentInfoList.Count;

                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelUp event on channel {channelInfo?.ChannelConfig.Name}")
                                .Append($"\tInstance Name {baseImpl.InstanceName}");

                            if(count > 0)
                            {
                                strBuilder.AppendLine().Append("\tComponent Version ");
                                for(int i = 0; i < count; i++)
                                {
                                    strBuilder.Append(m_ReactorChannelInfo.ChannelInfo.ComponentInfoList[i].ComponentVersion.ToString());
                                    if (i < count - 1)
                                        strBuilder.Append(", ");
                                }
                            }

                            baseImpl.LoggerClient.Info(CLIENT_NAME, strBuilder.ToString());
                        }

                        if(channelInfo != null && channelInfo.ChannelConfig.HighWaterMark > 0)
                        {
                            if(reactorChannel.IOCtl(Eta.Transports.IOCtlCode.HIGH_WATER_MARK, channelInfo.ChannelConfig.HighWaterMark, out var errorInfo) 
                                != ReactorReturnCode.SUCCESS)
                            {
                                if(baseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    StringBuilder strBuilder = baseImpl.GetStrBuilder();
                                    strBuilder.AppendLine($"Failed to set high water mark on channel {channelInfo.ChannelConfig.Name}")
                                        .AppendLine($"\tInstance Name {baseImpl.InstanceName}");

                                    if(reactorChannel != null && reactorChannel.Channel != null)
                                    {
                                        strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                            .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                                    }
                                    else
                                    {
                                        strBuilder.AppendLine($"\tChannel is null");
                                    }

                                    if (errorInfo != null)
                                    {
                                        strBuilder.AppendLine($"\tError Id {errorInfo.Error.ErrorId}")
                                            .AppendLine($"\tInternal sysError {errorInfo.Error.SysError}")
                                            .AppendLine($"\tError Location {errorInfo.Location}")
                                            .Append($"\tError text {errorInfo.Error.Text}");
                                    }

                                    baseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                                }

                                baseImpl.CloseReactorChannel(reactorChannel);

                                break;
                            }
                            else if (baseImpl.LoggerClient.IsInfoEnabled)
                            {
                                StringBuilder strBuilder = baseImpl.GetStrBuilder();
                                strBuilder.AppendLine($"high water mark set on channel {channelInfo.ChannelConfig.Name}")
                                    .Append($"\tInstance Name {baseImpl.InstanceName}");

                                baseImpl.LoggerClient.Info(CLIENT_NAME, strBuilder.ToString());
                            }
                        }

                        if (channelInfo != null && channelInfo.ChannelConfig.CompressionThresholdSet)
                        {
                            if (reactorChannel.IOCtl(Eta.Transports.IOCtlCode.COMPRESSION_THRESHOLD, channelInfo.ChannelConfig.CompressionThreshold, out var errorInfo)
                                != ReactorReturnCode.SUCCESS)
                            {
                                if (baseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    StringBuilder strBuilder = baseImpl.GetStrBuilder();
                                    strBuilder.AppendLine($"Failed to set compression threshold on channel {channelInfo.ChannelConfig.Name}")
                                        .AppendLine($"\tInstance Name {baseImpl.InstanceName}");

                                    if (reactorChannel != null && reactorChannel.Channel != null)
                                    {
                                        strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                            .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                                    }
                                    else
                                    {
                                        strBuilder.AppendLine($"\tChannel is null");
                                    }

                                    if (errorInfo != null)
                                    {
                                        strBuilder.AppendLine($"\tError Id {errorInfo.Error.ErrorId}")
                                            .AppendLine($"\tInternal sysError {errorInfo.Error.SysError}")
                                            .AppendLine($"\tError Location {errorInfo.Location}")
                                            .Append($"\tError text {errorInfo.Error.Text}");
                                    }

                                    baseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                                }

                                baseImpl.CloseReactorChannel(reactorChannel);

                                break;
                            }
                            else if (baseImpl.LoggerClient.IsInfoEnabled)
                            {
                                StringBuilder strBuilder = baseImpl.GetStrBuilder();
                                strBuilder.AppendLine($"compression threshold set on channel {channelInfo.ChannelConfig.Name}")
                                    .Append($"\tInstance Name {baseImpl.InstanceName}");

                                baseImpl.LoggerClient.Info(CLIENT_NAME, strBuilder.ToString());
                            }
                        }

                        break;
                    }
                case ReactorChannelEventType.FD_CHANGE:
                    {
                        baseImpl.UnregisterSocket(reactorChannel.OldSocket!);

                        baseImpl.RegisterSocket(reactorChannel.Socket!);

                        if (baseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received FD Change event on channel {channelInfo?.ChannelConfig.Name}")
                                .Append($"\tInstance Name {baseImpl.InstanceName}");

                            baseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_READY:
                    {
                        if (baseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelReady event on channel {channelInfo?.ChannelConfig.Name}")
                                .Append($"\tInstance Name {baseImpl.InstanceName}");

                            baseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                        }

                        baseImpl.ProcessChannelEvent(evt);

                        if (initialChannelReadyEventReceived)
                        {
                            baseImpl.LoginCallbackClient!.ProcessChannelEvent(evt);
                        }
                        else
                        {
                            initialChannelReadyEventReceived = true;
                        }

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    {
                        if ((InitialChannelConnectIndex + 1) < baseImpl.ConfigImpl.ConsumerConfig.ChannelSet.Count)
                        {
                            ++InitialChannelConnectIndex;
                        }
                        else
                        {
                            InitialChannelConnectIndex = 0;
                        }

                        if (baseImpl.LoggerClient.IsWarnEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelDownReconnecting event on channel {channelInfo?.ChannelConfig.Name}")
                                .AppendLine($"\tInstance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            strBuilder.AppendLine($"\tError Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"\tError text {evt.ReactorErrorInfo.Error.Text}");

                            baseImpl.LoggerClient.Warn(CLIENT_NAME, strBuilder.ToString());
                        }

                        baseImpl.UnregisterSocket(reactorChannel.Socket!);

                        baseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_DOWN);

                        baseImpl.ProcessChannelEvent(evt);

                        baseImpl.LoginCallbackClient!.ProcessChannelEvent(evt);

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        if (baseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelDown event on channel {channelInfo?.ChannelConfig.Name}")
                                .AppendLine($"\tInstance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            strBuilder.AppendLine($"\tError Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"\tError text {evt.ReactorErrorInfo.Error.Text}");

                            baseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                        }

                        baseImpl.UnregisterSocket(reactorChannel.Socket!);

                        baseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_DOWN);

                        baseImpl.ProcessChannelEvent(evt);

                        baseImpl.LoginCallbackClient!.ProcessChannelEvent(evt);

                        baseImpl.CloseReactorChannel(evt.ReactorChannel);

                        break;
                    }
                case ReactorChannelEventType.WARNING:
                    {
                        if (baseImpl.LoggerClient.IsWarnEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received Channel warning event on channel {channelInfo?.ChannelConfig.Name}")
                                .AppendLine($"\tInstance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            strBuilder.AppendLine($"\tError Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"\tError text {evt.ReactorErrorInfo.Error.Text}");

                            baseImpl.LoggerClient.Warn(CLIENT_NAME, strBuilder.ToString());
                        }

                        break;
                    }
                default:
                    {
                        if (baseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received unknown channel event type {evt.EventType} on channel {channelInfo?.ChannelConfig.Name}")
                                .AppendLine($"\tInstance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"\tReactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"\tChannel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"\tChannel is null");
                            }

                            strBuilder.AppendLine($"\tError Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"\tInternal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"\tError Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"\tError text {evt.ReactorErrorInfo.Error.Text}");

                            baseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                        }

                        return ReactorCallbackReturnCode.FAILURE;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        internal void CloseChannels()
        {
            for (int index = channelList.Count - 1; index >= 0; index--)
            {
                baseImpl.CloseReactorChannel(channelList[index].ReactorChannel);
            }
        }

        internal void RemoveChannel(ChannelInfo? channelInfo)
        {
            if (channelInfo != null)
            {
                baseImpl.LoginCallbackClient!.RemoveChannelInfo(channelInfo.ReactorChannel);
                channelList.Remove(channelInfo);
            }
        }

        internal ChannelInfo? GetChannelInfo(int index)
        {
            if(channelList != null && index < channelList.Count)
            {
                return channelList[index];
            }

            return null;
        }

        private void SetRsslReactorChannel(ReactorChannel reactorChannel, ReactorChannelInfo reactorChannlInfo,
            out ReactorErrorInfo? reactorErrorInfo)
        {
            reactorErrorInfo = null;
            for (int index = channelList.Count - 1; index >= 0; index--)
            {
                channelList[index].ReactorChannel = reactorChannel;
                channelList[index].ReactorChannel?.Info(reactorChannlInfo, out reactorErrorInfo);
            }
        }

        private string ChannelParametersToString(ReactorConnectOptions reactorConnectOpts, ClientChannelConfig channelConfig)
        {
            bool isValidChType = true;
            StringBuilder cfgParameters = new(256);
            string compType;
            string strConnectionType = "SOCKET";
            ReactorConnectInfo reactorConnectInfo = channelConfig.ConnectInfo;
            switch (reactorConnectInfo.ConnectOptions.CompressionType)
            {
                case Eta.Transports.CompressionType.ZLIB:
                    {
                        compType = "ZLib";
                        break;
                    }
                case Eta.Transports.CompressionType.LZ4:
                    {
                        compType = "LZ4";
                        break;
                    }
                case Eta.Transports.CompressionType.NONE:
                    {
                        compType = "None";
                        break;
                    }
                default:
                    {
                        compType = "Unknown Compression Type";
                        break;
                    }
            }

            switch(reactorConnectInfo.ConnectOptions.ConnectionType)
            {
                case Eta.Transports.ConnectionType.SOCKET:
                case Eta.Transports.ConnectionType.ENCRYPTED:
                    {
                        cfgParameters.AppendLine($"\tHostName {reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address}")
                            .AppendLine($"\tPort {reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName}")
                            .AppendLine($"\tCompressionType {compType}")
                            .AppendLine($"\tTcpNodelay {reactorConnectInfo.ConnectOptions.TcpOpts.TcpNoDelay}")
                            .AppendLine($"\tEnableSessionMgnt {reactorConnectInfo.EnableSessionManagement}")
                            .AppendLine($"\tLocation {reactorConnectInfo.Location}");

                        /*  */
                        if (reactorConnectInfo.ConnectOptions.ConnectionType == Eta.Transports.ConnectionType.ENCRYPTED)
                        {
                            strConnectionType = "ENCRYPTED";

                            cfgParameters.AppendLine($"\tEncryptedProtocolType {reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptedProtocol}");
                            cfgParameters.AppendLine($"\tEncryptedProtocolFlags {reactorConnectInfo.ConnectOptions.EncryptionOpts.EncryptionProtocolFlags}");
                            cfgParameters.AppendLine($"\tAuthenticationTimeout {reactorConnectInfo.ConnectOptions.EncryptionOpts.AuthenticationTimeout/1000} sec");

                            if (reactorConnectInfo.ConnectOptions.EncryptionOpts.TlsCipherSuites != null)
                            {
                                cfgParameters.Append($"\tTlsCipherSuites:");
                                foreach (var cipher in reactorConnectInfo.ConnectOptions.EncryptionOpts.TlsCipherSuites)
                                {
                                    cfgParameters.Append($" {cipher}");
                                }

                                cfgParameters.AppendLine();
                            }
                        }
                        break;
                    }
                default:
                    {
                        strConnectionType = $"Invalid ChannelType: {reactorConnectInfo.ConnectOptions.ConnectionType}";
                        isValidChType = false;
                        break;
                    }
            }

            StringBuilder strBuilder = new(1024);
            strBuilder.AppendLine(strConnectionType).AppendLine($"\tChannel name {channelConfig.Name}")
                .AppendLine($"\tInstance name {baseImpl.InstanceName}");

            if(isValidChType)
            {
                strBuilder.AppendLine($"\tReactor @{baseImpl.reactor?.GetHashCode()}")
                    .AppendLine($"\tInterfaceName {reactorConnectInfo.ConnectOptions.UnifiedNetworkInfo.InterfaceName}")
                    .Append(cfgParameters)
                    .AppendLine($"\tReconnectAttemptLimit {reactorConnectOpts.GetReconnectAttemptLimit()}")
                    .AppendLine($"\tReconnectMinDelay {reactorConnectOpts.GetReconnectMinDelay()} msec")
                    .AppendLine($"\tReconnectMaxDelay {reactorConnectOpts.GetReconnectMaxDelay()} msec")
                    .AppendLine($"\tGuaranteedOutputBuffers {reactorConnectInfo.ConnectOptions.GuaranteedOutputBuffers}")
                    .AppendLine($"\tNumInputBuffers {reactorConnectInfo.ConnectOptions.NumInputBuffers}")
                    .AppendLine($"\tSysRecvBufSize {reactorConnectInfo.ConnectOptions.SysRecvBufSize}")
                    .AppendLine($"\tSysSendBufSize {reactorConnectInfo.ConnectOptions.SysSendBufSize}")
                    .AppendLine($"\tConnectionPingTimeout {reactorConnectInfo.ConnectOptions.PingTimeout} sec")
                    .AppendLine($"\tInitializationTimeout {reactorConnectInfo.GetInitTimeout()} sec");
            }

            return strBuilder.ToString();
        }

    }
}
