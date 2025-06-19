/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class StreamInfo
    {
        internal enum StreamTypeEnum
        {
            CONSUMING = 1,
            PROVIDING =2
        }

        internal int ServiceId { get; set; }

        internal int DomainType { get; set; }

        internal int StreamId { get; set; }

        internal StreamTypeEnum StreamType { get; set; }

        internal long Handle { get; set; }

        public StreamInfo(StreamTypeEnum streamType, int streamId)
        {
            StreamType = streamType;
            StreamId = streamId;
        }

        public StreamInfo(StreamTypeEnum streamType, int streamId, int domainType)
        {
            StreamType = streamType;
            StreamId = streamId;
            DomainType = domainType;
        }

        public StreamInfo(StreamTypeEnum streamType, int streamId, int seviceId, int domainType)
        {
            StreamType = streamType;
            StreamId = streamId;
            DomainType = domainType;
            ServiceId = seviceId;
        }
    }

    internal class OmmNiProviderImpl : OmmBaseImpl<IOmmProviderClient>, IOmmProviderImpl, IDirectoryServiceStoreClient
    {
        OmmProvider m_OmmProvider;
        OmmNiProviderConfigImpl m_NiProvConfig;
        IOmmProviderErrorClient? m_ProviderErrorClient;
        IOmmProviderClient? m_AdminClient;
        object? m_AdminClosure;
        IDictionary<long, StreamInfo> m_HandleToStreamInfo = new Dictionary<long, StreamInfo>();
        int m_NextProviderStreamId;
        IList<int> m_ReusedProviderStreamIds;
        ItemWatchList m_ItemWatchList;
        OmmNiProviderDirectoryStore m_NiProviderDirectoryStore;
        private RequestMsg m_LoginRequest = new();
        private ChannelInfo? m_ActiveChannelInfo;
        private bool m_IsStreamIdZeroRefreshSubmitted = false;

        private const long MIN_LONG_VALUE = 1;
        private const long MAX_LONG_VALUE = long.MaxValue;

        private static long m_LongId = MAX_LONG_VALUE;

        private const int INIT_REUSE_STREAM_ID_LIST = 5000;

        private const int INIT_SOURCE_DIR_BUFFER_SIZE = 1024;

        internal OmmNiProviderConfigImpl NiProviderConfigImpl { get; private set; }

        public OmmProviderConfig.ProviderRoleEnum ProviderRole => OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE;

        public OmmNiProviderImpl(OmmProvider provider, OmmNiProviderConfigImpl config,
            IOmmProviderErrorClient? errorClient = null) : base(config)
        {
            m_OmmProvider = provider;
            NiProviderConfigImpl = (OmmNiProviderConfigImpl)OmmConfigBaseImpl;
            m_NiProvConfig = config;
            m_ProviderErrorClient = errorClient;

            m_ItemWatchList = new ItemWatchList(ItemCallbackClient!);

            m_NiProviderDirectoryStore = new(m_EmaObjectManager, this, this);

            m_NiProviderDirectoryStore.DirectoryServiceStoreClient = this;

            m_ReusedProviderStreamIds = new List<int>(INIT_REUSE_STREAM_ID_LIST);

            Initialize();
        }

        public OmmNiProviderImpl(OmmProvider provider, OmmNiProviderConfigImpl config,
            IOmmProviderClient client, object? closure) : base(config)
        {
            m_OmmProvider = provider;
            m_NiProvConfig = config;
            NiProviderConfigImpl = (OmmNiProviderConfigImpl)OmmConfigBaseImpl;
            m_AdminClient = client;
            m_AdminClosure = closure;

            m_ItemWatchList = new ItemWatchList(ItemCallbackClient!);

            m_NiProviderDirectoryStore = new OmmNiProviderDirectoryStore(m_EmaObjectManager, this, this);

            m_NiProviderDirectoryStore.DirectoryServiceStoreClient = this;

            m_ReusedProviderStreamIds = new List<int>(INIT_REUSE_STREAM_ID_LIST);

            Initialize();
        }

        public OmmNiProviderImpl(OmmProvider provider, OmmNiProviderConfigImpl config,
            IOmmProviderClient client, IOmmProviderErrorClient errorClient, object? closure) : base(config)
        {
            m_OmmProvider = provider;
            m_NiProvConfig = config;
            NiProviderConfigImpl = (OmmNiProviderConfigImpl)OmmConfigBaseImpl;
            m_AdminClient = client;
            m_AdminClosure = closure;
            m_ProviderErrorClient = errorClient;

            m_ItemWatchList = new ItemWatchList(ItemCallbackClient!);

            m_NiProviderDirectoryStore = new OmmNiProviderDirectoryStore(m_EmaObjectManager, this, this);

            m_NiProviderDirectoryStore.DirectoryServiceStoreClient = this;

            m_ReusedProviderStreamIds = new List<int>(INIT_REUSE_STREAM_ID_LIST);

            Initialize();
        }

        public string ProviderName { get => InstanceName; }

        public OmmProvider Provider { get => m_OmmProvider; }

        public ItemWatchList ItemWatchList { get => m_ItemWatchList; }

        public override long RegisterClient(RequestMsg reqMsg, IOmmProviderClient client, object? closure)
        {
            if (CheckClient(client, "IOmmProviderClient"))
                return 0;

            if(reqMsg.DomainType() != Rdm.EmaRdm.MMT_LOGIN && reqMsg.DomainType() != Rdm.EmaRdm.MMT_DICTIONARY)
            {
                StringBuilder strBuilder = GetStrBuilder();
                strBuilder.Append("OMM Non-Interactive provider supports registering LOGIN and DICTIONARY domain type only.");
                HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                return 0;
            }

            long handle = base.RegisterClient(reqMsg, client, closure);

            UserLock.Enter();

            try
            {
                var item = ItemCallbackClient!.GetItem(handle);

                if (handle != 0 && item != null)
                {
                    StreamInfo streamInfo = new(StreamInfo.StreamTypeEnum.CONSUMING,
                        item.StreamId, reqMsg.DomainType());

                    streamInfo.Handle = handle;
                    m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;
                }
            }
            finally
            {
                UserLock.Exit();
            }

            return handle;
        }

        public override void Reissue(RequestMsg reqMsg, long handle)
        {
            if (reqMsg.DomainType() != Rdm.EmaRdm.MMT_LOGIN && reqMsg.DomainType() != Rdm.EmaRdm.MMT_DICTIONARY)
            {
                StringBuilder strBuilder = GetStrBuilder();
                strBuilder.Append("OMM Non-Interactive provider supports reissuing LOGIN and DICTIONARY domain type only.");
                HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                return;
            }

            base.Reissue(reqMsg, handle);
        }

        public override void Submit(GenericMsg genericMsg, long handle)
        {
            UserLock.Enter();

            try
            {
                if (m_ActiveChannelInfo == null || m_ActiveChannelInfo.ReactorChannel == null)
                {
                    HandleInvalidUsage(GetStrBuilder().Append("No active channel to send message.").ToString(),
                        OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);

                    return;
                }

                if (LoggerClient.IsTraceEnabled)
                {
                    LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received GenericMsg;" +
                        $" Handle = {handle}, user assigned streamId = {genericMsg.StreamId()}").ToString());
                }

                if (m_HandleToStreamInfo.TryGetValue(handle, out StreamInfo? streamInfo))
                {
                    if (streamInfo.StreamType == StreamInfo.StreamTypeEnum.CONSUMING)
                    {
                        HandleInvalidHandle(handle, GetStrBuilder().Append("Attempt to submit( GenericMsg ) using a registered handle.").ToString());
                        return;
                    }

                    genericMsg.StreamId(streamInfo.StreamId);
                    if(genericMsg.DomainType() == 0)
                    {
                        genericMsg.DomainType(streamInfo.DomainType);
                    }
                }
                else
                {
                    HandleInvalidHandle(handle, GetStrBuilder().Append($"Attempt to submit GenericMsg on stream that" +
                        $" is not open yet. Handle = {handle}.").ToString());
                    return;
                }

                ReactorReturnCode ret;
                ReactorSubmitOptions submitOptions = GetSubmitOptions().ApplyClientChannelConfig(m_ActiveChannelInfo.ChannelConfig);
                if ((ret = m_ActiveChannelInfo.ReactorChannel!.Submit(genericMsg.m_rsslMsg, submitOptions, out var ErrorInfo))
                    < ReactorReturnCode.SUCCESS)
                {
                    StringBuilder message = GetStrBuilder();

                    if (LoggerClient.IsErrorEnabled)
                    {
                        Error? error = ErrorInfo?.Error;

                        message.Append($"Internal error: ReactorChannel.Submit() failed in OmmNiProviderImpl.Submit(GenericMsg) ")
                        .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"Error Id {error?.ErrorId}")
                            .AppendLine($"Internal sysError {error?.SysError}")
                            .AppendLine($"Error Location {ErrorInfo?.Location}")
                            .Append($"Error Text {error?.Text}");

                        LoggerClient.Error(InstanceName, message.ToString());

                        message.Clear();
                    }

                    message.Append($"Failed to send GenericMsg. Reason: ")
                        .Append(ret)
                        .Append(". Error text: ")
                        .Append(ErrorInfo?.Error.Text);

                    HandleInvalidUsage(message.ToString(), (int)ret);

                    return;
                }
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public void Submit(RefreshMsg refreshMsg, long handle)
        {
            bool isHandleAdded = false;

            UserLock.Enter();

            try
            {
                if (ChannelCallbackClient is null)
                {
                    return;
                }

                if (m_HandleToStreamInfo.TryGetValue(handle, out StreamInfo? streamInfo))
                {
                    if (streamInfo.StreamType == StreamInfo.StreamTypeEnum.CONSUMING)
                    {
                        HandleInvalidHandle(handle, "Attempt to submit( RefreshMsg ) using a registered handle.");
                        return;
                    }
                }

                if (m_ActiveChannelInfo is null)
                {
                    HandleInvalidUsage("No active channel to send message.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                    return;
                }

                Eta.Codec.Msg msg = refreshMsg.m_rsslMsg;
                if (refreshMsg.DomainType() == Rdm.EmaRdm.MMT_DIRECTORY)
                {
                    if (LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received RefreshMsg with SourceDirectory domain; Handle = {handle}" +
                            $", user assigned streamId = {refreshMsg.StreamId()}.").ToString());
                    }

                    if (msg.ContainerType != DataTypes.MAP)
                    {
                        HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit RefreshMsg with SourceDirectory domain using container with wrong data type." +
                            $"Expected container data type is Map. Passed in is {DataTypes.ToString(msg.ContainerType)}").ToString(),
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    StringBuilder strBuilder = GetStrBuilder();
                    if (!m_NiProviderDirectoryStore.DecodeSourceDirectory(msg, GetStrBuilder(), out int errorCode))
                    {
                        HandleInvalidUsage(strBuilder.ToString(), errorCode);
                        return;
                    }

                    if (!m_NiProviderDirectoryStore.SubmitSourceDirectory(null, msg, GetStrBuilder(), 
                        m_NiProvConfig.NiProviderConfig.RecoverUserSubmitSourceDirectory,
                        out errorCode))
                    {
                        StringBuilder text = new StringBuilder();
                        text.AppendLine("Attempt to submit invalid source directory domain message.")
                            .Append($"Reason = {strBuilder}");
                        HandleInvalidUsage(text.ToString(), errorCode);
                        return;
                    }

                    msg.Flags &= ~RefreshMsgFlags.SOLICITED;
                    if (m_NiProvConfig.NiProviderConfig.MergeSourceDirectoryStreams)
                    {
                        msg.StreamId = 0;
                    }
                    else
                    {
                        if (streamInfo != null)
                        {
                            msg.StreamId = streamInfo.StreamId;
                        }
                        else
                        {
                            streamInfo = new (StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId());
                            streamInfo.Handle = handle;
                            msg.StreamId = streamInfo.StreamId;
                            m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                            isHandleAdded = true;
                        }
                    }
                }
                else
                {
                    if (LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received RefreshMsg with market domain;" +
                            $" Handle = {handle}, user assigned streamId = {refreshMsg.StreamId()}").ToString());
                    }

                    if (streamInfo != null)
                    {
                        msg.StreamId = streamInfo.StreamId;

                        if ((msg.Flags & RefreshMsgFlags.HAS_MSG_KEY) != 0)
                        {
                            msg.MsgKey.ServiceId = streamInfo.ServiceId;
                            msg.MsgKey.ApplyHasServiceId();
                        }
                    }
                    else if (refreshMsg.HasServiceName)
                    {
                        string serviceName = refreshMsg.ServiceName();

                        if (m_NiProviderDirectoryStore.GetServiceIdByName(serviceName, out int serviceId) == false)
                        {
                            HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit initial RefreshMsg with service name of {serviceName}" +
                                $" that was not included in the SourceDirectory. Dropping this RefreshMsg.").ToString(),
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            return;
                        }

                        msg.MsgKey.ServiceId = serviceId;
                        msg.MsgKey.ApplyHasServiceId();

                        msg.Flags &= ~RefreshMsgFlags.SOLICITED;

                        streamInfo = new (StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId(), serviceId, refreshMsg.DomainType());
                        streamInfo.Handle = handle;

                        msg.StreamId = streamInfo.StreamId;
                        m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                        isHandleAdded = true;
                    }
                    else if(refreshMsg.HasServiceId)
                    {
                        int serviceId = refreshMsg.ServiceId();

                        if(m_NiProviderDirectoryStore.GetServiceNameById(serviceId, out var serviceName) == false)
                        {
                            HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit initial RefreshMsg with service id of {serviceId}" +
                                $" that was not included in the SourceDirectory. Dropping this RefreshMsg.").ToString(),
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            return;
                        }

                        msg.Flags &= ~RefreshMsgFlags.SOLICITED;

                        streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId(), serviceId, refreshMsg.DomainType());
                        streamInfo.Handle = handle;

                        msg.StreamId = streamInfo.StreamId;
                        m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                        isHandleAdded = true;
                    }
                    else
                    {
                        HandleInvalidUsage("Attempt to submit initial RefreshMsg without service name or id. Dropping this RefreshMsg.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }
                }

                ReactorReturnCode retCode;
                ReactorSubmitOptions submitOptions = GetSubmitOptions().ApplyClientChannelConfig(m_ActiveChannelInfo.ChannelConfig);
                if ((retCode = m_ActiveChannelInfo.ReactorChannel!.Submit(msg, submitOptions, out var errorInfo ))
                    < ReactorReturnCode.SUCCESS)
                {
                    if(isHandleAdded)
                    {
                        m_HandleToStreamInfo.Remove(handle);
                        ReturnProviderStreamId(msg.StreamId);
                    }

                    Error? error = errorInfo?.Error;
                    if (LoggerClient.IsErrorEnabled)
                    {
                        var message = GetStrBuilder();
                        message.Append("Internal error: ReactorChannel.Submit() failed in OmmNiProviderImpl.Submit(RefreshMsg)")
                        .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"Error Id {error?.ErrorId}")
                            .AppendLine($"Internal sysError {error?.SysError}")
                            .AppendLine($"Error Location {errorInfo?.Location}")
                            .Append($"Error Text {error?.Text}");

                        LoggerClient.Error(InstanceName, message.ToString());
                    }

                    HandleInvalidUsage(GetStrBuilder().Append($"Failed to submit RefreshMsg. Reason: {retCode}. " +
                        $"Error text: {error?.Text}").ToString(), (int)retCode);
                    return;
                }

                if(refreshMsg.State().StreamState == OmmState.StreamStates.CLOSED ||
                   refreshMsg.State().StreamState == OmmState.StreamStates.CLOSED_RECOVER ||
                   refreshMsg.State().StreamState == OmmState.StreamStates.CLOSED_REDIRECTED ||
                   (refreshMsg.State().StreamState == OmmState.StreamStates.NON_STREAMING && refreshMsg.Complete()))
                {
                    m_HandleToStreamInfo.Remove(handle);
                    ReturnProviderStreamId(msg.StreamId);
                }

                if(msg.StreamId == 0)
                {
                    m_IsStreamIdZeroRefreshSubmitted = true;
                }
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public void Submit(UpdateMsg updateMsg, long handle)
        {
            bool isHandleAdded = false;

            UserLock.Enter();

            try
            {
                if (ChannelCallbackClient is null)
                {
                    return;
                }

                if (m_HandleToStreamInfo.TryGetValue(handle, out StreamInfo? streamInfo))
                {
                    if (streamInfo.StreamType == StreamInfo.StreamTypeEnum.CONSUMING)
                    {
                        HandleInvalidHandle(handle, "Attempt to submit( UpdateMsg ) using a registered handle.");
                        return;
                    }
                }

                if (m_ActiveChannelInfo is null)
                {
                    HandleInvalidUsage("No active channel to send message.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                    return;
                }

                Eta.Codec.Msg msg = updateMsg.m_rsslMsg;
                if (updateMsg.DomainType() == Rdm.EmaRdm.MMT_DIRECTORY)
                {
                    if (LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received UpdateMsg with SourceDirectory domain; Handle = {handle}" +
                            $", user assigned streamId = {updateMsg.StreamId()}.").ToString());
                    }

                    if (msg.ContainerType != DataTypes.MAP)
                    {
                        HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit UpdateMsg with SourceDirectory domain using container with wrong data type." +
                            $"Expected container data type is Map.Passed in is {DataTypes.ToString(msg.ContainerType)}").ToString(),
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    StringBuilder strBuilder = GetStrBuilder();
                    if (!m_NiProviderDirectoryStore.DecodeSourceDirectory(msg, GetStrBuilder(), out int errorCode))
                    {
                        HandleInvalidUsage(strBuilder.ToString(), errorCode);
                        return;
                    }

                    if (!m_NiProviderDirectoryStore.SubmitSourceDirectory(null, msg, GetStrBuilder(),
                        m_NiProvConfig.NiProviderConfig.RecoverUserSubmitSourceDirectory,
                        out errorCode))
                    {
                        StringBuilder text = new StringBuilder();
                        text.AppendLine("Attempt to submit invalid source directory domain message.")
                            .Append($"Reason = {strBuilder}");
                        HandleInvalidUsage(text.ToString(), errorCode);
                        return;
                    }

                    if (m_NiProvConfig.NiProviderConfig.MergeSourceDirectoryStreams)
                    {
                        if(m_NiProvConfig.NiProviderConfig.RefreshFirstRequired && m_IsStreamIdZeroRefreshSubmitted == false)
                        {
                            HandleInvalidHandle(handle, GetStrBuilder().Append($"Attempt to submit UpdateMsg with SourceDirectory" +
                                $" while RefreshMsg was not submitted on this stream yet. Handle = {handle}.").ToString());

                            return;
                        }

                        msg.StreamId = 0;
                    }
                    else
                    {
                        if (streamInfo != null)
                        {
                            msg.StreamId = streamInfo.StreamId;
                        }
                        else
                        {
                            streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId());
                            streamInfo.Handle = handle;
                            msg.StreamId = streamInfo.StreamId;
                            m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                            isHandleAdded = true;
                        }
                    }
                }
                else
                {
                    if(LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received UpdateMsg with market" +
                            $" domain; Handle = {handle}, user assigned streamId = {updateMsg.StreamId()}").ToString());
                    }

                    if(streamInfo != null)
                    {
                        msg.StreamId = streamInfo.StreamId;

                        if((msg.Flags & UpdateMsgFlags.HAS_MSG_KEY) != 0)
                        {
                            msg.MsgKey.ServiceId = streamInfo.ServiceId;
                            msg.MsgKey.ApplyHasServiceId();
                        }
                    }
                    else if (updateMsg.HasServiceName)
                    {
                        string serviceName = updateMsg.ServiceName();

                        if (m_NiProviderDirectoryStore.GetServiceIdByName(serviceName, out int serviceId) == false)
                        {
                            HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit initial UpdateMsg with service name of {serviceName}" +
                                $" that was not included in the SourceDirectory. Dropping this UpdateMsg.").ToString(),
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            return;
                        }

                        msg.MsgKey.ServiceId = serviceId;
                        msg.MsgKey.ApplyHasServiceId();

                        streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId(), serviceId, updateMsg.DomainType());
                        streamInfo.Handle = handle;

                        msg.StreamId = streamInfo.StreamId;
                        m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                        isHandleAdded = true;
                    }
                    else if (updateMsg.HasServiceId)
                    {
                        int serviceId = updateMsg.ServiceId();

                        if (m_NiProviderDirectoryStore.GetServiceNameById(serviceId, out var serviceName) == false)
                        {
                            HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit initial UpdateMsg with service id of {serviceId}" +
                                $" that was not included in the SourceDirectory. Dropping this UpdateMsg.").ToString(),
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            return;
                        }

                        streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId(), serviceId,updateMsg.DomainType());
                        streamInfo.Handle = handle;

                        msg.StreamId = streamInfo.StreamId;
                        m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                        isHandleAdded = true;
                    }
                    else
                    {
                        HandleInvalidUsage("Attempt to submit initial UpdateMsg without service name or id. Dropping this UpdateMsg.",
                           OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }
                }

                ReactorReturnCode retCode;
                ReactorSubmitOptions submitOptions = GetSubmitOptions().ApplyClientChannelConfig(m_ActiveChannelInfo.ChannelConfig);
                if ((retCode = m_ActiveChannelInfo.ReactorChannel!.Submit(msg, submitOptions, out var errorInfo))
                    < ReactorReturnCode.SUCCESS)
                {
                    if (isHandleAdded)
                    {
                        m_HandleToStreamInfo.Remove(handle);
                        ReturnProviderStreamId(msg.StreamId);
                    }

                    Error? error = errorInfo?.Error;
                    if (LoggerClient.IsErrorEnabled)
                    {
                        var message = GetStrBuilder();
                        message.Append("Internal error: ReactorChannel.Submit() failed in OmmNiProviderImpl.Submit(UpdateMsg)")
                        .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"Error Id {error?.ErrorId}")
                            .AppendLine($"Internal sysError {error?.SysError}")
                            .AppendLine($"Error Location {errorInfo?.Location}")
                            .Append($"Error Text {error?.Text}");

                        LoggerClient.Error(InstanceName, message.ToString());
                    }

                    HandleInvalidUsage(GetStrBuilder().Append($"Failed to submit UpdateMsg. Reason: {retCode}. " +
                        $"Error text: {error?.Text}").ToString(), (int)retCode);
                    return;
                }
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public void Submit(StatusMsg statusMsg, long handle)
        {
            bool isHandleAdded = false;

            UserLock.Enter();

            try
            {
                if (ChannelCallbackClient is null)
                {
                    return;
                }

                if (m_HandleToStreamInfo.TryGetValue(handle, out StreamInfo? streamInfo))
                {
                    if (streamInfo.StreamType == StreamInfo.StreamTypeEnum.CONSUMING)
                    {
                        HandleInvalidHandle(handle, "Attempt to submit( StatusMsg ) using a registered handle.");
                        return;
                    }
                }

                if (m_ActiveChannelInfo is null)
                {
                    HandleInvalidUsage("No active channel to send message.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                    return;
                }

                Eta.Codec.Msg msg = statusMsg.m_rsslMsg;
                if (statusMsg.DomainType() == Rdm.EmaRdm.MMT_DIRECTORY)
                {
                    if (LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received StatusMsg with SourceDirectory domain; Handle = {handle}" +
                            $", user assigned streamId = {statusMsg.StreamId()}.").ToString());
                    }

                    if (msg.ContainerType != DataTypes.MAP)
                    {
                        HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit StatusMsg with SourceDirectory domain using container with wrong data type." +
                            $"Expected container data type is Map. Passed in is {DataTypes.ToString(msg.ContainerType)}").ToString(),
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }

                    StringBuilder strBuilder = GetStrBuilder();
                    if (!m_NiProviderDirectoryStore.DecodeSourceDirectory(msg, GetStrBuilder(), out int errorCode))
                    {
                        HandleInvalidUsage(strBuilder.ToString(), errorCode);
                        return;
                    }

                    if (!m_NiProviderDirectoryStore.SubmitSourceDirectory(null, msg, GetStrBuilder(),
                        m_NiProvConfig.NiProviderConfig.RecoverUserSubmitSourceDirectory,
                        out errorCode))
                    {
                        StringBuilder text = new StringBuilder();
                        text.AppendLine("Attempt to submit invalid source directory domain message.")
                            .Append($"Reason = {strBuilder}");
                        HandleInvalidUsage(text.ToString(), errorCode);
                        return;
                    }

                    if (m_NiProvConfig.NiProviderConfig.MergeSourceDirectoryStreams)
                    {
                        if(m_NiProvConfig.NiProviderConfig.RefreshFirstRequired && m_IsStreamIdZeroRefreshSubmitted == false)
                        {
                            HandleInvalidHandle(handle, GetStrBuilder().Append($"Attempt to submit StatusMsg with SourceDirectory" +
                               $" while RefreshMsg was not submitted on this stream yet. Handle = {handle}.").ToString());
                            return;
                        }

                        msg.StreamId = 0;
                    }
                    else
                    {
                        if (streamInfo != null)
                        {
                            msg.StreamId = streamInfo.StreamId;
                        }
                        else
                        {
                            streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId());
                            streamInfo.Handle = handle;
                            msg.StreamId = streamInfo.StreamId;
                            m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                            isHandleAdded = true;
                        }
                    }
                }
                else
                {
                    if (LoggerClient.IsTraceEnabled)
                    {
                        LoggerClient.Trace(InstanceName, GetStrBuilder().AppendLine($"Received StatusMsg with market domain;" +
                            $" Handle = {handle}, user assigned streamId = {statusMsg.StreamId()}").ToString());
                    }

                    if (streamInfo != null)
                    {
                        msg.StreamId = streamInfo.StreamId;

                        if ((msg.Flags & RefreshMsgFlags.HAS_MSG_KEY) != 0)
                        {
                            msg.MsgKey.ServiceId = streamInfo.ServiceId;
                            msg.MsgKey.ApplyHasServiceId();
                        }
                    }
                    else if (statusMsg.HasServiceName)
                    {
                        string serviceName = statusMsg.ServiceName();

                        if (m_NiProviderDirectoryStore.GetServiceIdByName(serviceName, out int serviceId) == false)
                        {
                            HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit initial StatusMsg with service name of {serviceName}" +
                                $" that was not included in the SourceDirectory. Dropping this StatusMsg.").ToString(),
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            return;
                        }

                        msg.MsgKey.ServiceId = serviceId;
                        msg.MsgKey.ApplyHasServiceId();

                        streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId(), serviceId, statusMsg.DomainType());
                        streamInfo.Handle = handle;

                        msg.StreamId = streamInfo.StreamId;
                        m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                        isHandleAdded = true;
                    }
                    else if (statusMsg.HasServiceId)
                    {
                        int serviceId = statusMsg.ServiceId();

                        if (m_NiProviderDirectoryStore.GetServiceNameById(serviceId, out var serviceName) == false)
                        {
                            HandleInvalidUsage(GetStrBuilder().Append($"Attempt to submit initial StatusMsg with service id of {serviceId}" +
                                $" that was not included in the SourceDirectory. Dropping this StatusMsg.").ToString(),
                                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                            return;
                        }

                        streamInfo = new(StreamInfo.StreamTypeEnum.PROVIDING, NextProviderStreamId(), serviceId, statusMsg.DomainType());
                        streamInfo.Handle = handle;

                        msg.StreamId = streamInfo.StreamId;
                        m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;

                        isHandleAdded = true;
                    }
                    else
                    {
                        HandleInvalidUsage("Attempt to submit initial StatusMsg without service name or id. Dropping this StatusMsg.",
                            OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                        return;
                    }
                }

                ReactorReturnCode retCode;
                ReactorSubmitOptions submitOptions = GetSubmitOptions().ApplyClientChannelConfig(m_ActiveChannelInfo.ChannelConfig);
                if ((retCode = m_ActiveChannelInfo.ReactorChannel!.Submit(msg, submitOptions, out var errorInfo))
                    < ReactorReturnCode.SUCCESS)
                {
                    if (isHandleAdded)
                    {
                        m_HandleToStreamInfo.Remove(handle);
                        ReturnProviderStreamId(msg.StreamId);
                    }

                    Error? error = errorInfo?.Error;
                    if (LoggerClient.IsErrorEnabled)
                    {
                        var message = GetStrBuilder();
                        message.Append("Internal error: ReactorChannel.Submit() failed in OmmNiProviderImpl.Submit(StatusMsg)")
                        .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"Error Id {error?.ErrorId}")
                            .AppendLine($"Internal sysError {error?.SysError}")
                            .AppendLine($"Error Location {errorInfo?.Location}")
                            .Append($"Error Text {error?.Text}");

                        LoggerClient.Error(InstanceName, message.ToString());
                    }

                    HandleInvalidUsage(GetStrBuilder().Append($"Failed to submit StatusMsg. Reason: {retCode}. " +
                        $"Error text: {error?.Text}").ToString(), (int)retCode);
                    return;
                }

                if (statusMsg.State().StreamState == OmmState.StreamStates.CLOSED ||
                   statusMsg.State().StreamState == OmmState.StreamStates.CLOSED_RECOVER ||
                   statusMsg.State().StreamState == OmmState.StreamStates.CLOSED_REDIRECTED)
                {
                    m_HandleToStreamInfo.Remove(handle);
                    ReturnProviderStreamId(msg.StreamId);
                }
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public override void Uninitialize()
        {
            base.Uninitialize();
        }

        public override void Unregister(long handle)
        {
            UserLock.Enter();

            try
            {
                if(m_HandleToStreamInfo.TryGetValue(handle, out var streamInfo) == false)
                {
                    return;
                }

                if(streamInfo.StreamType != StreamInfo.StreamTypeEnum.CONSUMING)
                {
                    HandleInvalidHandle(handle, "Attempt to unregister a handle that was not registered.");
                    return;
                }

                m_HandleToStreamInfo.Remove(handle);

                base.Unregister(handle);
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public DirectoryServiceStore GetDirectoryServiceStore()
        {
            return m_NiProviderDirectoryStore;
        }

        protected override void HandleAdminDomains()
        {
            LoginCallbackClient = new LoginCallbackClientProvider(this);
            LoginCallbackClient.Initialize();

            ItemCallbackClient = new ItemCallbackClientProvider(this);
            ItemCallbackClient.Initialize();

            if(m_AdminClient != null)
            {
                /* RegisterClient does not require a fully encoded login message to set the callbacks */
                m_LoginRequest.Clear();
                m_LoginRequest.DomainType(Rdm.EmaRdm.MMT_LOGIN);
                ItemCallbackClient.RegisterClient(m_LoginRequest, m_AdminClient, m_AdminClosure);
            }

            ChannelCallbackClient = new ChannelCallbackClient<IOmmProviderClient>(this, reactor!);

            /* This must be called to set the RefreshMsg's flag and filter before getting NIProviderRole below */
            DirectoryServiceStore.GetDirectoryRefreshMsg(m_NiProviderDirectoryStore,
               m_NiProviderDirectoryStore.GetApiControlDirectory().DirectoryRefresh, true);

            ChannelCallbackClient.InitializeNiProviderRole();

            HandleLoginReqTimeout();
        }

        protected override bool HasErrorClient()
        {
            return m_ProviderErrorClient != null;
        }

        protected override void NotifyErrorClient(OmmException ommException)
        {
            if (m_ProviderErrorClient != null)
            {
                switch (ommException.Type)
                {
                    case OmmException.ExceptionType.OmmInvalidUsageException:
                        OmmInvalidUsageException iue = (OmmInvalidUsageException)ommException;
                        m_ProviderErrorClient.OnInvalidUsage(iue.Message, iue.ErrorCode);
                        break;
                    case OmmException.ExceptionType.OmmInvalidHandleException:
                        OmmInvalidHandleException ihe = (OmmInvalidHandleException)ommException;
                        m_ProviderErrorClient.OnInvalidHandle(ihe.Handle, ihe.Message);
                        break;
                    default:
                        throw new NotImplementedException($"Unknown exception type: {ommException.Type}");
                }
            }
        }

        internal override IOmmCommonImpl.ImpleType GetImplType()
        {
            return IOmmCommonImpl.ImpleType.NIPROVIDER;
        }

        public override void HandleInvalidHandle(long handle, string text)
        {
            if (m_ProviderErrorClient != null)
            {
                m_ProviderErrorClient.OnInvalidHandle(handle, text);
            }
            else
            {
                throw new OmmInvalidHandleException(handle, text);
            }
        }

        public override void HandleInvalidUsage(string text, int errorCode)
        {
            if (m_ProviderErrorClient != null)
            {
                m_ProviderErrorClient.OnInvalidUsage(text, errorCode);
            }
            else
            {
                throw new OmmInvalidUsageException(text, errorCode);
            }
        }

        internal override long NextLongId()
        {
            long id = m_LongId;

            while(m_HandleToStreamInfo.ContainsKey(id))
            {
                id = ++m_LongId;

                if(m_LongId == 0)
                {
                    id = m_LongId = MAX_LONG_VALUE;
                }
            }

            return id;
        }

        internal int NextProviderStreamId()
        {
            if(m_ReusedProviderStreamIds.Count == 0)
            {
                if(m_NextProviderStreamId == int.MinValue)
                {
                    StringBuilder strBuilder = GetStrBuilder();
                    strBuilder.Append("Unable to obtain next available stream id for submitting item.");
                    HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                    return 0;
                }

                return --m_NextProviderStreamId;
            }
            else
            {
                int lastIndex = m_ReusedProviderStreamIds.Count - 1;
                int streamId = m_ReusedProviderStreamIds[lastIndex];
                m_ReusedProviderStreamIds.RemoveAt(lastIndex);
                return streamId;
            }
        }

        internal void ReturnProviderStreamId(int streamId)
        {
            m_ReusedProviderStreamIds.Add(streamId);
        }

        internal override void ReLoadDirectory() 
        {
            ReLoadConfigSourceDirectory();
            ReLoadUserSubmitSourceDirectory();
        }

        private void ReLoadConfigSourceDirectory()
        {
            if(m_NiProvConfig.AdminControlDirectory != OmmNiProviderConfig.AdminControlMode.API_CONTROL)
            {
                return;
            }

            DirectoryRefresh directoryRefresh = m_NiProviderDirectoryStore.GetApiControlDirectory().DirectoryRefresh;

            if (directoryRefresh.ServiceList.Count == 0)
                return;

            if(LoggerClient.IsTraceEnabled)
            {
                LoggerClient.Trace(InstanceName, "Reload of configured source directories.");
            }

            if (m_NiProvConfig.NiProviderConfig.RemoveItemsOnDisconnect)
            {
                RemapServiceIdAndServiceName(directoryRefresh);
            }

            m_IsStreamIdZeroRefreshSubmitted = true;

            if (LoggerClient.IsTraceEnabled)
            {
                LoggerClient.Trace(InstanceName, "Configured source directories were sent out on the wire after reconnect.");
            }
        }

        public override void ProcessChannelEvent(ReactorChannelEvent evt)
        {
            switch(evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_DOWN:
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:

                    m_ItemWatchList?.ProcessChannelEvent(evt);

                    if (m_NiProvConfig.NiProviderConfig.RemoveItemsOnDisconnect)
                        RemoveItems();

                    m_ActiveChannelInfo = null;
                    break;
            }
        }

        private void RemoveItems()
        {
            m_IsStreamIdZeroRefreshSubmitted = false;

            m_HandleToStreamInfo.Clear();

            m_NiProviderDirectoryStore.ClearServiceNameAndId();
        }

        private void ReLoadUserSubmitSourceDirectory()
        {
            if (!m_NiProvConfig.NiProviderConfig.RecoverUserSubmitSourceDirectory)
                return;

            DirectoryRefresh? directoryRefresh = m_NiProviderDirectoryStore.GetDirectoryCache().DirectoryRefresh;

            DirectoryRefresh dirRefresh = DirectoryServiceStore.GetDirectoryRefreshMsg(m_NiProviderDirectoryStore,
                m_NiProviderDirectoryStore.GetDirectoryCache().DirectoryRefresh, false);

            if (directoryRefresh.ServiceList.Count == 0)
                return;

            if (LoggerClient.IsTraceEnabled)
            {
                LoggerClient.Trace(InstanceName, "Reload of user submitted source directories.");
            }

            if(SubmitDirectoryRefresh(directoryRefresh) == CodecReturnCode.SUCCESS)
            {
                m_IsStreamIdZeroRefreshSubmitted = true;

                if (LoggerClient.IsTraceEnabled)
                {
                    LoggerClient.Trace(InstanceName, "User submitted source directoies were sent out on the wire after reconnect.");
                }
            }

        }

        private CodecReturnCode SubmitDirectoryRefresh(DirectoryRefresh directoryRefresh)
        {
            CodecReturnCode retCode;

            Buffer encodedBuffer = new ();
            encodedBuffer.Data(new ByteBuffer(INIT_SOURCE_DIR_BUFFER_SIZE));

            EncodeIterator encodeIt = new ();

            StringBuilder errorText = GetStrBuilder();

            if((retCode = encodeIt.SetBufferAndRWFVersion(encodedBuffer, Codec.MajorVersion(), Codec.MinorVersion()))
                != CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error. Failed to set encode iterator buffer and version in " +
                    $"OmmNiProviderImpl.SubmitDirectoryRefresh(). Reason = {retCode.GetAsString()}.");
                HandleInvalidUsage(errorText.ToString(), (int)retCode);
                return CodecReturnCode.FAILURE;
            }

            int capacity = encodedBuffer.Capacity;
            while ((retCode = directoryRefresh.Encode(encodeIt)) == CodecReturnCode.BUFFER_TOO_SMALL)
            {
                capacity *= 2;

                encodeIt.Clear();
                encodedBuffer.Clear();
                encodedBuffer.Data(new ByteBuffer(capacity));

                if ((retCode = encodeIt.SetBufferAndRWFVersion(encodedBuffer, Codec.MajorVersion(), Codec.MinorVersion())) != CodecReturnCode.SUCCESS)
                {
                    errorText.Append($"Internal error. Failed to set encode iterator buffer and version in " +
                        $"OmmNiProviderImpl.SubmitDirectoryRefresh(). Reason = {retCode.GetAsString()}.");
                    HandleInvalidUsage(errorText.ToString(), (int)retCode);
                    return CodecReturnCode.FAILURE;
                }
            }

            if (retCode != CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error. Failed to encode buffer from DirectoryRefresh in" +
                    $" OmmNiProviderImpl.SubmitDirectoryRefresh(). Reason = {retCode.GetAsString()}.");
                HandleInvalidUsage(errorText.ToString(), (int)retCode);
                return CodecReturnCode.FAILURE;
            }

            DecodeIterator decodeIt = new();

            if((retCode = decodeIt.SetBufferAndRWFVersion(encodedBuffer, Codec.MajorVersion(), Codec.MinorVersion())) 
                != CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error. Failed to set decode iterator buffer and version in" +
                    $" OmmNiProviderImpl.SubmitDirectoryRefresh(). Reason = {retCode.GetAsString()}.");
                HandleInvalidUsage(errorText.ToString(), (int)retCode);
                return CodecReturnCode.FAILURE;
            }

            IRefreshMsg refreshMsg = new Eta.Codec.Msg();
            if ((retCode = refreshMsg.Decode(decodeIt)) != CodecReturnCode.SUCCESS)
            {
                errorText.Append($"Internal error. Failed to decode message in" +
                    $" OmmNiProviderImpl.SubmitDirectoryRefresh(). Reason = {retCode.GetAsString()}.");
                HandleInvalidUsage(errorText.ToString(), (int)retCode);
                return CodecReturnCode.FAILURE;
            }

            refreshMsg.Flags &= ~RefreshMsgFlags.SOLICITED;

            if (m_NiProvConfig.NiProviderConfig.RemoveItemsOnDisconnect)
                RemapServiceIdAndServiceName(directoryRefresh);

            if(m_ActiveChannelInfo == null || m_ActiveChannelInfo.ReactorChannel == null)
            {
                errorText.Append("No active channel to send message.");
                HandleInvalidUsage(errorText.ToString(), OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                return CodecReturnCode.FAILURE;
            }

            ReactorReturnCode reactorRetCode;
            if ((reactorRetCode = m_ActiveChannelInfo.ReactorChannel.Submit(refreshMsg, GetSubmitOptions().ApplyClientChannelConfig(m_ActiveChannelInfo.ChannelConfig), out ReactorErrorInfo? errorInfo))
                < ReactorReturnCode.SUCCESS)
            {
                StringBuilder message = GetStrBuilder();

                if (LoggerClient.IsErrorEnabled)
                {
                    Error? error = errorInfo?.Error;

                    message.Append($"Internal error: ReactorChannel.Submit() failed in OmmNiProviderImpl.ReLoadUserSubmitSourceDirectory() ")
                    .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                        .AppendLine($"Error Id {error?.ErrorId}")
                        .AppendLine($"Internal sysError {error?.SysError}")
                        .AppendLine($"Error Location {errorInfo?.Location}")
                        .Append($"Error Text {error?.Text}");

                    LoggerClient.Error(InstanceName, message.ToString());

                    message.Clear();
                }

                message.Append($"Failed to send RefreshMsg. Reason: ")
                    .Append(reactorRetCode)
                    .Append(". Error text: ")
                    .Append(errorInfo?.Error.Text);

                HandleInvalidUsage(message.ToString(), (int)reactorRetCode);

                return CodecReturnCode.FAILURE;
            }

            return CodecReturnCode.SUCCESS;
        }

        private void RemapServiceIdAndServiceName(DirectoryRefresh directoryRefresh)
        {
            IList<Service> serviceList = directoryRefresh.ServiceList;
            foreach (Service service in serviceList)
            {
                m_NiProviderDirectoryStore.AddToServiceMap(service);
            }
        }

        internal override void SetActiveReactorChannel(ChannelInfo channelInfo) 
        {
            m_ActiveChannelInfo = channelInfo;
        }

        internal override void UnsetActiveRsslReactorChannel(ChannelInfo channelInfo)
        {
            if(ReferenceEquals(m_ActiveChannelInfo, channelInfo))
            {
                m_ActiveChannelInfo = null;
            }
        }

        public ItemWatchList ItemWatchlist() { return m_ItemWatchList; }

        public void OnServiceDelete(ClientSession? session, int serviceId)
        {
            m_ItemWatchList.ProcessServiceDelete(session, serviceId);
        }

        public void OnServiceStateChange(ClientSession? session, int serviceId, ServiceState state)
        {
            /* Do nothing for Non-interactive provider */
        }

        public void OnServiceGroupChange(ClientSession? session, int serviceId, IList<ServiceGroup> serviceGroupList)
        {
            /* Do nothing for Non-interactive provider */
        }

        internal int RequestTimeout()
        {
            return Utilities.Convert_ulong_int(m_NiProvConfig.NiProviderConfig.RequestTimeout);
        }

        public override void ChannelInformation(ChannelInformation channelInformation) 
        {
            if (channelInformation == null)
                return;

            try
            {
                UserLock.Enter();

                ChannelInfo? channelInfo = LoginCallbackClient?.ActiveChannelInfo();

                if (channelInfo == null || channelInfo.ReactorChannel == null)
                {
                    channelInformation.Clear();
                    return;
                }

                channelInformation.Set(channelInfo.ReactorChannel);
                channelInformation.IpAddress = "not available for OmmNiProvider connections";

            }
            finally
            {
                UserLock.Exit();
            }
        }

        public void ModifyIOCtl(IOCtlCode code, int val)
        {
            UserLock.Enter();

            try
            {
                ReactorChannel? reactorChannel = LoginCallbackClient!.ActiveChannelInfo() != null ?
                    LoginCallbackClient!.ActiveChannelInfo()!.ReactorChannel : null;

                base.ModifyIOCtl(code, val, reactorChannel);
            }
            finally
            {
                UserLock.Exit();
            }
        }

        public void ModifyIOCtl(IOCtlCode code, int val, long handle)
        {
            HandleInvalidUsage("NIProvider applications do not support the ModifyIOCtl(int code, int value, long handle) method",
                               OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
        }


        public void Submit(AckMsg ackMsg, long handle)
        {
            StringBuilder strBuilder = GetStrBuilder();

            if(LoggerClient.IsErrorEnabled)
            {
                strBuilder.Append($"Non interactive provider role does not support submitting AckMsg on handle =  {handle}");

                LoggerClient.Error(InstanceName, strBuilder.ToString());
                strBuilder.Clear();
            }

            strBuilder.Append("Failed to submit AckMsg. Reason: ")
                .Append($"Non interactive provider role does not support submitting AckMsg on handle = {handle}");
            HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        public void ConnectedClientChannelInfo(List<ChannelInformation> clientInfoList)
        {
            StringBuilder strBuilder = GetStrBuilder();
            strBuilder.Append("NIProvider applications do not support the ConnectedClientChannelInfo() method.");
            HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        public void CloseChannel(long clientHnadle)
        {
            StringBuilder strBuilder = GetStrBuilder();
            strBuilder.Append("NIProvider applications do not support the CloseChannel() method.");
            HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }

        public StreamInfo? GetStreamInfo(long itemHandle)
        {
            UserLock.Enter();
            StreamInfo? streamInfo = null;

            try
            {
                m_HandleToStreamInfo.TryGetValue(itemHandle, out streamInfo);
            }
            finally
            {
                UserLock.Exit();
            }

            return streamInfo;
        }

        public void AddStreamInfo(StreamInfo streamInfo)
        {
            m_HandleToStreamInfo[streamInfo.Handle] = streamInfo;
        }

        public void RemoveStreamInfo(long itemHandle)
        {
            m_HandleToStreamInfo.Remove(itemHandle);
        }

        public void Submit(PackedMsg packedMsg)
        {
            UserLock.Enter();

            try
            {
                if(m_ActiveChannelInfo == null || m_ActiveChannelInfo.ReactorChannel == null)
                {
                    HandleInvalidUsage("No active channel to send message.", OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);
                    return;
                }

                ReactorChannel reactorChannel = m_ActiveChannelInfo.ReactorChannel!;

                ITransportBuffer? packedBuffer = packedMsg.PackedBuffer;

                if (packedBuffer == null)
                {
                    HandleInvalidUsage("Attempt to submit PackedMsg with non init transport buffer.", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                    return;
                }

                var submitOptions = GetSubmitOptions().ApplyClientChannelConfig(m_ActiveChannelInfo.ChannelConfig);
                ReactorReturnCode retCode = ReactorReturnCode.SUCCESS;
                if ((retCode = reactorChannel.Submit(packedBuffer, submitOptions, out var errorInfo)) < ReactorReturnCode.SUCCESS)
                {
                    packedMsg.Clear();

                    var strBuilder = GetStrBuilder();
                    strBuilder.AppendLine("Internal error: ReactorChannel.Submit() failed in OmmNiProviderImpl.Submit( PackedMsg ).")
                        .AppendLine(m_ActiveChannelInfo.ToString())
                        .AppendLine($"Channel {errorInfo?.Error.Channel.GetHashCode()}")
                        .AppendLine("Error Id ").Append(errorInfo?.Error.ErrorId)
                        .AppendLine("Internal sysError ").Append(errorInfo?.Error.SysError)
                        .AppendLine("Error Location ").Append(errorInfo?.Location)
                        .AppendLine("Error Text ").Append(errorInfo?.Error.Text);

                    HandleInvalidUsage(strBuilder.ToString(), (int)retCode);
                    return;
                }

                packedMsg.PackedBuffer = null;
            }
            finally
            {
                UserLock.Exit();
            }
        }

        protected override void OnDispatchError(string text, int errorCode)
        {
            m_ProviderErrorClient?.OnDispatchError(text, errorCode);
        }
    }
}
