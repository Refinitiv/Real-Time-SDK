package com.thomsonreuters.upa.perftools.upajniprovperf;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.shared.ConsumerLoginState;
import com.thomsonreuters.upa.shared.PingHandler;
import com.thomsonreuters.upa.perftools.common.LoginHandler;
import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.perftools.common.ItemAttributes;
import com.thomsonreuters.upa.perftools.common.ItemFlags;
import com.thomsonreuters.upa.perftools.common.ItemInfo;
import com.thomsonreuters.upa.perftools.common.NIProvPerfConfig;
import com.thomsonreuters.upa.perftools.common.PerfToolsReturnCodes;
import com.thomsonreuters.upa.perftools.common.ProviderPerfConfig;
import com.thomsonreuters.upa.perftools.common.ProviderSession;
import com.thomsonreuters.upa.perftools.common.ProviderThread;
import com.thomsonreuters.upa.perftools.common.XmlItemInfoList;
import com.thomsonreuters.upa.perftools.common.XmlMsgData;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.IoctlCodes;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.reactor.NIProviderCallback;
import com.thomsonreuters.upa.valueadd.reactor.NIProviderRole;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorConnectOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;

/** Non-interactive provider implementation of the provider thread.
  * Handles connecting to channel and processing of login response,
  * source directory refresh, and market data refresh/update messages. */
public class NIProviderThread extends ProviderThread implements NIProviderCallback
{
    private static final int CONNECTION_RETRY_TIME = 1; // seconds
	
    private DecodeIterator _dIter; /* decode iterator */
    private EncodeIterator _eIter; /* encode iterator */
    private Msg _msg; /* response message */
    private LoginHandler _loginHandler; /* login handler */
    private NIDirectoryProvider _directoryProvider; /* directory provider */
	private ChannelInfo _channelInfo; /* channel information */
    private ConnectOptions _connectOpts; /* connect options for non-interactive provider */
    private Channel _channel;
    private Selector _selector;
    private ProviderSession _provSession;
    private InProgInfo _inProg; /* connection in progress information */
    private WriteArgs _writeArgs = TransportFactory.createWriteArgs();
    private ReadArgs _readArgs = TransportFactory.createReadArgs();
    private PingHandler _pingHandler; /* ping handler */
    private Msg _responseMsg; /* response message */
    
    private long _nsecPerTick; /* nanoseconds per tick */
    private long _millisPerTick; /* milliseconds per tick */
    
    private Reactor _reactor; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorOptions _reactorOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private NIProviderRole _role; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorErrorInfo _errorInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorConnectOptions _connectOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorDispatchOptions _dispatchOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorConnectInfo _connectInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorChannelInfo _reactorChannnelInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorChannel _reactorChannel; // Use the VA Reactor instead of the UPA Channel for sending and receiving

    {
    	_dIter = CodecFactory.createDecodeIterator();
    	_eIter = CodecFactory.createEncodeIterator();
    	_msg = CodecFactory.createMsg();
        _loginHandler = new LoginHandler();
        _directoryProvider = new NIDirectoryProvider();
        _channelInfo = TransportFactory.createChannelInfo();
        _error = TransportFactory.createError();
        _inProg = TransportFactory.createInProgInfo();
        _pingHandler = new PingHandler();
    	_responseMsg = CodecFactory.createMsg();
    	
        _nsecPerTick = 1000000000 / ProviderPerfConfig.ticksPerSec();
        _millisPerTick = 1000 / ProviderPerfConfig.ticksPerSec();
        
        _reactorOptions = ReactorFactory.createReactorOptions();
        _role = ReactorFactory.createNIProviderRole();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _connectOptions = ReactorFactory.createReactorConnectOptions();
        _dispatchOptions = ReactorFactory.createReactorDispatchOptions();
        _connectInfo = ReactorFactory.createReactorConnectInfo();
        _reactorChannnelInfo = ReactorFactory.createReactorChannelInfo();
    }

	public NIProviderThread(XmlMsgData xmlMsgData)
	{
		super(xmlMsgData);
	}
    
	/* Initialize the NIProvider thread for UPA Channel usage. */
    private void initializeChannel()
    {
		// set-up directory provider
        _directoryProvider.serviceName(NIProvPerfConfig.serviceName());
        _directoryProvider.serviceId(NIProvPerfConfig.serviceId());
        _directoryProvider.openLimit(NIProvPerfConfig.openLimit());
        _directoryProvider.initService(_xmlMsgData);

    	/* Configure connection options. */
    	_connectOpts = TransportFactory.createConnectOptions();
    	_connectOpts.guaranteedOutputBuffers(NIProvPerfConfig.guaranteedOutputBuffers());
    	_connectOpts.majorVersion(Codec.majorVersion());
    	_connectOpts.minorVersion(Codec.minorVersion());
    	_connectOpts.protocolType(Codec.protocolType());
    	if (NIProvPerfConfig.sendBufSize() > 0)
    	{
    		_connectOpts.sysSendBufSize(NIProvPerfConfig.sendBufSize());
    	}
    	if (NIProvPerfConfig.recvBufSize() > 0)
    	{
    		_connectOpts.sysRecvBufSize(NIProvPerfConfig.recvBufSize());
    	}
    	_connectOpts.connectionType(NIProvPerfConfig.connectionType());
    	if (_connectOpts.connectionType() == ConnectionTypes.SOCKET)
    	{
    		_connectOpts.unifiedNetworkInfo().address(NIProvPerfConfig.hostName());
    		_connectOpts.unifiedNetworkInfo().serviceName(NIProvPerfConfig.portNo());
    		_connectOpts.unifiedNetworkInfo().interfaceName(NIProvPerfConfig.interfaceName());
    	}
    	else if (_connectOpts.connectionType() == ConnectionTypes.RELIABLE_MCAST)
    	{
        	_connectOpts.segmentedNetworkInfo().sendAddress(NIProvPerfConfig.sendAddress());
        	_connectOpts.segmentedNetworkInfo().sendServiceName(NIProvPerfConfig.sendPort());
        	_connectOpts.segmentedNetworkInfo().recvAddress(NIProvPerfConfig.recvAddress());
        	_connectOpts.segmentedNetworkInfo().recvServiceName(NIProvPerfConfig.recvPort());
        	_connectOpts.segmentedNetworkInfo().unicastServiceName(NIProvPerfConfig.unicastPort());
    		_connectOpts.segmentedNetworkInfo().interfaceName(NIProvPerfConfig.interfaceName());
    	}
    	_connectOpts.tcpOpts().tcpNoDelay(NIProvPerfConfig.tcpNoDelay());

    	/* Setup connection. */
    	setupChannelConnection();

        if(_provSession.printEstimatedMsgSizes(_error) != PerfToolsReturnCodes.SUCCESS)
		{
			closeChannelAndShutDown(_error.text());
		} 

        
    	// set login parameters
        _loginHandler.applicationName("upajNIProvPerf");
        _loginHandler.userName(NIProvPerfConfig.username());
        _loginHandler.role(Login.RoleTypes.PROV);

        // Send login request message
        TransportBuffer msg= _loginHandler.getRequestMsg(_channel, _error, _eIter);
        if (msg != null)
        {
        	write(msg);
        }
        else
        {
        	closeChannelAndShutDown("Sending login request failed");
        }

        /* Initialize ping handler */
        _pingHandler.initPingHandler(_channel.pingTimeout());
        
        _provSession.timeActivated(System.nanoTime());

    }
    
    /* Initialize the NIProvider thread for UPA VA Reactor usage. */
    private void initializeReactor()
    {
        // open selector
        try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.out.println("selector open ");
            System.exit(-1);
        }

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

        /* Configure connection options. */
        _connectInfo.connectOptions().guaranteedOutputBuffers(NIProvPerfConfig.guaranteedOutputBuffers());
        _connectInfo.connectOptions().majorVersion(Codec.majorVersion());
        _connectInfo.connectOptions().minorVersion(Codec.minorVersion());
        _connectInfo.connectOptions().protocolType(Codec.protocolType());
        if (NIProvPerfConfig.sendBufSize() > 0)
        {
            _connectInfo.connectOptions().sysSendBufSize(NIProvPerfConfig.sendBufSize());
        }
        if (NIProvPerfConfig.recvBufSize() > 0)
        {
            _connectInfo.connectOptions().sysRecvBufSize(NIProvPerfConfig.recvBufSize());
        }
        _connectInfo.connectOptions().connectionType(NIProvPerfConfig.connectionType());
        if (_connectInfo.connectOptions().connectionType() == ConnectionTypes.SOCKET)
        {
            _connectInfo.connectOptions().unifiedNetworkInfo().address(NIProvPerfConfig.hostName());
            _connectInfo.connectOptions().unifiedNetworkInfo().serviceName(NIProvPerfConfig.portNo());
            _connectInfo.connectOptions().unifiedNetworkInfo().interfaceName(NIProvPerfConfig.interfaceName());
        }
        else if (_connectInfo.connectOptions().connectionType() == ConnectionTypes.RELIABLE_MCAST)
        {
            _connectInfo.connectOptions().segmentedNetworkInfo().sendAddress(NIProvPerfConfig.sendAddress());
            _connectInfo.connectOptions().segmentedNetworkInfo().sendServiceName(NIProvPerfConfig.sendPort());
            _connectInfo.connectOptions().segmentedNetworkInfo().recvAddress(NIProvPerfConfig.recvAddress());
            _connectInfo.connectOptions().segmentedNetworkInfo().recvServiceName(NIProvPerfConfig.recvPort());
            _connectInfo.connectOptions().segmentedNetworkInfo().unicastServiceName(NIProvPerfConfig.unicastPort());
            _connectInfo.connectOptions().segmentedNetworkInfo().interfaceName(NIProvPerfConfig.interfaceName());
        }
        _connectInfo.connectOptions().tcpOpts().tcpNoDelay(NIProvPerfConfig.tcpNoDelay());
        
        _connectOptions.connectionList().add(_connectInfo);

        // set consumer role information
        _role.channelEventCallback(this);
        _role.defaultMsgCallback(this);
        _role.loginMsgCallback(this);
        _role.initDefaultRDMLoginRequest();
        // set login parameters
        _role.rdmLoginRequest().applyHasAttrib();
        _role.rdmLoginRequest().attrib().applyHasApplicationName();
        _role.rdmLoginRequest().attrib().applicationName().data("upajNIProvPerf");
        if (NIProvPerfConfig.username() != null && !NIProvPerfConfig.username().equals(""))
        {
            _role.rdmLoginRequest().userName().data(NIProvPerfConfig.username());
        }
        // set-up directory provider and initialize directory refresh
        _directoryProvider.serviceName(NIProvPerfConfig.serviceName());
        _directoryProvider.serviceId(NIProvPerfConfig.serviceId());
        _directoryProvider.openLimit(NIProvPerfConfig.openLimit());
        _directoryProvider.initService(_xmlMsgData);
        _directoryProvider.initRefresh(-1);
        _role.rdmDirectoryRefresh(_directoryProvider.directoryRefresh());

        /* Setup connection. */
        setupReactorConnection();
    }
    
	/* Set-up the UPA Channel connection for the NIProvider thread. */
	private void setupChannelConnection()
	{ 
        int handshake;
        while (true)
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
        System.out.println("Connected ");
        _provSession = new ProviderSession(_xmlMsgData, _itemEncoder);
        ClientChannelInfo ccInfo = new ClientChannelInfo();
        ccInfo.channel = _channel;
        _provSession.init(ccInfo);
        		
        // set the high water mark if configured
    	if (ProviderPerfConfig.highWaterMark() > 0)
    	{
    		if (_channel.ioctl(IoctlCodes.HIGH_WATER_MARK, ProviderPerfConfig.highWaterMark(), _error) != TransportReturnCodes.SUCCESS)
    		{
    			closeChannelAndShutDown("Channel.ioctl() failed");
    		}
    	}

    	try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
    		System.out.println("selector open failure");
    		System.exit(-1);
        }

    	// register selector for read
    	addOption(SelectionKey.OP_READ, _channel);

		/* retrieve and print out channel information */
		if (_channel.info(_channelInfo, _error) != TransportReturnCodes.SUCCESS)
		{
			closeChannelAndShutDown("Channel.info() failed");
		} 
    	System.out.printf("Channel active. " + _channelInfo.toString() + "\n");

		/* Check that we can successfully pack, if packing messages. */
		if (NIProvPerfConfig.totalBuffersPerPack() > 1
				&& NIProvPerfConfig.packingBufferLength() > _channelInfo.maxFragmentSize())
		{
			System.err.printf("Error(Channel %s): MaxFragmentSize %d is too small for packing buffer size %d\n",
					_channel.selectableChannel(), _channelInfo.maxFragmentSize(), 
					NIProvPerfConfig.packingBufferLength());
			System.exit(-1);
		}

	}

    /* Set-up the UPA VA Reactor connection for the NIProvider thread. */
    private void setupReactorConnection()
    { 
        // connect via Reactor
        int ret;
        if ((ret = _reactor.connect(_connectOptions, _role, _errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("Reactor.connect failed with return code: " + ret + " error = " + _errorInfo.error().text());
            System.exit(ReactorReturnCodes.FAILURE);
        }
    }

    /** Run the non-interactive provider thread. Sets up the directory provider
	  * and connection. Then enters loop that continually reads from and sends
	  * refresh/update messages to the connected channel. */
	@Override
	public void run()
	{
		if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
		{
		    initializeChannel();
		}
		else // use UPA VA Reactor for sending and receiving
		{
		    initializeReactor();
		}
    	
    	/* Determine update rates on per-tick basis */
		long nextTickTime = initNextTickTime();

        /* this is the main loop */
        while (!shutdown())
        {
            // read until no more to read and then write leftover from previous burst
            long selectTime = selectTime(nextTickTime);
        	selectorRead(selectTime);
        	
            /* Handle pings */
        	if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
        	{
                if (_pingHandler.handlePings(_channel, _error) != CodecReturnCodes.SUCCESS)
                {
                	closeChannelAndShutDown("Error handling pings: " + _error.text());
                }
        	}

			if (nextTickTime <= currentTime())
			{
			    nextTickTime = nextTickTime(nextTickTime);
				sendMsgBurst();
			}
        }
        
        shutdownAck(true);
	}
	
	/* Reads from a channel. */
	private void selectorRead(long selectTime)
	{
		Set<SelectionKey> keySet = null;

		// set select time 
        try
        {
            int selectRetVal;
            
            if (ProviderPerfConfig.ticksPerSec() > 1000) // use nanosecond timer for tickRate of greater than 1000
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
        }

        // nothing to read or write
        if (keySet == null)
            return;

        Iterator<SelectionKey> iter = keySet.iterator();
        while (iter.hasNext())
        {
        	SelectionKey key = iter.next();
        	iter.remove();
    	    if(!key.isValid())
                continue;
    		if (key.isReadable())
    		{
                if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
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
                else // use UPA VA Reactor for sending and receiving
                {
                    int ret;
                    
                    // retrieve associated reactor channel and dispatch on that channel 
                    ReactorChannel reactorChnl = (ReactorChannel)key.attachment();
                    
                    /* read until no more to read */
                    while ((ret = reactorChnl.dispatch(_dispatchOptions, _errorInfo)) > 0) {}
                    if (ret == ReactorReturnCodes.FAILURE)
                    {
                        if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                            reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
                        {
                            closeChannelAndShutDown("ReactorChannel dispatch failed: " + ret + "(" + _errorInfo.error().text() + ")");
                            System.exit(ReactorReturnCodes.FAILURE);
                        }
                    }
                }
    		}
        }
    }

    /** Process transport response. */
	private void processResponse(TransportBuffer buffer)
	{
        /* clear decode iterator */
        _dIter.clear();

        /* set buffer and version info */
        _dIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion());

        int ret = _responseMsg.decode(_dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
        	closeChannelAndShutDown("DecodeMsg(): Error " + ret);
        }

        if (_responseMsg.domainType() == DomainTypes.LOGIN)
        {
        	ret = _loginHandler.processResponse(_responseMsg, _dIter, _error);
        	if (ret != CodecReturnCodes.SUCCESS)
        		if (ret != CodecReturnCodes.SUCCESS)
        		{
        			closeChannelAndShutDown(_error.text());
        		}

        	if (_loginHandler.loginState() == ConsumerLoginState.OK_SOLICITED)
        	{
        		/* send source directory refresh message */
        		TransportBuffer sourceDir = _directoryProvider.encodeRefresh(_channel, -1, _error);
        		if (sourceDir != null)
        		{
        			write(sourceDir);
        		}
        		else
        		{
        			closeChannelAndShutDown("publishDirectoryRefresh() failed: ");
        		}

        		/* create item list to publish */
        		createItemList();
        	}
        	else
        	{
        		System.out.printf("Login stream closed.\n");
        		ret = PerfToolsReturnCodes.FAILURE;
        	}
        }
        else
        	System.err.printf("Received message with unhandled domain: %d\n", _msg.domainType());
	}
	
	void createItemList()
	{
        if (NIProvPerfConfig.itemPublishCount() > 0)
        {
            int itemListUniqueIndex;
            int itemListCount;
            int itemListCountRemainder;

            /* Figure out which items this thread should publish. */

            /* Calculate unique index for each thread. Each thread publishes a common
             * and unique set of items. Unique index is so each thread has a unique
             * index into the shared item list. Unique items for this provider are after
             * the items assigned to providers with a lower index.
             */
            itemListUniqueIndex = NIProvPerfConfig.commonItemCount();
            itemListUniqueIndex += ((NIProvPerfConfig.itemPublishCount() - NIProvPerfConfig.commonItemCount())
                    / NIProvPerfConfig.threadCount()) * (providerIndex());

            itemListCount = NIProvPerfConfig.itemPublishCount() / NIProvPerfConfig.threadCount();
            itemListCountRemainder = NIProvPerfConfig.itemPublishCount() % NIProvPerfConfig.threadCount();

            if (providerIndex() < itemListCountRemainder)
            {
                /* This provider publishes an extra item */
                itemListCount += 1;

                /* Shift index by one for each provider before this one, since they publish extra items too. */
                itemListUniqueIndex += providerIndex();
            }
            else
                /* Shift index by one for each provider that publishes an extra item. */
                itemListUniqueIndex += itemListCountRemainder;

            if (addPublishingItems(NIProvPerfConfig.itemFilename(), _provSession, 
                    NIProvPerfConfig.commonItemCount(), itemListUniqueIndex, itemListCount - NIProvPerfConfig.commonItemCount(), 
                    NIProvPerfConfig.serviceId()) != PerfToolsReturnCodes.SUCCESS)
            {
                closeChannelAndShutDown("providerSessionAddPublishingItems() failed\n");
            }
            else
            {
                System.out.printf("Created publishing list.\n");
            }
        }	    
	}

    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        switch(event.eventType())
        {
            case ReactorChannelEventTypes.CHANNEL_UP:
            {
                System.out.println("Connected ");
                
                _reactorChannel = event.reactorChannel();
                        
                // set the high water mark if configured
                if (ProviderPerfConfig.highWaterMark() > 0)
                {
                    if (_reactorChannel.ioctl(IoctlCodes.HIGH_WATER_MARK, ProviderPerfConfig.highWaterMark(), _errorInfo) != TransportReturnCodes.SUCCESS)
                    {
                        closeChannelAndShutDown("ReactorChannel.ioctl() failed");
                    }
                }
        
                // register selector with channel event's reactorChannel
                try
                {
                    event.reactorChannel().selectableChannel().register(_selector,
                                                                        SelectionKey.OP_READ,
                                                                        event.reactorChannel());
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
        
                /* retrieve and print out channel information */
                if (_reactorChannel.info(_reactorChannnelInfo, _errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    closeChannelAndShutDown("ReactorChannel.info() failed");
                } 
                System.out.printf("Channel active. " + _reactorChannnelInfo.channelInfo().toString() + "\n");
        
                /* Check that we can successfully pack, if packing messages. */
                if (NIProvPerfConfig.totalBuffersPerPack() > 1
                        && NIProvPerfConfig.packingBufferLength() > _reactorChannnelInfo.channelInfo().maxFragmentSize())
                {
                    System.err.printf("Error(Channel %s): MaxFragmentSize %d is too small for packing buffer size %d\n",
                            _reactorChannel.selectableChannel(), _reactorChannnelInfo.channelInfo().maxFragmentSize(), 
                            NIProvPerfConfig.packingBufferLength());
                    System.exit(-1);
                }
                
                _provSession = new ProviderSession(_xmlMsgData, _itemEncoder);
                ClientChannelInfo ccInfo = new ClientChannelInfo();
                ccInfo.reactorChannel = _reactorChannel;
                ccInfo.channel = _reactorChannel.channel();
                _provSession.init(ccInfo);
                
                _provSession.timeActivated(System.nanoTime());
                
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_READY:
            {
                if(_provSession.printEstimatedMsgSizes(_error) != PerfToolsReturnCodes.SUCCESS)
                {
                    closeChannelAndShutDown(_error.text());
                } 
                
                break;
            }
            case ReactorChannelEventTypes.FD_CHANGE:
            {
                System.out.println("Channel Change - Old Channel: "
                        + event.reactorChannel().oldSelectableChannel() + " New Channel: "
                        + event.reactorChannel().selectableChannel());
                
    	        // cancel old reactorChannel select
                SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(_selector);
                if (key != null)
                    key.cancel();
    
                // register selector with channel event's new reactorChannel
                try
                {
                    event.reactorChannel().selectableChannel().register(_selector,
                                                                    SelectionKey.OP_READ,
                                                                    event.reactorChannel());
                }
                catch (Exception e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
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
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(_selector);
                    if (key != null)
                        key.cancel();
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
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(_selector);
                    if (key != null)
                        key.cancel();
                }

                // close ReactorChannel
                if (_reactorChannel != null)
                {
                    _reactorChannel.close(_errorInfo);
                }
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
        System.out.println("Received message with unhandled domain: " + event.msg().domainType());
        
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
                
                if (loginRefresh.state().streamState() == StreamStates.OPEN && loginRefresh.state().dataState() == DataStates.OK)
                {
                    /* create item list to publish */
                    createItemList();
                }
                else
                {
                    closeChannelAndShutDown("Login stream closed");
                }
                break;
            default:
                break;
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }

	/**
	 * Loads xml file containing item messages to send and add item information
	 * to be published to item watch list.
	 */
	private int addPublishingItems(String xmlItemInfoFile, ProviderSession providerSession, int commonItemCount, int itemListUniqueIndex, int uniqueItemCount, int serviceId)
	{
		XmlItemInfoList _xmlItemInfoList = new XmlItemInfoList(itemListUniqueIndex + uniqueItemCount);
        if (_xmlItemInfoList.parseFile(xmlItemInfoFile) == PerfToolsReturnCodes.FAILURE)
        {
            System.err.println("Failed to load item list from file '" + xmlItemInfoFile + "'.");
            return PerfToolsReturnCodes.FAILURE;
        }
        
        int itemListIndex = 0;
        for(int i = 0; i < commonItemCount + uniqueItemCount; ++i)
        {
            if(itemListIndex == commonItemCount && itemListIndex < itemListUniqueIndex)
                itemListIndex = itemListUniqueIndex;
            MsgKey msgKey = CodecFactory.createMsgKey();
            msgKey.flags(MsgKeyFlags.HAS_NAME_TYPE | MsgKeyFlags.HAS_NAME | MsgKeyFlags.HAS_SERVICE_ID);
            msgKey.nameType(InstrumentNameTypes.RIC);
            msgKey.serviceId(serviceId);
            msgKey.name().data(_xmlItemInfoList.itemInfoList()[itemListIndex].name());
            ItemAttributes attributes = new ItemAttributes();
            attributes.domainType(_xmlItemInfoList.itemInfoList()[itemListIndex].domainType());
            attributes.msgKey(msgKey);
            
            ItemInfo itemInfo = providerSession.createItemInfo(attributes, -i-6);
            if(itemInfo == null)
            {
                return PerfToolsReturnCodes.FAILURE;
            }
            itemInfo.itemFlags(ItemFlags.IS_STREAMING_REQ);
            providerSession.itemAttributesTable().put(itemInfo.attributes(), itemInfo);
            providerSession.itemStreamIdTable().put(itemInfo.streamId(), itemInfo);
            
            ++itemListIndex;
        }
        
        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Clean up provider threads.
     */
    public void cleanup()
    {
    	super.cleanup();
    }
    
    /**
     * Send refreshes & updates to channels open.
     */
    private void sendMsgBurst()
    {
        if (_provSession == null)
        {
            return;
        }
        
        // The application corrects for ticks that don't finish before the time 
        // that the next update burst should start.  But don't do this correction 
        // for new channels.
        if(_provSession.timeActivated() == 0)
        {
            return;
        }
     
        int ret = TransportReturnCodes.SUCCESS;

        // Send burst of updates
        if(ProviderPerfConfig.updatesPerSec() != 0 && _provSession.updateItemList().count() != 0)
        {
    	    ret = sendUpdateBurst(_provSession, _error);
            while (ret > TransportReturnCodes.SUCCESS)
            {
        	    // Need to flush
                if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
                {
                    ret = _channel.flush(_error);
                }
            }
        }
        // Use remaining time in the tick to send refreshes.
        while( ret >= TransportReturnCodes.SUCCESS &&  _provSession.refreshItemList().count() != 0)
            ret = sendRefreshBurst(_provSession, _error);
       
        if(ret < TransportReturnCodes.SUCCESS)
        {
            switch(ret)
            {
                case TransportReturnCodes.NO_BUFFERS:
                    if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
                    {
                        ret = _channel.flush(_error);
                    }
                    break;
                default:
            	    if (!Thread.interrupted())
            	    {
            		    closeChannelAndShutDown("Failure while writing message bursts: " + _error.text());
            	    }
                    break;
            }
        }
        else if (ret > TransportReturnCodes.SUCCESS)
        {
            while (ret > TransportReturnCodes.SUCCESS)
            {
        	    // Need to flush
                if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
                {
                    ret = _channel.flush(_error);
                }
            }
        }
    }
    
    /* Writes the content of the TransportBuffer to the UPA channel.*/
    private void write(TransportBuffer msgBuf)
    {
    	// write data to the channel
    	_writeArgs.clear();
    	_writeArgs.priority(WritePriorities.HIGH);
    	_writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
    	int ret = _channel.write(msgBuf, _writeArgs, _error);

        while (ret > TransportReturnCodes.SUCCESS)
        {
     	   // Need to flush
     	   ret = _channel.flush(_error);
        }
        
        if(ret < TransportReturnCodes.SUCCESS)
        {
            switch(ret)
            {
                case TransportReturnCodes.NO_BUFFERS:
             	   _channel.flush(_error);
                    break;
                default:
             	   if (!Thread.interrupted())
             	   {
             		   closeChannelAndShutDown("Failure while writing message bursts: " + _error.text());
             	   }
                   break;
            }
        }
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
		}
    }

    private void closeChannelAndShutDown(String text)
    {
		System.out.println(text);
        if (!NIProvPerfConfig.useReactor()) // use UPA Channel for sending and receiving
        {
            _channel.close(_error);
        }
        else // use UPA VA Reactor for sending and receiving
        {
            _reactorChannel.close(_errorInfo);
        }
		shutdown();
		shutdownAck(true);
		System.exit(-1);
    }

    private long currentTime()
    {
        long currentTime;
        
        if (ProviderPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
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
        
        if (ProviderPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
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
        if (ProviderPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
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
        
        if (ProviderPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
        {
            selectTime = nextTickTime - System.currentTimeMillis(); 
        }
        else // use nanosecond timer for tickRate of greater than 1000
        {
            selectTime = nextTickTime - System.nanoTime();
        }
    
        return selectTime;
    }
}
