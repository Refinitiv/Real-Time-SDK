/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Common;
using LSEG.Eta.Transports;

namespace LSEG.Ema.Access
{
    internal enum ItemType
    {
        SINGLE_ITEM = 0,
        BATCH_ITEM = 1,
        LOGIN_ITEM = 2,
        DICTIONARY_ITEM = 3,
        DIRECTORY_ITEM = 4,
        NIPROVIDER_DICTIONARY_ITEM = 7,
        IPROVIDER_SINGLE_ITEM = 8,
        IPROVIDER_DICTIONARY_ITEM = 9
    }
    internal interface Item<T>
    {
        public int DomainType { get; set; }
        public int StreamId { get; }
        public object? Closure { get; }
        public T? Client { get; }
        public long ItemId { get; set; }
        public Item<T>? Parent { get; }
        public ClosedStatusClient<T>? ClosedStatusClient { get; set; }
        public bool Close();
        public ServiceDirectory? Directory();
        public int GetNextStreamId(int numOfItem);
        public bool Open(RequestMsg reqMsg);
        public bool Modify(RequestMsg reqMsg);
        public void Remove();
        public ItemType Type();
        public bool Submit(RefreshMsg refreshMsg);
        public bool Submit(UpdateMsg updateMsg);
        public bool Submit(StatusMsg statusMsg);
        public bool Submit(PostMsg postMsg);
        public bool Submit(GenericMsg genericMsg);
        public void BackToPool();
    }

    internal class SingleItem<T> : VaNode, Item<T>
    {
        private static readonly string CLIENT_NAME = "SingleItem";
        internal ServiceDirectory? m_ServiceDirectory { get; set; }
        internal string? ServiceName { get; set; }
        internal OmmBaseImpl<T> m_OmmBaseImpl;
        internal ItemType m_type;

        internal GCHandle m_handle;

        internal bool m_InitReqWithServiceID;
        internal int m_ServiceId;

        public int DomainType { get; set; }
        public int StreamId { get; protected set; }
        public object? Closure { get; private set; }
        public T? Client { get; private set; }
        public Item<T>? Parent { get; private set; }
        public long ItemId { get; set; }
        public ClosedStatusClient<T>? ClosedStatusClient { get; set; }

#pragma warning disable CS8618
        public SingleItem()
        {
            m_type = ItemType.SINGLE_ITEM;
        }
#pragma warning restore CS8618

        public SingleItem(OmmBaseImpl<T> baseImpl, T? client, object? closure, SingleItem<T>? batchItem)
        {
            DomainType = 0;
            StreamId = 0;
            Closure = closure;
            Client = client;
            Parent = batchItem;
            m_type = ItemType.SINGLE_ITEM;

            m_OmmBaseImpl = baseImpl;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void BackToPool()
        {
            Closure = null;
            Client = default;
            ClosedStatusClient = null;

            base.ReturnToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void SetId(long handle, int streamId)
        {
            ItemId = handle;
            StreamId = streamId;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Reset(OmmBaseImpl<T> baseImpl, T client, object? closure, Item<T>? batchItem)
        {
            DomainType = 0;
            StreamId = 0;
            Closure = closure;
            Client = client;
            Parent = batchItem;
            ClosedStatusClient = null;

            m_OmmBaseImpl = baseImpl;
            m_ServiceDirectory = null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Close()
        {
            ICloseMsg closeMsg = m_OmmBaseImpl.ItemCallbackClient!.CloseMsg();
            closeMsg.ContainerType = DataTypes.NO_DATA;
            closeMsg.DomainType = DomainType;

            bool retCode = Submit(closeMsg);

            Remove();
            return retCode;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ServiceDirectory? Directory()
        {
            return m_ServiceDirectory;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual int GetNextStreamId(int numOfItem)
        {
            return m_OmmBaseImpl.ItemCallbackClient!.NextStreamId(numOfItem);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Modify(RequestMsg reqMsg)
        {
            string? serviceName = reqMsg.HasServiceName ? reqMsg.ServiceName() : null;
            return Submit(reqMsg.m_rsslMsg, serviceName, true);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Open(RequestMsg reqMsg)
        {
            ServiceDirectory? service = null;
            string? serviceName = null;

            if (reqMsg.HasServiceName)
            {
                serviceName = reqMsg.ServiceName();
                service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(serviceName);

                if (service == null && (!m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh.LoginAttrib.HasSingleOpen
                    || m_OmmBaseImpl.LoginCallbackClient.LoginRefresh.LoginAttrib.SingleOpen == 0))
                {
                    /* This ensures that the user will get a valid handle.  The callback should clean it up after. */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append($"Service name of '{reqMsg.ServiceName()}' is not found.");

                    ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                                message.ToString(), serviceName);

                    return true;
                }
            }
            else if (reqMsg.HasServiceId)
            {
                service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(reqMsg.ServiceId());

                if (service == null && (!m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh.LoginAttrib.HasSingleOpen
                || m_OmmBaseImpl.LoginCallbackClient.LoginRefresh.LoginAttrib.SingleOpen == 0))
                {
                    /* This ensures that the user will get a valid handle.  The callback should clean it up after. */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append($"Service id of '{reqMsg.ServiceId()}' is not found.");

                    ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                      message.ToString(), null);

                    return true;
                }
            }

            m_ServiceDirectory = service;

            return Submit(reqMsg.m_requestMsgEncoder.m_rsslMsg, serviceName, false);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual void Remove()
        {
           if (Type() != ItemType.BATCH_ITEM)
           {
                if (Parent != null)
                {
                    if(Parent.Type() == ItemType.BATCH_ITEM)
                    {
                        ((BatchItem<T>)Parent).DecreaseItemCount();
                    }
                }

                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this);
           }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ItemType Type()
        {
            return m_type;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Submit(RefreshMsg refreshMsg)
        {
            return Submit(refreshMsg.m_rsslMsg as IRefreshMsg, null);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Submit(UpdateMsg updateMsg)
        {
            return Submit(updateMsg.m_rsslMsg as IUpdateMsg, null);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Submit(StatusMsg statusMsg)
        {
            return Submit(statusMsg.m_rsslMsg as IStatusMsg, null);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Submit(PostMsg postMsg)
        {
            string? serviceName = postMsg.HasServiceName ? postMsg.ServiceName() : null;

            if (!ValidateServiceName(serviceName))
            {
                return false;
            }

            return Submit(postMsg.m_rsslMsg as IPostMsg, serviceName);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Submit(GenericMsg genericMsg)
        {
            return Submit(genericMsg.m_rsslMsg as IGenericMsg, null);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        protected bool Submit(ICloseMsg closeMsg)
        {
            if (StreamId == 0)
            {
                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME,
                        $"Invalid streamId for this item in in SingleItem.Submit(CloseMsg)");
                }

                return false;
            }
            else
            {
                closeMsg.StreamId = StreamId;
            }

            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            ReactorChannel reactorChannel = m_OmmBaseImpl.LoginCallbackClient!.ActiveChannelInfo()!.ReactorChannel!;

            submitOptions.ApplyClientChannelConfig(reactorChannel.GetClientChannelConfig()!);
            submitOptions.ServiceName = null;

            submitOptions.RequestMsgOptions.UserSpecObj = this;

            ReactorReturnCode retCode = reactorChannel.Submit((Eta.Codec.Msg)closeMsg, submitOptions, out ReactorErrorInfo? errorInfo);

            if (retCode < ReactorReturnCode.SUCCESS)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    strBuilder.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(ICloseMsg)")
                    .AppendLine($"\tChannel {errorInfo?.Error.Channel}")
                    .AppendLine($"\tError Id {errorInfo?.Error.ErrorId}")
                    .AppendLine($"\tInternal SysError {errorInfo?.Error.SysError}")
                    .AppendLine($"\tError Location {errorInfo?.Location}")
                    .AppendLine($"\tError Text {errorInfo?.Error.Text}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                    strBuilder.Clear();
                }

                strBuilder.Append($"Failed to close item request. Reason: {retCode}")
                .Append($". Error text: {errorInfo?.Error.Text}");

                m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), (int)retCode);

                return false;
            }

            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        protected virtual bool Submit(IRequestMsg requestMsg, string? serviceName, bool isReissue)
        {
            ReactorChannel? reactorChannel = m_OmmBaseImpl.LoginCallbackClient?.ActiveChannelInfo()?.ReactorChannel;

            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            if (reactorChannel != null)
            {
                submitOptions.ApplyClientChannelConfig(reactorChannel.GetClientChannelConfig()!);
            }

            bool restoreServiceIDFlag = false;

            if (m_ServiceDirectory != null && serviceName != null)
            {
                if (requestMsg.MsgKey.CheckHasServiceId())
                {
                    requestMsg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                    restoreServiceIDFlag = true;
                }

                ServiceName = serviceName;

                m_InitReqWithServiceID = false;
                submitOptions.ServiceName = m_ServiceDirectory.ServiceName;
            }
            else
            {
                submitOptions.ServiceName = null;
                if (!isReissue)
                {
                    /* Use the service ID to send the request instead. */
                    if (requestMsg.MsgKey.CheckHasServiceId())
                    {
                        if (m_ServiceDirectory != null)
                        {
                            m_ServiceId = requestMsg.MsgKey.ServiceId;
                            m_InitReqWithServiceID = true;
                        }
                    }
                    else
                    {
                        ServiceName = serviceName;
                        submitOptions.ServiceName = serviceName;
                    }
                }
                else
                {
                    /* Handles item reissue case */
                    if (!requestMsg.MsgKey.CheckHasServiceId())
                    {
                        if (m_InitReqWithServiceID)
                        {
                            requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                            requestMsg.MsgKey.ServiceId = m_ServiceId;
                        }
                        else
                        {
                            if (serviceName != null)
                            {
                                ServiceName = serviceName;
                                submitOptions.ServiceName = serviceName;
                            }
                            else if (m_ServiceDirectory != null)
                            {
                                ServiceName = m_ServiceDirectory.ServiceName;
                                submitOptions.ServiceName = m_ServiceDirectory.ServiceName;
                            }
                        }
                    }
                }
            }

            if (!requestMsg.CheckHasQos())
            {
                requestMsg.ApplyHasQos();
                requestMsg.ApplyHasWorstQos();
                requestMsg.Qos.IsDynamic = false;
                requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                requestMsg.WorstQos.Rate(QosRates.TIME_CONFLATED);
                requestMsg.WorstQos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
                requestMsg.WorstQos.RateInfo(65535);
            }

            if (m_OmmBaseImpl.BaseType == IOmmCommonImpl.ImpleType.CONSUMER)
            {
                if (((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).ConsumerConfig.MsgKeyInUpdates)
                {
                    requestMsg.ApplyMsgKeyInUpdates();
                }
            }

            submitOptions.RequestMsgOptions.UserSpecObj = this;

            int domainType = requestMsg.DomainType;

            if (StreamId == 0)
            {
                if (requestMsg.CheckHasBatch())
                {
                    var items = ((BatchItem<T>)this).SingleItemList;
                    int numOfItem = items.Count;

                    requestMsg.StreamId = GetNextStreamId(numOfItem);
                    StreamId = requestMsg.StreamId;
                    m_OmmBaseImpl.ItemCallbackClient!.AddToMap(m_OmmBaseImpl.NextLongId(), this);

                    SingleItem<T>? item;
                    int itemStreamStart = StreamId;
                    for (int index = 0; index < numOfItem; index++)
                    {
                        item = items[index];
                        item.m_ServiceDirectory = m_ServiceDirectory;
                        item.StreamId = ++itemStreamStart;
                        item.DomainType = DomainType;
                        m_OmmBaseImpl.ItemCallbackClient!.AddToMap(m_OmmBaseImpl.NextLongId(), item);
                    }
                }
                else
                {
                    StreamId = GetNextStreamId(0);
                    requestMsg.StreamId = StreamId;
                    m_OmmBaseImpl.ItemCallbackClient!.AddToMap(m_OmmBaseImpl.NextLongId(), this);
                }
            }
            else
            {
                requestMsg.StreamId = StreamId;
            }

            if (DomainType == 0)
            {
                DomainType = domainType;
            }
            else
            {
                requestMsg.DomainType = DomainType;
            }

            if (reactorChannel != null)
            {
                ReactorReturnCode ret = reactorChannel.Submit((Eta.Codec.Msg)requestMsg, submitOptions, out var ErrorInfo);

                if (ret < ReactorReturnCode.SUCCESS)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        Error? error = ErrorInfo?.Error;

                        message.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(IRequestMsg requestMsg)")
                        .AppendLine($"\tChannel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"\tError Id {error?.ErrorId}")
                            .AppendLine($"\tInternal sysError {error?.SysError}")
                            .AppendLine($"\tError Location {ErrorInfo?.Location}")
                            .Append($"\tError Text {error?.Text}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                        message.Clear();
                    }

                    message.Append("Failed to open or modify item request. Reason: ")
                        .Append(ret)
                        .Append(". Error text: ")
                        .Append(ErrorInfo?.Error.Text);

                    if (restoreServiceIDFlag)
                    {
                        requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                    }

                    m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ret);

                    return false;
                }
            }
            else
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    message.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(IRequestMsg requestMsg)")
                    .AppendLine($"\tReactorChannel is not avaliable");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append("Failed to open or modify item request. Reason: ReactorChannel is not avaliable");

                if (restoreServiceIDFlag)
                {
                    requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                }

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);

                return false;
            }

            if (restoreServiceIDFlag)
            {
                requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
            }

            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool Submit<V>(V submitMsg, string? serviceName) where V : IMsg
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            submitOptions.Clear();
            submitOptions.ServiceName = serviceName;

            submitMsg.StreamId = StreamId;

            if (submitMsg.MsgClass == MsgClasses.GENERIC && submitMsg.DomainType == 0)
            {
                submitMsg.DomainType = DomainType;
            }

            ReactorChannel? reactorChannel = m_OmmBaseImpl.LoginCallbackClient?.ActiveChannelInfo()?.ReactorChannel;

            if (reactorChannel != null)
            {
                submitOptions.ApplyClientChannelConfig(reactorChannel.GetClientChannelConfig()!);

                ReactorReturnCode ret = reactorChannel.Submit(submitMsg, submitOptions, out var ErrorInfo);

                if (ret < ReactorReturnCode.SUCCESS)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        Error? error = ErrorInfo?.Error;

                        message.Append($"Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(IMsg submitMsg) " +
                            $"for {submitMsg.GetType().Name}")
                        .AppendLine($"\tChannel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"\tError Id {error?.ErrorId}")
                            .AppendLine($"\tInternal sysError {error?.SysError}")
                            .AppendLine($"\tError Location {ErrorInfo?.Location}")
                            .Append($"\tError Text {error?.Text}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                        message.Clear();
                    }

                    message.Append($"Failed to send {submitMsg.GetType().Name}. Reason: ")
                        .Append(ret)
                        .Append(". Error text: ")
                        .Append(ErrorInfo?.Error.Text);

                    m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ret);

                    return false;
                }
            }
            else
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    message.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(IRefreshMsg refreshMsg)")
                    .AppendLine($"\tReactorChannel is not avaliable");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append($"Failed to send {submitMsg.GetType().Name}. Reason: ReactorChannel is not avaliable");

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);

                return false;
            }

            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private bool ValidateServiceName(string? serviceName)
        {
            if (serviceName == null || m_OmmBaseImpl.DirectoryCallbackClient!.GetService(serviceName) != null)
            {
                return true;
            }

            StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
            if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                strBuilder.AppendLine("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(PostMsg)")
                    .AppendLine($"\tError Id {ReactorReturnCode.INVALID_USAGE}")
                    .AppendLine("\tError Location ItemCallbackClient.Submit(IPostMsg,String)")
                    .AppendLine($"\tError Text Message submitted with unknown service name {serviceName}");

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                strBuilder.Clear();
            }

            strBuilder.Append("Failed to submit PostMsg on item stream. Reason: ")
                       .Append(ReactorReturnCode.INVALID_USAGE)
                       .Append($". Error text: Message submitted with unknown service name {serviceName}");

            m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), (int)ReactorReturnCode.INVALID_USAGE);
            return false;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void ScheduleItemClosedStatus(CallbackClient<T> client, SingleItem<T> item, IMsg rsslMsg, string statusText, string? serviceName)
        {
            if (ClosedStatusClient != null) return;

            ClosedStatusClient = new ClosedStatusClient<T>(client, item, rsslMsg, statusText, serviceName);
            m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(1000, ClosedStatusClient);
        }
    }

    internal sealed class BatchItem<T> : SingleItem<T>
    {
        private static readonly string CLIENT_NAME = "BatchItem";

        internal List<SingleItem<T>> SingleItemList { get; set; } = new();

        internal int ItemCount { get; set; }

        internal bool StatusFlag { get; set; }

        public BatchItem()
        {
            m_type = ItemType.BATCH_ITEM;
        }

        public BatchItem(OmmBaseImpl<T> baseImpl, T client, object? closure) : base(baseImpl, client, closure, null)
        {
            m_type = ItemType.BATCH_ITEM;
            ItemCount = 1;
        }

        public void ResetBatchItem(OmmBaseImpl<T> baseImpl, T client, object? closure)
        {
            Reset(baseImpl, client, closure, null);
            SingleItemList.Clear();
            ItemCount = 1;
        }

        public override bool Modify(RequestMsg reqMsg)
        {
            return HandleInvalidAttemp("Invalid attempt to modify batch stream");
        }

        public override bool Submit(PostMsg postMsg)
        {
            return HandleInvalidAttemp("Invalid attempt to submit PostMsg on batch stream");
        }

        public override bool Submit(GenericMsg genericMsg)
        {
            return HandleInvalidAttemp("Invalid attempt to submit GenericMsg on batch stream");
        }

        public override bool Close()
        {
            return HandleInvalidAttemp("Invalid attempt to close batch stream");
        }

        internal void AddBatchItems(int numOfItem)
        {
            SingleItem<T> singleItem;

            for(int i = 0; i < numOfItem; i++)
            {
                singleItem = new SingleItem<T>(m_OmmBaseImpl, Client, Closure, this);

                SingleItemList.Add(singleItem);
            }

            ItemCount = numOfItem;
        }

        internal SingleItem<T>? GetSingleItem(int streamId)
        {
            int index = streamId - StreamId;
            if (index < 0 || index > SingleItemList.Count)
                return null;

            return (index == 0) ? this : SingleItemList[index - 1];
        }

        internal void DecreaseItemCount()
        {
            if ( --ItemCount == 0 && StatusFlag)
            {
                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this);
            }
        }

        public override void Remove()
        {
            if (ItemCount == 0)
            {
                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this);
            }
        }

        private bool HandleInvalidAttemp(string message)
        {
            StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
            strBuilder.Append($"{message}. Instance name='{m_OmmBaseImpl.InstanceName}'.");

            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
            }

            m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);

            return false;
        }
    }

    internal class ClosedStatusClient<T> : ITimeoutClient
    {
        private IMsgKey msgKey = new MsgKey();
        private Buffer statusText = new Buffer();
        private string? serviceName;
        private int domainType;
        private int streamId;
        private Item<T> item;
        private bool isPrivateStream;
        private CallbackClient<T> client;
        private State state = new State();

        public ClosedStatusClient(CallbackClient<T> client, Item<T> item, IMsg msg, string statusText, string? serviceName)
        {
            this.client = client;
            this.item = item;
            this.statusText.Data(statusText);
            domainType = msg.DomainType;
            msgKey.Clear();
            this.serviceName = serviceName;
            streamId = msg.StreamId;

            if (msg.MsgKey != null)
                msg.MsgKey.Copy(msgKey);

            state.StreamState(StreamStates.CLOSED);
            state.DataState(DataStates.SUSPECT);
            state.Code(StateCodes.NONE);

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    isPrivateStream = (msg.Flags & RefreshMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
                    break;
                case MsgClasses.STATUS:
                    isPrivateStream = (msg.Flags & StatusMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
                    break;
                case MsgClasses.REQUEST:
                    isPrivateStream = (msg.Flags & RequestMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
                    break;
                case MsgClasses.ACK:
                    isPrivateStream = (msg.Flags & AckMsgFlags.PRIVATE_STREAM) > 0 ? true : false;
                    break;
                default:
                    isPrivateStream = false;
                    break;
            }
        }

        public ClosedStatusClient(CallbackClient<T> client, Item<T> item, MsgKey msgKey, bool privateStream,
            string statusText, string? serviceName)
        {
            this.client = client;
            this.item = item;
            this.statusText.Data(statusText);
            domainType = item.DomainType;
            this.serviceName = serviceName;
            this.msgKey = msgKey;

            state.StreamState(StreamStates.CLOSED_RECOVER);
            state.DataState(DataStates.SUSPECT);
            state.Code(StateCodes.NONE);
        }

        public void HandleTimeoutEvent()
        {
            IStatusMsg statusMsg = client.StatusMsg();

            statusMsg.StreamId = streamId;
            statusMsg.DomainType = domainType;
            statusMsg.ContainerType = DataTypes.NO_DATA;

            statusMsg.ApplyHasState();
            statusMsg.State.StreamState(state.StreamState());
            if (item.Type() != ItemType.BATCH_ITEM)
                statusMsg.State.DataState(state.DataState());
            else
                statusMsg.State.DataState(DataStates.OK);
            statusMsg.State.Code(state.Code());
            statusMsg.State.Text(statusText);

            statusMsg.ApplyHasMsgKey();
            msgKey.Copy(statusMsg.MsgKey);

            if (isPrivateStream)
                statusMsg.ApplyPrivateStream();

            if (client.m_StatusMsg == null)
                client.m_StatusMsg = new StatusMsg();

            client.m_StatusMsg.Decode(statusMsg, Codec.MajorVersion(), Codec.MajorVersion(), null!);

            client.m_StatusMsg.SetServiceName(serviceName!);
            client.EventImpl.Item = item;
            client.NotifyOnAllMsg(client.m_StatusMsg!);
            client.NotifyOnStatusMsg();

            if(item.Type() != ItemType.BATCH_ITEM)
            {
                item.Remove();
            }
            else
            {
                ((ItemCallbackClient<T>)client).RemoveFromMap(item);
            }
        }
    }

    internal class ItemCallbackClient<T> : CallbackClient<T>, IDefaultMsgCallback
    {
        private static readonly string CLIENT_NAME = "ItemCallbackClient";
        private const int CONSUMER_STARTING_STREAM_ID = 4;
        private const int PROVIDER_STARTING_STREAM_ID = 0;
        private const int CONSUMER_MAX_STREAM_ID_MINUSONE = int.MaxValue - 1;

        private readonly Dictionary<long, Item<T>> m_ItemHandleDict;
        private readonly Dictionary<int, Item<T>> m_StreamIdDict;
        private int m_NextStreamId;
        private bool m_NextStreamIdWrapAround;

        protected readonly OmmBaseImpl<T> m_OmmBaseImpl;

        private MonitorWriteLocker? m_StreamIdAccessLock;

        public ItemCallbackClient(OmmBaseImpl<T> baseImpl) : base(baseImpl, CLIENT_NAME)
        {
            m_OmmBaseImpl = baseImpl;
            int itemCountHint;

            if (commonImpl.BaseType == IOmmCommonImpl.ImpleType.CONSUMER)
               itemCountHint = (int)((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).ConsumerConfig.ItemCountHint;
            else
               itemCountHint = (int)((OmmNiProviderConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).NiProviderConfig.ItemCountHint;

            m_ItemHandleDict = new Dictionary<long, Item<T>>(itemCountHint);
            m_StreamIdDict = new Dictionary<int, Item<T>>(itemCountHint);

            m_UpdateMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmUpdateMsg();
            m_RefreshMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();
        }

#pragma warning disable CS8618
        public ItemCallbackClient(OmmServerBaseImpl serverBaseImpl) : base(serverBaseImpl, CLIENT_NAME)
#pragma warning restore CS8618
        {
            m_ItemHandleDict = new Dictionary<long, Item<T>>((int)serverBaseImpl.ConfigImpl.IProviderConfig.ItemCountHint);
            m_StreamIdDict = new Dictionary<int, Item<T>>((int)serverBaseImpl.ConfigImpl.IProviderConfig.ItemCountHint);

            m_UpdateMsg = serverBaseImpl.GetEmaObjManager().GetOmmUpdateMsg();
            m_RefreshMsg = serverBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            commonImpl.EventReceived();

            IMsg? msg = msgEvent.Msg;
            ChannelInfo channelInfo = (ChannelInfo)msgEvent.ReactorChannel!.UserSpecObj!;

            if (msg == null)
            {
                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                    StringBuilder strBuilder = commonImpl.GetStrBuilder();
                    Error error = msgEvent.ReactorErrorInfo.Error;

                    strBuilder.AppendLine("Received an item event without IMsg message")
                        .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                        .AppendLine($"\tReactor {channelInfo.Reactor.GetHashCode()}")
                        .AppendLine($"\tChannel {error.Channel?.GetHashCode()}")
                        .AppendLine($"\tError Id {error.ErrorId}")
                        .AppendLine($"\tError Location {msgEvent.ReactorErrorInfo.Location}")
                        .Append($"\tError Text {error.Text}");

                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            EventImpl.Item = (Item<T>?)(msgEvent.StreamInfo != null ? msgEvent.StreamInfo.UserSpec : null);
            EventImpl.ReactorChannel = msgEvent.ReactorChannel;

            if (EventImpl.Item == null && msg.StreamId != 1)
            {
                if (commonImpl.BaseType != IOmmCommonImpl.ImpleType.CONSUMER)
                {
                    if(m_StreamIdDict.TryGetValue(msg.StreamId, out var value) == false)
                    {
                        if (commonImpl.GetLoggerClient().IsErrorEnabled)
                        {
                            using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                            StringBuilder strBuilder = commonImpl.GetStrBuilder()
                                .AppendLine($"Received an item event without a matching stream Id {msg.StreamId}")
                                .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                                .AppendLine($"\tReactor {channelInfo.Reactor.GetHashCode()}");

                            if (msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                            {
                                strBuilder.AppendLine($"ReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                                    .Append($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine("\tReactorChannel is null");
                            }

                            commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                        }

                        return ReactorCallbackReturnCode.SUCCESS;
                    }

                    EventImpl.Item = value;
                    IProviderItem providerItem = (IProviderItem)EventImpl.Item;
                    providerItem.CancelReqTimerEvent();

                    int streamId = msg.StreamId;

                    msg = ItemWatchList.ProcessRwfMsg(msg, providerItem);

                    if(msg == null)
                    {
                        if (commonImpl.GetLoggerClient().IsErrorEnabled)
                        {
                            using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                            StringBuilder strBuilder = commonImpl.GetStrBuilder()
                                .AppendLine($"Received response is mismatch with the initlal request for stream Id {streamId}")
                                .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                                .AppendLine($"\tReactor {channelInfo.Reactor.GetHashCode()}");

                            if (msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                            {
                                strBuilder.AppendLine($"ReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                                    .Append($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                            }
                            else
                            {
                                strBuilder.AppendLine("\tReactorChannel is null");
                            }

                            commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                        }

                        return ReactorCallbackReturnCode.SUCCESS;
                    }
                }
                else
                {
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        StringBuilder strBuilder = commonImpl.GetStrBuilder()
                            .AppendLine("Received an item event without user specified pointer or stream info")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.Reactor.GetHashCode()}");

                        if (msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                        {
                            strBuilder.AppendLine($"ReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                                .Append($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                        }
                        else
                        {
                            strBuilder.AppendLine("\tReactorChannel is null");
                        }

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.SUCCESS;
                }
            }

            switch (msg.MsgClass)
            {
                case MsgClasses.UPDATE:
                    return ProcessUpdateMsg(msg, channelInfo);
                case MsgClasses.REFRESH:
                    return ProcessRefreshMsg(msg, channelInfo.ReactorChannel!, channelInfo.DataDictionary);
                case MsgClasses.ACK:
                    {
                        if (msg.StreamId == 1)
                        {
                            return m_OmmBaseImpl.LoginCallbackClient!.ProcessAckMsg(msg, channelInfo);
                        }
                        else
                        {
                            return ProcessAckMsg(msg, channelInfo);
                        }
                    }
                case MsgClasses.GENERIC:
                    {
                        if(msg.StreamId == 1)
                        {
                            return m_OmmBaseImpl.LoginCallbackClient!.ProcessGenericMsg(msg, channelInfo);
                        }
                        else
                        {
                            return ProcessGenericMsg(msg, channelInfo);
                        }
                    }
                case MsgClasses.STATUS:
                    return ProcessStatusMsg(msg, channelInfo.ReactorChannel!, channelInfo.DataDictionary);
                default:

                    if(commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        StringBuilder strBuilder = commonImpl.GetStrBuilder()
                            .AppendLine("Received an item event with message containing unhandled message class")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.Reactor?.GetHashCode()}");

                        if (msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                        {
                            strBuilder.AppendLine($"\tReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                                .Append($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                        }
                        else
                        {
                            strBuilder.AppendLine($"\tReactorChannel is null");
                        }

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ReactorCallbackReturnCode ProcessIProviderMsgCallback(ReactorMsgEvent msgEvent, DataDictionary? dataDictionary)
        {
            IMsg? msg = msgEvent.Msg!;
            ClientSession clientSession = (ClientSession)msgEvent.ReactorChannel!.UserSpecObj!;

            if (!m_StreamIdDict.TryGetValue(msg.StreamId, out var streamDict))
            {
                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = commonImpl.GetStrBuilder()
                        .AppendLine($"Received an item event without a matching stream Id {msg.StreamId}")
                        .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                        .AppendLine($"\tReactor {clientSession.Channel().Reactor!.GetHashCode()}");

                    if (msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                    {
                        temp.AppendLine($"\tReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                            .AppendLine($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                    }
                    else
                    {
                        temp.Append("RsslReactorChannel is null").Append(ILoggerClient.CR);
                    }

                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            EventImpl.Item = streamDict;
            EventImpl.clientHandle = clientSession.ClientHandle;
            EventImpl.SetOmmProvider(((OmmServerBaseImpl)commonImpl).Provider);

            IProviderItem providerItem = (IProviderItem)EventImpl.Item;
            providerItem.CancelReqTimerEvent();

            int streamId = msg.StreamId;

            msg = ItemWatchList.ProcessRwfMsg(msg, providerItem);

            if (msg is null)
            {
                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                    StringBuilder temp = commonImpl.GetStrBuilder()
                        .AppendLine($"Received response is mismatch with the initlal request for stream Id {streamId}")
                        .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                        .AppendLine($"\tReactor {clientSession.Channel().Reactor!.GetHashCode()}");

                    if (msgEvent.ReactorChannel?.Socket is not null)
                    {
                        temp.AppendLine($"\tReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                            .AppendLine($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                    }
                    else
                    {
                        temp.AppendLine("ReactorChannel is null");
                    }

                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            switch (msg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    return ProcessRefreshMsg(msg, clientSession.Channel(), dataDictionary);
                case MsgClasses.STATUS:
                    return ProcessStatusMsg(msg, clientSession.Channel(), dataDictionary);
                default:
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        StringBuilder temp = commonImpl.GetStrBuilder()
                            .AppendLine("Received an item event with message containing unhandled message class")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {msgEvent.ReactorChannel.GetHashCode()}");

                        if (msgEvent.ReactorChannel?.Socket is not null)
                        {
                            temp.AppendLine($"\tReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                                .AppendLine($"\tSocket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                        }
                        else
                        {
                            temp.AppendLine("RsslReactorChannel is null");
                        }

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, temp.ToString());
                    }
                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorCallbackReturnCode ProcessRefreshMsg(IMsg msg, ReactorChannel channelInfo, DataDictionary? dataDictionary)
        {
            m_RefreshMsg!.Decode(msg, channelInfo.MajorVersion, channelInfo.MinorVersion, dataDictionary);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item == null)
                {
                    if(commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        var strBuilder = commonImpl.GetStrBuilder()
                            .AppendLine($"Received an item event with invalid refresh message stream Id {msg.StreamId}")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.Reactor!.GetHashCode()}");

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            ServiceDirectory? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_RefreshMsg.SetServiceName(serviceDirectory.ServiceName!);
            }
            else if (EventImpl.Item.Type() == ItemType.SINGLE_ITEM)
            {
                m_RefreshMsg.SetServiceName(((SingleItem<T>)EventImpl.Item).ServiceName!);
            }
            else
            {
                m_RefreshMsg.SetServiceName(null!);
            }

            NotifyOnAllMsg(m_RefreshMsg);
            NotifyOnRefreshMsg();

            int streamState = m_RefreshMsg.m_rsslMsg.State.StreamState();

            if (streamState == StreamStates.NON_STREAMING)
            {
                if (m_RefreshMsg.m_rsslMsg.CheckRefreshComplete())
                {
                    EventImpl.Item.Remove();
                }
            }
            else if (streamState != StreamStates.OPEN)
            {
                EventImpl.Item.Remove();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorCallbackReturnCode ProcessUpdateMsg(IMsg msg, ChannelInfo channelInfo)
        {
            m_UpdateMsg!.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item == null)
                {
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        var strBuilder = commonImpl.GetStrBuilder()
                            .AppendLine($"Received an item event with invalid update message stream Id {msg.StreamId}")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            ServiceDirectory? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_UpdateMsg.SetServiceName(serviceDirectory.ServiceName!);
            }
            else if (EventImpl.Item.Type() == ItemType.SINGLE_ITEM)
            {
                m_UpdateMsg.SetServiceName(((SingleItem<T>)EventImpl.Item).ServiceName!);
            }
            else
            {
                m_UpdateMsg.SetServiceName(null!);
            }

            NotifyOnAllMsg(m_UpdateMsg);
            NotifyOnUpdateMsg();

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorCallbackReturnCode ProcessStatusMsg(IMsg msg, ReactorChannel channelInfo, DataDictionary? dataDictionary)
        {
            if (m_StatusMsg == null)
            {
                m_StatusMsg = commonImpl.GetEmaObjManager().GetOmmStatusMsg();
            }

            m_StatusMsg.Decode(msg, channelInfo.MajorVersion, channelInfo.MinorVersion, dataDictionary);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                if ((EventImpl.Item.StreamId == m_StatusMsg.StreamId()) &&
                    m_StatusMsg.HasState && m_StatusMsg.State().StreamState == StreamStates.CLOSED)
                    ((BatchItem<T>)EventImpl.Item).StatusFlag = true;

                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item == null)
                {
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        var strBuilder = commonImpl.GetStrBuilder()
                            .AppendLine($"Received an item event with invalid status message stream Id {msg.StreamId}")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.Reactor!.GetHashCode()}");

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            ServiceDirectory? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_StatusMsg.SetServiceName(serviceDirectory.ServiceName!);
            }
            else if (EventImpl.Item.Type() == ItemType.SINGLE_ITEM)
            {
                m_StatusMsg.SetServiceName(((SingleItem<T>)EventImpl.Item).ServiceName!);
            }
            else
            {
                m_StatusMsg.SetServiceName(null!);
            }

            NotifyOnAllMsg(m_StatusMsg);
            NotifyOnStatusMsg();

            if (m_StatusMsg.m_rsslMsg.CheckHasState() && m_StatusMsg.m_rsslMsg.State.StreamState() != StreamStates.OPEN )
            {
                EventImpl.Item.Remove();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorCallbackReturnCode ProcessGenericMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_GenericMsg == null)
                m_GenericMsg = commonImpl.GetEmaObjManager().GetOmmGenericMsg();

            m_GenericMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item == null)
                {
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                        var strBuilder = commonImpl.GetStrBuilder()
                            .AppendLine($"Received an item event with invalid generic message stream Id {msg.StreamId}")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            NotifyOnAllMsg(m_GenericMsg);
            NotifyOnGenericMsg();

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        private ReactorCallbackReturnCode ProcessAckMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_AckMsg == null)
                m_AckMsg = commonImpl.GetEmaObjManager().GetOmmAckMsg();

            m_AckMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if(EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item == null)
                {
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                    {
                        var strBuilder = m_OmmBaseImpl.GetStrBuilder();

                        strBuilder.AppendLine($"Received an item event with invalid ack message stream Id {msg.StreamId}")
                            .AppendLine($"\tInstance Name {commonImpl.InstanceName}")
                            .AppendLine($"\tReactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            ServiceDirectory? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_AckMsg.SetServiceName(serviceDirectory.ServiceName!);
            }
            else
            {
                m_AckMsg.SetServiceName(null!);
            }

            NotifyOnAllMsg(m_AckMsg);
            NotifyOnAckMsg();

            return ReactorCallbackReturnCode.SUCCESS;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal void Initialize()
        {
            if (commonImpl.BaseType == IOmmCommonImpl.ImpleType.CONSUMER)
            {
                m_NextStreamId = CONSUMER_STARTING_STREAM_ID;
            }
            else
            {
                m_NextStreamId = PROVIDER_STARTING_STREAM_ID;
            }

            m_NextStreamIdWrapAround = false;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long RegisterClient(RequestMsg reqMsg, T client, object? closure)
        {
            IRequestMsg requestMsg = reqMsg.m_requestMsgEncoder.m_rsslMsg;
            using var lockScope = commonImpl.GetUserLocker().EnterLockScope();

            switch (requestMsg.DomainType)
            {
                case (int)DomainType.LOGIN:
                    {
                        SingleItem<T> item = m_OmmBaseImpl.LoginCallbackClient!.CreateLoginItem(reqMsg, client, closure);

                        return AddToItemMap(m_OmmBaseImpl.NextLongId(), item);
                    }
                case (int)DomainType.DICTIONARY:
                    {
                        int nameType = requestMsg.MsgKey.NameType;
                        if ((nameType != InstrumentNameTypes.UNSPECIFIED) && (nameType != InstrumentNameTypes.RIC))
                        {
                            StringBuilder message = commonImpl.GetStrBuilder()
                                .AppendLine($"Invalid ReqMsg's name type : {nameType}.  Instance name='{commonImpl.InstanceName}'");

                            if (commonImpl.GetLoggerClient().IsErrorEnabled)
                            {
                                commonImpl.GetLoggerClient().Error(CLIENT_NAME, message.ToString());
                            }

                            commonImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                            return 0;
                        }

                        if (!requestMsg.MsgKey.CheckHasName())
                        {
                            StringBuilder message = commonImpl.GetStrBuilder()
                                .Append($"ReqMsg's name is not defined. Instance name='{commonImpl.InstanceName}'.");

                            if (commonImpl.GetLoggerClient().IsErrorEnabled)
                            {
                                commonImpl.GetLoggerClient().Error(CLIENT_NAME, message.ToString());
                            }

                            commonImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                            return 0;
                        }

                        if (commonImpl.BaseType == IOmmCommonImpl.ImpleType.CONSUMER)
                        {
                            DictionaryItem<T>? item = (DictionaryItem<T>?)commonImpl.GetEmaObjManager().m_dictionaryItemPool.Poll();
                            if (item == null)
                            {
                                item = new DictionaryItem<T>(m_OmmBaseImpl, client, closure);
                                commonImpl.GetEmaObjManager().m_dictionaryItemPool.UpdatePool(item);
                            }
                            else
                            {
                                item.ResetDictionaryItem(m_OmmBaseImpl, client, closure);
                            }

                            if (!item.OpenDictionaryItem(reqMsg))
                            {
                                RemoveFromMap(item);
                                return 0;
                            }
                            else
                            {
                                return item.ItemId;
                            }
                        }
                        else if (commonImpl.BaseType == IOmmCommonImpl.ImpleType.NIPROVIDER)
                        {
                            NiProviderDictionaryItem<T> dictionaryItem = new (m_OmmBaseImpl, client, closure);

                            if (!dictionaryItem.Open(reqMsg))
                            {
                                RemoveFromMap(dictionaryItem);
                                return 0;
                            }
                            else
                            {
                                return dictionaryItem.ItemId;
                            }
                        }
                        else
                        {
                            Item<T> dictionaryItem = (Item<T>)(object)new IProviderDictionaryItem((OmmIProviderImpl)commonImpl,
                                (IOmmProviderClient)client!, closure);

                            if (!dictionaryItem.Open(reqMsg))
                            {
                                RemoveFromMap(dictionaryItem);
                                return 0;
                            }
                            else
                            {
                                return dictionaryItem.ItemId;
                            }
                        }
                    }
                case (int)DomainType.SOURCE:
                    {
                        ChannelInfo? channelInfo = m_OmmBaseImpl.LoginCallbackClient!.ActiveChannelInfo();
                        if (channelInfo == null)
                        {
                            StringBuilder message = commonImpl.GetStrBuilder()
                                .Append($"Failed to send a directory request due to no active channel.")
                                .Append($" Instance name='{commonImpl.InstanceName}' in RegisterClient().");

                            if (commonImpl.GetLoggerClient().IsErrorEnabled)
                            {
                                commonImpl.GetLoggerClient().Error(CLIENT_NAME, message.ToString());
                            }

                            m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);

                            return 0;
                        }

                        DirectoryItem<T>? item = (DirectoryItem<T>?)commonImpl.GetEmaObjManager().m_directoryItemPool.Poll();
                        if (item == null)
                        {
                            item = new DirectoryItem<T>(m_OmmBaseImpl, client, closure);
                            commonImpl.GetEmaObjManager().m_directoryItemPool.UpdatePool(item);
                        }
                        else
                        {
                            item.ResetDirectoryItem(m_OmmBaseImpl, client, closure);
                        }

                        item.ChannelInfo = channelInfo;

                        if (!item.OpenDirectoryItem(reqMsg))
                        {
                            RemoveFromMap(item);
                            return 0;
                        }
                        else
                        {
                            return item.ItemId;
                        }
                    }
                default:
                    {
                        if (requestMsg.CheckHasBatch())
                        {
                            BatchItem<T>? batchItem = (BatchItem<T>?)commonImpl.GetEmaObjManager().m_batchItemPool.Poll();
                            if (batchItem == null)
                            {
                                batchItem = new BatchItem<T>(m_OmmBaseImpl, client, closure);
                                commonImpl.GetEmaObjManager().m_batchItemPool.UpdatePool(batchItem);
                            }
                            else
                            {
                                batchItem.ResetBatchItem(m_OmmBaseImpl, client, closure);
                            }

                            /* Start splitting the batch request into individual item request */
                            if (NextStreamIdWrapAround(reqMsg.m_requestMsgEncoder.BatchItemList!.Count))
                            {
                                List<string> itemList = reqMsg.m_requestMsgEncoder.BatchItemList;
                                SingleItem<T> item;
                                int flags = requestMsg.Flags;
                                flags &= ~RequestMsgFlags.HAS_BATCH;
                                requestMsg.Flags = flags;
                                requestMsg.MsgKey.ApplyHasName();
                                foreach (string itemName in itemList)
                                {
                                    item = new SingleItem<T>(m_OmmBaseImpl, client, closure, null);

                                    requestMsg.MsgKey.Name.Data(itemName);

                                    if (!item.Open(reqMsg))
                                    {
                                        RemoveFromMap(item);
                                        return 0;
                                    }
                                }

                                AddToItemMap(LongIdGenerator.NextLongId(), batchItem);

                                /* Send stream close status for the batch stream */
                                int keyFlags = requestMsg.MsgKey.Flags;
                                keyFlags &= ~MsgKeyFlags.HAS_NAME;
                                requestMsg.MsgKey.Flags = keyFlags;

                                batchItem.ScheduleItemClosedStatus((m_OmmBaseImpl).ItemCallbackClient!, batchItem, requestMsg, "Stream closed for batch", reqMsg.ServiceName());

                                return batchItem.ItemId;
                            }
                            else
                            {
                                batchItem.AddBatchItems(reqMsg.m_requestMsgEncoder.BatchItemList.Count);
                                var items = batchItem.SingleItemList;
                                int numOfItem = items.Count;

                                if (!batchItem.Open(reqMsg))
                                {
                                    SingleItem<T> item;
                                    for (int i = 1; i < numOfItem; i++)
                                    {
                                        item = items[i];
                                        RemoveFromMap(item);
                                    }

                                    RemoveFromMap(batchItem);
                                }
                                else
                                {
                                    return batchItem.ItemId;
                                }
                            }
                        }
                        else
                        {
                            // Single item request
                            SingleItem<T>? item = (SingleItem<T>?)commonImpl.GetEmaObjManager().m_singleItemPool.Poll();
                            if (item == null)
                            {
                                item = new SingleItem<T>(m_OmmBaseImpl, client, closure, null);
                                commonImpl.GetEmaObjManager().m_singleItemPool.UpdatePool(item);
                            }
                            else
                            {
                                item.Reset(m_OmmBaseImpl, client, closure, null);
                            }

                            if (!item.Open(reqMsg))
                            {
                                RemoveFromMap(item);
                                return 0;
                            }
                            else
                            {
                                return item.ItemId;
                            }
                        }
                    }
                    break;
            }

            return 0;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void UnregisterClient(long handle)
        {
            if (m_ItemHandleDict.TryGetValue(handle, out var item))
            {
                item.Close();
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Reissue(RequestMsg requestMsg, long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            if (item == null || item.ClosedStatusClient != null)
            {
                using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                StringBuilder strBuilder = commonImpl.GetStrBuilder()
                    .Append("Attempt to use invalid Handle on Reissue().")
                    .Append($" Instance name='{commonImpl.InstanceName}'.");

                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                }

                commonImpl.HandleInvalidHandle(handle, strBuilder.ToString());

                return;
            }

            item.Modify(requestMsg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Submit(PostMsg postMsg, long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            if (item == null || item.ClosedStatusClient != null)
            {
                using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                StringBuilder strBuilder = commonImpl.GetStrBuilder()
                    .Append("Attempt to use invalid Handle on Submit(PostMsg).")
                    .Append($" Instance name='{commonImpl.InstanceName}'.");

                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                }

                m_OmmBaseImpl.HandleInvalidHandle(handle, strBuilder.ToString());

                return;
            }

            item.Submit(postMsg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Submit(GenericMsg genericMsg, long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            if (item == null || item.ClosedStatusClient != null)
            {
                using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                StringBuilder strBuilder = commonImpl.GetStrBuilder()
                    .Append("Attempt to use invalid Handle on Submit(GenericMsg).")
                    .Append($" Instance name='{commonImpl.InstanceName}'.");

                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, strBuilder.ToString());
                }

                m_OmmBaseImpl.HandleInvalidHandle(handle, strBuilder.ToString());

                return;
            }

            item.Submit(genericMsg);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long AddToMap(long itemId, Item<T> item)
        {
            item.ItemId = itemId;
            m_ItemHandleDict[itemId] = item;
            m_StreamIdDict[item.StreamId] = item;

            if (commonImpl.GetLoggerClient().IsTraceEnabled)
            {
                using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                StringBuilder message = commonImpl.GetStrBuilder()
                    .AppendLine($"Added Item {itemId}  of StreamId {item.StreamId} to item map")
                    .Append($"\tInstance name {commonImpl.InstanceName}");

                commonImpl.GetLoggerClient().Trace(CLIENT_NAME, message.ToString());
            }

            return itemId;
        }


        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public long AddToItemMap(long itemId, Item<T> item)
        {
            item.ItemId = itemId;
            m_ItemHandleDict[itemId] = item;

            if (commonImpl.GetLoggerClient().IsTraceEnabled)
            {
                using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                StringBuilder message = commonImpl.GetStrBuilder()
                    .AppendLine($"Added Item {itemId} to item map")
                    .Append($"\tInstance name {commonImpl.InstanceName}");

                commonImpl.GetLoggerClient().Trace(CLIENT_NAME, message.ToString());
            }

            return itemId;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public Item<T>? GetItem(long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            return item;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void RemoveFromMap(Item<T> item)
        {
            if (commonImpl.GetLoggerClient().IsTraceEnabled)
            {
                using var lockScope = commonImpl.GetUserLocker().EnterLockScope();
                StringBuilder message = commonImpl.GetStrBuilder();
                if (item.StreamId != 0)
                    message.AppendLine($"Removed Item {item.ItemId} of StreamId {item.StreamId} from item map")
                        .Append($"\tInstance name {commonImpl.InstanceName}");
                else
                    message.AppendLine($"Removed Item {item.ItemId} from item map")
                        .Append($"\tInstance name {commonImpl.InstanceName}");

                commonImpl.GetLoggerClient().Trace(CLIENT_NAME, message.ToString());
            }

            if (item.ItemId != 0)
            {
                m_ItemHandleDict.Remove(item.ItemId);
            }

            if (item.StreamId != 0)
            {
                m_StreamIdDict.Remove(item.StreamId);
            }

            item.BackToPool();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        bool IsStreamIdInUse(int nextStreamId)
        {
            return m_StreamIdDict.ContainsKey(nextStreamId);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public int NextStreamId(int numOfItem)
        {
            if (m_NextStreamId > CONSUMER_MAX_STREAM_ID_MINUSONE - numOfItem)
            {
                if (commonImpl.BaseType == IOmmCommonImpl.ImpleType.CONSUMER)
                {
                    m_NextStreamId = CONSUMER_STARTING_STREAM_ID;
                }
                else
                {
                    m_NextStreamId = PROVIDER_STARTING_STREAM_ID;
                }

                m_NextStreamIdWrapAround = true;

                if (m_StreamIdAccessLock == null)
                    m_StreamIdAccessLock = new MonitorWriteLocker(new object());

                if (commonImpl.GetLoggerClient().IsTraceEnabled)
                    commonImpl.GetLoggerClient().Trace(CLIENT_NAME,
                            "Reach max number available for next stream id, will wrap around");
            }

            if (!m_NextStreamIdWrapAround)
            {
                if (numOfItem > 0)
                {
                    int retVal = ++m_NextStreamId;
                    m_NextStreamId += numOfItem;
                    return retVal;
                }

                return ++m_NextStreamId;
            }
            else
            {
                m_StreamIdAccessLock?.Enter();
                while (IsStreamIdInUse(++m_NextStreamId)) ;
                m_StreamIdAccessLock?.Exit();

                if (m_NextStreamId < 0)
                {
                    var message = "Unable to obtain next available stream id for item request.";
                    if (commonImpl.GetLoggerClient().IsErrorEnabled)
                        commonImpl.GetLoggerClient().Error(CLIENT_NAME, message);

                    commonImpl.HandleInvalidUsage(message, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                return m_NextStreamId;
            }
        }

        public bool NextStreamIdWrapAround(int numOfItem)
        {
            return m_NextStreamId > (CONSUMER_MAX_STREAM_ID_MINUSONE - numOfItem);
        }
    }

    internal sealed class ItemCallbackClientConsumer : ItemCallbackClient<IOmmConsumerClient>
    {
        private static readonly string CLIENT_NAME = "ItemCallbackClientConsumer";

        public ItemCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl) : base(baseImpl)
        {
            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)baseImpl;
            EventImpl.SetOmmConsumer(ommConsumerImpl.Consumer);

            NotifyOnAllMsg = NotifyOnAllMsgImpl;
            NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
            NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
            NotifyOnGenericMsg = NotifyOnGenericMsgImpl;
            NotifyOnUpdateMsg = NotifyOnUpdateMsgImpl;
            NotifyOnAckMsg = NotifyOnAckMsgImpl;
        }

        public void NotifyOnAllMsgImpl(Msg msg)
        {
            if(EventImpl.Item?.Client == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.AppendLine($"An incoming Msg to non-existent IOmmConsumerClient has been dropped.");
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }
            }
            else
            {
                EventImpl.Item?.Client.OnAllMsg(msg, EventImpl);
            }
        }

        public void NotifyOnRefreshMsgImpl()
        {
            if (EventImpl.Item?.Client == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.AppendLine($"An incoming RefreshMsg to non-existent IOmmConsumerClient has been dropped.");
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }
            }
            else
            {
                EventImpl.Item?.Client.OnRefreshMsg(m_RefreshMsg!, EventImpl);
            }
        }

        public void NotifyOnUpdateMsgImpl()
        {
            if (EventImpl.Item?.Client == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.AppendLine($"An incoming UpdateMsg to non-existent IOmmConsumerClient has been dropped.");
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }
            }
            else
            {
                EventImpl.Item?.Client.OnUpdateMsg(m_UpdateMsg!, EventImpl);
            }
        }

        public void NotifyOnStatusMsgImpl()
        {
            if (EventImpl.Item?.Client == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.AppendLine($"An incoming StatusMsg to non-existent IOmmConsumerClient has been dropped.");
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }
            }
            else
            {
                EventImpl.Item?.Client.OnStatusMsg(m_StatusMsg!, EventImpl);
            }
        }

        public void NotifyOnGenericMsgImpl()
        {
            if (EventImpl.Item?.Client == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.AppendLine($"An incoming GenericMsg to non-existent IOmmConsumerClient has been dropped.");
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }
            }
            else
            {
                EventImpl.Item?.Client.OnGenericMsg(m_GenericMsg!, EventImpl);
            }
        }

        public void NotifyOnAckMsgImpl()
        {
            if (EventImpl.Item?.Client == null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.AppendLine($"An incoming AckMsg to non-existent IOmmConsumerClient has been dropped.");
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                }
            }
            else
            {
                EventImpl.Item?.Client.OnAckMsg(m_AckMsg!, EventImpl);
            }
        }
    }

    internal class ItemCallbackClientProvider : ItemCallbackClient<IOmmProviderClient>
    {
        private static readonly string CLIENT_NAME = "ItemCallbackClientProvider";

        public ItemCallbackClientProvider(OmmBaseImpl<IOmmProviderClient> baseImpl) : base(baseImpl)
        {
            OmmNiProviderImpl ommNiProviderImpl = (OmmNiProviderImpl)baseImpl;
            EventImpl.SetOmmProvider(ommNiProviderImpl.Provider);

            NotifyOnAllMsg = NotifyOnAllMsgImpl;
            NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
            NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
            NotifyOnGenericMsg = NotifyOnGenericMsgImpl;
        }

        public ItemCallbackClientProvider(OmmServerBaseImpl ommServerBaseImpl) : base(ommServerBaseImpl)
        {
            EventImpl.SetOmmProvider(ommServerBaseImpl.Provider);

            NotifyOnAllMsg = NotifyOnAllMsgImpl;
            NotifyOnRefreshMsg = NotifyOnRefreshMsgImpl;
            NotifyOnStatusMsg = NotifyOnStatusMsgImpl;
            NotifyOnGenericMsg = NotifyOnGenericMsgImpl;
        }

        public void NotifyOnAllMsgImpl(Msg msg)
        {
            if (EventImpl.Item?.Client is null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    var message = "An incoming Msg to non-existent IOmmProviderClient has been dropped.";
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message);
                }
            }
            else
            {
                EventImpl.Item?.Client.OnAllMsg(msg, EventImpl);
            }
        }

        public void NotifyOnRefreshMsgImpl()
        {
            if (EventImpl.Item?.Client is null)
            {
                if (commonImpl.GetLoggerClient().IsErrorEnabled)
                {
                    var message = "An incoming RefreshMsg to non-existent IOmmProviderClient has been dropped.";
                    commonImpl.GetLoggerClient().Error(CLIENT_NAME, message);
                }
            }
            else
            {
                EventImpl.Item?.Client.OnRefreshMsg(m_RefreshMsg!, EventImpl);
            }
        }

        public void NotifyOnStatusMsgImpl()
        {
            if (EventImpl.Item?.Client is null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    var message = "An incoming StatusMsg to non-existent IOmmProviderClient has been dropped.";
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message);
                }
            }
            else
            {
                EventImpl.Item?.Client.OnStatusMsg(m_StatusMsg!, EventImpl);
            }
        }

        public void NotifyOnGenericMsgImpl()
        {
            if (EventImpl.Item?.Client is null)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    var message = "An incoming GenericMsg to non-existent IOmmProviderClient has been dropped.";
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message);
                }
            }
            else
            {
                EventImpl.Item?.Client.OnGenericMsg(m_GenericMsg!, EventImpl);
            }
        }
    }

    internal interface IProviderItem
    {
        public void ScheduleItemClosedRecoverableStatus(string statusText, bool initiateTimeout);

        public MsgKey MsgKey { get; }

        public ClientSession? ClientSession { get; }

        public void SendCloseMsg();

        public int ServiceId { get; }

        public TimeoutEvent? ReqTimeoutEvent { get; }

        public bool ProcessInitialResp(IRefreshMsg refreshMsg);

        public ItemWatchList ItemWatchList { get; }

        public void CancelReqTimerEvent();

        public abstract bool RequestWithService();

        public Locker UserLock { get; }

        public LinkedListNode<IProviderItem>? ItemListNode { get; set; }
    }
}
