package com.thomsonreuters.upa.perftools.upajprovperf;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.perftools.common.ItemRejectReason;
import com.thomsonreuters.upa.perftools.common.ChannelHandler;
import com.thomsonreuters.upa.perftools.common.ClientChannelInfo;
import com.thomsonreuters.upa.perftools.common.DictionaryProvider;
import com.thomsonreuters.upa.perftools.common.PerfToolsReturnCodes;
import com.thomsonreuters.upa.perftools.common.ProviderPerfConfig;
import com.thomsonreuters.upa.perftools.common.ProviderSession;
import com.thomsonreuters.upa.perftools.common.ProviderThread;
import com.thomsonreuters.upa.perftools.common.XmlMsgData;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.IoctlCodes;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/**
 * Interactive provider implementation of the provider thread callback. Handles
 * accepting of new channels, processing of incoming messages and sending of
 * message bursts.
 */
public class IProviderThread extends ProviderThread
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

    {
        _decodeIter = CodecFactory.createDecodeIterator();
        _tmpMsg = CodecFactory.createMsg();
        _loginProvider = new LoginProvider();
        _directoryProvider = new IDirectoryProvider();
        _dictionaryProvider = new DictionaryProvider();
        _channelInfo = TransportFactory.createChannelInfo();
        _itemRequestHandler = new ItemRequestHandler();
        _channelHandler = new ChannelHandler(this);
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
            provSession.init(_channelHandler.addChannel(channel, provSession, true));
    }
    
    /**
     * Number of client sessions currently connected.
     */
    public int connectionCount()
    {
       return _channelHandler.initializingChannelList().size() + _channelHandler.activeChannelList().size();
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

        // Determine update rates on per-tick basis
        long nsecPerTick = 1000000000 / ProviderPerfConfig.ticksPerSec();
        long nextTickTime = System.nanoTime() + nsecPerTick;

        // this is the main loop
        while (!shutdown())
        {
        	if (nextTickTime <= System.nanoTime())
        	{
        		nextTickTime += nsecPerTick;
        		sendMsgBurst(nextTickTime);
        		_channelHandler.processNewChannels();
        	}
    		_channelHandler.readChannels(nextTickTime, _error);
    		_channelHandler.checkPings();
        }

        shutdownAck(true);
    }

    /**
     * Send refreshes and updates to open channels.
     * If an operation on channel returns unrecoverable error,
     * the channel is closed.
     * 
     */
    private void sendMsgBurst(long stopTimeNSec)
    {
       for(ClientChannelInfo clientChannelInfo : _channelHandler.activeChannelList())
       {
           ProviderSession providerSession = (ProviderSession)clientChannelInfo.userSpec;

           // The application corrects for ticks that don't finish before the time 
           // that the next update burst should start.  But don't do this correction 
           // for new channels.
           if(stopTimeNSec < providerSession.timeActivated())
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
            	   _channelHandler.requestFlush(clientChannelInfo);
               }
           }
           
           // Send burst of generic messages
           if(ProviderPerfConfig.genMsgsPerSec() != 0 && providerSession.genMsgItemList().count() != 0)
           {
               ret = sendGenMsgBurst(providerSession, _error);
               if (ret > TransportReturnCodes.SUCCESS)
               {
            	   // Need to flush
            	   _channelHandler.requestFlush(clientChannelInfo);
               }
           }

           // Use remaining time in the tick to send refreshes.
           while( ret >= TransportReturnCodes.SUCCESS &&  providerSession.refreshItemList().count() != 0 && System.nanoTime() < stopTimeNSec)
               ret = sendRefreshBurst(providerSession, _error);
           
           if(ret < TransportReturnCodes.SUCCESS)
           {
               switch(ret)
               {
                   case TransportReturnCodes.NO_BUFFERS:
                       _channelHandler.requestFlush(clientChannelInfo);
                       break;
                   default:
                	   if (!Thread.interrupted())
                	   {
                		   System.out.printf("Failure while writing message bursts: %s (%d)\n", _error.text(), ret);
                	        _channelHandler.closeChannel(clientChannelInfo, _error); //Failed to send an update. Remove this client
                	   }
                       break;
               }
           }
           else if (ret > TransportReturnCodes.SUCCESS)
           {
               // need to flush
               _channelHandler.requestFlush(clientChannelInfo);
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
  
}
