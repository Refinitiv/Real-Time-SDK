/*|-----------------------------------------------------------------------------
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
            if (consumerRole.ReactorOAuthCredential is not null)
                consumerRole.ReactorOAuthCredential.ReactorOAuthCredentialEventCallback = credentialCallback;

            // finished configuring OAuth credential

            ConsumerWatchlistOptions watchlistOptions = consumerRole.WatchlistOptions;
            watchlistOptions.ChannelOpenEventCallback = this;

            reactorRole = consumerRole;

            InitializeReactor();
        }

        private void InitializeReactor()
        {
            string channelNames = "";
            reactorConnOptions = baseImpl.ConfigImpl.GenerateReactorConnectOpts();

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

                // Set the channelInfo on the ETA userSpecObject, this will
                channelInfo.ChannelConfig.ConnectInfo.ConnectOptions.UserSpecObject = channelInfo;
                channelList.Add(channelInfo);
                channelNames += channelName;
            }

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
                        if(baseImpl.LoggerClient.IsTraceEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelOpened event on channel {channelInfo?.ChannelConfig.Name}")
                                .Append($"Instance Name {baseImpl}");

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
                                .Append($"Instance Name {baseImpl.InstanceName}");

                            if(count > 0)
                            {
                                strBuilder.AppendLine().Append("Component Version ");
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
                                        .AppendLine($"Instance Name {baseImpl.InstanceName}");

                                    if(reactorChannel != null && reactorChannel.Channel != null)
                                    {
                                        strBuilder.AppendLine($"Reactor {reactorChannel.Reactor?.GetHashCode()}")
                                            .AppendLine($"Channel {reactorChannel.Channel.GetHashCode()}");
                                    }
                                    else
                                    {
                                        strBuilder.AppendLine($"Channel is null");
                                    }

                                    if (errorInfo != null)
                                    {
                                        strBuilder.AppendLine($"Error Id {errorInfo.Error.ErrorId}")
                                            .AppendLine($"Internal sysError {errorInfo.Error.SysError}")
                                            .AppendLine($"Error Location {errorInfo.Location}")
                                            .Append($"Error text {errorInfo.Error.Text}");
                                    }

                                    baseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                                }

                                baseImpl.CloseReactorChannel(reactorChannel);
                            }
                            else if (baseImpl.LoggerClient.IsInfoEnabled)
                            {
                                StringBuilder strBuilder = baseImpl.GetStrBuilder();
                                strBuilder.AppendLine($"high water mark set on channel {channelInfo.ChannelConfig.Name}")
                                    .Append($"Instance Name {baseImpl.InstanceName}");

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
                                .Append($"Instance Name {baseImpl}");

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
                                .Append($"Instance Name {baseImpl}");

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
                        if (baseImpl.LoggerClient.IsWarnEnabled)
                        {
                            StringBuilder strBuilder = baseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"Received ChannelDownReconnecting event on channel {channelInfo?.ChannelConfig.Name}")
                                .AppendLine($"Instance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"Reactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"Channel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"Channel is null");
                            }

                            strBuilder.AppendLine($"Error Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"Internal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"Error Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"Error text {evt.ReactorErrorInfo.Error.Text}");

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
                                .AppendLine($"Instance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"Reactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"Channel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"Channel is null");
                            }

                            strBuilder.AppendLine($"Error Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"Internal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"Error Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"Error text {evt.ReactorErrorInfo.Error.Text}");

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
                                .AppendLine($"Instance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"Reactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"Channel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"Channel is null");
                            }

                            strBuilder.AppendLine($"Error Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"Internal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"Error Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"Error text {evt.ReactorErrorInfo.Error.Text}");

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
                                .AppendLine($"Instance Name {baseImpl.InstanceName}");

                            if (reactorChannel.Channel != null)
                            {
                                strBuilder.AppendLine($"Reactor {reactorChannel.Reactor?.GetHashCode()}")
                                    .AppendLine($"Channel {reactorChannel.Channel.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine($"Channel is null");
                            }

                            strBuilder.AppendLine($"Error Id {evt.ReactorErrorInfo.Error.ErrorId}")
                                    .AppendLine($"Internal sysError {evt.ReactorErrorInfo.Error.SysError}")
                                    .AppendLine($"Error Location {evt.ReactorErrorInfo.Location}")
                                    .Append($"Error text {evt.ReactorErrorInfo.Error.Text}");

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

    }
}
