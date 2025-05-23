/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|              Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;

namespace LSEG.Ema.Access
{
    internal class SessionDirectory<T>
    {
        private ConsumerSession<T> m_ConsumerSession;

        internal Service? Service { get; set; }

        internal string ServiceName { get; set; }

        internal bool IsUpdated { get; set; } = false;  // This is used to indicate whether there is an update to fan-out.

        // This is used to restore the original service flag after generating a source directory response from request filter
        internal ServiceFlags OriginalServiceFlags { get; set; }

        internal List<SessionChannelInfo<IOmmConsumerClient>> SessionChannelInfoList = new();

        private Dictionary<string, HashSet<SingleItem<T>>> _itemNameMap; // This is active item map for requesting the same item name and service name to the same channel.

        // This is recovery item map to wait for the service to recover for a channel. This map is used for normal channel only.
        private Dictionary<SessionChannelInfo<T>, SortedDictionary<string, HashSet<SingleItem<T>>>> _recoveringItemMapBySessionChannel;
        private Queue<SingleItem<T>> _pendingItemQueue; // This is used to recover items when the concrete service is available.
        internal WatchlistResult WatchlistResult { get; private set; }
        private WatchlistResult _tempWatchlistResult;

        private Queue<SingleItem<T>> m_TempRemoveSingleItems = new Queue<SingleItem<T>>();

        readonly Qos _matchedQos = new Qos();

        internal SessionDirectory(ConsumerSession<T> consumerSession, string serviceName)
        {
            ServiceName = serviceName;
            m_ConsumerSession = consumerSession;

            _itemNameMap = new Dictionary<string, HashSet<SingleItem<T>>>();
            _recoveringItemMapBySessionChannel = new Dictionary<SessionChannelInfo<T>, SortedDictionary<string, HashSet<SingleItem<T>>>>();
            _pendingItemQueue = new Queue<SingleItem<T>>();

            WatchlistResult = new WatchlistResult();
            _tempWatchlistResult = new WatchlistResult();
        }

        internal void HandleSessionChannelClose(SessionChannelInfo<T> sessionChannelInfo)
        {
            SortedDictionary<string, HashSet<SingleItem<T>>>? recoveringItemMap;

            if (_recoveringItemMapBySessionChannel.TryGetValue(sessionChannelInfo, out recoveringItemMap))
            {
                var iter = recoveringItemMap.GetEnumerator();

                while (iter.MoveNext())
                {
                    var entry = iter.Current;
                    string itemName = entry.Key;
                    HashSet<SingleItem<T>> hashSet = entry.Value;

                    var singleItemIt = hashSet.GetEnumerator();
                    m_TempRemoveSingleItems.Clear();

                    SingleItem<T>? singleItem;
                    while (singleItemIt.MoveNext())
                    {
                        singleItem = singleItemIt.Current;

                        ServiceDirectory<T>? directory;
                        directory = Directory(singleItem.RequestMsg);

                        if (directory != null)
                        {
                            singleItem.Directory(directory);
                            singleItem.ServiceName = ServiceName;

                            /* The item state is changed to normal item stream */
                            singleItem.State = SingleItem<T>.StateEnum.NORMAL;
                            if (singleItem.Submit(singleItem.RequestMsg!, singleItem.ServiceName, false, false) == false)
                            {
                                singleItem.State = SingleItem<T>.StateEnum.RECOVERING;
                                m_ConsumerSession.SessionWatchlist.RecoverItemQueue().Enqueue(singleItem.RequestMsg!);
                                m_ConsumerSession.NextDispatchTime(1000); // Wait for 1 millisecond to recover
                            }
                        }
                        else
                        {
                            singleItem.State = SingleItem<T>.StateEnum.RECOVERING;
                            m_ConsumerSession.SessionWatchlist.RecoverItemQueue().Enqueue(singleItem.RequestMsg!);
                            m_ConsumerSession.NextDispatchTime(1000); // Wait for 1 millisecond to recover
                        }

                        /* Removes from the recovery queue after the while loop */
                        m_TempRemoveSingleItems.Enqueue(singleItem);

                        /* Adds to the active queue */
                        _itemNameMap[itemName].Add(singleItem);
                    }

                    singleItem = m_TempRemoveSingleItems.Count > 0 ? m_TempRemoveSingleItems.Dequeue() : null;
                    while (singleItem != null && hashSet != null)
                    {
                        /* Removes the SingleItem from the recovery queue */
                        hashSet.Remove(singleItem);

                        if (m_TempRemoveSingleItems.Count == 0)
                            break;

                        singleItem = m_TempRemoveSingleItems.Dequeue();
                    }
                }

                _recoveringItemMapBySessionChannel.Remove(sessionChannelInfo);
            }
        }

        internal void ResetUpdateFlags()
        {
            IsUpdated = false;
        }

        internal void RestoreServiceFlags()
        {
            if (Service != null)
            {
                Service.Flags = OriginalServiceFlags;
            }
        }

        internal ConsumerSession<T> ConsumerSession()
        {
            return m_ConsumerSession;
        }

        internal void SetService(Service service)
        {
            Service = service;
            OriginalServiceFlags = service.Flags;
        }

        internal void AddPendingRequest(SingleItem<T> singleItem, RequestMsg reqMsg)
        {
            singleItem.RequestMsg = new Eta.Codec.Msg();

            singleItem.RequestMsg.MsgClass = MsgClasses.REQUEST;
            reqMsg.m_rsslMsg.Copy(singleItem.RequestMsg, CopyMsgFlags.ALL_FLAGS);

            _pendingItemQueue.Enqueue(singleItem);
        }

        internal void HandlePendingRequests(SessionChannelInfo<T> sessionChannelInfo, Service service)
        {
            int count = _pendingItemQueue.Count;
            SingleItem<T>? singleItem = _pendingItemQueue.Count > 0 ? _pendingItemQueue.Dequeue() : null;

            ServiceDirectory<T>? directory;

            while (singleItem != null)
            {
                /* Handles this item when it hasn't been removed from the item map */
                if (m_ConsumerSession.SessionWatchlist.ItemHandleMap().ContainsKey(singleItem.ItemId))
                {
                    directory = Directory(singleItem.RequestMsg);

                    if (directory != null)
                    {
                        singleItem.Directory(directory);
                        singleItem.ServiceName = ServiceName;

                        /* The item state is changed to normal item stream */
                        singleItem.State = SingleItem<T>.StateEnum.NORMAL;
                        singleItem.Submit(singleItem.RequestMsg!, ServiceName, false, false);

                        if (!string.IsNullOrEmpty(singleItem.ItemName))
                            PutDirectoryByItemName(singleItem.ItemName, singleItem);
                    }
                    else
                    {
                        _pendingItemQueue.Enqueue(singleItem);
                    }
                }

                if ((--count) == 0)
                {
                    break;
                }

                singleItem = _pendingItemQueue.Dequeue();
            }

            HandleRecoveringRequests(sessionChannelInfo, true);
        }

        internal void HandleRecoveringRequests(SessionChannelInfo<T> sessionChannelInfo, bool sameChannel)
        {
            SortedDictionary<string, HashSet<SingleItem<T>>>? recoveringItemMap = null;

            if (_recoveringItemMapBySessionChannel.TryGetValue(sessionChannelInfo, out recoveringItemMap))
            {
                var iter = recoveringItemMap.GetEnumerator();

                HashSet<SingleItem<T>>? hashSet = null;
                while (iter.MoveNext())
                {
                    var entry = iter.Current;
                    string itemName = entry.Key;
                    hashSet = entry.Value;

                    var singleItemIt = hashSet.GetEnumerator();

                    int count = hashSet.Count;

                    /* Always clears the quque in order to add a list of SingleItem. */
                    m_TempRemoveSingleItems.Clear();

                    SingleItem<T>? singleItem;
                    while (singleItemIt.MoveNext())
                    {
                        singleItem = singleItemIt.Current;

                        if (sameChannel)
                        {
                            if (ReferenceEquals(singleItem.Directory()!.ChannelInfo!.SessionChannelInfo,sessionChannelInfo))
                            {
                                bool isMatched = m_ConsumerSession.SessionWatchlist.CheckMatchingService(singleItem.RequestMsg!, singleItem.Directory()!.Service!, _tempWatchlistResult);

                                if (isMatched)
                                {
                                    singleItem.ServiceName = ServiceName;

                                    /* The item state is changed to normal item stream */
                                    singleItem.State = SingleItem<T>.StateEnum.NORMAL;
                                    singleItem.Submit(singleItem.RequestMsg!, ServiceName, false, false);

                                    /* Removes from the recovery queue after the while loop */
                                    m_TempRemoveSingleItems.Enqueue(singleItem);

                                    /* Adds to the active queue */
                                    _itemNameMap[itemName].Add(singleItem);
                                }
                                else
                                {
                                    /* Don't remove this item in order to try again later once the service is ready */
                                }
                            }
                        }
                        else
                        {
                            ServiceDirectory<T>? directory;
                            directory = Directory(singleItem.RequestMsg);

                            if (directory != null)
                            {
                                singleItem.Directory(directory);
                                singleItem.ServiceName = ServiceName;

                                /* The item state is changed to normal item stream */
                                singleItem.State = SingleItem<T>.StateEnum.NORMAL;
                                singleItem.Submit(singleItem.RequestMsg!, ServiceName, false, false);
                            }
                            else
                            {
                                singleItem.State = SingleItem<T>.StateEnum.RECOVERING;
                                m_ConsumerSession.SessionWatchlist.RecoverItemQueue().Enqueue(singleItem.RequestMsg!);
                                m_ConsumerSession.NextDispatchTime(1000); // Wait for 1 millisecond to recover
                            }

                            /* Removes from the recovery queue after the while loop */
                            m_TempRemoveSingleItems.Enqueue(singleItem);

                            /* Adds to the active queue */
                            _itemNameMap[itemName].Add(singleItem);
                        }

                        if ((--count) == 0)
                        {
                            break;
                        }

                    } // End while loop

                    singleItem =  m_TempRemoveSingleItems.Count > 0 ? m_TempRemoveSingleItems.Dequeue() : null;
                    while(singleItem != null && hashSet != null)
                    {
                        /* Removes the SingleItem from the recovery queue */
                        hashSet.Remove(singleItem);

                        if (m_TempRemoveSingleItems.Count == 0)
                            break;

                        singleItem = m_TempRemoveSingleItems.Dequeue();
                    }
                }
            }
        }

        internal ServiceDirectory<T>? UpdateSessionChannelInfo(IRequestMsg requestMsg, ReactorChannel? current, bool retryWithCurrentDir, 
            HashSet<ServiceDirectory<T>>? itemClosedDirHash)
        {
            if (SessionChannelInfoList.Count != 0)
            {
                /* Gets the Directory from the first available ReactorChannel*/
                ServiceDirectory<T>? directory = null;
                SessionChannelInfo<T>? currentSession = null;
                ServiceDirectory<T>? currentDirectory = null;

                foreach (SessionChannelInfo<IOmmConsumerClient> sessionChannel in SessionChannelInfoList)
                {
                    directory = (dynamic?)sessionChannel.GetDirectoryByName(ServiceName);

                    if (directory != null)
                    {
                        if (itemClosedDirHash != null)
                        {   /* Moves to next directory if the directory is requested */
                            if (itemClosedDirHash.Contains(directory))
                                continue;
                        }

                        /* Ensure that the service provides the requested capability */
                        if (!SessionWatchlist<T>.IsCapabilitySupported(requestMsg.DomainType, directory.Service!))
                        {
                            continue;
                        }

                        /* Ensure that the service provides the requested QoS */
                        _matchedQos.Clear();
                        if (!m_ConsumerSession.SessionWatchlist.IsQosSupported(requestMsg.Qos, requestMsg.WorstQos, directory.Service!, _matchedQos))
                        {
                            continue;
                        }

                        if (object.ReferenceEquals(directory.ChannelInfo!.ReactorChannel,current))
                        {
                            if (retryWithCurrentDir)
                            {
                                currentSession = (dynamic)sessionChannel;
                                currentDirectory = directory;
                            }
                            continue;
                        }

                        if (directory.Service!.HasState)
                        {
                            ServiceState serviceState = directory.Service.State;
                            /* Check to ensure that the service is up and accepting request */
                            if (serviceState.ServiceStateVal != 0 && (!serviceState.HasAcceptingRequests || serviceState.AcceptingRequests != 0))
                            {
                                UpdateSessionDirectory(directory, this);/* Update SessionDirectory with a new one */
                                ReactorChannel? reactorChannel = directory.ChannelInfo.ReactorChannel;

                                if (reactorChannel != null &&
                                        (reactorChannel.State == ReactorChannelState.UP || reactorChannel.State == ReactorChannelState.READY))
                                {
                                    return directory;
                                }
                            }
                        }
                    }
                }

                /* Try to check with the current session */
                if (currentSession != null)
                {
                    if (currentDirectory!.Service!.HasState)
                    {
                        /* Check to ensure that the service is up and accepting request */
                        if (currentDirectory.Service.State.ServiceStateVal != 0 && (!currentDirectory.Service.State.HasAcceptingRequests || currentDirectory.Service.State.AcceptingRequests != 0))
                        {
                            ReactorChannel? reactorChannel = currentDirectory.ChannelInfo!.ReactorChannel;
                            if (reactorChannel != null &&
                                    (reactorChannel.State == ReactorChannelState.UP || reactorChannel.State == ReactorChannelState.READY))
                            {
                                UpdateSessionDirectory(currentDirectory, this); /* Update with the current SessionDirectory */
                                return currentDirectory;
                            }
                        }
                    }
                }
            }

            return null;
        }

        internal static void UpdateSessionDirectory(ServiceDirectory<T>? directory, SessionDirectory<T> sessionDirectory)
        {
            if (directory != null)
            {
                directory.GeneratedServiceId(sessionDirectory.Service!.ServiceId);
                directory.SessionDirectory = sessionDirectory;
            }
        }

        internal ServiceDirectory<T>? Directory(IRequestMsg? requestMsg)
        {
            if (SessionChannelInfoList.Count != 0)
            {
                /* Gets the Directory from the first available ReactorChannel*/
                ServiceDirectory<T>? directory = null;
                foreach (var session in SessionChannelInfoList)
                {
                    directory = (dynamic?)session.GetDirectoryByName(ServiceName);
                    if (directory != null)
                    {
                        Service? service = directory.Service;
                        if (service != null)
                        {
                            if (m_ConsumerSession.SessionWatchlist.CheckMatchingService(requestMsg!, service, WatchlistResult))
                            {
                                SessionDirectory<T>.UpdateSessionDirectory(directory, this);
                                break;
                            }
                            else
                            {
                                directory = null;
                            }
                        }
                    }
                }

                return directory;
            }
            else
            {
                WatchlistResult.ResultText = "No matching service present.";
                WatchlistResult.ResultCode = WatchlistResult.CodeEnum.SUCCESS;
            }

            return null;
        }

        /* This function is used by the tunnel stream feature to get a ServiceDirectory for opening a tunnel stream.*/
        internal ServiceDirectory<IOmmConsumerClient>? Directory()
        {
            WatchlistResult.Clear();
            if (SessionChannelInfoList.Count != 0)
            {
                /* Gets the Directory from the first available ReactorChannel*/
                ServiceDirectory<IOmmConsumerClient>? directory = null;
                foreach (var session in SessionChannelInfoList)
                {
                    directory = session.GetDirectoryByName(ServiceName);
                    if (directory != null)
                    {
                        Service? service = directory.Service;
                        if (service != null)
                        {
                            if (!service.HasState || service.State.ServiceStateVal == 0 || service.State.AcceptingRequests == 0)
                            {
                                WatchlistResult.ResultText = "Service not up";
                                continue;
                            }
                            else
                            {
                                return directory;
                            }
                        }
                    }
                }

                return directory;
            }
            else
            {
                WatchlistResult.ResultText = "No matching service present.";
            }

            return null;
        }

        internal ServiceDirectory<T>? MatchingWithExistingDirectory(HashSet<SingleItem<T>>? itemHashSet, IRequestMsg rsslRequestMsg)
        {
            if (itemHashSet != null)
            {
                var it = itemHashSet.GetEnumerator();

                SingleItem<T> singleItem;
                while (it.MoveNext())
                {
                    singleItem = it.Current;
                    ServiceDirectory<T>? directory = singleItem.Directory();

                    if (directory != null && m_ConsumerSession.SessionWatchlist.CheckMatchingService(rsslRequestMsg, directory.Service!, _tempWatchlistResult))
                    {
                        return directory;
                    }
                }
            }
            return null;
        }

        internal HashSet<SingleItem<T>>? GetDirectoryByItemName(string itemName)
        {
            if(_itemNameMap.TryGetValue(itemName, out var itemHashSet))
            {
                return itemHashSet;
            }
            else
                return null;
        }

        internal void PutDirectoryByItemName(string itemName, SingleItem<T> _item)
        {
            HashSet<SingleItem<T>>? itemSet;
            if (!_itemNameMap.TryGetValue(itemName, out itemSet))
            {
                itemSet = new HashSet<SingleItem<T>>();
            }

            itemSet.Add(_item);

            _itemNameMap[itemName] = itemSet;
        }

        internal void PutDirectoryByHashSet(HashSet<SingleItem<T>> itemSet, SingleItem<T> _item)
        {
            itemSet.Add(_item);
        }

        internal void RemoveDirectoryByItemName(string itemName, SingleItem<T> singleItem)
        {
            HashSet<SingleItem<T>>? itemSet;
            bool itemRemoved = false;
            if (_itemNameMap.TryGetValue(itemName, out itemSet))
            {
                itemRemoved = itemSet.Remove(singleItem);
            }

            if (!itemRemoved)
            {
                SessionChannelInfo<T> sessionChannelInfo = (dynamic)singleItem.Directory()!.ChannelInfo!.SessionChannelInfo!;

                SortedDictionary<string, HashSet<SingleItem<T>>>? recoveringItemMap;

                if(_recoveringItemMapBySessionChannel.TryGetValue(sessionChannelInfo, out recoveringItemMap))
                {
                    if(recoveringItemMap.TryGetValue(itemName, out itemSet))
                    {
                        itemSet.Remove(singleItem);
                    }
                }
            }
        }

        internal void AddRecoveringQueue(SingleItem<T> singleItem)
        {
            HashSet<SingleItem<T>>? hashSet;

            if (_itemNameMap.TryGetValue(singleItem.ItemName, out hashSet))
            {
                SessionChannelInfo<T> sessionChannelInfo = (dynamic)singleItem.Directory()!.ChannelInfo!.SessionChannelInfo!;

                SortedDictionary<string, HashSet<SingleItem<T>>>? recoveringItemMap;

                if (!_recoveringItemMapBySessionChannel.TryGetValue(sessionChannelInfo, out recoveringItemMap))
                {
                    recoveringItemMap = new SortedDictionary<string, HashSet<SingleItem<T>>>();
                    _recoveringItemMapBySessionChannel[sessionChannelInfo] = recoveringItemMap;
                }

                HashSet<SingleItem<T>>? recoveryHashSet;
                if (!recoveringItemMap.TryGetValue(singleItem.ItemName, out recoveryHashSet))
                {
                    HashSet<SingleItem<T>> itemSet = new()
                    {
                        singleItem
                    };

                    recoveringItemMap[singleItem.ItemName] =  itemSet;
                }
                else
                {
                    recoveryHashSet.Add(singleItem);
                }

                hashSet.Remove(singleItem);
            }
        }

        /* This is used to fan-out item closed for remaining recovery items */
        internal void Close()
        {
            /* Send item closed status from pending item queue */
            SingleItem<T>? singleItem = _pendingItemQueue.Count > 0 ? _pendingItemQueue.Dequeue() : null;

            while (singleItem != null && singleItem.RequestMsg != null)
            {
                /* Handles this item when it hasn't been removed from the item map */
                if (m_ConsumerSession.SessionWatchlist.ItemHandleMap().ContainsKey(singleItem.ItemId))
                {
                    m_ConsumerSession.SessionWatchlist.SendItemStatus(singleItem, singleItem.RequestMsg, OmmState.StreamStates.CLOSED,
                        OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Consumer session is closed.");
                }

                if (_pendingItemQueue.Count == 0)
                    break;

                singleItem = _pendingItemQueue.Dequeue();
            }

            var recoveringIt = _recoveringItemMapBySessionChannel.Values.GetEnumerator();
            while (recoveringIt.MoveNext())
            {
                var hashSetIt = recoveringIt.Current.Values.GetEnumerator();

                while (hashSetIt.MoveNext())
                {
                    foreach (SingleItem<T> sintleItemEntry in hashSetIt.Current)
                    {
                        m_ConsumerSession.SessionWatchlist.SendItemStatus(sintleItemEntry, sintleItemEntry.RequestMsg!, OmmState.StreamStates.CLOSED,
                                OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Consumer session is closed.");
                    }
                }
            }
        }
    }
}
