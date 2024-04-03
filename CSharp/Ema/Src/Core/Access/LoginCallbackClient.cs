/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class LoginItem<T> : SingleItem<T>, ITimeoutClient
    {
        private static readonly string CLIENT_NAME = "LoginItem";

        internal LoginRequest m_loginRequest;
        public List<ChannelInfo>? LoginChannelList { get; set; }

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

            if (LoginChannelList == null || LoginChannelList.Count == 0)
            {
                return;
            }

            m_OmmBaseImpl.EventReceived();
            ReactorChannel reactorChannel = LoginChannelList[0].ReactorChannel!;

            RefreshMsg refreshMsg = loginCallbackClient.m_RefreshMsg!;
            refreshMsg.Decode(loginCallbackClient.GetRefreshMsg()!, reactorChannel.MajorVersion,
                reactorChannel.MinorVersion, null!);

            loginCallbackClient.EventImpl.Item = this;
            loginCallbackClient.EventImpl.ReactorChannel = reactorChannel;

            loginCallbackClient.NotifyOnAllMsg(refreshMsg);
            loginCallbackClient.NotifyOnRefreshMsg();

            if(refreshMsg.State().StreamState != OmmState.StreamStates.OPEN)
            {
                if(loginCallbackClient.LoginItems != null)
                    loginCallbackClient.LoginItems.Remove(this);

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

            bool submitRet = Submit(m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest);

            /* Unset the pause all and no refresh flags on the stored request. */
            m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest.Flags &= ~LoginRequestFlags.PAUSE_ALL;
            m_OmmBaseImpl.LoginCallbackClient!.CurrentLoginRequest.Flags &= ~LoginRequestFlags.NO_REFRESH;

            return submitRet;
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

                if(ReactorReturnCode.SUCCESS > (ret = channelInfo.ReactorChannel.Submit(rdmRequestMsg, submitOptions,
                    out var errorInfo)))
                {
                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                    if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        strBuilder.Append("Internal error: ReactorChannel.Submit() failed in LoginItem.Submit(RequestMsg)")
                        .AppendLine($"\tReactorChannel {channelInfo.ReactorChannel.Channel?.GetHashCode()}")
                        .AppendLine($"\tError Id {errorInfo?.Error.ErrorId}")
                        .AppendLine($"\tInternal SysError {errorInfo?.Error.SysError}")
                        .AppendLine($"\tError Location {errorInfo?.Location}")
                        .Append($"\tError Text {errorInfo?.Error.Text}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());

                        strBuilder.Clear();
                    }

                    strBuilder.Append($"Failed to open or modify item request. Reason: {ret}.")
                    .Append($" Error text: {errorInfo?.Error.Text}");

                    m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.FAILURE);

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
    }

    internal class LoginCallbackClientConsumer : LoginCallbackClient<IOmmConsumerClient>
    {
        public LoginCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl) : base(baseImpl)
        {
            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)baseImpl;
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

            m_RefreshMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();

            CurrentLoginRequest = m_OmmBaseImpl.ConfigImpl.AdminLoginRequest;

            /* Override the default login request from OmmConsumerConfig */
            if (!string.IsNullOrEmpty(m_OmmBaseImpl.ConfigImpl.UserName))
            {
                if (!m_OmmBaseImpl.ConfigImpl.UserName.Equals(CurrentLoginRequest.UserName.ToString()))
                {
                    CurrentLoginRequest.UserName.Data(m_OmmBaseImpl.ConfigImpl.UserName);
                }
            }

            if (!string.IsNullOrEmpty(m_OmmBaseImpl.ConfigImpl.Password) && CurrentLoginRequest.HasPassword)
            {
                if (!m_OmmBaseImpl.ConfigImpl.Password.Equals(CurrentLoginRequest.Password.ToString()))
                {
                    CurrentLoginRequest.Password.Data(m_OmmBaseImpl.ConfigImpl.Password);
                }
            }

            if(!string.IsNullOrEmpty(m_OmmBaseImpl.ConfigImpl.ApplicationId))
            {
                if(CurrentLoginRequest.HasAttrib && CurrentLoginRequest.LoginAttrib.HasApplicationId)
                {
                    if (!m_OmmBaseImpl.ConfigImpl.ApplicationId.Equals(CurrentLoginRequest.LoginAttrib.ApplicationId.ToString()))
                    {
                        CurrentLoginRequest.LoginAttrib.ApplicationId.Data(m_OmmBaseImpl.ConfigImpl.ApplicationId);
                    }
                }
            }
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
            ReactorChannel reactorChannel = loginEvent.ReactorChannel!;
            ChannelInfo channelInfo = (ChannelInfo)reactorChannel.UserSpecObj!;

            m_OmmBaseImpl.EventReceived();

            if(loginMsg == null)
            {
                m_OmmBaseImpl.CloseReactorChannel(reactorChannel);

                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
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
                        if(!m_LoginChannelList.Contains(channelInfo))
                        {
                            RemoveChannelInfo(loginEvent.ReactorChannel);
                            m_LoginChannelList.Add(channelInfo);
                        }

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

                        loginMsg.LoginRefresh!.Copy(LoginRefresh);
                        State state = loginMsg.LoginRefresh.State;

                        bool closeChannel = false;

                        if (state.StreamState() != StreamStates.OPEN)
                        {
                            closeChannel = true;

                            m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN);

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

                            if(m_OmmBaseImpl.ImplState >= OmmBaseImpl<T>.OmmImplState.CHANNEL_UP)
                                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
                        }
                        else
                        {
                            m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK);
                            m_OmmBaseImpl.SetActiveReactorChannel(channelInfo);

                            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                            {
                                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                strBuilder.AppendLine("RDMLogin stream was open with refresh message")
                                    .AppendLine(loginMsg.ToString());

                                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
                            }
                        }

                        ProcessRefreshMsg(msg!, reactorChannel, loginMsg);

                        if(closeChannel)
                        {
                            m_OmmBaseImpl.UnsetActiveRsslReactorChannel(channelInfo);
                            m_OmmBaseImpl.CloseReactorChannel(reactorChannel);
                        }

                        break;
                    }
                case LoginMsgType.STATUS:
                    {
                        bool closeChannel = false;

                        LoginStatus loginStatus = loginMsg.LoginStatus!;

                        if(loginStatus.HasState)
                        {
                            State state = loginStatus.State;

                            if(state.StreamState() != StreamStates.OPEN)
                            {
                                closeChannel = true;

                                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.CHANNEL_UP_STREAM_NOT_OPEN);

                                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                                strBuilder.AppendLine($"RDMLogin stream was closed with status message");
                                LoginMsgToString(strBuilder, loginMsg, LoginMsgType.STATUS);
                                strBuilder.Append(state.ToString());

                                LoginFailureMsg = strBuilder.ToString();

                                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                                {
                                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, LoginFailureMsg);
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

                                if (m_OmmBaseImpl.ImplState >= OmmBaseImpl<T>.OmmImplState.CHANNEL_UP)
                                    m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_SUSPECT);
                            }
                            else
                            {
                                m_OmmBaseImpl.SetOmmImplState(OmmBaseImpl<T>.OmmImplState.LOGIN_STREAM_OPEN_OK);
                                m_OmmBaseImpl.SetActiveReactorChannel(channelInfo);

                                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                                {
                                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                                    strBuilder.AppendLine("RDMLogin stream was open with status message")
                                        .AppendLine(loginMsg.ToString());

                                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, strBuilder.ToString());
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

                        ProcessStatusMsg(msg!, reactorChannel, loginMsg);

                        if (closeChannel)
                        {
                            m_OmmBaseImpl.UnsetActiveRsslReactorChannel(channelInfo);
                            m_OmmBaseImpl.CloseReactorChannel(reactorChannel);
                        }

                        break;
                    }
                default:
                    {
                        if(msg != null && msg.MsgClass == MsgClasses.GENERIC)
                        {
                            ProcessGenericMsg(msg!, reactorChannel, loginEvent);
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

            foreach (ChannelInfo entry in m_LoginChannelList)
            {
                rsslSubmitOptions.Clear();
                rsslSubmitOptions.ApplyClientChannelConfig(entry.ChannelConfig);
                entry.ReactorChannel?.Submit((Eta.Codec.Msg)closeMsg, rsslSubmitOptions, out _);
            }

            return m_LoginChannelList.Count;
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
                m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(10, item);
            }

            return item;
        }

        internal ChannelInfo? ActiveChannelInfo()
        {
            if (m_LoginChannelList != null)
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

            return null;
        }

        internal void RemoveChannelInfo(ReactorChannel? reactorChannel)
        {
            if(m_LoginChannelList != null)
            {
                for(int index = 0; index < m_LoginChannelList.Count; index++)
                {
                    ChannelInfo channelInfo = m_LoginChannelList[index];
                    if(channelInfo.ReactorChannel == reactorChannel)
                    {
                        m_LoginChannelList.RemoveAt(index);
                        break;
                    }
                }
            }
        }

        ReactorCallbackReturnCode ProcessRefreshMsg(IMsg msg, ReactorChannel reactorChannel, LoginMsg loginMsg)
        {
            if (m_LoginItemList == null)
                return ReactorCallbackReturnCode.SUCCESS;

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

            m_LoginItemLock.Enter();

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

            m_LoginItemLock.Exit();

            return ReactorCallbackReturnCode.SUCCESS;
        }

        ReactorCallbackReturnCode ProcessStatusMsg(IMsg msg, ReactorChannel reactorChannel, LoginMsg loginMsg)
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

            m_LoginItemLock.Exit();

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

            for (int idx = 0; idx < m_LoginItemList.Count; ++idx)
            {
                EventImpl.Item = m_LoginItemList[idx];
                EventImpl.ReactorChannel = reactorChannel;

                NotifyOnAllMsg(m_GenericMsg);
                NotifyOnGenericMsg();
            }

            m_LoginItemLock.Exit();

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
