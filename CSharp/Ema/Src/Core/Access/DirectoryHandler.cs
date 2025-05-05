/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class DirectoryHandler : IDirectoryMsgCallback
    {
        private static string CLIENT_NAME = "DirectoryHandler";

        internal OmmServerBaseImpl m_ServerBaseImpl;

        private DirectoryStatus m_directoryStatus =  new DirectoryStatus();
        private DirectoryRefresh m_directoryRefresh = new DirectoryRefresh();
        private ReactorSubmitOptions m_rsslSubmitOptions = new ReactorSubmitOptions(); 
        private Buffer m_statusText = new Buffer();

        private bool m_apiAdminControl;

        internal List<ItemInfo> ItemInfoList = new List<ItemInfo>();

        public DirectoryHandler(OmmServerBaseImpl serverBaseImpl)
        {
            m_ServerBaseImpl = serverBaseImpl;
            m_statusText.Data("Source Directory Refresh Completed");
        }

        public void Initialize()
        {
            m_apiAdminControl = m_ServerBaseImpl.ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
        {
            m_ServerBaseImpl.EventReceived();
            using var lockScope = m_ServerBaseImpl.GetUserLocker().EnterLockScope();
            StringBuilder temp = m_ServerBaseImpl.GetStrBuilder();

            ItemInfo? itemInfo = null;

            ClientSession clientSession = (ClientSession)directoryMsgEvent.ReactorChannel!.UserSpecObj!;
            DirectoryMsg? directoryMsg = directoryMsgEvent.DirectoryMsg;

            if (directoryMsg == null || directoryMsgEvent.Msg == null)
            {
                temp.Append("Directory message rejected - Invalid directory domain message.");

                if (directoryMsgEvent.Msg != null)
                {
                    SendDirectoryReject(directoryMsgEvent.ReactorChannel, directoryMsgEvent.Msg.StreamId, StateCodes.USAGE_ERROR, temp.ToString());
                }

                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    temp.Append(ILoggerClient.CR).Append("Stream Id ").Append(directoryMsgEvent.Msg?.StreamId)
                    .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                    .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName);

                    m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            if (!m_ServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageWithoutBeingLogin && !clientSession.IsLogin)
            {
                temp.Append("Directory message rejected - there is no logged in user for this session.");

                SendDirectoryReject(directoryMsgEvent.ReactorChannel, directoryMsgEvent.Msg.StreamId, StateCodes.USAGE_ERROR,
                        temp.ToString());

                if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                {
                    temp.Append(ILoggerClient.CR).Append("Stream Id ").Append(directoryMsgEvent.Msg.StreamId)
                    .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                    .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName);

                    m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME,
                            temp.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            switch (directoryMsg.DirectoryMsgType)
            {
                case DirectoryMsgType.REQUEST:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received directory request message.")
                                .Append(ILoggerClient.CR).Append("Stream Id ").Append(directoryMsgEvent.Msg.StreamId)
                                .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                                .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        itemInfo = clientSession.GetItemInfo(directoryMsgEvent.Msg.StreamId);
                        DirectoryRequest directoryRequest = directoryMsg.DirectoryRequest!;

                        if (m_ServerBaseImpl.ConfigImpl.IProviderConfig.AcceptDirMessageWithoutMinFilters ||
                                ((directoryRequest.Filter & Eta.Rdm.Directory.ServiceFilterFlags.INFO) == Eta.Rdm.Directory.ServiceFilterFlags.INFO)
                                && ((directoryRequest.Filter & Eta.Rdm.Directory.ServiceFilterFlags.STATE) == Eta.Rdm.Directory.ServiceFilterFlags.STATE))
                        {

                            if (itemInfo == null)
                            {
                                itemInfo = m_ServerBaseImpl.ServerPool.GetItemInfo();

                                itemInfo.RequestMsg((IRequestMsg)directoryMsgEvent.Msg);
                                itemInfo.ClientSession = clientSession;

                                m_ServerBaseImpl.AddItemInfo(itemInfo);

                                ItemInfoList.Add(itemInfo);

                                if (m_apiAdminControl == false)
                                {
                                    var reqMsg = m_ServerBaseImpl.RequestMsg();

                                    reqMsg.Decode(directoryMsgEvent.Msg, directoryMsgEvent.ReactorChannel.MajorVersion,
                                        directoryMsgEvent.ReactorChannel.MinorVersion, null);

                                    m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                                    m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                                    m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                                    m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;
                                    m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = directoryMsgEvent.ReactorChannel;

                                    m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
                                    m_ServerBaseImpl.OmmProviderClient.OnReqMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

                                    reqMsg.Clear();
                                }
                                else
                                {
                                    HandleDirectoryRequest(directoryMsgEvent.ReactorChannel, directoryRequest);
                                }
                            }
                            else
                            {
                                itemInfo.RequestMsg((IRequestMsg)directoryMsgEvent.Msg);

                                if (m_apiAdminControl == false)
                                {
                                    var reqMsg = m_ServerBaseImpl.RequestMsg();

                                    reqMsg.Decode(directoryMsgEvent.Msg, directoryMsgEvent.ReactorChannel.MajorVersion, directoryMsgEvent.ReactorChannel.MinorVersion, null);

                                    m_ServerBaseImpl.OmmProviderEvent.clientHandle = clientSession.ClientHandle;
                                    m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                                    m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                                    m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;
                                    m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = directoryMsgEvent.ReactorChannel;

                                    m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
                                    m_ServerBaseImpl.OmmProviderClient.OnReissue(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

                                    reqMsg.Clear();
                                }
                                else
                                {
                                    HandleDirectoryRequest(directoryMsgEvent.ReactorChannel, directoryRequest);
                                }
                            }
                        }
                        else
                        {
                            temp.Append("Source directory request rejected - request message must minimally have SERVICE_INFO_FILTER and SERVICE_STATE_FILTER filters");

                            SendDirectoryReject(directoryMsgEvent.ReactorChannel, directoryMsgEvent.Msg.StreamId, StateCodes.USAGE_ERROR, temp.ToString());

                            if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                            {
                                temp.Append(ILoggerClient.CR).Append("stream Id ").Append(directoryMsgEvent.Msg.StreamId)
                                .Append(ILoggerClient.CR).Append("client handle ").Append(clientSession.ClientHandle);
                                m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                            }

                            if (itemInfo != null)
                            {
                                if (m_apiAdminControl == false)
                                {
                                    NotifyOnClose(directoryMsgEvent.ReactorChannel, directoryMsgEvent.Msg, itemInfo);
                                }

                                ItemInfoList.Remove(itemInfo);
                                m_ServerBaseImpl.RemoveItemInfo(itemInfo, false);
                            }
                        }

                        break;
                    }
                case DirectoryMsgType.CONSUMER_STATUS:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received directory consumer status message.")
                                .Append(ILoggerClient.CR).Append("Stream Id ").Append(directoryMsgEvent.Msg.StreamId)
                                .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                                .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        itemInfo = clientSession.GetItemInfo(directoryMsgEvent.Msg.StreamId);

                        if (itemInfo != null)
                        {
                            GenericMsg genericMsg = m_ServerBaseImpl.GenericMsg();

                            genericMsg.Decode(directoryMsgEvent.Msg, directoryMsgEvent.ReactorChannel.MajorVersion, directoryMsgEvent.ReactorChannel.MinorVersion, null);

                            m_ServerBaseImpl.OmmProviderEvent.handle = clientSession.ClientHandle;
                            m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
                            m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
                            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;
                            m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = directoryMsgEvent.ReactorChannel;

                            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(genericMsg, m_ServerBaseImpl.OmmProviderEvent);
                            m_ServerBaseImpl.OmmProviderClient.OnGenericMsg(genericMsg, m_ServerBaseImpl.OmmProviderEvent);

                            genericMsg.Clear();
                        }

                        break;
                    }
                case DirectoryMsgType.CLOSE:
                    {
                        if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                        {
                            temp.Append("Received directory close message.")
                                .Append(ILoggerClient.CR).Append("Stream Id ").Append(directoryMsgEvent.Msg.StreamId)
                                .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                                .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName);

                            m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                        }

                        itemInfo = clientSession.GetItemInfo(directoryMsgEvent.Msg.StreamId);

                        if (itemInfo != null)
                        {
                            if (m_apiAdminControl == false)
                            {
                                m_ServerBaseImpl.OmmProviderEvent.ReactorChannel = directoryMsgEvent.ReactorChannel;
                                NotifyOnClose(directoryMsgEvent.ReactorChannel, directoryMsgEvent.Msg, itemInfo);
                            }

                            ItemInfoList.Remove(itemInfo);
                            m_ServerBaseImpl.RemoveItemInfo(itemInfo, false);
                        }

                        break;
                    }
                default:
                    temp.Append("Rejected unhandled directory message type ").Append(directoryMsg.DirectoryMsgType.ToString());

                    itemInfo = clientSession.GetItemInfo(directoryMsgEvent.Msg.StreamId);

                    if (itemInfo == null)
                    {
                        SendDirectoryReject(directoryMsgEvent.ReactorChannel, directoryMsgEvent.Msg.StreamId, StateCodes.USAGE_ERROR, temp.ToString());
                    }

                    if (m_ServerBaseImpl.GetLoggerClient().IsTraceEnabled)
                    {
                        temp.Append(ILoggerClient.CR).Append("Stream Id ").Append(directoryMsgEvent.Msg.StreamId)
                           .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                           .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName);

                        m_ServerBaseImpl.GetLoggerClient().Trace(CLIENT_NAME, temp.ToString());
                    }

                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        void NotifyOnClose(ReactorChannel reactorChannel, Eta.Codec.IMsg msg, ItemInfo itemInfo)
        {
            IRequestMsg rsslReqMsg = m_ServerBaseImpl.GetRequestMsg();

            rsslReqMsg.ApplyNoRefresh();

            rsslReqMsg.StreamId = msg.StreamId;

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

            rsslReqMsg.DomainType = msg.DomainType;

            var reqMsg = m_ServerBaseImpl.RequestMsg();

            if (itemInfo.MsgKey.CheckHasServiceId())
            {
                rsslReqMsg.MsgKey.ApplyHasServiceId();
                rsslReqMsg.MsgKey.ServiceId = itemInfo.MsgKey.ServiceId;

                reqMsg.Decode(rsslReqMsg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null);

                m_ServerBaseImpl.GetDirectoryServiceStore().GetServiceNameById(itemInfo.MsgKey.ServiceId, out string? serviceName);

                if (serviceName != null)
                {
                    reqMsg.m_rsslMsg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                    reqMsg.ServiceName(serviceName);
                    reqMsg.m_rsslMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                }
            }
            else
            {
                reqMsg.Decode(rsslReqMsg, reactorChannel.MajorVersion, reactorChannel.MinorVersion, null);
            }

            int flags = reqMsg.m_rsslMsg.Flags;
            flags &= ~RequestMsgFlags.STREAMING;
            rsslReqMsg.Flags = flags;

            m_ServerBaseImpl.OmmProviderEvent.clientHandle = itemInfo.ClientSession!.ClientHandle;
            m_ServerBaseImpl.OmmProviderEvent.closure = m_ServerBaseImpl.Closure;
            m_ServerBaseImpl.OmmProviderEvent.m_OmmProvider = m_ServerBaseImpl.Provider;
            m_ServerBaseImpl.OmmProviderEvent.handle = itemInfo.Handle;

            m_ServerBaseImpl.OmmProviderClient.OnAllMsg(reqMsg, m_ServerBaseImpl.OmmProviderEvent);
            m_ServerBaseImpl.OmmProviderClient.OnClose(reqMsg, m_ServerBaseImpl.OmmProviderEvent);

            reqMsg.Clear();
        }

        void SendDirectoryReject(ReactorChannel reactorChannel, int streamId, int statusCode, string text)
        {
            m_directoryStatus.Clear();
            m_directoryStatus.StreamId = streamId;
            m_directoryStatus.HasState = true;
            m_directoryStatus.State.StreamState(StreamStates.CLOSED_RECOVER);
            m_directoryStatus.State.DataState(DataStates.SUSPECT);
            m_directoryStatus.State.Code(statusCode);
            Buffer buffer = new Buffer();
            buffer.Data(text);
            m_directoryStatus.State.Text(buffer);

            m_rsslSubmitOptions.Clear();

            ReactorReturnCode retCode = reactorChannel.Submit(m_directoryStatus, m_rsslSubmitOptions, out var errorInfo);

            if (retCode != ReactorReturnCode.SUCCESS)
            {
                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_ServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ServerBaseImpl.GetStrBuilder();
                    Eta.Transports.Error? error = errorInfo?.Error;
                    ClientSession clientSession = (ClientSession)reactorChannel.UserSpecObj!;

                    temp.Append("Internal error: ReactorChannel.Submit() failed in DirectoryHandler.SendDirectoryReject().")
                        .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                        .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                        .Append(ILoggerClient.CR).Append("Error Id ").Append(error?.ErrorId)
                        .Append(ILoggerClient.CR).Append("Internal sysError ").Append(error?.SysError)
                        .Append(ILoggerClient.CR).Append("Error Location ").Append(errorInfo?.Location)
                        .Append(ILoggerClient.CR).Append("Error Text ").Append(error?.Text);

                    m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }
            }
        }

        void HandleDirectoryRequest(ReactorChannel reactorChannel, DirectoryRequest directoryRequest)
        {
            m_directoryRefresh.Clear();
            m_directoryRefresh.State.StreamState(StreamStates.OPEN);
            m_directoryRefresh.State.DataState(DataStates.OK);
            m_directoryRefresh.State.Code(StateCodes.NONE);
            m_directoryRefresh.State.Text(m_statusText);

            long filters = m_ServerBaseImpl.GetDirectoryServiceStore().EncodeDirectoryMsg(
                m_ServerBaseImpl.GetDirectoryServiceStore().GetDirectoryCache().DirectoryRefresh.ServiceList, 
                m_directoryRefresh.ServiceList, 
                directoryRequest.Filter, 
                true, 
                directoryRequest.HasServiceId, 
                directoryRequest.ServiceId);

            m_directoryRefresh.Filter = filters;
            m_directoryRefresh.ClearCache = true;
            m_directoryRefresh.StreamId = directoryRequest.StreamId;
            m_directoryRefresh.Solicited = true;

            m_rsslSubmitOptions.Clear();

            ReactorReturnCode retCode = reactorChannel.Submit(m_directoryRefresh, m_rsslSubmitOptions, out var errorInfo);

            if (retCode != ReactorReturnCode.SUCCESS)
            {
                if (m_ServerBaseImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = m_ServerBaseImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = m_ServerBaseImpl.GetStrBuilder();
                    Eta.Transports.Error? error = errorInfo?.Error;

                    ClientSession clientSession = (ClientSession)reactorChannel.UserSpecObj!;

                    temp.Append("Internal error: rsslChannel.Submit() failed in DirectoryHandler.HandleDirectoryRequest().")
                        .Append(ILoggerClient.CR).Append("Client handle ").Append(clientSession.ClientHandle)
                        .Append(ILoggerClient.CR).Append("Instance Name ").Append(m_ServerBaseImpl.InstanceName)
                        .Append(ILoggerClient.CR).Append("Error Id ").Append(error?.ErrorId)
                        .Append(ILoggerClient.CR).Append("Internal sysError ").Append(error?.SysError)
                        .Append(ILoggerClient.CR).Append("Error Location ").Append(errorInfo?.Location)
                        .Append(ILoggerClient.CR).Append("Error Text ").Append(error?.Text);

                    m_ServerBaseImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());

                    temp.Length = 0;
                }
            }

            m_ServerBaseImpl.GetDirectoryServiceStore().ReturnServiceToPool(m_directoryRefresh.ServiceList);
        }
    }
}
