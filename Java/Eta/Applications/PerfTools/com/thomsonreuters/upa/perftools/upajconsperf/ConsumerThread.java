package com.thomsonreuters.upa.perftools.upajconsperf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.PostUserInfo;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.perftools.common.ConsumerLoginState;
import com.thomsonreuters.upa.perftools.common.PingHandler;
import com.thomsonreuters.upa.perftools.common.DictionaryHandler;
import com.thomsonreuters.upa.perftools.common.DirectoryHandler;
import com.thomsonreuters.upa.perftools.common.ItemEncoder;
import com.thomsonreuters.upa.perftools.common.ItemFlags;
import com.thomsonreuters.upa.perftools.common.ItemInfo;
import com.thomsonreuters.upa.perftools.common.LatencyRandomArray;
import com.thomsonreuters.upa.perftools.common.LatencyRandomArrayOptions;
import com.thomsonreuters.upa.perftools.common.LoginHandler;
import com.thomsonreuters.upa.perftools.common.MarketPriceItem;
import com.thomsonreuters.upa.perftools.common.PerfToolsReturnCodes;
import com.thomsonreuters.upa.perftools.common.ResponseCallback;
import com.thomsonreuters.upa.perftools.common.ShutdownCallback;
import com.thomsonreuters.upa.perftools.common.XmlItemInfoList;
import com.thomsonreuters.upa.perftools.common.XmlMsgData;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.IoctlCodes;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;

/** Provides the logic that consumer connections use in upajConsPerf for
  * connecting to a provider, requesting items, and processing the received
  * refreshes and updates.
  */
public class ConsumerThread implements Runnable, ResponseCallback
{
    private static final int CONNECTION_RETRY_TIME = 1; // seconds
    private static final int ITEM_STREAM_ID_START = 6;
    private static final int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
    
    public static final int MAX_MSG_SIZE = 1024;
    public static int TRANSPORT_BUFFER_SIZE_REQUEST = MAX_MSG_SIZE;
    public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

    protected ConsumerThreadInfo _consThreadInfo; /* thread information */
    protected ConsPerfConfig _consPerfConfig; /* configuration information */
    protected EncodeIterator _eIter; /* encode iterator */
    protected DecodeIterator _dIter; /* decode iterator */
    private WriteArgs _writeArgs = TransportFactory.createWriteArgs();
    private ReadArgs _readArgs = TransportFactory.createReadArgs();
    private Msg _responseMsg; /* response message */
    private LoginHandler _loginHandler; /* login handler */
    private DirectoryHandler _srcDirHandler; /* source directory handler */ 
    protected DictionaryHandler _dictionaryHandler; /* dictionary handler */
    private PingHandler _pingHandler; /* ping handler */
    private MarketPriceDecoder _marketPriceDecoder; /* market price decoder */
    private InProgInfo _inProg; /* connection in progress information */
    protected Error _error; /* error structure */
    private XmlItemInfoList _itemInfoList; /* item information list from XML file */
    private XmlMsgData _msgData; /* message data information from XML file */
    private ItemRequest[] _itemRequestList; /* item request list */
    private int _postItemCount; /* number of items in _itemRequestList that are posting items */
    private int _genMsgItemCount; /* number of items in _itemRequestList that are for sending generic msgs on items */
    private RequestMsg _requestMsg; /* request message */
    protected PostUserInfo _postUserInfo; /* post user information */
    protected boolean _requestsSent; /* indicates if requested service is up */
    protected long _nsecPerTick; /* nanoseconds per tick */
    private int _requestListSize; /* request list size */
    private int _requestListIndex; /* current request list index */
    protected ShutdownCallback _shutdownCallback; /* shutdown callback to main application */
    private ConnectOptions _connectOpts; /* connection options */
    private ChannelInfo _chnlInfo; /* channel information */
    private boolean _haveMarketPricePostItems; /* indicates there are post items in the item list */
    private long _postsPerTick; /* posts per tick */
    private long _postsPerTickRemainder; /* posts per tick remainder */
    private boolean _haveMarketPriceGenMsgItems; /* indicates there are generic msg items in the item list */
    private long _genMsgsPerTick; /* gen msgs per tick */
    private long _genMsgsPerTickRemainder; /* gen msgs per tick remainder */
    private LatencyRandomArrayOptions _randomArrayOpts; /* random array options */
    protected LatencyRandomArray _postLatencyRandomArray; /* post random latency array */
    private int _postItemIndex; /* current post item index */
    protected LatencyRandomArray _genMsgLatencyRandomArray; /* generic msg random latency array */
    private int _genMsgItemIndex; /* current generic msg item index */
    protected ItemEncoder _itemEncoder; /* item encoder */
    private MarketPriceItem _mpItem; /* market price item */
    private MsgKey _msgKey; /* message key */
    private ItemInfo _itemInfo; /* item information */
    private int _JVMPrimingRefreshCount; /* used to determine when JVM priming is complete */
    private Channel _channel;
    private Selector _selector;

    {
    	_eIter = CodecFactory.createEncodeIterator();
    	_dIter = CodecFactory.createDecodeIterator();
    	_responseMsg = CodecFactory.createMsg();
        _loginHandler = new LoginHandler();
        _srcDirHandler = new DirectoryHandler();
        _dictionaryHandler = new DictionaryHandler();
        _pingHandler = new PingHandler();
        _inProg = TransportFactory.createInProgInfo();
        _error = TransportFactory.createError();
        _connectOpts = TransportFactory.createConnectOptions();
        _chnlInfo = TransportFactory.createChannelInfo();
        _requestMsg = (RequestMsg)CodecFactory.createMsg();
        _randomArrayOpts = new LatencyRandomArrayOptions();
        _postLatencyRandomArray = new LatencyRandomArray();
        _genMsgLatencyRandomArray = new LatencyRandomArray();
        _mpItem = new MarketPriceItem();
        _msgKey = CodecFactory.createMsgKey();
        _itemInfo = new ItemInfo();
    }

	public ConsumerThread(ConsumerThreadInfo consInfo, ConsPerfConfig consConfig, XmlItemInfoList itemList, XmlMsgData msgData, PostUserInfo postUserInfo, ShutdownCallback shutdownCallback) 
	{
		_consThreadInfo = consInfo;
		_consPerfConfig = consConfig;
		_shutdownCallback = shutdownCallback;
		_requestListSize = _consThreadInfo.itemListCount() + ITEM_STREAM_ID_START;
        _itemInfoList = itemList;
		_itemRequestList = new ItemRequest[_requestListSize];
		_postUserInfo = postUserInfo;
    	for(int i = 0; i < _requestListSize; ++i)
    	{
			_itemRequestList[i] = new ItemRequest();
		}
		_requestListIndex = ITEM_STREAM_ID_START;
		_postItemIndex = ITEM_STREAM_ID_START;
		_genMsgItemIndex = ITEM_STREAM_ID_START;
		_msgData = msgData;
		_itemEncoder = new ItemEncoder(msgData);
		_marketPriceDecoder = new MarketPriceDecoder(_postUserInfo);
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

        // load dictionary
        _dictionaryHandler.loadDictionary();
        
        // connect to provider application
        connect();
	}
	
	/* Connect and wait until connection is active */
    protected void connect()
    {
        // Initialize Transport
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(_consPerfConfig.threadCount() > 1 ? true : false);
        if (Transport.initialize(initArgs, _error) != TransportReturnCodes.SUCCESS)
        {
    		System.out.println("Transport.initialize failed ");
    		System.exit(-1);
        }
        
		/* set connect options  */
        _connectOpts.majorVersion(Codec.majorVersion());
        _connectOpts.minorVersion(Codec.minorVersion());        
		_connectOpts.connectionType(_consPerfConfig.connectionType());
		_connectOpts.guaranteedOutputBuffers(_consPerfConfig.guaranteedOutputBuffers());
		_connectOpts.numInputBuffers(_consPerfConfig.numInputBuffers());
		if (_consPerfConfig.sendBufSize() > 0)
		{
			_connectOpts.sysSendBufSize(_consPerfConfig.sendBufSize());
		}
		if (_consPerfConfig.recvBufSize() > 0)
		{
			_connectOpts.sysRecvBufSize(_consPerfConfig.recvBufSize());
		}
		if(_consPerfConfig.connectionType() == ConnectionTypes.SOCKET)
		{
			_connectOpts.tcpOpts().tcpNoDelay(_consPerfConfig.tcpNoDelay());
		}
        // set the connection parameters on the connect options 
		_connectOpts.unifiedNetworkInfo().address(_consPerfConfig.hostName());
		_connectOpts.unifiedNetworkInfo().serviceName(_consPerfConfig.portNo());
		_connectOpts.unifiedNetworkInfo().interfaceName(_consPerfConfig.interfaceName());
		
    	// Connection recovery loop. It will try to connect until successful
        System.out.println("Starting connection...");
        int handshake;
        while (!_consThreadInfo.shutdown())
        {
            _channel = Transport.connect(_connectOpts, _error);
            if (_channel == null)
            {
                System.err.println("Error: Transport connect failure: " + _error.text());
                System.exit(-1);
            }
            
            while ((handshake = _channel.init(_inProg, _error)) != TransportReturnCodes.SUCCESS)
            {
            	if (handshake == TransportReturnCodes.FAILURE)
            		break;
            	
            	try
            	{
            		Thread.sleep(1000);
            	}
            	catch(Exception e)
            	{
            		System.out.println("Thread.sleep failed ");
            		System.exit(-1);
            	}
            }
            if (handshake == TransportReturnCodes.SUCCESS)
            	break;
            
            System.out.println("Connection failure: " + _error.text() + ". Will retry shortly.");
            try
        	{
        		Thread.sleep(CONNECTION_RETRY_TIME * 1000);
        	}
        	catch(Exception e)
        	{
        		System.out.println("Thread.sleep failed ");
        		System.exit(-1);
        	}
        }        
        if (!_consThreadInfo.shutdown())
        {
    	_consThreadInfo.channel(_channel);
        System.out.println("Connected ");

        // set the high water mark if configured
    	if (_consPerfConfig.highWaterMark() > 0)
    	{
    		if (_channel.ioctl(IoctlCodes.HIGH_WATER_MARK, _consPerfConfig.highWaterMark(), _error) != TransportReturnCodes.SUCCESS)
    		{
    			closeChannelAndShutDown("Channel.ioctl() failed");
            	return;
    		}
    	}

    	try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
    		System.out.println("selector open ");
    		System.exit(-1);
        }

    	// register selector for read
    	addOption(SelectionKey.OP_READ, _channel);
    }
	}

	/** Run the consumer thread. */
	public void run()
	{
		// initialize the test data from configuration and xml files
		initialize();

        if (!_consThreadInfo.shutdown())
        {
            // Check if the test is configured for the correct buffer size to fit post messages
            printEstimatedPostMsgSizes(_channel);

            // Check if the test is configured for the correct buffer size to fit generic messages
            printEstimatedGenMsgSizes(_channel);

            // set service name in directory handler
            _srcDirHandler.serviceName(_consPerfConfig.serviceName());

            // get and print the channel info
            if (_channel.info(_chnlInfo, _error) != TransportReturnCodes.SUCCESS)
            {
                closeChannelAndShutDown("Channel.info() failed");
                return;
            } 
            System.out.printf("Channel active. " + _chnlInfo.toString() + "\n");
            
            // set login parameters
            _loginHandler.applicationName("upajConsPerf");
            _loginHandler.userName(_consPerfConfig.username());
            _loginHandler.role(Login.RoleTypes.CONS);
    
            // Send login request message
            TransportBuffer msg= _loginHandler.getRequestMsg(_channel, _error, _eIter);
            if (msg != null)
            {
                write(msg);
            }
            else
            {
                closeChannelAndShutDown("Sending login request failed");
                return;
            }
    
            /* Initialize ping handler */
            _pingHandler.initPingHandler(_channel.pingTimeout());
        }

        int ret = 0;
        int currentTicks = 0;
        long nextTickTime = System.nanoTime() + _nsecPerTick;
        long selectTime;

        while (!_consThreadInfo.shutdown())
        {

            // read until no more to read and then write leftover from previous burst
        	selectTime = nextTickTime-System.nanoTime(); 
        	selectorReadAndWrite(selectTime);
        	
            /* Handle pings */
            if (_pingHandler.handlePings(_channel, _error) != CodecReturnCodes.SUCCESS)
            {
            	closeChannelAndShutDown("Error handling pings: " + _error.text());
            	return;
            }

            if (System.nanoTime() >= nextTickTime)
            {
            	nextTickTime += _nsecPerTick;

            	// only send bursts on tick boundary
            	if (_requestsSent)
            	{
            		// send item request and post bursts
            		if ((ret = sendBursts(currentTicks, _channel, _srcDirHandler.serviceInfo())) < TransportReturnCodes.SUCCESS)
            		{
            			if (ret != TransportReturnCodes.NO_BUFFERS)
            			{
            				continue;
            			}
            			// not successful cases were handled in sendBursts method
            		}
            	}

            	if (++currentTicks == _consPerfConfig.ticksPerSec())
            		currentTicks = 0;
            }
        }	// end of run loop
        
        try 
        {
        	if (_selector != null)
        	_selector.close();
		} 
        catch (IOException e) 
        {
			System.out.println("Selector close failure ");
		}
    	_consThreadInfo.shutdownAck(true);
        closeChannel();
        System.out.println("\nConsumerThread " + _consThreadInfo.threadId() + " exiting...");
	}

	/* Reads from a channel. */
	private void selectorReadAndWrite(long selectTime)
	{
		Set<SelectionKey> keySet = null;

		// set select time 
        try
        {
        	int selectRetVal;
        	selectTime /= 1000000;
        	if (selectTime > 0)
        		selectRetVal = _selector.select(selectTime);
        	else
        		selectRetVal = _selector.selectNow();
        		
            if (selectRetVal > 0)
            {
                keySet = _selector.selectedKeys();
            }
        }
        catch (IOException e1)
        {
        	closeChannelAndShutDown(e1.getMessage());
    		return;
        }

        // nothing to read or write
        if (keySet == null)
            return;

        Iterator<SelectionKey> iter = keySet.iterator();
        int ret = CodecReturnCodes.SUCCESS;
        while (iter.hasNext())
        {
        	SelectionKey key = iter.next();
        	iter.remove();
        	try
        	{
        		if (key.isReadable())
        		{
        			read();
        		}

        		/* flush for write file descriptor and active state */
        		if (key.isWritable())
        		{
        			ret = _channel.flush(_error);
        			if (ret == TransportReturnCodes.SUCCESS)
        			{
        				removeOption(SelectionKey.OP_WRITE, _channel);
        			}
        		}
        	}
        	catch (CancelledKeyException e)
        	{
        	} // key can be canceled during shutdown
        }
    }
	
	/* Writes an message to the UPA channel. */
	protected int writeMsg(Msg msg, Channel channel)
	{
	    int ret = TransportReturnCodes.SUCCESS;
	    
        TransportBuffer msgBuf = null;
        if ((msgBuf = channel.getBuffer(512, false, _error)) == null)
        {
            return TransportReturnCodes.NO_BUFFERS;
        }

        _eIter.clear();
        ret = _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.printf("setBufferAndRWFVersion() failed: %d\n", ret);
            return ret;
        }

        if ((ret = _requestMsg.encode(_eIter)) != CodecReturnCodes.SUCCESS)
        {
            System.out.printf("encodeMsg() failed: %d.\n", ret);
            return ret;
        }
        
        write(msgBuf);
        
        return ret;
	}

    /* Writes the content of the TransportBuffer to the UPA channel.*/
    protected void write(TransportBuffer msgBuf)
    {
    	// write data to the channel
    	_writeArgs.clear();
    	_writeArgs.priority(WritePriorities.HIGH);
    	_writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
    	int retval = _channel.write(msgBuf, _writeArgs, _error);

    	if (retval > TransportReturnCodes.FAILURE)
    	{
    		if (retval > TransportReturnCodes.SUCCESS)
    		{
    			// register for write if there's still data queued
    			addOption(SelectionKey.OP_WRITE, _channel);
    		}
    	}
    	else
    	{
    		if (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
    		{
    			//call flush and write again until there is data in the queue
    			while (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
    			{
    				retval = _channel.flush(_error);
    				if (retval < TransportReturnCodes.SUCCESS)
    					System.out.println("_channel flush failed with returned code: " + retval + " - " + _error.text());
    				retval = _channel.write(msgBuf, _writeArgs, _error);
    			}

    			//register for write if there's still data queued
    			if (retval > TransportReturnCodes.SUCCESS)
    			{
    				addOption(SelectionKey.OP_WRITE, _channel);
    			}
    		}
    		else
    		{
    			// write failed, release buffer and shut down 
    			_channel.releaseBuffer(msgBuf, _error);
            	closeChannelAndShutDown(_error.text());
            	return;
    		}
    	}
    }
    
    private void read()
    {
    	TransportBuffer msgBuf;
    	do /* read until no more to read */
    	{
    		msgBuf = _channel.read(_readArgs, _error);
    		if (msgBuf != null)
    		{
    			processResponse(msgBuf);

    			//set flag for server message received
    			_pingHandler.receivedMsg();
    		}
    		else
    		{
    			if (_readArgs.readRetVal() == TransportReturnCodes.READ_PING)
    			{
    				//set flag for server message received
    				_pingHandler.receivedMsg();
    			}
    		}
    	}
    	while (_readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
    }

    /** Process transport response. */
	public void processResponse(TransportBuffer buffer)
	{
        /* clear decode iterator */
        _dIter.clear();

        /* set buffer and version info */
        _dIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion());

        int ret = _responseMsg.decode(_dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
        	closeChannelAndShutDown("DecodeMsg(): Error " + ret);
    		return;
        }
        
        switch (_responseMsg.domainType())
        {
            case DomainTypes.LOGIN:
                processLoginResp();
                break;
            case DomainTypes.SOURCE:
                processSourceDirectoryResp();
                break;
            case DomainTypes.DICTIONARY:
                processDictionaryResp(_responseMsg, _dIter);
                break;
            case DomainTypes.MARKET_PRICE:
                processMarketPriceResp(_responseMsg, _dIter);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + _responseMsg.domainType());
                break;
        }
    }
    
    /* Process login response. */
    private void processLoginResp()
    {
        int ret = _loginHandler.processResponse(_responseMsg, _dIter, _error);
        if (ret != CodecReturnCodes.SUCCESS)
            if (ret != CodecReturnCodes.SUCCESS)
            {
            	closeChannelAndShutDown(_error.text());
        		return;
            }

        if (_responseMsg.msgClass() == MsgClasses.REFRESH)
        {
        	if (_consPerfConfig.postsPerSec() > 0 &&
        			(!_loginHandler.refreshInfo().checkHasFeatures() ||
        					!_loginHandler.refreshInfo().features().checkHasSupportPost() ||
        					_loginHandler.refreshInfo().features().supportOMMPost() == 0))
        	{
        		closeChannelAndShutDown("Provider for this connection does not support posting.");
        		return;
        	}
        }

        //Handle login states
        ConsumerLoginState loginState = _loginHandler.loginState();
        if (loginState == ConsumerLoginState.OK_SOLICITED)
        {
            TransportBuffer buffer = _srcDirHandler.getRequest(_channel, _error);
            if (buffer != null)
            {
            	write(buffer);
            }
            else
            {
            	closeChannelAndShutDown("Error sending directory request: " + _error.text());
        		return;
            }
        }
        else
        {
        	closeChannelAndShutDown("Invalid login state : " + loginState);
    		return;
        }
    }
    
    //process source directory response.
    private void processSourceDirectoryResp()
    {
        int ret = _srcDirHandler.processResponse(_channel, _responseMsg, _dIter, _error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
        	closeChannelAndShutDown(_error.text());
    		return;
        }

        if (_srcDirHandler.isRequestedServiceUp())
        {
            sendDictionaryRequests(_channel, _srcDirHandler.serviceInfo());
        }
        else
        {
            // service not up or
            // previously up service went down
            _requestsSent = false;

            System.out.println("Requested service '" + _consPerfConfig.serviceName() + "' not up. Waiting for service to be up...");
        }
    }

    private void sendDictionaryRequests(Channel channel, Service service)
    {
        if (_requestsSent)
            return;

        // dictionaries were loaded at initialization. send item requests and post
        // messages only after dictionaries are loaded.
        if (isDictionariesLoaded())
        {
            _consThreadInfo.dictionary(_dictionaryHandler.dictionary());
            System.out.println("Dictionary ready, requesting item(s)...\n");
        }
        else
        {
            _dictionaryHandler.serviceId(service.serviceId());
            TransportBuffer buffer = _dictionaryHandler.getRequest(channel, _dictionaryHandler.FIELD_DICTIONARY_TYPE, _error);
            if (buffer != null)
            {
            	write(buffer);
            }
            else
            {
            	closeChannelAndShutDown("Sending dictionary request failed");
        		return;
            }
            
            buffer = _dictionaryHandler.getRequest(channel, _dictionaryHandler.ENUM_DICTIONARY_TYPE, _error);
            if (buffer != null)
            {
            	write(buffer);
            }
            else
            {
            	closeChannelAndShutDown("Sending dictionary request failed");
        		return;
            }            
        }
        
        _requestsSent = true;
    }
    
    /* Process dictionary response. */
    protected void processDictionaryResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (_dictionaryHandler.processResponse(_channel, responseMsg, dIter, _error) != CodecReturnCodes.SUCCESS)
        {
        	closeChannelAndShutDown("Processing dictionary response failed");
    		return;
        }

        if (_dictionaryHandler.isFieldDictionaryLoaded()
                && _dictionaryHandler.isEnumTypeDictionaryLoaded())
        {
        	_consThreadInfo.dictionary(_dictionaryHandler.dictionary());
            System.out.println("Dictionary ready, requesting item(s)...\n");
        }
    }
    
    //process market price response.
    protected void processMarketPriceResp(Msg responseMsg, DecodeIterator dIter)
    {
    	int ret = CodecReturnCodes.SUCCESS;
    	int msgClass = responseMsg.msgClass();
    	
    	switch (msgClass)
    	{
    	case MsgClasses.REFRESH:
    		RefreshMsg refreshMsg = (RefreshMsg)responseMsg;
    		_consThreadInfo.stats().refreshCount().increment();
			
    		//If we are still retrieving images, check if this item still needs one.
			if((ret = _marketPriceDecoder.decodeUpdate(dIter, responseMsg, _consThreadInfo)) != CodecReturnCodes.SUCCESS)
			{
            	closeChannelAndShutDown("Decoding failure: " + ret);
        		return;
			}
    		if (_consThreadInfo.stats().imageRetrievalEndTime() == 0)
    		{
				if (refreshMsg.state().isFinal())
				{
	            	closeChannelAndShutDown("Received unexpected final state in refresh for item: " + StreamStates.toString(refreshMsg.state().streamState())+
							refreshMsg.msgKey().name().toString());
	        		return;
				}

				if (refreshMsg.checkRefreshComplete())
				{
    				if (_consPerfConfig.primeJVM() == false)
    				{
    	                if (refreshMsg.state().dataState() == DataStates.OK)
    	                {
                            _itemRequestList[refreshMsg.streamId()].requestState(ItemRequestState.HAS_REFRESH);
                            _consThreadInfo.stats().refreshCompleteCount().increment();
                            if (_consThreadInfo.stats().refreshCompleteCount().getTotal() == (_requestListSize - ITEM_STREAM_ID_START))
                            {
                                _consThreadInfo.stats().imageRetrievalEndTime(System.nanoTime());
                            }
    	                }
    				}
    				else // JVM priming enabled
    				{
    				    // Count snapshot images used for priming, ignoring state
                        if (_JVMPrimingRefreshCount < (_requestListSize - ITEM_STREAM_ID_START))
                        {
                            _JVMPrimingRefreshCount++;
                            // reset request state so items can be re-requested
                            _itemRequestList[refreshMsg.streamId()].requestState(ItemRequestState.NOT_REQUESTED);
                            if (_JVMPrimingRefreshCount == (_requestListSize - ITEM_STREAM_ID_START))
                            {
                                // reset request count and _requestListIndex so items can be re-requested
                            	//set the image retrieval start time
                                _consThreadInfo.stats().requestCount().init();
                                _consThreadInfo.stats().refreshCompleteCount().init();
                                _consThreadInfo.stats().refreshCount().init();
                                _requestListIndex = ITEM_STREAM_ID_START;
                            }
                        }
                        else // the streaming image responses after priming
                        {
                            if (refreshMsg.state().dataState() == DataStates.OK)
                            {
                                _itemRequestList[refreshMsg.streamId()].requestState(ItemRequestState.HAS_REFRESH);
                                _consThreadInfo.stats().refreshCompleteCount().increment();
                                if (_consThreadInfo.stats().refreshCompleteCount().getTotal() == (_requestListSize - ITEM_STREAM_ID_START))
                                {
                                    _consThreadInfo.stats().imageRetrievalEndTime(System.nanoTime());
                                }    
                            }
                        }
    				}
				}
    		}
    		break;
    	case MsgClasses.UPDATE:
    		if (_consThreadInfo.stats().imageRetrievalEndTime() > 0)
    		{
    			_consThreadInfo.stats().steadyStateUpdateCount().increment();
    		}
    		else
    		{
    			_consThreadInfo.stats().startupUpdateCount().increment();
    		}
			if (_consThreadInfo.stats().firstUpdateTime() == 0)
				_consThreadInfo.stats().firstUpdateTime(System.nanoTime());
			if((ret = _marketPriceDecoder.decodeUpdate(dIter, responseMsg, _consThreadInfo)) != CodecReturnCodes.SUCCESS)
			{
            	closeChannelAndShutDown("Decoding failure: " + ret);
        		return;
			}
    		break;
        case MsgClasses.GENERIC:
            _consThreadInfo.stats().genMsgRecvCount().increment();
            if((ret = _marketPriceDecoder.decodeUpdate(dIter, responseMsg, _consThreadInfo)) != CodecReturnCodes.SUCCESS)
            {
                closeChannelAndShutDown("Decoding failure: " + ret);
                return;
            }
            break;
    	case MsgClasses.STATUS:
    		StatusMsg statusMsg = (StatusMsg)responseMsg;
    		_consThreadInfo.stats().statusCount().increment();
    		
			if (statusMsg.checkHasState() &&
				statusMsg.state().isFinal())
			{
            	closeChannelAndShutDown("Received unexpected final state for item: " + StreamStates.toString(statusMsg.state().streamState())+
            			_itemRequestList[statusMsg.streamId()].itemName());
        		return;
			}

    		break;
    	default:
    		break;
    	}
    }
    
    //sends a burst of item requests.
    private int sendItemRequestBurst(int itemBurstCount, Channel channel, Service service)
    {
    	int ret;
    	Qos qos = null;

    	//Use a QoS from the service, if one is given.
    	if (service.checkHasInfo() &&
    		service.info().checkHasQos() &&
    		service.info().qosList().size() > 0)
    	{
    		qos = service.info().qosList().get(0);
    	}

    	for(int i = 0; i < itemBurstCount; ++i)
    	{
    		ItemRequest itemRequest;
    		
    		if (_requestListIndex == _requestListSize)
    			return TransportReturnCodes.SUCCESS;

    		itemRequest = _itemRequestList[_requestListIndex];

    		//Encode request msg.
    		_requestMsg.msgClass(MsgClasses.REQUEST);
    		//don't apply streaming for JVM priming and snapshot
    		if (_consPerfConfig.requestSnapshots() == false &&
    			((itemRequest.itemInfo().itemFlags() & ItemFlags.IS_STREAMING_REQ) > 0) &&
    			((_consPerfConfig.primeJVM() == false) ||
    			 (_consPerfConfig.primeJVM() == true &&
    			  _JVMPrimingRefreshCount == (_requestListSize - ITEM_STREAM_ID_START))))
    		{
    			_requestMsg.applyStreaming();
    		}

    		if (qos != null)
    		{
    			_requestMsg.applyHasQos();
    			_requestMsg.qos().dynamic(qos.isDynamic());
    			_requestMsg.qos().rate(qos.rate());
    			_requestMsg.qos().rateInfo(qos.rateInfo());
    			_requestMsg.qos().timeInfo(qos.timeInfo());
    			_requestMsg.qos().timeliness(qos.timeliness());
    		}

    		_requestMsg.streamId(itemRequest.itemInfo().streamId());
    		_requestMsg.domainType(itemRequest.itemInfo().attributes().domainType());
    		_requestMsg.containerType(DataTypes.NO_DATA);

    		_requestMsg.msgKey().flags(itemRequest.msgKey().flags());
    		_requestMsg.msgKey().name(itemRequest.msgKey().name());
    		_requestMsg.msgKey().applyHasServiceId();
    		_requestMsg.msgKey().serviceId(service.serviceId());
    		
            if ((ret = writeMsg(_requestMsg, channel)) != TransportReturnCodes.SUCCESS)
            {
                System.out.printf("writeMsg() failed: %d.\n", ret);
                return ret;
            }

    		//request has been made.
    		itemRequest.requestState(ItemRequestState.WAITING_FOR_REFRESH);

    		_requestListIndex++;
    		_consThreadInfo.stats().requestCount().increment();
    	}

    	return TransportReturnCodes.SUCCESS;
    }
    
    //sends a burst of post messages. 
	protected int sendPostBurst(int itemBurstCount, Channel channel)
	{
		int ret;
    	TransportBuffer msgBuf = null;
		long encodeStartTime;
		int latencyUpdateNumber;

		latencyUpdateNumber = (_consPerfConfig.latencyPostsPerSec() > 0) ?
				_postLatencyRandomArray.next() : -1; 

		for(int i = 0; i < itemBurstCount; ++i)
		{
			ItemRequest postItem = nextPostItem();
			int bufLen = _itemEncoder.estimateItemPostBufferLength(postItem.itemInfo());

    		if ((msgBuf = channel.getBuffer(bufLen, false, _error)) == null)
    		{
    			return TransportReturnCodes.NO_BUFFERS;
    		}

			if (latencyUpdateNumber == i)
				encodeStartTime = System.nanoTime()/1000;
			else
				encodeStartTime = 0;

			if ((ret = _itemEncoder.encodeItemPost(channel, postItem.itemInfo(), msgBuf, _postUserInfo, encodeStartTime)) != CodecReturnCodes.SUCCESS)
    		{
				System.out.printf("encodeItemPost() failed: %d.\n", ret);
				return ret;
			}

			write(msgBuf);

			_consThreadInfo.stats().postSentCount().increment();
		}

		return TransportReturnCodes.SUCCESS;
	}

	// sends a burst of generic messages.
	protected int sendGenMsgBurst(int itemBurstCount, Channel channel) 
	{
		int ret;
		TransportBuffer msgBuf = null;
		long encodeStartTime;
		int latencyGenMsgNumber;

		latencyGenMsgNumber = (_consPerfConfig.latencyGenMsgsPerSec() > 0) ? _genMsgLatencyRandomArray
				.next() : -1;

		for (int i = 0; i < itemBurstCount; ++i) 
		{
			ItemRequest genMsgItem = nextGenMsgItem();
			int bufLen = _itemEncoder.estimateItemGenMsgBufferLength(genMsgItem.itemInfo());

			if ((msgBuf = channel.getBuffer(bufLen, false, _error)) == null) 
			{
				return TransportReturnCodes.NO_BUFFERS;
			}

			if (latencyGenMsgNumber == i)
			{
				_consThreadInfo.stats().latencyGenMsgSentCount().increment();
				encodeStartTime = System.nanoTime() / 1000;
			}
			else
			    encodeStartTime = 0;

			if ((ret = _itemEncoder.encodeItemGenMsg(channel, genMsgItem.itemInfo(), msgBuf, encodeStartTime)) != CodecReturnCodes.SUCCESS) 
			{
				System.out.printf("encodeItemGenMsg() failed: %d.\n", ret);
				return ret;
			}

			write(msgBuf);

			_consThreadInfo.stats().genMsgSentCount().increment();
		}

    	return TransportReturnCodes.SUCCESS;
	}
	
	//retrieves next post item information to send.
	protected ItemRequest nextPostItem()
	{
		ItemRequest itemRequest = null;
		
		do
		{
			if (_postItemIndex == _requestListSize)
				_postItemIndex = ITEM_STREAM_ID_START;
			
			if ((_itemRequestList[_postItemIndex].itemInfo().itemFlags() & ItemFlags.IS_POST) > 0)
			{
				itemRequest = _itemRequestList[_postItemIndex];
			}

			_postItemIndex++;
		} while (itemRequest == null);

		return itemRequest;
	}

	// retrieves next generic msg item information to send.
	protected ItemRequest nextGenMsgItem() {
		ItemRequest itemRequest = null;

		do {
			if (_genMsgItemIndex == _requestListSize)
				_genMsgItemIndex = ITEM_STREAM_ID_START;

			if ((_itemRequestList[_genMsgItemIndex].itemInfo().itemFlags() & ItemFlags.IS_GEN_MSG) > 0) {
				itemRequest = _itemRequestList[_genMsgItemIndex];
			}

			_genMsgItemIndex++;
		} while (itemRequest == null);

		return itemRequest;
	}
    
    //indicates if dictionaries have been loaded.
    protected boolean isDictionariesLoaded()
    {
        return _dictionaryHandler.isFieldDictionaryLoaded()
                && _dictionaryHandler.isEnumTypeDictionaryLoaded();
    }

    //close channel used by this thread.
	private void closeChannel()
    {
		_channel.close(_error);
        Transport.uninitialize();
    }

    //print estimated post message sizes.
    protected void printEstimatedPostMsgSizes(Channel channel)
    {
        int ret;
    	TransportBuffer testBuffer;

    	_msgKey.clear();
    	_msgKey.applyHasNameType();
    	_msgKey.applyHasServiceId();
    	_msgKey.nameType(InstrumentNameTypes.RIC);
    	_msgKey.name().data("RDT0");
    	_msgKey.serviceId(0);

    	//Market Price
        if (_msgData.marketPricePostMsgCount() > 0)
        {
            _itemInfo.clear();
            _itemInfo.attributes().msgKey(_msgKey);
            _itemInfo.attributes().domainType(DomainTypes.MARKET_PRICE);
            _itemInfo.itemData(_mpItem);

            if (_consPerfConfig.postsPerSec() > 0) 
            {
                System.out.printf("Approximate message sizes:\n");

                for (int i = 0; i < _msgData.marketPricePostMsgCount(); ++i) 
                {
                    int bufLen = _itemEncoder.estimateItemPostBufferLength(_itemInfo);
                    testBuffer = channel.getBuffer(bufLen, false, _error);
                    if ((ret = _itemEncoder.encodeItemPost(_consThreadInfo.channel(), _itemInfo, testBuffer, _postUserInfo, 0)) != CodecReturnCodes.SUCCESS) 
                    {
                        closeChannelAndShutDown("printEstimatedPostMsgSizes: encodeItemPost() failed: " + CodecReturnCodes.toString(ret));
                        return;
                    }
                    System.out.printf("  MarketPrice PostMsg %d: \n", i + 1);
                    System.out.printf("         estimated length: %d bytes\n", bufLen);
                    System.out.printf("    approx encoded length: %d bytes\n", testBuffer.length());
                    channel.releaseBuffer(testBuffer, _error);
                }
            }
        }
		System.out.printf("\n");
	}

	// print estimated generic message sizes.
	protected void printEstimatedGenMsgSizes(Channel channel) 
	{
	    int ret;
		TransportBuffer testBuffer;

		_msgKey.clear();
		_msgKey.applyHasNameType();
		_msgKey.applyHasServiceId();
		_msgKey.nameType(InstrumentNameTypes.RIC);
		_msgKey.name().data("RDT0");
		_msgKey.serviceId(0);

		// Market Price
		if (_msgData.marketPriceGenMsgCount() > 0)
		{
			_itemInfo.clear();
			_itemInfo.attributes().msgKey(_msgKey);
			_itemInfo.attributes().domainType(DomainTypes.MARKET_PRICE);
			_itemInfo.itemData(_mpItem);

			if (_consPerfConfig.genMsgsPerSec() > 0) 
			{
				System.out.printf("Approximate message sizes:\n");

				for (int i = 0; i < _msgData.marketPriceGenMsgCount(); ++i) 
				{
					int bufLen = _itemEncoder.estimateItemGenMsgBufferLength(_itemInfo);
					testBuffer = channel.getBuffer(bufLen, false, _error);
					if ((ret = _itemEncoder.encodeItemGenMsg(_consThreadInfo.channel(),	_itemInfo, testBuffer, 0)) != CodecReturnCodes.SUCCESS) 
					{
						closeChannelAndShutDown("printEstimatedGenMsgSizes: encodeItemGenMsg() failed: " + CodecReturnCodes.toString(ret));
						return;
					}
					System.out.printf("  MarketPrice GenMsg %d: \n", i + 1);
					System.out.printf("         estimated length: %d bytes\n", bufLen);
					System.out.printf("    approx encoded length: %d bytes\n", testBuffer.length());
					channel.releaseBuffer(testBuffer, _error);
				}
			}
		}

    	System.out.printf("\n");
	}
    
    // populates item information from the XML list.
    private void populateItemInfo()
    {
		int itemListIndex = 0;
		_postItemCount = 0;
		_genMsgItemCount = 0;

    	for(int streamId = ITEM_STREAM_ID_START; streamId < _requestListSize; ++streamId)
    	{
    		/* Once we have filled our list with the common items,
    		 * start using the range of items unique to this consumer thread. */
			if (itemListIndex == _consPerfConfig.commonItemCount()
					&& itemListIndex < _consThreadInfo.itemListUniqueIndex())
				itemListIndex = _consThreadInfo.itemListUniqueIndex();

			_itemRequestList[streamId].clear();
			_itemRequestList[streamId].itemInfo().streamId(streamId);
			_itemRequestList[streamId].itemInfo().attributes().domainType(_itemInfoList.itemInfoList()[itemListIndex].domainType());

			_itemRequestList[streamId].itemName(_itemInfoList.itemInfoList()[itemListIndex].name());
			_itemRequestList[streamId].msgKey().applyHasName();
			Buffer buffer = CodecFactory.createBuffer();
			buffer.data(_itemInfoList.itemInfoList()[itemListIndex].name());
			_itemRequestList[streamId].msgKey().name(buffer);
    		_itemRequestList[streamId].itemInfo().attributes().msgKey(_itemRequestList[streamId].msgKey());

    		if (!_itemInfoList.itemInfoList()[itemListIndex].isSnapshot())
    		{
    			int flags = _itemRequestList[streamId].itemInfo().itemFlags() | ItemFlags.IS_STREAMING_REQ;
    			_itemRequestList[streamId].itemInfo().itemFlags(flags);
    		}

    		if (_itemInfoList.itemInfoList()[itemListIndex].isPost() && _consPerfConfig.postsPerSec() > 0)
    		{
    			Object itemData = null;
    			
    			++_postItemCount;

    			int flags = _itemRequestList[streamId].itemInfo().itemFlags() | ItemFlags.IS_POST;
    			_itemRequestList[streamId].itemInfo().itemFlags(flags);

    			//get item info data from appropriate pool
    			switch(_itemRequestList[streamId].itemInfo().attributes().domainType())
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

				_itemRequestList[streamId].itemInfo().itemData(itemData);
			}

			if (_consPerfConfig.postsPerSec() > 0)
			{
				if (_haveMarketPricePostItems && _msgData.marketPricePostMsgCount() == 0) 
				{
					System.out.printf(
							"Error: No MarketPrice posting data in file: %s\n",
							_consPerfConfig.msgFilename());
					System.exit(-1);
				}
			}

			if (_itemInfoList.itemInfoList()[itemListIndex].isGenMsg() && _consPerfConfig.genMsgsPerSec() > 0) 
			{
				Object itemData = null;

				++_genMsgItemCount;

				int flags = _itemRequestList[streamId].itemInfo().itemFlags() | ItemFlags.IS_GEN_MSG;
				_itemRequestList[streamId].itemInfo().itemFlags(flags);

				// get item info data from appropriate pool
				switch (_itemRequestList[streamId].itemInfo().attributes().domainType()) 
				{
				case DomainTypes.MARKET_PRICE:
					itemData = new MarketPriceItem();
					_haveMarketPriceGenMsgItems = true;
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

				_itemRequestList[streamId].itemInfo().itemData(itemData);
			}

			if (_consPerfConfig.genMsgsPerSec() > 0)
			{
				if (_haveMarketPriceGenMsgItems && _msgData.marketPriceGenMsgCount() == 0) 
				{
					System.out.printf("Error: No MarketPrice generic msg data in file: %s\n", _consPerfConfig.msgFilename());
					System.exit(-1);
				}
			}

			++itemListIndex;
		}
	}

	// send item requests, post bursts and or generic msg bursts.
	protected int sendBursts(int currentTicks, Channel channel, Service service) 
	{
		int ret = TransportReturnCodes.SUCCESS;

    	//send item requests until all sent
    	if (_consThreadInfo.stats().requestCount().getTotal() < (_requestListSize - ITEM_STREAM_ID_START))
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
                    if (_JVMPrimingRefreshCount == (_requestListSize - ITEM_STREAM_ID_START))
                    {
                        _consThreadInfo.stats().imageRetrievalStartTime(System.nanoTime());
                    }
                }
			}

			if ((ret = sendItemRequestBurst(requestBurstCount, channel, service)) < TransportReturnCodes.SUCCESS) 
			{
				if (ret != TransportReturnCodes.NO_BUFFERS)
				{
					_shutdownCallback.shutdown();
				}
				return ret;
			}
		}

		// send bursts of posts and or generic msgs
		if (_consThreadInfo.stats().imageRetrievalEndTime() > 0) 
		{
			if (_consPerfConfig.postsPerSec() > 0 && _postItemCount > 0) 
			{
				if ((ret = sendPostBurst((int) (_postsPerTick + ((currentTicks < _postsPerTickRemainder) ? 1 : 0)), channel)) < TransportReturnCodes.SUCCESS) 
				{
					if (ret != TransportReturnCodes.NO_BUFFERS) 
					{
						_shutdownCallback.shutdown();
					}
					return ret;
				}
			}
			if (_consPerfConfig.genMsgsPerSec() > 0 && _genMsgItemCount > 0) 
			{
				if ((ret = sendGenMsgBurst((int) (_genMsgsPerTick + ((currentTicks < _genMsgsPerTickRemainder) ? 1 : 0)), channel)) < TransportReturnCodes.SUCCESS) 
				{
					if (ret != TransportReturnCodes.NO_BUFFERS) 
					{
						_shutdownCallback.shutdown();
					}
					return ret;
				}
			}
		}

		return TransportReturnCodes.SUCCESS;
	}

    private void addOption(int option, Object attachment)
    {
        SelectionKey key = _channel.selectableChannel().keyFor(_selector);
        int newoption = option;
        int oldoption = 0;
        if (key != null)
        {
            oldoption = key.interestOps();
            newoption |= oldoption;
        }
        
        try 
        {
			_channel.selectableChannel().register(_selector, newoption, attachment);
		} 
        catch (ClosedChannelException e) 
        {
        	closeChannelAndShutDown("Add option failed failed");
        	return;
		}
    }

    private void removeOption(int option, Object attachment)
    {
        SelectionKey key = _channel.selectableChannel().keyFor(_selector);
        if (key == null)
            return;
        if ((option & key.interestOps()) == 0)
        	return;
        int newoption = key.interestOps() - option;
        if (newoption != 0)
        	try 
        {
        		_channel.selectableChannel().register(_selector, newoption, attachment);
        } 
        catch (ClosedChannelException e) 
        {
        	closeChannelAndShutDown("Remove option failed failed");
        	return;
        }
        else
        	key.cancel();
    }

    protected void closeChannelAndShutDown(String text)
    {
		System.out.println(text);
		_shutdownCallback.shutdown();
		_consThreadInfo.shutdownAck(true);
    	closeChannel();
    	return;    	
    }
}
