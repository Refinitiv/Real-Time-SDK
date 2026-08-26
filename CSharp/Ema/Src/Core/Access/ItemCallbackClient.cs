/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.Transports;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Reactor;
using Microsoft.Extensions.Primitives;

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
        IPROVIDER_DICTIONARY_ITEM = 9,
        SYMBOL_LIST_SOURCE_ITEM = 10,
        SINGLE_ITEM_WITH_SOURCE = 11
    }
    internal interface Item<T>
    {
        public int DomainType { get; set; }
        public int StreamId { get; }
        public object? Closure { get; }
        public T? Client { get; }
        public long ItemId { get; set; }
        public Item<T>? Parent { get; }
        public bool AssignedItemId { get; set; }
        public ClosedStatusClient<T>? ClosedStatusClient { get; set; }
        public bool Close();
        public ServiceDirectory<T>? Directory();
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
        public int EtaStreamId { get => StreamId; }

        public void SetUserSpecObject(ReactorSubmitOptions submitOptions)
        {
            submitOptions.RequestMsgOptions.UserSpecObj = this;
        }
    }

    internal class SingleItem<T> : VaNode, Item<T>
    {
        /// <summary>
        /// Defines Item states for handling item watchlist by EMA
        /// </summary>
        public enum StateEnum
        {
            /// <summary>
            /// This is normal state without the request routing feature.
            /// </summary>
            NORMAL = 0,
            /// <summary>
            /// This indicates that this item is being recovered.
            /// </summary>
            RECOVERING = 1,
            /// <summary>
            /// This indicates that this item is being recovered but there is no matching service.
            /// </summary>
            RECOVERING_NO_MATHCING = 2,
            /// <summary>
            /// This is used to close this stream with the watchlist only without removing this item from EMA.
            /// </summary>
            CLOSING_STREAM = 3,
            /// <summary>
            /// This is used to indicate that this item is removed so it is not recovered
            /// </summary>
            REMOVED = 4,

            /// <summary>
            /// This indicates that this item is being recovered by watchlist when the requested service is operational.
            /// </summary>
            RECOVERING_BY_WATCHLIST = 5,
        }

        private static readonly string CLIENT_NAME = "SingleItem";
        internal ServiceDirectory<T>? m_ServiceDirectory { get; set; }
        internal string? ServiceName { get; set; }
        internal OmmBaseImpl<T> m_OmmBaseImpl;
        internal ItemType m_type;

        internal GCHandle m_handle;
        internal int m_StreamId;

        internal bool m_InitReqWithServiceID;
        internal int m_ServiceId;

        public int DomainType { get; set; }
        public int StreamId { get => m_StreamId; set { m_StreamId = value; } }

        public virtual int EtaStreamId 
        { 
            get { return m_StreamId; }
            private set { m_StreamId = value; } 
        }

        public object? Closure { get; private set; }
        public T? Client { get; private set; }
        public Item<T>? Parent { get; private set; }
        public long ItemId { get; set; }
        public ClosedStatusClient<T>? ClosedStatusClient { get; set; }

        #region Request Routing Members
        internal Eta.Codec.IRequestMsg? RequestMsg { get; set; }
        internal StateEnum State { get; set; } = StateEnum.NORMAL;
        internal ServiceList? ServiceList { get; set; }
        internal HashSet<ServiceDirectory<T>>? ItemClosedDirHash { get; set; }
        internal int LastDataState { get; set; } = 0;
        internal int LastStatusCode { get; set; } = 0;
        internal string LastStatusText { get; set; } = string.Empty;
        internal string ItemName { get; set; } = string.Empty;
        internal bool RetrytosameChannel { get; set; } = false;
        public bool AssignedItemId { get; set; } = false;
        public OpenSuspectClient<T>? OpenSuspectClient { get; set; }
        #endregion

        public virtual void SetUserSpecObject(ReactorSubmitOptions submitOptions)
        {
            submitOptions.RequestMsgOptions.UserSpecObj = this;
        }

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
        public virtual void Reset(OmmBaseImpl<T> baseImpl, T client, object? closure, Item<T>? batchItem)
        {
            DomainType = 0;
            StreamId = 0;
            Closure = closure;
            Client = client;
            Parent = batchItem;
            ClosedStatusClient = null;

            m_OmmBaseImpl = baseImpl;
            m_ServiceDirectory = null;
            ServiceName = null;

            AssignedItemId = false;
            RequestMsg = null;
            State = StateEnum.NORMAL;
            ServiceList = null;
            ItemClosedDirHash = null;
            LastDataState = 0;
            LastStatusCode = 0;
            LastStatusText = string.Empty;
            ItemName = string.Empty;
            RetrytosameChannel = false;
            OpenSuspectClient = null;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Close()
        {
            bool retCode = true;

            /* Don't send a close message to the ReactorChannel when the item is being recovered by EMA */
            if (State == StateEnum.NORMAL || State == StateEnum.CLOSING_STREAM)
            {
                ICloseMsg closeMsg = m_OmmBaseImpl.ItemCallbackClient!.CloseMsg();
                closeMsg.ContainerType = DataTypes.NO_DATA;
                closeMsg.DomainType = DomainType;

                retCode = Submit(closeMsg);

                if (State == StateEnum.CLOSING_STREAM)
                    return retCode;
            }

            Remove();
            return retCode;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public ServiceDirectory<T>? Directory()
        {
            return m_ServiceDirectory;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public void Directory(ServiceDirectory<T>? directory)
        {
            m_ServiceDirectory = directory;
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
            ServiceDirectory<T>? service = null;
            ConsumerSession<T> consumerSession = Session();
            Eta.ValueAdd.Rdm.LoginRefresh loginRefresh;
            IRequestMsg rsslRequestMsg = reqMsg.m_rsslMsg;
            SessionDirectory<T>? sessionDirectory = null;
            string? serviceName = null;

            if (reqMsg.HasServiceName)
            {
                if (consumerSession != null)
                {
                    sessionDirectory = consumerSession.GetSessionDirectoryByName(reqMsg.ServiceName());
                    if (sessionDirectory != null)
                    {
                        HashSet<SingleItem<T>>? existingItemSet = null;

                        if (reqMsg.HasName)
                        {
                            ItemName = reqMsg.Name();
                            existingItemSet = sessionDirectory.GetDirectoryByItemName(reqMsg.Name());

                            service = sessionDirectory.MatchingWithExistingDirectory(existingItemSet, rsslRequestMsg);
                        }

                        /* Joins the same ReactorChannel with the existing item */
                        if (service != null && existingItemSet != null)
                        {
                            sessionDirectory.PutDirectoryByHashSet(existingItemSet, this);
                        }
                        else
                        {
                            service = sessionDirectory.Directory(rsslRequestMsg);

                            if (reqMsg.HasName && service != null)
                            {
                                sessionDirectory.PutDirectoryByItemName(ItemName, this);
                            }
                        }
                    }

                    serviceName = reqMsg.ServiceName();
                    loginRefresh = consumerSession.LoginRefresh().LoginRefresh!;
                }
                else
                {
                    serviceName = reqMsg.ServiceName();
                    service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(serviceName);
                    loginRefresh = m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh;
                }

                if (service == null && (!loginRefresh.LoginAttrib.HasSingleOpen
                    || loginRefresh.LoginAttrib.SingleOpen == 0))
                {
                    /* This ensures that the user will get a valid handle.  The callback should clean it up after. */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                    StringBuilder message = m_OmmBaseImpl.GetStrBuilder();
                    message.Append($"Service name of '{reqMsg.ServiceName()}' is not found.");

                    ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                                message.ToString(), ServiceName);

                    return true;
                }
                else if (service == null && consumerSession != null)
                {
                    /* Add this item into the pending queue of the requested service name */
                    consumerSession.AddPendingRequestByServiceName(reqMsg.ServiceName(), this, reqMsg);

                    if (reqMsg.HasName)
                    {
                        ItemName = reqMsg.Name();
                    }

                    /* Specify service name */
                    ServiceName = reqMsg.ServiceName();

                    /* Assign a handle to this request.  This will be valid until the user close this handle */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);
                    AssignedItemId = true;

                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                    if (sessionDirectory == null)
                    {
                        temp.Append("No matching service present.");
                    }
                    else
                    {
                        temp.Append(sessionDirectory.WatchlistResult.ResultText);
                    }

                    ScheduleItemOpenOkStatus(m_OmmBaseImpl.ItemCallbackClient, this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                temp.ToString(), reqMsg.ServiceName());
                    return true;
                }
            }
            else if (reqMsg.HasServiceListName)
            {
                if (consumerSession != null)
                {
                    ServiceList? serviceList = ConsumerSession<T>.GetServiceList(consumerSession.ServiceListDict, reqMsg.ServiceListName());

                    if (serviceList != null)
                    {
                        ServiceList = serviceList;

                        foreach (string concreteServiceName in serviceList.ConcreteServiceList)
                        {
                            sessionDirectory = consumerSession.GetSessionDirectoryByName(concreteServiceName);
                            if (sessionDirectory != null)
                            {
                                HashSet<SingleItem<T>>? existingItemSet = null;
                                if (reqMsg.HasName)
                                {
                                    ItemName = reqMsg.Name();
                                    existingItemSet = sessionDirectory.GetDirectoryByItemName(ItemName);
                                    service = sessionDirectory.MatchingWithExistingDirectory(existingItemSet, rsslRequestMsg);
                                }

                                /* Joins the same ReactorChannel with the existing item */
                                if (service != null && existingItemSet != null)
                                {
                                    sessionDirectory.PutDirectoryByHashSet(existingItemSet, this);
                                    serviceName = service.ServiceName;
                                    break;
                                }
                                else
                                {
                                    service = sessionDirectory.Directory(rsslRequestMsg);

                                    if (service != null)
                                    {
                                        if (reqMsg.HasName)
                                        {
                                            ItemName = reqMsg.Name();
                                            sessionDirectory.PutDirectoryByItemName(ItemName, this);
                                        }

                                        serviceName = service.ServiceName;
                                        break;
                                    }
                                }
                            }
                        }

                        if (service == null)
                        {
                            serviceList.AddPendingRequest((dynamic)this, reqMsg);

                            if (reqMsg.HasName)
                            {
                                ItemName = reqMsg.Name();
                            }

                            /* Specify service list name */
                            ServiceName = reqMsg.ServiceListName();

                            /* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
                            m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);

                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                            if (sessionDirectory == null)
                            {
                                temp.Append("No matching service present.");
                            }
                            else
                            {
                                temp.Append(sessionDirectory.WatchlistResult.ResultText);
                            }

                            ScheduleItemOpenOkStatus(m_OmmBaseImpl.ItemCallbackClient,
                                                        this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                        temp.ToString(), reqMsg.ServiceListName());
                            return true;
                        }
                    }
                    else
                    {
                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                        temp.Append($"The service list name '{reqMsg.ServiceListName()}' does not exist.");

                        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                        return false;
                    }
                } // End checking ConsumerSession instance
                else
                {
                    ServiceList? serviceList = ConsumerSession<T>.GetServiceList(m_OmmBaseImpl.ServiceListDict, reqMsg.ServiceListName());

                    if (serviceList != null)
                    {
                        foreach (string concreteServiceName in serviceList.ConcreteServiceList)
                        {
                            service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(concreteServiceName);

                            if (service != null)
                            {
                                serviceName = service.ServiceName;
                                break;
                            }
                        }
                    }
                    else
                    {
                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                        temp.Append($"The service list name '{reqMsg.ServiceListName()}' does not exist.");

                        m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

                        return false;
                    }
                }
            }
            else
            {
                if (reqMsg.HasServiceId)
                {
                    if (consumerSession != null)
                    {
                        sessionDirectory = consumerSession.GetSessionDirectoryById(reqMsg.ServiceId());

                        if (sessionDirectory != null)
                        {
                            HashSet<SingleItem<T>>? existingItemSet = null;

                            if (reqMsg.HasName)
                            {
                                ItemName = reqMsg.Name();

                                existingItemSet = sessionDirectory.GetDirectoryByItemName(reqMsg.Name());

                                service = sessionDirectory.MatchingWithExistingDirectory(existingItemSet, rsslRequestMsg);
                            }

                            /* Joins the same ReactorChannel with the existing item */
                            if (service != null && existingItemSet != null)
                            {
                                sessionDirectory.PutDirectoryByHashSet(existingItemSet, this);
                            }
                            else
                            {
                                service = sessionDirectory.Directory(rsslRequestMsg);

                                if (reqMsg.HasName && service != null)
                                {
                                    ItemName = reqMsg.Name();
                                    sessionDirectory.PutDirectoryByItemName(ItemName, this);
                                }
                            }

                            if (service != null)
                            {
                                /* Overrides with the actual service ID from source directory */
                                reqMsg.ServiceId(service.Service!.ServiceId);
                            }
                        }

                        loginRefresh = consumerSession.LoginRefresh().LoginRefresh!;
                    }
                    else
                    {
                        service = m_OmmBaseImpl.DirectoryCallbackClient!.GetService(reqMsg.ServiceId());
                        loginRefresh = m_OmmBaseImpl.LoginCallbackClient!.LoginRefresh;
                    }
                }
                else
                {
                    /* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
                    m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(LongIdGenerator.NextLongId(), this);
                    ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient,
                            this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                            "Passed in request message does not identify any service.",
                            null);

                    return true;
                }

                if (consumerSession == null)
                {
                    if (service == null && (!loginRefresh.LoginAttrib.HasSingleOpen || loginRefresh.LoginAttrib.SingleOpen == 0))
                    {
                        /* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
                        m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(LongIdGenerator.NextLongId(), this);

                        StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                        temp.Append("Service id of '")
                            .Append(reqMsg.ServiceId())
                            .Append("' is not found.");

                        ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient!,
                                this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                temp.ToString(), null);

                        return true;
                    }
                }
                else
                {   /* ConsumerSession is enabled */
                    if (service == null)
                    {
                        if (sessionDirectory == null)
                        {
                            /* Assign a handle to this request.  This will be valid until the closed status event is given to the user */
                            m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(LongIdGenerator.NextLongId(), this);

                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                            temp.Append("Service id of '")
                                .Append(reqMsg.ServiceId())
                                .Append("' is not found.");

                            ScheduleItemClosedStatus(m_OmmBaseImpl.ItemCallbackClient,
                                    this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                    temp.ToString(), null);

                            return true;
                        }
                        else
                        {
                            StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();

                            /* Add this item into the pending queue of the requested service Id */
                            consumerSession.AddPendingRequestByServiceId(sessionDirectory, this, reqMsg);

                            if (reqMsg.HasName)
                            {
                                ItemName = reqMsg.Name();
                            }

                            /* Specify service name */
                            ServiceName = sessionDirectory.ServiceName;

                            /* Assign a handle to this request.  This will be valid until the user close this handle */
                            m_OmmBaseImpl.ItemCallbackClient!.AddToItemMap(m_OmmBaseImpl.NextLongId(), this);
                            AssignedItemId = true;

                            temp.Append(sessionDirectory.WatchlistResult.ResultText);

                            ScheduleItemOpenOkStatus(m_OmmBaseImpl.ItemCallbackClient,
                                                        this, reqMsg.m_requestMsgEncoder.m_rsslMsg,
                                                        temp.ToString(), sessionDirectory.ServiceName);
                            return true;
                        }
                    }
                }
            }

            m_ServiceDirectory = service;

            return Submit(reqMsg.m_requestMsgEncoder.m_rsslMsg, serviceName, false);
        }

        private void ScheduleItemOpenOkStatus(ItemCallbackClient<T> client, SingleItem<T> item, Eta.Codec.Msg rsslMsg, string statusText, string serviceName)
        {
            if (OpenSuspectClient != null) return;

            OpenSuspectClient = new OpenSuspectClient<T>(client, item, rsslMsg, statusText, serviceName);
            m_OmmBaseImpl.TimeoutEventManager!.AddTimeoutEvent(1000, OpenSuspectClient);
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

                if (m_ServiceDirectory != null)
                {
                    SessionDirectory<T>? sessionDirectory = m_ServiceDirectory.SessionDirectory;
                    if (sessionDirectory != null)
                    {
                        sessionDirectory.RemoveDirectoryByItemName(ItemName, this);
                    }

                    RequestMsg = null;
                    ItemClosedDirHash = null;
                }

                bool returnToPool = !(State == StateEnum.RECOVERING || State == StateEnum.RECOVERING_NO_MATHCING || State == StateEnum.RECOVERING_BY_WATCHLIST);

                m_OmmBaseImpl.ItemCallbackClient!.RemoveFromMap(this, returnToPool);

                State = StateEnum.REMOVED;
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

            if (!ValidateServiceName(postMsg, serviceName))
            {
                return false;
            }

            return Submit(postMsg.m_rsslMsg as IPostMsg, serviceName);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public virtual bool Submit(GenericMsg genericMsg)
        {
            var consumerSession = Session();
            if(consumerSession != null)
            {
                if(genericMsg.HasServiceId && m_ServiceDirectory?.Service != null)
                {
                    int serviceId = genericMsg.ServiceId();
                    if(serviceId == m_ServiceDirectory.GeneratedServiceId())
                    {
                        /* Translate to the actual service Id */
                        genericMsg.ServiceId(m_ServiceDirectory.Service.ServiceId);
                    }
                }
            }
            
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
                closeMsg.StreamId = EtaStreamId;
            }

            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            ReactorChannel reactorChannel;

            if (m_OmmBaseImpl.ConsumerSession == null)
            {
                reactorChannel = m_OmmBaseImpl.LoginCallbackClient!.ActiveChannelInfo()!.ReactorChannel!;
            }
            else
            {
                reactorChannel = m_ServiceDirectory!.ChannelInfo!.ReactorChannel!;
            }

            if(reactorChannel == null)
            {
                StringBuilder message = m_OmmBaseImpl.GetStrBuilder();

                if (m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
                {
                    message.Append("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(ICloseMsg closeMsg)")
                    .AppendLine($"\tReactorChannel is not avaliable");

                    m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, message.ToString());

                    message.Clear();
                }

                message.Append("Failed to close item request. Reason: ReactorChannel is not avaliable");

                m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);

                return false;
            }

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
        internal virtual bool Submit(IRequestMsg requestMsg, string? serviceName, bool isReissue, bool reportError = true)
        {
            ReactorChannel? reactorChannel;

            if (Session() != null)
            {
                reactorChannel = Directory()?.ChannelInfo?.ReactorChannel;
            }
            else
            {
                reactorChannel = m_OmmBaseImpl.LoginCallbackClient?.ActiveChannelInfo()?.ReactorChannel;
            }

            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            if (reactorChannel != null)
            {
                submitOptions.ApplyClientChannelConfig(reactorChannel.GetClientChannelConfig()!);
            }
            else
            {
                if (reportError)
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

                    m_OmmBaseImpl.HandleInvalidUsage(message.ToString(), (int)ReactorReturnCode.FAILURE);
                }

                return false;
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

            SetUserSpecObject(submitOptions);

            int domainType = requestMsg.DomainType;

            if (StreamId == 0)
            {
                if (requestMsg.CheckHasBatch())
                {
                    var items = ((BatchItem<T>)this).SingleItemList;
                    int numOfItem = items.Count;

                    requestMsg.StreamId = GetNextStreamId(numOfItem);
                    StreamId = requestMsg.StreamId;

                    if (AssignedItemId == false)
                    {
                        m_OmmBaseImpl.ItemCallbackClient!.AddToMap(m_OmmBaseImpl.NextLongId(), this);
                    }
                    else
                    {
                        m_OmmBaseImpl.ItemCallbackClient!.AddToMap(ItemId, this);
                    }

                    SingleItem<T>? item;
                    int itemStreamStart = StreamId;
                    int originalBatchFlags = requestMsg.Flags;
                    for (int index = 0; index < numOfItem; index++)
                    {
                        item = items[index];
                        item.m_ServiceDirectory = m_ServiceDirectory;
                        item.StreamId = ++itemStreamStart;
                        item.DomainType = DomainType;
                        item.ServiceList = ServiceList;
                        item.ServiceName = ServiceName;
                        m_OmmBaseImpl.ItemCallbackClient!.AddToMap(m_OmmBaseImpl.NextLongId(), item);

                        if (Session() != null)
                        {
                            int flags = requestMsg.Flags;
                            flags &= ~RequestMsgFlags.HAS_BATCH;
                            requestMsg.Flags = flags;

                            if (item.RequestMsg == null)
                            {
                                item.RequestMsg = new Eta.Codec.Msg();

                                item.RequestMsg.MsgClass = MsgClasses.REQUEST;
                                requestMsg.Copy(item.RequestMsg, CopyMsgFlags.ALL_FLAGS);
                                item.RequestMsg.MsgKey.ApplyHasName();
                                item.RequestMsg.MsgKey.Name.Data(item.ItemName);
                                item.RequestMsg.StreamId = item.StreamId;
                            }
                        }
                    }

                    /* Restore the original flags for the batch request */
                    requestMsg.Flags = originalBatchFlags;
                }
                else
                {
                    StreamId = GetNextStreamId(0);
                    requestMsg.StreamId = StreamId;

                    if (AssignedItemId == false)
                    {
                        m_OmmBaseImpl.ItemCallbackClient!.AddToMap(m_OmmBaseImpl.NextLongId(), this);
                    }
                    else
                    {
                        m_OmmBaseImpl.ItemCallbackClient!.AddToMap(ItemId, this);
                    }
                }
            }
            else
            {
                requestMsg.StreamId = EtaStreamId;
            }

            if (DomainType == 0)
            {
                DomainType = domainType;
            }
            else
            {
                requestMsg.DomainType = DomainType;
            }

            ReactorReturnCode ret = reactorChannel.Submit((Eta.Codec.Msg)requestMsg, submitOptions, out var ErrorInfo);

            if (ret < ReactorReturnCode.SUCCESS)
            {
                if (reportError)
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
                }

                return false;
            }

            if (restoreServiceIDFlag)
            {
                requestMsg.MsgKey.Flags |= MsgKeyFlags.HAS_SERVICE_ID;
            }

            /* There is no need to clone batch request as it is not used for recovering */
            if (Session() != null && RequestMsg == null && !requestMsg.CheckHasBatch())
            {
                RequestMsg = new Eta.Codec.Msg();

                RequestMsg.MsgClass = MsgClasses.REQUEST;
                requestMsg.Copy(RequestMsg, CopyMsgFlags.ALL_FLAGS);
            }

            return true;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal bool Submit<V>(V submitMsg, string? serviceName) where V : IMsg
        {
            ReactorSubmitOptions submitOptions = m_OmmBaseImpl.GetSubmitOptions();
            submitOptions.Clear();
            submitOptions.ServiceName = serviceName;

            submitMsg.StreamId = EtaStreamId;

            if (submitMsg.MsgClass == MsgClasses.GENERIC && submitMsg.DomainType == 0)
            {
                submitMsg.DomainType = DomainType;
            }

            ReactorChannel? reactorChannel;

            if (Session() != null)
            {
                reactorChannel = Directory()?.ChannelInfo?.ReactorChannel;
            }
            else
            {
                reactorChannel = m_OmmBaseImpl.LoginCallbackClient?.ActiveChannelInfo()?.ReactorChannel;
            }

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
        private bool ValidateServiceName(PostMsg postMsg, string? serviceName)
        {
            ServiceDirectory<IOmmConsumerClient>? directory = null;
            string? errorText = null;
            if (Session() != null)
            {
                IPostMsg rsslPostMsg = postMsg.m_rsslMsg;
                SessionChannelInfo<IOmmConsumerClient>? sessionChannelInfo = m_ServiceDirectory?.ChannelInfo?.SessionChannelInfo;
                if (serviceName != null)
                {
                    directory = sessionChannelInfo?.GetDirectoryByName(serviceName);
                    if (directory?.Service != null && directory.Service.Action != MapEntryActions.DELETE)
                    {
                        return true;
                    }

                    errorText = $"Message submitted with unknown service name {serviceName}";
                }
                else if (rsslPostMsg.CheckHasMsgKey() && rsslPostMsg.MsgKey.CheckHasServiceId())
                {
                    /* Search from the generated service Id */
                    var sessionDirectory = Session().GetSessionDirectoryById(rsslPostMsg.MsgKey.ServiceId);

                    if (sessionDirectory != null)
                    {
                        /* Search from service name whether this SessionChannelInfo has it. */
                        directory = sessionChannelInfo?.GetDirectoryByName(sessionDirectory.ServiceName);

                        if (directory?.Service != null && directory.Service.Action != MapEntryActions.DELETE)
                        {
                            /* Translate to the actual service ID from the provider */
                            rsslPostMsg.MsgKey.ServiceId = directory.Service.ServiceId;
                            return true;
                        }
                    }

                    errorText = $"Message submitted with unknown service Id {rsslPostMsg.MsgKey.ServiceId}";
                }
                else
                {
                    errorText = "Message submitted with unspecified service Name and Id";
                }
            }
            else
            {
                if (serviceName == null || m_OmmBaseImpl.DirectoryCallbackClient!.GetService(serviceName) != null)
                {
                    return true;
                }

                errorText = $"Message submitted with unknown service name {serviceName}";
            }

            StringBuilder strBuilder = m_OmmBaseImpl.GetStrBuilder();
            if(m_OmmBaseImpl.LoggerClient.IsErrorEnabled)
            {
                strBuilder.AppendLine("Internal error: ReactorChannel.Submit() failed in SingleItem.Submit(PostMsg)")
                    .AppendLine($"\tError Id {ReactorReturnCode.INVALID_USAGE}")
                    .AppendLine("\tError Location ItemCallbackClient.Submit(IPostMsg,String)")
                    .AppendLine($"\tError Text {errorText}");

                m_OmmBaseImpl.LoggerClient.Error(CLIENT_NAME, strBuilder.ToString());
                strBuilder.Clear();
            }

            strBuilder.Append("Failed to submit PostMsg on item stream. Reason: ")
                       .Append(ReactorReturnCode.INVALID_USAGE)
                       .Append($". Error text: {errorText}");

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

        internal ConsumerSession<T> Session()
        {
            return m_OmmBaseImpl.ConsumerSession!;
        }
    }

    internal class SourceItem<T> : Item<T>
    {
        internal IRequestMsg m_InitialRequestMsg = new Eta.Codec.Msg();
        internal Dictionary<int, SingleItemWithSource<T>> m_StreamIdItemsMap = new Dictionary<int, SingleItemWithSource<T>>(); // stores items by msg.StreamId (from ValueAdd layer)
        internal SingleItemWithSource<T>? m_SymbolListItem;

        internal bool m_HasBehavior = false;
        internal int m_SymbolListFlags;

        int Item<T>.DomainType { get => throw new System.NotImplementedException(); set => throw new System.NotImplementedException(); }

        int Item<T>.StreamId { get; }

        object? Item<T>.Closure => throw new System.NotImplementedException();

        T? Item<T>.Client => throw new System.NotImplementedException();

        long Item<T>.ItemId { get => throw new System.NotImplementedException(); set => throw new System.NotImplementedException(); }

        Item<T>? Item<T>.Parent => null;

        bool Item<T>.AssignedItemId { get => throw new System.NotImplementedException(); set => throw new System.NotImplementedException(); }

        ClosedStatusClient<T>? Item<T>.ClosedStatusClient { get => throw new System.NotImplementedException(); set => throw new System.NotImplementedException(); }

        public bool Open(RequestMsg reqMsg) 
        { 
            return false; 
        }

        public bool Modify(RequestMsg reqMsg) 
        { 
            return false; 
        }

        public bool Close() 
        { 
            return false; 
        }

        public void Remove() { }

        public ItemType Type()
        {
            return ItemType.SYMBOL_LIST_SOURCE_ITEM;
        }

        public ServiceDirectory<T>? Directory()
        {
            return m_SymbolListItem?.Directory();
        }

        public int GetNextStreamId(int numOfItem)
        {
            return 0;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal Item<T>? GetItem(int streamId)
        {
            SingleItemWithSource<T>? item = null;

            if (m_StreamIdItemsMap.ContainsKey(streamId))
            {                
                item = m_StreamIdItemsMap[streamId];
            }

            return item;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal SingleItemWithSource<T>? GetItem(Msg msg, bool createIfNotFound)
        {
            return GetItem(msg, msg.StreamId(), createIfNotFound);
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal SingleItemWithSource<T>? GetItem(Msg msg, int streamId, bool createIfNotFound)
        {
            SingleItemWithSource<T>? item = null;

            if (m_StreamIdItemsMap.ContainsKey(msg.StreamId()))
            {
                item = m_StreamIdItemsMap[msg.StreamId()];
            }
            else if (m_SymbolListItem != null && createIfNotFound)
            {
                if ((item = (SingleItemWithSource<T>?)m_SymbolListItem.m_OmmBaseImpl.GetEmaObjManager().m_singleItemWithSourcePool.Poll()) == null)
                {
                    item = new SingleItemWithSource<T>(m_SymbolListItem.m_OmmBaseImpl, m_SymbolListItem.Client, m_SymbolListItem.Closure);
                    m_SymbolListItem.m_OmmBaseImpl.GetEmaObjManager().m_singleItemWithSourcePool.UpdatePool(item);
                }
                else
                    item.Reset(m_SymbolListItem.m_OmmBaseImpl, m_SymbolListItem.Client!, m_SymbolListItem.Closure, null);

                item.StreamId = streamId;
                item.m_EtaStreamId = msg.StreamId();
                item.m_HasEtaStreamId = true;
                m_StreamIdItemsMap.Add(msg.StreamId(), item);
                item.m_Source = this;


                item.m_ServiceDirectory = m_SymbolListItem.m_ServiceDirectory;
                item.DomainType = msg.DomainType();
                item.ServiceList = m_SymbolListItem.ServiceList;
                item.ServiceName = m_SymbolListItem.ServiceName;

                item.RequestMsg = new Eta.Codec.Msg();
                item.RequestMsg.MsgClass = MsgClasses.REQUEST;
                item.RequestMsg.StreamId = msg.m_rsslMsg.StreamId;

                if (msg.m_rsslMsg.MsgKey.CheckHasNameType())
                {
                    item.RequestMsg.MsgKey.ApplyHasNameType();
                    item.RequestMsg.MsgKey.NameType = msg.m_rsslMsg.MsgKey.NameType;
                }

                item.RequestMsg.DomainType = msg.DomainType();
                item.RequestMsg.ContainerType = DataTypes.NO_DATA;
                if ((m_SymbolListFlags & SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS) > 0) item.RequestMsg.ApplyStreaming();

                int serviceId = 0;
                ServiceDirectory<T>? directory = null;
                if (m_SymbolListItem.m_ServiceDirectory != null)
                {
                    directory = m_SymbolListItem.m_ServiceDirectory;
                }
                else
                {
                    if (m_InitialRequestMsg.MsgKey.CheckHasServiceId())
                    {
                        serviceId = m_InitialRequestMsg.MsgKey.ServiceId;
                        directory = m_SymbolListItem.m_OmmBaseImpl.DirectoryCallbackClient!.GetService(m_InitialRequestMsg.MsgKey.ServiceId);
                    }
                    else if (m_SymbolListItem.ServiceName != null)
                    {
                        directory = m_SymbolListItem.m_OmmBaseImpl.DirectoryCallbackClient!.GetService(m_SymbolListItem.ServiceName);
                        if (directory != null) serviceId = directory.Service!.ServiceId;
                    }
                }

                if (serviceId != 0)
                {
                    item.RequestMsg.MsgKey.ApplyHasServiceId();
                    item.RequestMsg.MsgKey.ServiceId = serviceId;
                }

                Qos? itemQos = null;

                item.RequestMsg.ApplyHasQos();
                if (directory != null && directory.Service!.Info.QosList.Count != 0)
                {
                    itemQos = directory.Service.Info.BestQos;
                    itemQos.Copy(item.RequestMsg.Qos);
                }
                else
                {
                    item.RequestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
                    item.RequestMsg.Qos.Timeliness(QosTimeliness.REALTIME);
                }

                if (msg.m_rsslMsg.MsgKey != null && msg.m_rsslMsg.MsgKey.CheckHasName())
                {
                    ByteBuffer name = new ByteBuffer(msg.m_rsslMsg.MsgKey.Name.Length);
                    
                    name.Put(msg.m_rsslMsg.MsgKey.Name.Data().Contents, msg.m_rsslMsg.MsgKey.Name.Position, msg.m_rsslMsg.MsgKey.Name.Position + msg.m_rsslMsg.MsgKey.Name.Length);
                    name.Flip();
                    item.RequestMsg.MsgKey.ApplyHasName();
                    item.RequestMsg.MsgKey.Name.Data(name);

                    item.ItemName = msg.m_rsslMsg.MsgKey.Name.ToString();
                }

                long handle = m_SymbolListItem.m_OmmBaseImpl.NextLongId();
                m_SymbolListItem.m_OmmBaseImpl.ItemCallbackClient!.AddToMap(handle, item);
            }

            return item;
        }

        bool Item<T>.Submit(RefreshMsg refreshMsg)
        {
            throw new System.NotImplementedException();
        }

        bool Item<T>.Submit(UpdateMsg updateMsg)
        {
            throw new System.NotImplementedException();
        }

        bool Item<T>.Submit(StatusMsg statusMsg)
        {
            throw new System.NotImplementedException();
        }

        bool Item<T>.Submit(PostMsg postMsg)
        {
            throw new System.NotImplementedException();
        }

        bool Item<T>.Submit(GenericMsg genericMsg)
        {
            throw new System.NotImplementedException();
        }

        void Item<T>.BackToPool()
        {
            throw new System.NotImplementedException();
        }
    }



    class SingleItemWithSource<T> : SingleItem<T>
    {
        Eta.Codec.ElementList? m_ElementList;
        Eta.Codec.ElementEntry? m_ElementEntry;
        Eta.Codec.ElementList? m_BehaviourElementList;
        Eta.Codec.ElementEntry? m_BehaviourEntry;
        UInt? m_DataStreamFlag;
        DecodeIterator? m_DecodeIterator;
        

        bool m_Initialized = false;

        internal bool m_HasEtaStreamId = false;
        internal int m_EtaStreamId;

        internal SourceItem<T>? m_Source;
        internal ReactorChannel? m_ReactorChannel;

        public override int EtaStreamId { get { if (m_HasEtaStreamId) return m_EtaStreamId; else return StreamId; } }

        internal SingleItemWithSource() : base() 
        {
            m_type = ItemType.SINGLE_ITEM_WITH_SOURCE;
        }

        internal SingleItemWithSource(OmmBaseImpl<T> baseImpl, T? client, object? closure) : base(baseImpl, client, closure, null) 
        {
            m_type = ItemType.SINGLE_ITEM_WITH_SOURCE;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void SetUserSpecObject(ReactorSubmitOptions rsslSubmitOptions)
        {
            SourceItem<T>? source = this.m_Source;

            if (source == null)
            {
                source = new SourceItem<T>();
                source.m_SymbolListItem = Copy();
                this.m_Source = source;
            }

            rsslSubmitOptions.RequestMsgOptions.UserSpecObj = source;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void Remove()
        {
            if (m_Source != null)
            {
                m_Source.m_StreamIdItemsMap.Remove(base.EtaStreamId);
                m_Initialized = false;
                this.m_Source = null;
            }
            if (m_ReactorChannel != null && m_OmmBaseImpl.ItemCallbackClient != null && m_OmmBaseImpl.ItemCallbackClient.m_ProviderDrivenItemsByReactorChannel.ContainsKey(m_ReactorChannel))
            {
                m_OmmBaseImpl.ItemCallbackClient.m_ProviderDrivenItemsByReactorChannel[m_ReactorChannel].Remove(this.EtaStreamId);
            }
            m_ReactorChannel = null;

            base.Remove();
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        internal override bool Submit(IRequestMsg rsslRequestMsg, string? serviceName, bool reissue, bool reportError = true)
        {
            bool res = base.Submit(rsslRequestMsg, serviceName, reissue, reportError);

            if (!m_Initialized && !reissue)
            {
                m_Initialized = true;
                ExtractFlags(rsslRequestMsg, m_Source!);
                m_Source!.m_InitialRequestMsg = new Eta.Codec.Msg();
                rsslRequestMsg.Copy(m_Source?.m_InitialRequestMsg, CopyMsgFlags.ALL_FLAGS);
                m_Source?.m_StreamIdItemsMap.Add(rsslRequestMsg.StreamId, this);
            }

            return res;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        SingleItemWithSource<T> Copy()
        {
            // this object should not be taken from pool
            SingleItemWithSource<T> res = new SingleItemWithSource<T>(this.m_OmmBaseImpl, this.Client, this.Closure);

            res.m_ServiceDirectory = this.m_ServiceDirectory;
            res.ServiceList = this.ServiceList;
            res.DomainType = this.DomainType;

            return res;
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        void ExtractFlags(IRequestMsg requestMsg, SourceItem<T> source)
        {
            if (requestMsg.ContainerType != DataTypes.ELEMENT_LIST)
                return; // nothing to extract


            if (m_DecodeIterator == null) m_DecodeIterator = new DecodeIterator();
            if (m_ElementList == null) m_ElementList = new Eta.Codec.ElementList();
            m_ElementList.Clear();

            m_DecodeIterator.Clear();
            m_DecodeIterator.SetBufferAndRWFVersion(requestMsg.EncodedDataBody, Codec.MajorVersion(), Codec.MinorVersion());

            CodecReturnCode ret = m_ElementList.Decode(m_DecodeIterator, null);
            if (ret != CodecReturnCode.SUCCESS)
            {
                return;
            }

            if (m_ElementEntry == null) m_ElementEntry = new();
            m_ElementEntry.Clear();
            if (m_BehaviourElementList == null) m_BehaviourElementList = new();
            m_BehaviourElementList.Clear();
            if (m_BehaviourEntry == null) m_BehaviourEntry = new();
            m_BehaviourEntry.Clear();

            while ((ret = m_ElementEntry.Decode(m_DecodeIterator)) != CodecReturnCode.END_OF_CONTAINER)
            {
                if (ret != CodecReturnCode.SUCCESS)
                {
                    return;
                }

                if (m_ElementEntry.Name.Equals(SymbolList.ElementNames.SYMBOL_LIST_BEHAVIORS))
                {
                    source.m_HasBehavior = true;

                    if (m_ElementEntry.DataType != DataTypes.ELEMENT_LIST)
                    {
                        // Nothing to extract
                        return;
                    }

                    ret = m_BehaviourElementList.Decode(m_DecodeIterator, null);
                    if (ret == CodecReturnCode.SUCCESS)
                    {
                        while ((ret = m_BehaviourEntry.Decode(m_DecodeIterator)) != CodecReturnCode.END_OF_CONTAINER)
                        {
                            if (ret < CodecReturnCode.SUCCESS)
                            {
                                return;
                            }
                            else
                            {
                                if (m_BehaviourEntry.Name.Equals(SymbolList.ElementNames.SYMBOL_LIST_DATA_STREAMS))
                                {
                                    if (m_BehaviourEntry.DataType != DataTypes.UINT)
                                    {
                                        return;
                                    }

                                    if (m_DataStreamFlag == null) m_DataStreamFlag = new UInt();

                                    ret = m_DataStreamFlag.Decode(m_DecodeIterator);
                                    if (ret != CodecReturnCode.SUCCESS)
                                    {
                                        return;
                                    }

                                    if ((int)m_DataStreamFlag.ToLong() < SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_NAMES_ONLY
                                            || (int)m_DataStreamFlag.ToLong() > SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS)
                                    {
                                        return;
                                    }
                                    source.m_SymbolListFlags = (int)m_DataStreamFlag.ToLong();
                                }
                            }
                        }
                    }
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveOptimization | MethodImplOptions.AggressiveInlining)]
        public override void Reset(OmmBaseImpl<T> baseImpl, T client, object? closure, Item<T>? batchItem)
        {
            base.Reset(baseImpl, client, closure, batchItem);
            
            m_Source = null;
            m_ReactorChannel = null;
            m_HasEtaStreamId = false;
            m_Initialized = false;
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
            return HandleInvalidAttempt("Invalid attempt to modify batch stream");
        }

        public override bool Submit(PostMsg postMsg)
        {
            return HandleInvalidAttempt("Invalid attempt to submit PostMsg on batch stream");
        }

        public override bool Submit(GenericMsg genericMsg)
        {
            return HandleInvalidAttempt("Invalid attempt to submit GenericMsg on batch stream");
        }

        public override bool Close()
        {
            return HandleInvalidAttempt("Invalid attempt to close batch stream");
        }

        internal void AddBatchItems(List<string> itemList)
        {
            SingleItem<T>? singleItem;

            for(int i = 0; i < itemList.Count; i++)
            {
                if ((singleItem = (SingleItem<T>?)m_OmmBaseImpl.GetEmaObjManager().m_singleItemPool.Poll()) == null)
                {
                    singleItem = new SingleItem<T>(m_OmmBaseImpl, Client, Closure, this);
                    m_OmmBaseImpl.GetEmaObjManager().m_singleItemPool.UpdatePool(singleItem);
                }
                else
                {
                    singleItem.Reset(m_OmmBaseImpl, Client!, Closure, this);
                }

                singleItem.ItemName = itemList[i];

                SingleItemList.Add(singleItem);
            }

            ItemCount = itemList.Count;
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

        private bool HandleInvalidAttempt(string message)
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

            if(serviceName != null)
                client.m_StatusMsg.SetServiceName(serviceName);

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

    internal class OpenSuspectClient<T> : ITimeoutClient
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

        public OpenSuspectClient(CallbackClient<T> client, Item<T> item, IMsg msg, string statusText, string? serviceName)
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

            state.StreamState(StreamStates.OPEN);
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

            if (client.m_StatusMsg == null)
                client.m_StatusMsg = new StatusMsg();

            client.m_StatusMsg.Decode(statusMsg, Codec.MajorVersion(), Codec.MajorVersion(), null!);

            if(serviceName != null)
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

        private MonitorWriteLocker? m_StreamIdAccessLock;

        private ConsumerSession<T>? m_ConsumerSession; /* This is used when there is a consumer session */

        internal Dictionary<ReactorChannel, Dictionary<int, Item<T>>> m_ProviderDrivenItemsByReactorChannel = new Dictionary<ReactorChannel, Dictionary<int, Item<T>>>();
        internal int _nextProviderStreamId = 0;

        public ItemCallbackClient(OmmBaseImpl<T> baseImpl) : base(baseImpl, CLIENT_NAME)
        {
            m_OmmBaseImpl = baseImpl;
            m_ConsumerSession = baseImpl.ConsumerSession;

            if (m_ConsumerSession != null)
            {
                m_ItemHandleDict = m_ConsumerSession.SessionWatchlist.ItemHandleMap();
                m_StreamIdDict = m_ConsumerSession.SessionWatchlist.StreamIdMap();
                m_ConsumerSession.SessionWatchlist.CallbackClient(this);
            }
            else
            {
                int itemCountHint;

                if (commonImpl.BaseType == IOmmCommonImpl.ImpleType.CONSUMER)
                    itemCountHint = (int)((OmmConsumerConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).ConsumerConfig.ItemCountHint;
                else
                    itemCountHint = (int)((OmmNiProviderConfigImpl)m_OmmBaseImpl.OmmConfigBaseImpl).NiProviderConfig.ItemCountHint;

                m_ItemHandleDict = new Dictionary<long, Item<T>>(itemCountHint);
                m_StreamIdDict = new Dictionary<int, Item<T>>(itemCountHint);
            }

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

        int GetNextProviderStreamId()
        {
            if (_nextProviderStreamId == int.MaxValue) _nextProviderStreamId = 0;

            int startStreamId = _nextProviderStreamId;
            int nextProviderStreamId = ++_nextProviderStreamId;

            while (IsStreamIdInUse(-nextProviderStreamId) && _nextProviderStreamId < int.MaxValue)
            {
                nextProviderStreamId = ++_nextProviderStreamId;
            }

            if (IsStreamIdInUse(-nextProviderStreamId)) // start from the beginning to startStreamId to make the full circle
            {
                _nextProviderStreamId = 0;
                while (IsStreamIdInUse(-nextProviderStreamId) && _nextProviderStreamId < startStreamId)
                {
                    nextProviderStreamId = ++_nextProviderStreamId;
                }

                if (IsStreamIdInUse(-nextProviderStreamId)) // free streamId not found
                {
                    StringBuilder temp = m_OmmBaseImpl.GetStrBuilder();
                    temp.Append("Unable to obtain next available stream id to retrieve provider-driven item.");
                    m_OmmBaseImpl.HandleInvalidUsage(temp.ToString(), OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }
            }

            return -nextProviderStreamId;
        }

        Item<T> GetProviderDrivenItem(Msg rsslMsg, ReactorChannel reactorChannel, object userSpecObject, bool createIfNotFound)
        {
            Item<T> item;
            Dictionary<int, Item<T>> map;

            if (m_ProviderDrivenItemsByReactorChannel.ContainsKey(reactorChannel))
            {
                map = m_ProviderDrivenItemsByReactorChannel[reactorChannel];
            }
            else
            {
                map = new Dictionary<int, Item<T>>();
                m_ProviderDrivenItemsByReactorChannel.Add(reactorChannel, map);
            }

            if (map.ContainsKey(rsslMsg.StreamId()))
            {
                item = map[rsslMsg.StreamId()];
            }
            else // this streamId has not been yet processed for the given reactorChannel, create new Item
            {
                item = CreateProviderDrivenItem(rsslMsg, userSpecObject, map, reactorChannel, createIfNotFound);
            }

            return item;
        }

        Item<T> CreateProviderDrivenItem(Msg rsslMsg, object userSpecObject, Dictionary<int, Item<T>> map, ReactorChannel channel, bool createIfNotFound)
        {
            SingleItemWithSource<T> item;

            if (userSpecObject != null && userSpecObject is SourceItem<T>) // streamId of an item requested by the API from the Symbol List Refresh, streamId should be negativey
            {
                SourceItem<T> sourceItem = (SourceItem<T>)userSpecObject;
                int streamId = rsslMsg.StreamId() < 0 ? GetNextProviderStreamId() : rsslMsg.StreamId();
                item = sourceItem.GetItem(rsslMsg, streamId, true)!; // we are getting new tiem here so it is not null
                item.m_ReactorChannel = channel;

                map.Add(rsslMsg.StreamId(), item);
            }
            else throw new OmmInvalidUsageException("ItemCallbackClient.CreateProviderDrivenItem : Invalid attempt to get provider-driven item for Symbol List request");

            return item;
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

            bool fromBatchRequest = false;
            bool fromSymbolListRequest = false;

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
            {
                fromBatchRequest = true;
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
            else if (EventImpl.Item!.Type() == ItemType.SYMBOL_LIST_SOURCE_ITEM)
            {
                fromSymbolListRequest = true;
                EventImpl.Item = msg.StreamId > 0
                        ? ((SourceItem<T>)EventImpl.Item).GetItem(m_RefreshMsg, true)
                        : GetProviderDrivenItem(m_RefreshMsg, channelInfo, EventImpl.Item, true);
                m_RefreshMsg.StreamId(EventImpl.Item!.StreamId);
                m_RefreshMsg.m_rsslMsg.StreamId = EventImpl.Item.StreamId;
            }

            ServiceDirectory<T>? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_RefreshMsg.SetServiceName(serviceDirectory.ServiceName!);

                if (IsSingleItem())
                {
                    SingleItem<T> singleItem = ((SingleItem<T>)EventImpl.Item);

                    singleItem.ItemClosedDirHash = null;

                    if (singleItem.ServiceList != null)
                    {
                        m_RefreshMsg.SetServiceName(singleItem.ServiceList.Name);
                        m_RefreshMsg.ServiceIdInt(singleItem.ServiceList.ServiceId);
                    }
                    else if (serviceDirectory.HasGeneratedServiceId) /* Gets the mapping alternate service Id if any */
                        m_RefreshMsg.ServiceIdInt(serviceDirectory.GeneratedServiceId());
                }

                /* Adds the item to the item map of SessionDirectory when the item is requested by batch request. */
                if (fromBatchRequest || fromSymbolListRequest)
                {
                    if (serviceDirectory.SessionDirectory != null && IsSingleItem())
                    {
                        if (m_RefreshMsg.HasName)
                        {
                            serviceDirectory.SessionDirectory.PutDirectoryByItemName(m_RefreshMsg.Name(), ((SingleItem<T>)EventImpl.Item));
                        }
                    }
                }
            }
            else if (IsSingleItem())
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
            else if (EventImpl.Item!.Type() == ItemType.SYMBOL_LIST_SOURCE_ITEM)
            {
                EventImpl.Item = ((SourceItem<T>)EventImpl.Item).GetItem(m_UpdateMsg.StreamId());
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
                else
                {
                    m_UpdateMsg.StreamId(EventImpl.Item.StreamId);
                    m_UpdateMsg.m_rsslMsg.StreamId = EventImpl.Item.StreamId;
                }
            }

            ServiceDirectory<T>? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_UpdateMsg.SetServiceName(serviceDirectory.ServiceName!);

                if (IsSingleItem())
                {
                    SingleItem<T> singleItem = ((SingleItem<T>)EventImpl.Item);

                    if (singleItem.ServiceList != null)
                    {
                        m_UpdateMsg.SetServiceName(singleItem.ServiceList.Name);
                        m_UpdateMsg.ServiceIdInt(singleItem.ServiceList.ServiceId);

                        singleItem.ItemClosedDirHash = null;
                    }
                    else if (serviceDirectory.HasGeneratedServiceId) /* Gets the mapping alternate service Id if any */
                    {
                        m_UpdateMsg.ServiceIdInt(serviceDirectory.GeneratedServiceId());

                        singleItem.ItemClosedDirHash = null;
                    }
                }
            }
            else if (IsSingleItem())
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
            else if (EventImpl.Item!.Type() == ItemType.SYMBOL_LIST_SOURCE_ITEM)
            {
                EventImpl.Item = msg.StreamId > 0
                        ? ((SourceItem<T>)EventImpl.Item).GetItem(m_StatusMsg, true)
                        : GetProviderDrivenItem(m_StatusMsg, channelInfo, EventImpl.Item, true);
                m_StatusMsg.StreamId(EventImpl.Item!.StreamId);
                m_StatusMsg.m_rsslMsg.StreamId = EventImpl.Item.StreamId;
            }

            ServiceDirectory<T>? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_StatusMsg.SetServiceName(serviceDirectory.ServiceName!);
            }
            else if (IsSingleItem())
            {
                m_StatusMsg.SetServiceName(((SingleItem<T>)EventImpl.Item).ServiceName!);
            }
            else
            {
                m_StatusMsg.SetServiceName(null!);
            }

            if(m_ConsumerSession != null)
            {
                return m_ConsumerSession.SessionWatchlist.HandleItemStatus((IStatusMsg)msg, m_StatusMsg, EventImpl.Item);
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
            else if (EventImpl.Item!.Type() == ItemType.SYMBOL_LIST_SOURCE_ITEM)
            {
                EventImpl.Item = ((SourceItem<T>)EventImpl.Item).GetItem(m_GenericMsg.StreamId());
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
                else
                {
                    m_GenericMsg.StreamId(EventImpl.Item.StreamId);
                    m_GenericMsg.m_rsslMsg.StreamId = EventImpl.Item.StreamId;
                }
            }

            ServiceDirectory<T>? serviceDirectory = EventImpl.Item.Directory();
            if (serviceDirectory != null)
            {
                if (msg.MsgKey.CheckHasServiceId())
                {
                    int serviceId = msg.MsgKey.ServiceId;
                    if (serviceId == serviceDirectory.Service!.ServiceId)
                    {
                        /* Gets the mapping alternate service Id if any */
                        if (serviceDirectory.HasGeneratedServiceId)
                            m_GenericMsg.ServiceIdInt(serviceDirectory.GeneratedServiceId());
                    }
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

            if (EventImpl.Item!.Type() == ItemType.BATCH_ITEM)
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
            else if (EventImpl.Item!.Type() == ItemType.SYMBOL_LIST_SOURCE_ITEM)
            {
                EventImpl.Item = ((SourceItem<T>)EventImpl.Item).GetItem(msg.StreamId);
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
                else
                {
                    m_AckMsg.StreamId(EventImpl.Item.StreamId);
                    m_AckMsg.m_rsslMsg.StreamId = EventImpl.Item.StreamId;
                }
            }

            ServiceDirectory<T>? serviceDirectory = EventImpl.Item.Directory();

            if (serviceDirectory != null)
            {
                m_AckMsg.SetServiceName(serviceDirectory.ServiceName!);

                if (msg.MsgKey.CheckHasServiceId())
                {
                    int serviceId = msg.MsgKey.ServiceId;
                    if (serviceId == serviceDirectory.Service!.ServiceId)
                    {
                        /* Gets the mapping alternate service Id if any */
                        if (serviceDirectory.HasGeneratedServiceId)
                            m_AckMsg.ServiceIdInt(serviceDirectory.GeneratedServiceId());
                    }
                }
            }
            else
            {
                m_AckMsg.SetServiceName(null!);
            }

            NotifyOnAllMsg(m_AckMsg);
            NotifyOnAckMsg();

            return ReactorCallbackReturnCode.SUCCESS;
        }

        bool IsSingleItem()
        {
            return EventImpl.Item!.Type() == ItemType.SINGLE_ITEM || EventImpl.Item.Type() == ItemType.SINGLE_ITEM_WITH_SOURCE;
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
                        ConsumerSession<T>? consumerSession = m_OmmBaseImpl.ConsumerSession;

                        if (consumerSession != null)
                        {
                            SingleItem<T> dirItem = m_OmmBaseImpl.DirectoryCallbackClient!.DirectoryItem(consumerSession, reqMsg, client, closure);
                            return dirItem.ItemId;
                        }

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
                case (int)DomainType.SYMBOL_LIST:
                    SingleItemWithSource<T>? itemWithSource = null;
                    if ((itemWithSource = (SingleItemWithSource<T>?)m_OmmBaseImpl.GetEmaObjManager().m_singleItemWithSourcePool.Poll()) == null)
                    {
                        itemWithSource = new SingleItemWithSource<T>(m_OmmBaseImpl, client, closure);
                        m_OmmBaseImpl.GetEmaObjManager().m_singleItemWithSourcePool.UpdatePool(itemWithSource);
                    }
                    else
                    {
                        itemWithSource.Reset(m_OmmBaseImpl, client, closure, null);
                    }

                    if (!itemWithSource.Open(reqMsg))
                    {
                        RemoveFromMap(itemWithSource, true);
                        return 0;
                    }
                    else
                        return itemWithSource.ItemId;
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
                                batchItem.AddBatchItems(reqMsg.m_requestMsgEncoder.BatchItemList);
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
        public void RemoveFromMap(Item<T> item, bool returnToPool = true)
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

            if (returnToPool)
            {
                item.BackToPool();
            }
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

        /* Remove all pending request items */
        internal void RemoveAllItems()
        {
            foreach(var item in m_ItemHandleDict.Values)
            {
                item.Remove();
            }
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
