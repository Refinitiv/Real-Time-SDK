/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.consperf;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.ByteArrayInputStream;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.charset.StandardCharsets;
import java.util.*;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;
import com.fasterxml.jackson.core.json.JsonReadFeature;
import com.fasterxml.jackson.core.JsonGenerator;

import com.refinitiv.eta.perftools.common.ByteBufferInputStream;
import com.refinitiv.eta.perftools.common.ByteBufferOutputStream;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.PostUserInfo;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.shared.ConsumerLoginState;
import com.refinitiv.eta.shared.PingHandler;
import com.refinitiv.eta.perftools.common.DictionaryHandler;
import com.refinitiv.eta.perftools.common.DirectoryHandler;
import com.refinitiv.eta.perftools.common.DirectoryHandlerJson;
import com.refinitiv.eta.perftools.common.ItemEncoder;
import com.refinitiv.eta.perftools.common.ItemFlags;
import com.refinitiv.eta.perftools.common.ItemInfo;
import com.refinitiv.eta.perftools.common.JsonConverterSession;
import com.refinitiv.eta.perftools.common.JsonServiceNameToIdCallback;
import com.refinitiv.eta.perftools.common.LatencyRandomArray;
import com.refinitiv.eta.perftools.common.LatencyRandomArrayOptions;
import com.refinitiv.eta.perftools.common.LoginHandler;
import com.refinitiv.eta.perftools.common.LoginHandlerJson;
import com.refinitiv.eta.perftools.common.MarketPriceItem;
import com.refinitiv.eta.perftools.common.PerfToolsReturnCodes;
import com.refinitiv.eta.perftools.common.ResponseCallback;
import com.refinitiv.eta.perftools.common.ShutdownCallback;
import com.refinitiv.eta.perftools.common.XmlItemInfoList;
import com.refinitiv.eta.perftools.common.XmlMsgData;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.IoctlCodes;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.WritePriorities;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.reactor.ConsumerCallback;
import com.refinitiv.eta.valueadd.reactor.ConsumerRole;
import com.refinitiv.eta.valueadd.reactor.DictionaryDownloadModes;
import com.refinitiv.eta.valueadd.reactor.RDMDictionaryMsgEvent;
import com.refinitiv.eta.valueadd.reactor.RDMDirectoryMsgEvent;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgEvent;
import com.refinitiv.eta.valueadd.reactor.Reactor;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelType;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorConnectOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorDispatchOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorJsonConversionEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorJsonConversionEventCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorJsonConverterOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorServiceNameToId;
import com.refinitiv.eta.valueadd.reactor.ReactorServiceNameToIdCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorServiceNameToIdEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyGroup;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServerInfo;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamInfo;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamSubmitOptions;

/** Provides the logic that consumer connections use in ConsPerf for
  * connecting to a provider, requesting items, and processing the received
  * refreshes and updates.
  */
public class ConsumerThread implements Runnable, ResponseCallback, ConsumerCallback, JsonServiceNameToIdCallback,
												ReactorJsonConversionEventCallback, ReactorServiceNameToIdCallback
{
    private static final int CONNECTION_RETRY_TIME = 1; // seconds
    private static final int ITEM_STREAM_ID_START = 6;
    private static final int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
    
    public static final int MAX_MSG_SIZE = 1024;

    private ConsumerThreadInfo _consThreadInfo; /* thread information */
    private ConsPerfConfig _consPerfConfig; /* configuration information */
    private EncodeIterator _eIter; /* encode iterator */
    private DecodeIterator _dIter; /* decode iterator */
    private WriteArgs _writeArgs = TransportFactory.createWriteArgs();
    private ReadArgs _readArgs = TransportFactory.createReadArgs();
    private Msg _responseMsg; /* response message */
    private LoginHandler _loginHandler; /* login handler */
    private LoginHandlerJson _loginHandlerJson; /* login handler JSON */
    private DirectoryHandler _srcDirHandler; /* source directory handler */ 
    private DirectoryHandlerJson _srcDirHandlerJson; /* source directory handler JSON */ 
    private DictionaryHandler _dictionaryHandler; /* dictionary handler */
    private PingHandler _pingHandler; /* ping handler */
    private MarketPriceDecoder _marketPriceDecoder; /* market price decoder */
    private MarketPriceDecoderJson _marketPriceDecoderJson; /* market price decoder JSON */
    private InProgInfo _inProg; /* connection in progress information */
    private Error _error; /* error structure */
    private XmlItemInfoList _itemInfoList; /* item information list from XML file */
    private XmlMsgData _msgData; /* message data information from XML file */
    private ItemRequest[] _itemRequestList; /* item request list */
    private int _postItemCount; /* number of items in _itemRequestList that are posting items */
    private int _genMsgItemCount; /* number of items in _itemRequestList that are for sending generic msgs on items */
    private RequestMsg _requestMsg; /* request message */
    private PostUserInfo _postUserInfo; /* post user information */
    private boolean _requestsSent; /* indicates if requested service is up */
    private long _nsecPerTick; /* nanoseconds per tick */
    private long _millisPerTick; /* milliseconds per tick */
    private int _requestListSize; /* request list size */
    private int _requestListIndex; /* current request list index */
    private ShutdownCallback _shutdownCallback; /* shutdown callback to main application */
    private ConnectOptions _connectOpts; /* connection options */
    private ChannelInfo _chnlInfo; /* channel information */
    private boolean _haveMarketPricePostItems; /* indicates there are post items in the item list */
    private long _postsPerTick; /* posts per tick */
    private long _postsPerTickRemainder; /* posts per tick remainder */
    private boolean _haveMarketPriceGenMsgItems; /* indicates there are generic msg items in the item list */
    private long _genMsgsPerTick; /* gen msgs per tick */
    private long _genMsgsPerTickRemainder; /* gen msgs per tick remainder */
    private LatencyRandomArrayOptions _randomArrayOpts; /* random array options */
    private LatencyRandomArray _postLatencyRandomArray; /* post random latency array */
    private int _postItemIndex; /* current post item index */
    private LatencyRandomArray _genMsgLatencyRandomArray; /* generic msg random latency array */
    private int _genMsgItemIndex; /* current generic msg item index */
    private ItemEncoder _itemEncoder; /* item encoder */
    private MarketPriceItem _mpItem; /* market price item */
    private MsgKey _msgKey; /* message key */
    private ItemInfo _itemInfo; /* item information */
    private int _JVMPrimingRefreshCount; /* used to determine when JVM priming is complete */
    private Channel _channel;
    private Selector _selector;
    
    private Reactor _reactor; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorOptions _reactorOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ConsumerRole _role; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorErrorInfo _errorInfo; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorConnectOptions _connectOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorDispatchOptions _dispatchOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorSubmitOptions _submitOptions; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private Service _service; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorConnectInfo _connectInfo;  // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorChannelInfo _reactorChannnelInfo;  // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorChannel _reactorChannel; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private PostMsg _postMsg; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private Buffer _postBuffer; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private GenericMsg _genericMsg; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private Buffer _genericBuffer; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private DictionaryRequest _dictionaryRequest; // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private Buffer _fieldDictionaryName = CodecFactory.createBuffer(); // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private Buffer _enumTypeDictionaryName = CodecFactory.createBuffer(); // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private TunnelStreamHandler _tunnelStreamHandler; // Use the VA Reactor tunnel stream instead of the ETA Channel for sending and receiving
    private TunnelStreamSubmitOptions _tunnelStreamSubmitOptions;
    private TunnelStreamInfo _tunnelStreamInfo;
    protected JsonConverterSession		_jsonConverterSession;				// Use the converter library to converter between JSON and RWF messages.

    private ObjectMapper		_jsonObjectMapper;
    private JsonGenerator		_jsonGenerator;
    private ByteBufferInputStream	_jsonInputStream;
    private ByteBufferOutputStream	_jsonOutputStream;

    {
    	_eIter = CodecFactory.createEncodeIterator();
    	_dIter = CodecFactory.createDecodeIterator();
    	_responseMsg = CodecFactory.createMsg();
        _loginHandler = new LoginHandler();
        _loginHandlerJson = new LoginHandlerJson();
        _srcDirHandler = new DirectoryHandler();
        _srcDirHandlerJson = new DirectoryHandlerJson();
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
        _tunnelStreamInfo = ReactorFactory.createTunnelStreamInfo();
        _jsonConverterSession = new JsonConverterSession();
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
		_marketPriceDecoderJson = new MarketPriceDecoderJson(_postUserInfo);
        
        _reactorOptions = ReactorFactory.createReactorOptions();
        _role = ReactorFactory.createConsumerRole();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _connectOptions = ReactorFactory.createReactorConnectOptions();
        _dispatchOptions = ReactorFactory.createReactorDispatchOptions();
        _submitOptions = ReactorFactory.createReactorSubmitOptions();
        _connectInfo = ReactorFactory.createReactorConnectInfo();
        _reactorChannnelInfo = ReactorFactory.createReactorChannelInfo();
        _postMsg = (PostMsg)CodecFactory.createMsg();
        _postBuffer = CodecFactory.createBuffer();
        _postBuffer.data(ByteBuffer.allocate(512));
        _genericMsg = (GenericMsg)CodecFactory.createMsg();
        _genericBuffer = CodecFactory.createBuffer();
        _genericBuffer.data(ByteBuffer.allocate(512));
        _dictionaryRequest = (DictionaryRequest)DictionaryMsgFactory.createMsg();
        _dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
        _fieldDictionaryName.data("RWFFld");
        _enumTypeDictionaryName.data("RWFEnum");
        
        /* Creates the tunnel stream handle to send and receive messages using a tunnel stream. */
        if (consConfig.useTunnel())
        {
        	_tunnelStreamHandler = new TunnelStreamHandler(this, consConfig.tunnelAuth(), DomainTypes.SYSTEM);
        	_tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
        	_tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
        }
	}

	/* Initializes consumer thread. */
	private void initialize()
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
		_millisPerTick = 1000 / _consPerfConfig.ticksPerSec();
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
    private void connect()
    {
        ConnectOptions connectOptions;
        
        // open selector
        try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.out.println("selector open failure");
            System.exit(-1);
        }
        
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            connectOptions = _connectOpts;
        }
        else // use ETA VA Reactor for sending and receiving
        {
            connectOptions =  _connectInfo.connectOptions();
        }
        
        /* set connect options  */
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());        
        connectOptions.connectionType(_consPerfConfig.connectionType());
        connectOptions.wSocketOpts().protocols(_consPerfConfig.protocolList());
        connectOptions.guaranteedOutputBuffers(_consPerfConfig.guaranteedOutputBuffers());
        connectOptions.numInputBuffers(_consPerfConfig.numInputBuffers());
        if (_consPerfConfig.sendBufSize() > 0)
        {
            connectOptions.sysSendBufSize(_consPerfConfig.sendBufSize());
        }
        if (_consPerfConfig.recvBufSize() > 0)
        {
            connectOptions.sysRecvBufSize(_consPerfConfig.recvBufSize());
        }
        if(_consPerfConfig.connectionType() == ConnectionTypes.SOCKET)
        {
            connectOptions.tcpOpts().tcpNoDelay(_consPerfConfig.tcpNoDelay());
        }
        // set the connection parameters on the connect options 
        connectOptions.unifiedNetworkInfo().address(_consPerfConfig.hostName());
        connectOptions.unifiedNetworkInfo().serviceName(_consPerfConfig.portNo());
        connectOptions.unifiedNetworkInfo().interfaceName(_consPerfConfig.interfaceName());

        if(connectOptions.connectionType() == ConnectionTypes.ENCRYPTED)
        {
        	connectOptions.tunnelingInfo().tunnelingType("None");
        	connectOptions.encryptionOptions().connectionType(_consPerfConfig.encryptedConnectionType());
        	connectOptions.encryptionOptions().KeystoreFile(_consPerfConfig.keyfile());
        	connectOptions.encryptionOptions().KeystorePasswd(_consPerfConfig.keypasswd());
        }
        
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            // Initialize Transport
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(_consPerfConfig.threadCount() > 1 ? true : false);
            if (Transport.initialize(initArgs, _error) != TransportReturnCodes.SUCCESS)
            {
        		System.out.println("Transport.initialize failed ");
        		System.exit(-1);
            }
            		
        	// Connection recovery loop. It will try to connect until successful
            System.out.println("Starting connection...");
            int handshake;
            boolean initializedChannel;
            while (!_consThreadInfo.shutdown())
            {
            	initializedChannel = false;
            	while (!initializedChannel)
            	{
                    _channel = Transport.connect(connectOptions, _error);
                    if (_channel == null)
                    {
                        System.err.println("Error: Transport connect failure: " + _error.text() + ". Will retry shortly.");
                        try
                    	{
                    		Thread.sleep(CONNECTION_RETRY_TIME * 1000);
                    		continue;
                    	}
                    	catch(Exception e)
                    	{
                    		System.out.println("Thread.sleep failed ");
                    		System.exit(-1);
                    	}
                    }
                    initializedChannel = true;
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

                if (handshake == TransportReturnCodes.SUCCESS) {
                    break;
                }
                
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
            	if (_consPerfConfig.calcRWFJSONConversionLatency() && _channel.protocolType() == Codec.RWF_PROTOCOL_TYPE) {
                    _consThreadInfo.conversionTimeHandled(true);
                }
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
        
            	// register selector for read
            	addOption(SelectionKey.OP_READ, _channel);
            }
        }
        else // use ETA VA Reactor for sending and receiving
        {
            // initialize Reactor
            _reactor = ReactorFactory.createReactor(_reactorOptions, _errorInfo);
            if (_errorInfo.code() != ReactorReturnCodes.SUCCESS)
            {
                System.out.println("createReactor() failed: " + _errorInfo.toString());
                System.exit(ReactorReturnCodes.FAILURE);            
            }
            
            // register selector with reactor's reactorChannel.
            try
            {
                _reactor.reactorChannel().selectableChannel().register(_selector,
                                                                    SelectionKey.OP_READ,
                                                                    _reactor.reactorChannel());
            }
            catch (ClosedChannelException e)
            {
                System.out.println("selector register failed: " + e.getLocalizedMessage());
                System.exit(ReactorReturnCodes.FAILURE);
            }
            
            ReactorJsonConverterOptions jsonConverterOptions = ReactorFactory.createReactorJsonConverterOptions();
        	jsonConverterOptions.dataDictionary(_dictionaryHandler.dictionary());
        	jsonConverterOptions.userSpec(this);
            jsonConverterOptions.serviceNameToIdCallback(this);
            jsonConverterOptions.jsonConversionEventCallback(this);

            // Initialize the JSON converter
            if ( _reactor.initJsonConverter(jsonConverterOptions, _errorInfo) != ReactorReturnCodes.SUCCESS)
            {
                System.out.println("Reactor.initJsonConverter() failed: " + _errorInfo.toString());
                System.exit(ReactorReturnCodes.FAILURE);
            }
            
            
            _connectOptions.connectionList().add(_connectInfo);
            
            // set consumer role information
            _role.channelEventCallback(this);
            _role.defaultMsgCallback(this);
            _role.loginMsgCallback(this);
            _role.directoryMsgCallback(this);
            _role.dictionaryMsgCallback(this);
            if (!isDictionariesLoaded() && !_consPerfConfig.useWatchlist())
            {
                _role.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            }
            _role.initDefaultRDMLoginRequest();
            // set login parameters
            _role.rdmLoginRequest().applyHasAttrib();
            _role.rdmLoginRequest().attrib().applyHasApplicationName();
            _role.rdmLoginRequest().attrib().applicationName().data("ConsPerf");
            if (_consPerfConfig.username() != null && !_consPerfConfig.username().equals(""))
            {
                _role.rdmLoginRequest().userName().data(_consPerfConfig.username());
            }
            _role.initDefaultRDMDirectoryRequest();
            // enable watchlist if configured
            if (_consPerfConfig.useWatchlist())
            {
                _role.watchlistOptions().enableWatchlist(true);
                // set itemCountHint to itemRequestCount
                _role.watchlistOptions().itemCountHint(_consPerfConfig.itemRequestCount());
                // set request timeout to 0, request timers reduce performance
                _role.watchlistOptions().requestTimeout(0);
            }
            
            _connectOptions.reconnectAttemptLimit(-1);
            _connectOptions.reconnectMaxDelay(5000);
            _connectOptions.reconnectMinDelay(1000);
            
            if(!_consPerfConfig.startingHostName().isEmpty())
            {
            	
            	if(!_consPerfConfig.useWatchlist())
            	{
            		 System.out.println("Warm Standby cannot be enabled without enabling reactor and watchlist functionality");
                     System.exit(ReactorReturnCodes.FAILURE);            		
            	}
            	
            	ReactorWarmStandbyGroup wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
            	wsbGroup.warmStandbyMode(_consPerfConfig.wsbMode());
            	
            	connectOptions = wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions();
            	
            	/* set connect options  */
                connectOptions.majorVersion(Codec.majorVersion());
                connectOptions.minorVersion(Codec.minorVersion());        
                connectOptions.connectionType(_consPerfConfig.connectionType());
                connectOptions.wSocketOpts().protocols(_consPerfConfig.protocolList());
                connectOptions.guaranteedOutputBuffers(_consPerfConfig.guaranteedOutputBuffers());
                connectOptions.numInputBuffers(_consPerfConfig.numInputBuffers());
                if (_consPerfConfig.sendBufSize() > 0)
                {
                    connectOptions.sysSendBufSize(_consPerfConfig.sendBufSize());
                }
                if (_consPerfConfig.recvBufSize() > 0)
                {
                    connectOptions.sysRecvBufSize(_consPerfConfig.recvBufSize());
                }
                if(_consPerfConfig.connectionType() == ConnectionTypes.SOCKET)
                {
                    connectOptions.tcpOpts().tcpNoDelay(_consPerfConfig.tcpNoDelay());
                }
                // set the connection parameters on the connect options 
                connectOptions.unifiedNetworkInfo().address(_consPerfConfig.startingHostName());
                connectOptions.unifiedNetworkInfo().serviceName(_consPerfConfig.startingPort());
                connectOptions.unifiedNetworkInfo().interfaceName(_consPerfConfig.interfaceName());

                if(connectOptions.connectionType() == ConnectionTypes.ENCRYPTED)
                {
                	connectOptions.tunnelingInfo().tunnelingType("None");
                	connectOptions.encryptionOptions().connectionType(_consPerfConfig.encryptedConnectionType());
                	connectOptions.encryptionOptions().KeystoreFile(_consPerfConfig.keyfile());
                	connectOptions.encryptionOptions().KeystorePasswd(_consPerfConfig.keypasswd());
                }
                
                if(!_consPerfConfig.standbyHostName().isEmpty())
                {
                	ReactorWarmStandbyServerInfo wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
                	
                	connectOptions = wsbServerInfo.reactorConnectInfo().connectOptions();
                	/* set connect options  */
                    connectOptions.majorVersion(Codec.majorVersion());
                    connectOptions.minorVersion(Codec.minorVersion());        
                    connectOptions.connectionType(_consPerfConfig.connectionType());
                    connectOptions.wSocketOpts().protocols(_consPerfConfig.protocolList());
                    connectOptions.guaranteedOutputBuffers(_consPerfConfig.guaranteedOutputBuffers());
                    connectOptions.numInputBuffers(_consPerfConfig.numInputBuffers());
                    if (_consPerfConfig.sendBufSize() > 0)
                    {
                        connectOptions.sysSendBufSize(_consPerfConfig.sendBufSize());
                    }
                    if (_consPerfConfig.recvBufSize() > 0)
                    {
                        connectOptions.sysRecvBufSize(_consPerfConfig.recvBufSize());
                    }
                    if(_consPerfConfig.connectionType() == ConnectionTypes.SOCKET)
                    {
                        connectOptions.tcpOpts().tcpNoDelay(_consPerfConfig.tcpNoDelay());
                    }
                    // set the connection parameters on the connect options 
                    connectOptions.unifiedNetworkInfo().address(_consPerfConfig.standbyHostName());
                    connectOptions.unifiedNetworkInfo().serviceName(_consPerfConfig.standbyPort());
                    connectOptions.unifiedNetworkInfo().interfaceName(_consPerfConfig.interfaceName());

                    if(connectOptions.connectionType() == ConnectionTypes.ENCRYPTED)
                    {
                    	connectOptions.tunnelingInfo().tunnelingType("None");
                    	connectOptions.encryptionOptions().connectionType(_consPerfConfig.encryptedConnectionType());
                    	connectOptions.encryptionOptions().KeystoreFile(_consPerfConfig.keyfile());
                    	connectOptions.encryptionOptions().KeystorePasswd(_consPerfConfig.keypasswd());
                    }
                    wsbGroup.standbyServerList().add(wsbServerInfo);
                }
                
                _connectOptions.reactorWarmStandbyGroupList().add(wsbGroup);
            }

            // connect via Reactor
            int ret;
            if ((ret = _reactor.connect(_connectOptions, _role, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("Reactor.connect failed with return code: " + ret + " error = " + _errorInfo.error().text());
                System.exit(ReactorReturnCodes.FAILURE);
            }       
        }
	}
    
    public ConsPerfConfig consumerPerfConfig()
    {
    	return _consPerfConfig;
    }

	/** Run the consumer thread. */
	public void run()
	{
		// initialize the test data from configuration and xml files
		initialize();

        if (!_consThreadInfo.shutdown())
        {
            if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
            {
                // Check if the test is configured for the correct buffer size to fit post messages
                printEstimatedPostMsgSizes(_channel);
    
                // Check if the test is configured for the correct buffer size to fit generic messages
                printEstimatedGenMsgSizes(_channel);

                if (initializeJsonSession() != CodecReturnCodes.SUCCESS) {
                    closeChannelAndShutDown("RWF/JSON Converter failed: " + _error.text());
                }

                if (_channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && !_consPerfConfig.convertJSON())
                {
                    _jsonInputStream = new ByteBufferInputStream();
                    _jsonOutputStream = new ByteBufferOutputStream();

                    _jsonObjectMapper = new ObjectMapper();
                    _jsonObjectMapper.enable(JsonReadFeature.ALLOW_LEADING_ZEROS_FOR_NUMBERS.mappedFeature());

                    try
                    {
                        _jsonGenerator = _jsonObjectMapper.getFactory().createGenerator(_jsonOutputStream);
                    }
                    catch (IOException e)
                    {
                        closeChannelAndShutDown("JSON Generator failed");
                        return;
                    }
                }
    
                // set service name in directory handler
                _srcDirHandler.serviceName(_consPerfConfig.serviceName());
    
                // get and print the channel info
                if (_channel.info(_chnlInfo, _error) != TransportReturnCodes.SUCCESS)
                {
                    closeChannelAndShutDown("Channel.info() failed");
                    return;
                } 
                System.out.printf("Channel active. " + _chnlInfo.toString() + "\n");
                
                TransportBuffer msg = null;

                if (_channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && !_consPerfConfig.convertJSON())
                {
                    // Send login request message
                    // .applicationName and .role are fixed parts of JSON message here
                    msg = _loginHandlerJson.getRequestMsg(_channel, _error, _consPerfConfig.username(),
                                                          _jsonObjectMapper, _jsonGenerator, _jsonOutputStream);
                }
                else
                {
                    // set login parameters
                    _loginHandler.applicationName("ConsPerf");
                    _loginHandler.userName(_consPerfConfig.username());
                    _loginHandler.role(Login.RoleTypes.CONS);

                    msg = _loginHandler.getRequestMsg(_channel, _error, _eIter);
                }

                // Send login request message
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
        }

        int ret = 0;
        int currentTicks = 0;
        long nextTickTime;
        long selectTime;
        
        nextTickTime = initNextTickTime();

        while (!_consThreadInfo.shutdown())
        {
            // read until no more to read and then write leftover from previous burst
            selectTime = selectTime(nextTickTime);
        	
        	if (!_consPerfConfig.busyRead())
        	{
        	    selectorReadAndWrite(selectTime);
        	}
        	else
        	{
        	    busyReadAndWrite(selectTime);
        	}
        	
            /* Handle pings */
            if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
            {
                if (_pingHandler.handlePings(_channel, _error) != CodecReturnCodes.SUCCESS)
                {
                	closeChannelAndShutDown("Error handling pings: " + _error.text());
                	return;
                }
            }

            long currentTime = currentTime();            
            if (currentTime >= nextTickTime)
            {
                nextTickTime = nextTickTime(nextTickTime);

            	// only send bursts on tick boundary
            	if (_requestsSent)
            	{
            	    Service service;
            	    
            		// send item request and post bursts
                    if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
                    {
                        service = _srcDirHandler.serviceInfo();
                    }
                    else // use ETA VA Reactor for sending and receiving
                    {
                        service = _service;
                    }
                    
                    if(_tunnelStreamHandler != null)
                    {
                    	 if(_reactorChannel != null && !_tunnelStreamHandler.tunnelStreamOpenSent()) 
                         {
                         	if ( _tunnelStreamHandler.openStream(_reactorChannel, service, _errorInfo) != ReactorReturnCodes.SUCCESS)
                         	{
                         		closeChannelAndShutDown(_errorInfo.error().text());
                         		return;
                         	}
                         }
                    	 else
                    	 {
                    		/* Checks to ensure that the tunnel stream is established */
                    		if(_tunnelStreamHandler.tunnelStream() != null)
                    		{
                    			if ((ret = sendBursts(currentTicks, service)) < TransportReturnCodes.SUCCESS)
                    			{
                    				if (ret != TransportReturnCodes.NO_BUFFERS)
                    				{
                    					continue;
                    				}
                    				// not successful cases were handled in sendBursts method
                    			}
                    		}
                    	 }
                    }
                    else
                    {
                        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist() && _channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && !_consPerfConfig.convertJSON())
                    	{
                    		ret = sendBurstsJson(currentTicks, _srcDirHandlerJson.serviceId(), _srcDirHandlerJson.serviceQos());
                    	}
                    	else
                    	{
                    		ret = sendBursts(currentTicks, service);
                    	}

                    	if (ret < TransportReturnCodes.SUCCESS)
                    	{
                    		if (ret != TransportReturnCodes.NO_BUFFERS)
                    		{
                    			continue;
                    		}
                    		// not successful cases were handled in sendBursts method
                    	}
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
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            closeChannel();
        }
        else // use ETA VA Reactor for sending and receiving
        {
        	if(_tunnelStreamHandler != null)
        	{
        		_tunnelStreamHandler.closeStreams(_errorInfo);
        	}
        	
            closeReactor();
        }
        System.out.println("\nConsumerThread " + _consThreadInfo.threadId() + " exiting...");
	}

	private int initializeJsonSession() {
        // Initializes the JSON converter library.
        _jsonConverterSession.jsonConverterOptions().datadictionary(_dictionaryHandler.dictionary());
        _jsonConverterSession.jsonConverterOptions().userClosure(this);
        _jsonConverterSession.jsonConverterOptions().defaultServiceId(1);
        _jsonConverterSession.jsonConverterOptions().serviceNameToIdCallback(this);

        return _jsonConverterSession.initialize(_error);
    }

	/* Reads from a channel. */
	private void selectorReadAndWrite(long selectTime)
	{
		Set<SelectionKey> keySet = null;

		// set select time 
        try
        {
        	int selectRetVal;
        	
        	if (_consPerfConfig.ticksPerSec() > 1000) // use nanosecond timer for tickRate of greater than 1000
        	{
        	    selectTime /= 1000000;
        	}

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
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
	        Iterator<SelectionKey> iter = keySet.iterator();
	        int ret = CodecReturnCodes.SUCCESS;
	        while (iter.hasNext())
	        {
	        	SelectionKey key = iter.next();
	        	iter.remove();
	        	if(!key.isValid())
	                continue;
	    		if (key.isReadable())
	    		{
	                if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
	                {
	                    read();
	                }
	    		}
	
	    		/* flush for write file descriptor and active state */
	            if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
	            {
	        		if (key.isWritable())
	        		{
	        			ret = _channel.flush(_error);
	        			if (ret == TransportReturnCodes.SUCCESS)
	        			{
	        				removeOption(SelectionKey.OP_WRITE, _channel);
	        			}
	        		}
	            }
	        }
        }
        else // use ETA VA Reactor for sending and receiving
        {
            reactorRead(keySet);
        }
    }
	
    /* Reads from a channel with no select call. */
    private void busyReadAndWrite(long selectTime)
    {
        long pollTime, currentTime = currentTime();
        
        if (selectTime < 0)
        {
            selectTime = 0;
        }
        
        pollTime = currentTime + selectTime;
        
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            // call flush once
            _channel.flush(_error);
            
            while (currentTime <= pollTime)
            {
                read();
                
                currentTime = currentTime();
            }
            
        }
        else // use ETA VA Reactor for sending and receiving
        {
            int ret;
            while (currentTime <= pollTime)
            {
                /* read until no more to read */
                if (_reactor != null && _reactor.reactorChannel() != null)
                {
                    while (( ret =  _reactor.dispatchAll(null, _dispatchOptions, _errorInfo)) > 0 ) {}
                    if (ret == ReactorReturnCodes.FAILURE)
                    {
                        System.out.println("Reactor.ReactorChannel dispatch failed: " + ret + "(" + _errorInfo.error().text() + ")");
                        closeReactor();
                        System.exit(ReactorReturnCodes.FAILURE);
                    }
                }

                currentTime = currentTime();
            }
        }
    }
	
	/* Writes an message to the ETA channel. */
	private int writeMsg(Msg msg)
	{
	    int ret = TransportReturnCodes.SUCCESS;
	    
        if (!_consPerfConfig.useWatchlist()) // VA Reactor Watchlist not enabled
        {
            TransportBuffer msgBuf = null;
            
            if ((msgBuf = getBuffer(512, false)) == null)
            {
                return TransportReturnCodes.NO_BUFFERS;
            }
    
            _eIter.clear();
            ret = _eIter.setBufferAndRWFVersion(msgBuf, majorVersion(), minorVersion());
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
        }
        else // VA Reactor Watchlist is enabled, submit message instead of buffer
        {
            if (_reactorChannel.state() == com.refinitiv.eta.valueadd.reactor.ReactorChannel.State.READY)
            {
            	if(Objects.isNull(_tunnelStreamHandler))
            	{
            		ret = _reactorChannel.submit(msg, _submitOptions, _errorInfo);
            	}
            	else
            	{
            		ret = _tunnelStreamHandler.tunnelStream().submit(msg, _errorInfo);
            		
					if(_consPerfConfig.tunnelStreamBufsUsed())
					{
						if (_tunnelStreamHandler.tunnelStream().info(_tunnelStreamInfo, _errorInfo) == ReactorReturnCodes.SUCCESS)
						{
							_consThreadInfo.stats().tunnelStreamBufUsageStats().update(_tunnelStreamInfo.buffersUsed());
						}
					}
				}
                
                if (ret == ReactorReturnCodes.NO_BUFFERS)
                {
                    ret = TransportReturnCodes.NO_BUFFERS;
                }
            }
        }
        
        return ret;
	}

    /* Writes the content of the TransportBuffer to the ETA channel.*/
    private void write(TransportBuffer msgBuf)
    {
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            // write data to the channel
            _writeArgs.clear();
            _writeArgs.priority(WritePriorities.HIGH);
            _writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            
            if(_channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && _consPerfConfig.convertJSON())
            {
            	TransportBuffer convertedMsgBuffer = null;
            	int ret = 0;
            	
            	do 
            	{
            		if ((convertedMsgBuffer = _jsonConverterSession.convertToJsonMsg(_channel, msgBuf, _error)) == null)
            		{
            			if(_error.errorId() == TransportReturnCodes.NO_BUFFERS)
            			{
            				if ((ret = _channel.flush(_error)) < TransportReturnCodes.SUCCESS)
            				{
            					closeChannelAndShutDown("rsslFlush() failed with return code " + ret + "<" + _error.text() + ">");
            					return;
            				}
            			}
            			else
            			{
            				_channel.releaseBuffer(msgBuf, _error);
            				closeChannelAndShutDown("convertToJsonMsg(): Failed to convert RWF > JSON with error text: " + _error.text());
            				return;
            			}
            			
            		}
            	} while(_error.errorId() == TransportReturnCodes.NO_BUFFERS);
            	
            	if(Objects.nonNull(convertedMsgBuffer))
            	{
            		/* Release the original RWF buffer and point the writing buffer to the JSON buffer */
            		_channel.releaseBuffer(msgBuf, _error);
            		msgBuf = convertedMsgBuffer;
            	}
            	
            }
            
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
        else // use ETA VA Reactor for sending and receiving
        {
            // write data to the channel
            _submitOptions.writeArgs().clear();
            _submitOptions.writeArgs().priority(WritePriorities.HIGH);
            _submitOptions.writeArgs().flags(WriteFlags.DIRECT_SOCKET_WRITE);
            
            int retval;
            
            if(Objects.isNull(_tunnelStreamHandler))
            {
            	retval = _reactorChannel.submit(msgBuf, _submitOptions, _errorInfo);
            }
            else
            {
            	retval = _tunnelStreamHandler.tunnelStream().submit(msgBuf, _tunnelStreamSubmitOptions, _errorInfo);
            	
				if(_consPerfConfig.tunnelStreamBufsUsed())
				{
					if (_tunnelStreamHandler.tunnelStream().info(_tunnelStreamInfo, _errorInfo) == ReactorReturnCodes.SUCCESS)
					{
						_consThreadInfo.stats().tunnelStreamBufUsageStats().update(_tunnelStreamInfo.buffersUsed());
					}
				}
            }

            if (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
            {
                //call flush and write again until there is data in the queue
                while (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
                {
                    if(Objects.isNull(_tunnelStreamHandler))
                    {
                    	retval = _reactorChannel.submit(msgBuf, _submitOptions, _errorInfo);
                    }
                    else
                    {
                    	retval = _tunnelStreamHandler.tunnelStream().submit(msgBuf, _tunnelStreamSubmitOptions, _errorInfo);
                    	
    					if(_consPerfConfig.tunnelStreamBufsUsed())
    					{
    						if (_tunnelStreamHandler.tunnelStream().info(_tunnelStreamInfo, _errorInfo) == ReactorReturnCodes.SUCCESS)
    						{
    							_consThreadInfo.stats().tunnelStreamBufUsageStats().update(_tunnelStreamInfo.buffersUsed());
    						}
    					}
                    }
                }
            }
            else if (retval < ReactorReturnCodes.SUCCESS)
            {
                // write failed, release buffer and shut down
                if (_reactorChannel.state() != ReactorChannel.State.CLOSED)
                {
                    if(Objects.isNull(_tunnelStreamHandler))
                    {
                	_reactorChannel.releaseBuffer(msgBuf, _errorInfo);
                    }
                    else
                    {
                	_tunnelStreamHandler.tunnelStream().releaseBuffer(msgBuf, _errorInfo);
                    }
                }
                
                closeChannelAndShutDown(_errorInfo.error().text());
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
        int ret = 0;
        int cRet = 0;
        TransportBuffer origBuffer;
        Buffer decodedMsg;
        int numConverted = 0;
		
        do {

			if(_channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && !_consPerfConfig.convertJSON())
        		{
				processResponseJson(buffer);
				return;
			}
        	
        	// clear decode iterator
        	_dIter.clear();
        	_responseMsg.clear();
        	
        	if(_channel.protocolType() == Codec.JSON_PROTOCOL_TYPE)
        	{
        		origBuffer = buffer;

        		if((cRet = _jsonConverterSession.convertFromJsonMessage(_channel,
        				(numConverted == 0 ? origBuffer : null), _error)) == CodecReturnCodes.FAILURE)
        		{
        			ret = cRet;
        			closeChannelAndShutDown("Error in Json Conversion with error text: " + _error.text());
        			return;
        		}
        		
        		numConverted++;
        		
        		if (cRet == CodecReturnCodes.SUCCESS)
        		{
        			decodedMsg = _jsonConverterSession.jsonConverterState().jsonMsg().rwfMsg().encodedMsgBuffer();
        			ret = _dIter.setBufferAndRWFVersion(decodedMsg, _channel.majorVersion(), _channel.minorVersion());
        		}
    		
    			if (cRet == CodecReturnCodes.END_OF_CONTAINER)
    				break;

    			if (cRet != CodecReturnCodes.SUCCESS)
    				continue;
        	}
        	else
        	{
        		ret = _dIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion());
        	}
        	
	        if (ret != CodecReturnCodes.SUCCESS)
	        {
	        	closeChannelAndShutDown("DecodeIterator.setBufferAndRWFVersion() failed with return code:  " + CodecReturnCodes.toString(ret));
	            return;
	        }

            ret = _responseMsg.decode(_dIter);

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
        
        } while(_channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && cRet != CodecReturnCodes.END_OF_CONTAINER);
    }


	public static final String DOMAIN_STR_LOGIN = "Login";
	public static final String DOMAIN_STR_SOURCE = "Source";
	public static final String DOMAIN_STR_DICTIONARY = "Dictionary";
	public static final String DOMAIN_STR_MARKET_PRICE = "MarketPrice";

	static final String UPDATE_STR = "Update";
	static final String GENERIC_STR = "Generic";
	static final String REFRESH_STR = "Refresh";
	static final String REQUEST_STR = "Request";
	static final String POST_STR = "Post";
	static final String STATUS_STR = "Status";
	static final String CLOSE_STR = "Close";
	static final String ACK_STR = "Ack";
	static final String PING_STR = "Ping";
	static final String PONG_STR = "Pong";

	static final String JSON_PONG_MESSAGE = "{\"Type\":\"Pong\"}";

	int sendJsonMessage(Channel channel, TransportBuffer buffer, int wrtFlags, Error error)
	{
		int ret = TransportReturnCodes.SUCCESS;
		WriteArgs writeArgs = TransportFactory.createWriteArgs();
		writeArgs.flags(wrtFlags);

		ret = channel.write(buffer, writeArgs, error);

		while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
		{
			if ((ret = channel.flush(error)) < TransportReturnCodes.SUCCESS)
			{
				error.text("rsslFlush() failed with error text: " + error.text());
				return ret;
			}

			ret = channel.write(buffer, writeArgs, error);
		}

		if(ret != TransportReturnCodes.SUCCESS)
		{
			error.text("rsslWrite() failed with error text: " + error.text());
		}

		return ret;
	}

    /** Parse JSON message. */
	private JsonNode parseJsonMessage(TransportBuffer buffer)
	{
		try {
			ByteBuffer data = buffer.data();
			_jsonInputStream.setByteBuffer(data, buffer.dataStartPosition(), data.limit());
			return _jsonObjectMapper.readTree(_jsonInputStream);
		} catch (IOException e) {
			return null;
		}
	}

    /** Parse JSON message. */
	private JsonNode parseJsonMessage(JsonNode jsonNode)
	{
		_responseMsg.clear();

		JsonNode typeNode = jsonNode.path("Type");
		if (typeNode == null || typeNode.textValue() == null)
		{
			System.out.println("No MsgType.");
			return null;
		}

		switch (typeNode.textValue()) {
		case UPDATE_STR:
			_responseMsg.msgClass(MsgClasses.UPDATE);
			break;
		case GENERIC_STR:
			_responseMsg.msgClass(MsgClasses.GENERIC);
			break;
		case REFRESH_STR:
			_responseMsg.msgClass(MsgClasses.REFRESH);
			break;
		case REQUEST_STR:
			_responseMsg.msgClass(MsgClasses.REQUEST);
			break;
		case POST_STR:
			_responseMsg.msgClass(MsgClasses.POST);
			break;
		case STATUS_STR:
			_responseMsg.msgClass(MsgClasses.STATUS);
			break;
		case CLOSE_STR:
			_responseMsg.msgClass(MsgClasses.CLOSE);
			break;
		case ACK_STR:
			_responseMsg.msgClass(MsgClasses.ACK);
			break;
		case PING_STR:
		{
			TransportBuffer buffer = _channel.getBuffer(JSON_PONG_MESSAGE.length(), false, _error);
			if(Objects.nonNull(buffer))
			{
				buffer.data().put(JSON_PONG_MESSAGE.getBytes());
	
				int ret = sendJsonMessage(_channel, buffer, WriteFlags.DIRECT_SOCKET_WRITE, _error);
				if( ret != TransportReturnCodes.FAILURE )
				{
					ret = TransportReturnCodes.READ_PING;
				}
			}
			else
			{
				int ret = TransportReturnCodes.FAILURE;
				_error.channel(_channel);
				_error.errorId(CodecReturnCodes.FAILURE);
				_error.sysError(0);
				_error.text("Failed to get buffer for Ping response : " + _error.text());
			}
			return null;
		}
		case PONG_STR:
			return null;
		default:
			System.out.println("Bad MsgType: " + typeNode.textValue());
			return null;
		}

		JsonNode domainNode = jsonNode.path("Domain");
		if (domainNode != null && domainNode.textValue() != null)
		{
			switch (domainNode.textValue()) {
			case DOMAIN_STR_LOGIN:
				_responseMsg.domainType(DomainTypes.LOGIN);
				break;
			case DOMAIN_STR_SOURCE:
				_responseMsg.domainType(DomainTypes.SOURCE);
				break;
			case DOMAIN_STR_DICTIONARY:
				_responseMsg.domainType(DomainTypes.DICTIONARY);
				break;
			case DOMAIN_STR_MARKET_PRICE:
				_responseMsg.domainType(DomainTypes.MARKET_PRICE);
				break;
			default:
				System.out.println("Bad Domain: " + domainNode.textValue());
				return null;
			}
		}
		else
		{
			_responseMsg.domainType(DomainTypes.MARKET_PRICE);
		}

		return jsonNode;
	}

    /** Process JSON message. */
	private void processJsonMessage(JsonNode jsonNode)
	{
		if (parseJsonMessage(jsonNode) != null)
		{
			switch (_responseMsg.domainType())
			{
				case DomainTypes.LOGIN:
					processLoginRespJson(jsonNode);
					break;
				case DomainTypes.SOURCE:
					processSourceDirectoryRespJson(jsonNode);
					break;
				case DomainTypes.DICTIONARY:
					processDictionaryRespJson(_responseMsg, jsonNode);
					break;
				case DomainTypes.MARKET_PRICE:
					processMarketPriceRespJson(_responseMsg, jsonNode);
					break;
				default:
					System.out.println("Unhandled Domain Type: " + _responseMsg.domainType());
					break;
			}
		}
	}

    /** Process transport response. */
	private void processResponseJson(TransportBuffer buffer)
	{
		JsonNode root = parseJsonMessage(buffer);
		if (root.isObject())
		{
			processJsonMessage(root);
		}
		else if (root.isArray())
		{
			for (final JsonNode node : root)
			{
				if (root.isObject())
				{
					System.out.println("JSON not Object");
					continue;
				}
				processJsonMessage(node);
			}
		}
		else
		{
			closeChannelAndShutDown("Error in JSON parsing: JSON not Object not Array.");
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
    
    /* Process login response JSON. */
    private void processLoginRespJson(JsonNode jsonNode)
    {
        int ret = _loginHandlerJson.processResponse(_responseMsg, jsonNode, _error);
        if (ret != CodecReturnCodes.SUCCESS)
            if (ret != CodecReturnCodes.SUCCESS)
            {
            	closeChannelAndShutDown(_error.text());
        		return;
            }

        if (_responseMsg.msgClass() == MsgClasses.REFRESH)
        {
        	if (_consPerfConfig.postsPerSec() > 0 &&
        			(!_loginHandlerJson.refreshInfo().checkHasFeatures() ||
        					!_loginHandlerJson.refreshInfo().features().checkHasSupportPost() ||
        					_loginHandlerJson.refreshInfo().features().supportOMMPost() == 0))
        	{
        		closeChannelAndShutDown("Provider for this connection does not support posting.");
        		return;
        	}
        }

        //Handle login states
        ConsumerLoginState loginState = _loginHandlerJson.loginState();
        if (loginState == ConsumerLoginState.OK_SOLICITED)
        {
            _srcDirHandlerJson.serviceName(_consPerfConfig.serviceName());
            TransportBuffer buffer = _srcDirHandlerJson.getRequest(_channel, _error,
                                                                   _jsonObjectMapper, _jsonGenerator, _jsonOutputStream);
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
            if (isDictionariesLoaded())
            {
                _consThreadInfo.dictionary(_dictionaryHandler.dictionary());
                System.out.println("Dictionary ready, requesting item(s)...\n");

                _requestsSent = true;
            }
            else // dictionaries not loaded yet
            {
                sendDictionaryRequests(_channel, _srcDirHandler.serviceInfo());
            }
        }
        else
        {
            // service not up or
            // previously up service went down
            _requestsSent = false;

            System.out.println("Requested service '" + _consPerfConfig.serviceName() + "' not up. Waiting for service to be up...");
        }
    }

    //process source directory response JSON.
    private void processSourceDirectoryRespJson(JsonNode jsonNode)
    {
        int ret = _srcDirHandlerJson.processResponse(_channel, _responseMsg, jsonNode, _error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
        	closeChannelAndShutDown(_error.text());
    		return;
        }

        if (_srcDirHandlerJson.isRequestedServiceUp())
        {
            if (isDictionariesLoaded())
            {
                _consThreadInfo.dictionary(_dictionaryHandler.dictionary());
                System.out.println("Dictionary ready, requesting item(s)...\n");

                _requestsSent = true;
            }
            else // dictionaries not loaded yet
            {
                closeChannelAndShutDown("JSON dictionary request not supported");
                return;
            }
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
    }
    

    private void sendWatchlistDictionaryRequests(ReactorChannel reactorChannel, Service service)
    {
        int ret;
        
        if (_requestsSent)
            return;
        
        _dictionaryRequest.applyStreaming();
        _dictionaryRequest.verbosity(Dictionary.VerbosityValues.NORMAL);
        _dictionaryRequest.serviceId(service.serviceId());
        
        _dictionaryRequest.streamId(3);
        _dictionaryRequest.dictionaryName(_fieldDictionaryName);

        ret = reactorChannel.submit(_dictionaryRequest, _submitOptions, _errorInfo);
        if (ret < ReactorReturnCodes.SUCCESS && ret != ReactorReturnCodes.NO_BUFFERS)
        {
            closeChannelAndShutDown("Sending field dictionary request failed");
            return;
        }

        _dictionaryRequest.streamId(4);
        _dictionaryRequest.dictionaryName(_enumTypeDictionaryName);

        ret = reactorChannel.submit(_dictionaryRequest, _submitOptions, _errorInfo);
        if (ret < ReactorReturnCodes.SUCCESS && ret != ReactorReturnCodes.NO_BUFFERS)
        {
            closeChannelAndShutDown("Sending enum type dictionary request failed");
            return;
        }
    }
    
    /* Process dictionary response. */
    private void processDictionaryResp(Msg responseMsg, DecodeIterator dIter)
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
            
            _requestsSent = true;
        }
    }
    
    /* Process dictionary response JSON. */
    private void processDictionaryRespJson(Msg responseMsg, JsonNode jsonNode)
    {
        closeChannelAndShutDown("Processing dictionary response JSON not implemented.");
    }

    //process market price response.
    private void processMarketPriceResp(Msg responseMsg, DecodeIterator dIter)
    {
    	int ret = CodecReturnCodes.SUCCESS;
    	int msgClass = responseMsg.msgClass();

        if (_consThreadInfo.conversionTimeHandled()) {
            do {
                TransportBuffer buffer = _jsonConverterSession.convertToJsonMsg(_channel, _responseMsg.encodedMsgBuffer(), _error);
                if (buffer == null) {
                    if (_error.errorId() == TransportReturnCodes.NO_BUFFERS) {
                        if ((ret = _channel.flush(_error)) < TransportReturnCodes.SUCCESS) {
                            closeChannelAndShutDown("rsslFlush() failed with return code " + ret + "<" + _error.text() + ">");
                            return;
                        }
                    } else {
                        closeChannelAndShutDown("convertToJsonMsg(): Failed to convert RWF > JSON with error text: " + _error.text());
                        return;
                    }

                } else {
                    _channel.releaseBuffer(buffer, _error);
                }
            } while (_error.errorId() == TransportReturnCodes.NO_BUFFERS);
        }
    	
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
                                _consThreadInfo.stats().steadyStateLatencyTime(_consThreadInfo.stats().imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
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
                                    _consThreadInfo.stats().steadyStateLatencyTime(_consThreadInfo.stats().imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
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
    
	private boolean jsonIsRefreshStateFinal(JsonNode jsonNode)
	{
		JsonNode stateNode = jsonNode.get("State");
		JsonNode streamNode = stateNode.get("Stream");
		return (!streamNode.textValue().equals("Open"));
	}

	private boolean jsonRefreshMsgDataStateOK(JsonNode jsonNode)
	{
		JsonNode stateNode = jsonNode.get("State");
		JsonNode dataNode = stateNode.get("Data");
		return (dataNode.textValue().equals("Ok"));
	}

	private boolean jsonIsRefreshComplete(JsonNode jsonNode)
	{
		JsonNode node = jsonNode.get("Complete");
		if(node == null) {
			return true;
		}
		else if(node.isBoolean()) {
			return node.booleanValue();
		}
		else if(node.isTextual() && node.asText().equals("false")) {
			return false;
		}
		else if(node.isInt() && node.asInt() == 0) {
			return false;
		}
		else {
			return true;
		}
	}

	private int jsonStreamId(JsonNode jsonNode)
	{
		int id = 0;
		JsonNode node = jsonNode.get("ID");
		if(node.isInt()) {
			id = node.asInt();
		}

		return id;
	}

	private String streamStateToString(JsonNode jsonNode)
	{
		JsonNode stateNode = jsonNode.get("State");
		return stateNode.toString();
	}

	private String msgKeyNameToString(JsonNode jsonNode)
	{
		JsonNode keyNode = jsonNode.get("Key");
		return keyNode.toString();
	}

	//process market price response JSON.
	private void processMarketPriceRespJson(Msg responseMsg, JsonNode jsonNode)
	{
		int ret = CodecReturnCodes.SUCCESS;
		int msgClass = responseMsg.msgClass();
		int streamId = jsonStreamId(jsonNode);

		switch (msgClass)
		{
		case MsgClasses.REFRESH:
			_consThreadInfo.stats().refreshCount().increment();

			//If we are still retrieving images, check if this item still needs one.
			if ((ret = _marketPriceDecoderJson.decodeUpdate(jsonNode, responseMsg, _consThreadInfo)) != CodecReturnCodes.SUCCESS)
			{
				closeChannelAndShutDown("Decoding failure: " + ret);
				return;
			}
			if (_consThreadInfo.stats().imageRetrievalEndTime() == 0)
			{
				if (jsonIsRefreshStateFinal(jsonNode))
				{
					closeChannelAndShutDown("Received unexpected final state in refresh for item: " +
						streamStateToString(jsonNode) +
						msgKeyNameToString(jsonNode));
					return;
				}

				if (jsonIsRefreshComplete(jsonNode))
				{
					if (_consPerfConfig.primeJVM() == false)
					{
						if (jsonRefreshMsgDataStateOK(jsonNode))
						{
							_itemRequestList[streamId].requestState(ItemRequestState.HAS_REFRESH);
							_consThreadInfo.stats().refreshCompleteCount().increment();
							if (_consThreadInfo.stats().refreshCompleteCount().getTotal() == (_requestListSize - ITEM_STREAM_ID_START))
							{
								_consThreadInfo.stats().imageRetrievalEndTime(System.nanoTime());
								_consThreadInfo.stats().steadyStateLatencyTime(_consThreadInfo.stats().imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
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
							_itemRequestList[streamId].requestState(ItemRequestState.NOT_REQUESTED);
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
							if (jsonRefreshMsgDataStateOK(jsonNode))
							{
								_itemRequestList[streamId].requestState(ItemRequestState.HAS_REFRESH);
								_consThreadInfo.stats().refreshCompleteCount().increment();
								if (_consThreadInfo.stats().refreshCompleteCount().getTotal() == (_requestListSize - ITEM_STREAM_ID_START))
								{
									_consThreadInfo.stats().imageRetrievalEndTime(System.nanoTime());
									_consThreadInfo.stats().steadyStateLatencyTime(_consThreadInfo.stats().imageRetrievalEndTime() + _consPerfConfig.delaySteadyStateCalc() * 1000000L);
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
			if ((ret = _marketPriceDecoderJson.decodeUpdate(jsonNode, responseMsg, _consThreadInfo)) != CodecReturnCodes.SUCCESS)
			{
				closeChannelAndShutDown("Decoding failure: " + ret);
				return;
			}
			break;
		case MsgClasses.GENERIC:
			_consThreadInfo.stats().genMsgRecvCount().increment();
			if ((ret = _marketPriceDecoderJson.decodeUpdate(jsonNode, responseMsg, _consThreadInfo)) != CodecReturnCodes.SUCCESS)
			{
				closeChannelAndShutDown("Decoding failure: " + ret);
				return;
			}
			break;
		case MsgClasses.STATUS:
			_consThreadInfo.stats().statusCount().increment();

			if (jsonIsRefreshStateFinal(jsonNode))
			{
				closeChannelAndShutDown("Received unexpected final state for item: " +
					streamStateToString(jsonNode) +
					_itemRequestList[streamId].itemName());
				return;
			}

			break;
		default:
			break;
		}
	}

    //sends a burst of item requests.
    private int sendItemRequestBurst(int itemBurstCount, Service service)
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
    		
            if ((ret = writeMsg(_requestMsg)) != TransportReturnCodes.SUCCESS)
            {
                return ret;
            }

    		//request has been made.
    		itemRequest.requestState(ItemRequestState.WAITING_FOR_REFRESH);

    		_requestListIndex++;
    		_consThreadInfo.stats().requestCount().increment();
    	}

    	return TransportReturnCodes.SUCCESS;
    }
    
	//sends a burst of item requests.
	private int sendItemRequestBurstJson(int itemBurstCount, int serviceId, Qos qos)
	{
		int ret;

		for (int i = 0; i < itemBurstCount; ++i)
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

			/* get a buffer for the login request */
			TransportBuffer msgBuf = _channel.getBuffer(512 /*TRANSPORT_BUFFER_SIZE_REQUEST*/, false, _error);
			if (msgBuf == null)
				return TransportReturnCodes.NO_BUFFERS;

			_jsonOutputStream.setByteBuffer(msgBuf.data());

			ObjectMapper mapper = _jsonObjectMapper;
			JsonGenerator generator = _jsonGenerator;

			// [{"ID":6,"Type":"Request","KeyInUpdates":false,"Qos":{"Timeliness":"Realtime","Rate":"TickByTick"},"Key":{"Service":1,"Name":"RDT1"}}]
			ObjectNode json = mapper.createObjectNode()
				.put("ID", Integer.valueOf(itemRequest.itemInfo().streamId()))
				.put("Type", "Request")
				.put("KeyInUpdates", Boolean.FALSE)
				.set("Qos", mapper.createObjectNode()
					.put("Timeliness", "Realtime")
					.put("Rate", "TickByTick"));

			json.set("Key", mapper.createObjectNode()
				.put("Service", Integer.valueOf(serviceId))
				.put("Name", itemRequest.msgKey().name().toString()));
			try
			{
				mapper.writeTree(generator, json);
			}
			catch (IOException e)
			{
				return TransportReturnCodes.FAILURE;
			}

			write(msgBuf);

			//request has been made.
			itemRequest.requestState(ItemRequestState.WAITING_FOR_REFRESH);

			_requestListIndex++;
			_consThreadInfo.stats().requestCount().increment();
		}

		return TransportReturnCodes.SUCCESS;
	}

    //sends a burst of post messages. 
	private int sendPostBurst(int itemBurstCount)
	{
		int ret;
		long encodeStartTime;
		int latencyUpdateNumber;

		latencyUpdateNumber = (_consPerfConfig.latencyPostsPerSec() > 0) ?
				_postLatencyRandomArray.next() : -1; 

		for(int i = 0; i < itemBurstCount; ++i)
		{
			ItemRequest postItem = nextPostItem();

			if (latencyUpdateNumber == i)
				encodeStartTime = System.nanoTime()/1000;
			else
				encodeStartTime = 0;

	        if (!_consPerfConfig.useWatchlist()) // VA Reactor Watchlist not enabled
	        {
	        	Channel channel = _channel;
                if (_consPerfConfig.useReactor()) // use VA Reactor 
                {
                    channel = _reactorChannel.channel();
                }
                
                int bufLen = _itemEncoder.estimateItemPostBufferLength(postItem.itemInfo(), channel.protocolType());
                
                TransportBuffer msgBuf = null;
                if ((msgBuf = getBuffer(bufLen, false)) == null)
                {
                    return TransportReturnCodes.NO_BUFFERS;
                }
                
    			if ((ret = _itemEncoder.encodeItemPost(channel, postItem.itemInfo(), msgBuf, _postUserInfo, encodeStartTime)) != CodecReturnCodes.SUCCESS)
        		{
    				System.out.printf("encodeItemPost() failed: %d.\n", ret);
    				return ret;
    			}
    
    			write(msgBuf);
	        }
	        else // VA Reactor Watchlist is enabled, submit message instead of buffer
	        {
                // create properly encoded post message
	            if (_reactorChannel.state() == ReactorChannel.State.UP || _reactorChannel.state() == ReactorChannel.State.READY)
	            {
                    _postMsg.clear();
                    _postBuffer.data().clear();
                    if ((ret = _itemEncoder.createItemPost(_reactorChannel.channel(), postItem.itemInfo(), _postMsg, _postBuffer, _postUserInfo, encodeStartTime)) != CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("createItemPost() failed: %d.\n", ret);
                        return ret;
                    }
    
                    writeMsg(_postMsg);
	            }
	        }

			_consThreadInfo.stats().postSentCount().increment();
		}

		return TransportReturnCodes.SUCCESS;
	}

	private boolean isSendPostBurstJson = true;

	// sends a burst of post messages JSON.
	private int sendPostBurstJson(int itemBurstCount)
	{
		// to show the message only once
		if (isSendPostBurstJson)
		{
			System.out.printf("It is not supported to send JSON generic messages.\n");
			isSendPostBurstJson = false;
		}
	    	return TransportReturnCodes.SUCCESS;
	}

	// sends a burst of generic messages.
	private int sendGenMsgBurst(int itemBurstCount) 
	{
		int ret;
		long encodeStartTime;
		int latencyGenMsgNumber;

		latencyGenMsgNumber = (_consPerfConfig.latencyGenMsgsPerSec() > 0) ? _genMsgLatencyRandomArray.next() : -1;

		for (int i = 0; i < itemBurstCount; ++i) 
		{
			ItemRequest genMsgItem = nextGenMsgItem();

			if (latencyGenMsgNumber == i)
            {
                _consThreadInfo.stats().latencyGenMsgSentCount().increment();
                encodeStartTime = System.nanoTime() / 1000;
            }
            else
                encodeStartTime = 0;
			
            if (!_consPerfConfig.useWatchlist()) // VA Reactor Watchlist not enabled
            {
            	Channel channel = _channel;
    			if (_consPerfConfig.useReactor()) // use VA Reactor 
    			{
    			    channel = _reactorChannel.channel();
    			}
    			
    			int bufLen = _itemEncoder.estimateItemGenMsgBufferLength(genMsgItem.itemInfo(), channel.protocolType());
    
    	        TransportBuffer msgBuf = null;
    			if ((msgBuf = getBuffer(bufLen, false)) == null) 
    			{
    				return TransportReturnCodes.NO_BUFFERS;
    			}
    			
    			if ((ret = _itemEncoder.encodeItemGenMsg(channel, genMsgItem.itemInfo(), msgBuf, encodeStartTime)) != CodecReturnCodes.SUCCESS) 
    			{
    				System.out.printf("encodeItemGenMsg() failed: %d.\n", ret);
    				return ret;
    			}
    
    			write(msgBuf);
            }
            else // VA Reactor Watchlist is enabled, submit message instead of buffer
            {
                // create properly encoded generic message
                if (_reactorChannel.state() == ReactorChannel.State.UP || _reactorChannel.state() == ReactorChannel.State.READY)
                {
                    _genericMsg.clear();
                    _genericBuffer.data().clear();
                    if ((ret = _itemEncoder.createItemGenMsg(_reactorChannel.channel(), genMsgItem.itemInfo(), _genericMsg, _genericBuffer, encodeStartTime)) != CodecReturnCodes.SUCCESS)
                    {
                        System.out.printf("createItemPost() failed: %d.\n", ret);
                        return ret;
                    }
    
                    writeMsg(_genericMsg);
                }
            }
            
			_consThreadInfo.stats().genMsgSentCount().increment();
		}

    	return TransportReturnCodes.SUCCESS;
	}

	private boolean isSendGenMsgBurstJson = true;

	// sends a burst of generic messages JSON.
	private int sendGenMsgBurstJson(int itemBurstCount)
	{
		// to show the message only once
		if (isSendGenMsgBurstJson)
		{
			System.out.printf("It is not supported to send JSON generic messages.\n");
			isSendGenMsgBurstJson = false;
		}
	    	return TransportReturnCodes.SUCCESS;
	}

    //retrieves next post item information to send.
	private ItemRequest nextPostItem()
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
	private ItemRequest nextGenMsgItem() {
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
    private boolean isDictionariesLoaded()
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
    private void printEstimatedPostMsgSizes(Channel channel)
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
                    int bufLen = _itemEncoder.estimateItemPostBufferLength(_itemInfo, channel.protocolType());
                    testBuffer = channel.getBuffer(bufLen, false, _error);
                    if (testBuffer == null)
                    {
                        closeChannelAndShutDown("printEstimatedPostMsgSizes: getBuffer() failed");
                        return;
                    }
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
	private void printEstimatedGenMsgSizes(Channel channel) 
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
					int bufLen = _itemEncoder.estimateItemGenMsgBufferLength(_itemInfo, channel.protocolType());
					testBuffer = channel.getBuffer(bufLen, false, _error);
                    if (testBuffer == null)
                    {
                        closeChannelAndShutDown("printEstimatedGenMsgSizes: getBuffer() failed");
                        return;
                    }
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
	private int sendBursts(int currentTicks, Service service) 
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

			if ((ret = sendItemRequestBurst(requestBurstCount, service)) < TransportReturnCodes.SUCCESS) 
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
				if ((ret = sendPostBurst((int) (_postsPerTick + ((currentTicks < _postsPerTickRemainder) ? 1 : 0)))) < TransportReturnCodes.SUCCESS) 
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
				if ((ret = sendGenMsgBurst((int) (_genMsgsPerTick + ((currentTicks < _genMsgsPerTickRemainder) ? 1 : 0)))) < TransportReturnCodes.SUCCESS) 
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

	// send item requests, post bursts and or generic msg bursts.
	private int sendBurstsJson(int currentTicks, int serviceId, Qos qos)
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

			if ((ret = sendItemRequestBurstJson(requestBurstCount, serviceId, qos)) < TransportReturnCodes.SUCCESS)
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
				if ((ret = sendPostBurstJson((int)(_postsPerTick + ((currentTicks < _postsPerTickRemainder) ? 1 : 0)))) < TransportReturnCodes.SUCCESS)
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
				if ((ret = sendGenMsgBurstJson((int)(_genMsgsPerTick + ((currentTicks < _genMsgsPerTickRemainder) ? 1 : 0)))) < TransportReturnCodes.SUCCESS)
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

    private void closeChannelAndShutDown(String text)
    {
		System.out.println(text);
		_shutdownCallback.shutdown();
		_consThreadInfo.shutdownAck(true);
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            closeChannel();
        }
        else // use ETA VA Reactor for sending and receiving
        {
            closeReactor();
        }
    }
    
    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        switch(event.eventType())
        {
            case ReactorChannelEventTypes.CHANNEL_UP:
            {
                _reactorChannel = event.reactorChannel();
                _consThreadInfo.channel(_reactorChannel.channel());
                System.out.println("Connected ");

                // set the high water mark if configured
                if (_consPerfConfig.highWaterMark() > 0)
                {
                    if (_reactorChannel.ioctl(IoctlCodes.HIGH_WATER_MARK, _consPerfConfig.highWaterMark(), _errorInfo) != TransportReturnCodes.SUCCESS)
                    {
                        closeChannelAndShutDown("Channel.ioctl() failed");
                        return ReactorCallbackReturnCodes.FAILURE;
                    }
                }

                // register selector with channel event's reactorChannel
                try
                {
                	if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
                	{
                		for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); ++i)
                		{
                			event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).register(_selector,  
                																									SelectionKey.OP_READ, 
                																									event.reactorChannel());	
                		}
                	}
                	else
                	{
                		event.reactorChannel().selectableChannel().register(_selector,
                                                                        SelectionKey.OP_READ,
                                                                        event.reactorChannel());
                	}
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                
                // get and print the channel info
                if (_reactorChannel.info(_reactorChannnelInfo, _errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    closeChannelAndShutDown("Channel.info() failed");
                    return ReactorCallbackReturnCodes.FAILURE;
                }

                System.out.printf("Channel active. " + _reactorChannnelInfo.channelInfo().toString() + "\n");

                break;
            }
            case ReactorChannelEventTypes.FD_CHANGE:
            {
                System.out.println("Channel Change - Old Channel: "
                        + event.reactorChannel().oldSelectableChannel() + " New Channel: "
                        + event.reactorChannel().selectableChannel());
                
                // cancel old reactorChannel select
                SelectionKey key;
                try
                {
                	if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
                	{
                		for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().size(); ++i)
                		{
                			if(!event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(
                					event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i)))
                			{
                				key = event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i).keyFor(_selector);
                				
                				if(key != null)
                					key.cancel();
                			}
                		}
                		
                		for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); ++i)
                		{
                			event.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i).register(_selector, 
                																									SelectionKey.OP_READ,
                																									event.reactorChannel());
                		}
                	}
                	else
                	{
		                key = event.reactorChannel().oldSelectableChannel().keyFor(_selector);
		                if (key != null)
		                    key.cancel();
		    
		                // register selector with channel event's new reactorChannel
	                
	                    event.reactorChannel().selectableChannel().register(_selector,
	                                                                    SelectionKey.OP_READ,
	                                                                    event.reactorChannel());
                	}
                }
                catch (Exception e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_READY:
            {
                printEstimatedPostMsgSizes(event.reactorChannel().channel());
                printEstimatedGenMsgSizes(event.reactorChannel().channel());
                
                if (isRequestedServiceUp())
                {
                    // dictionaries were loaded at initialization. send item requests and post
                    // messages only after dictionaries are loaded.
                    if (isDictionariesLoaded())
                    {
                        _consThreadInfo.dictionary(_dictionaryHandler.dictionary());
                        System.out.println("Dictionary ready, requesting item(s)...\n");

                        _requestsSent = true;
                    }
                    else // dictionaries not loaded yet
                    {
                        // request dictionaries if watchlist enabled
                        if (_consPerfConfig.useWatchlist())
                        {
                            sendWatchlistDictionaryRequests(_reactorChannel, _service);
                        }
                    }
                }
                else
                {
                    // service not up or
                    // previously up service went down
                    _requestsSent = false;

                    System.out.println("Requested service '" + _consPerfConfig.serviceName() + "' not up. Waiting for service to be up...");
                }
                
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
            {
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down reconnecting: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down reconnecting");
    
                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");
                            
                // allow Reactor to perform connection recovery
                
                // unregister selectableChannel from Selector
                SelectionKey key;
                if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
            	{
            		for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().size(); ++i)
            		{
            			key = event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i).keyFor(_selector);
            				
        				if(key != null)
        					key.cancel();
            		}
            	} 
                else
            	{
	                if (event.reactorChannel().selectableChannel() != null)
	                {
	                    key = event.reactorChannel().selectableChannel().keyFor(_selector);
	                    if (key != null)
	                        key.cancel();
	                }
            	}
                
                // only allow one connect
                if (_service != null)
                {
                    _shutdownCallback.shutdown();
                    _consThreadInfo.shutdownAck(true);
                }

                break;
            }
            case ReactorChannelEventTypes.CHANNEL_DOWN:
            {                
                if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down");

                if (event.errorInfo() != null && event.errorInfo().error().text() != null)
                    System.out.println("    Error text: " + event.errorInfo().error().text() + "\n");

                // unregister selectableChannel from Selector
                SelectionKey key;
                if(event.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
            	{
            		for(int i = 0; i < event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().size(); ++i)
            		{
            			key = event.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i).keyFor(_selector);
            				
        				if(key != null)
        					key.cancel();
            		}
            	} 
                else
            	{
	                if (event.reactorChannel().selectableChannel() != null)
	                {
	                    key = event.reactorChannel().selectableChannel().keyFor(_selector);
	                    if (key != null)
	                        key.cancel();
	                }
            	}

                _shutdownCallback.shutdown();
                _consThreadInfo.shutdownAck(true);

                break;
            }
            case ReactorChannelEventTypes.WARNING:
                System.out.println("Received ReactorChannel WARNING event\n");
                break;
            default:
            {
                System.out.println("Unknown channel event!\n");
                return ReactorCallbackReturnCodes.SUCCESS;
            }
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int defaultMsgCallback(ReactorMsgEvent event)
    {
        Msg msg = event.msg();
        if (msg.domainType() == DomainTypes.MARKET_PRICE) {
            if (msg.encodedDataBody() != null && msg.encodedDataBody().data() != null) {
                _dIter.clear();
                _dIter.setBufferAndRWFVersion(msg.encodedDataBody(), _reactorChannel.majorVersion(), _reactorChannel.minorVersion());
            }
            processMarketPriceResp(msg, _dIter);
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }
    
    public int handleTunnelStreamMsgCallback(Msg msg, ReactorChannel reactorChannel)
    {
    	_dIter.clear();
        _dIter.setBufferAndRWFVersion(msg.encodedDataBody(), reactorChannel.majorVersion(), reactorChannel.minorVersion());

        processMarketPriceResp(msg, _dIter);
    	
    	return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        LoginMsg loginMsg = event.rdmLoginMsg();
        
        switch (loginMsg.rdmMsgType())
        {
            case REFRESH:
                LoginRefresh loginRefresh = (LoginRefresh)loginMsg;
                System.out.println("Received Login Response for Username: " + loginRefresh.userName());
                System.out.println(loginRefresh.toString());
                
                State state = loginRefresh.state();
                if (state.streamState() != StreamStates.OPEN && state.dataState() != DataStates.OK)
                {
                    closeChannelAndShutDown("Invalid login state : " + state);
                }

                if (_consPerfConfig.postsPerSec() > 0 &&
                    (!loginRefresh.checkHasFeatures() ||
                     !loginRefresh.features().checkHasSupportPost() ||
                     loginRefresh.features().supportOMMPost() == 0))
                {
                    closeChannelAndShutDown("Provider for this connection does not support posting.");
                }
                break;
            default:
                break;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        DirectoryMsg directoryMsg = event.rdmDirectoryMsg();
        
        switch (directoryMsg.rdmMsgType())
        {
            case REFRESH:
                DirectoryRefresh directoryRefresh = (DirectoryRefresh)directoryMsg;
                System.out.println("Received Source Directory Refresh");
                System.out.println(directoryRefresh.toString());

               for (Service rdmService : directoryRefresh.serviceList())
                {
                    if(rdmService.info().serviceName().toString() != null)
                    {
                        System.out.println("Received serviceName: " + rdmService.info().serviceName());
                    }

                    // cache service requested by the application
                    if (rdmService.info().serviceName().toString().equals(_consPerfConfig.serviceName()))
                    {
                        _service = DirectoryMsgFactory.createService();
                        rdmService.copy(_service);
                    }
                }
               
                break;
            case UPDATE:
                DirectoryUpdate directoryUpdate = (DirectoryUpdate)directoryMsg;
                System.out.println("Received Source Directory Update");
                System.out.println(directoryUpdate.toString());

               for (Service rdmService : directoryUpdate.serviceList())
                {
                    if(rdmService.info().serviceName().toString() != null)
                    {
                        System.out.println("Received serviceName: " + rdmService.info().serviceName());

                        // cache service requested by the application
                        if (rdmService.info().serviceName().toString().equals(_consPerfConfig.serviceName()))
                        {
                            _service = DirectoryMsgFactory.createService();
                            rdmService.copy(_service);
                        }
                    }
                }
               
                if (isRequestedServiceUp())
                {
                    // dictionaries were loaded at initialization. send item requests and post
                    // messages only after dictionaries are loaded.
                    if (isDictionariesLoaded())
                    {
                        _consThreadInfo.dictionary(_dictionaryHandler.dictionary());
                        System.out.println("Dictionary ready, requesting item(s)...\n");

                        _requestsSent = true;
                    }
                }
                else
                {
                    // service not up or
                    // previously up service went down
                    _requestsSent = false;

                    System.out.println("Requested service '" + _consPerfConfig.serviceName() + "' not up. Waiting for service to be up...");
                }
                break;
            default:
                break;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }
    
    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        Msg msg = event.msg();
        
        _dIter.clear();
        
        _dIter.setBufferAndRWFVersion(event.msg().encodedDataBody(), event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion());
            
        processDictionaryResp(msg, _dIter);
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }    
    
    private boolean isRequestedServiceUp()
    {
        
        return  _service != null &&
                _service.checkHasState() &&
                _service.state().checkHasAcceptingRequests() &&
                _service.state().acceptingRequests() == 1 &&
                _service.state().serviceState() == 1;
    }

    private void reactorRead(Set<SelectionKey> keySet)
    {
        int ret;
        
        /* read until no more to read */
        while ((ret = _reactor.dispatchAll(keySet, _dispatchOptions, _errorInfo)) > 0) {}
        if (ret == ReactorReturnCodes.FAILURE)
        {
           System.out.println("ReactorChannel dispatch failed: " + ret + "(" + _errorInfo.error().text() + ")");
           closeReactor();
           System.exit(ReactorReturnCodes.FAILURE);
        }
    }

    // close Reactor used by this thread
    private void closeReactor()
    {
        _reactor.shutdown(_errorInfo);
    }
    
    private TransportBuffer getBuffer(int length, boolean packedBuffer)
    {
        TransportBuffer msgBuf = null;
        
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            msgBuf = _channel.getBuffer(length, packedBuffer, _error);
        }
        else // use ETA VA Reactor for sending and receiving
        {
            if (_reactorChannel != null && _reactorChannel.state() == ReactorChannel.State.READY)
            {
            	if(Objects.isNull(_tunnelStreamHandler))
            	{
            		msgBuf = _reactorChannel.getBuffer(length, packedBuffer, _errorInfo);
            	}
            	else
            	{
            		msgBuf = _tunnelStreamHandler.tunnelStream().getBuffer(length, _errorInfo);
            	}
            }
        }
        
        return msgBuf;        
    }

    private int majorVersion()
    {
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            return _channel.majorVersion();
        }
        else // use ETA VA Reactor for sending and receiving
        {
            return _reactorChannel.majorVersion();
        }
    }

    private int minorVersion()
    {
        if (!_consPerfConfig.useReactor() && !_consPerfConfig.useWatchlist()) // use ETA Channel for sending and receiving
        {
            return _channel.minorVersion();
        }
        else // use ETA VA Reactor for sending and receiving
        {
            return _reactorChannel.minorVersion();
        }
    }
    
    private long currentTime()
    {
        long currentTime;
        
        if (_consPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
        {
            currentTime = System.currentTimeMillis(); 
        }
        else // use nanosecond timer for tickRate of greater than 1000
        {
            currentTime = System.nanoTime();
        }
        
        return currentTime;
    }
    
    private long initNextTickTime()
    {
        long nextTickTime;
        
        if (_consPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
        {
            nextTickTime = System.currentTimeMillis() + _millisPerTick;
        }
        else // use nanosecond timer for tickRate of greater than 1000
        {
            nextTickTime = System.nanoTime() + _nsecPerTick;
        }
        
        return nextTickTime;
    }

    private long nextTickTime(long nextTickTime)
    {
        if (_consPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
        {
            nextTickTime += _millisPerTick;
        }
        else // use nanosecond timer for tickRate of greater than 1000
        {
            nextTickTime += _nsecPerTick;
        }
    
        return nextTickTime;
    }
    
    private long selectTime(long nextTickTime)
    {
        long selectTime;
        
        if (_consPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
        {
            selectTime = nextTickTime - System.currentTimeMillis(); 
        }
        else // use nanosecond timer for tickRate of greater than 1000
        {
            selectTime = nextTickTime - System.nanoTime();
        }
    
        return selectTime;
    }

	@Override
	public int serviceNameToIdCallback(String serviceName, Object closure)
    {
		ConsumerThread consumerThread = (ConsumerThread)closure;

		Buffer infoServiceName = consumerThread._srcDirHandler.serviceInfo().info().serviceName();
		if(infoServiceName != null && infoServiceName.toString().equals(serviceName))
		{
			return consumerThread._srcDirHandler.serviceInfo().serviceId();
		}
		
		return CodecReturnCodes.FAILURE;
	}

	@Override
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId,
			ReactorServiceNameToIdEvent serviceNameToIdEvent)
	{
		ConsumerThread consumerThread = (ConsumerThread)serviceNameToIdEvent.userSpecObj();
		
		if(consumerThread._service.info().serviceName().toString().equals(serviceNameToId.serviceName()))
		{
			serviceNameToId.serviceId(consumerThread._service.serviceId());
			return ReactorReturnCodes.SUCCESS;
		}
		
		return ReactorReturnCodes.FAILURE;
	}

	@Override
	public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent)
	{
		System.out.println("JSON Conversion error: " + jsonConversionEvent.error().text());

        return ReactorCallbackReturnCodes.SUCCESS;
	}
}
