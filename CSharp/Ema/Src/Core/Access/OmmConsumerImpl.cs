/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Text;

namespace LSEG.Ema.Access
{
    internal class OmmConsumerImpl : OmmBaseImpl<IOmmConsumerClient>
    {
        private IOmmConsumerClient? m_AdminClient;
        internal object? m_AdminClosure;

        private IOmmConsumerErrorClient? m_ErrorClient;
        private RequestMsg m_LoginRequest = new RequestMsg();
        internal readonly IOmmOAuth2ConsumerClient? m_OAuthConsumerClient = null;

        internal OmmConsumer Consumer { get; private set; }

        // Reference to the OmmConsumerConfigImpl that's copied into OmmBaseImpl.OmmBaseConfigImpl
        internal OmmConsumerConfigImpl ConsumerConfigImpl { get; private set; }

        #region Constructors

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config) :
            base(config.OmmConsConfigImpl)
        {
            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmOAuth2ConsumerClient oauthClient, object? closure = null) :
            base(config.OmmConsConfigImpl)
        {
            m_OAuthConsumerClient = oauthClient;
            m_AdminClosure = closure;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmConsumerErrorClient errorClient) :
            base(config.OmmConsConfigImpl)
        {
            m_ErrorClient = errorClient;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmConsumerClient client, object? closure) :
            base(config.OmmConsConfigImpl)
        {
            m_AdminClient = client;
            m_AdminClosure = closure;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmConsumerClient client, IOmmOAuth2ConsumerClient oauthClient, object? closure) :
            base(config.OmmConsConfigImpl)
        {
            m_OAuthConsumerClient = oauthClient;

            m_AdminClient = client;
            m_AdminClosure = closure;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmConsumerErrorClient errorClient, IOmmOAuth2ConsumerClient oauthClient, object? closure = null) :
            base(config.OmmConsConfigImpl)
        {
            m_ErrorClient = errorClient;

            m_OAuthConsumerClient = oauthClient;
            m_AdminClosure = closure;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmConsumerClient client, IOmmConsumerErrorClient errorClient, object? closure = null) :
            base(config.OmmConsConfigImpl)
        {
            m_ErrorClient = errorClient;

            m_AdminClient = client;
            m_AdminClosure = closure;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        public OmmConsumerImpl(OmmConsumer ommConsumer, OmmConsumerConfig config, IOmmConsumerClient client, IOmmConsumerErrorClient errorClient,
            IOmmOAuth2ConsumerClient oauthClient, object? closure = null) : base(config.OmmConsConfigImpl)
        {
            m_ErrorClient = errorClient;

            m_AdminClient = client;
            m_OAuthConsumerClient = oauthClient;
            m_AdminClosure = closure;

            Consumer = ommConsumer;
            ConsumerConfigImpl = (OmmConsumerConfigImpl)OmmConfigBaseImpl;
        }

        #endregion

        public string ConsumerName { get; internal set; } = string.Empty;

        /// <summary>
        /// Retrieves channel information on the OmmConsumer object.
        /// </summary>
        /// <param name="channelInformation"><see cref="Access.ChannelInformation"/> instance to be filled</param>
        public override void ChannelInformation(ChannelInformation channelInformation)
        {
            if (LoginCallbackClient == null || LoginCallbackClient.m_LoginChannelList.Count == 0)
            {
                channelInformation.Clear();
                return;
            }

            try
            {
                UserLock.Enter();

                ReactorChannel? reactorChannel = null;
                // return first item in channel list with proper status
                foreach (ChannelInfo ci in LoginCallbackClient.m_LoginChannelList)
                {
                    if (ci.ReactorChannel!.State == ReactorChannelState.READY || ci.ReactorChannel!.State == ReactorChannelState.UP)
                    {
                        reactorChannel = ci.ReactorChannel;
                        break;
                    }
                }

                // if reactorChannel is not set, then just use the first element in LoginCallbackClient.m_LoginChannelList
                if (reactorChannel == null)
                    reactorChannel = LoginCallbackClient.m_LoginChannelList[0].ReactorChannel;

                channelInformation.Hostname = reactorChannel!.HostName;
                channelInformation.IpAddress = "not available for OmmConsumer connections";
                channelInformation.Port = reactorChannel.Port;

                ReactorChannelInfo rci = new ReactorChannelInfo();
                if (reactorChannel.Info(rci, out _) != ReactorReturnCode.SUCCESS)
                {
                    channelInformation.ComponentInfo = "unavailable";
                    channelInformation.EncryptionProtocol = System.Security.Authentication.SslProtocols.None;
                }
                else
                {
                    channelInformation.EncryptionProtocol = rci.ChannelInfo.EncryptionProtocol;
                    channelInformation.MaxOutputBuffers = rci.ChannelInfo.MaxOutputBuffers;
                    channelInformation.NumInputBuffers = rci.ChannelInfo.NumInputBuffers;
                    channelInformation.CompressionThreshold = rci.ChannelInfo.CompressionThresHold;
                    channelInformation.CompressionType = Access.ChannelInformation.EtaToEmaCompressionType(rci.ChannelInfo.CompressionType);
                    channelInformation.GuaranteedOutputBuffers = rci.ChannelInfo.GuaranteedOutputBuffers;
                    channelInformation.MaxFragmentSize = rci.ChannelInfo.MaxFragmentSize;
                    channelInformation.SysRecvBufSize = rci.ChannelInfo.SysRecvBufSize;
                    channelInformation.SysSendBufSize = rci.ChannelInfo.SysSendBufSize;

                    if (rci.ChannelInfo == null || rci.ChannelInfo.ComponentInfoList == null || rci.ChannelInfo.ComponentInfoList.Count == 0)
                        channelInformation.ComponentInfo = "unavailable";
                    else
                    {
                        channelInformation.ComponentInfo = rci.ChannelInfo.ComponentInfoList[0].ComponentVersion.ToString();
                    }
                }

                IChannel? channel = reactorChannel.Channel;

                if (channel != null)
                {
                    channelInformation.ChannelState = Access.ChannelInformation.EtaToEmaChannelState(channel.State);
                    channelInformation.ConnectionType = Access.ChannelInformation.EtaToEmaConnectionType(channel.ConnectionType);

                    if(channelInformation.ConnectionType == ConnectionType.ENCRYPTED)
                    {
                        channelInformation.EncryptedConnectionType = ConnectionType.SOCKET;
                    }

                    channelInformation.ProtocolType = Access.ChannelInformation.EtaToEmaProtocolType(channel.ProtocolType);
                    channelInformation.MajorVersion = channel.MajorVersion;
                    channelInformation.MinorVersion = channel.MinorVersion;
                    channelInformation.PingTimeout = channel.PingTimeOut;
                }
                else
                {
                    channelInformation.ChannelState = ChannelState.INACTIVE;
                    channelInformation.ConnectionType = ConnectionType.UNIDENTIFIED;
                    channelInformation.EncryptedConnectionType = ConnectionType.UNIDENTIFIED;

                    channelInformation.ProtocolType = ProtocolType.UNKNOWN;
                    channelInformation.MajorVersion = 0;
                    channelInformation.MinorVersion = 0;
                    channelInformation.PingTimeout = 0;
                }
            }
            finally
            {
                UserLock.Exit();
            }

        }

        protected override void HandleAdminDomains()
        {
            LoginCallbackClient = new LoginCallbackClientConsumer(this);
            LoginCallbackClient.Initialize();

            DirectoryCallbackClient = new DirectoryCallbackClientConsumer(this);
            DirectoryCallbackClient.Initialize();

            DictionaryCallbackClient = new DictionaryCallbackClientConsumer(this);
            DictionaryCallbackClient.Initialize();

            ItemCallbackClient = new ItemCallbackClientConsumer(this);
            ItemCallbackClient.Initialize();

            if (m_AdminClient != null)
            {
                /* RegisterClient does not require a fully encoded login message to set the callbacks */
                m_LoginRequest.Clear();
                m_LoginRequest.DomainType(Rdm.EmaRdm.MMT_LOGIN);
                ItemCallbackClient.RegisterClient(m_LoginRequest, m_AdminClient, m_AdminClosure);
            }

            if (m_OAuthConsumerClient is null)
                OAuthCallbackClient = null;
            else
                OAuthCallbackClient = new OAuthCallbackClientConsumer(this);

            ChannelCallbackClient = new ChannelCallbackClient<IOmmConsumerClient>(this, reactor!);
            ChannelCallbackClient.InitializeConsumerRole(OAuthCallbackClient);

            ChannelCallbackClient.InitializeEmaManagerItemPools();

            HandleLoginReqTimeout();
            HandleDirectoryReqTimeout();
            HandleDictionaryReqTimeout();
        }

        public void HandleDirectoryReqTimeout()
        {
            long directoryRequestTimeOut = ConsumerConfigImpl.ConsumerConfig.DirectoryRequestTimeOut;
            if (directoryRequestTimeOut == 0)
            {
                while (ImplState < OmmImplState.DIRECTORY_STREAM_OPEN_OK)
                    base.ReactorDispatchLoop(DispatchTimeoutApiThread, MaxDispatchCountApiThread);
            }
            else
            {
                m_EventTimeout = false;
                TimeoutEvent timeoutEvent = TimeoutEventManager!.AddTimeoutEvent(directoryRequestTimeOut * 1000, this);

                while (!m_EventTimeout && (ImplState < OmmImplState.DIRECTORY_STREAM_OPEN_OK))
                {
                    ReactorDispatchLoop(DispatchTimeoutApiThread, MaxDispatchCountApiThread);
                }

                if (m_EventTimeout)
                {
                    ChannelInfo? channelInfo = LoginCallbackClient!.ActiveChannelInfo();
                    ClientChannelConfig? channelConfig = channelInfo?.ChannelConfig;

                    StringBuilder message = GetStrBuilder();
                    message.Append($"directory retrieval failed (timed out after waiting {directoryRequestTimeOut} milliseconds) for ");

                    if (channelConfig != null)
                    {
                        message.Append($"{channelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.Address}:" +
                            $"{channelConfig.ConnectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName})");
                    }

                    if (LoggerClient.IsErrorEnabled)
                        LoggerClient.Error(InstanceName, message.ToString());

                    throw new OmmInvalidUsageException(message.ToString(), OmmInvalidUsageException.ErrorCodes.DIRECTORY_REQUEST_TIME_OUT);
                }
                else
                    timeoutEvent.Cancel();
            }
        }

        internal void HandleDictionaryReqTimeout()
        {
            long dictionaryRequestTimeOut = ConsumerConfigImpl.ConsumerConfig.DictionaryRequestTimeOut;

            if (dictionaryRequestTimeOut == 0)
            {
                while (!DictionaryCallbackClient!.IsDictionaryReady)
                    ReactorDispatchLoop(DispatchTimeoutApiThread, MaxDispatchCountApiThread);
            }
            else
            {
                m_EventTimeout = false;

                TimeoutEvent timeoutEvent = TimeoutEventManager!.AddTimeoutEvent(dictionaryRequestTimeOut * 1000, this);

                while (!m_EventTimeout && !DictionaryCallbackClient!.IsDictionaryReady)
                {
                    ReactorDispatchLoop(DispatchTimeoutApiThread, MaxDispatchCountApiThread);
                }

                if (m_EventTimeout)
                {
                    StringBuilder message = GetStrBuilder();

                    message.Append("Dictionary retrieval failed (timed out after waiting ")
                        .Append(dictionaryRequestTimeOut).Append(" milliseconds) for ");
                    ChannelInfo? loginChanInfo = LoginCallbackClient!.ActiveChannelInfo();

                    string excepText = message.ToString();

                    if (LoggerClient.IsErrorEnabled)
                    {
                        LoggerClient.Error(InstanceName, excepText);
                    }

                    HandleInvalidUsage(excepText, OmmInvalidUsageException.ErrorCodes.DICTIONARY_REQUEST_TIME_OUT);
                }
                else
                    timeoutEvent.Cancel();
            }
        }

        public override void Uninitialize()
        {
            base.Uninitialize();

            if(LoginCallbackClient != null)
            {
                LoginCallbackClient.EventImpl.m_OmmConsumer = null;
            }

            if(DirectoryCallbackClient != null)
            {
                DirectoryCallbackClient.EventImpl.m_OmmConsumer = null;
            }

            if(DictionaryCallbackClient != null)
            {
                DictionaryCallbackClient.EventImpl.m_OmmConsumer = null;
            }
                
            if (ItemCallbackClient != null)
            {
                ItemCallbackClient.EventImpl.m_OmmConsumer = null;
            }
#pragma warning disable CS8625 // Cannot convert null literal to non-nullable reference type.
            Consumer = null;
#pragma warning restore CS8625 // Cannot convert null literal to non-nullable reference type.
        }

        public override void Reissue(RequestMsg requestMsg, long handle)
        {
            base.Reissue(requestMsg, handle);
        }

        public override void Submit(GenericMsg genericMsg, long handle)
        {
            base.Submit(genericMsg, handle);
        }

        public override void Submit(PostMsg postMsg, long handle)
        {
            base.Submit(postMsg, handle);
        }

        public override void Unregister(long handle)
        {
            base.Unregister(handle);
        }

        protected override bool HasErrorClient()
        {
            return m_ErrorClient != null;
        }

        public override void HandleInvalidUsage(string text, int errorCode)
        {
            if (m_ErrorClient != null)
            {
                m_ErrorClient.OnInvalidUsage(text, errorCode);
            }
            else
            {
                throw new OmmInvalidUsageException(text, errorCode);
            }
        }

        public override void HandleInvalidHandle(long handle, string text)
        {
            if (m_ErrorClient != null)
            {
                m_ErrorClient.OnInvalidHandle(handle, text);
            }
            else
            {
                throw new OmmInvalidHandleException(handle, text);
            }
        }

        internal override IOmmCommonImpl.ImpleType GetImplType()
        {
            return IOmmCommonImpl.ImpleType.CONSUMER;
        }

        protected override void NotifyErrorClient(OmmException ommException)
        {
            if (m_ErrorClient != null)
            {
                switch (ommException.Type)
                {
                    case OmmException.ExceptionType.OmmInvalidUsageException:
                        OmmInvalidUsageException iue = (OmmInvalidUsageException)ommException;
                        m_ErrorClient.OnInvalidUsage(iue.Message, iue.ErrorCode);
                        break;
                    case OmmException.ExceptionType.OmmInvalidHandleException:
                        OmmInvalidHandleException ihe = (OmmInvalidHandleException)ommException;
                        m_ErrorClient.OnInvalidHandle(ihe.Handle, ihe.Message);
                        break;
                    default:
                        throw new NotImplementedException($"Unknown exception type: {ommException.Type}");
                }
            }
        }

        internal override long NextLongId()
        {
            return LongIdGenerator.NextLongId();
        }

        internal void ModifyIOCtl(IOCtlCode code, int val)
        {
            UserLock.Enter();

            try
            {
                base.ModifyIOCtl(code, val, LoginCallbackClient!.ActiveChannelInfo());
            }
            finally
            {
                UserLock.Exit();
            }
        }

        protected override void OnDispatchError(string text, int errorCode)
        {
            m_ErrorClient?.OnDispatchError(text, errorCode);
        }
    }
}
