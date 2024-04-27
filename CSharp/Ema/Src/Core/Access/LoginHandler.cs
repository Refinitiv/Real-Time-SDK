/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using System.Text;
using LoginStatus = LSEG.Eta.ValueAdd.Rdm.LoginStatus;

namespace LSEG.Ema.Access
{
    internal class LoginHandler : IRDMLoginMsgCallback
    {
        private OmmServerBaseImpl m_ServerBaseImpl;

        private const string CLIENT_NAME = "LoginHandler";
	
	    private LoginStatus m_loginStatus = new LoginStatus();
        private ReactorSubmitOptions m_rsslSubmitOptions = new ReactorSubmitOptions();

        internal List<ItemInfo> ItemInfoList { get; set; } = new List<ItemInfo>();

        public LoginHandler(OmmServerBaseImpl serverBaseImpl)
        {
            m_ServerBaseImpl = serverBaseImpl;
        }

        public void Initialize() { }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginMsgEvent)
        {
            m_ServerBaseImpl.EventReceived();
            ClientSession clientSession = (ClientSession)loginMsgEvent.ReactorChannel!.UserSpecObj!;
            LoginMsg? loginMsg = loginMsgEvent.LoginMsg;

            StringBuilder temp = m_ServerBaseImpl.GetStrBuilder();
            if (loginMsg == null || loginMsgEvent.Msg == null)
            {
                temp.Append("Login message rejected - Invalid login domain message.");

                if (loginMsgEvent.Msg != null)
                {
                    SendLoginReject(loginMsgEvent.ReactorChannel, loginMsgEvent.Msg.StreamId, StateCodes.USAGE_ERROR, temp.ToString());
                }

                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    temp.Append(ILoggerClient.CR).Append("Stream Id ").Append(loginMsgEvent.Msg?.StreamId)
                    .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                    .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle);
                    m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            ItemInfo? itemInfo;
            switch (loginMsg.LoginMsgType)
            {
                case LoginMsgType.REQUEST:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received login request message.")
                            .Append(ILoggerClient.CR).Append("Stream Id ").Append(loginMsgEvent.Msg.StreamId)
                            .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                            .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        RequestMsg reqMsg = m_ServerBaseImpl.RequestMsg();

                        reqMsg.Decode(loginMsgEvent.Msg, loginMsgEvent.ReactorChannel.MajorVersion, loginMsgEvent.ReactorChannel.MinorVersion, null);

                        m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                        m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                        m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                        m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = loginMsgEvent.ReactorChannel;

                        itemInfo = clientSession.GetItemInfo(loginMsgEvent.Msg.StreamId);
                        IRequestMsg requestMsg = (IRequestMsg)loginMsgEvent.Msg;

                        if (itemInfo == null)
                        {
                            itemInfo = m_ServerBaseImpl.ServerPool.GetItemInfo();

                            itemInfo.RequestMsg(requestMsg);
                            itemInfo.ClientSession = clientSession;
                            clientSession.LoginHandle = itemInfo.Handle;

                            m_ServerBaseImpl.AddItemInfo(itemInfo);

                            ItemInfoList.Add(itemInfo);

                            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;

                            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
                            m_ServerBaseImpl.OmmProviderClient.OnReqMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

                            reqMsg.Clear();
                        }
                        else
                        {
                            itemInfo.RequestMsg(requestMsg);

                            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;

                            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
                            m_ServerBaseImpl.OmmProviderClient.OnReissue(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

                            reqMsg.Clear();
                        }

                        break;
                    }
                case LoginMsgType.CONSUMER_CONNECTION_STATUS:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received Consumer Connection Status message.")
                            .Append(ILoggerClient.CR).Append("Stream Id ").Append(loginMsgEvent.Msg.StreamId)
                            .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                            .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        itemInfo = clientSession.GetItemInfo(loginMsgEvent.Msg.StreamId);

                        if (itemInfo != null)
                        {
                            var genericMsg = m_ServerBaseImpl.GenericMsg();

                            genericMsg.Decode(loginMsgEvent.Msg, loginMsgEvent.ReactorChannel.MajorVersion, loginMsgEvent.ReactorChannel.MinorVersion, null);

                            m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                            m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                            m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;
                            m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = loginMsgEvent.ReactorChannel;

                            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(genericMsg, m_ServerBaseImpl.OmmProviderEvent);
                            m_ServerBaseImpl.OmmProviderClient.OnGenericMsg(genericMsg, m_ServerBaseImpl.OmmProviderEvent);

                            genericMsg.Clear();
                        }

                        break;
                    }
                case LoginMsgType.RTT:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received RoundTripLatency message.")
                                    .Append(ILoggerClient.CR).Append("Stream Id ").Append(loginMsgEvent.Msg.StreamId)
                                    .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                                    .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        itemInfo = clientSession.GetItemInfo(loginMsgEvent.Msg.StreamId);

                        if (itemInfo != null)
                        {
                            var genericMsg = m_ServerBaseImpl.GenericMsg();

                            genericMsg.Decode(loginMsgEvent.Msg, loginMsgEvent.ReactorChannel.MajorVersion, loginMsgEvent.ReactorChannel.MinorVersion, null);

                            m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                            m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                            m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;
                            m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = loginMsgEvent.ReactorChannel;

                            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(genericMsg, m_ServerBaseImpl.OmmProviderEvent);
                            m_ServerBaseImpl.OmmProviderClient.OnGenericMsg(genericMsg, m_ServerBaseImpl.OmmProviderEvent);

                            genericMsg.Clear();
                        }

                        break;
                    }
                case LoginMsgType.CLOSE:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received login close message.")
                            .Append(ILoggerClient.CR).Append("Stream Id ").Append(loginMsgEvent.Msg.StreamId)
                            .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                            .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        itemInfo = clientSession.GetItemInfo(loginMsgEvent.Msg.StreamId);

                        if (itemInfo != null)
                        {
                            IRequestMsg rsslReqMsg = m_ServerBaseImpl.GetRequestMsg();

                            rsslReqMsg.ApplyNoRefresh();
                            rsslReqMsg.StreamId = loginMsgEvent.Msg.StreamId;

                            if (itemInfo.MsgKey.CheckHasName())
                            {
                                rsslReqMsg.MsgKey.ApplyHasName();
                                rsslReqMsg.MsgKey.Name = itemInfo.MsgKey.Name;
                            }

                            if (itemInfo.MsgKey.CheckHasNameType())
                            {
                                rsslReqMsg.MsgKey.ApplyHasNameType();
                                rsslReqMsg.MsgKey.NameType = itemInfo.MsgKey.NameType;
                            }

                            rsslReqMsg.DomainType = loginMsgEvent.Msg.DomainType;

                            RequestMsg reqMsg = m_ServerBaseImpl.RequestMsg();

                            reqMsg.Decode(rsslReqMsg, loginMsgEvent.ReactorChannel.MajorVersion, loginMsgEvent.ReactorChannel.MinorVersion, null);

                            int flags = reqMsg.m_rsslMsg.Flags;
                            flags &= ~RequestMsgFlags.STREAMING;
                            rsslReqMsg.Flags = flags;

                            m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                            m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                            m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;
                            m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = loginMsgEvent.ReactorChannel;
                            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
                            m_ServerBaseImpl.OmmProviderClient.OnClose(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

                            reqMsg.Clear();

                            clientSession.ResetLoginHandle();
                            m_ServerBaseImpl.ServerChannelHandler.CloseChannel(loginMsgEvent.ReactorChannel);
                        }

                        break;
                    }
                default:
                    temp.Append("Rejected unhandled login message type ").Append(loginMsg.LoginMsgType.ToString()).Append(".");

                    itemInfo = clientSession.GetItemInfo(loginMsgEvent.Msg.StreamId);

                    if (itemInfo == null)
                    {
                        SendLoginReject(loginMsgEvent.ReactorChannel, loginMsgEvent.Msg.StreamId, StateCodes.USAGE_ERROR, temp.ToString());
                    }

                    if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        temp.Append(ILoggerClient.CR).Append("Stream Id ").Append(loginMsgEvent.Msg.StreamId)
                        .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                        .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle);
                        m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }

                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        internal void SendLoginReject(ReactorChannel reactorChannel, int streamId, int statusCode, string text)
        {
            m_loginStatus.Clear();
            m_loginStatus.StreamId = streamId;
            m_loginStatus.HasState = true;
            m_loginStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_loginStatus.State.DataState(DataStates.SUSPECT);
            m_loginStatus.State.Code(statusCode);
            Buffer buffer = new Buffer();
            buffer.Data(text);
            m_loginStatus.State.Text(buffer);

            m_rsslSubmitOptions.Clear();
            ReactorReturnCode retCode = reactorChannel.Submit(m_loginStatus, m_rsslSubmitOptions, out var errorInfo);

            if (retCode != ReactorReturnCode.SUCCESS)
            {
                StringBuilder temp = m_ServerBaseImpl.GetStrBuilder();
                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    var error = errorInfo?.Error;

                    temp.Append("Internal error: rsslChannel.Submit() failed in LoginHandler.SendLoginReject().")
                        .Append("RsslChannel ").Append((error?.Channel != null ? error.Channel.GetHashCode() : 0).ToString("X"))
                        .Append(ILoggerClient.CR)
                        .Append("Error Id ").Append(error?.ErrorId).Append(ILoggerClient.CR)
                        .Append("Internal sysError ").Append(error?.SysError).Append(ILoggerClient.CR)
                        .Append("Error Location ").Append(errorInfo?.Location).Append(ILoggerClient.CR)
                        .Append("Error Text ").Append(error?.Text);

                    m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }
            }
        }

        internal void NotifyChannelDown(ClientSession clientSession)
        {
            m_ServerBaseImpl.GetUserLocker().Enter();

            try
            {
                for (int index = 0; index < ItemInfoList.Count; index++)
                {
                    ItemInfo itemInfo = ItemInfoList[index];

                    if (clientSession == itemInfo.ClientSession)
                    {
                        IRequestMsg rsslReqMsg = m_ServerBaseImpl.GetRequestMsg();

                        rsslReqMsg.ApplyNoRefresh();
                        rsslReqMsg.StreamId = itemInfo.StreamId;

                        if (itemInfo.MsgKey.CheckHasName())
                        {
                            rsslReqMsg.MsgKey.ApplyHasName();
                            rsslReqMsg.MsgKey.Name = itemInfo.MsgKey.Name;
                        }

                        if (itemInfo.MsgKey.CheckHasNameType())
                        {
                            rsslReqMsg.MsgKey.ApplyHasNameType();
                            rsslReqMsg.MsgKey.NameType = itemInfo.MsgKey.NameType;
                        }

                        rsslReqMsg.DomainType = EmaRdm.MMT_LOGIN;

                        RequestMsg reqMsg = m_ServerBaseImpl.RequestMsg();

                        reqMsg.Decode(rsslReqMsg, clientSession.m_ReactorChannel!.MajorVersion,
                                clientSession.m_ReactorChannel.MinorVersion, null);

                        int flags = reqMsg.m_internalRsslMsg.Flags;
                        flags &= ~RequestMsgFlags.STREAMING;
                        rsslReqMsg.Flags = flags;

                        m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                        m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                        m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                        m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;

                        m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
                        m_ServerBaseImpl.OmmProviderClient.OnClose(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

                        reqMsg.Clear();

                        clientSession.ResetLoginHandle();
                    }
                }
            }
            finally
            {
                m_ServerBaseImpl.GetUserLocker().Exit();
            }
        }
    }
}
