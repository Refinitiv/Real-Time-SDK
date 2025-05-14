/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Reactor;
using System.Text;

namespace LSEG.Ema.Access
{
    internal class MarketItemHandler : IDefaultMsgCallback
    {
        const string CLIENT_NAME = "MarketItemHandler";

        OmmServerBaseImpl m_OmmServerBaseImpl;

        EncodeIterator m_EncodeIterator = new();
        Eta.Codec.Msg m_EtaMsg = new();
        Buffer m_MsgBuffer = new();
        Buffer m_TextBuffer = new();
        ReactorErrorInfo m_ErrorInfo = new();
        ReactorSubmitOptions m_SubmitOptions = new();
        bool m_IsDirectoryApiControl;

        internal bool AcceptMessageWithoutBeingLogin { get; set; } = false;
        internal bool AcceptMessageWithoutAcceptingRequests { get; set; } = false;
        internal bool AcceptMessageWithoutQosInRange { get; set; } = false;
        internal bool AcceptMessageSameKeyButDiffStream { get; set; } = false;
        internal bool EnforceAckIDValidation { get; set; } = false;
        internal bool AcceptMessageThatChangesService { get; set;} = false;

        /* TODO : this is used to override the value of API directory control from configuration. */
        internal bool IsDirectoryApiControl 
        { 
            set
            {
                m_IsDirectoryApiControl = value;
            }
        }
        /* END */

        public MarketItemHandler(OmmServerBaseImpl ommServerBaseImpl)
        {
            m_OmmServerBaseImpl = ommServerBaseImpl;

            m_IsDirectoryApiControl = ommServerBaseImpl.ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL ?
                                           true : false;
        }

        public void Initialize()
        {
            m_MsgBuffer.Data(new Eta.Common.ByteBuffer(1024));
            m_TextBuffer.Data(new Eta.Common.ByteBuffer(512));

            AcceptMessageWithoutBeingLogin = m_OmmServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageWithoutBeingLogin;
            AcceptMessageWithoutAcceptingRequests = m_OmmServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageWithoutAcceptingRequests;
            AcceptMessageWithoutQosInRange = m_OmmServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageWithoutQosInRange;
            AcceptMessageSameKeyButDiffStream = m_OmmServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageSameKeyButDiffStream;
            EnforceAckIDValidation = m_OmmServerBaseImpl.ConfigImpl.IProviderConfig.EnforceAckIDValidation;
            AcceptMessageThatChangesService = m_OmmServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageThatChangesService;
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            m_OmmServerBaseImpl.EventReceived();
            IMsg? msg = msgEvent.Msg;
            ReactorChannel reactorChannel = msgEvent.ReactorChannel!;
            ClientSession? clientSession = (ClientSession?)reactorChannel?.UserSpecObj;

            if(clientSession == null || clientSession.Channel() == null)
            {
                if(m_OmmServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                        .AppendLine("Received non-existence client session")
                        .AppendLine($"ErrorText {msgEvent.ReactorErrorInfo.Error.Text}")
                        .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                    m_OmmServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, text.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            if(msg == null)
            {
                if (m_OmmServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                        .AppendLine("Received error message.")
                        .AppendLine($"ErrorText {msgEvent.ReactorErrorInfo.Error.Text}")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                    m_OmmServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, text.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            if(!AcceptMessageWithoutBeingLogin && !clientSession.IsLogin)
            {
                using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                StringBuilder temp = m_OmmServerBaseImpl.GetStrBuilder();
                temp.Append("Message rejected - there is no logged in user for this session.");

                SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, temp.ToString());

                if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                {
                    temp
                        .AppendLine($"Stream Id {msg.StreamId}")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                    m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            ItemInfo? itemInfo = clientSession.GetItemInfo(msg.StreamId);

            // Ensure the rsslMsg of all relevant message types contain the same flags of the message event
            int messageFlags = msg.Flags;

            switch(msg.MsgClass)
            {
                case MsgClasses.REQUEST:
                    {
                        LogReceivedMessage(msg, clientSession, "Request");

                        RequestMsg reqMsg = m_OmmServerBaseImpl.RequestMsg();

                        int keyFlags = msg.MsgKey.Flags;

                        if( (keyFlags & MsgKeyFlags.HAS_SERVICE_ID) == MsgKeyFlags.HAS_SERVICE_ID)
                        {
                            reqMsg.Decode(msg, reactorChannel!.MajorVersion, reactorChannel.MinorVersion,
                                    m_OmmServerBaseImpl.DictionaryHandler.GetDictionaryByServiceId(msg.MsgKey.ServiceId));

                            if (m_OmmServerBaseImpl.GetDirectoryServiceStore().GetServiceNameById(msg.MsgKey.ServiceId, out var serviceName))
                            {
                                keyFlags &= ~MsgKeyFlags.HAS_SERVICE_ID;

                                reqMsg.m_rsslMsg.MsgKey.Flags = keyFlags;

                                reqMsg.ServiceName(serviceName!);

                                reqMsg.m_rsslMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;

                                OmmIProviderDirectoryStore iProviderServiceStore = (OmmIProviderDirectoryStore)m_OmmServerBaseImpl.GetDirectoryServiceStore();

                                if(!AcceptMessageWithoutAcceptingRequests && m_IsDirectoryApiControl
                                    && !iProviderServiceStore.IsAcceptingRequests(msg.MsgKey.ServiceId))
                                {
                                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                                        .Append($"Request message rejected - the service Id = {msg.MsgKey.ServiceId} does not accept any requests.");

                                    SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, text.ToString());

                                    if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                                    {
                                        text.AppendLine().AppendLine($"Stream Id {msg.StreamId}")
                                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                                            .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                                        m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, text.ToString());
                                    }

                                    if(itemInfo != null)
                                    {
                                        NotifyOnClose(reactorChannel!, msg, itemInfo);
                                    }

                                    break;
                                }

                                if(!AcceptMessageWithoutQosInRange && m_IsDirectoryApiControl && 
                                    !iProviderServiceStore.IsValidQosRange(msg.MsgKey.ServiceId, (IRequestMsg)msg))
                                {
                                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                                        .Append($"Request message rejected - the service Id = {msg.MsgKey.ServiceId} does not support the specified QoS(");

                                    IRequestMsg requestMsg = (IRequestMsg)msg;

                                    text.Append($"{requestMsg.Qos})");

                                    if(requestMsg.CheckHasWorstQos())
                                    {
                                        text.Append($" and Worst QoS({requestMsg.WorstQos}).");
                                    }
                                    else
                                    {
                                        text.Append(".");
                                    }

                                    SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, text.ToString());

                                    if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                                    {
                                        text.AppendLine().AppendLine($"Stream Id {msg.StreamId}")
                                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                                            .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                                        m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, text.ToString());
                                    }

                                    if (itemInfo != null)
                                    {
                                        NotifyOnClose(reactorChannel!, msg, itemInfo);
                                    }

                                    break;
                                }
                            }
                            else
                            {
                                HandleNonExistenServiceId(msg, clientSession, itemInfo);

                                break;
                            }
                        }
                        else
                        {
                            reqMsg.Decode(msg, reactorChannel!.MajorVersion, reactorChannel.MinorVersion, null);
                        }

                        /* Checks whether this is new or existing request. */
                        if(itemInfo == null)
                        {
                            itemInfo = m_OmmServerBaseImpl.ServerPool.GetItemInfo();

                            itemInfo.RequestMsg((IRequestMsg)msg);
                            itemInfo.ClientSession = clientSession;

                            if(!AcceptMessageSameKeyButDiffStream)
                            {
                                if(clientSession.CheckingExistingReq(itemInfo))
                                {
                                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                                        .Append("Request Message rejected - Item already open with exact same message key on another stream.");

                                    SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, text.ToString());

                                    if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                                    {
                                        text.AppendLine().AppendLine($"Stream Id {msg.StreamId}")
                                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                                            .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                                        m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, text.ToString());
                                    }

                                    itemInfo.ReturnToPool();

                                    return ReactorCallbackReturnCode.SUCCESS;
                                }
                            }

                            m_OmmServerBaseImpl.AddItemInfo(itemInfo);

                            SetCommonProviderEventAttributes(reactorChannel!, itemInfo.Handle, clientSession.ClientHandle);

                            m_OmmServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_OmmServerBaseImpl.OmmProviderEvent);
                            m_OmmServerBaseImpl.OmmProviderClient.OnReqMsg(reqMsg, m_OmmServerBaseImpl.OmmProviderEvent);
                        }
                        else
                        {
                            bool setMessageKey = false;

                            if(msg.MsgKey.CheckHasServiceId() && itemInfo.MsgKey.CheckHasServiceId())
                            {
                                if(itemInfo.MsgKey.ServiceId != msg.MsgKey.ServiceId)
                                {
                                    if(!AcceptMessageThatChangesService)
                                    {
                                        using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                                        StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                                            .Append($"Request Message rejected - Attempt to reissue the service Id from {itemInfo.ServiceId}")
                                            .Append($" to {msg.MsgKey.ServiceId} while this is not supported.");

                                        SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, text.ToString());

                                        if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                                        {
                                            text.AppendLine().AppendLine($"Stream Id {msg.StreamId}")
                                                .AppendLine($"Client handle {clientSession.ClientHandle}")
                                                .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                                            m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, text.ToString());
                                        }

                                        NotifyOnClose(reactorChannel!, msg, itemInfo);

                                        break;
                                    }
                                    else
                                    {
                                        if(itemInfo.HasItemGroup)
                                        {
                                            m_OmmServerBaseImpl.RemoveItemGroup(itemInfo);

                                            itemInfo.RequestMsg((IRequestMsg)msg);

                                            setMessageKey = true;

                                            m_OmmServerBaseImpl.AddItemGroup(itemInfo, itemInfo.ItemGroup);
                                        }
                                    }
                                }
                            }

                            if(!setMessageKey)
                            {
                                itemInfo.RequestMsg((IRequestMsg)msg);
                            }

                            SetCommonProviderEventAttributes(reactorChannel!, itemInfo.Handle, clientSession.ClientHandle);

                            m_OmmServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_OmmServerBaseImpl.OmmProviderEvent);
                            m_OmmServerBaseImpl.OmmProviderClient.OnReissue(reqMsg, m_OmmServerBaseImpl.OmmProviderEvent);
                        }

                        break;
                    }
                case MsgClasses.CLOSE:
                    {
                        LogReceivedMessage(msg, clientSession, "Close");

                        m_OmmServerBaseImpl.OmmProviderEvent.ReactorChannel = reactorChannel;
                        if(itemInfo != null)
                        {
                            NotifyOnClose(reactorChannel!, msg, itemInfo);
                        }

                        break;
                    }
                case MsgClasses.POST:
                    {
                        /* This is null handling when the itemInfo is null in the ProcessPost method*/
                        ProcessPost(msg, reactorChannel!, clientSession, itemInfo!);

                        break;
                    }
                case MsgClasses.GENERIC:
                    {
                        LogReceivedMessage(msg, clientSession, "Generic");

                        if(itemInfo != null)
                        {
                            GenericMsg genericMsg = m_OmmServerBaseImpl.GenericMsg();

                            genericMsg.Decode(msg, reactorChannel!.MajorVersion, reactorChannel.MinorVersion,
                                GetDataDictionary(itemInfo, msg));

                            SetCommonProviderEventAttributes(reactorChannel!, itemInfo.Handle, clientSession.ClientHandle);

                            m_OmmServerBaseImpl.OmmProviderClient.OnAllMsg(genericMsg, m_OmmServerBaseImpl.OmmProviderEvent);
                            m_OmmServerBaseImpl.OmmProviderClient.OnGenericMsg(genericMsg, m_OmmServerBaseImpl.OmmProviderEvent);
                        }

                        break;
                    }
                default:
                    {
                        using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                        StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                            .Append($"Rejected unhandled message type {MsgClasses.ToString(msg.MsgClass)}");

                        if(itemInfo == null)
                        {
                            SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, text.ToString());
                        }

                        if (m_OmmServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                        {
                            text.AppendLine().AppendLine($"Stream Id {msg.StreamId}")
                                .AppendLine($"Client handle {clientSession.ClientHandle}")
                                .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                            m_OmmServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, text.ToString());
                        }

                        break;
                    }

            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        void NotifyOnClose(ReactorChannel reactorChannel, IMsg msg, ItemInfo itemInfo)
        {
            IRequestMsg requestMsg = m_OmmServerBaseImpl.GetRequestMsg();

            requestMsg.ApplyNoRefresh();
            requestMsg.StreamId = msg.StreamId;

            if(itemInfo.MsgKey.CheckHasName())
            {
                requestMsg.MsgKey.ApplyHasName();
                requestMsg.MsgKey.Name = itemInfo.MsgKey.Name;
            }

            if(itemInfo.MsgKey.CheckHasNameType())
            {
                requestMsg.MsgKey.ApplyHasNameType();
                requestMsg.MsgKey.NameType = itemInfo.MsgKey.NameType;
            }

            requestMsg.DomainType = msg.DomainType;

            var emaReqMsg = m_OmmServerBaseImpl.RequestMsg();

            int flags;

            if (itemInfo.MsgKey.CheckHasServiceId())
            {
                requestMsg.MsgKey.ApplyHasServiceId();
                requestMsg.MsgKey.ServiceId = itemInfo.MsgKey.ServiceId;

                emaReqMsg.Decode(requestMsg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null);

                flags = emaReqMsg.m_rsslMsg.MsgKey.Flags;

                if(m_OmmServerBaseImpl.GetDirectoryServiceStore().GetServiceNameById(itemInfo.MsgKey.ServiceId, out string? serviceName))
                {
                    flags &= ~MsgKeyFlags.HAS_SERVICE_ID;

                    emaReqMsg.m_rsslMsg.MsgKey.Flags = flags;

                    emaReqMsg.ServiceName(serviceName!);

                    emaReqMsg.m_rsslMsg.MsgKey.Flags = flags | MsgKeyFlags.HAS_SERVICE_ID;
                }
            }
            else
            {
                emaReqMsg.Decode(requestMsg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null);
            }

            flags = emaReqMsg.m_rsslMsg.Flags;
            flags &= ~RequestMsgFlags.STREAMING;
            requestMsg.Flags = flags;

            m_OmmServerBaseImpl.OmmProviderEvent.clientHandle = itemInfo.ClientSession!.ClientHandle;
            m_OmmServerBaseImpl.OmmProviderEvent.closure = m_OmmServerBaseImpl.Closure;
            m_OmmServerBaseImpl.OmmProviderEvent.SetOmmProvider(m_OmmServerBaseImpl.Provider);
            m_OmmServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;

            m_OmmServerBaseImpl.OmmProviderClient.OnAllMsg(emaReqMsg, m_OmmServerBaseImpl.OmmProviderEvent);
            m_OmmServerBaseImpl.OmmProviderClient.OnClose(emaReqMsg, m_OmmServerBaseImpl.OmmProviderEvent);

            m_OmmServerBaseImpl.RemoveItemInfo(itemInfo, true);
        }

        void SendRejectMessage(ClientSession clientSession, IMsg msg, int statusCode, string statusText)
        {
            IStatusMsg statusMsg = m_EtaMsg;
            statusMsg.Clear();

            statusMsg.MsgClass = MsgClasses.STATUS;
            statusMsg.StreamId = msg.StreamId;
            statusMsg.DomainType = msg.DomainType;
            statusMsg.ContainerType = DataTypes.NO_DATA;
            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(StreamStates.CLOSED_RECOVER);
            statusMsg.State.DataState(DataStates.SUSPECT);
            statusMsg.State.Code(statusCode);
            m_TextBuffer.Data(statusText);
            statusMsg.State.Text(m_TextBuffer);

            if(msg.MsgClass == MsgClasses.REQUEST)
            {
                if( ((IRequestMsg)msg).CheckPrivateStream())
                {
                    statusMsg.ApplyPrivateStream();
                }
            }

            // Clears underlying ByteBuffer to reset buffer's length
            Eta.Common.ByteBuffer byteBuf = m_MsgBuffer.Data();
            byteBuf.Clear();
            m_MsgBuffer.Data(byteBuf, 0, byteBuf.Capacity);

            m_EncodeIterator.Clear();
            CodecReturnCode retCode = m_EncodeIterator.SetBufferAndRWFVersion(m_MsgBuffer, clientSession.Channel().MajorVersion,
                            clientSession.Channel().MinorVersion);

            if(retCode != CodecReturnCode.SUCCESS)
            {
                if (m_OmmServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                        .AppendLine("Internal error. Failed to set decode iterator buffer and version in MarketPriceHandler.SendRejectMessage().")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                    m_OmmServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, text.ToString());
                }

                return;
            }

            while ((retCode = statusMsg.Encode(m_EncodeIterator)) == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                retCode = Utilities.RealignBuffer(m_EncodeIterator, m_MsgBuffer.Capacity * 2);

                if (retCode != CodecReturnCode.SUCCESS)
                    break;
            }

            if(retCode < CodecReturnCode.SUCCESS)
            {
                if (m_OmmServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                        .AppendLine("Internal error. Failed to encode status message in MarketPriceHandler.SendRejectMessage().")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                    m_OmmServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, text.ToString());
                }

                return;
            }

            ReactorReturnCode reactorRetCode;

            m_SubmitOptions.Clear();
            if (ReactorReturnCode.SUCCESS > (reactorRetCode = clientSession.Channel().Submit(statusMsg, m_SubmitOptions, out var errorInfo)))
            {
                if (m_OmmServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                        .AppendLine("Internal error: ReactorChannel.Submit() failed in MarketItemHandler.SendRejectMessage().")
                        .AppendLine($"Stream Id {msg.StreamId}")
                        .AppendLine($"Client handle {clientSession.ClientHandle}")
                        .AppendLine($"Error Id {errorInfo?.Error.ErrorId}")
                        .AppendLine($"Internal SysError {errorInfo?.Error.SysError}")
                        .AppendLine($"Error Location {errorInfo?.Location}")
                        .AppendLine($"Error Text {errorInfo?.Error.Text}");

                    m_OmmServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, text.ToString());
                }

                return;
            }
        }

        void ProcessPost(IMsg msg, ReactorChannel reactorChannel, ClientSession clientSession, ItemInfo itemInfo)
        {
            LogReceivedMessage(msg, clientSession, "Post");
            if(itemInfo == null)
            {
                return;
            }

            PostMsg emaPostMsg = m_OmmServerBaseImpl.PostMsg();
            emaPostMsg.Decode(msg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, GetDataDictionary(itemInfo, msg));

            if(((IPostMsg)msg).CheckHasMsgKey() && msg.MsgKey.CheckHasServiceId())
            {
                if(m_OmmServerBaseImpl.GetDirectoryServiceStore().GetServiceNameById(msg.MsgKey.ServiceId, out var serviceName) == false)
                {
                    if(m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                        StringBuilder temp = m_OmmServerBaseImpl.GetStrBuilder()
                            .Append($"Post Message invalid - the service Id = {msg.MsgKey.ServiceId}")
                            .Append(" does not exist in the source directory.").AppendLine()
                            .AppendLine($"Stream Id {msg.StreamId}")
                            .AppendLine($"Client handle {clientSession.ClientHandle}")
                            .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                        m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }
                }
                else
                {
                    emaPostMsg.ServiceName(serviceName!);
                }
            }

            SetCommonProviderEventAttributes(reactorChannel, itemInfo.Handle, clientSession.ClientHandle);

            if(EnforceAckIDValidation)
            {
                itemInfo.AddPostId(emaPostMsg.PostId());
            }

            m_OmmServerBaseImpl.OmmProviderClient.OnAllMsg(emaPostMsg, m_OmmServerBaseImpl.OmmProviderEvent);
            m_OmmServerBaseImpl.OmmProviderClient.OnPostMsg(emaPostMsg, m_OmmServerBaseImpl.OmmProviderEvent);
        }

        void LogReceivedMessage(IMsg msg, ClientSession clientSession, string messageName)
        {
            if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
            {
                using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
                StringBuilder text = m_OmmServerBaseImpl.GetStrBuilder()
                    .AppendLine($"Received  {messageName} message.")
                    .AppendLine($"Stream Id {msg.StreamId}")
                    .AppendLine($"Client Handle {clientSession.ClientHandle}")
                    .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, text.ToString());
            }
        }

        void SetCommonProviderEventAttributes(ReactorChannel reactorChannel, long handle, long clientHandle)
        {
            m_OmmServerBaseImpl.OmmProviderEvent.clientHandle = clientHandle;
            m_OmmServerBaseImpl.OmmProviderEvent.closure = m_OmmServerBaseImpl.Closure;
            m_OmmServerBaseImpl.OmmProviderEvent.SetOmmProvider(m_OmmServerBaseImpl.Provider);
            m_OmmServerBaseImpl.OmmProviderEvent.ReactorChannel = reactorChannel;
            m_OmmServerBaseImpl.OmmProviderEvent.handle = handle;
        }

        void HandleNonExistenServiceId(IMsg msg, ClientSession clientSession, ItemInfo? itemInfo)
        {
            using var lockScope = m_OmmServerBaseImpl.GetUserLocker().EnterLockScope();
            StringBuilder temp = m_OmmServerBaseImpl.GetStrBuilder()
                .Append($"Request Message rejected - the service Id = {msg.MsgKey.ServiceId} does not exist in the source directory.");

            SendRejectMessage(clientSession, msg, StateCodes.USAGE_ERROR, temp.ToString());

            if (m_OmmServerBaseImpl.GetLoggerClient().IsTraceEnabled)
            {
                temp.AppendLine().AppendLine($"Stream Id {msg.StreamId}")
                    .AppendLine($"Client Handle {clientSession.ClientHandle}")
                    .AppendLine($"Instance Name {m_OmmServerBaseImpl.InstanceName}");

                m_OmmServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
            }

            if(itemInfo != null)
            {
                NotifyOnClose(clientSession.Channel(), msg, itemInfo);
            }
        }

        DataDictionary? GetDataDictionary(ItemInfo itemInfo, IMsg msg)
        {
            if (itemInfo.MsgKey.CheckHasServiceId())
            {
                return m_OmmServerBaseImpl.DictionaryHandler.GetDictionaryByServiceId(itemInfo.MsgKey.ServiceId);
            }
            else if (msg.MsgKey != null && msg.MsgKey.CheckHasServiceId())
            {
                return m_OmmServerBaseImpl.DictionaryHandler.GetDictionaryByServiceId(msg.MsgKey.ServiceId);
            }

            return null;
        }

    }
}
