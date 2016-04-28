package com.thomsonreuters.upa.perftools.upajconsperf;

import java.io.IOException;
import java.nio.ByteBuffer;
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
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.PostUserInfo;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.perftools.common.ShutdownCallback;
import com.thomsonreuters.upa.perftools.common.XmlItemInfoList;
import com.thomsonreuters.upa.perftools.common.XmlMsgData;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.IoctlCodes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerCallback;
import com.thomsonreuters.upa.valueadd.reactor.ConsumerRole;
import com.thomsonreuters.upa.valueadd.reactor.DictionaryDownloadModes;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
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
import com.thomsonreuters.upa.valueadd.reactor.ReactorRole;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

public class ConsumerThreadReactor extends ConsumerThread implements ConsumerCallback
{
    private Reactor _reactor;
    private ReactorOptions _reactorOptions;
    private ConsumerRole _role;
    private ReactorErrorInfo _errorInfo;
    private ReactorConnectOptions _connectOptions;
    private ReactorDispatchOptions _dispatchOptions;
    private ReactorSubmitOptions _submitOptions;
    private Service _service;
    private ReactorConnectInfo _connectInfo; /* connection information */
    private ReactorChannelInfo _chnlInfo; /* channel information */
    private ReactorChannel _reactorChannel;
    private Selector _selector;
    private PostMsg _postMsg;
    private Buffer _postBuffer;
    private GenericMsg _genericMsg;
    private Buffer _genericBuffer;

    public ConsumerThreadReactor(ConsumerThreadInfo consInfo, ConsPerfConfig consConfig,
            XmlItemInfoList itemList, XmlMsgData msgData, PostUserInfo postUserInfo,
            ShutdownCallback shutdownCallback)
    {
        super(consInfo, consConfig, itemList, msgData, postUserInfo, shutdownCallback);

        _reactorOptions = ReactorFactory.createReactorOptions();
        _role = ReactorFactory.createConsumerRole();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _connectOptions = ReactorFactory.createReactorConnectOptions();
        _dispatchOptions = ReactorFactory.createReactorDispatchOptions();
        _submitOptions = ReactorFactory.createReactorSubmitOptions();
        _connectInfo = ReactorFactory.createReactorConnectInfo();
        _chnlInfo = ReactorFactory.createReactorChannelInfo();
        _service = DirectoryMsgFactory.createService();
        _postMsg = (PostMsg)CodecFactory.createMsg();
        _postBuffer = CodecFactory.createBuffer();
        _postBuffer.data(ByteBuffer.allocate(512));
        _genericMsg = (GenericMsg)CodecFactory.createMsg();
        _genericBuffer = CodecFactory.createBuffer();
        _genericBuffer.data(ByteBuffer.allocate(512));
    }

    protected void connect()
    {
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
                
        /* set connect options  */
        _connectInfo.connectOptions().majorVersion(Codec.majorVersion());
        _connectInfo.connectOptions().minorVersion(Codec.minorVersion());        
        _connectInfo.connectOptions().connectionType(_consPerfConfig.connectionType());
        _connectInfo.connectOptions().guaranteedOutputBuffers(_consPerfConfig.guaranteedOutputBuffers());
        _connectInfo.connectOptions().numInputBuffers(_consPerfConfig.numInputBuffers());
        if (_consPerfConfig.sendBufSize() > 0)
        {
            _connectInfo.connectOptions().sysSendBufSize(_consPerfConfig.sendBufSize());
        }
        if (_consPerfConfig.recvBufSize() > 0)
        {
            _connectInfo.connectOptions().sysRecvBufSize(_consPerfConfig.recvBufSize());
        }
        if(_consPerfConfig.connectionType() == ConnectionTypes.SOCKET)
        {
            _connectInfo.connectOptions().tcpOpts().tcpNoDelay(_consPerfConfig.tcpNoDelay());
        }
        // set the connection parameters on the connect options 
        _connectInfo.connectOptions().unifiedNetworkInfo().address(_consPerfConfig.hostName());
        _connectInfo.connectOptions().unifiedNetworkInfo().serviceName(_consPerfConfig.portNo());
        _connectInfo.connectOptions().unifiedNetworkInfo().interfaceName(_consPerfConfig.interfaceName());
        
        _connectOptions.connectionList().add(_connectInfo);
        
        // set consumer role information
        _role.channelEventCallback(this);
        _role.defaultMsgCallback(this);
        _role.loginMsgCallback(this);
        _role.directoryMsgCallback(this);
        _role.dictionaryMsgCallback(this);
        if (!isDictionariesLoaded())
        {
            _role.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
        }
        _role.initDefaultRDMLoginRequest();
        // set login parameters
        _role.rdmLoginRequest().applyHasAttrib();
        _role.rdmLoginRequest().attrib().applyHasApplicationName();
        _role.rdmLoginRequest().attrib().applicationName().data("upajConsPerf");
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
        
        // connect via Reactor
        int ret;
        if ((ret = _reactor.connect(_connectOptions, (ReactorRole)_role, _errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("Reactor.connect failed with return code: " + ret + " error = " + _errorInfo.error().text());
            System.exit(ReactorReturnCodes.FAILURE);
        }       
    }

    /** Run the consumer thread. */
    public void run()
    {
        // initialize the test data from configuration and xml files
        initialize();

        int ret = 0;
        int currentTicks = 0;
        long nextTickTime = System.nanoTime() + _nsecPerTick;
        long selectTime;

        while (!_consThreadInfo.shutdown())
        {

            // read until no more to read and then write leftover from previous burst
            selectTime = nextTickTime-System.nanoTime(); 
            selectorRead(selectTime);
            
            if (System.nanoTime() >= nextTickTime)
            {
                nextTickTime += _nsecPerTick;

                // only send bursts on tick boundary
                if (_requestsSent && _reactorChannel.state() == ReactorChannel.State.READY)
                {
                    // send item request and post bursts
                    if ((ret = sendBursts(currentTicks, _reactorChannel.channel(), _service)) < TransportReturnCodes.SUCCESS)
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
        }   // end of run loop
        
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
        closeReactor();
        System.out.println("\nConsumerThread " + _consThreadInfo.threadId() + " exiting...");
    }
    
    /* Reads from a channel. */
    private void selectorRead(long selectTime)
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

        while (iter.hasNext())
        {
            SelectionKey key = iter.next();
            iter.remove();
            try
            {
                if (key.isReadable())
                {
                    _reactorChannel = (ReactorChannel)key.attachment();
                    read();
                }
            }
            catch (CancelledKeyException e)
            {
            } // key can be canceled during shutdown
        }
    }
    
    /* Writes the content of the TransportBuffer to the UPA channel.*/
    protected void write(TransportBuffer msgBuf)
    {
        // write data to the channel
        _submitOptions.writeArgs().clear();
        _submitOptions.writeArgs().priority(WritePriorities.HIGH);
        _submitOptions.writeArgs().flags(WriteFlags.DIRECT_SOCKET_WRITE);
        int retval = _reactorChannel.submit(msgBuf, _submitOptions, _errorInfo);

        if (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
        {
            //call flush and write again until there is data in the queue
            while (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
            {
                retval = _reactorChannel.submit(msgBuf, _submitOptions, _errorInfo);
            }
        }
        else if (retval < ReactorReturnCodes.SUCCESS)
        {
            // write failed, release buffer and shut down 
            _reactorChannel.releaseBuffer(msgBuf, _errorInfo);
            closeChannelAndShutDown(_errorInfo.error().text());
        }
    }
    
    /* Writes an message to the UPA channel. */
    protected int writeMsg(Msg msg, Channel channel)
    {
        int ret = TransportReturnCodes.SUCCESS;
        
        if (!_consPerfConfig.useWatchlist()) // VA Reactor Watchlist not enabled
        {
            ret = super.writeMsg(msg, channel);
        }
        else // VA Reactor Watchlist is enabled, submit message instead of buffer
        {
            if (_reactorChannel.state() == com.thomsonreuters.upa.valueadd.reactor.ReactorChannel.State.READY)
            {
                int retval = _reactorChannel.submit(msg, _submitOptions, _errorInfo);

                if (retval < ReactorReturnCodes.SUCCESS)
                {
                    if (_errorInfo.error().text() == null)
                    {
                        _errorInfo.error().text("ReactorChannel.submit() failed");
                    }
                    // write failed, shut down 
                    closeChannelAndShutDown(_errorInfo.error().text());
                }
            }
        }
        
        return ret;
    }

    //sends a burst of post messages. 
    protected int sendPostBurst(int itemBurstCount, Channel channel)
    {
        int ret = TransportReturnCodes.SUCCESS;

        if (!_consPerfConfig.useWatchlist()) // VA Reactor Watchlist not enabled
        {
            ret = super.sendPostBurst(itemBurstCount, channel);
        }
        else // VA Reactor Watchlist is enabled, submit message instead of buffer
        {
            long encodeStartTime;
            int latencyUpdateNumber;

            latencyUpdateNumber = (_consPerfConfig.latencyPostsPerSec() > 0) ?
                    _postLatencyRandomArray.next() : -1; 

            for (int i = 0; i < itemBurstCount; ++i)
            {
                ItemRequest postItem = nextPostItem();

                if (latencyUpdateNumber == i)
                    encodeStartTime = System.nanoTime()/1000;
                else
                    encodeStartTime = 0;

                // create properly encoded post message
                _postMsg.clear();
                _postBuffer.data().clear();
                if ((ret = _itemEncoder.createItemPost(channel, postItem.itemInfo(), _postMsg, _postBuffer, _postUserInfo, encodeStartTime)) != CodecReturnCodes.SUCCESS)
                {
                    System.out.printf("createItemPost() failed: %d.\n", ret);
                    return ret;
                }

                writeMsg(_postMsg, channel);

                _consThreadInfo.stats().postSentCount().increment();
            }
        }

        return ret;
    }

    // sends a burst of generic messages.
    protected int sendGenMsgBurst(int itemBurstCount, Channel channel) 
    {
        int ret = TransportReturnCodes.SUCCESS;

        if (!_consPerfConfig.useWatchlist()) // VA Reactor Watchlist not enabled
        {
            ret = super.sendGenMsgBurst(itemBurstCount, channel);
        }
        else // VA Reactor Watchlist is enabled, submit message instead of buffer
        {
            long encodeStartTime;
            int latencyGenMsgNumber;

            latencyGenMsgNumber = (_consPerfConfig.latencyGenMsgsPerSec() > 0) ? _genMsgLatencyRandomArray
                    .next() : -1;
    
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
    
                // create properly encoded generic message
                _genericMsg.clear();
                _genericBuffer.data().clear();
                if ((ret = _itemEncoder.createItemGenMsg(channel, genMsgItem.itemInfo(), _genericMsg, _genericBuffer, encodeStartTime)) != CodecReturnCodes.SUCCESS)
                {
                    System.out.printf("createItemPost() failed: %d.\n", ret);
                    return ret;
                }

                writeMsg(_genericMsg, channel);
    
                _consThreadInfo.stats().genMsgSentCount().increment();
            }
        }

        return ret;
    }
    
    private void read()
    {
        int ret;
        
        /* read until no more to read */
        while ((ret = _reactorChannel.dispatch(_dispatchOptions, _errorInfo)) > 0) {}
        if (ret == ReactorReturnCodes.FAILURE)
        {
            if (_reactorChannel.state() != ReactorChannel.State.CLOSED &&
                    _reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
            {
                System.out.println("ReactorChannel dispatch failed: " + ret + "(" + _errorInfo.error().text() + ")");
                closeReactor();
                System.exit(ReactorReturnCodes.FAILURE);
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
                    event.reactorChannel().selectableChannel().register(_selector,
                                                                        SelectionKey.OP_READ,
                                                                        event.reactorChannel());
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                
                // get and print the channel info
                if (_reactorChannel.info(_chnlInfo, _errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    closeChannelAndShutDown("Channel.info() failed");
                    return ReactorCallbackReturnCodes.FAILURE;
                } 
                System.out.printf("Channel active. " + _chnlInfo.toString() + "\n");

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
                    }

                    _requestsSent = true;
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
        Msg msg = event.msg();
        
        _dIter.clear();
        _dIter.setBufferAndRWFVersion(msg.encodedDataBody(), _reactorChannel.majorVersion(), _reactorChannel.minorVersion());

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
                        rdmService.copy(_service);
                    }
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
        return  _service.checkHasState() && _service.state().checkHasAcceptingRequests() && _service.state().acceptingRequests() == 1 && _service.state().serviceState() == 1;
    }

    // close Reactor used by this thread and shut down
    protected void closeChannelAndShutDown(String text)
    {
        System.out.println(text);
        _shutdownCallback.shutdown();
        _consThreadInfo.shutdownAck(true);
        closeReactor();
    }
    
    // close Reactor used by this thread
    private void closeReactor()
    {
        _reactor.shutdown(_errorInfo);
    }
}
