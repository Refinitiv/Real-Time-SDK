/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Ema.Rdm;
using LSEG.Eta.Transports;

using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using static LSEG.Ema.Access.OmmState;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class OmmIProviderImpl : OmmServerBaseImpl, IOmmProviderImpl, IDirectoryServiceStoreClient
    {
        private OmmProvider m_OmmProvider;
        private OmmIProviderDirectoryStore m_OmmIProviderDirectoryStore;
        internal ItemWatchList m_ItemWatchList;
        private bool m_StoreUserSubmitted;
        private DirectoryMsg m_FanoutDirectoryMsg;
        private ServiceIdConverter m_serviceIdConverter;

        /* This is used to override to store user submitted directory from configuration for unit testing*/
        internal bool StoreUserSubmittedSource
        {
            set
            {
                m_StoreUserSubmitted = true;
            }
        }

        internal bool EnforceAckIDValidation { get; set; } = false;

        public OmmIProviderImpl(OmmProvider ommProvider, OmmIProviderConfigImpl configImpl, IOmmProviderClient client, object? closure = null) :
            base(configImpl, client, closure)
        {
            m_OmmProvider = ommProvider;
            m_OmmIProviderDirectoryStore = new OmmIProviderDirectoryStore(m_EmaObjectManager, this);
            m_OmmIProviderDirectoryStore.DirectoryServiceStoreClient = this;
            m_ItemWatchList = new ItemWatchList(ItemCallbackClient);
            m_StoreUserSubmitted = ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL;
            m_FanoutDirectoryMsg = new DirectoryMsg();
            m_serviceIdConverter = new ServiceIdConverter(m_OmmIProviderDirectoryStore);
            EnforceAckIDValidation = ConfigImpl.IProviderConfig.EnforceAckIDValidation;
        }

        public OmmIProviderImpl(OmmProvider ommProvider, OmmIProviderConfigImpl configImpl, IOmmProviderClient client,
            IOmmProviderErrorClient errorClient, object? closure = null)
            : base(configImpl, client, errorClient, closure)
        {
            m_OmmProvider = ommProvider;
            m_OmmIProviderDirectoryStore = new OmmIProviderDirectoryStore(m_EmaObjectManager, this);
            m_ItemWatchList = new ItemWatchList(ItemCallbackClient);
            m_StoreUserSubmitted = ConfigImpl.AdminControlDirectory == OmmIProviderConfig.AdminControlMode.API_CONTROL;
            m_FanoutDirectoryMsg = new DirectoryMsg();
            m_serviceIdConverter = new ServiceIdConverter(m_OmmIProviderDirectoryStore);
            EnforceAckIDValidation = ConfigImpl.IProviderConfig.EnforceAckIDValidation;
        }

        public string ProviderName => InstanceName;

        public override OmmProvider Provider => m_OmmProvider;

        public OmmProviderConfig.ProviderRoleEnum ProviderRole => OmmProviderConfig.ProviderRoleEnum.INTERACTIVE;

        public void CloseChannel(long clientHandle)
        {
            GetUserLocker().Enter();

            try
            {
                ClientSession? clientSession = ServerChannelHandler.GetClientSession(clientHandle);

                if (clientSession != null)
                {
                    m_ItemWatchList.ProcessCloseLogin(clientSession);
                    ServerChannelHandler.CloseChannel(clientSession.Channel());
                }
                else
                {
                    StringBuilder text = GetStrBuilder();
                    text
                        .Append("Invalid passed in client handle: ")
                        .Append(clientHandle)
                        .Append(" in the closeChannel() method.");

                    HandleInvalidUsage(text.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                }
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void ConnectedClientChannelInfo(List<ChannelInformation> clientInfoList)
        {
            clientInfoList.Clear();
            try
            {
                Eta.Transports.ChannelInfo etaChannelInfo = new Eta.Transports.ChannelInfo();
                StringBuilder componentInfo = new StringBuilder();

                GetUserLocker().Enter();
                foreach (ReactorChannel reactorChannel in ConnectedChannelList)
                {
                    IChannel? channel = reactorChannel.Channel;
                    if (channel is null)
                    {
                        continue;
                    }

                    componentInfo.Clear();
                    etaChannelInfo.Clear();

                    channel.Info(etaChannelInfo, out var _);

                    if (etaChannelInfo.ComponentInfoList is null
                        || etaChannelInfo.ComponentInfoList.Count == 0)
                    {
                        componentInfo.Append("unavailable");
                    }
                    else
                    {
                        foreach (ComponentInfo item in etaChannelInfo.ComponentInfoList)
                        {
                            componentInfo.Append(item.ComponentVersion.ToString());
                        }
                    }

                    ChannelInformation tmp =
                            new ChannelInformation()
                            {
                                ComponentInfo = componentInfo.ToString(),
                                Hostname = etaChannelInfo.ClientHostname,
                                IpAddress = etaChannelInfo.ClientIP,
                                ChannelState = Access.ChannelInformation.EtaToEmaChannelState(reactorChannel.Channel!.State),
                                ConnectionType = Access.ChannelInformation.EtaToEmaConnectionType(channel.ConnectionType),
                                ProtocolType = Access.ChannelInformation.EtaToEmaProtocolType(channel.ProtocolType),
                                EncryptedConnectionType =
                                    (Access.ChannelInformation.EtaToEmaConnectionType(channel.ConnectionType) == ConnectionType.ENCRYPTED)
                                        ? ConnectionType.SOCKET
                                        : ConnectionType.UNIDENTIFIED,
                                MajorVersion = channel.MajorVersion,
                                MinorVersion = channel.MinorVersion,
                                PingTimeout = channel.PingTimeOut,
                                MaxFragmentSize = etaChannelInfo.MaxFragmentSize,
                                MaxOutputBuffers = etaChannelInfo.MaxOutputBuffers,
                                GuaranteedOutputBuffers = etaChannelInfo.GuaranteedOutputBuffers,
                                NumInputBuffers = etaChannelInfo.NumInputBuffers,
                                SysSendBufSize = etaChannelInfo.SysSendBufSize,
                                SysRecvBufSize = etaChannelInfo.SysRecvBufSize,
                                CompressionType = Access.ChannelInformation.EtaToEmaCompressionType(etaChannelInfo.CompressionType),
                                CompressionThreshold = etaChannelInfo.CompressionThresHold
                            };

                    clientInfoList.Add(tmp);
                }
            }
            finally
            {
                GetUserLocker().Exit();
            }
            return;
        }

        public void ModifyIOCtl(IOCtlCode code, int val)
        {
            StringBuilder text = GetStrBuilder();

            text.Append("IProvider applications do not support the ModifyIOCtl(int code, int value) method.");

            HandleInvalidUsage(text.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }

        public void ModifyIOCtl(IOCtlCode code, int val, long handle)
        {
            GetUserLocker().Enter();

            try
            {
                if (m_Server is null)
                {
                    return;
                }

                ReactorReturnCode ret;
                Error? error = null;

                if (code == IOCtlCode.SERVER_NUM_POOL_BUFFERS)
                {
                    TransportReturnCode serverRet = m_Server.IOCtl((Eta.Transports.IOCtlCode)(int)code, val, out error);

                    if ((int)serverRet != val)
                    {
                        ret = ReactorReturnCode.FAILURE;
                    }
                    else
                    {
                        ret = ReactorReturnCode.SUCCESS;
                    }
                }
                else
                {
                    ItemInfo? itemInfo = GetItemInfo(handle);
                    ReactorChannel reactorChannel;

                    if (itemInfo is null
                        || itemInfo.ClientSession?.Channel() is null)
                    {
                        StringBuilder text = GetStrBuilder();
                        text.Append("Attempt to modify I/O option with non existent Handle = ")
                            .Append(handle).Append('.');

                        HandleInvalidUsage(text.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    reactorChannel = itemInfo!.ClientSession!.Channel();

                    ret = reactorChannel.IOCtl((Eta.Transports.IOCtlCode)(int)code, val, out var channelError);

                    if (channelError is not null)
                    {
                        error = channelError.Error;
                    }
                }

                if (ret != ReactorReturnCode.SUCCESS)
                {
                    StringBuilder text = GetStrBuilder();
                    text.Append("Failed to modify I/O option = ").Append(code)
                        .Append(". Reason: ").Append(ret.ToString())
                        .Append(". Error text: ").Append(error?.Text ?? "N/A");

                    HandleInvalidUsage(text.ToString(), OmmInvalidUsageException.ErrorCodes.FAILURE);
                    return;
                }

            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void OnServiceDelete(ClientSession? session, int serviceId)
        {
            if (session != null)
            {
                RemoveServiceId(session, serviceId);
                m_ItemWatchList.ProcessServiceDelete(session, serviceId);
            }
            else
            {
                if (DirectoryHandler != null)
                {
                    List<ItemInfo> itemInfoList = DirectoryHandler.ItemInfoList;
                    var enumerator = itemInfoList.GetEnumerator();
                    while (enumerator.MoveNext())
                    {
                        var clientSession = enumerator.Current.ClientSession!;
                        RemoveServiceId(clientSession, serviceId);
                        m_ItemWatchList.ProcessServiceDelete(clientSession, serviceId);
                    }
                }
            }
        }

        public void OnServiceGroupChange(ClientSession? session, int serviceId, IList<ServiceGroup> serviceGroupList)
        {
            foreach (var serviceGroup in serviceGroupList)
            {
                if (serviceGroup.HasMergedToGroup)
                {
                    if (session != null)
                    {
                        MergeToGroupId(session, serviceId, serviceGroup.Group, serviceGroup.MergedToGroup);
                    }
                    else
                    {
                        if (DirectoryHandler != null)
                        {
                            List<ItemInfo> itemInfoList = DirectoryHandler.ItemInfoList;

                            foreach (var item in itemInfoList)
                            {
                                MergeToGroupId(item.ClientSession!, serviceId, serviceGroup.Group, serviceGroup.MergedToGroup);
                            }
                        }
                    }
                }

                if (serviceGroup.HasStatus)
                {
                    if (serviceGroup.Status.StreamState() == OmmState.StreamStates.CLOSED_RECOVER)
                    {
                        if (session != null)
                        {
                            RemoveGroupId(session, serviceId, serviceGroup.Group);
                        }
                        else
                        {
                            if (DirectoryHandler != null)
                            {
                                List<ItemInfo> itemInfoList = DirectoryHandler.ItemInfoList;
                                foreach (var item in itemInfoList)
                                {
                                    RemoveGroupId(item.ClientSession!, serviceId, serviceGroup.Group);
                                }
                            }
                        }
                    }
                }
            }
        }

        public void OnServiceStateChange(ClientSession? session, int serviceId, ServiceState state)
        {
            if (state.HasStatus)
            {
                if (state.Status.StreamState() == OmmState.StreamStates.CLOSED_RECOVER)
                {
                    if (session != null)
                    {
                        RemoveServiceId(session, serviceId);
                    }
                    else
                    {
                        if (DirectoryHandler != null)
                        {
                            List<ItemInfo> itemInfoList = DirectoryHandler.ItemInfoList;
                            foreach (var item in itemInfoList)
                            {
                                RemoveServiceId(item.ClientSession!, serviceId);
                            }
                        }
                    }
                }
            }
        }

        void RemoveServiceId(ClientSession clientSession, int serviceId)
        {
            GetUserLocker().Enter();

            clientSession.ServiceGroupIdToItemInfoMap.TryGetValue(serviceId, out var groupIdToItemInfoList);
            if (groupIdToItemInfoList != null)
            {
                var enumerator = groupIdToItemInfoList.Values.GetEnumerator();
                List<ItemInfo> itemInfoList;

                while (enumerator.MoveNext())
                {
                    itemInfoList = enumerator.Current;
                    itemInfoList.Clear();
                }

                groupIdToItemInfoList.Clear();
                clientSession.ServiceGroupIdToItemInfoMap.Remove(serviceId);
            }

            var itemInfoEnumer = clientSession.ItemInfoList().GetEnumerator();

            while (itemInfoEnumer.MoveNext())
            {
                ItemInfo itemInfo = itemInfoEnumer.Current;
                if (itemInfo.MsgKey.CheckHasServiceId() && itemInfo.ServiceId == serviceId)
                {
                    if (itemInfo.DomainType > 5)
                    {
                        RemoveItemInfo(itemInfo, false);
                    }
                }
            }

            GetUserLocker().Exit();
        }

        void RemoveGroupId(ClientSession clientSession, int serviceId, Buffer groupId)
        {
            GetUserLocker().Enter();

            try
            {
                clientSession.ServiceGroupIdToItemInfoMap.TryGetValue(serviceId, out var groupIdToItemInfoList);
                if (groupIdToItemInfoList != null)
                {
                    List<ItemInfo> itemInfoList = groupIdToItemInfoList[groupId];
                    if (itemInfoList != null)
                    {
                        foreach (var item in itemInfoList)
                        {
                            RemoveItemInfo(item, false);
                        }

                        itemInfoList.Clear();

                        groupIdToItemInfoList.Remove(groupId);
                    }
                }
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        void MergeToGroupId(ClientSession clientSession, int serviceId, Buffer groupId, Buffer newGroupId)
        {
            GetUserLocker().Enter();

            try
            {
                if (groupId.Equals(newGroupId))
                {
                    return;
                }

                clientSession.ServiceGroupIdToItemInfoMap.TryGetValue(serviceId, out var groupIdToItemInfoList);

                if (groupIdToItemInfoList != null)
                {
                    List<ItemInfo> oldItemInfoList = groupIdToItemInfoList[groupId];
                    if (oldItemInfoList != null)
                    {
                        List<ItemInfo> mergeItemInfoList = groupIdToItemInfoList[newGroupId];

                        if (mergeItemInfoList != null)
                        {
                            mergeItemInfoList.AddRange(oldItemInfoList);
                            oldItemInfoList.Clear();
                            groupIdToItemInfoList.Remove(groupId);
                        }
                        else
                        {
                            mergeItemInfoList = new List<ItemInfo>(oldItemInfoList.Count + 100);
                            mergeItemInfoList.AddRange(oldItemInfoList);
                            oldItemInfoList.Clear();
                            groupIdToItemInfoList.Remove(groupId);
                            groupIdToItemInfoList.TryAdd(newGroupId, mergeItemInfoList);
                        }
                    }
                }
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public long RegisterClient(RequestMsg reqMsg, IOmmProviderClient client, object? closure)
        {
            if (CheckClient(client))
                return 0;

            GetUserLocker().Enter();

            try
            {
                if (reqMsg.DomainType() != EmaRdm.MMT_DICTIONARY)
                {
                    HandleInvalidUsage("OMM Interactive provider supports registering DICTIONARY domain type only.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return 0;
                }

                if (ServerChannelHandler.m_ClientSessionDict.Count == 0)
                {
                    HandleInvalidUsage("There is no active client session available for registering.",
                        OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                    return 0;
                }

                return ItemCallbackClient.RegisterClient(reqMsg, client, closure);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Reissue(RequestMsg reqMsg, long handle)
        {
            GetUserLocker().Enter();

            try
            {
                if (reqMsg.DomainType() != EmaRdm.MMT_DICTIONARY)
                {
                    HandleInvalidUsage("OMM Interactive provider supports reissuing DICTIONARY domain type only.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }

                ItemCallbackClient.Reissue(reqMsg, handle);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Submit(GenericMsg genericMsg, long handle)
        {
            GetUserLocker().Enter();

            try
            {
                ItemInfo? itemInfo = GetItemInfo(handle);

                if (itemInfo == null)
                {
                    var strBuilder = GetStrBuilder();
                    strBuilder.Append("Attempt to submit GenericMsg with non existent Handle = ").Append(handle).Append(".");
                    HandleInvalidHandle(handle, strBuilder.ToString());
                    return;
                }

                if (itemInfo.DomainType == EmaRdm.MMT_DICTIONARY)
                {
                    var strBuilder = GetStrBuilder();
                    strBuilder.Append("Attempt to submit GenericMsg with Dictionary domain while this is not supported.").Append(handle).Append(".");
                    HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }

                genericMsg.m_rsslMsg.StreamId = itemInfo.StreamId;
                if (genericMsg.m_rsslMsg.DomainType == 0)
                {
                    genericMsg.m_rsslMsg.DomainType = itemInfo.DomainType;
                }

                Submit(genericMsg, itemInfo.ClientSession!);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Submit(RefreshMsg refreshMsg, long handle)
        {
            GetUserLocker().Enter();

            try
            {
                RefreshMsg refreshMsgImpl = refreshMsg;
                Eta.Codec.IRefreshMsg rsslRefreshMsg = refreshMsgImpl.m_rsslMsg;

                ItemInfo? itemInfo = GetItemInfo(handle);

                if ((itemInfo == null) && (handle != 0))
                {
                    StringBuilder temp = GetStrBuilder();
                    temp.Append("Attempt to submit RefreshMsg with non existent Handle = ").Append(handle).Append('.');

                    HandleInvalidHandle(handle, temp.ToString());
                    return;
                }

                ClientSession? clientSession;

                if (refreshMsgImpl.DomainType() == EmaRdm.MMT_LOGIN)
                {
                    if (handle == 0 || itemInfo == null)
                    {
                        Submit(refreshMsgImpl, LoginHandler.ItemInfoList, "Fanout login message for item handle = ", false);
                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        refreshMsgImpl.m_rsslMsg.StreamId = itemInfo.StreamId;

                        if ((rsslRefreshMsg.State.StreamState() == OmmState.StreamStates.OPEN) && (rsslRefreshMsg.State.DataState() == OmmState.DataStates.OK))
                        {
                            itemInfo.ClientSession!.IsLogin = true;
                        }
                    }
                }
                else if (refreshMsgImpl.DomainType() == EmaRdm.MMT_DIRECTORY)
                {
                    if (refreshMsgImpl.m_rsslMsg.ContainerType != Eta.Codec.DataTypes.MAP)
                    {
                        StringBuilder text = GetStrBuilder();
                        text.Append("Attempt to submit RefreshMsg with directory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
                                .Append(DataType.AsString(refreshMsgImpl.Payload().DataType));
                        HandleInvalidUsage(text.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    int errorCode;
                    var errorText = GetStrBuilder();
                    if (!m_OmmIProviderDirectoryStore.DecodeSourceDirectory(refreshMsgImpl.m_rsslMsg, errorText, out errorCode))
                    {
                        HandleInvalidUsage(errorText.ToString(), errorCode);
                        return;
                    }

                    clientSession = handle != 0 ? itemInfo!.ClientSession : null;

                    if (!m_OmmIProviderDirectoryStore.SubmitSourceDirectory(clientSession, refreshMsgImpl.m_rsslMsg, GetStrBuilder(), m_StoreUserSubmitted, out errorCode))
                    {
                        StringBuilder text = new StringBuilder();
                        text.Append("Attempt to submit invalid directory domain message.")
                            .Append(ILoggerClient.CR)
                            .Append("Reason = ")
                            .Append(m_StringBuilder);
                        HandleInvalidUsage(text.ToString(), errorCode);
                        return;
                    }

                    if (handle == 0 || itemInfo == null)
                    {
                        Submit(refreshMsgImpl, DirectoryHandler.ItemInfoList, "Fanout directory message for item handle = ", true);
                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        refreshMsgImpl.m_rsslMsg.StreamId = itemInfo.StreamId;
                    }
                }
                else if (refreshMsgImpl.DomainType() == EmaRdm.MMT_DICTIONARY)
                {
                    if (refreshMsgImpl.m_rsslMsg.ContainerType != Eta.Codec.DataTypes.SERIES)
                    {
                        StringBuilder text = GetStrBuilder();

                        HandleInvalidUsage(text.Append("Attempt to submit RefreshMsg with dictionary domain using container with wrong data type. Expected container data type is Series. Passed in is ")
                            .Append(DataType.AsString(refreshMsgImpl.Payload().DataType)).ToString(),
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                        return;
                    }

                    ServiceIdConversionError encodingError = m_serviceIdConverter.EncodeServiceId(refreshMsgImpl, Eta.Codec.RefreshMsgFlags.HAS_MSG_KEY);
                    if (encodingError != ServiceIdConversionError.NONE)
                    {
                        HandleServiceIdConversionError(encodingError, refreshMsgImpl);
                        return;
                    }

                    if (handle == 0 || itemInfo == null)
                    {
                        StringBuilder text = GetStrBuilder();
                        text.Append("Fanout dictionary message for item handle = ");

                        if (!Submit(refreshMsgImpl, DictionaryHandler.ItemInfoList, text.ToString(), false))
                        {
                            return;
                        }

                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        refreshMsgImpl.m_rsslMsg.StreamId = (int)itemInfo.StreamId;
                    }
                }
                else
                {
                    if (handle == 0 || itemInfo == null )
                    {
                        StringBuilder temp = GetStrBuilder();
                        temp.Append("Attempt to fanout RefreshMsg with domain type ")
                            .Append(Utilities.RdmDomainAsString(refreshMsgImpl.DomainType()))
                            .Append('.');

                        HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    if (GetLoggerClient().IsTraceEnabled)
                    {
                        StringBuilder text = GetStrBuilder();
                        text.Append("Received RefreshMsg with domain type ")
                            .Append(Utilities.RdmDomainAsString(refreshMsgImpl.DomainType()))
                            .Append("; handle = ")
                            .Append(handle)
                            .Append(", user assigned streamId = ")
                            .Append(refreshMsgImpl.StreamId())
                            .Append(".");

                        GetLoggerClient().Trace(InstanceName, text.ToString());
                    }

                    ServiceIdConversionError encodingError = m_serviceIdConverter.EncodeServiceId(refreshMsgImpl, Eta.Codec.RefreshMsgFlags.HAS_MSG_KEY);
                    if (encodingError != ServiceIdConversionError.NONE)
                    {
                        HandleServiceIdConversionError(encodingError, refreshMsgImpl);
                        return;
                    }

                    clientSession = itemInfo.ClientSession;
                    refreshMsgImpl.m_rsslMsg.StreamId = itemInfo.StreamId;

                    HandleItemGroup(itemInfo, rsslRefreshMsg.GroupId, rsslRefreshMsg.State);

                    if (itemInfo.IsPrivateStream)
                    {
                        refreshMsgImpl.m_rsslMsg.ApplyPrivateStream();
                    }
                }

                if (!Submit(refreshMsgImpl, clientSession!))
                {
                    return;
                }

                itemInfo.SentRefresh = true;
                HandleItemInfo(rsslRefreshMsg.DomainType, handle, rsslRefreshMsg.State, rsslRefreshMsg.CheckRefreshComplete());
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Submit(UpdateMsg updateMsg, long handle)
        {
            GetUserLocker().Enter();
            try
            {
                ItemInfo? itemInfo = GetItemInfo(handle);

                if (itemInfo == null && handle != 0)
                {
                    m_StringBuilder.Length = 0;
                    m_StringBuilder.Append("Attempt to submit UpdateMsg with non existent Handle = ")
                        .Append(handle)
                        .Append(".");

                    HandleInvalidHandle(handle, m_StringBuilder.ToString());
                    return;
                }

                ClientSession? clientSession = null;

                if (updateMsg.DomainType() == EmaRdm.MMT_LOGIN)
                {
                    HandleInvalidUsage("Attempt to submit UpdateMsg with login domain while this is not supported.",
                        OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }
                else if (updateMsg.DomainType() == EmaRdm.MMT_DIRECTORY)
                {
                    if (ConfigImpl.IProviderConfig.RefreshFirstRequired)
                    {
                        if (itemInfo != null && !itemInfo.SentRefresh)
                        {
                            m_StringBuilder.Length = 0;
                            m_StringBuilder.Append("Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = ").
                                Append(itemInfo.Handle);
                            HandleInvalidUsage(m_StringBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                            return;
                        }
                    }

                    if (updateMsg.m_rsslMsg.ContainerType != Eta.Codec.DataTypes.MAP)
                    {
                        m_StringBuilder.Length = 0;
                        HandleInvalidUsage(m_StringBuilder.Append("Attempt to submit UpdateMsg with directory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
                            .Append(DataType.AsString(updateMsg.Payload().DataType)).ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    if (!m_OmmIProviderDirectoryStore.DecodeSourceDirectory(updateMsg.m_rsslMsg, GetStrBuilder(), out var intCode))
                    {
                        HandleInvalidUsage(m_StringBuilder.ToString(), intCode);
                        return;
                    }

                    clientSession = handle != 0 ? itemInfo!.ClientSession : null;

                    if (!m_OmmIProviderDirectoryStore.SubmitSourceDirectory(clientSession, updateMsg.m_rsslMsg, GetStrBuilder(), m_StoreUserSubmitted, out intCode))
                    {
                        var tmp = new StringBuilder();
                        tmp.Append("Attempt to submit invalid directory domain message.")
                            .Append(ILoggerClient.CR)
                            .Append("Reason = ")
                            .Append(m_StringBuilder);
                        HandleInvalidUsage(tmp.ToString(), intCode);
                        return;
                    }

                    if (handle == 0 || itemInfo == null )
                    {
                        Submit(updateMsg, DirectoryHandler.ItemInfoList, "Fanout directory message for item handle = ", true);
                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        updateMsg.m_rsslMsg.StreamId = itemInfo.StreamId;
                    }
                }
                else if (updateMsg.DomainType() == EmaRdm.MMT_DICTIONARY)
                {
                    HandleInvalidUsage("Attempt to submit UpdateMsg with dictionary domain while this is not supported.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }
                else
                {
                    if (handle == 0 || itemInfo == null)
                    {
                        StringBuilder temp = GetStrBuilder();
                        temp.Append("Attempt to fanout UpdateMsg with domain type ")
                            .Append(Utilities.RdmDomainAsString(updateMsg.DomainType()))
                            .Append(".");
                        HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    if (GetLoggerClient().IsTraceEnabled)
                    {
                        StringBuilder text = GetStrBuilder();
                        text.Append("Received UpdateMsg with domain type ")
                        .Append(Utilities.RdmDomainAsString(updateMsg.DomainType()))
                        .Append("; handle = ")
                        .Append(handle)
                        .Append(", user assigned streamId = ")
                        .Append(updateMsg.StreamId()).Append(".");

                        GetLoggerClient().Trace(InstanceName, text.ToString());
                    }

                    ServiceIdConversionError encodingError = m_serviceIdConverter.EncodeServiceId(updateMsg, UpdateMsgFlags.HAS_MSG_KEY);
                    if (encodingError != ServiceIdConversionError.NONE)
                    {
                        HandleServiceIdConversionError(encodingError, updateMsg);
                        return;
                    }

                    if (ConfigImpl.IProviderConfig.RefreshFirstRequired && !itemInfo.SentRefresh)
                    {
                        StringBuilder temp = GetStrBuilder();
                        temp.Append("Attempt to submit UpdateMsg while RefreshMsg was not submitted on this stream yet. Handle = ");
                        temp.Append(itemInfo.Handle);
                        HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
                        return;
                    }

                    clientSession = itemInfo.ClientSession;
                    updateMsg.m_rsslMsg.StreamId = itemInfo.StreamId;
                }

                Submit(updateMsg, clientSession!);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Submit(StatusMsg statusMsg, long handle)
        {
            GetUserLocker().Enter();

            try
            {
                ItemInfo? itemInfo = GetItemInfo(handle);

                if (itemInfo == null && handle != 0)
                {
                    StringBuilder temp = GetStrBuilder();
                    temp.Append("Attempt to submit StatusMsg with non existent Handle = ")
                        .Append(handle)
                        .Append(".");

                    HandleInvalidHandle(handle, temp.ToString());
                    return;
                }

                ClientSession? clientSession;

                if (statusMsg.DomainType() == EmaRdm.MMT_LOGIN)
                {
                    if (handle == 0 || itemInfo == null)
                    {
                        Submit(statusMsg, LoginHandler.ItemInfoList, "Fanout login message for item handle = ", false);
                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        statusMsg.m_rsslMsg.StreamId = itemInfo.StreamId;
                    }
                }
                else if (statusMsg.DomainType() == EmaRdm.MMT_DIRECTORY)
                {
                    if (handle == 0 || itemInfo == null)
                    {
                        Submit(statusMsg, DirectoryHandler.ItemInfoList, "Fanout directory message for item handle = ", false);
                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        statusMsg.m_rsslMsg.StreamId = itemInfo.StreamId;
                    }
                }
                else if (statusMsg.DomainType() == EmaRdm.MMT_DICTIONARY)
                {
                    ServiceIdConversionError encodingError = m_serviceIdConverter.EncodeServiceId(statusMsg, StatusMsgFlags.HAS_MSG_KEY);
                    if (encodingError != ServiceIdConversionError.NONE)
                    {
                        HandleServiceIdConversionError(encodingError, statusMsg);
                        return;
                    }

                    if (handle == 0 || itemInfo == null)
                    {
                        Submit(statusMsg, DictionaryHandler.ItemInfoList, "Fanout dictionary message for item handle = ", false);
                        return;
                    }
                    else
                    {
                        clientSession = itemInfo.ClientSession;
                        statusMsg.m_rsslMsg.StreamId = itemInfo.StreamId;
                    }
                }
                else
                {
                    if (handle == 0 || itemInfo == null)
                    {
                        StringBuilder temp = GetStrBuilder();
                        temp.Append("Attempt to fanout StatusMsg with domain type ")
                            .Append(Utilities.RdmDomainAsString(statusMsg.DomainType()))
                            .Append(".");
                        HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    if (GetLoggerClient().IsTraceEnabled)
                    {
                        StringBuilder text = GetStrBuilder();
                        text.Append("Received StatusMsg with domain type ")
                            .Append(Utilities.RdmDomainAsString(statusMsg.DomainType()))
                            .Append("; handle = ")
                            .Append(handle)
                            .Append(", user assigned streamId = ")
                            .Append(statusMsg.StreamId())
                            .Append(".");

                        GetLoggerClient().Trace(InstanceName, text.ToString());
                    }

                    ServiceIdConversionError encodingError = m_serviceIdConverter.EncodeServiceId(statusMsg, StatusMsgFlags.HAS_MSG_KEY);
                    if (encodingError != ServiceIdConversionError.NONE)
                    {
                        HandleServiceIdConversionError(encodingError, statusMsg);
                        return;
                    }

                    clientSession = itemInfo.ClientSession;
                    statusMsg.m_rsslMsg.StreamId = itemInfo.StreamId;

                    var rsslStatusMsg = statusMsg.m_rsslMsg;

                    if (rsslStatusMsg.CheckHasGroupId())
                    {
                        HandleItemGroup(itemInfo, rsslStatusMsg.GroupId, rsslStatusMsg.State);
                    }

                    if (itemInfo.IsPrivateStream)
                    {
                        statusMsg.m_rsslMsg.ApplyPrivateStream();
                    }
                }

                if (!Submit(statusMsg, clientSession!))
                {
                    return;
                }

                HandleItemInfo(statusMsg.DomainType(), handle, statusMsg.m_rsslMsg.State, false);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Submit(AckMsg ackMsg, long handle)
        {
            GetUserLocker().Enter();

            try
            {
                ItemInfo? itemInfo = GetItemInfo(handle);

                if (itemInfo == null)
                {
                    var strBuilder = GetStrBuilder();
                    strBuilder.Append($"Attempt to submit AckMsg with non existent Handle = {handle}.");

                    HandleInvalidHandle(handle, strBuilder.ToString());
                    return;
                }
                if (EnforceAckIDValidation && !RemovePostId(ackMsg, itemInfo))
                {
                    return;
                }

                ServiceIdConversionError encodingResult = m_serviceIdConverter.EncodeServiceId(ackMsg, AckMsgFlags.HAS_MSG_KEY);
                if (encodingResult != ServiceIdConversionError.NONE)
                {
                    LogServiceIdConversionError(encodingResult, ackMsg);
                }

                ackMsg.StreamId(itemInfo.StreamId);
                Submit(ackMsg, itemInfo.ClientSession!);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        public void Unregister(long handle)
        {
            GetUserLocker().Enter();

            try
            {
                ItemCallbackClient.UnregisterClient(handle);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        internal override DirectoryServiceStore GetDirectoryServiceStore()
        {
            return m_OmmIProviderDirectoryStore;
        }

        internal override void ProcessChannelEvent(ReactorChannelEvent evt)
        {
            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_DOWN:
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:

                    if (m_ItemWatchList != null)
                    {
                        m_ItemWatchList.ProcessChannelEvent(evt);
                    }
                    break;
                default:
                    break;
            }
        }

        internal bool Submit(Msg msgImpl, List<ItemInfo> itemInfoList, string logText, bool applyDirectoryFilter)
        {
            ItemInfo itemInfo;
	    int itemInfoCount;

            for (int index = 0; index < itemInfoList.Count;)
            {
                itemInfo = itemInfoList[index];
		itemInfoCount = itemInfoList.Count;

                if (GetLoggerClient().IsTraceEnabled)
                {
                    StringBuilder text = GetStrBuilder();
                    text.Append(logText).Append(itemInfo.Handle)
                        .Append(", client handle = ")
                        .Append(itemInfo.ClientSession!.ClientHandle).Append(".");

                    GetLoggerClient().Trace(InstanceName, text.ToString());
                }

                msgImpl.m_rsslMsg.StreamId = itemInfo.StreamId;

                if (applyDirectoryFilter)
                {
                    MsgBase? rdmMsgBase = null;
                    DirectoryMsg directoryMsg = m_OmmIProviderDirectoryStore.SubmittedDirectoryMsg;

                    switch (directoryMsg.DirectoryMsgType)
                    {
                        case DirectoryMsgType.REFRESH:
                            {
                                DirectoryRefresh directoryRefresh = directoryMsg.DirectoryRefresh!;

                                m_FanoutDirectoryMsg.DirectoryMsgType = DirectoryMsgType.REFRESH;

                                DirectoryRefresh fanoutDirectoryRefresh = m_FanoutDirectoryMsg.DirectoryRefresh!;
                                fanoutDirectoryRefresh.Clear();

                                long filter = m_OmmIProviderDirectoryStore.EncodeDirectoryMsg(directoryRefresh.ServiceList,
                                    fanoutDirectoryRefresh.ServiceList,
                                    itemInfo.MsgKey.Filter,
                                    false,
                                    itemInfo.MsgKey.CheckHasServiceId(),
                                    itemInfo.MsgKey.ServiceId);

                                fanoutDirectoryRefresh.Filter = filter;
                                fanoutDirectoryRefresh.StreamId = itemInfo.StreamId;
                                fanoutDirectoryRefresh.State = directoryRefresh.State;

                                if (directoryRefresh.ClearCache)
                                {
                                    fanoutDirectoryRefresh.ClearCache = true;
                                }

                                if (directoryRefresh.HasSequenceNumber)
                                {
                                    fanoutDirectoryRefresh.HasSequenceNumber = true;
                                    fanoutDirectoryRefresh.SequenceNumber = directoryRefresh.SequenceNumber;
                                }

                                if (directoryRefresh.HasServiceId)
                                {
                                    fanoutDirectoryRefresh.HasServiceId = true;
                                    fanoutDirectoryRefresh.ServiceId = directoryRefresh.ServiceId;
                                }

                                if (directoryRefresh.Solicited)
                                {
                                    fanoutDirectoryRefresh.Solicited= true;
                                }

                                rdmMsgBase = fanoutDirectoryRefresh;
                            }
                            break;
                        case DirectoryMsgType.UPDATE:
                            {
                                if (!m_StoreUserSubmitted && ConfigImpl.IProviderConfig.RefreshFirstRequired)
                                {
                                    if (!itemInfo.SentRefresh)
                                    {
                                        if (GetLoggerClient().IsWarnEnabled)
                                        {
                                            StringBuilder text = GetStrBuilder();
                                            text.Append("Skip sending source directory update message for handle ")
                                                .Append(itemInfo.Handle).Append(", client handle ")
                                                .Append(itemInfo.ClientSession!.ClientHandle).Append(" as refresh message is required first.");

                                            GetLoggerClient().Warn(InstanceName, text.ToString());
                                        }

                                        continue;
                                    }
                                }

                                DirectoryUpdate directoryUpdate = directoryMsg.DirectoryUpdate!;

                                m_FanoutDirectoryMsg.DirectoryMsgType = DirectoryMsgType.UPDATE;

                                DirectoryUpdate fanoutDirectoryUpdate = m_FanoutDirectoryMsg.DirectoryUpdate!;
                                fanoutDirectoryUpdate.Clear();

                                long filter = m_OmmIProviderDirectoryStore.EncodeDirectoryMsg(directoryUpdate.ServiceList,
                                    fanoutDirectoryUpdate.ServiceList,
                                    itemInfo.MsgKey.Filter,
                                    false,
                                    itemInfo.MsgKey.CheckHasServiceId(),
                                    itemInfo.MsgKey.ServiceId);

                                fanoutDirectoryUpdate.StreamId = itemInfo.StreamId;

                                if (directoryUpdate.HasFilter && (filter != 0))
                                {
                                    fanoutDirectoryUpdate.HasFilter = true;
                                    fanoutDirectoryUpdate.Filter = filter;
                                }

                                if (directoryUpdate.HasSequenceNumber)
                                {
                                    fanoutDirectoryUpdate.HasSequenceNumber = true;
                                    fanoutDirectoryUpdate.SequenceNumber = directoryUpdate.SequenceNumber;
                                }

                                if (directoryUpdate.HasServiceId)
                                {
                                    fanoutDirectoryUpdate.HasServiceId = true;
                                    fanoutDirectoryUpdate.ServiceId = directoryUpdate.ServiceId;
                                }

                                rdmMsgBase = fanoutDirectoryUpdate;
                            }
                            break;
                        default:
                            break;
                    }

                    if (rdmMsgBase != null)
                    {
                        ReactorReturnCode ret = itemInfo.ClientSession!.Channel().Submit(rdmMsgBase, GetSubmitOptions(), out m_ReactorErrorInfo);

                        if (ReactorReturnCode.SUCCESS > ret)
                        {
                            if (GetLoggerClient().IsErrorEnabled)
                            {
                                Eta.Transports.Error error = m_ReactorErrorInfo!.Error;

                                GetStrBuilder().Append("Internal error: ReactorChannel.Submit() failed in OmmProviderImpl.Submit(")
                                    .Append(DataType.AsString(msgImpl.DataType)).Append(")").Append(ILoggerClient.CR)
                                    .Append("Client handle ").Append(itemInfo.ClientSession.ClientHandle).Append(ILoggerClient.CR)
                                    .Append("Error Id ").Append(error.ErrorId).Append(ILoggerClient.CR)
                                    .Append("Internal sysError ").Append(error.SysError).Append(ILoggerClient.CR)
                                    .Append("Error Location ").Append(m_ReactorErrorInfo.Location).Append(ILoggerClient.CR)
                                    .Append("Error Text ").Append(error.Text);

                                GetLoggerClient().Error(InstanceName, m_StringBuilder.ToString());
                            }
                            var failText = GetStrBuilder();
                            failText.Append("Failed to submit ")
                                .Append(DataType.AsString(msgImpl.DataType)).Append(". Reason: ")
                                .Append(ret)
                                .Append(". Error text: ")
                                .Append(m_ReactorErrorInfo!.Error.Text);

                            HandleInvalidUsage(failText.ToString(), (int)ret);
                            return false;
                        }
                    }
                }
                else
                {
                    if (!m_StoreUserSubmitted && (msgImpl.m_rsslMsg.MsgClass == MsgClasses.UPDATE) && ConfigImpl.IProviderConfig.RefreshFirstRequired)
                    {
                        if (!itemInfo.SentRefresh)
                        {
                            if (GetLoggerClient().IsWarnEnabled)
                            {
                                StringBuilder warnText = GetStrBuilder();
                                warnText.Append("Skip sending update message for handle ")
                                    .Append(itemInfo.Handle)
                                    .Append(", client handle ")
                                    .Append(itemInfo.ClientSession!.ClientHandle).Append(" as refresh message is required first.");

                                GetLoggerClient().Warn(InstanceName, warnText.ToString());
                            }

                            continue;
                        }
                    }

                    if (!Submit(msgImpl, itemInfo.ClientSession!))
                    {
                        return false;
                    }
                }

                switch (msgImpl.m_rsslMsg.MsgClass)
                {
                    case MsgClasses.REFRESH:
                        itemInfo.SentRefresh = true;
                        HandleItemInfo(msgImpl.m_rsslMsg.DomainType, itemInfo.Handle, msgImpl.m_rsslMsg.State, msgImpl.m_rsslMsg.CheckRefreshComplete());
                        break;
                    case MsgClasses.STATUS:
                        HandleItemInfo(msgImpl.m_rsslMsg.DomainType, itemInfo.Handle, msgImpl.m_rsslMsg.State, false);
                        break;
                    default:
                        break;
                }
		index += (itemInfoList.Count == itemInfoCount) ? 1 : 0;
            }

            return true;
        }

        internal bool Submit(Msg msgImpl, ClientSession clientSession)
        {
            ReactorReturnCode ret;
            if (ReactorReturnCode.SUCCESS > (ret = clientSession.Channel().Submit(msgImpl.m_rsslMsg, GetSubmitOptions(), out m_ReactorErrorInfo)))
            {
                if (GetLoggerClient().IsErrorEnabled)
                {
                    Eta.Transports.Error error = m_ReactorErrorInfo!.Error;

                    StringBuilder errorLogText = GetStrBuilder();
                    errorLogText.Append("Internal error: rsslChannel.Submit() failed in OmmProviderImpl.Submit(")
                        .Append(DataType.AsString(msgImpl.DataType)).Append(")").Append(ILoggerClient.CR)
                        .Append("Client handle ").Append(clientSession.ClientHandle).Append(ILoggerClient.CR)
                        .Append("Error Id ").Append(error.ErrorId).Append(ILoggerClient.CR)
                        .Append("Internal sysError ").Append(error.SysError).Append(ILoggerClient.CR)
                        .Append("Error Location ").Append(m_ReactorErrorInfo.Location).Append(ILoggerClient.CR)
                        .Append("Error Text ").Append(error.Text);

                    GetLoggerClient().Error(InstanceName, errorLogText.ToString());
                }

                StringBuilder submitLogText = GetStrBuilder();
                submitLogText.Append("Failed to submit ")
                    .Append(DataType.AsString(msgImpl.DataType)).Append(". Reason: ")
                    .Append(ret)
                    .Append(". Error text: ")
                    .Append(m_ReactorErrorInfo!.Error.Text);

                HandleInvalidUsage(submitLogText.ToString(), (int)ret);
                return false;
            }

            return true;
        }

        void HandleItemGroup(ItemInfo itemInfo, Buffer groupId, State state)
        {
            if ((groupId.Length < 2)
                || (groupId.Data().ReadByteAt(0) == 0 && groupId.Data().ReadByteAt(0) == 0)
                || !itemInfo.MsgKey.CheckHasServiceId())
            {
                return;
            }

            if (itemInfo.HasItemGroup)
            {
                if (!groupId.Equals(itemInfo.ItemGroup))
                {
                    UpdateItemGroup(itemInfo, groupId);
                    itemInfo.ItemGroup = groupId;
                }
            }
            else
            {
                itemInfo.ItemGroup = groupId;
                AddItemGroup(itemInfo, groupId);
            }
        }

        internal void HandleItemInfo(int domainType, long handle, State state, bool refreshComplete)
        {
            if ((state.StreamState() == OmmState.StreamStates.CLOSED)
                || (state.StreamState() == OmmState.StreamStates.CLOSED_RECOVER)
                || (state.StreamState() == OmmState.StreamStates.CLOSED_REDIRECTED)
                || (state.StreamState() == OmmState.StreamStates.NON_STREAMING && refreshComplete))
            {
                ItemInfo? itemInfo = GetItemInfo(handle);

                if (itemInfo != null)
                {
                    switch (domainType)
                    {
                        case EmaRdm.MMT_LOGIN:
                            {
                                ServerChannelHandler.CloseChannel(itemInfo.ClientSession!.Channel());
                                m_ItemWatchList.ProcessCloseLogin(itemInfo.ClientSession);
                            }
                            break;
                        case EmaRdm.MMT_DIRECTORY:
                            {
                                DirectoryHandler.ItemInfoList.Remove(itemInfo);
                                RemoveItemInfo(itemInfo, false);
                            }
                            break;
                        case EmaRdm.MMT_DICTIONARY:
                            {
                                DictionaryHandler.ItemInfoList.Remove(itemInfo);
                                RemoveItemInfo(itemInfo, false);
                            }
                            break;
                        default:
                            {
                                RemoveItemInfo(itemInfo, true);
                            }
                            break;
                    }
                }
            }
        }

        private void HandleServiceIdConversionError(ServiceIdConversionError encodingResult, Msg msg)
        {
            IMsg rsslMsg = msg.m_rsslMsg;
            GetUserLocker().Enter();

            string errorMessage;
            try
            {
                if (encodingResult == ServiceIdConversionError.ID_IS_MISSING_FOR_NAME)
                {
                    errorMessage = GetStrBuilder().Append("Attempt to submit ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service name of ").Append(msg.ServiceName())
                        .Append(" that was not included in the SourceDirectory. Dropping this ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(".").ToString();
                }
                else if (encodingResult == ServiceIdConversionError.ID_IS_INVALID_FOR_NAME)
                {
                    m_OmmIProviderDirectoryStore.GetServiceIdByName(msg.ServiceName(), out var serviceId);
                    errorMessage = GetStrBuilder().Append("Attempt to submit ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service name of ")
                        .Append(msg.ServiceName())
                        .Append(" whose matching service id of ")
                        .Append(serviceId)
                        .Append(" is out of range. Dropping this ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(".").ToString();
                }
                else if (encodingResult == ServiceIdConversionError.NAME_IS_MISSING_FOR_ID)
                {
                    errorMessage = GetStrBuilder().Append("Attempt to submit ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service Id of ").Append(msg.ServiceId())
                        .Append(" that was not included in the SourceDirectory. Dropping this ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(".").ToString();
                }
                else if (encodingResult == ServiceIdConversionError.USER_DEFINED_ID_INVALID)
                {
                    errorMessage = GetStrBuilder().Append("Attempt to submit ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service Id of ").Append(msg.ServiceId())
                        .Append(" is out of range. Dropping this ")
                        .Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(".").ToString();
                }
                else
                {
                    errorMessage = GetStrBuilder().Append("Attempt to submit ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service Id of ")
                        .Append(msg.ServiceId())
                        .Append(" caused unknown error.").ToString();
                }

                HandleInvalidUsage(errorMessage, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        private void LogServiceIdConversionError(ServiceIdConversionError encodingResult, Msg msg)
        {
            if (GetLoggerClient().IsErrorEnabled)
            {
                IMsg rsslMsg = msg.m_rsslMsg;
                string errorMessage;
                if (encodingResult == ServiceIdConversionError.ID_IS_MISSING_FOR_NAME)
                {
                    errorMessage = GetStrBuilder().Append("Submitting ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service name of ")
                        .Append(msg.ServiceName())
                        .Append(" that was not included in the SourceDirectory.")
                        .ToString();
                }
                else if (encodingResult == ServiceIdConversionError.ID_IS_INVALID_FOR_NAME)
                {
                    m_OmmIProviderDirectoryStore.GetServiceIdByName(msg.ServiceName(), out var serviceId);
                    errorMessage = GetStrBuilder().Append("Submitting ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service name of ")
                        .Append(msg.ServiceName())
                        .Append(" whose matching service id of ")
                        .Append(serviceId)
                        .Append(" is out of range.")
                        .ToString();
                }
                else if (encodingResult == ServiceIdConversionError.NAME_IS_MISSING_FOR_ID)
                {
                    errorMessage = GetStrBuilder().Append("Submitting ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service Id of ")
                        .Append(msg.ServiceId())
                        .Append(" that was not included in the SourceDirectory.")
                        .ToString();
                }
                else if (encodingResult == ServiceIdConversionError.USER_DEFINED_ID_INVALID)
                {
                    errorMessage = GetStrBuilder().Append("Submitting ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service Id of ")
                        .Append(msg.ServiceId())
                        .Append(" is out of range.")
                        .ToString();
                }
                else
                {
                    errorMessage = GetStrBuilder().Append("Submitting ").Append(DataType.AsString(Utilities.ToEmaMsgClass[rsslMsg.MsgClass]))
                        .Append(" with service Id of ")
                        .Append(msg.ServiceId())
                        .Append(" caused unknown error.")
                        .ToString();
                }
                GetLoggerClient().Error("OmmIProvider", errorMessage);
            }
        }

        internal void UpdateItemGroup(ItemInfo itemInfo, Buffer newGroupId)
        {
            RemoveItemGroup(itemInfo);
            AddItemGroup(itemInfo, newGroupId);
        }

        private bool CheckClient(IOmmProviderClient client)
        {
            if (client == null)
            {
                HandleInvalidUsage("A derived class of IOmmProviderClient is not set", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                return true;
            }
            return false;
        }

        private bool RemovePostId(AckMsg ackMsg, ItemInfo itemInfo)
        {
            var ackId = ackMsg.AckId();
            bool removedPostId = itemInfo.RemovePostId(ackId);

            if (!removedPostId)
            {
                HandleInvalidUsage(
                        GetStrBuilder().Append("Attempt to submit AckMsg with non existent AckId = ")
                        .Append(ackId)
                        .Append(". Handle = ")
                        .Append(itemInfo.Handle)
                        .ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                return false;
            }
            return true;
        }

        public void Submit(PackedMsg packedMsg)
        {
            GetUserLocker().Enter();

            try
            {
                ReactorChannel? reactorChannel = packedMsg.ReactorChannel;

                if(reactorChannel == null)
                {
                    HandleInvalidUsage("Attempt to submit PackedMsg with no channel.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }

                ITransportBuffer? packedBuffer = packedMsg.PackedBuffer;

                if(packedBuffer == null)
                {
                    HandleInvalidUsage("Attempt to submit PackedMsg with non init transport buffer.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }

                var submitOptions = GetSubmitOptions();
                ReactorReturnCode retCode = ReactorReturnCode.SUCCESS;
                if( (retCode = reactorChannel.Submit(packedBuffer, submitOptions, out var errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    packedMsg.Clear();

                    var strBuilder = GetStrBuilder();
                    strBuilder.AppendLine("Internal error: ReactorChannel.Submit() failed in OmmIProviderImpl.Submit( PackedMsg ).")
                         .Append("Client handle ").Append(packedMsg.ClientHandle).Append(ILoggerClient.CR)
                        .Append("Error Id ").Append(errorInfo?.Error.ErrorId).Append(ILoggerClient.CR)
                        .Append("Internal sysError ").Append(errorInfo?.Error.SysError).Append(ILoggerClient.CR)
                        .Append("Error Location ").Append(errorInfo?.Location).Append(ILoggerClient.CR)
                        .Append("Error Text ").Append(errorInfo?.Error.Text);

                    HandleInvalidUsage(strBuilder.ToString(), (int)retCode);
                    return;
                }

                packedMsg.PackedBuffer = null;
            }
            finally
            {
                GetUserLocker().Exit();
            }
        }

        private const long MIN_LONG_VALUE = 1;
        private const long MAX_LONG_VALUE = long.MaxValue;

        private static long m_LongId = int.MaxValue;

        public long NextLongId()
        {
            long id = m_LongId;

            while (GetItemInfo(id) is not null
                && ItemCallbackClient.GetItem(id) is not null)
            {
                if (m_LongId == MAX_LONG_VALUE)
                {
                    m_LongId = MIN_LONG_VALUE;
                }
                else
                {
                    id = ++m_LongId;
                }
            }

            ++m_LongId;

            return id;
        }
    }
}
