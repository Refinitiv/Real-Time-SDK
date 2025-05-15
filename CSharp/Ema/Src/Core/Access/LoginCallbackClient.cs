/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Text;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class LoginItem<T> : SingleItem<T>, ITimeoutClient
    {
        private static readonly string CLIENT_NAME = "LoginItem";

        internal LoginRequest m_loginRequest;
        internal List<ChannelInfo>? LoginChannelList { get; set; }

#pragma warning disable CS8618
        public LoginItem() : base()
        {
            m_type = ItemType.LOGIN_ITEM;
        }
#pragma warning restore CS8618

        public LoginItem(OmmBaseImpl<T> baseImpl, T client, object? closure) : base(baseImpl, client, closure, null)
        {
            m_loginRequest = baseImpl.LoginCallbackClient!.CurrentLoginRequest;
            StreamId = 1;
            m_type = ItemType.LOGIN_ITEM;
        }

        public void ResetLoginItem(OmmBaseImpl<T> baseImpl, T client, object? closure)
        {
            base.Reset(baseImpl, client, closure, null);
            m_loginRequest = baseImpl.LoginCallbackClient!.CurrentLoginRequest;
            StreamId = 1;
        }

        public void HandleTimeoutEvent()
        {
            LoginCallbackClient<T> loginCallbackClient = m_OmmBaseImpl.LoginCallbackClient!;
            ConsumerSession<T>? consumerSession = m_OmmBaseImpl.ConsumerSession;
            List<SessionChannelInfo<IOmmConsumerClient>>? sessionChannelList = (consumerSession != null && consumerSession.SessionChannelList.Count > 0)
                ? consumerSession.SessionChannelList : null;

            if (LoginChannelList?.Count == 0 && (consumerSession == null || !consumerSession.CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK)))
                return;

            m_OmmBaseImpl.EventReceived();
            ReactorChannel reactorChannel;

            RefreshMsg? refreshMsg = loginCallbackClient.m_RefreshMsg;

            if (refreshMsg == null)
                return;

            if (m_OmmBaseImpl.ConsumerSession == null)
            {
                reactorChannel = LoginChannelList![0].ReactorChannel!;

                refreshMsg.Decode(loginCallbackClient.GetRefreshMsg()!, reactorChannel.MajorVersion,
                    reactorChannel.MinorVersion, null!);
            }
            else
            {
                reactorChannel = sessionChannelList![0].ReactorChannel!;
            }

            loginCallbackClient.EventImpl.Item = this;
            loginCallbackClient.EventImpl.ReactorChannel = reactorChannel;

            loginCallbackClient.NotifyOnAllMsg(refreshMsg);
            loginCallbackClient.NotifyOnRefreshMsg();

            if(refreshMsg.State().StreamState != OmmState.StreamStates.OPEN)
            {
                loginCallbackClient.LoginItems?.Remove(this);

                Remove();
            }
        }

        public override bool Modify(RequestMsg reqMsg)
        {
            CodecReturnCode ret;
            if((ret = m_OmmBaseImpl.LoginCallbackClient!.OverlayLoginRequest(reqMsg.m_rsslMsg))
                != CodecReturnCode.SUCCESS)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    strBuilder.Append("Internal error: Error caching login reissue.");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                    strBuilder.Clear();
                }

                strBuilder.Append($"Failed cache login reissue. Reason: {ret.GetAsString()}");

                m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);

                return false;
            }

            bool submitRet;

            if (m_OmmBaseImpl.ConsumerSession == null)
            {
                submitRet = Submit(m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest);
            }
            else
            {
                submitRet = Submit(m_OmmBaseImpl.ConsumerSession, m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest);
            }

            /* Unset the pause all and no refresh flags on the stored request. */
            m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
            m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest.Flags &= ~LoginRequestFlags.NO_REFRESH;

            return submitRet;
        }

        public override bool Close()
        {
            Remove();

            m_OmmBaseImpl.LoginCallbackClient!.LoginItems?.Remove(this);
            return true;
        }

        ReactorReturnCode  SubmitLoginRequest(ReactorChannel reactorChannel, ReactorSubmitOptions submitOptions, LoginRequest loginRequest)
        {
            ReactorReturnCode ret;

            if (ReactorReturnCode.SUCCESS > (ret = reactorChannel.Submit(loginRequest, submitOptions,
                    out var errorInfo)))
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    strBuilder.Append("Internal error: ReactorChannel.Submit() failed in LoginItem.Submit(RequestMsg)")
                    .AppendLine($"\tReactorChannel {reactorChannel.Channel?.GetHashCode()}")
                    .AppendLine($"\tError Id {errorInfo?.Error.ErrorId}")
                    .AppendLine($"\tInternal SysError {errorInfo?.Error.SysError}")
                    .AppendLine($"\tError Location {errorInfo?.Location}")
                    .Append($"\tError Text {errorInfo?.Error.Text}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());

                    strBuilder.Clear();
                }

                strBuilder.Append($"Failed to open or modify item request. Reason: {ret}.")
                .Append($" Error text: {errorInfo?.Error.Text}");

                m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), (int)ret);

                return ret;
            }

            return ret;
        }

        bool Submit(ConsumerSession<T> consumerSession, LoginRequest rdmRequestMsg)
        {
            ReactorSubmitOptions rsslSubmitOptions = m_OmmBaseImpl.GetSubmitOptions();
            rdmRequestMsg.StreamId = StreamId;
            
            if (!rdmRequestMsg.HasUserNameType)
            {
                rdmRequestMsg.HasUserNameType = true;
                rdmRequestMsg.UserNameType = Eta.Rdm.Login.UserIdTypes.NAME;
            }

            ReactorReturnCode ret;
            foreach (var entry in consumerSession.SessionChannelList)
            {
                rsslSubmitOptions.ServiceName = null;
                rsslSubmitOptions.RequestMsgOptions.Clear();

                /* Ensure that the ReactorChannel is ready to submit the message */
                if (entry.ReactorChannel!.State == ReactorChannelState.UP || entry.ReactorChannel!.State == ReactorChannelState.READY)
                {
                    ret = SubmitLoginRequest(entry.ReactorChannel, rsslSubmitOptions, rdmRequestMsg);
                    if (ReactorReturnCode.SUCCESS > ret)
                    {
                        return false;
                    }
                }
                ret = 0;
            }

            return true;
        }

        bool Submit(LoginRequest rdmRequestMsg)
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            rdmRequestMsg.StreamId = StreamId;

            if(!rdmRequestMsg.HasUserNameType)
            {
                rdmRequestMsg.HasUserNameType = true;
                rdmRequestMsg.UserNameType = Eta.Rdm.Login.UserIdTypes.NAME;
            }

            ChannelInfo? channelInfo = m_OmmBaseImpl.LoginCallbackClient!.ActiveChannelInfo();

            ReactorReturnCode ret;
            if(channelInfo != null && channelInfo.ReactorChannel != null)
            {
                submitOptions.ServiceName = null;
                submitOptions.RequestMsgOptions.UserSpecObj = this;

                ret = SubmitLoginRequest(channelInfo.ReactorChannel, submitOptions, rdmRequestMsg);

                if (ReactorReturnCode.SUCCESS > ret)
                {
                    return false;
                }
            }
            else
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    message.AppendLine("Internal error: ReactorChannel.Submit() failed in LoginItem.Submit(RequestMsg).")
                    .AppendLine($"\tReactorChannel is not avaliable");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append($"Failed to submit CurrentLoginRequest. Reason: ReactorChannel is not avaliable");

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);

                return false;
            }

            return true;
        }

        public override bool Submit(PostMsg postMsg)
        {
            var consumerSession = m_OmmBaseImpl.ConsumerSession;

            if (consumerSession == null)
            {
                return base.Submit(postMsg);
            }
            else
            {
                bool supportPosting = true;
                if(consumerSession.LoginRefresh()!.LoginRefresh!.HasFeatures && consumerSession.LoginRefresh()!.LoginRefresh!.SupportedFeatures.HasSupportPost)
                {
                    if(consumerSession.LoginRefresh()!.LoginRefresh!.SupportedFeatures.SupportOMMPost == 0)
                    {
                        supportPosting = false;
                    }
                }
                else
                {
                    supportPosting = false;
                }

                if(!supportPosting)
                {
                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                    temp.Append("Invalid attempt to submit PostMsg while posting not supported by provider.");

                    if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());
                    }

                    m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

                    return false;
                }

                return Submit(consumerSession, postMsg.m_rsslMsg, postMsg.HasServiceName ? postMsg.ServiceName() : null);
            }
        }

        bool Submit(ConsumerSession<T> consumerSession, IPostMsg rsslPostMsg, string? serviceName)
        {
            ReactorSubmitOptions reactorSubmitOptions = m_OmmBaseImpl.GetSubmitOptions();
            ReactorReturnCode ret;

            rsslPostMsg.StreamId = StreamId;

            foreach (var entry in consumerSession.SessionChannelList)
            {
                /* Validate whether the service name is valid for SessionChannelInfo before submitting it. */
                bool result = consumerSession.ValidateServiceName(entry, rsslPostMsg, serviceName);

                if(result == false)
                {
                    /* The PostMsg is dropped from this session channel */
                    continue;
                }

                reactorSubmitOptions.ServiceName = serviceName;
                reactorSubmitOptions.RequestMsgOptions.Clear();

                /* Ensure that the ReactorChannel is ready to submit the message */
                if(entry.ReactorChannel!.State == ReactorChannelState.UP || entry.ReactorChannel.State == ReactorChannelState.READY)
                {
                    if(ReactorReturnCode.SUCCESS > (ret = entry.ReactorChannel.Submit(rsslPostMsg, reactorSubmitOptions, out var errorInfo)))
                    {
                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                        if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            temp.Append($"Internal error: ReactorChannel.Submit() failed in LoginItem.Submit(IPostMsg) ")
                            .AppendLine($"\tChannel {errorInfo?.Error.Channel?.GetHashCode()}")
                            .AppendLine($"\tError Id {errorInfo?.Error.ErrorId}")
                            .AppendLine($"\tInternal sysError {errorInfo?.Error.SysError}")
                            .AppendLine($"\tError Location {errorInfo?.Location}")
                            .Append($"\tError Text {errorInfo?.Error.Text}");

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

                            temp.Clear();
                        }

                        temp.Append("Failed to submit PostMsg on item stream. Reason: ")
                        .Append(ret)
                        .Append(". Error text: ")
                        .Append(errorInfo?.Error.Text);

                        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), (int)ret);

                        return false;
                    }
                }
            }

            return true;
        }

        public override bool Submit(GenericMsg genericMsg)
        {
            var consumerSession = m_OmmBaseImpl.ConsumerSession;

            if (consumerSession == null)
            {
                return base.Submit(genericMsg);
            }
            else
            {
                return Submit(consumerSession, genericMsg.m_rsslMsg);
            }
        }

        bool Submit(ConsumerSession<T> consumerSession, IGenericMsg rsslGenericMsg)
        {
            ReactorSubmitOptions reactorSubmitOptions = m_OmmBaseImpl.GetSubmitOptions();
            ReactorReturnCode ret;
            rsslGenericMsg.StreamId = StreamId;
            bool hasServiceId = rsslGenericMsg.CheckHasMsgKey() && rsslGenericMsg.MsgKey.CheckHasServiceId();
            int originalServiceId = 0; /* Keeps the original service Id in order to check it with every SessionChannelInfo */

            if(hasServiceId)
            {
                originalServiceId = rsslGenericMsg.MsgKey.ServiceId;
            }

            foreach (var entry in consumerSession.SessionChannelList)
            {
                /* Translate from generated service Id to the actual service Id */
                consumerSession.CheckServiceId(entry, rsslGenericMsg);

                reactorSubmitOptions.ServiceName = null;
                reactorSubmitOptions.RequestMsgOptions.Clear();

                /* Ensure that the ReactorChannel is ready to submit the message */
                if (entry.ReactorChannel!.State == ReactorChannelState.UP || entry.ReactorChannel.State == ReactorChannelState.READY)
                {
                    if(ReactorReturnCode.SUCCESS > (ret = entry.ReactorChannel.Submit(rsslGenericMsg, reactorSubmitOptions, out var errorInfo)))
                    {
                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                        if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            temp.Append($"Internal error: ReactorChannel.Submit() failed in LoginItem.Submit(IGenericMsg) ")
                            .AppendLine($"\tChannel {errorInfo?.Error.Channel?.GetHashCode()}")
                            .AppendLine($"\tError Id {errorInfo?.Error.ErrorId}")
                            .AppendLine($"\tInternal sysError {errorInfo?.Error.SysError}")
                            .AppendLine($"\tError Location {errorInfo?.Location}")
                            .Append($"\tError Text {errorInfo?.Error.Text}");

                            m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, temp.ToString());

                            temp.Clear();
                        }

                        temp.Append("Failed to submit GenericMsg on item stream. Reason: ")
                        .Append(ret)
                        .Append(". Error text: ")
                        .Append(errorInfo?.Error.Text);

                        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), (int)ret);

                        return false;
                    }
                }

                // Restore the original service Id in order to check with another SessionChannelInfo
                if(hasServiceId)
                {
                    rsslGenericMsg.MsgKey.ServiceId = originalServiceId;
                }
            }

            return true;
        }
    }

    internal class LoginCallbackClientConsumer : LoginCallbackClient<IOmmConsumerClient>
    {
        public LoginCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl) : base(baseImpl)
        {
            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)baseImpl;

            ommConsumerImpl.Consumer.m_OmmConsumerImpl = ommConsumerImpl;

            EventImpl.SetOmmConsumer(ommConsumerImpl.Consumer);

            NotifyOnAllMsg = NotifyOnAllMsgImpl;
            NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
            NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
            NotifyOnGenericMsg = NotifyOnGenericMsgImpl;
            NotifyOnAckMsg = NotifyOnAckMsgImpl;
        }

        public void NotifyOnAllMsgImpl(Msg msg)
        {
            EventImpl.Item!.Client!.OnAllMsg(msg, EventImpl);
        }

        public void NotifyOnRefreshMsgImpl()
        {
            EventImpl.Item!.Client!.OnRefreshMsg(m_RefreshMsg!, EventImpl);
        }

        public void NotifyOnStatusMsgImpl()
        {
            EventImpl.Item!.Client!.OnStatusMsg(m_StatusMsg!, EventImpl);
        }

        public void NotifyOnGenericMsgImpl()
        {
            EventImpl.Item!.Client!.OnGenericMsg(m_GenericMsg!, EventImpl);
        }

        public void NotifyOnAckMsgImpl()
        {
            EventImpl.Item!.Client!.OnAckMsg(m_AckMsg!, EventImpl);
        }
    }

    internal class LoginCallbackClientProvider : LoginCallbackClient<IOmmProviderClient>
    {
        public LoginCallbackClientProvider(OmmBaseImpl<IOmmProviderClient> baseImpl) : base(baseImpl)
        {
            OmmNiProviderImpl ommNiProviderImpl = (OmmNiProviderImpl)baseImpl;

            ommNiProviderImpl.Provider.m_OmmProviderImpl = ommNiProviderImpl;

            EventImpl.SetOmmProvider(ommNiProviderImpl.Provider);

            NotifyOnAllMsg = NotifyOnAllMsgImpl;
            NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
            NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
            NotifyOnGenericMsg = NotifyOnGenericMsgImpl;
        }

        public void NotifyOnAllMsgImpl(Msg msg)
        {
            EventImpl.Item!.Client!.OnAllMsg(msg, EventImpl);
        }

        public void NotifyOnRefreshMsgImpl()
        {
            EventImpl.Item!.Client!.OnRefreshMsg(m_RefreshMsg!, EventImpl);
        }

        public void NotifyOnStatusMsgImpl()
        {
            EventImpl.Item!.Client!.OnStatusMsg(m_StatusMsg!, EventImpl);
        }

        public void NotifyOnGenericMsgImpl()
        {
            EventImpl.Item!.Client!.OnGenericMsg(m_GenericMsg!, EventImpl);
        }
    }

    internal class LoginCallbackClient<T> : CallbackClient<T>, IRDMLoginMsgCallback
    {
        private static readonly string CLIENT_NAME = "LoginCallbackClient";
        private const int REFRESH_MSG_SIZE = 1024;

        internal List<ChannelInfo> m_LoginChannelList = new();
        private List<LoginItem<T>>? m_LoginItemList;
        private Buffer m_EncodedBuffer = new();
        private MonitorWriteLocker m_LoginItemLock = new MonitorWriteLocker(new object());
        private bool m_NotifyChannelDownReconnecting;
        private DecodeIterator m_DecIter = new();
        private EncodeIterator m_EncIter = new();
        private Buffer m_TempBuffer = new();
        private ByteBuffer m_TempByteBuffer = new (8192);
        private ByteBuffer m_TempUserNameByteBuffer = new(8192);
        private Eta.Codec.Msg m_TempMsg = new Eta.Codec.Msg();
        private LoginRequest m_TempLoginReq = new LoginRequest();
        protected OmmBaseImpl<T> m_OmmBaseImpl;
        private LoginRefresh? m_LoginRefresh;
        private State? m_EtaState;

        internal List<LoginItem<T>>? LoginItems { get => m_LoginItemList; }

        public LoginRequest CurrentLoginRequest { get; internal set; }

        public string LoginFailureMsg { get; private set; } = string.Empty;

        public LoginCallbackClient(OmmBaseImpl<T> baseImpl) : base(baseImpl, CLIENT_NAME)
        {
            m_OmmBaseImpl = baseImpl;
            m_NotifyChannelDownReconnecting = false;

            CurrentLoginRequest = m_OmmBaseImpl.OmmConfigBaseImpl.AdminLoginRequest;

            if (!m_OmmBaseImpl.OmmConfigBaseImpl.SetAdminLoginRequest)
            {
                /* Override the default login request from OmmConsumerConfig */
                if (!string.IsNullOrEmpty(m_OmmBaseImpl.OmmConfigBaseImpl.UserName))
                {
                    if (!m_OmmBaseImpl.OmmConfigBaseImpl.UserName.Equals(CurrentLoginRequest.UserName.ToString()))
                    {
                        CurrentLoginRequest.UserName.Data(m_OmmBaseImpl.OmmConfigBaseImpl.UserName);
                    }
                }

                if (!string.IsNullOrEmpty(m_OmmBaseImpl.OmmConfigBaseImpl.Password) && CurrentLoginRequest.HasPassword)
                {
                    if (!m_OmmBaseImpl.OmmConfigBaseImpl.Password.Equals(CurrentLoginRequest.Password.ToString()))
                    {
                        CurrentLoginRequest.Password.Data(m_OmmBaseImpl.OmmConfigBaseImpl.Password);
                    }
                }

                if (!string.IsNullOrEmpty(m_OmmBaseImpl.OmmConfigBaseImpl.ApplicationId))
                {
                    if (CurrentLoginRequest.HasAttrib && CurrentLoginRequest.LoginAttrib.HasApplicationId)
                    {
                        if (!m_OmmBaseImpl.OmmConfigBaseImpl.ApplicationId.Equals(CurrentLoginRequest.LoginAttrib.ApplicationId.ToString()))
                        {
                            CurrentLoginRequest.LoginAttrib.ApplicationId.Data(m_OmmBaseImpl.OmmConfigBaseImpl.ApplicationId);
                        }
                    }
                }

                if (!string.IsNullOrEmpty(m_OmmBaseImpl.OmmConfigBaseImpl.ApplicationName))
                {
                    if (CurrentLoginRequest.HasAttrib && CurrentLoginRequest.LoginAttrib.HasApplicationName)
                    {
                        if (!m_OmmBaseImpl.OmmConfigBaseImpl.ApplicationName.Equals(CurrentLoginRequest.LoginAttrib.ApplicationName.ToString()))
                        {
                            CurrentLoginRequest.LoginAttrib.ApplicationName.Data(m_OmmBaseImpl.OmmConfigBaseImpl.ApplicationName);
                        }
                    }
                }
            }
        }

        internal void RemoveSessionChannelInfo(SessionChannelInfo<IOmmConsumerClient>? sessionChannelInfo)
        {
            sessionChannelInfo?.ConsumerSession.SessionChannelList.Remove(sessionChannelInfo);
        }

        public LoginRefresh LoginRefresh
        {
            get
            {
                if(m_LoginRefresh == null)
                {
                    m_LoginRefresh = new LoginRefresh();
                }

                return m_LoginRefresh;
            }
        }

        internal void Initialize()
        {
            m_TempBuffer.Data(m_TempByteBuffer);

            if(m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                strBuilder.AppendLine($"RDMLogin request message was populated with this info: {CurrentLoginRequest.ToString()}");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
            }
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            IMsg? msg = loginEvent.Msg;
            LoginMsg? loginMsg = loginEvent.LoginMsg;
            ReactorChannel? reactorChannel = loginEvent.ReactorChannel;
            ChannelInfo? channelInfo = (ChannelInfo?)reactorChannel?.UserSpecObj;
            SessionChannelInfo<IOmmConsumerClient>? sessionChannelInfo = channelInfo?.SessionChannelInfo;
            ConsumerSession<IOmmConsumerClient>? consumerSession = sessionChannelInfo?.ConsumerSession;

            m_OmmBaseImpl.EventReceived();

            if(channelInfo == null)
            {
                if(consumerSession != null)
                {
                    m_OmmBaseImpl.CloseSessionChannel(sessionChannelInfo);
                }
                else
                {
                    m_OmmBaseImpl.CloseReactorChannel(reactorChannel);
                }

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                    strBuilder.AppendLine("Received a ReactorChannel without UserSpecObj")
                        .AppendLine($"\tReactor {reactorChannel?.GetHashCode()}")
                        .AppendLine($"\tChannel {reactorChannel?.Channel?.GetHashCode()}")
                        .AppendLine($"\tError Id {loginEvent.ReactorErrorInfo.Error.ErrorId}")
                        .AppendLine($"\tInternal SysError {loginEvent.ReactorErrorInfo.Error.SysError}")
                        .AppendLine($"\tError Location {loginEvent.ReactorErrorInfo}")
                        .AppendLine($"\tError Text {loginEvent.ReactorErrorInfo}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            if(loginMsg == null)
            {
                if (consumerSession != null)
                {
                    m_OmmBaseImpl.CloseSessionChannel(sessionChannelInfo);
                }
                else
                {
                    m_OmmBaseImpl.CloseReactorChannel(reactorChannel);
                }

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                    strBuilder.AppendLine("Received an event without RDMLogin message")
                        .AppendLine($"\tReactor {reactorChannel?.GetHashCode()}")
                        .AppendLine($"\tChannel {reactorChannel?.Channel?.GetHashCode()}")
                        .AppendLine($"\tError Id {loginEvent.ReactorErrorInfo.Error.ErrorId}")
                        .AppendLine($"\tInternal SysError {loginEvent.ReactorErrorInfo.Error.SysError}")
                        .AppendLine($"\tError Location {loginEvent.ReactorErrorInfo}")
                        .AppendLine($"\tError Text {loginEvent.ReactorErrorInfo}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            switch(loginMsg.LoginMsgType)
            {
                case LoginMsgType.REFRESH:
                    {
                        bool notifyRefreshMsg = true;

                        if(codecRefreshMsg == null)
                        {
                            codecRefreshMsg = new Eta.Codec.Msg();
                        }
                        else
                        {
                            codecRefreshMsg.Clear();
                        }

                        codecRefreshMsg.MsgClass = MsgClasses.REFRESH;
                        msg?.Copy(codecRefreshMsg, CopyMsgFlags.ALL_FLAGS);

                        LoginRefresh loginRefresh;

                        if (consumerSession != null)
                        {
                            loginRefresh = sessionChannelInfo!.LoginRefresh();

                            if (!consumerSession.SessionChannelList.Contains(sessionChannelInfo))
                            {
                                consumerSession.SessionChannelList.Add(sessionChannelInfo);
                            }

                            sessionChannelInfo.ReceivedLoginRefresh = true;
                        }
                        else
                        {
                            loginRefresh = LoginRefresh;

                            if (!m_LoginChannelList.Contains(channelInfo))
                            {
                                RemoveChannelInfo(loginEvent.ReactorChannel);
                                m_LoginChannelList.Add(channelInfo);
                            }
                        }

                        loginRefresh.Clear();
                        loginMsg.LoginRefresh!.Copy(loginRefresh);
                        State state = loginMsg.LoginRefresh.State;

                        bool closeChannel = false;

                        if (state.StreamState() != StreamStates.OPEN)
                        {
                            closeChannel = true;

                            if(consumerSession != null)
                            {
                                sessionChannelInfo!.State = OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN;

                                consumerSession.IncreaseNumOfLoginClose();

                                sessionChannelInfo!.LoginRefresh().State.StreamState(state.StreamState());
                                sessionChannelInfo.LoginRefresh().State.DataState(state.DataState());

                                if (consumerSession.CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN))
                                {
                                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN);
                                }
                            }
                            else
                            {
                                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN);
                            }

                            StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                            strBuilder.AppendLine($"RDMLogin stream was closed with refresh message");
                            LoginMsgToString(strBuilder, loginMsg, LoginMsgType.REFRESH);
                            strBuilder.Append(state.ToString());

                            LoginFailureMsg = strBuilder.ToString();

                            if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, LoginFailureMsg);
                            }
                        }
                        else if (state.DataState() == DataStates.SUSPECT)
                        {
                            if(m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                            {
                                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine("RDMLogin stream state was changed to suspect with refresh message");
                                LoginMsgToString(strBuilder, loginMsg, loginMsg.LoginMsgType);
                                strBuilder.AppendLine().Append(state.ToString());

                                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, strBuilder.ToString());
                            }

                            if (sessionChannelInfo != null)
                            {
                                if (sessionChannelInfo.State >= OmmBaseImpl<T>.OmmImplState.CHANNEL_UP)
                                    sessionChannelInfo.State = OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT;

                                sessionChannelInfo.LoginRefresh().State.StreamState(state.StreamState());
                                sessionChannelInfo.LoginRefresh().State.DataState(state.DataState());
                            }
                            else
                            {
                                if (m_OmmBaseImpl.ImplState >= OmmBaseImpl<T>.OmmImplState.CHANNEL_UP)
                                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
                            }
                        }
                        else
                        {
                            if (consumerSession != null)
                            {
                                sessionChannelInfo!.State = OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK;

                                consumerSession.IncreaseNumOfLoginOk();

                                sessionChannelInfo.LoginRefresh().State.StreamState(state.StreamState());
                                sessionChannelInfo.LoginRefresh().State.DataState(state.DataState());

                                if (consumerSession.CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK) || consumerSession.SendInitialLoginRefresh)
                                {
                                    consumerSession.AggregateLoginResponse();

                                    /* Swap to send with the aggregated login refresh */
                                    loginMsg = (LoginMsg)consumerSession.LoginRefresh();
                                    msg = null;
                                    consumerSession.SendInitialLoginRefresh = true;
                                }
                                else
                                {
                                    /* Wait until EMA's receives login refresh message from all channels. */
                                    notifyRefreshMsg = false;
                                }
                            }

                            if (notifyRefreshMsg)
                            {
                                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK);
                                m_OmmBaseImpl.SetActiveReactorChannel(channelInfo);
                                m_OmmBaseImpl.ReLoadDirectory();

                                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                                {
                                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                    strBuilder.AppendLine("RDMLogin stream was open with refresh message")
                                        .AppendLine(loginMsg.ToString());

                                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                                }
                            }
                        }

                        if (notifyRefreshMsg)
                        {
                            ProcessRefreshMsg(msg, reactorChannel!, loginMsg);
                        }

                        if (closeChannel)
                        {
                            if (sessionChannelInfo != null)
                            {
                                m_OmmBaseImpl.CloseSessionChannel(sessionChannelInfo);
                            }
                            else
                            {
                                m_OmmBaseImpl.UnsetActiveRsslReactorChannel(channelInfo);
                                m_OmmBaseImpl.CloseReactorChannel(reactorChannel);
                            }
                        }

                        break;
                    }
                case LoginMsgType.STATUS:
                    {
                        bool closeChannel = false;
                        bool notifyStatusMsg = true;

                        LoginStatus loginStatus = loginMsg.LoginStatus!;

                        if(loginStatus.HasState)
                        {
                            State state = loginStatus.State;

                            if(state.StreamState() != StreamStates.OPEN)
                            {
                                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                                strBuilder.AppendLine($"RDMLogin stream was closed with status message");
                                LoginMsgToString(strBuilder, loginMsg, LoginMsgType.STATUS);
                                strBuilder.Append(state.ToString());

                                LoginFailureMsg = strBuilder.ToString();

                                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, LoginFailureMsg);
                                }

                                closeChannel = true;

                                if (consumerSession != null)
                                {
                                    sessionChannelInfo!.State = OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN;

                                    consumerSession.IncreaseNumOfLoginClose();

                                    sessionChannelInfo!.LoginRefresh().State.StreamState(state.StreamState());
                                    sessionChannelInfo.LoginRefresh().State.DataState(state.DataState());

                                    if (consumerSession.CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN))
                                    {
                                        m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN);
                                    }
                                    else
                                    {
                                        notifyStatusMsg = false;

                                        int streamState = loginStatus.State.StreamState();
                                        loginStatus.State.StreamState(StreamStates.OPEN);
                                        msg = null;

                                        ProcessStatusMsg(msg, reactorChannel!, loginMsg);
                                        loginStatus.State.StreamState(streamState);
                                    }
                                }
                                else
                                {
                                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN);
                                }
                            }
                            else if (state.DataState() == DataStates.SUSPECT)
                            {
                                if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                                {
                                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                    strBuilder.AppendLine("RDMLogin stream state was changed to suspect with status message");
                                    LoginMsgToString(strBuilder, loginMsg, loginMsg.LoginMsgType);
                                    strBuilder.AppendLine().Append(state.ToString());

                                    m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, strBuilder.ToString());
                                }

                                if (consumerSession != null)
                                {
                                    sessionChannelInfo!.LoginRefresh().State.StreamState(state.StreamState());
                                    sessionChannelInfo.LoginRefresh().State.DataState(state.DataState());

                                    if (sessionChannelInfo.State >= OmmBaseImpl<T>.OmmImplState.CHANNEL_UP)
                                    {
                                        sessionChannelInfo.State = OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT;
                                    }

                                    if (consumerSession.CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT))
                                    {
                                        m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
                                    }
                                    else
                                    {
                                        notifyStatusMsg = false;
                                    }
                                }
                                else
                                {
                                    if (m_OmmBaseImpl.ImplState >= OmmBaseImpl<T>.OmmImplState.CHANNEL_UP)
                                        m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
                                }
                            }
                            else
                            {
                                m_OmmBaseImpl.SetActiveReactorChannel(channelInfo);

                                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                                {
                                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                    strBuilder.AppendLine("RDMLogin stream was open with status message")
                                        .AppendLine(loginMsg.ToString());

                                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                                }

                                if (consumerSession != null)
                                {
                                    sessionChannelInfo!.State = OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK;

                                    consumerSession.IncreaseNumOfLoginOk();

                                    sessionChannelInfo.LoginRefresh().State.StreamState(state.StreamState());
                                    sessionChannelInfo.LoginRefresh().State.DataState(state.DataState());

                                    if (consumerSession.CheckAllSessionChannelHasState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK) || consumerSession.SendInitialLoginRefresh)
                                    {
                                        m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK);

                                        consumerSession.AggregateLoginResponse();
                                        msg = null;
                                        ProcessRefreshMsg(msg, reactorChannel!, loginMsg);
                                        consumerSession.SendInitialLoginRefresh = true;
                                        notifyStatusMsg = false;
                                    }
                                }
                                else
                                {
                                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK);
                                }
                            }
                        }
                        else
                        {
                            if (m_OmmBaseImpl.LoggerClient.IsWarnEnabled)
                            {
                                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine("Received RDMLogin status message without the state");
                                strBuilder.AppendLine().Append(loginMsg.ToString());

                                m_OmmBaseImpl.LoggerClient.Warn(CLIENT_NAME, strBuilder.ToString());
                            }
                        }

                        if (notifyStatusMsg)
                        {
                            ProcessStatusMsg(msg!, reactorChannel!, loginMsg);
                        }

                        if (closeChannel)
                        {
                            if (sessionChannelInfo != null)
                            {
                                m_OmmBaseImpl.CloseSessionChannel(sessionChannelInfo);
                            }
                            else
                            {
                                m_OmmBaseImpl.UnsetActiveRsslReactorChannel(channelInfo);
                                m_OmmBaseImpl.CloseReactorChannel(reactorChannel);
                            }
                        }

                        break;
                    }
                default:
                    {
                        if(msg != null && msg.MsgClass == MsgClasses.GENERIC)
                        {
                            ProcessGenericMsg(msg!, reactorChannel!, loginEvent);
                            break;
                        }

                        if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        {
                            StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                            strBuilder.AppendLine("Received unknown RDMLogin message type")
                                .Append($"Message type value {loginMsg.LoginMsgType}");
                        }

                        break;
                    }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        internal int SendLoginClose()
        {
            ICloseMsg closeMsg = CloseMsg();

            closeMsg.StreamId = 1;
            closeMsg.ContainerType = DataTypes.NO_DATA;
            closeMsg.DomainType = (int)Eta.Rdm.DomainType.LOGIN;

            ReactorSubmitOptions rsslSubmitOptions = m_OmmBaseImpl.GetSubmitOptions();

            if (m_OmmBaseImpl.ConsumerSession != null)
            {
                List<SessionChannelInfo<IOmmConsumerClient>> sessionChannelList = m_OmmBaseImpl.ConsumerSession.SessionChannelList;

                foreach (SessionChannelInfo<IOmmConsumerClient> entry in sessionChannelList)
                {
                    entry.ReactorChannel?.Submit(closeMsg, rsslSubmitOptions, out _);
                }

                return sessionChannelList.Count;
            }
            else
            {
                foreach (ChannelInfo entry in m_LoginChannelList)
                {
                    rsslSubmitOptions.Clear();
                    rsslSubmitOptions.ApplyClientChannelConfig(entry.ChannelConfig);
                    entry.ReactorChannel?.Submit((Eta.Codec.Msg)closeMsg, rsslSubmitOptions, out _);
                }

                return m_LoginChannelList.Count;
            }
        }

        internal SingleItem<T> CreateLoginItem(RequestMsg reqMsg, T client, object? closure)
        {
            if (m_LoginItemList == null)
            {
                m_LoginItemList = new();
            }

            LoginItem<T>? item = (LoginItem<T>?)m_OmmBaseImpl.GetEmaObjManager().m_loginItemPool.Poll();
            if (item == null)
            {
                item = new LoginItem<T>(m_OmmBaseImpl, client, closure);
                m_OmmBaseImpl.GetEmaObjManager().m_loginItemPool.UpdatePool(item);
            }
            else
            {
                item.ResetLoginItem(m_OmmBaseImpl, client, closure);
            }

            item.LoginChannelList = m_LoginChannelList;
            m_LoginItemList.Add(item);

            /* Do not give a refresh msg to the user if one is not present */
            if (m_RefreshMsg != null)
            {
                if (m_OmmBaseImpl.ConsumerSession != null)
                {
                    if (m_OmmBaseImpl.ConsumerSession.SendInitialLoginRefresh)
                    {
                        m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(10, item);
                    }
                }
                else
                {
                    m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(10, item);
                }
            }

            return item;
        }

        internal ChannelInfo? ActiveChannelInfo()
        {
            int numOfChannel = m_LoginChannelList.Count;
            for (int idx = 0; idx < numOfChannel; ++idx)
            {
                ReactorChannelState state = m_LoginChannelList[idx].ReactorChannel!.State;
                if (state == ReactorChannelState.READY || state == ReactorChannelState.UP)
                    return m_LoginChannelList[idx];
            }

            return numOfChannel > 0 ? m_LoginChannelList[numOfChannel - 1] : null;
        }

        internal void RemoveChannelInfo(ReactorChannel? reactorChannel)
        {
            for (int index = 0; index < m_LoginChannelList.Count; index++)
            {
                ChannelInfo channelInfo = m_LoginChannelList[index];
                if (channelInfo.ReactorChannel == reactorChannel)
                {
                    m_LoginChannelList.RemoveAt(index);
                    break;
                }
            }
        }

        internal ReactorCallbackReturnCode ProcessRefreshMsg(IMsg? msg, ReactorChannel reactorChannel, LoginMsg loginMsg)
        {
            if (m_RefreshMsg == null)
                m_RefreshMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();

            if (msg != null)
            {
                m_RefreshMsg.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null!);
            }
            else
            {
                if (ConvertRdmLoginToRsslBuffer(reactorChannel, loginMsg) != ReactorCallbackReturnCode.SUCCESS)
                    return ReactorCallbackReturnCode.SUCCESS;


                m_RefreshMsg.DecodeMsg(reactorChannel.MajorVersion, reactorChannel.MinorVersion, m_EncodedBuffer, null, null);
            }

            if (m_LoginItemList == null)
                return ReactorCallbackReturnCode.SUCCESS;

            m_LoginItemLock.Enter();

            /* Special handing for the request routing feature */
            if (m_OmmBaseImpl.ConsumerSession != null && m_OmmBaseImpl.ConsumerSession.SendInitialLoginRefresh)
            {
                /* Checks whether the user is still login to a server */
                if (m_OmmBaseImpl.ConsumerSession.CheckUserStillLogin())
                {
                    m_RefreshMsg.SetDataState(DataStates.OK);
                }
                else
                {
                    m_RefreshMsg.SetDataState(DataStates.SUSPECT);
                }
            }

            try
            {
                int itemSize = m_LoginItemList.Count;
                for (int idx = 0; idx < itemSize; ++idx)
                {
                    EventImpl.Item = m_LoginItemList[idx];
                    EventImpl.ReactorChannel = reactorChannel;

                    NotifyOnAllMsg(m_RefreshMsg);
                    NotifyOnRefreshMsg();
                }

                if (loginMsg.LoginRefresh!.State.StreamState() != StreamStates.OPEN)
                {
                    for (int idx = 0; idx < itemSize; ++idx)
                        m_LoginItemList[idx].Remove();

                    m_LoginItemList.Clear();
                }
            }
            finally
            {
                m_LoginItemLock.Exit();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        ReactorCallbackReturnCode ProcessStatusMsg(IMsg? msg, ReactorChannel reactorChannel, LoginMsg loginMsg)
        {
            if (m_LoginItemList == null)
                return ReactorCallbackReturnCode.SUCCESS;

            if (m_StatusMsg == null)
                m_StatusMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();

            if (msg != null)
            {
                m_StatusMsg.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null!);
            }
            else
            {
                if (ConvertRdmLoginToRsslBuffer(reactorChannel, loginMsg) != ReactorCallbackReturnCode.SUCCESS)
                    return ReactorCallbackReturnCode.SUCCESS;

                m_StatusMsg.DecodeMsg(reactorChannel.MajorVersion, reactorChannel.MinorVersion, m_EncodedBuffer, null, null);
            }

            m_LoginItemLock.Enter();

            /* Special handing for the request routing feature */
            if(m_OmmBaseImpl.ConsumerSession != null && m_OmmBaseImpl.ConsumerSession.SendInitialLoginRefresh)
            {
                /* Checks whether the user is still login to a server */
                if (m_StatusMsg.HasState && m_OmmBaseImpl.ConsumerSession.CheckUserStillLogin())
                {
                    m_StatusMsg.SetDataState(DataStates.OK);
                }
                else
                {
                    m_StatusMsg.SetDataState(DataStates.SUSPECT);
                }
            }

            try
            {
                int itemSize = m_LoginItemList.Count;
                for (int idx = 0; idx < itemSize; ++idx)
                {
                    EventImpl.Item = m_LoginItemList[idx];
                    EventImpl.ReactorChannel = reactorChannel;

                    NotifyOnAllMsg(m_StatusMsg);
                    NotifyOnStatusMsg();
                }

                if (loginMsg.LoginStatus!.HasState && loginMsg.LoginStatus.State.StreamState() != StreamStates.OPEN)
                {
                    for (int idx = 0; idx < itemSize; ++idx)
                        m_LoginItemList[idx].Remove();

                    m_LoginItemList.Clear();
                }
            }
            finally
            {
                m_LoginItemLock.Exit();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        ReactorCallbackReturnCode ProcessGenericMsg(IMsg msg, ReactorChannel reactorChannel, RDMLoginMsgEvent loginEvent)
        {
            if (m_LoginItemList == null)
                return ReactorCallbackReturnCode.SUCCESS;

            if (m_GenericMsg == null)
                m_GenericMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmGenericMsg();

            m_GenericMsg.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion,((ChannelInfo)reactorChannel.UserSpecObj!).DataDictionary!);

            m_LoginItemLock.Enter();

            try
            {
                for (int idx = 0; idx < m_LoginItemList.Count; ++idx)
                {
                    EventImpl.Item = m_LoginItemList[idx];
                    EventImpl.ReactorChannel = reactorChannel;

                    NotifyOnAllMsg(m_GenericMsg);
                    NotifyOnGenericMsg();
                }
            }
            finally
            { 
                m_LoginItemLock.Exit();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        ReactorCallbackReturnCode ConvertRdmLoginToRsslBuffer(ReactorChannel reactorChannel, LoginMsg loginMsg)
        {
            if (m_EncodedBuffer.Data() == null)
            {
                m_EncodedBuffer.Data( new ByteBuffer(REFRESH_MSG_SIZE));
            }
            else
            {
                ByteBuffer byteBuf = m_EncodedBuffer.Data();
                byteBuf.Clear();
                m_EncodedBuffer.Data(byteBuf, 0, byteBuf.Capacity);
            }

            m_EncIter.Clear();
            if (m_EncIter.SetBufferAndRWFVersion(m_EncodedBuffer, reactorChannel.MajorVersion, reactorChannel.MinorVersion) != CodecReturnCode.SUCCESS)
            {
                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, "Internal error. Failed to set encode iterator buffer in LoginCallbackClient.ConvertRdmLoginToRsslBuffer()");
                }

                return ReactorCallbackReturnCode.FAILURE;
            }

            CodecReturnCode ret;
            if ((ret = loginMsg.Encode(m_EncIter)) != CodecReturnCode.SUCCESS)
            {
                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                    strBuilder.AppendLine("Internal error: failed to encode LoginMsg in LoginCallbackClient.ConvertRdmLoginToRsslBuffer()")
                        .AppendLine($"Error code {ret}").AppendLine($"Error text {ret.GetAsString()}");
                }

                return ReactorCallbackReturnCode.FAILURE;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        static void LoginMsgToString(StringBuilder toString, LoginMsg loginMsg, LoginMsgType loginType)
        {
            switch (loginType)
            {
                case LoginMsgType.REFRESH:
                    LoginRefresh loginRefresh = loginMsg.LoginRefresh!;
                    LoginAttrib? attrib = loginRefresh.HasAttrib ? loginRefresh.LoginAttrib : null;

                    toString.Append("username ").AppendLine(loginRefresh.HasUserName ? loginRefresh.UserName.ToString() : "<not set>")
                            .Append("usernameType ").AppendLine(loginRefresh.HasUserNameType ? loginRefresh.UserNameType.ToString() : "<not set>");
                    if (attrib == null)
                        return;

                    toString.Append("position ").AppendLine(attrib.HasPosition ? attrib.Position.ToString() : "<not set>")
                    .Append("appId ").AppendLine(attrib.HasApplicationId ? attrib.ApplicationId.ToString() : "<not set>")
                    .Append("applicationName ").AppendLine(attrib.HasApplicationName ? attrib.ApplicationName.ToString() : "<not set>")
                    .Append("singleOpen ").AppendLine(attrib.HasSingleOpen ? attrib.SingleOpen.ToString() : "<not set>")
                    .Append("allowSuspect ").AppendLine(attrib.HasAllowSuspectData ? attrib.AllowSuspectData.ToString() : "<not set>")
                    .Append("permissionExpressions ").AppendLine(attrib.HasProvidePermissionExpressions ? attrib.ProvidePermissionExpressions.ToString() : "<not set>")
                    .Append("permissionProfile ").AppendLine(attrib.HasProvidePermissionProfile ? attrib.ProvidePermissionProfile.ToString() : "<not set>");
                    break;
                case LoginMsgType.STATUS:
                    LoginStatus loginStatus = loginMsg.LoginStatus!;

                    toString.Append("username ").AppendLine(loginStatus.HasUserName ? loginStatus.UserName.ToString() : "<not set>")
                            .Append("usernameType ").AppendLine(loginStatus.HasUserNameType ? loginStatus.UserNameType.ToString() : "<not set>");
                    break;
                default:
                    break;
            }
        }

        State State()
        {
            if(m_EtaState == null)
            {
                m_EtaState = new State();
            }
            else
            {
                m_EtaState.Clear();
            }

            return m_EtaState;
        }

        public IRefreshMsg? GetRefreshMsg()
        {
            return codecRefreshMsg;
        }

        private void PopulateStatusMsg()
        {
            IStatusMsg etaStatusMsg = StatusMsg();
            etaStatusMsg.StreamId = 1;
            etaStatusMsg.DomainType = (int)Eta.Rdm.DomainType.LOGIN;

            if (codecRefreshMsg != null && codecRefreshMsg.CheckHasMsgKey())
            {
                etaStatusMsg.ApplyHasMsgKey();
                IMsgKey msgKey = etaStatusMsg.MsgKey;
                msgKey.Clear();
                if (codecRefreshMsg.MsgKey.CheckHasNameType())
                {
                    msgKey.ApplyHasNameType();
                    msgKey.NameType = codecRefreshMsg.MsgKey.NameType;
                }

                if (codecRefreshMsg.MsgKey.CheckHasName())
                {
                    msgKey.ApplyHasName();
                    msgKey.Name = codecRefreshMsg.MsgKey.Name;
                }
            }
        }

        internal void ProcessChannelEvent(ReactorChannelEvent evt)
        {
            if (m_LoginItemList == null)
                return;

            State state = State();
            if (m_StatusMsg == null)
                m_StatusMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();

            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_READY:
                    {
                        if (!m_NotifyChannelDownReconnecting)
                            break;

                        PopulateStatusMsg();

                        state.StreamState(StreamStates.OPEN);
                        state.DataState(DataStates.OK);
                        state.Code(StateCodes.NONE);
                        state.Text().Data("channel up");
                        codecStatusMsg!.State = state;
                        codecStatusMsg.ApplyHasState();

                        m_StatusMsg.Decode(codecStatusMsg, evt.ReactorChannel!.MajorVersion, evt.ReactorChannel.MinorVersion, null!);

                        foreach (var item in m_LoginItemList)
                        {
                            EventImpl.Item = item;

                            NotifyOnAllMsg(m_StatusMsg);
                            NotifyOnStatusMsg();
                        }

                        m_NotifyChannelDownReconnecting = false;

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    {
                        if (m_NotifyChannelDownReconnecting)
                            break;

                        PopulateStatusMsg();

                        state.StreamState(StreamStates.OPEN);
                        state.DataState(DataStates.SUSPECT);
                        state.Code(StateCodes.NONE);
                        state.Text().Data("channel down");
                        codecStatusMsg!.State = state;
                        codecStatusMsg.ApplyHasState();

                        m_StatusMsg.Decode(codecStatusMsg, evt.ReactorChannel!.MajorVersion, evt.ReactorChannel.MinorVersion, null!);

                        foreach (var item in m_LoginItemList)
                        {
                            EventImpl.Item = item;

                            NotifyOnAllMsg(m_StatusMsg);
                            NotifyOnStatusMsg();
                        }

                        m_NotifyChannelDownReconnecting = true;

                        break;
                    }
                case ReactorChannelEventType.CHANNEL_DOWN:
                    {
                        PopulateStatusMsg();

                        state.StreamState(StreamStates.CLOSED);
                        state.DataState(DataStates.SUSPECT);
                        state.Code(StateCodes.NONE);
                        state.Text().Data("channel closed");
                        codecStatusMsg!.State = state;
                        codecStatusMsg.ApplyHasState();

                        m_StatusMsg.Decode(codecStatusMsg, evt.ReactorChannel!.MajorVersion, evt.ReactorChannel.MinorVersion, null!);

                        foreach (var item in m_LoginItemList)
                        {
                            EventImpl.Item = item;

                            NotifyOnAllMsg(m_StatusMsg);
                            NotifyOnStatusMsg();
                        }

                        break;
                    }
                default:
                    break;
            }
        }

        internal CodecReturnCode OverlayLoginRequest(Eta.Codec.Msg request)
        {
            CodecReturnCode ret = CodecReturnCode.SUCCESS;

            // Clears temp buffer
            m_TempBuffer.Clear();
            m_TempByteBuffer.Clear();
            m_TempBuffer.Data(m_TempByteBuffer);

            // Encode Codec message into buffer
            m_EncIter.Clear();
            m_EncIter.SetBufferAndRWFVersion(m_TempBuffer, Codec.MajorVersion(), Codec.MinorVersion());

            while((ret = request.Encode(m_EncIter)) == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                m_TempByteBuffer = new ByteBuffer(m_TempByteBuffer.Capacity * 2);
                m_TempBuffer.Clear();
                m_TempBuffer.Data(m_TempByteBuffer);
                m_EncIter.Clear();
                m_EncIter.SetBufferAndRWFVersion(m_TempBuffer, Codec.MajorVersion(), Codec.MinorVersion());
            }

            if (ret >= CodecReturnCode.SUCCESS)
            {
                // Decode encoded Codec message into RDM message
                m_DecIter.Clear();
                m_DecIter.SetBufferAndRWFVersion(m_TempBuffer, Codec.MajorVersion(), Codec.MinorVersion());
                m_TempMsg.Clear();
                m_TempLoginReq.Clear();
                ret = m_TempMsg.Decode(m_DecIter);
                if ((ret = m_TempLoginReq.Decode(m_DecIter, m_TempMsg)) < CodecReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            else
                return ret;

            /* Apply the following changed fields to the cached login request:
             * UserName
             * Attrib
             * ApplicationName
             * Position
             * InstanceId
             * Password
             * Pause
             * No refresh
             *
             * Note: Pause and no refresh should be removed from the cached request after submitting to the reactor
            */

            if (!m_TempLoginReq.UserName.IsBlank && !m_TempLoginReq.UserName.Equals(CurrentLoginRequest.UserName))
            {
                m_TempUserNameByteBuffer.Clear();
                if (m_TempLoginReq.UserName.Copy(m_TempUserNameByteBuffer) < CodecReturnCode.SUCCESS)
                {
                    m_TempUserNameByteBuffer = new ByteBuffer(m_TempLoginReq.UserName.Length * 2);
                    m_TempLoginReq.UserName.Copy(m_TempUserNameByteBuffer);
                }

                CurrentLoginRequest.UserName.Data(m_TempUserNameByteBuffer, 0, m_TempLoginReq.UserName.Length);
            }

            if(m_TempLoginReq.HasAttrib)
            {
                if(m_TempLoginReq.LoginAttrib.HasApplicationId)
                {
                    if (!CurrentLoginRequest.LoginAttrib.HasApplicationId ||
                    !m_TempLoginReq.LoginAttrib.ApplicationId.Equals(CurrentLoginRequest.LoginAttrib.ApplicationId))
                    {
                        CurrentLoginRequest.HasAttrib = true;
                        ByteBuffer byteBuffer = new (m_TempLoginReq.LoginAttrib.ApplicationId.Length);
                        m_TempLoginReq.LoginAttrib.ApplicationId.Copy(byteBuffer);
                        CurrentLoginRequest.LoginAttrib.HasApplicationId = true;
                        CurrentLoginRequest.LoginAttrib.ApplicationId.Data(byteBuffer);
                    }
                }

                if (m_TempLoginReq.LoginAttrib.HasApplicationName)
                {
                    if (!CurrentLoginRequest.LoginAttrib.HasApplicationName ||
                    !m_TempLoginReq.LoginAttrib.ApplicationName.Equals(CurrentLoginRequest.LoginAttrib.ApplicationName))
                    {
                        CurrentLoginRequest.HasAttrib = true;
                        ByteBuffer byteBuffer = new(m_TempLoginReq.LoginAttrib.ApplicationName.Length);
                        m_TempLoginReq.LoginAttrib.ApplicationName.Copy(byteBuffer);
                        CurrentLoginRequest.LoginAttrib.HasApplicationName = true;
                        CurrentLoginRequest.LoginAttrib.ApplicationName.Data(byteBuffer);
                    }
                }

                if (m_TempLoginReq.LoginAttrib.HasPosition)
                {
                    if (!CurrentLoginRequest.LoginAttrib.HasPosition ||
                    !m_TempLoginReq.LoginAttrib.Position.Equals(CurrentLoginRequest.LoginAttrib.Position))
                    {
                        CurrentLoginRequest.HasAttrib = true;
                        ByteBuffer byteBuffer = new(m_TempLoginReq.LoginAttrib.Position.Length);
                        m_TempLoginReq.LoginAttrib.Position.Copy(byteBuffer);
                        CurrentLoginRequest.LoginAttrib.HasPosition = true;
                        CurrentLoginRequest.LoginAttrib.Position.Data(byteBuffer);
                    }
                }
            }

            if (m_TempLoginReq.HasInstanceId)
            {
                if (!CurrentLoginRequest.HasInstanceId ||
                    !m_TempLoginReq.InstanceId.Equals(CurrentLoginRequest.InstanceId))
                {
                    ByteBuffer byteBuffer = new(m_TempLoginReq.InstanceId.Length);
                    m_TempLoginReq.InstanceId.Copy(byteBuffer);
                    CurrentLoginRequest.HasInstanceId = true;
                    CurrentLoginRequest.InstanceId.Data(byteBuffer);
                }
            }

            if (m_TempLoginReq.HasPassword)
            {
                if (!CurrentLoginRequest.HasPassword ||
                    !m_TempLoginReq.Password.Equals(CurrentLoginRequest.Password))
                {
                    ByteBuffer byteBuffer = new(m_TempLoginReq.Password.Length);
                    m_TempLoginReq.Password.Copy(byteBuffer);
                    CurrentLoginRequest.HasPassword = true;
                    CurrentLoginRequest.Password.Data(byteBuffer);
                }
            }

            if (m_TempLoginReq.Pause)
            {
                CurrentLoginRequest.Pause = true;
            }

            if (m_TempLoginReq.NoRefresh)
            {
                CurrentLoginRequest.NoRefresh = true;
            }

            return ret;
        }

        internal ReactorCallbackReturnCode ProcessGenericMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_LoginItemList != null)
            {
                if (m_GenericMsg == null)
                {
                    m_GenericMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmGenericMsg();
                }

                m_GenericMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel.MinorVersion,
                    channelInfo.DataDictionary!);

                if (channelInfo.SessionChannelInfo != null && m_GenericMsg.HasServiceId)
                {
                    var directory = channelInfo.SessionChannelInfo.GetDirectoryById(m_GenericMsg.ServiceId());
                    if (directory != null && directory.HasGeneratedServiceId)
                    {
                        m_GenericMsg.ServiceIdInt(directory.GeneratedServiceId());
                    }
                }

                foreach (var loginItem in m_LoginItemList)
                {
                    EventImpl.Item = loginItem;

                    NotifyOnAllMsg(m_GenericMsg);
                    NotifyOnGenericMsg();
                }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        internal ReactorCallbackReturnCode ProcessAckMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_LoginItemList != null)
            {
                if (m_AckMsg == null)
                {
                    m_AckMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmAckMsg();
                }

                m_AckMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel.MinorVersion,
                    channelInfo.DataDictionary!);

                m_AckMsg.SetServiceName(null!);

                if (m_AckMsg.HasServiceId)
                {
                    if (channelInfo.SessionChannelInfo != null)
                    {
                        var directory = channelInfo.SessionChannelInfo.GetDirectoryById(m_AckMsg.ServiceId());
                        if (directory != null && directory.HasGeneratedServiceId)
                        {
                            m_AckMsg.ServiceIdInt(directory.GeneratedServiceId());
                            m_AckMsg.ServiceName(directory.ServiceName!);
                        }
                    }
                    else
                    {
                        var directory = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(m_AckMsg.ServiceId());

                        if (directory != null && directory.ServiceName != null)
                            m_AckMsg.ServiceName(directory.ServiceName);
                    }
                }

                foreach (var loginItem in m_LoginItemList)
                {
                    EventImpl.Item = loginItem;

                    NotifyOnAllMsg(m_AckMsg);
                    NotifyOnAckMsg();
                }
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }
    }
}
