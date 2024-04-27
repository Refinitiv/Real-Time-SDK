/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.PerfTools.Common;
using LSEG.Ema.Access;
using LSEG.Eta.Rdm;

namespace LSEG.Ema.PerfTools.ConsPerf
{
    public class ConsumerThread : IOmmConsumerClient
    {
        private const int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
        private const string DEFAULT_PERF_CONSUMER_CONFIG_NAME = "Perf_Consumer_1";
        private const string DEFAULT_PERF_CONSUMER_NAME_WSJSON = "Perf_Consumer_WSJSON_1";
        private const string DEFAULT_PERF_CONSUMER_NAME_WSRWF = "Perf_Consumer_WSRWF_1";

        protected ConsumerThreadInfo _consThreadInfo; /* thread information */
        protected EmaConsPerfConfig _consPerfConfig; /* configuration information */
        private MarketPriceDecoder _marketPriceDecoder; /* market price decoder */
        private EmaDirectoryHandler _srcDirHandler = new EmaDirectoryHandler(); /* source directory handler */
        private XmlItemInfoList _itemInfoList; /* item information list from XML file */
        private XmlMsgData _xmlMsgData;
        private ItemEncoder _itemEncoder;
        private ItemRequest[] _itemRequestList; /* item request list */
        private int _postItemCount; /* number of items in _itemRequestList that are posting items */
        private int _genMsgItemCount; /* number of items in _itemRequestList that are for sending generic msgs on items */
        private RequestMsg _requestMsg = new RequestMsg(); /* request message */
        private GenericMsg _genMsg = new GenericMsg(); /* request message */
        private PostMsg _postMsg = new PostMsg();
        protected PostUserInfo _postUserInfo; /* post user information */
        protected bool _requestsSent; /* indicates if requested service is up */
        protected long _usecPerTick; /* nanoseconds per tick */
        private int _requestListSize; /* request list size */
        private int _requestListIndex; /* current request list index */
        private long _postsPerTick; /* posts per tick */
        private long _postsPerTickRemainder; /* posts per tick remainder */
        private long _genMsgsPerTick; /* gen msgs per tick */
        private long _genMsgsPerTickRemainder; /* gen msgs per tick remainder */
        private LatencyRandomArrayOptions _randomArrayOpts = new LatencyRandomArrayOptions(); /* random array options */
        protected LatencyRandomArray _postLatencyRandomArray = new LatencyRandomArray(); /* post random latency array */
        protected LatencyRandomArray _genMsgLatencyRandomArray = new LatencyRandomArray(); /* generic msg random latency array */
        private OmmConsumer _consumer;
        private OmmConsumerConfig _ommConfig;
        private long _itemHandle;
        private int _genMsgItemIndex;
        private bool _haveMarketPricePostItems; /* indicates there are post items in the item list */
        private bool _haveMarketPriceGenMsgItems; /* indicates there are generic msg items in the item list */
        private int _postMsgItemIndex;

#pragma warning disable CS8618
        public ConsumerThread(ConsumerThreadInfo consInfo, 
            EmaConsPerfConfig consConfig, 
            XmlItemInfoList itemList, 
            XmlMsgData xmlMsgData, 
            PostUserInfo postUserInfo)
        {
            _consThreadInfo = consInfo;
            _consPerfConfig = consConfig;
            _requestListSize = _consThreadInfo.ItemListCount;
            _itemInfoList = itemList;
            _xmlMsgData = xmlMsgData;
            _itemRequestList = new ItemRequest[_requestListSize];
            _postUserInfo = postUserInfo;
            for (int i = 0; i < _requestListSize; ++i)
            {
                _itemRequestList[i] = new ItemRequest();
            }
            _requestListIndex = 0;
            _marketPriceDecoder = new MarketPriceDecoder(_postUserInfo);
            _itemEncoder = new ItemEncoder(_xmlMsgData);
        }
#pragma warning restore CS8618

        /* Initializes consumer thread. */
        protected void Initialize()
        {
            // create latency log file writer for this thread 
            if (_consPerfConfig.LogLatencyToFile)
            {
                // Open latency log file. 
                string latencyFileName = _consPerfConfig.LatencyLogFilename + _consThreadInfo.ThreadId + ".csv";
                _consThreadInfo.LatencyLogFile = latencyFileName;
                
                try
                {
                    _consThreadInfo.LatencyLogFileWriter = new StreamWriter(latencyFileName);
                }
                catch
                {
                    Console.WriteLine($"Error: Failed to open latency log file {_consThreadInfo.LatencyLogFile}.\n");
                    Environment.Exit(-1);
                }
                _consThreadInfo.LatencyLogFileWriter.WriteLine("Message type, Send Time, Receive Time, Latency (usec)\n");
            }

            // create stats file writer for this thread 
            string fileName = _consPerfConfig.StatsFilename + _consThreadInfo.ThreadId + ".csv";
            _consThreadInfo.StatsFile = fileName;
            
            try
            {
                _consThreadInfo.StatsFileWriter = new StreamWriter(fileName);
            }
            catch
            {
                Console.WriteLine($"Error: Failed to open stats file {_consThreadInfo.StatsFile}.\n");
                Environment.Exit(-1);
            }
            _consThreadInfo.StatsFileWriter.WriteLine("UTC, Latency updates, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), Images, Update rate, Posting Latency updates, Posting Latency avg (usec), Posting Latency std dev (usec), Posting Latency max (usec), Posting Latency min (usec), GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%), Memory(MB)");

            // Create latency random array for post messages. Latency random array is used
            // to randomly insert latency RIC fields into post messages while sending bursts. 
            if (_consPerfConfig.LatencyPostsPerSec > 0)
            {
                _randomArrayOpts.TotalMsgsPerSec = _consPerfConfig.PostsPerSec;
                _randomArrayOpts.LatencyMsgsPerSec = _consPerfConfig.LatencyPostsPerSec;
                _randomArrayOpts.TicksPerSec = _consPerfConfig.TicksPerSec;
                _randomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (_postLatencyRandomArray.Create(_randomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error: Incorrect Post LatencyRandomArrayOptions.");
                    Environment.Exit(-1);
                }
            }

            // Create latency random array for generic messages. Latency random array is used
            // to randomly insert latency RIC fields into generic messages while sending bursts.
            if (_consPerfConfig.LatencyGenMsgsPerSec > 0)
            {
                _randomArrayOpts.TotalMsgsPerSec = _consPerfConfig.GenMsgsPerSec;
                _randomArrayOpts.LatencyMsgsPerSec = _consPerfConfig.LatencyGenMsgsPerSec;
                _randomArrayOpts.TicksPerSec = _consPerfConfig.TicksPerSec;
                _randomArrayOpts.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (_genMsgLatencyRandomArray.Create(_randomArrayOpts) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.WriteLine("Error: Incorrect Generic Msg LatencyRandomArrayOptions.");
                    Environment.Exit(-1);
                }
            }

            // populate item information from the XML list. 
            PopulateItemInfo();

            // initialize time tracking parameters 
            _usecPerTick = 1000000 / _consPerfConfig.TicksPerSec;
            _postsPerTick = _consPerfConfig.PostsPerSec / _consPerfConfig.TicksPerSec;
            _postsPerTickRemainder = _consPerfConfig.PostsPerSec % _consPerfConfig.TicksPerSec;
            _genMsgsPerTick = _consPerfConfig.GenMsgsPerSec / _consPerfConfig.TicksPerSec;
            _genMsgsPerTickRemainder = _consPerfConfig.GenMsgsPerSec % _consPerfConfig.TicksPerSec;

            // connect to provider application
            InitializeOmmConsumer();
        }

        protected void InitializeOmmConsumer()
        {
            try
            {
                string consumerName = DEFAULT_PERF_CONSUMER_CONFIG_NAME;

                if (_consPerfConfig.ConsumerName == null || _consPerfConfig.ConsumerName.Equals(string.Empty))
                {
                    consumerName = DEFAULT_PERF_CONSUMER_CONFIG_NAME;
                }
                else
                {
                    consumerName = _consPerfConfig.ConsumerName;
                }

                _ommConfig = new OmmConsumerConfig().ConsumerName(consumerName);
                // A blank user name is an invalid input to OmmConsumerConfig.Username and will trigger an invalid usage exception.
                if (_consPerfConfig.Username!.Length != 0)
                    _ommConfig.UserName(_consPerfConfig.Username);

                if (_consPerfConfig.UseUserDispatch)
                    _ommConfig.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);
                else
                    _ommConfig.OperationModel(OmmConsumerConfig.OperationModelMode.API_DISPATCH);

                if (_consPerfConfig.EncryptionProtocol != EmaConfig.EncryptedTLSProtocolFlags.NONE)
                    _ommConfig.EncryptedProtocolFlags(_consPerfConfig.EncryptionProtocol);

                _consumer = new OmmConsumer(_ommConfig);
            }
            catch (Exception e)
            {
                Console.WriteLine("Exception found" + e);
                _consThreadInfo.Shutdown = true;
                return;
            }

            _srcDirHandler.ServiceName(_consPerfConfig.ServiceName);
            long directoryHandle = _consumer.RegisterClient(_srcDirHandler.GetRequest(), _srcDirHandler);
            
            if (directoryHandle == 0)
            {
                ShutdownConsumer("Unable to send source directory request out.");
                return;
            }

            while (!_srcDirHandler.IsRequestedServiceUp())
            {
                if (_consPerfConfig.UseUserDispatch)
                    _consumer.Dispatch();

                try
                {
                    Thread.Sleep(500);
                }
                catch (Exception e)
                {
                    Console.WriteLine($"Thread.Sleep failed: {e.Message}");
                    Environment.Exit(-1);
                }
            }

            _requestsSent = true;
        }

        /** Run the consumer thread. */
        public void Run()
        {
            // initialize the test data from configuration and xml files
            Initialize();

            int currentTicks = 0;
            long nextTickTime = (long)GetTime.GetMicroseconds() + _usecPerTick;

            if (_consPerfConfig.UseUserDispatch)
            {
                int dispatchTime;

                while (!_consThreadInfo.Shutdown)
                {
                    // read until no more to read and then write leftover from previous burst
                    dispatchTime = (int)(nextTickTime - GetTime.GetMicroseconds());
                    if (dispatchTime > 0)
                        _consumer.Dispatch(dispatchTime);
                    else
                        _consumer.Dispatch();

                    if (GetTime.GetMicroseconds() >= nextTickTime)
                    {
                        nextTickTime += _usecPerTick;

                        // only send bursts on tick boundary
                        if (_requestsSent)
                        {
                            // send item request and post bursts
                            if (!SendBursts(currentTicks))
                                Console.WriteLine("\nConsumerThread " + _consThreadInfo.ThreadId + " Sending item request failed.");
                        }

                        if (++currentTicks == _consPerfConfig.TicksPerSec)
                            currentTicks = 0;
                    }
                }   // end of run loop
            }
            else
            {
                long currentTime;

                while (!_consThreadInfo.Shutdown)
                {
                    currentTime = (long)GetTime.GetMicroseconds();
                    // read until no more to read and then write leftover from previous burst
                    if (currentTime >= nextTickTime)
                    {
                        nextTickTime += _usecPerTick;

                        // only send bursts on tick boundary
                        if (_requestsSent)
                        {
                            // send item request and post bursts
                            if (!SendBursts(currentTicks))
                                Console.WriteLine($"ConsumerThread {_consThreadInfo.ThreadId}: Sending item request failed.");
                        }

                        if (++currentTicks == _consPerfConfig.TicksPerSec)
                            currentTicks = 0;
                    }
                    else
                    {
                        try
                        {
                            long sleepTime = (nextTickTime - currentTime) / 1000000;
                            Thread.Sleep(sleepTime > 0 ? (int)sleepTime : 0);

                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.Message);
                        }
                    }
                }   // end of run loop
            }

            _consThreadInfo.ShutdownAck = true;
            Console.WriteLine("\nConsumerThread " + _consThreadInfo.ThreadId + " exiting...");
        }

        //sends a burst of item requests.
        private bool SendItemRequestBurst(int itemBurstCount)
        {
            ItemRequest itemRequest;

            for (int i = 0; i < itemBurstCount; ++i)
            {
                if (_requestListIndex == _requestListSize)
                    return true;

                itemRequest = _itemRequestList[_requestListIndex];

                _requestMsg.Clear();

                if (_consPerfConfig.RequestSnapshots || (itemRequest.ItemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) == 0)
                    _requestMsg.InterestAfterRefresh(false);
                else
                    _requestMsg.InterestAfterRefresh(true);

                _requestMsg.DomainType(itemRequest.ItemInfo.Attributes.DomainType);
                _requestMsg.Name(itemRequest.ItemName!);
                if (_consPerfConfig.UseServiceId)
                    _requestMsg.ServiceId(_srcDirHandler.ServiceId());
                else
                    _requestMsg.ServiceName(_consPerfConfig.ServiceName!);

                _itemHandle = _consumer.RegisterClient(_requestMsg, this, itemRequest.ItemInfo);
                if (_itemHandle == 0)
                {
                    Console.WriteLine($"Sending request for item {itemRequest.ItemName} failed.");
                    return false;
                }

                //request has been made.
                itemRequest.RequestState = (int)ItemRequestState.WAITING_FOR_REFRESH;

                _requestListIndex++;
                _consThreadInfo.Stats!.RequestCount.Increment();
            }

            return true;
        }

        //sends a burst of post messages. 
        protected bool SendPostBurst(int itemBurstCount)
        {
            long encodeStartTime;
            int latencyUpdateNumber;

            latencyUpdateNumber = (_consPerfConfig.LatencyPostsPerSec > 0) ?
                    _postLatencyRandomArray.Next() : -1;

            for (int i = 0; i < itemBurstCount; i++)
            {
                ItemRequest postItem = NextPostItem();

                if ((postItem.ItemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) != 0)
                {
                    if (latencyUpdateNumber == i)
                        encodeStartTime = (long)GetTime.GetMicroseconds();
                    else
                        encodeStartTime = 0;

                    _postMsg.Clear();
                    _postMsg.Name(postItem.ItemName!);

                    _itemEncoder.PopulatePostMsg(_postMsg, postItem.ItemInfo, _postUserInfo, encodeStartTime);

                    try
                    {
                        if (postItem.ItemInfo.ClientHandle >= 0)
                        {
                            _consumer.Submit(_postMsg, postItem.ItemInfo.ClientHandle);
                            _postMsgItemIndex++;
                            _consThreadInfo.Stats!.PostSentCount.Increment();
                        }
                    }
                    catch (OmmException e)
                    {
                        Console.WriteLine(e.Message);
                    }
                }
            }
            return true;
        }

        // sends a burst of generic messages.
        protected bool SendGenMsgBurst(int itemBurstCount)
        {
            ItemRequest itemRequest;

            long encodeStartTime;
            int latencyGenMsgNumber;

            latencyGenMsgNumber = (_consPerfConfig.LatencyGenMsgsPerSec > 0) ? _genMsgLatencyRandomArray.Next() : -1;

            for (int i = 0; i < itemBurstCount; ++i)
            {
                if (_genMsgItemIndex == _requestListSize)
                    return true;

                itemRequest = NextGenMsgItem();
                if ((itemRequest.ItemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) != 0)
                {
                    if (latencyGenMsgNumber == i)
                    {
                        _consThreadInfo.Stats!.LatencyGenMsgSentCount.Increment();
                        encodeStartTime = (long)GetTime.GetMicroseconds();
                    }
                    else
                        encodeStartTime = 0;

                    _genMsg.Clear();
                    _genMsg.Name(itemRequest.ItemName!);

                    _itemEncoder.PopulateGenericMsg(_genMsg, itemRequest.ItemInfo, encodeStartTime);

                    try
                    {
                        if (itemRequest.ItemInfo.ClientHandle >= 0)
                        {
                            _consumer.Submit(_genMsg, itemRequest.ItemInfo.ClientHandle);
                            _genMsgItemIndex++;
                            _consThreadInfo.Stats!.GenMsgSentCount.Increment();
                        }
                    }
                    catch (OmmException e)
                    {
                        Console.WriteLine(e.Message);
                    }
                }
            }

            return true;
        }

        private ItemRequest NextGenMsgItem()
        {
            ItemRequest? itemRequest = null;

            do
            {
                if (_genMsgItemIndex == _requestListSize)
                    _genMsgItemIndex = 0;

                if ((_itemRequestList[_genMsgItemIndex].ItemInfo.ItemFlags & (int)ItemFlags.IS_GEN_MSG) > 0)
                {
                    itemRequest = _itemRequestList[_genMsgItemIndex];
                }

                _genMsgItemIndex++;
            } while (itemRequest == null);

            return itemRequest;
        }

        private ItemRequest NextPostItem()
        {
            ItemRequest? itemRequest = null;

            do
            {
                if (_postMsgItemIndex == _requestListSize)
                    _postMsgItemIndex = 0;

                if ((_itemRequestList[_postMsgItemIndex].ItemInfo.ItemFlags & (int)ItemFlags.IS_POST) > 0)
                {
                    itemRequest = _itemRequestList[_postMsgItemIndex];
                }

                _postMsgItemIndex++;
            } while (itemRequest == null);

            return itemRequest;
        }

        //print estimated post message sizes.
        protected void PrintEstimatedPostMsgSizes()
        {
            //TODO
        }

        // print estimated generic message sizes.
        protected void PrintEstimatedGenMsgSizes()
        {
            //TODO
        }

        // populates item information from the XML list.
        private void PopulateItemInfo()
        {
            int itemListIndex = 0;
            _postItemCount = 0;
            _genMsgItemCount = 0;

            for (int itemId = 0; itemId < _requestListSize; ++itemId)
            {
                /* Once we have filled our list with the common items,
                 * start using the range of items unique to this consumer thread. */
                if (itemListIndex == _consPerfConfig.CommonItemCount && itemListIndex < _consThreadInfo.ItemListUniqueIndex)
                    itemListIndex = _consThreadInfo.ItemListUniqueIndex;

                _itemRequestList[itemId].Clear();
                _itemRequestList[itemId].ItemInfo.Attributes.DomainType = _itemInfoList.ItemInfoList[itemListIndex].DomainType;

                _itemRequestList[itemId].ItemName = _itemInfoList.ItemInfoList[itemListIndex].Name!;

                if (!_itemInfoList.ItemInfoList[itemListIndex].IsSnapshot)
                {
                    int flags = _itemRequestList[itemId].ItemInfo.ItemFlags | (int)ItemFlags.IS_STREAMING_REQ;
                    _itemRequestList[itemId].ItemInfo.ItemFlags = flags;
                }

                if (_itemInfoList.ItemInfoList[itemListIndex].IsPost && _consPerfConfig.PostsPerSec > 0)
                {
                    MarketPriceItem? itemData;

                    ++_postItemCount;

                    int flags = _itemRequestList[itemId].ItemInfo.ItemFlags | (int)ItemFlags.IS_POST;
                    _itemRequestList[itemId].ItemInfo.ItemFlags = flags;

                    switch (_itemRequestList[itemId].ItemInfo.Attributes.DomainType)
                    {
                        case (int)Eta.Rdm.DomainType.MARKET_PRICE:
                            itemData = new MarketPriceItem();
                            _haveMarketPricePostItems = true;
                            break;
                        default:
                            itemData = null;
                            break;
                    }

                    if (itemData == null)
                    {
                        Console.WriteLine("Failed to get storage for ItemInfo data.");
                        Environment.Exit(-1);
                    }

                    _itemRequestList[itemId].ItemInfo.MarketPriceItem = itemData;
                }

                if (_consPerfConfig.PostsPerSec > 0)
                {
                    if (_haveMarketPricePostItems && _itemInfoList.PostMsgItemCount == 0)
                    {
                        Console.WriteLine($"Error: No MarketPrice posting data in file: {_consPerfConfig.MsgFilename}");
                        Environment.Exit(-1);
                    }
                }

                if (_itemInfoList.ItemInfoList[itemListIndex].IsGenMsg && _consPerfConfig.GenMsgsPerSec > 0)
                {
                    MarketPriceItem? itemData;

                    ++_genMsgItemCount;

                    int flags = _itemRequestList[itemId].ItemInfo.ItemFlags | (int)ItemFlags.IS_GEN_MSG;
                    _itemRequestList[itemId].ItemInfo.ItemFlags = flags;

                    switch (_itemRequestList[itemId].ItemInfo.Attributes.DomainType)
                    {
                        case (int)DomainType.MARKET_PRICE:
                            itemData = new MarketPriceItem();
                            _haveMarketPriceGenMsgItems = true;
                            break;
                        default:
                            itemData = null;
                            break;
                    }

                    if (itemData == null)
                    {
                        Console.WriteLine("\nFailed to get storage for ItemInfo data.\n");
                        Environment.Exit(-1);
                    }

                    _itemRequestList[itemId].ItemInfo.MarketPriceItem = itemData;
                }

                if (_consPerfConfig.GenMsgsPerSec > 0)
                {
                    if (_haveMarketPriceGenMsgItems && _itemInfoList.GenMsgItemCount == 0)
                    {
                        Console.WriteLine("Error: No MarketPrice generic msg data in file: %s\n", _consPerfConfig.MsgFilename);
                        Environment.Exit(-1);
                    }
                }

                ++itemListIndex;
            }
        }

        // send item requests, post bursts and or generic msg bursts.
        protected bool SendBursts(int currentTicks)
        {
            //send item requests until all sent
            if (_consThreadInfo.Stats!.RequestCount.GetTotal() < _requestListSize)
            {
                int requestBurstCount;

                requestBurstCount = _consPerfConfig.RequestsPerTick;
                if (currentTicks > _consPerfConfig.RequestsPerTickRemainder)
                    ++requestBurstCount;

                if (_consThreadInfo.Stats.ImageRetrievalStartTime == 0)
                {
                    _consThreadInfo.Stats.ImageRetrievalStartTime = (long)GetTime.GetMicroseconds();
                }

                return SendItemRequestBurst(requestBurstCount);
            }

            // send bursts of posts and or generic msgs
            if (_consThreadInfo.Stats.ImageRetrievalEndTime > 0)
            {
                if (_consPerfConfig.PostsPerSec > 0 && _postItemCount > 0)
                {
                    if (!SendPostBurst((int)(_postsPerTick + ((currentTicks < _postsPerTickRemainder) ? 1 : 0))))
                        return false;
                }

                if (_consPerfConfig.GenMsgsPerSec > 0 && _genMsgItemCount > 0)
                {
                    if (!SendGenMsgBurst((int)(_genMsgsPerTick + ((currentTicks < _genMsgsPerTickRemainder) ? 1 : 0))))
                        return false;
                }

            }

            return true;
        }

        protected void ShutdownConsumer(string text)
        {
            Console.WriteLine(text);
            //_shutdownCallback.Shutdown();
            _consThreadInfo.ShutdownAck = true;
            _consumer.Uninitialize();
            return;
        }


        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
        {
            _consThreadInfo.Stats!.RefreshCount.Increment();

            //If we are still retrieving images, check if this item still needs one.
            if (!_marketPriceDecoder.DecodeResponse(refreshMsg, refreshMsg.Payload().FieldList(), _consThreadInfo, _consPerfConfig.DowncastDecoding))
            {
                ShutdownConsumer("Decoding failure");
                return;
            }
            if (_consThreadInfo.Stats.ImageRetrievalEndTime == 0)
            {
                if (refreshMsg.State().StreamState == OmmState.StreamStates.CLOSED
                        || refreshMsg.State().StreamState == OmmState.StreamStates.CLOSED_RECOVER
                        || refreshMsg.State().StreamState == OmmState.StreamStates.CLOSED_REDIRECTED)
                {
                    ShutdownConsumer("Received unexpected final state in refresh for item: " + refreshMsg.State().ToString() +
                            refreshMsg.Name().ToString());
                    return;
                }

                if (refreshMsg.Complete())
                {
                    ((ItemInfo)consumerEvent.Closure!).ClientHandle = consumerEvent.Handle;
                    if (refreshMsg.State().DataState == OmmState.DataStates.OK)
                    {
                        _itemRequestList[((ItemInfo)consumerEvent.Closure).ItemId].RequestState = (int)ItemRequestState.HAS_REFRESH;
                        _consThreadInfo.Stats.RefreshCompleteCount.Increment();
                        if (_consThreadInfo.Stats.RefreshCompleteCount.GetTotal() == _requestListSize)
                        {
                            _consThreadInfo.Stats.ImageRetrievalEndTime = (long)GetTime.GetMicroseconds();
                            _consThreadInfo.Stats.SteadyStateLatencyTime = _consThreadInfo.Stats.ImageRetrievalEndTime + _consPerfConfig.DelaySteadyStateCalc * 1000000L;
                        }
                    }
                }
            }
        }


        public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
        {
            if (_consThreadInfo.Stats!.ImageRetrievalEndTime > 0)
            {
                _consThreadInfo.Stats.SteadyStateUpdateCount.Increment();
            }
            else
            {
                _consThreadInfo.Stats.StartupUpdateCount.Increment();
            }
            if (_consThreadInfo.Stats.FirstUpdateTime == 0)
                _consThreadInfo.Stats.FirstUpdateTime = (long)GetTime.GetMicroseconds();
            if (!_marketPriceDecoder.DecodeResponse(updateMsg, updateMsg.Payload().FieldList(), _consThreadInfo, _consPerfConfig.DowncastDecoding))
            {
                ShutdownConsumer("Decoding failure");
                return;
            }
        }


        public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
        {
            _consThreadInfo.Stats!.StatusCount.Increment();

            if (statusMsg.State().DataState == OmmState.DataStates.SUSPECT)
            {
                ((ItemInfo)consumerEvent.Closure!).ClientHandle = -1;
            }
            if (statusMsg.HasState && (statusMsg.State().StreamState == OmmState.StreamStates.CLOSED
                    || statusMsg.State().StreamState == OmmState.StreamStates.CLOSED_RECOVER
                    || statusMsg.State().StreamState == OmmState.StreamStates.CLOSED_REDIRECTED))
            {
                ShutdownConsumer("Received unexpected final state for item: " + statusMsg.State().ToString() +
                        _itemRequestList[((ItemInfo)consumerEvent.Closure!).ItemId].ItemName);
                return;
            }
        }


        public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent)
        {
            if (!_marketPriceDecoder.DecodeResponse(genericMsg, genericMsg.Payload().FieldList(), _consThreadInfo, _consPerfConfig.DowncastDecoding))
            {
                ShutdownConsumer("Decoding failure");
                return;
            }
        }


        public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent) { }

        public void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent) { }
    }
}
