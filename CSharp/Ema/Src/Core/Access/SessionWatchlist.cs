/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|              Copyright (C) 2025 LSEG. All rights reserved.                --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;

namespace LSEG.Ema.Access;

internal sealed class WatchlistResult
{
    public enum CodeEnum
    {
        SUCCESS = 0,
        SERVICE_NOT_UP = -1,
        CAPABILITY_NOT_FOUND = -2,
        MATCHING_QOS_NOT_FOUND = -3
    }

    public WatchlistResult()
    {
        Clear();
    }

    public void Clear()
    {
        ResultCode = CodeEnum.SUCCESS;
        ResultText = string.Empty;
    }

    public CodeEnum ResultCode { get; set; } = CodeEnum.SUCCESS;
    public string ResultText { get; set; } = string.Empty;
}

internal sealed class SessionWatchlist<T>
{
    private readonly Dictionary<long, Item<T>> m_ItemHandleDict;
    private readonly Dictionary<int, Item<T>> m_StreamIdDict;

    /* The message queue is used to queue item when there is no suitable ReactorChannel to submit a request message */
    private Queue<IRequestMsg> _recoveryItemQueue;
    private Queue<SingleItem<T>> _closingItemQueue;

    private Buffer _statusText = new Buffer();

    private bool _sessionEnhancedItemRecovery;

    ConsumerSession<T> _consumerSession;
    CallbackClient<T>? _callbackClient;

    readonly Qos _defaultQos = new Qos();
    readonly Qos _defaultWorstQos = new Qos();
    readonly Qos _matchedQos = new Qos();

    public SessionWatchlist(ConsumerSession<T> consumerSession, int itemCountHint)
    {
        _recoveryItemQueue = new Queue<IRequestMsg>(itemCountHint);
        _closingItemQueue = new Queue<SingleItem<T>>(1000);

        _consumerSession = consumerSession;

        m_ItemHandleDict = new Dictionary<long, Item<T>>(itemCountHint);
        m_StreamIdDict = new Dictionary<int, Item<T>>(itemCountHint);

        _sessionEnhancedItemRecovery = consumerSession.ConsumerConfigImpl.ConsumerConfig.SessionEnhancedItemRecovery != 0;

        _defaultQos.Clear();
        _defaultQos.IsDynamic = false;
        _defaultQos.Timeliness(QosTimeliness.REALTIME);
        _defaultQos.Rate(QosRates.TICK_BY_TICK);

        _defaultWorstQos.Clear();
        _defaultWorstQos.Rate(QosRates.TIME_CONFLATED);
        _defaultWorstQos.Timeliness(QosTimeliness.DELAYED_UNKNOWN);
        _defaultWorstQos.RateInfo(65535);
    }

    public Queue<IRequestMsg> RecoverItemQueue()
    {
        return _recoveryItemQueue;
    }

    public void CallbackClient(CallbackClient<T> client)
    {
        _callbackClient = client;
    }

    public Dictionary<long, Item<T>> ItemHandleMap()
    {
        return m_ItemHandleDict;
    }

    public Dictionary<int, Item<T>> StreamIdMap()
    {
        return m_StreamIdDict;
    }

    public void SendItemStatus(SingleItem<T> item, IRequestMsg requestMsg, int streamState, int dataState, int statusCode, string statusText)
    {
        Eta.Codec.IStatusMsg rsslStatusMsg = _callbackClient!.StatusMsg();

        rsslStatusMsg.StreamId = item.StreamId;
        rsslStatusMsg.DomainType = requestMsg.DomainType;
        rsslStatusMsg.ContainerType = DataTypes.NO_DATA;

        rsslStatusMsg.ApplyHasState();
        rsslStatusMsg.State.StreamState(streamState);
        rsslStatusMsg.State.DataState(dataState);
        rsslStatusMsg.State.Code(statusCode);

        _statusText.Data(statusText);
        rsslStatusMsg.State.Text(_statusText);

        rsslStatusMsg.ApplyHasMsgKey();
        requestMsg.MsgKey.Copy(rsslStatusMsg.MsgKey);

        if (requestMsg.CheckPrivateStream())
            rsslStatusMsg.ApplyPrivateStream();

        if (_callbackClient.m_StatusMsg == null)
            _callbackClient.m_StatusMsg = new StatusMsg(_callbackClient.commonImpl.GetEmaObjManager());

        _callbackClient.m_StatusMsg.Decode(rsslStatusMsg, Codec.MajorVersion(), Codec.MajorVersion(), null);

        var serviceDirectory = item.Directory();

        if (item.ServiceList != null)
        {
            _callbackClient.m_StatusMsg.SetServiceName(item.ServiceList.Name);
            _callbackClient.m_StatusMsg.ServiceIdInt(item.ServiceList.ServiceId);
        }
        else if (serviceDirectory != null && serviceDirectory.ServiceName != null)
        {
            _callbackClient.m_StatusMsg.SetServiceName(serviceDirectory.ServiceName);
            _callbackClient.m_StatusMsg.ServiceIdInt(serviceDirectory.GeneratedServiceId());
        }
        else if (item.ServiceName != null)
        {
            _callbackClient.m_StatusMsg.SetServiceName(item.ServiceName);
        }

        _callbackClient.EventImpl.Item = item;
        _callbackClient.NotifyOnAllMsg(_callbackClient.m_StatusMsg);
        _callbackClient.NotifyOnStatusMsg();
    }

    public void SubmitItemRecovery()
    {
        int count = _recoveryItemQueue.Count;
        IRequestMsg? rsslRequestMsg = _recoveryItemQueue.Count > 0 ? _recoveryItemQueue.Dequeue() : null;

        while (rsslRequestMsg != null)
        {
            m_StreamIdDict.TryGetValue(rsslRequestMsg.StreamId, out var result);

            SingleItem<T>? item = (SingleItem<T>?)result;

            /* Checks to ensure that the item exists. */
            if (item != null && (item.State == SingleItem<T>.StateEnum.RECOVERING || item.State == SingleItem<T>.StateEnum.RECOVERING_NO_MATHCING))
            {
                ServiceList? serviceList = item.ServiceList;

                if (serviceList == null)
                {
                    ChannelInfo channelInfo = item.Directory()!.ChannelInfo!;

                    // Gets a ServiceDirectory from SessionDirectory
                    ServiceDirectory<T>? directory = item.Directory()!.SessionDirectory!.UpdateSessionChannelInfo(rsslRequestMsg.DomainType,
                        channelInfo.ReactorChannel, true, item.ItemClosedDirHash);

                    if (directory == null)
                    {
                        if (item.State == SingleItem<T>.StateEnum.RECOVERING)
                        {
                            if (item.ItemClosedDirHash == null)
                            {
                                SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.OPEN,
                                        OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "No matching service present.");

                                item.State  = SingleItem<T>.StateEnum.RECOVERING_NO_MATHCING;

                                _recoveryItemQueue.Enqueue(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/

                                _consumerSession.NextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
                            }
                            else
                            {
                                /* Sends the closed status message and removes this item as there is no session channel to retry */
                                SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.CLOSED,
                                        item.LastDataState, item.LastStatusCode, item.LastStatusText);

                                item.Remove();
                            }
                        }
                        else
                        {
                            _recoveryItemQueue.Enqueue(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/

                            _consumerSession.NextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
                        }
                    }
                    else
                    {
                        item.Directory(directory);

                        if (item.ItemClosedDirHash != null)
                        {
                            /* Notify the application with the last status message from the provider */
                            SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.OPEN,
                                    item.LastDataState, item.LastStatusCode, item.LastStatusText);
                        }

                        if (item.Submit(rsslRequestMsg, item.ServiceName, false, false))
                        {
                            /* The item state is changed to normal item stream */
                            item.State = SingleItem<T>.StateEnum.NORMAL;
                        }
                        else
                        {
                            _recoveryItemQueue.Enqueue(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
                            _consumerSession.NextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
                        }
                    }
                }
                else
                {
                    ChannelInfo channelInfo = item.Directory()!.ChannelInfo!;
                    SessionDirectory<T> currentSessionDir = item.Directory()!.SessionDirectory!;

                    // Gets the same service from others connection if any.
                    ServiceDirectory<T>? directory = currentSessionDir.UpdateSessionChannelInfo(rsslRequestMsg.DomainType, channelInfo.ReactorChannel, 
                        item.RetrytosameChannel, item.ItemClosedDirHash);

                    if (directory == null)
                    {
                        string currentServiceName = item.Directory()!.ServiceName!;
                        foreach (string serviceName in serviceList.ConcreteServiceList)
                        {
                            /* Try to recover with the next service name in the list as the current service name is not available from others connection. */
                            if (serviceName.Equals(currentServiceName))
                                continue;

                            SessionDirectory<T>? sessionDirectory = item.Session().GetSessionDirectoryByName(serviceName);

                            if (sessionDirectory != null)
                            {
                                directory = sessionDirectory.Directory(item.RequestMsg);

                                if (directory == null)
                                {
                                    continue;
                                }
                                else
                                {
                                    if (item.ItemClosedDirHash != null)
                                    {   /* Moves to next directory if the directory is requested */
                                        if (item.ItemClosedDirHash.Contains(directory))
                                        {
                                            directory = null;
                                            continue;
                                        }
                                    }

                                    item.Directory(directory);
                                    item.ServiceName = directory.ServiceName;

                                    if (item.ItemClosedDirHash != null)
                                    {
                                        /* Notify the application with the last status message from the provider */
                                        SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.OPEN,
                                                item.LastDataState, item.LastStatusCode, item.LastStatusText);
                                    }

                                    if (item.Submit(rsslRequestMsg, item.ServiceName, false, false))
                                    {
                                        /* The item state is changed to normal item stream */
                                        item.State = SingleItem<T>.StateEnum.NORMAL;
                                        item.RetrytosameChannel = false;
                                        break;
                                    }
                                }
                            }
                        }

                        /* Try again with the same service name on the same ReactorChannel */
                        if (directory == null)
                        {
                            directory = currentSessionDir.UpdateSessionChannelInfo(rsslRequestMsg.DomainType, channelInfo.ReactorChannel, true,
                                item.ItemClosedDirHash);

                            if (directory != null)
                            {
                                item.Directory(directory);

                                if (item.ItemClosedDirHash != null)
                                {
                                    /* Notify the application with the last status message from the provider */
                                    SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.OPEN,
                                            item.LastDataState, item.LastStatusCode, item.LastStatusText);
                                }

                                if (item.Submit(rsslRequestMsg, item.ServiceName, false, false))
                                {
                                    /* The item state is changed to normal item stream */
                                    item.State = SingleItem<T>.StateEnum.NORMAL;
                                    item.RetrytosameChannel = false;
                                }
                            }
                        }

                        if (item.State !=  SingleItem<T>.StateEnum.NORMAL)
                        {
                            item.RetrytosameChannel = true;

                            if (item.State == SingleItem<T>.StateEnum.RECOVERING)
                            {
                                if (item.ItemClosedDirHash == null)
                                {
                                    SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.OPEN,
                                            OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "No matching service present.");

                                    item.State = SingleItem<T>.StateEnum.RECOVERING_NO_MATHCING;

                                    _recoveryItemQueue.Enqueue(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/

                                    _consumerSession.NextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
                                }
                                else
                                {
                                    /* Sends the closed status message and removes this item as there is no session channel to retry */
                                    SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.CLOSED,
                                            item.LastDataState, item.LastStatusCode, item.LastStatusText);

                                    item.Remove();
                                }
                            }
                            else
                            {
                                _recoveryItemQueue.Enqueue(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/

                                _consumerSession.NextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
                            }
                        }
                    }
                    else
                    {
                        item.Directory(directory);

                        if (item.ItemClosedDirHash != null)
                        {
                            /* Notify the application with the last status message from the provider */
                            SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.OPEN,
                                    item.LastDataState, item.LastStatusCode, item.LastStatusText);
                        }

                        if (item.Submit(rsslRequestMsg, item.ServiceName, false, false))
                        {
                            /* The item state is changed to normal item stream */
                            item.State = SingleItem<T>.StateEnum.NORMAL;
                            item.RetrytosameChannel = false;
                        }
                        else
                        {
                            _recoveryItemQueue.Enqueue(rsslRequestMsg); /* Add this RequestMsg back to the request item queue to process later.*/
                            _consumerSession.NextDispatchTime(1000000); /* Wait for 1 second to recover the recover queue */
                        }
                    }
                }
            }

            if ((--count) == 0)
                break;

            rsslRequestMsg = _recoveryItemQueue.Dequeue();
        }
    }

    /* This must be called only after the connection is recovered for a ReactorChannel */
    public void SubmitItemCloseForChannel(ReactorChannel reactorChannel)
    {
        int count = _closingItemQueue.Count;
        SingleItem<T>? singleItem = _closingItemQueue.Count > 0 ? _closingItemQueue.Dequeue() : null;

        while (singleItem != null)
        {
            if (singleItem.Directory()!.ChannelInfo!.ReactorChannel == reactorChannel)
            {
                singleItem.Close();

                /* Sets the state that the item is being recovered */
                singleItem.State = SingleItem<T>.StateEnum.RECOVERING;

                /* Add item to recovery queue to retry with another connection if any */
                _recoveryItemQueue.Enqueue(singleItem.RequestMsg!);
            }
            else
            {
                _closingItemQueue.Enqueue(singleItem);
            }

            if ((--count) == 0)
                break;

            singleItem = _closingItemQueue.Dequeue();
        }
    }

    public ReactorCallbackReturnCode HandleItemStatus(IStatusMsg rsslStatusMsg, StatusMsg statusMsg, Item<T> item)
    {
        SingleItem<T>? singleItem = null;
        if (item is not null and SingleItem<T>)
        {
            singleItem = (SingleItem<T>)item;
        }

        else
        {
            return ReactorCallbackReturnCode.SUCCESS;
        }

        bool handleConnectionRecovering = false;
        var serviceDirectory = item.Directory();

        if (serviceDirectory != null)
        {
            if (singleItem.ServiceList != null)
            {
                statusMsg.SetServiceName(singleItem.ServiceList.Name);
                statusMsg.ServiceIdInt(singleItem.ServiceList.ServiceId);
            }
            else if (serviceDirectory.HasGeneratedServiceId) /* Gets the mapping alternate service Id if any */
                statusMsg.ServiceIdInt(serviceDirectory.GeneratedServiceId());

            if (!_sessionEnhancedItemRecovery && serviceDirectory.ChannelInfo!.ReactorChannel != null)
            {
                if (serviceDirectory.ChannelInfo!.ReactorChannel.State != ReactorChannelState.UP &&
                        serviceDirectory.ChannelInfo!.ReactorChannel.State != ReactorChannelState.READY)
                {
                    handleConnectionRecovering = true;
                }
            }
        }

        State state = rsslStatusMsg.State;
        int originalStreamState = rsslStatusMsg.CheckHasState() ? state.StreamState() : StreamStates.UNSPECIFIED;
        bool notifyStatusMsg = true;

        while (true && singleItem.RequestMsg != null && rsslStatusMsg.CheckHasState())
        {
            IRequestMsg requestMsg = singleItem.RequestMsg;

            if (item.Type() == ItemType.SINGLE_ITEM && rsslStatusMsg.CheckHasState())
            {
                if (state.StreamState() == StreamStates.CLOSED_RECOVER)
                {
                    /* Recover item only for non-private item stream. */
                    if (requestMsg.CheckPrivateStream() == false)
                    {
                        state.StreamState(StreamStates.OPEN);

                        /* Sets the state that the item is being recovered */
                        singleItem.State = SingleItem<T>.StateEnum.RECOVERING;

                        /* Add item to recovery queue to retry with another connection if any */
                        _recoveryItemQueue.Enqueue(requestMsg);
                    }
                }
                else if (state.StreamState() == StreamStates.OPEN && state.DataState() == DataStates.SUSPECT)
                {
                    /* Recover item only for non-private item stream. */
                    if (requestMsg.CheckPrivateStream() == false)
                    {
                        /* This is used to close this stream with the watchlist only */
                        singleItem.State = SingleItem<T>.StateEnum.CLOSING_STREAM;

                        try
                        {
                            singleItem.Close();
                        }
                        catch (OmmInvalidUsageException iue)
                        {
                            /* This will be closed later when the ReactorChannel is operational  */
                            if (singleItem.Directory()!.ChannelInfo!.ReactorChannel!.State == ReactorChannelState.CLOSED)
                            {
                                _closingItemQueue.Enqueue(singleItem);
                                break;
                            }
                            else if (iue.ErrorCode == OmmInvalidUsageException.ErrorCodes.SHUTDOWN)
                            {
                                /* Remove this item as Reactor is shutdown */
                                singleItem.Remove();

                                state.StreamState(StreamStates.CLOSED);
                                break;
                            }
                        }

                        if (handleConnectionRecovering)
                        {
                            /* Sets the state that the item is being recovered */
                            singleItem.State = SingleItem<T>.StateEnum.RECOVERING;

                            /* Waiting to recover this item with the same channel */
                            singleItem.RetrytosameChannel = true;

                            /* Added this item into the recovering queue of SessionDirectory to recover once the requested service is ready. */
                            singleItem.Directory()!.SessionDirectory!.AddRecoveringQueue(singleItem);

                        }
                        else
                        {
                            /* Sets the state that the item is being recovered */
                            singleItem.State = SingleItem<T>.StateEnum.RECOVERING;

                            /* Add item to recovery queue to retry with another connection if any */
                            _recoveryItemQueue.Enqueue(requestMsg);
                        }
                    }
                }
                else if (state.StreamState() == StreamStates.CLOSED)
                {
                    /* Recover item only for non-private item stream. */
                    if (requestMsg.CheckPrivateStream() == false)
                    {
                        state.StreamState(StreamStates.OPEN);

                        /* Sets the state that the item is being recovered */
                        singleItem.State = SingleItem<T>.StateEnum.RECOVERING;

                        singleItem.ItemClosedDirHash ??= new HashSet<ServiceDirectory<T>>();

                        singleItem.ItemClosedDirHash.Add(item.Directory()!);

                        singleItem.LastDataState = state.DataState();
                        singleItem.LastStatusCode = state.Code();
                        singleItem.LastStatusText = state.Text().ToString();

                        /* Add item to recovery queue to retry with another connection if any */
                        _recoveryItemQueue.Enqueue(requestMsg);

                        notifyStatusMsg = false;
                    }
                }
            }
            break;
        }

        if (notifyStatusMsg)
        {
            _callbackClient!.NotifyOnAllMsg(statusMsg);
            _callbackClient.NotifyOnStatusMsg();
        }

        if (rsslStatusMsg.CheckHasState() && state.StreamState() != StreamStates.OPEN)
        {
            item.Remove();
        }

        /* Restore the original stream state */
        if (rsslStatusMsg.CheckHasState())
        {
            state.StreamState(originalStreamState);
        }

        _consumerSession.NextDispatchTime(1000); // Wait for 1 millisecond to recover

        return ReactorCallbackReturnCode.SUCCESS;
    }

    /* Checks whether service's capability and QoS match with the request message */
    public bool CheckMatchingService(IRequestMsg rsslRequestMsg, Service service,
            WatchlistResult result)
    {
        /* Check to ensure that the service is up and accepting request */
        if (!service.HasState || service.State.ServiceStateVal == 0 || service.State.AcceptingRequests == 0)
        {
            result.ResultCode = WatchlistResult.CodeEnum.SERVICE_NOT_UP;
            result.ResultText = "Service not up";
            return false;
        }

        if (!IsCapabilitySupported(rsslRequestMsg.DomainType, service))
        {
            result.ResultCode = WatchlistResult.CodeEnum.CAPABILITY_NOT_FOUND;
            result.ResultText = "Capability not supported";
            return false;
        }

        _matchedQos.Clear();

        Qos qos;
        Qos worstQos;

        if (!rsslRequestMsg.CheckHasQos())
        {
            qos = _defaultQos;
            worstQos = _defaultWorstQos;
        }
        else
        {
            qos = rsslRequestMsg.Qos;
            worstQos = rsslRequestMsg.WorstQos;
        }

        if (!IsQosSupported(qos, worstQos, service, _matchedQos))
        {
            result.ResultCode = WatchlistResult.CodeEnum.MATCHING_QOS_NOT_FOUND;
            result.ResultText = "Service does not provide a matching QoS";
            return false;
        }

        result.ResultCode = WatchlistResult.CodeEnum.SUCCESS;
        result.ResultText = "";
        return true;
    }

    /* Determines if a service supports a capability. */
    internal static bool IsCapabilitySupported(int domainType, Service service)
    {
        bool ret = false;

        if (service.HasInfo)
        {
            for (int i = 0; i < service.Info.CapabilitiesList.Count; i++)
            {
                if (service.Info.CapabilitiesList[i] == domainType)
                {
                    ret = true;
                    break;
                }
            }
        }

        return ret;
    }

    /* Determines if a service supports a qos or qos range. */
    internal bool IsQosSupported(Qos qos, Qos worstQos, Service service, Qos matchedQos)
    {
        bool ret = false;

        if (service.HasInfo)
        {
            if (service.Info.HasQos)
            {
                // service has qos
                for (int i = 0; i < service.Info.QosList.Count; i++)
                {
                    Qos serviceQos = service.Info.QosList[i];
                    if (worstQos == null)
                    {
                        // no worst qos, determine if request qos supported by service
                        if (serviceQos.Equals(qos))
                        {
                            ret = true;
                            serviceQos.Copy(matchedQos);
                            break;
                        }
                    }
                    else // worstQos specified
                    {
                        if (serviceQos.IsInRange(qos, worstQos))
                        {
                            if (serviceQos.IsBetter(matchedQos))
                            {
                                ret = true;
                                serviceQos.Copy(matchedQos);
                            }
                        }
                    }
                }
            }
            else // service has no qos
            {
                // determine if qos matches default of Realtime, Tick-By-Tick
                if (worstQos == null)
                {
                    ret = _defaultQos.Equals(qos);
                }
                else // worstQos specified
                {
                    ret = _defaultQos.IsInRange(qos, worstQos);
                }

                if (ret == true)
                {
                    _defaultQos.Copy(matchedQos);
                }
            }
        }

        // set isDynamic flag to that of requested qos before returning
        if (ret == true)
        {
            matchedQos.IsDynamic = qos.IsDynamic;
        }

        return ret;
    }

    /* This is used to fan-out item closed for remaining recovery items */
    public void Close()
    {
        IRequestMsg? rsslRequestMsg = _recoveryItemQueue.Count > 0 ? _recoveryItemQueue.Dequeue() : null;

        while (rsslRequestMsg != null)
        {
            m_StreamIdDict.TryGetValue(rsslRequestMsg.StreamId, out var result);

            SingleItem<T>? item = (SingleItem<T>?)result;

            if (item != null)
            {
                SendItemStatus(item, rsslRequestMsg, OmmState.StreamStates.CLOSED,
                    OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Consumer session is closed.");
            }

            if (_recoveryItemQueue.Count == 0)
                break;

            rsslRequestMsg = _recoveryItemQueue.Dequeue();
        }
    }
}

