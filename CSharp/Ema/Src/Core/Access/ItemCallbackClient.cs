/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.Rdm;
using System.Text;
using System.Collections.Generic;
using LSEG.Eta.Common;
using System.Threading;
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

    internal abstract class Item<T> : VaNode
    {
        public int DomainType { get; set; }
        public int StreamId { get; protected set; }
        public object? Closure { get; private set; }
        public T? Client { get; private set; }

        public Item<T>? Parent { get; private set; }

        public long ItemId { get; set; }
        public ClosedStatusClient<T>? ClosedStatusClient { get; set; }

        public Item() { }

        public Item(T? client, object? closure, Item<T>? parent)
        {
            DomainType = 0;
            StreamId = 0;
            Closure = closure;
            Client = client;
            Parent = parent;
        }

        public virtual void Reset(T client, object? closure, Item<T>? parent)
        {
            DomainType = 0;
            StreamId = 0;
            Closure = closure;
            Client = client;
            Parent = parent;
            ClosedStatusClient = null;
        }

        public void BackToPool()
        {
            Closure = null;
            Client = default;
            ClosedStatusClient = null;

            base.ReturnToPool();
        }

        public void SetId(long handle, int streamId)
        {
            ItemId = handle;
            StreamId = streamId;
        }

        public abstract bool Open(RequestMsg reqMsg);
        public abstract bool Modify(RequestMsg reqMsg);
        public abstract bool Close();
        public abstract void Remove();
        public abstract ItemType Type();
        public abstract ServiceDirectory? Directory();
        public abstract int GetNextStreamId(int numOfItem);

        public virtual bool Submit(RefreshMsg refreshMsg)
        {
            return false;
        }
        public virtual bool Submit(UpdateMsg updateMsg)
        {
            return false;
        }
        public virtual bool Submit(StatusMsg statusMsg)
        {
            return false;
        }
        public virtual bool Submit(PostMsg postMsg)
        {
            return false;
        }
        public virtual bool Submit(GenericMsg genericMsg)
        {
            return false;
        }
    }

    internal class SingleItem<T> : Item<T>
    {
        private static readonly string CLIENT_NAME = "SingleItem";
        internal ServiceDirectory? m_ServiceDirectory { get; set; }
        internal string? ServiceName { get; set; }
        internal OmmBaseImpl<T> m_OmmBaseImpl;

#pragma warning disable CS8618 
        public SingleItem() { }
#pragma warning restore CS8618

        public SingleItem(OmmBaseImpl<T> baseImpl, T? client, object? closure, Item<T>? batchItem)
            : base(client, closure, batchItem)
        {
            m_OmmBaseImpl = baseImpl;
        }

        public virtual void Reset(OmmBaseImpl<T> baseImpl, T client, object? closure, Item<T>? batchItem)
        {
            base.Reset(client, closure, batchItem);
            m_OmmBaseImpl = baseImpl;

            m_ServiceDirectory = null;
        }

        public override bool Close()
        {
            ICloseMsg closeMsg = m_OmmBaseImpl.ItemCallbackClient!.CloseMsg();
            closeMsg.ContainerType = DataTypes.NO_DATA;
            closeMsg.DomainType = DomainType;

            bool retCode = Submit(closeMsg);

            Remove();
            return retCode;
        }

        public override ServiceDirectory? Directory()
        {
            return m_ServiceDirectory;
        }

        public override int GetNextStreamId(int numOfItem)
        {
            return m_OmmBaseImpl.ItemCallbackClient!.NextStreamId(numOfItem);
        }

        public override bool Modify(RequestMsg reqMsg)
        {
            return Submit(reqMsg.m_rsslMsg as IRequestMsg);
        }

        public override bool Open(RequestMsg reqMsg)
        {
            ServiceDirectory? service = null;

            if (reqMsg.HasServiceName)
            {
                service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(reqMsg.ServiceName());

                if (service is null && (!m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh.LoginAttrib.HasSingleOpen
                    || m_OmmBaseImpl.LoginCallbackClient.LoginRefresh.LoginAttrib.SingleOpen == 0))
                {
                    /* This ensures that the user will get a valid handle.  The callback should clean it up after. */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append($"Service name of '{reqMsg.ServiceName()}' is not found.");

                    ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                                message.ToString(), reqMsg.ServiceName());

                    return true;
                }
            }
            else if (reqMsg.HasServiceId)
            {
                service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(reqMsg.ServiceId());

                if (service is null && (!m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh.LoginAttrib.HasSingleOpen
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

            if (m_ServiceDirectory != null)
            {
                ServiceName = m_ServiceDirectory.ServiceName;
            }
            else
            {
                ServiceName = reqMsg.HasServiceName ? reqMsg.ServiceName() : null;
            }

            return Submit(reqMsg.m_requestMsgEncoder.m_rsslMsg as IRequestMsg);
        }

        public override void Remove()
        {
           if(Type() != ItemType.BATCH_ITEM)
           {
                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this);
           }
        }

        public override ItemType Type()
        {
            return ItemType.SINGLE_ITEM;
        }

        public override bool Submit(RefreshMsg refreshMsg)
        {
            return Submit(refreshMsg.m_rsslMsg as IRefreshMsg, null);
        }
        public override bool Submit(UpdateMsg updateMsg)
        {
            return Submit(updateMsg.m_rsslMsg as IUpdateMsg, null);
        }
        public override bool Submit(StatusMsg statusMsg)
        {
            return Submit(statusMsg.m_rsslMsg as IStatusMsg, null);
        }
        public override bool Submit(PostMsg postMsg)
        {
            string? serviceName = postMsg.HasServiceName ? postMsg.ServiceName() : null;

            if(!ValidateServiceName(serviceName))
            {
                return false;
            }

            return Submit(postMsg.m_rsslMsg as IPostMsg, serviceName);
        }
        public override bool Submit(GenericMsg genericMsg)
        {
            return Submit(genericMsg.m_rsslMsg as IGenericMsg, null);
        }

        protected bool Submit(ICloseMsg closeMsg)
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.SubmitOptions;
            submitOptions.ServiceName = null;

            submitOptions.RequestMsgOptions.UserSpecObj = this;

            if(StreamId == 0)
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

            ReactorChannel reactorChannel = m_OmmBaseImpl.LoginCallbackClient!.ActiveChannelInfo()!.ReactorChannel!;
            ReactorReturnCode retCode = reactorChannel.Submit((Eta.Codec.Msg)closeMsg, submitOptions, out ReactorErrorInfo? errorInfo);

            if(retCode < ReactorReturnCode.SUCCESS)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    strBuilder.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(ICloseMsg)")
                    .AppendLine($"Channel {errorInfo?.Error.Channel}")
                    .AppendLine($"Error Id {errorInfo?.Error.ErrorId}")
                    .AppendLine($"Internal SysError {errorInfo?.Error.SysError}")
                    .AppendLine($"Error Location {errorInfo?.Location}")
                    .AppendLine($"Error Text {errorInfo?.Error.Text}");

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

        protected bool Submit(IRequestMsg requestMsg)
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.SubmitOptions;

            bool removeServiceIDFlag = false;

            if(m_ServiceDirectory is not null)
            {
                if(requestMsg.MsgKey.CheckHasServiceId())
                {
                    requestMsg.MsgKey.Flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
                    removeServiceIDFlag = true;
                }

                submitOptions.ServiceName = m_ServiceDirectory.ServiceName;
            }
            else
            {
                submitOptions.ServiceName = ServiceName;
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

            if (m_OmmBaseImpl.ConfigImpl.ConsumerConfig.MsgKeyInUpdates)
            {
                requestMsg.ApplyMsgKeyInUpdates();
            }

            submitOptions.RequestMsgOptions.UserSpecObj = this;

            int domainType = requestMsg.DomainType;

            if(StreamId == 0)
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
                        m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), item);
                    }
                }
                else
                {
                    StreamId = GetNextStreamId(0);
                    requestMsg.StreamId = StreamId;
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);
                }
            }
            else
            {
                requestMsg.StreamId = StreamId;
            }

            if(DomainType == 0)
            {
                DomainType = domainType;
            }
            else
            {
                requestMsg.DomainType = DomainType;
            }

            ReactorChannel? reactorChannel = m_OmmBaseImpl.LoginCallbackClient?.ActiveChannelInfo()?.ReactorChannel;
 
            if(reactorChannel != null)
            {
                ReactorReturnCode ret = reactorChannel.Submit((Eta.Codec.Msg)requestMsg, submitOptions,
                    out var ErrorInfo);

                if (ret < ReactorReturnCode.SUCCESS)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        Error? error = ErrorInfo?.Error;

                        message.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(IRequestMsg requestMsg)")
                        .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"Error Id {error?.ErrorId}")
                            .AppendLine($"Internal sysError {error?.SysError}")
                            .AppendLine($"Error Location {ErrorInfo?.Location}")
                            .Append($"Error Text {error?.Text}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                        message.Clear();
                    }

                    message.Append("Failed to open or modify item request. Reason: ")
                        .Append(ret)
                        .Append(". Error text: ")
                        .Append(ErrorInfo?.Error.Text);

                    if(removeServiceIDFlag)
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
                    .AppendLine($"ReactorChannel is not avaliable");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append("Failed to open or modify item request. Reason: ReactorChannel is not avaliable");

                if (removeServiceIDFlag)
                {
                    requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
                }

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);

                return false;
            }

            if (removeServiceIDFlag)
            {
                requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
            }

            return true;
        }

        protected bool Submit<V>(V submitMsg, string? serviceName) where V : IMsg
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.SubmitOptions;
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
                ReactorReturnCode ret = reactorChannel.Submit(submitMsg, submitOptions, out var ErrorInfo);

                if (ret < ReactorReturnCode.SUCCESS)
                {
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        Error? error = ErrorInfo?.Error;

                        message.Append($"Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(IMsg submitMsg) " +
                            $"for {submitMsg.GetType().Name}")
                        .AppendLine($"Channel {error?.Channel?.GetHashCode()}")
                            .AppendLine($"Error Id {error?.ErrorId}")
                            .AppendLine($"Internal sysError {error?.SysError}")
                            .AppendLine($"Error Location {ErrorInfo?.Location}")
                            .Append($"Error Text {error?.Text}");

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
                    .AppendLine($"ReactorChannel is not avaliable");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append($"Failed to send {submitMsg.GetType().Name}. Reason: ReactorChannel is not avaliable");

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);

                return false;
            }

            return true;
        }

        private bool ValidateServiceName(string? serviceName)
        {
            if(serviceName is null || m_OmmBaseImpl.DirectoryCallbackClient!.GetService(serviceName) != null)
            {
                return true;
            }

            StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
            if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                strBuilder.AppendLine("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(PostMsg)")
                    .AppendLine($"Error Id {ReactorReturnCode.INVALID_USAGE}")
                    .AppendLine("Error Location ItemCallbackClient.Submit(IPostMsg,String)")
                    .AppendLine($"Error Text Message submitted with unknown service name {serviceName}");

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                strBuilder.Clear();
            }

            strBuilder.Append("Failed to submit PostMsg on item stream. Reason: ")
                       .Append(ReactorReturnCode.INVALID_USAGE)
                       .Append($". Error text: Message submitted with unknown service name {serviceName}");

            m_OmmBaseImpl.HandleInvalidUsage(strBuilder.ToString(), (int)ReactorReturnCode.INVALID_USAGE);
            return false;
        }

        public void ScheduleItemClosedStatus(CallbackClient<T> client, Item<T> item, IMsg rsslMsg, string statusText, 
            string? serviceName)
        {
            if (ClosedStatusClient != null) return;

            ClosedStatusClient = new ClosedStatusClient<T>(client, item, rsslMsg, statusText, serviceName);
            m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(1000, ClosedStatusClient);
        }
    }

    internal class BatchItem<T> : SingleItem<T>
    {
        private static readonly string CLIENT_NAME = "BatchItem";
        
        internal List<SingleItem<T>> SingleItemList { get; set; } = new();

        internal int ItemCount { get; set; }

        internal bool StatusFlag { get; set; }

        public BatchItem() { }

        public BatchItem(OmmBaseImpl<T> baseImpl, T client, object? closure)
            : base(baseImpl, client, closure, null)
        {
            ItemCount = 1;
        }

        public override void Reset(OmmBaseImpl<T> baseImpl, T client, object? closure, Item<T>? batchItem)
        {
            base.Reset(baseImpl, client, closure, null);
            SingleItemList.Clear();
            ItemCount = 1;
        }

        public override ItemType Type()
        {
            return ItemType.BATCH_ITEM;
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
            if( --ItemCount == 0 && StatusFlag)
            {
                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this);
            }
        }

        public override void Remove()
        {
            if(ItemCount == 0)
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

            if (client.m_StatusMsg is null)
                client.m_StatusMsg = new StatusMsg();

            client.m_StatusMsg.Decode(statusMsg, Codec.MajorVersion(), Codec.MajorVersion(), null!);

            client.m_StatusMsg.SetServiceName(serviceName!);
            client.EventImpl.Item = item;
            client.NotifyOnAllMsg(client.m_StatusMsg!);
            client.NotifyOnStatusMsg();
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
        private Locker? m_StreamIdAccessLock;

        public ItemCallbackClient(OmmBaseImpl<T> baseImpl) : base(baseImpl, CLIENT_NAME)
        {
            m_OmmBaseImpl = baseImpl;

            int itemCountHint = (int)m_OmmBaseImpl.ConfigImpl.ConsumerConfig.ItemCountHint;
            m_ItemHandleDict = new Dictionary<long, Item<T>>(itemCountHint);
            m_StreamIdDict = new Dictionary<int, Item<T>>(itemCountHint);

            m_UpdateMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmUpdateMsg();
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            m_OmmBaseImpl.EventReceived();

            IMsg? msg = msgEvent.Msg;
            ChannelInfo channelInfo = (ChannelInfo)msgEvent.ReactorChannel!.UserSpecObj!;

            if(msg is null)
            {
                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                    Error error = msgEvent.ReactorErrorInfo.Error;

                    strBuilder.AppendLine("Received an item event without IMsg message")
                        .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                        .AppendLine($"Reactor {channelInfo.Reactor.GetHashCode()}")
                        .AppendLine($"Channel {error.Channel?.GetHashCode()}")
                        .AppendLine($"Error Id {error.ErrorId}")
                        .AppendLine($"Error Location {msgEvent.ReactorErrorInfo.Location}")
                        .Append($"Error Text {error.Text}");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }

                return ReactorCallbackReturnCode.SUCCESS;
            }

            EventImpl.Item = (Item<T>?)(msgEvent.StreamInfo != null ? msgEvent.StreamInfo.UserSpec : null);
            EventImpl.ReactorChannel = msgEvent.ReactorChannel;

            if(EventImpl.Item is null && msg.StreamId != 1)
            {
                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();

                    strBuilder.AppendLine("Received an item event without user specified pointer or stream info")
                        .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                        .AppendLine($"Reactor {channelInfo.Reactor.GetHashCode()}");
                        
                    if(msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                    {
                        strBuilder.AppendLine($"ReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                            .Append($"Socket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                    }
                    else
                    {
                        strBuilder.AppendLine($"ReactorChannel is null");
                    }

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }
            }

            switch(msg.MsgClass)
            {
                case MsgClasses.ACK:
                    {
                        if(msg.StreamId == 1)
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
                case MsgClasses.REFRESH:
                    return ProcessRefreshMsg(msg, channelInfo);
                case MsgClasses.STATUS:
                    return ProcessStatusMsg(msg, channelInfo);
                case MsgClasses.UPDATE:
                    return ProcessUpdateMsg(msg, channelInfo);
                default:

                    if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                        strBuilder.AppendLine("Received an item event with message containing unhandled message class")
                            .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                            .AppendLine($"Reactor {channelInfo.Reactor?.GetHashCode()}");

                        if (msgEvent.ReactorChannel != null && msgEvent.ReactorChannel.Socket != null)
                        {
                            strBuilder.AppendLine($"ReactorChannel {msgEvent.ReactorChannel.GetHashCode()}")
                                .Append($"Socket {msgEvent.ReactorChannel.Socket.GetHashCode()}");
                        }
                        else
                        {
                            strBuilder.AppendLine($"ReactorChannel is null");
                        }

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    break;
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private ReactorCallbackReturnCode ProcessRefreshMsg(IMsg msg,ChannelInfo channelInfo)
        {
            if (m_RefreshMsg is null)
            {
                m_RefreshMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmRefreshMsg();
            }

            m_RefreshMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if(EventImpl.Item is null)
                {
                    if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = m_OmmBaseImpl.GetStrBuilder();

                        strBuilder.AppendLine($"Received an item event with invalid refresh message stream Id {msg.StreamId}")
                            .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                            .AppendLine($"Reactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
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

            if(streamState == StreamStates.NON_STREAMING)
            {
                if(m_RefreshMsg.m_rsslMsg.CheckRefreshComplete())
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

        private ReactorCallbackReturnCode ProcessUpdateMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_UpdateMsg is null)
            {
                m_UpdateMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmUpdateMsg();
            }

            m_UpdateMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item is null)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = m_OmmBaseImpl.GetStrBuilder();

                        strBuilder.AppendLine($"Received an item event with invalid update message stream Id {msg.StreamId}")
                            .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                            .AppendLine($"Reactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
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

            if (m_OmmBaseImpl.ConfigImpl.ConsumerConfig.MsgKeyInUpdates)
            {
                Eta.Codec.Msg decodeMsg = m_UpdateMsg.m_rsslMsg;

                m_UpdateMsg.m_rsslMsg = (Eta.Codec.Msg)msg;

                NotifyOnAllMsg(m_UpdateMsg);
                NotifyOnUpdateMsg();

                m_UpdateMsg.m_rsslMsg = decodeMsg;
            }
            else
            {
                NotifyOnAllMsg(m_UpdateMsg);
                NotifyOnUpdateMsg();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private ReactorCallbackReturnCode ProcessStatusMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if(m_StatusMsg is null)
            {
                m_StatusMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmStatusMsg();
            }

            m_StatusMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                if ((EventImpl.Item.StreamId == m_StatusMsg.StreamId()) &&
                    m_StatusMsg.HasState && m_StatusMsg.State().StreamState == StreamStates.CLOSED)
                    ((BatchItem<T>)EventImpl.Item).StatusFlag = true;

                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item is null)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = m_OmmBaseImpl.GetStrBuilder();

                        strBuilder.AppendLine($"Received an item event with invalid status message stream Id {msg.StreamId}")
                            .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                            .AppendLine($"Reactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
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

            if(m_StatusMsg.m_rsslMsg.CheckHasState() && m_StatusMsg.m_rsslMsg.State.StreamState() != StreamStates.OPEN )
            {
                EventImpl.Item.Remove();
            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private ReactorCallbackReturnCode ProcessGenericMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_GenericMsg is null)
                m_GenericMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmGenericMsg();

            m_GenericMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item is null)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = m_OmmBaseImpl.GetStrBuilder();

                        strBuilder.AppendLine($"Received an item event with invalid generic message stream Id {msg.StreamId}")
                            .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                            .AppendLine($"Reactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                    }

                    return ReactorCallbackReturnCode.FAILURE;
                }
            }

            NotifyOnAllMsg(m_GenericMsg);
            NotifyOnGenericMsg();

            return ReactorCallbackReturnCode.SUCCESS;
        }

        private ReactorCallbackReturnCode ProcessAckMsg(IMsg msg, ChannelInfo channelInfo)
        {
            if (m_AckMsg is null)
                m_AckMsg = m_OmmBaseImpl.GetEmaObjManager().GetOmmAckMsg();

            m_AckMsg.Decode(msg, channelInfo.ReactorChannel!.MajorVersion, channelInfo.ReactorChannel!.MinorVersion,
                channelInfo.DataDictionary!);

            if(EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                EventImpl.Item = ((BatchItem<T>)EventImpl.Item).GetSingleItem(msg.StreamId);

                if (EventImpl.Item is null)
                {
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                    {
                        var strBuilder = m_OmmBaseImpl.GetStrBuilder();

                        strBuilder.AppendLine($"Received an item event with invalid ack message stream Id {msg.StreamId}")
                            .AppendLine($"Instance Name {m_OmmBaseImpl.InstanceName}")
                            .AppendLine($"Reactor {channelInfo.ReactorChannel!.Reactor!.GetHashCode()}");

                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
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

        internal void Initialize()
        {
            if (m_OmmBaseImpl.GetImplType() == IOmmCommonImpl.ImpleType.CONSUMER)
            {
                m_NextStreamId = CONSUMER_STARTING_STREAM_ID;
            }
            else
            {
                m_NextStreamId = PROVIDER_STARTING_STREAM_ID;
            }

            m_NextStreamIdWrapAround = false;
        }

        public long RegisterClient(RequestMsg reqMsg, T client, object? closure)
        {
            IRequestMsg requestMsg = reqMsg.m_requestMsgEncoder.m_rsslMsg;

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
                            StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                            message.AppendLine($"Invalid ReqMsg's name type : {nameType}.  Instance name='{m_OmmBaseImpl.InstanceName}'");

                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                            }

                            m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                            return 0;
                        }

                        if (!requestMsg.MsgKey.CheckHasName())
                        {
                            StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                            message.Append($"ReqMsg's name is not defined. Instance name='{m_OmmBaseImpl.InstanceName}'.");

                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                            }

                            m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                            return 0;
                        }

                        // RTSDK-7263 todo: pool DictionaryItem instances
                        Item<T> item = new DictionaryItem<T>(m_OmmBaseImpl, client, closure);

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
                case (int)DomainType.SOURCE:
                    {
                        ChannelInfo? channelInfo = m_OmmBaseImpl.LoginCallbackClient!.ActiveChannelInfo();
                        if(channelInfo is null)
                        {
                            StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                            message.Append($"Failed to send a directory request due to no active channel." +
                                $" Instance name='{m_OmmBaseImpl.InstanceName}' in RegisterClient().");

                            if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                            {
                                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());
                            }

                            m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.NO_ACTIVE_CHANNEL);

                            return 0;
                        }

                        DirectoryItem<T> item = new (m_OmmBaseImpl, client, closure); // TODO: Reuse this object from the EmaObjectManager

                        item.ChannelInfo = channelInfo;

                        if(!item.Open(reqMsg))
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
                        if(requestMsg.CheckHasBatch())
                        {
                            BatchItem<T> batchItem = new BatchItem<T>(m_OmmBaseImpl, client, closure);

                            /* Start splitting the batch request into individual item request */
                            if(NextStreamIdWrapAround(reqMsg.m_requestMsgEncoder.BatchItemList!.Count))
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
                                batchItem.ScheduleItemClosedStatus((m_OmmBaseImpl).ItemCallbackClient!,
                                        batchItem, requestMsg, "Stream closed for batch", reqMsg.ServiceName());

                                return batchItem.ItemId;
                            }
                            else
                            {
                                batchItem.AddBatchItems(reqMsg.m_requestMsgEncoder.BatchItemList.Count);
                                var items = batchItem.SingleItemList;
                                int numOfItem = items.Count;

                                if(!batchItem.Open(reqMsg))
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
                            SingleItem<T> item = new SingleItem<T>(m_OmmBaseImpl, client, closure, null);

                            if(!item.Open(reqMsg))
                            {
                                RemoveFromMap(item);
                                return 0;
                            }
                            else
                            {
                                return item.ItemId;
                            }
                        }

                        break;
                    }
            }

            return 0;
        }

        public void UnregisterClient(long handle)
        {
            if(m_ItemHandleDict.TryGetValue(handle, out var item))
            {
                item.Close();
            }
        }

        public void Reissue(RequestMsg requestMsg, long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            if(item is null || item.ClosedStatusClient != null)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                strBuilder.AppendLine($"Attempt to use invalid Handle on Reissue()." +
                    $" Instance name='{m_OmmBaseImpl.InstanceName}'.");

                if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }

                m_OmmBaseImpl.HandleInvalidHandle(handle, strBuilder.ToString());

                return;
            }

            item.Modify(requestMsg);
        }

        public void Submit(PostMsg postMsg, long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            if (item is null || item.ClosedStatusClient != null)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                strBuilder.AppendLine($"Attempt to use invalid Handle on Submit(PostMsg)." +
                    $" Instance name='{m_OmmBaseImpl.InstanceName}'.");

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }

                m_OmmBaseImpl.HandleInvalidHandle(handle, strBuilder.ToString());

                return;
            }

            item.Submit(postMsg);
        }

        public void Submit(GenericMsg genericMsg, long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            if (item is null || item.ClosedStatusClient != null)
            {
                StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
                strBuilder.AppendLine($"Attempt to use invalid Handle on Submit(GenericMsg)." +
                    $" Instance name='{m_OmmBaseImpl.InstanceName}'.");

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                }

                m_OmmBaseImpl.HandleInvalidHandle(handle, strBuilder.ToString());

                return;
            }

            item.Submit(genericMsg);
        }

        public long AddToMap(long itemId, Item<T> item)
        {
            item.ItemId = itemId;
            m_ItemHandleDict[itemId] = item;
            m_StreamIdDict[item.StreamId] = item;

            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                message.AppendLine($"Added Item {itemId}  of StreamId {item.StreamId} to item map")
                .Append($"Instance name {m_OmmBaseImpl.InstanceName}");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
            }

            return itemId;
        }

        public long AddToItemMap(long itemId, Item<T> item)
        {
            item.ItemId = itemId;
            m_ItemHandleDict[itemId] = item;

            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                message.AppendLine($"Added Item {itemId} to item map").Append($"Instance name {m_OmmBaseImpl.InstanceName}");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
            }

            return itemId;
        }

        public Item<T>? GetItem(long handle)
        {
            m_ItemHandleDict.TryGetValue(handle, out var item);

            return item;
        }

        public void RemoveFromMap(Item<T> item)
        {
            if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                if (item.StreamId != 0)
                    message.AppendLine($"Removed Item {item.ItemId} of StreamId {item.StreamId} from item map")
                    .Append($"Instance name {m_OmmBaseImpl.InstanceName}");
                else
                    message.AppendLine($"Removed Item {item.ItemId} from item map")
                    .Append($"Instance name {m_OmmBaseImpl.InstanceName}");

                m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME, message.ToString());
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

        bool IsStreamIdInUse(int nextStreamId)
        {
            return (m_StreamIdDict.ContainsKey(nextStreamId));
        }

        public int NextStreamId(int numOfItem)
        {
            if (m_NextStreamId > CONSUMER_MAX_STREAM_ID_MINUSONE - numOfItem)
            {
                if (m_OmmBaseImpl.GetImplType() == IOmmCommonImpl.ImpleType.CONSUMER)
                {
                    m_NextStreamId = CONSUMER_STARTING_STREAM_ID;
                }
                else
                {
                    m_NextStreamId = PROVIDER_STARTING_STREAM_ID;
                }

                m_NextStreamIdWrapAround = true;

                if (m_StreamIdAccessLock is null)
                    m_StreamIdAccessLock = new WriteLocker(new ReaderWriterLockSlim(LockRecursionPolicy.SupportsRecursion));

                if (m_OmmBaseImpl.LoggerClient.IsTraceEnabled)
                    m_OmmBaseImpl.LoggerClient.Trace(CLIENT_NAME,
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
                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append("Unable to obtain next available stream id for item request.");
                    if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                        m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                return m_NextStreamId;
            }
        }

        public bool NextStreamIdWrapAround(int numOfItem)
        {
            return (m_NextStreamId > (CONSUMER_MAX_STREAM_ID_MINUSONE - numOfItem));
        }
    }

    internal class ItemCallbackClientConsumer : ItemCallbackClient<IOmmConsumerClient>
    {
        private static readonly string CLIENT_NAME = "ItemCallbackClientConsumer";

        public ItemCallbackClientConsumer(OmmBaseImpl<IOmmConsumerClient> baseImpl) : base(baseImpl)
        {
            OmmConsumerImpl ommConsumerImpl = (OmmConsumerImpl)baseImpl;
            EventImpl.SetOmmConsumer(ommConsumerImpl.Consumer);
        }

        public override void NotifyOnAllMsg(Msg msg)
        {
            if(EventImpl.Item?.Client is null)
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

        public override void NotifyOnRefreshMsg()
        {
            if (EventImpl.Item?.Client is null)
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

        public override void NotifyOnUpdateMsg()
        {
            if (EventImpl.Item?.Client is null)
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

        public override void NotifyOnStatusMsg()
        {
            if (EventImpl.Item?.Client is null)
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

        public override void NotifyOnGenericMsg()
        {
            if (EventImpl.Item?.Client is null)
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

        public override void NotifyOnAckMsg()
        {
            if (EventImpl.Item?.Client is null)
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
}
