package com.thomsonreuters.upa.perftools.upajprovperf;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.locks.Lock;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.shared.provider.ItemRejectReason;
import com.thomsonreuters.upa.perftools.common.ChannelHandler;
import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.perftools.common.DictionaryProvider;
import com.thomsonreuters.upa.perftools.common.PerfToolsReturnCodes;
import com.thomsonreuters.upa.perftools.common.ProviderPerfConfig;
import com.thomsonreuters.upa.perftools.common.ProviderSession;
import com.thomsonreuters.upa.perftools.common.ProviderThread;
import com.thomsonreuters.upa.perftools.common.XmlMsgData;
import com.thomsonreuters.upa.perftools.common.DictionaryProvider.DictionaryRejectReason;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.IoctlCodes;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.reactor.ProviderCallback;
import com.thomsonreuters.upa.valueadd.reactor.ProviderRole;
import com.thomsonreuters.upa.valueadd.reactor.RDMDictionaryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMDirectoryMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.RDMLoginMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorAcceptOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;

/**
 * Interactive provider implementation of the provider thread callback. Handles
 * accepting of new channels, processing of incoming messages and sending of
 * message bursts.
 */
public class IProviderThread extends ProviderThread implements ProviderCallback
{
    private static final String applicationName = "upajProvPerf";
    private static final String applicationId = "256";

    private IDirectoryProvider _directoryProvider;   // Source directory requests handler
    private LoginProvider _loginProvider;           // Login requests handler
    private DictionaryProvider _dictionaryProvider; // Dictionary requests handler
    private ItemRequestHandler _itemRequestHandler; // Iterm requests handler
    private DecodeIterator _decodeIter;             // Decode iterator
    
    private ChannelInfo _channelInfo;               // Active channel information
    private ChannelHandler   _channelHandler;       // Channel handler.

    private Msg _tmpMsg;
    
    private int _connectionCount; //Number of client sessions currently connected.
    
    private long _nsecPerTick; /* nanoseconds per tick */
    private long _millisPerTick; /* milliseconds per tick */
    private long _divisor = 1;

    private Reactor _reactor; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ProviderRole _providerRole; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorErrorInfo _errorInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorOptions _reactorOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorAcceptOptions _acceptOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorDispatchOptions _dispatchOptions; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private ReactorChannelInfo _reactorChannnelInfo; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private Selector _selector; // Use the VA Reactor instead of the UPA Channel for sending and receiving
    private boolean _reactorInitialized; // Use the VA Reactor instead of the UPA Channel for sending and receiving

    {
        _decodeIter = CodecFactory.createDecodeIterator();
        _tmpMsg = CodecFactory.createMsg();
        _loginProvider = new LoginProvider();
        _directoryProvider = new IDirectoryProvider();
        _dictionaryProvider = new DictionaryProvider();
        _channelInfo = TransportFactory.createChannelInfo();
        _itemRequestHandler = new ItemRequestHandler();
        _channelHandler = new ChannelHandler(this);
        
        _nsecPerTick = 1000000000 / ProviderPerfConfig.ticksPerSec();
        _millisPerTick = 1000 / ProviderPerfConfig.ticksPerSec();
        if (ProviderPerfConfig.ticksPerSec() > 1000)
        {
            _divisor = 1000000;
        }
        
        _providerRole = ReactorFactory.createProviderRole();
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _reactorOptions = ReactorFactory.createReactorOptions();
        _acceptOptions = ReactorFactory.createReactorAcceptOptions();
        _dispatchOptions = ReactorFactory.createReactorDispatchOptions();
        _reactorChannnelInfo = ReactorFactory.createReactorChannelInfo();
    }

    public IProviderThread(XmlMsgData xmlMsgData)
    {
        super(xmlMsgData);
    }
    
    /**
     * Handles newly accepted client channel.
     */
    public void acceptNewChannel(Channel channel)
    {
            ProviderSession provSession = new ProviderSession(_xmlMsgData, _itemEncoder);
            ++_connectionCount;
            provSession.init(_channelHandler.addChannel(channel, provSession, true));
    }
    
    /**
     * Handles a new reactor channel.
     */
    public int acceptNewReactorChannel(Server server, ReactorErrorInfo errorInfo)
    {
        while (!_reactorInitialized)
        {
            try
            {
                Thread.sleep(10);
            }
            catch (InterruptedException e)
            {
                System.out.printf("acceptNewReactorChannel() Thread.sleep() exception: " + e.getLocalizedMessage());
            }
        }
        // create provider session
        ProviderSession provSession = new ProviderSession(_xmlMsgData, _itemEncoder);
        ClientChannelInfo ccInfo = new ClientChannelInfo();
        provSession.init(ccInfo);
        ccInfo.userSpec = provSession;
        provSession.providerThread(this);
        ++_connectionCount;

        // initialize provider role
        _providerRole.channelEventCallback(this);
        _providerRole.defaultMsgCallback(this);
        _providerRole.loginMsgCallback(this);
        _providerRole.directoryMsgCallback(this);
        _providerRole.dictionaryMsgCallback(this);

        System.out.printf("Accepting new Reactor connection...\n");
        
        _acceptOptions.clear();
        _acceptOptions.acceptOptions().userSpecObject(provSession);

        return _reactor.accept(server, _acceptOptions, _providerRole, errorInfo);
    }
    
    /**
     * Number of client sessions currently connected.
     */
    public int connectionCount()
    {
       return _connectionCount;
    }
        
    /**
     * Process the channel active event for the interactive provider
     * application. This causes some additional channel configuration and a
     * print out of the channel configuration.
     */
    public int processActiveChannel(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
    {
        int ret;
        if (ProviderPerfConfig.highWaterMark() > 0)
        {
            ret = clientChannelInfo.channel.ioctl(IoctlCodes.HIGH_WATER_MARK, ProviderPerfConfig.highWaterMark(), error);
            if (ret != TransportReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;
        }

        ret = clientChannelInfo.channel.info(_channelInfo, error);
        if (ret != TransportReturnCodes.SUCCESS)
            return PerfToolsReturnCodes.FAILURE;

        System.out.printf("Client channel active. Channel Info:\n" +
                "  Max Fragment Size: %d\n" +
                "  Output Buffers: %d Max, %d Guaranteed\n" +
                "  Input Buffers: %d\n" +
                "  Send/Recv Buffer Sizes: %d/%d\n" +
                "  Ping Timeout: %d\n" +
                "  Connected component version: ",
                          _channelInfo.maxFragmentSize(),
                          _channelInfo.maxOutputBuffers(), _channelInfo.guaranteedOutputBuffers(),
                          _channelInfo.numInputBuffers(),
                          _channelInfo.sysSendBufSize(), _channelInfo.sysRecvBufSize(),
                          _channelInfo.pingTimeout()
                );

        int count = _channelInfo.componentInfo().size();
        if (count == 0)
            System.out.printf("(No component info)");
        else
        {
            for (int i = 0; i < count; ++i)
            {
                System.out.print(_channelInfo.componentInfo().get(i).componentVersion());
                if (i < count - 1)
                    System.out.print(", ");
            }
        }

        System.out.printf("\n\n");

        // Check that we can successfully pack, if packing messages
        if (ProviderPerfConfig.totalBuffersPerPack() > 1 && ProviderPerfConfig.packingBufferLength() > _channelInfo.maxFragmentSize())
        {
            System.err.println("Error(Channel " + clientChannelInfo.channel.selectableChannel() + "): MaxFragmentSize " + _channelInfo.maxFragmentSize() + " is too small for packing buffer size " + ProviderPerfConfig.packingBufferLength() + "\n");
            return PerfToolsReturnCodes.FAILURE;
        }

        ProviderSession provSession = (ProviderSession)clientChannelInfo.userSpec;
        ret = provSession.printEstimatedMsgSizes(error);
        if (ret != PerfToolsReturnCodes.SUCCESS)
            return ret;

        provSession.timeActivated(System.nanoTime());

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Method called by ChannelHandler when a channel is closed.
     */
    public int processInactiveChannel(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, Error error)
    {
    	long inactiveTime = System.nanoTime();
        channelHandler.providerThread().getProvThreadInfo().stats().inactiveTime(inactiveTime);
        System.out.printf("processInactiveChannel(%d)", inactiveTime);
        
        _channelHandler.handlerLock().lock();
        --_connectionCount;
        _channelHandler.handlerLock().unlock();

        if (error != null)
            System.out.println("Channel Closed: " + error.text());
        else
            System.out.println("Channel Closed");

        ProviderSession provSession = (ProviderSession)clientChannelInfo.userSpec;
        if (provSession != null)
        {
            provSession.cleanup();
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Method called by ChannelHandler when UPA read() returns a buffer.
     */
    public int processMsg(ChannelHandler channelHandler, ClientChannelInfo clientChannelInfo, TransportBuffer msgBuf, Error error)
    {
        ProviderThread providerThread = channelHandler.providerThread();

        // clear decode iterator
        _decodeIter.clear();

        int ret = _decodeIter.setBufferAndRWFVersion(msgBuf, clientChannelInfo.channel.majorVersion(), clientChannelInfo.channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.err.println("DecodeIterator.setBufferAndRWFVersion() failed with return code:  " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        ret = _tmpMsg.decode(_decodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.err.println("Msg.decode() failed with return code:  " + CodecReturnCodes.toString(ret));
            error.errorId(ret);
            return PerfToolsReturnCodes.FAILURE;
        }

        switch (_tmpMsg.domainType())
        {
            case DomainTypes.LOGIN:
                ret = _loginProvider.processMsg(channelHandler, clientChannelInfo, _tmpMsg, _decodeIter, error);
                break;
            case DomainTypes.SOURCE:
                ret = _directoryProvider.processMsg(channelHandler, clientChannelInfo, _tmpMsg, _decodeIter, error);
                break;
            case DomainTypes.DICTIONARY:
                ret = _dictionaryProvider.processMsg(channelHandler, clientChannelInfo, _tmpMsg, _decodeIter, error);
                break;
            case DomainTypes.MARKET_PRICE:
                if (_xmlMsgData.marketPriceUpdateMsgCount() > 0)
                    ret = _itemRequestHandler.processMsg(providerThread, (ProviderSession)clientChannelInfo.userSpec, _tmpMsg, _directoryProvider.openLimit(), _directoryProvider.serviceId(), _directoryProvider.qos(), _decodeIter, error);
                else
                    ret = _itemRequestHandler.sendRequestReject(providerThread, (ProviderSession)clientChannelInfo.userSpec, _tmpMsg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, error);
                break;
            default:
                ret = _itemRequestHandler.sendRequestReject(providerThread, (ProviderSession)clientChannelInfo.userSpec, _tmpMsg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, error);
                break;
        }

        if (ret > PerfToolsReturnCodes.SUCCESS)
        {
            // The method sent a message and indicated that we need to flush.
            _channelHandler.requestFlush(clientChannelInfo);
        }

        return ret;
    }

    /**
     * Interactive provider thread's run method.
     */
    public void run()
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
        
        // create reactor
        if (ProviderPerfConfig.useReactor()) // use UPA VA Reactor
        {
            _reactorOptions.clear();
            if ((_reactor = ReactorFactory.createReactor(_reactorOptions, _errorInfo)) == null)
            {
                System.out.printf("Reactor creation failed: %s\n", _errorInfo.error().text());
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
        }
        
        _directoryProvider.serviceName(ProviderPerfConfig.serviceName());
        _directoryProvider.serviceId(ProviderPerfConfig.serviceId());
        _directoryProvider.openLimit(ProviderPerfConfig.openLimit());
        _directoryProvider.initService(_xmlMsgData);

        _loginProvider.initDefaultPosition();
        _loginProvider.applicationId(applicationId);
        _loginProvider.applicationName(applicationName);

        if (!_dictionaryProvider.loadDictionary(_error))
        {
            System.out.println("Error loading dictionary: " + _error.text());
            System.exit(CodecReturnCodes.FAILURE);
        }
        
        getProvThreadInfo().dictionary(_dictionaryProvider.dictionary());
        
        _reactorInitialized = true;

        // Determine update rates on per-tick basis
        long nextTickTime = initNextTickTime();
        
        // this is the main loop
        while (!shutdown())
        {
            if (!ProviderPerfConfig.useReactor()) // use UPA Channel
            {
            	if (nextTickTime <= currentTime())
            	{
            	    nextTickTime = nextTickTime(nextTickTime);
            		sendMsgBurst(nextTickTime);
            		_channelHandler.processNewChannels();
            	}
	        	_channelHandler.readChannels(nextTickTime, _error);
	        	_channelHandler.checkPings();
            }
            else // use UPA VA Reactor
            {
                if (nextTickTime <= currentTime())
                {
                    nextTickTime = nextTickTime(nextTickTime);
                    sendMsgBurst(nextTickTime);
                }

                Set<SelectionKey> keySet = null;

                // set select time 
                try
                {
                    int selectRetVal;
                    long selTime = (long)(selectTime(nextTickTime) / _divisor);
                    
                    if (selTime <= 0)
                        selectRetVal = _selector.selectNow();
                    else
                        selectRetVal = _selector.select(selTime);
                    
                    if (selectRetVal > 0)
                    {
                        keySet = _selector.selectedKeys();
                    }
                }
                catch (IOException e1)
                {
                    System.exit(CodecReturnCodes.FAILURE);
                }

                // nothing to read or write
                if (keySet == null)
                    continue;

                Iterator<SelectionKey> iter = keySet.iterator();
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    if(!key.isValid())
                        continue;
                    if (key.isReadable())
                    {
                        int ret;
                        
                        // retrieve associated reactor channel and dispatch on that channel 
                        ReactorChannel reactorChnl = (ReactorChannel)key.attachment();
                        
                        /* read until no more to read */
                        while ((ret = reactorChnl.dispatch(_dispatchOptions, _errorInfo)) > 0 && !shutdown()) {}
                        if (ret == ReactorReturnCodes.FAILURE)
                        {
                            if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                                reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
                            {
                                System.out.println("ReactorChannel dispatch failed");
                                reactorChnl.close(_errorInfo);
                                System.exit(CodecReturnCodes.FAILURE);
                            }
                        }
                    }
                }
            }
        }

        shutdownAck(true);
    }

    /**
     * Send refreshes and updates to open channels.
     * If an operation on channel returns unrecoverable error,
     * the channel is closed.
     * 
     */
    private void sendMsgBurst(long stopTime)
    {
       for (ClientChannelInfo clientChannelInfo : _channelHandler.activeChannelList())
       {
           ProviderSession providerSession = (ProviderSession)clientChannelInfo.userSpec;

           // The application corrects for ticks that don't finish before the time 
           // that the next update burst should start.  But don't do this correction 
           // for new channels.
           if(providerSession.timeActivated() == 0)
           {
               continue;
           }
         
           int ret = TransportReturnCodes.SUCCESS;

           // Send burst of updates
           if(ProviderPerfConfig.updatesPerSec() != 0 && providerSession.updateItemList().count() != 0)
           {
               ret = sendUpdateBurst(providerSession, _error);
               if (ret > TransportReturnCodes.SUCCESS)
               {
            	   // Need to flush
                   if (!ProviderPerfConfig.useReactor()) // use UPA Channel
                   {
                       _channelHandler.requestFlush(clientChannelInfo);
                   }
               }
           }
           
           // Send burst of generic messages
           if(ProviderPerfConfig.genMsgsPerSec() != 0 && providerSession.genMsgItemList().count() != 0)
           {
               ret = sendGenMsgBurst(providerSession, _error);
               if (ret > TransportReturnCodes.SUCCESS)
               {
            	   // Need to flush
                   if (!ProviderPerfConfig.useReactor()) // use UPA Channel
                   {
                       _channelHandler.requestFlush(clientChannelInfo);
                   }
               }
           }

           // Use remaining time in the tick to send refreshes.
           while(ret >= TransportReturnCodes.SUCCESS &&  providerSession.refreshItemList().count() != 0 && currentTime() < stopTime)
               ret = sendRefreshBurst(providerSession, _error);
           
           if(ret < TransportReturnCodes.SUCCESS)
           {
               switch(ret)
               {
                   case TransportReturnCodes.NO_BUFFERS:
                       if (!ProviderPerfConfig.useReactor()) // use UPA Channel
                       {
                           _channelHandler.requestFlush(clientChannelInfo);
                       }
                       break;
                   default:
                	   if (!Thread.interrupted())
                	   {
                		   System.out.printf("Failure while writing message bursts: %s (%d)\n", _error.text(), ret);
                		   if (!ProviderPerfConfig.useReactor()) // use UPA Channel
                           {
                		       _channelHandler.closeChannel(clientChannelInfo, _error); //Failed to send an update. Remove this client
                           }
                           else // use UPA VA Reactor
                           {
                               System.out.println("Channel Closed.");
                               
                               long inactiveTime = System.nanoTime();
                               getProvThreadInfo().stats().inactiveTime(inactiveTime);
                               
                               --_connectionCount;
                               
                               // unregister selectableChannel from Selector
                               try
                               {
                                   SelectionKey key = clientChannelInfo.reactorChannel.selectableChannel().keyFor(_selector);
                                   key.cancel();
                               }
                               catch (Exception e) { } // channel may be null so ignore
                               
                               if (providerSession.clientChannelInfo().parentQueue.size() > 0)
                               {
                                   providerSession.clientChannelInfo().parentQueue.remove(providerSession.clientChannelInfo());
                               }

                               clientChannelInfo.reactorChannel.close(_errorInfo); //Failed to send an update. Remove this client
                           }
                	   }
                       break;
               }
           }
           else if (ret > TransportReturnCodes.SUCCESS)
           {
               // need to flush
               if (!ProviderPerfConfig.useReactor()) // use UPA Channel
               {
                   _channelHandler.requestFlush(clientChannelInfo);
               }
           }
       }
    }
    
    /**
     * Clean up provider threads.
     */
    public void cleanup()
    {
    	super.cleanup();
        _channelHandler.cleanup();
    }
  
    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        ReactorChannel reactorChannel = event.reactorChannel();
        ProviderSession provSession = (ProviderSession)reactorChannel.userSpecObj();
        
        switch(event.eventType())
        {
            case ReactorChannelEventTypes.CHANNEL_UP:
            {                        
                // set the high water mark if configured
                if (ProviderPerfConfig.highWaterMark() > 0)
                {
                    if (reactorChannel.ioctl(IoctlCodes.HIGH_WATER_MARK, ProviderPerfConfig.highWaterMark(), _errorInfo) != TransportReturnCodes.SUCCESS)
                    {
                        System.out.println("ReactorChannel.ioctl() failed");
                        reactorChannel.close(_errorInfo);
                    }
                }
        
                // register selector with channel event's reactorChannel
                try
                {
                    reactorChannel.selectableChannel().register(_selector,
                                                                  SelectionKey.OP_READ,
                                                                  reactorChannel);
                }
                catch (ClosedChannelException e)
                {
                    System.out.println("selector register failed: " + e.getLocalizedMessage());
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
        
                /* retrieve and print out channel information */
                if (reactorChannel.info(_reactorChannnelInfo, _errorInfo) != TransportReturnCodes.SUCCESS)
                {
                    System.out.println("ReactorChannel.info() failed");
                    reactorChannel.close(_errorInfo);
                } 
                System.out.printf("Channel active. " + _reactorChannnelInfo.channelInfo().toString() + "\n");
        
                /* Check that we can successfully pack, if packing messages. */
                if (ProviderPerfConfig.totalBuffersPerPack() > 1
                        && ProviderPerfConfig.packingBufferLength() > _reactorChannnelInfo.channelInfo().maxFragmentSize())
                {
                    System.err.printf("Error(Channel %s): MaxFragmentSize %d is too small for packing buffer size %d\n",
                            reactorChannel.selectableChannel(), _reactorChannnelInfo.channelInfo().maxFragmentSize(), 
                            ProviderPerfConfig.packingBufferLength());
                    System.exit(-1);
                }
                                
                provSession.clientChannelInfo().reactorChannel = reactorChannel;
                provSession.clientChannelInfo().channel = reactorChannel.channel();
                provSession.clientChannelInfo().parentQueue = _channelHandler.activeChannelList();
                provSession.clientChannelInfo().parentQueue.add(provSession.clientChannelInfo());

                provSession.timeActivated(System.nanoTime());
                
                break;
            }
            case ReactorChannelEventTypes.CHANNEL_READY:
            {
                if(provSession.printEstimatedMsgSizes(_error) != PerfToolsReturnCodes.SUCCESS)
                {
                    System.out.println("_error.text()");
                    reactorChannel.close(_errorInfo);
                } 
                
                break;
            }
            case ReactorChannelEventTypes.FD_CHANGE:
            {
                System.out.println("Channel Change - Old Channel: "
                        + event.reactorChannel().oldSelectableChannel() + " New Channel: "
                        + event.reactorChannel().selectableChannel());
                
                // cancel old reactorChannel select
                try
                {
                    SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(_selector);
                    key.cancel();
                }
                catch (Exception e)
                {
                } // old channel may be null so ignore
    
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
            case ReactorChannelEventTypes.CHANNEL_DOWN:
            {
                System.out.println("Channel Closed.");
                
                long inactiveTime = System.nanoTime();
                getProvThreadInfo().stats().inactiveTime(inactiveTime);
                
                --_connectionCount;
                
                // unregister selectableChannel from Selector
                if (reactorChannel.selectableChannel() != null)
                {
                    SelectionKey key = reactorChannel.selectableChannel().keyFor(_selector);
                    if (key != null)
                    {
                        key.cancel();
                    }
                }
                
                if (provSession.clientChannelInfo().reactorChannel != null && provSession.clientChannelInfo().parentQueue.size() > 0)
                {
                    provSession.clientChannelInfo().parentQueue.remove(provSession.clientChannelInfo());
                }

                // close ReactorChannel
                if (reactorChannel != null)
                {
                    reactorChannel.close(_errorInfo);
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
        ReactorChannel reactorChannel = event.reactorChannel();
        ProviderSession provSession = (ProviderSession)reactorChannel.userSpecObj();
        ProviderThread providerThread = provSession.providerThread();
        
        Msg msg = event.msg();
        
        _decodeIter.clear();
        
        if (msg.encodedDataBody() != null && msg.encodedDataBody().data() != null)
        {
            _decodeIter.setBufferAndRWFVersion(msg.encodedDataBody(), reactorChannel.majorVersion(), reactorChannel.minorVersion());
        }
        
        switch (msg.domainType())
        {
            case DomainTypes.MARKET_PRICE:
                if (_xmlMsgData.marketPriceUpdateMsgCount() > 0)
                    _itemRequestHandler.processMsg(providerThread, provSession, msg, _directoryProvider.openLimit(), _directoryProvider.serviceId(), _directoryProvider.qos(), _decodeIter, event.errorInfo().error());
                else
                    _itemRequestHandler.sendRequestReject(providerThread, provSession, msg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, event.errorInfo().error());
                break;
            default:
                _itemRequestHandler.sendRequestReject(providerThread, provSession, msg, ItemRejectReason.DOMAIN_NOT_SUPPORTED, event.errorInfo().error());
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        ReactorChannel reactorChannel = event.reactorChannel();
        ProviderSession provSession = (ProviderSession)reactorChannel.userSpecObj();

        LoginMsg loginMsg = event.rdmLoginMsg();
        
        switch (loginMsg.rdmMsgType())
        {
            case REQUEST:
                //send login response
                LoginRequest loginRequest = (LoginRequest)loginMsg;
                loginRequest.copy(_loginProvider.loginRequest());
                _loginProvider.sendRefreshReactor(provSession.clientChannelInfo(), event.errorInfo().error());
                break;
            case CLOSE:
                System.out.println("Received Login Close for streamId " + loginMsg.streamId());
                break;
            default:
                event.errorInfo().error().text("Received Unhandled Login Msg Class: " + event.msg().msgClass());
                event.errorInfo().error().errorId(PerfToolsReturnCodes.FAILURE);
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        ReactorChannel reactorChannel = event.reactorChannel();
        ProviderSession provSession = (ProviderSession)reactorChannel.userSpecObj();

        DirectoryMsg directoryMsg = event.rdmDirectoryMsg();
        
        switch (directoryMsg.rdmMsgType())
        {
            case REQUEST:
                DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsg;
                directoryRequest.copy(_directoryProvider.directoryRequest());
                System.out.println("Received Source Directory Request");
                // send source directory response
                _directoryProvider.sendRefreshReactor(provSession.clientChannelInfo(), event.errorInfo().error());
                break;
            case CLOSE:
                System.out.println("Received Directory Close for streamId " + directoryMsg.streamId());
                break;
            default:
                event.errorInfo().error().text("Received unhandled Source Directory msg type: " + event.msg().msgClass());
                event.errorInfo().error().errorId(PerfToolsReturnCodes.FAILURE);
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        ReactorChannel reactorChannel = event.reactorChannel();
        ProviderSession provSession = (ProviderSession)reactorChannel.userSpecObj();
        
        DictionaryMsg dictionaryMsg = event.rdmDictionaryMsg();
        
        switch (dictionaryMsg.rdmMsgType())
        {
            case REQUEST:
                DictionaryRequest dictionaryRequest = (DictionaryRequest)dictionaryMsg;
                dictionaryRequest.copy(_dictionaryProvider.dictionaryRequest());
                System.out.println("Received Dictionary Request for DictionaryName: " + dictionaryRequest.dictionaryName());
                if (_dictionaryProvider.fieldDictionaryDownloadName().equals(dictionaryRequest.dictionaryName()))
                {
                    _dictionaryProvider.sendFieldDictionaryResponseReactor(provSession.clientChannelInfo(), event.errorInfo().error());
                }
                else if (_dictionaryProvider.enumTypeDictionaryDownloadName().equals(dictionaryRequest.dictionaryName()))
                {
                    _dictionaryProvider.sendEnumTypeDictionaryResponseReactor(provSession.clientChannelInfo(), event.errorInfo().error());
                }
                else
                {
                    _dictionaryProvider.sendRequestRejectReactor(provSession.clientChannelInfo(), dictionaryMsg.streamId(), DictionaryRejectReason.UNKNOWN_DICTIONARY_NAME, event.errorInfo().error());
                }
                break;
            case CLOSE:
                System.out.println("Received Dictionary Close for streamId " + dictionaryMsg.streamId());
                break;

            default:
                event.errorInfo().error().text("Received unhandled Source Directory msg type: " + event.msg().msgClass());
                event.errorInfo().error().errorId(PerfToolsReturnCodes.FAILURE);
        }

        return ReactorCallbackReturnCodes.SUCCESS;
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
    
    Lock handlerLock()
    {
    	return _channelHandler.handlerLock();
    }
}