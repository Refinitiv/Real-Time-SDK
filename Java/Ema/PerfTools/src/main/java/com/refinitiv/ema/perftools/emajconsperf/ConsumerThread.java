/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajconsperf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.access.OmmConsumerConfig.OperationModel;
import com.refinitiv.ema.perftools.common.*;
import com.refinitiv.eta.rdm.DomainTypes;

/** Provides the logic that consumer connections use in emajConsPerf for
  * connecting to a provider, requesting items, and processing the received
  * refreshes and updates.
  */
public class ConsumerThread implements Runnable, OmmConsumerClient
{
    private static final int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
    private static final String DEFAULT_PERF_CONSUMER_CONFIG_NAME = "Perf_Consumer_1";
    private static final String DEFAULT_PERF_CONSUMER_NAME_WSJSON = "Perf_Consumer_WSJSON_1";
    private static final String DEFAULT_PERF_CONSUMER_NAME_WSRWF = "Perf_Consumer_WSRWF_1";

    protected ConsumerThreadInfo _consThreadInfo; /* thread information */
    protected ConsPerfConfig _consPerfConfig; /* configuration information */
    private MarketPriceDecoder _marketPriceDecoder; /* market price decoder */
    protected Error _error; /* error structure */
    private DirectoryHandler _srcDirHandler; /* source directory handler */
    private XmlItemInfoList _itemInfoList; /* item information list from XML file */
	private XmlMsgData _xmlMsgData;
	private ItemEncoder _itemEncoder;
	private ItemRequest[] _itemRequestList; /* item request list */
    private int _postItemCount; /* number of items in _itemRequestList that are posting items */
    private int _genMsgItemCount; /* number of items in _itemRequestList that are for sending generic msgs on items */
    private ReqMsg _requestMsg; /* request message */
	private GenericMsg _genMsg; /* request message */
	private PostMsg _postMsg;
    protected PostUserInfo _postUserInfo; /* post user information */
    protected boolean _requestsSent; /* indicates if requested service is up */
    protected long _nsecPerTick; /* nanoseconds per tick */
    private int _requestListSize; /* request list size */
    private int _requestListIndex; /* current request list index */
    protected ShutdownCallback _shutdownCallback; /* shutdown callback to main application */
    private long _postsPerTick; /* posts per tick */
    private long _postsPerTickRemainder; /* posts per tick remainder */
    private long _genMsgsPerTick; /* gen msgs per tick */
    private long _genMsgsPerTickRemainder; /* gen msgs per tick remainder */
    private LatencyRandomArrayOptions _randomArrayOpts; /* random array options */
    protected LatencyRandomArray _postLatencyRandomArray; /* post random latency array */
    protected LatencyRandomArray _genMsgLatencyRandomArray; /* generic msg random latency array */
    private int _JVMPrimingRefreshCount; /* used to determine when JVM priming is complete */
    private OmmConsumer _consumer;
    private OmmConsumerConfig _ommConfig;
    private long _itemHandle;
	private int _genMsgItemIndex;
	private boolean _haveMarketPricePostItems; /* indicates there are post items in the item list */
	private boolean _haveMarketPriceGenMsgItems; /* indicates there are generic msg items in the item list */
	private int _postMsgItemIndex;

	{
    	_srcDirHandler = new DirectoryHandler();
        _requestMsg = EmaFactory.createReqMsg();
		_genMsg = EmaFactory.createGenericMsg();
		_postMsg = EmaFactory.createPostMsg();
        _randomArrayOpts = new LatencyRandomArrayOptions();
        _postLatencyRandomArray = new LatencyRandomArray();
        _genMsgLatencyRandomArray = new LatencyRandomArray();
    }

	public ConsumerThread(ConsumerThreadInfo consInfo,
						  ConsPerfConfig consConfig,
						  XmlItemInfoList itemList,
						  XmlMsgData xmlMsgData,
						  PostUserInfo postUserInfo,
						  ShutdownCallback shutdownCallback)
	{
		_consThreadInfo = consInfo;
		_consPerfConfig = consConfig;
		_shutdownCallback = shutdownCallback;
		_requestListSize = _consThreadInfo.itemListCount();
        _itemInfoList = itemList;
        _xmlMsgData = xmlMsgData;
		_itemRequestList = new ItemRequest[_requestListSize];
		_postUserInfo = postUserInfo;
    	for(int i = 0; i < _requestListSize; ++i)
    	{
			_itemRequestList[i] = new ItemRequest();
		}
		_requestListIndex = 0;
		_marketPriceDecoder = new MarketPriceDecoder(_postUserInfo);
		_itemEncoder = new ItemEncoder(_xmlMsgData);
	}

	/* Initializes consumer thread. */
	protected void initialize()
	{
		// create latency log file writer for this thread 
		if (_consPerfConfig.logLatencyToFile())
		{
			// Open latency log file. 
			_consThreadInfo.latencyLogFile(new File(_consPerfConfig.latencyLogFilename() + _consThreadInfo.threadId() + ".csv"));
			try
			{
				_consThreadInfo.latencyLogFileWriter(new PrintWriter(_consThreadInfo.latencyLogFile()));
			}
			catch (FileNotFoundException e)
			{
				System.out.printf("Error: Failed to open latency log file '%s'.\n", _consThreadInfo.latencyLogFile().getName());
				System.exit(-1);
			}
			_consThreadInfo.latencyLogFileWriter().println("Message type, Send Time, Receive Time, Latency (usec)\n");
		}

		// create stats file writer for this thread 
		_consThreadInfo.statsFile(new File(_consPerfConfig.statsFilename() + _consThreadInfo.threadId() + ".csv"));
		try
		{
			_consThreadInfo.statsFileWriter(new PrintWriter(_consThreadInfo.statsFile()));
		}
		catch (FileNotFoundException e)
		{
			System.out.printf("Error: Failed to open stats file '%s'.\n", _consThreadInfo.statsFile().getName());
			System.exit(-1);
		}
		_consThreadInfo.statsFileWriter().println("UTC, Latency updates, Latency avg (usec), Latency std dev (usec), Latency max (usec), Latency min (usec), Images, Update rate, Posting Latency updates, Posting Latency avg (usec), Posting Latency std dev (usec), Posting Latency max (usec), Posting Latency min (usec), GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%), Memory(MB)");
		
		// Create latency random array for post messages. Latency random array is used
		// to randomly insert latency RIC fields into post messages while sending bursts. 
		if (_consPerfConfig.latencyPostsPerSec() > 0)
		{
			_randomArrayOpts.totalMsgsPerSec(_consPerfConfig.postsPerSec());
			_randomArrayOpts.latencyMsgsPerSec(_consPerfConfig.latencyPostsPerSec());
			_randomArrayOpts.ticksPerSec(_consPerfConfig.ticksPerSec());
			_randomArrayOpts.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);

			if (_postLatencyRandomArray.create(_randomArrayOpts) != PerfToolsReturnCodes.SUCCESS) 
			{
				System.out.print("Error: Incorrect Post LatencyRandomArrayOptions.\n");
				System.exit(-1);
			}
		}

		// Create latency random array for generic messages. Latency random array is used
		// to randomly insert latency RIC fields into generic messages while sending bursts.
		if (_consPerfConfig.latencyGenMsgsPerSec() > 0) 
		{
			_randomArrayOpts.totalMsgsPerSec(_consPerfConfig.genMsgsPerSec());
			_randomArrayOpts.latencyMsgsPerSec(_consPerfConfig.latencyGenMsgsPerSec());
			_randomArrayOpts.ticksPerSec(_consPerfConfig.ticksPerSec());
			_randomArrayOpts.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);

			if (_genMsgLatencyRandomArray.create(_randomArrayOpts) != PerfToolsReturnCodes.SUCCESS) 
			{
				System.out.print("Error: Incorrect Generic Msg LatencyRandomArrayOptions.\n");
				System.exit(-1);
			}
		}

    	// populate item information from the XML list. 
    	populateItemInfo();
    			
		// initialize time tracking parameters 
		_nsecPerTick = 1000000000 / _consPerfConfig.ticksPerSec();
		_postsPerTick = _consPerfConfig.postsPerSec() / _consPerfConfig.ticksPerSec();
		_postsPerTickRemainder = _consPerfConfig.postsPerSec() % _consPerfConfig.ticksPerSec();
		_genMsgsPerTick = _consPerfConfig.genMsgsPerSec() / _consPerfConfig.ticksPerSec();
		_genMsgsPerTickRemainder = _consPerfConfig.genMsgsPerSec() % _consPerfConfig.ticksPerSec();

		// connect to provider application
		initializeOmmConsumer();
	}
	
	protected void initializeOmmConsumer()
	{
		try
		{
			String consumerName = DEFAULT_PERF_CONSUMER_CONFIG_NAME;
			
			if(_consPerfConfig.consumerName().isEmpty())
			{
				switch(_consPerfConfig.webSocketSubProtocol())
				{
				case ConsPerfConfig.WebSocketSubProtocol.RSSL_JSON_V2:
					consumerName = DEFAULT_PERF_CONSUMER_NAME_WSJSON;
					break;
				case ConsPerfConfig.WebSocketSubProtocol.RSSL_RWF:
					consumerName = DEFAULT_PERF_CONSUMER_NAME_WSRWF;
					break;
				}
			}
			else
			{
				consumerName = _consPerfConfig.consumerName();
			}
			
			_ommConfig = EmaFactory.createOmmConsumerConfig().consumerName(consumerName);
			// A blank user name is an invalid input to OmmConsumerConfig.username and will trigger an invalid usage exception.
			if(_consPerfConfig.username().length() != 0)
				_ommConfig.username(_consPerfConfig.username());
			
			if(_consPerfConfig.useUserDispatch())
				_ommConfig.operationModel(OperationModel.USER_DISPATCH);
			else
				_ommConfig.operationModel(OperationModel.API_DISPATCH);
			
			_ommConfig.tunnelingKeyStoreFile(_consPerfConfig.keyfile());
			_ommConfig.tunnelingKeyStorePasswd(_consPerfConfig.keypasswd());

			// Set security protocol versions of TLS based on configured values, with default having TLS 1.2 and 1.3 enabled
	        	_ommConfig.tunnelingSecurityProtocol(_consPerfConfig.securityProtocol());
	        	_ommConfig.tunnelingSecurityProtocolVersions(_consPerfConfig.securityProtocolVersions());
				
			_consumer = EmaFactory.createOmmConsumer(_ommConfig);
		}
		catch(Exception e)
		{
			System.out.println("Exception found"+e);
			_consThreadInfo.shutdown(true);
			return;
		}
		
		_srcDirHandler.serviceName(_consPerfConfig.serviceName());
		long directoryHandle = _consumer.registerClient(_srcDirHandler.getRequest(), _srcDirHandler);
		if (directoryHandle == 0)
		{
			shutdownConsumer("Unable to send source directory request out.");
    		return;
		}
		
		while (!_srcDirHandler.isRequestedServiceUp())
	    {
		  if (_consPerfConfig.useUserDispatch())
			  _consumer.dispatch(); 
		  
	    	try
	    	{
	    		Thread.sleep(500);
	    	}
	    	catch(Exception e)
	    	{
	    		System.out.println("Thread.sleep failed ");
	    		System.exit(-1);
	    	}
	    }
	    
	    _requestsSent = true;
	}
	
	/** Run the consumer thread. */
	public void run()
	{
		// initialize the test data from configuration and xml files
		initialize();

	    int currentTicks = 0;
	    long nextTickTime = System.nanoTime() + _nsecPerTick;
	    
	    if (_consPerfConfig.useUserDispatch())
	    {
	    	long dispatchTime;
	    	
	    	while (!_consThreadInfo.shutdown())
	        {
	            // read until no more to read and then write leftover from previous burst
	    		dispatchTime =  (nextTickTime - System.nanoTime())/1000;
	            if (dispatchTime > 0)
	            	_consumer.dispatch(dispatchTime);
	            else
	            	_consumer.dispatch(); 
	            
	            if (System.nanoTime() >= nextTickTime)
	            {
	                nextTickTime += _nsecPerTick;

	                // only send bursts on tick boundary
	                if (_requestsSent)
	                {
	                    // send item request and post bursts
	                    if (!sendBursts(currentTicks))
	                    	   System.out.println("\nConsumerThread " + _consThreadInfo.threadId() + " Sending item request failed.");
	                }

	                if (++currentTicks == _consPerfConfig.ticksPerSec())
	                    currentTicks = 0;
	            }
	        }   // end of run loop
	    }
	    else
	    {
	    	long currentTime;
	    	
	    	while (!_consThreadInfo.shutdown())
	        {
	    		currentTime = System.nanoTime();
	            // read until no more to read and then write leftover from previous burst
	            if (currentTime >= nextTickTime)
	            {
	                nextTickTime += _nsecPerTick;

	                // only send bursts on tick boundary
	                if (_requestsSent)
	                {
	                    // send item request and post bursts
	                    if (!sendBursts(currentTicks))
	                    	   System.out.println("\nConsumerThread " + _consThreadInfo.threadId() + " Sending item request failed.");
	                }

	                if (++currentTicks == _consPerfConfig.ticksPerSec())
	                    currentTicks = 0;
	            }
	            else
	            {
						try
						{
							long sleepTime = (nextTickTime - currentTime)/1000000;
							Thread.sleep( (sleepTime > 0 ? sleepTime : 10));
							 
						} catch (InterruptedException e)
						{
							e.printStackTrace();
						} 
	            }
	        }   // end of run loop
	    }
	        
	    _consThreadInfo.shutdownAck(true);
	    System.out.println("\nConsumerThread " + _consThreadInfo.threadId() + " exiting...");
	}

    //sends a burst of item requests.
    private boolean sendItemRequestBurst(int itemBurstCount)
    {
    	ItemRequest itemRequest;

    	for(int i = 0; i < itemBurstCount; ++i)
    	{
	    		if (_requestListIndex == _requestListSize)
	    			return true;
	
	    		itemRequest = _itemRequestList[_requestListIndex];
	
	    		_requestMsg.clear();
	    		
	    		//don't apply streaming for JVM priming and snapshot
	    		if (_consPerfConfig.requestSnapshots() || (itemRequest.itemInfo().itemFlags() & ItemFlags.IS_STREAMING_REQ) == 0 || 
	    			 (_consPerfConfig.primeJVM() && _JVMPrimingRefreshCount < _requestListSize))
	    			_requestMsg.interestAfterRefresh(false);
	    		else
	    			_requestMsg.interestAfterRefresh(true);
	
	    		_requestMsg.domainType(itemRequest.itemInfo().attributes().domainType());
	    		_requestMsg.name(itemRequest.itemName());
	    		if (_consPerfConfig.useServiceId())
	    			_requestMsg.serviceId(_srcDirHandler.serviceId());
	    		else
	    			_requestMsg.serviceName(_consPerfConfig.serviceName());
	    		
	    		_itemHandle = _consumer.registerClient(_requestMsg, this,  itemRequest.itemInfo());
	            if (_itemHandle == 0)
	            {
	                System.out.printf("Sending item request failed: %d.\n");
	                return false;
	            }
	
	    		//request has been made.
	    		itemRequest.requestState(ItemRequestState.WAITING_FOR_REFRESH);
	
	    		_requestListIndex++;
	    		_consThreadInfo.stats().requestCount().increment();
	    	}

    	return true;
    }
    
    //sends a burst of post messages. 
	protected boolean sendPostBurst(int itemBurstCount)
	{
		long encodeStartTime;
		int latencyUpdateNumber;

		latencyUpdateNumber = (_consPerfConfig.latencyPostsPerSec() > 0) ?
				_postLatencyRandomArray.next() : -1;

		for (int i = 0; i < itemBurstCount; i++) {
			ItemRequest postItem = nextPostItem();

			if ((postItem.itemInfo().itemFlags() & ItemFlags.IS_STREAMING_REQ) != 0) {
				if (latencyUpdateNumber == i)
					encodeStartTime = System.nanoTime()/1000;
				else
					encodeStartTime = 0;

				_postMsg.clear();
				_postMsg.name(postItem.itemName());

				_itemEncoder.populatePostMsg(_postMsg, postItem.itemInfo(), _postUserInfo, encodeStartTime);

				try {
					if (postItem.itemInfo().clientHandle() >= 0) {
						_consumer.submit(_postMsg, postItem.itemInfo().clientHandle());
						_postMsgItemIndex++;
						_consThreadInfo.stats().postSentCount().increment();
					}
				} catch (OmmException e) {
					System.out.println(e.getMessage());
				}
			}
		}
		return true;
	}

	// sends a burst of generic messages.
	protected boolean sendGenMsgBurst(int itemBurstCount) 
	{
		ItemRequest itemRequest;

		long encodeStartTime;
		int latencyGenMsgNumber;

		latencyGenMsgNumber = (_consPerfConfig.latencyGenMsgsPerSec() > 0) ? _genMsgLatencyRandomArray.next() : -1;

		for(int i = 0; i < itemBurstCount; ++i)
		{
			if (_genMsgItemIndex == _requestListSize)
				return true;

			itemRequest = nextGenMsgItem();
			if ((itemRequest.itemInfo().itemFlags() & ItemFlags.IS_STREAMING_REQ) != 0) {
				if (latencyGenMsgNumber == i)
				{
					_consThreadInfo.stats().latencyGenMsgSentCount().increment();
					encodeStartTime = System.nanoTime() / 1000;
				}
				else
					encodeStartTime = 0;

				_genMsg.clear();
				_genMsg.name(itemRequest.itemName());

				_itemEncoder.populateGenericMsg(_genMsg, itemRequest.itemInfo(), encodeStartTime);

				try {
					if (itemRequest.itemInfo().clientHandle() >= 0) {
						_consumer.submit(_genMsg, itemRequest.itemInfo().clientHandle());
						_genMsgItemIndex++;
						_consThreadInfo.stats().genMsgSentCount().increment();
					}
				} catch (OmmException e) {
					System.out.println(e.getMessage());
				}
			}
		}

		return true;
	}

	private ItemRequest nextGenMsgItem() {
		ItemRequest itemRequest = null;

		do {
			if (_genMsgItemIndex == _requestListSize)
				_genMsgItemIndex = 0;

			if ((_itemRequestList[_genMsgItemIndex].itemInfo().itemFlags() & ItemFlags.IS_GEN_MSG) > 0) {
				itemRequest = _itemRequestList[_genMsgItemIndex];
			}

			_genMsgItemIndex++;
		} while (itemRequest == null);

		return itemRequest;
	}

	private ItemRequest nextPostItem() {
		ItemRequest itemRequest = null;

		do {
			if (_postMsgItemIndex == _requestListSize)
				_postMsgItemIndex = 0;

			if ((_itemRequestList[_postMsgItemIndex].itemInfo().itemFlags() & ItemFlags.IS_POST) > 0) {
				itemRequest = _itemRequestList[_postMsgItemIndex];
			}

			_postMsgItemIndex++;
		} while (itemRequest == null);

		return itemRequest;
	}

    //print estimated post message sizes.
    protected void printEstimatedPostMsgSizes()
    {
    	//TODO
	}

	// print estimated generic message sizes.
	protected void printEstimatedGenMsgSizes() 
	{
		//TODO
	}
    
    // populates item information from the XML list.
    private void populateItemInfo()
    {
		int itemListIndex = 0;
		_postItemCount = 0;
		_genMsgItemCount = 0;

    	for(int itemId = 0; itemId < _requestListSize; ++itemId)
    	{
    		/* Once we have filled our list with the common items,
    		 * start using the range of items unique to this consumer thread. */
			if (itemListIndex == _consPerfConfig.commonItemCount()
					&& itemListIndex < _consThreadInfo.itemListUniqueIndex())
				itemListIndex = _consThreadInfo.itemListUniqueIndex();

			_itemRequestList[itemId].clear();
			_itemRequestList[itemId].itemInfo().attributes().domainType(_itemInfoList.itemInfoList()[itemListIndex].domainType());

			_itemRequestList[itemId].itemName(_itemInfoList.itemInfoList()[itemListIndex].name());

    		if (!_itemInfoList.itemInfoList()[itemListIndex].isSnapshot())
    		{
    			int flags = _itemRequestList[itemId].itemInfo().itemFlags() | ItemFlags.IS_STREAMING_REQ;
    			_itemRequestList[itemId].itemInfo().itemFlags(flags);
    		}

    		if (_itemInfoList.itemInfoList()[itemListIndex].isPost() && _consPerfConfig.postsPerSec() > 0)
    		{
				MarketPriceItem itemData = null;

				++_postItemCount;

				int flags = _itemRequestList[itemId].itemInfo().itemFlags() | ItemFlags.IS_POST;
				_itemRequestList[itemId].itemInfo().itemFlags(flags);

				switch(_itemRequestList[itemId].itemInfo().attributes().domainType())
				{
					case DomainTypes.MARKET_PRICE:
						itemData = new MarketPriceItem();
						_haveMarketPricePostItems = true;
						break;
					default:
						itemData = null;
						break;
				}

				if (itemData == null)
				{
					System.out.printf("\nFailed to get storage for ItemInfo data.\n");
					System.exit(-1);
				}

				_itemRequestList[itemId].itemInfo().marketPriceItem(itemData);
			}

			if (_consPerfConfig.postsPerSec() > 0)
			{
				if (_haveMarketPricePostItems && _itemInfoList.postItemCount() == 0)
				{
					System.out.printf(
							"Error: No MarketPrice posting data in file: %s\n",
							_consPerfConfig.msgFilename());
					System.exit(-1);
				}
			}

			if (_itemInfoList.itemInfoList()[itemListIndex].isGenMsg() && _consPerfConfig.genMsgsPerSec() > 0) {
				MarketPriceItem itemData = null;

				++_genMsgItemCount;

				int flags = _itemRequestList[itemId].itemInfo().itemFlags() | ItemFlags.IS_GEN_MSG;
				_itemRequestList[itemId].itemInfo().itemFlags(flags);

				switch (_itemRequestList[itemId].itemInfo().attributes().domainType()) {
					case DomainTypes.MARKET_PRICE:
						itemData = new MarketPriceItem();
						_haveMarketPriceGenMsgItems = true;
						break;
					default:
						itemData = null;
						break;
				}

				if (itemData == null) {
					System.out.printf("\nFailed to get storage for ItemInfo data.\n");
					System.exit(-1);
				}

				_itemRequestList[itemId].itemInfo().marketPriceItem(itemData);
			}

			if (_consPerfConfig.genMsgsPerSec() > 0)
			{
				if (_haveMarketPriceGenMsgItems && _itemInfoList.genMsgItemCount() == 0)
				{
					System.out.printf("Error: No MarketPrice generic msg data in file: %s\n", _consPerfConfig.msgFilename());
					System.exit(-1);
				}
			}

			++itemListIndex;
		}
	}

	// send item requests, post bursts and or generic msg bursts.
	protected boolean sendBursts(int currentTicks) 
	{
    	//send item requests until all sent
    	if (_consThreadInfo.stats().requestCount().getTotal() < _requestListSize)
        {
			int requestBurstCount;

			requestBurstCount = _consPerfConfig.requestsPerTick();
			if (currentTicks > _consPerfConfig.requestsPerTickRemainder())
				++requestBurstCount;

			if (_consThreadInfo.stats().imageRetrievalStartTime() == 0)
			{
				if (!_consPerfConfig.primeJVM())
				{
					_consThreadInfo.stats().imageRetrievalStartTime(System.nanoTime());
				}
                else
                {
                    if (_JVMPrimingRefreshCount == _requestListSize)
                    {
                        _consThreadInfo.stats().imageRetrievalStartTime(System.nanoTime());
                    }
                }
			}

			return sendItemRequestBurst(requestBurstCount); 
		}

		// send bursts of posts and or generic msgs
		if (_consThreadInfo._stats.imageRetrievalEndTime() > 0)
		{
			if (_consPerfConfig.postsPerSec() > 0 && _postItemCount > 0) {
				if (!sendPostBurst((int) (_postsPerTick + ((currentTicks < _postsPerTickRemainder) ? 1 : 0))))
					return false;
			}

			if (_consPerfConfig.genMsgsPerSec() > 0 && _genMsgItemCount > 0)  {
				if (!sendGenMsgBurst((int) (_genMsgsPerTick + ((currentTicks < _genMsgsPerTickRemainder) ? 1 : 0))))
					return false;
			}

		}

		return true;
	}

    protected void shutdownConsumer(String text)
    {
		System.out.println(text);
		_shutdownCallback.shutdown();
		_consThreadInfo.shutdownAck(true);
    	_consumer.uninitialize();
    	return;    	
    }

	@Override
	public void onRefreshMsg(com.refinitiv.ema.access.RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent)
	{
    		_consThreadInfo._stats.refreshCount().increment();
    		
    		//If we are still retrieving images, check if this item still needs one.
			if(!_marketPriceDecoder.decodeResponse(refreshMsg, refreshMsg.payload().fieldList(), _consThreadInfo, _consPerfConfig.downcastDecoding()))
			{
				shutdownConsumer("Decoding failure");
        		return;
			}
    		if (_consThreadInfo._stats.imageRetrievalEndTime() == 0)
    		{
    			if (refreshMsg.state().streamState() == OmmState.StreamState.CLOSED 
    					|| refreshMsg.state().streamState() == OmmState.StreamState.CLOSED_RECOVER
    					|| refreshMsg.state().streamState() == OmmState.StreamState.CLOSED_REDIRECTED)
				{
    				shutdownConsumer("Received unexpected final state in refresh for item: " + refreshMsg.state().toString() +
							refreshMsg.name().toString());
	        		return;
				}

				if (refreshMsg.complete())
				{
    				if (!_consPerfConfig.primeJVM())
    				{
						((ItemInfo)consumerEvent.closure()).clientHandle(consumerEvent.handle());
    	                if (refreshMsg.state().dataState() == OmmState.DataState.OK)
    	                {
                            _itemRequestList[((ItemInfo)consumerEvent.closure()).itemId()].requestState(ItemRequestState.HAS_REFRESH);
                            _consThreadInfo.stats().refreshCompleteCount().increment();
                            if (_consThreadInfo._stats.refreshCompleteCount().getTotal() == _requestListSize)
                            {
                                _consThreadInfo._stats.imageRetrievalEndTime(System.nanoTime());
                                _consThreadInfo._stats.steadyStateLatencyTime(_consThreadInfo._stats.imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
                            }
    	                }
    				}
    				else // JVM priming enabled
    				{
    				    // Count snapshot images used for priming, ignoring state
                        if (_JVMPrimingRefreshCount < _requestListSize)
                        {
                            _JVMPrimingRefreshCount++;
                            // reset request state so items can be re-requested
                            _itemRequestList[((ItemInfo)consumerEvent.closure()).itemId()].requestState(ItemRequestState.NOT_REQUESTED);
                            if (_JVMPrimingRefreshCount == _requestListSize)
                            {
                                // reset request count and _requestListIndex so items can be re-requested
                            	//set the image retrieval start time
                                _consThreadInfo._stats.requestCount().init();
                                _consThreadInfo._stats.refreshCompleteCount().init();
                                _consThreadInfo._stats.refreshCount().init();
                                _requestListIndex = 0;
                            }
                        }
                        else // the streaming image responses after priming
                        {
							((ItemInfo)consumerEvent.closure()).clientHandle(consumerEvent.handle());
                            if (refreshMsg.state().dataState() == OmmState.DataState.OK)
                            {
                                _itemRequestList[((ItemInfo)consumerEvent.closure()).itemId()].requestState(ItemRequestState.HAS_REFRESH);
                                _consThreadInfo._stats.refreshCompleteCount().increment();
                                if (_consThreadInfo._stats.refreshCompleteCount().getTotal() == _requestListSize)
                                {
                                    _consThreadInfo._stats.imageRetrievalEndTime(System.nanoTime());
                                    _consThreadInfo._stats.steadyStateLatencyTime(_consThreadInfo._stats.imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
                                }
                            }
                        }
    				}
				}
    		}
	}

	@Override
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent)
	{
		if (_consThreadInfo._stats.imageRetrievalEndTime() > 0)
		{
			_consThreadInfo._stats.steadyStateUpdateCount().increment();
		}
		else
		{
			_consThreadInfo._stats.startupUpdateCount().increment();
		}
		if (_consThreadInfo._stats.firstUpdateTime() == 0)
			_consThreadInfo._stats.firstUpdateTime(System.nanoTime());
		if(!_marketPriceDecoder.decodeResponse(updateMsg, updateMsg.payload().fieldList(), _consThreadInfo, _consPerfConfig.downcastDecoding()))
		{
			shutdownConsumer("Decoding failure");
    		return;
		}
	}
	
	@Override
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent)
	{
		_consThreadInfo._stats.statusCount().increment();

		if (statusMsg.state().dataState() == OmmState.DataState.SUSPECT) {
			((ItemInfo)consumerEvent.closure()).clientHandle(-1);
		}
		if (statusMsg.hasState() && (statusMsg.state().streamState() == OmmState.StreamState.CLOSED 
				|| statusMsg.state().streamState() == OmmState.StreamState.CLOSED_RECOVER
				|| statusMsg.state().streamState() == OmmState.StreamState.CLOSED_REDIRECTED))
		{
			shutdownConsumer("Received unexpected final state for item: " + statusMsg.state().toString()+
        			_itemRequestList[((ItemInfo)consumerEvent.closure()).itemId()].itemName());
    		return;
		}
	}

	@Override
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {
		if(!_marketPriceDecoder.decodeResponse(genericMsg, genericMsg.payload().fieldList(), _consThreadInfo, _consPerfConfig.downcastDecoding()))
		{
			shutdownConsumer("Decoding failure");
			return;
		}
	}

	@Override
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {}
	@Override
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {}
}
