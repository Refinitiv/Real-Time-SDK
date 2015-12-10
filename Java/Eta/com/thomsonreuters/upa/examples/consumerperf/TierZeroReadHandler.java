package com.thomsonreuters.upa.examples.consumerperf;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

public class TierZeroReadHandler implements Runnable, LoginSuccessCallback
{
    /**
     * The default latency stream ID
     */
    public static final int DEFAULT_LATENCY_STREAM_ID = 6;    
    
	final ConsumerInfo consInfo;
	final LoginConsumer _loginConsumer;
	final DirectoryHandler _srcDirHandler;
	final ItemHandler _itemHandler;
	
	public TierZeroReadHandler(ConsumerInfo consInfo)
	{
		this.consInfo = consInfo;
		
		_loginConsumer = new LoginConsumer();
		_loginConsumer.username(System.getProperty("user.name"));
		
		_srcDirHandler = new DirectoryHandler();

		_itemHandler = new ItemHandler();
		addItems();
	}
	
	private void addItems()
	{
	    // add latency items
	    if (this.consInfo.latencyItems != null)
	    {
	        for (Item latencyItem : this.consInfo.latencyItems)
	        {
	            _itemHandler.addItemName(latencyItem.name());
	        }
	    }
	    
	    // add "regular" items
        if (this.consInfo.items != null)
        {
            int maxItemsToSend = this.consInfo.itemCount - 1;
            int totalItems = this.consInfo.items.size();
            
            for (int i = 0; (i < maxItemsToSend && i < totalItems); i++)
            {
                _itemHandler.addItemName(this.consInfo.items.get(i).name());
            }
        }
	}

    public void run()
    {
        Error error = TransportFactory.createError();
        Channel chnl = null;
        ConnectOptions copts = TransportFactory.createConnectOptions();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        LocalFieldSetDefDb localSetDb = CodecFactory.createLocalFieldSetDefDb();
        int  ret = 0;

        consInfo.fieldDictionary = dictionary;
        consInfo.fListSetDef = localSetDb;
        
        /* Load dictionary from file if possible. Otherwise die. */
        dictionary.clear();
        localSetDb.clear();
        if (consInfo.fieldDictionary.loadFieldDictionary("RDMFieldDictionary", error) < 0)
        {
            System.out.println("\nUnable to load dictionary. Will not decode fully.\tText: " + error.text());
            System.exit(-1);
        }

        if (consInfo.fieldDictionary.loadEnumTypeDictionary("enumtype.def", error) < 0)
        {
            System.out.println("\nUnable to load enum type dictionary.  Will not decode fully.\tText: " + error.text());
            System.exit(-1);
        }

        consInfo.latStreamId = DEFAULT_LATENCY_STREAM_ID;
        
        /* set service name in directory handler */
        _srcDirHandler.serviceName(consInfo.serviceName);
                
        System.out.println("Starting connection...");

        /* this is the connection recovery loop */
        while(consInfo.channelActive != true && consInfo.shutdown == false)
        {
            /* Connect to server */
            System.out.println("\nAttempting to connect to server " + consInfo.srvrHostname + ":" + consInfo.srvrPortNo + "...");
    
            if(consInfo.interfacePresent)
                copts.unifiedNetworkInfo().interfaceName(consInfo.interfaceName);

            copts.guaranteedOutputBuffers(consInfo.guaranteedOutputBuffers);
            copts.numInputBuffers(consInfo.numInputBuffers);
            copts.unifiedNetworkInfo().address(consInfo.srvrHostname);
            copts.unifiedNetworkInfo().serviceName(consInfo.srvrPortNo);
            copts.majorVersion(Codec.majorVersion());
            copts.minorVersion(Codec.minorVersion());
            copts.protocolType(Codec.protocolType());
            copts.connectionType(consInfo.connectionType);
            
            if ((consInfo.consumerChannel = Transport.connect(copts,error)) != null)
            {
                chnl = consInfo.consumerChannel;
                System.out.println("\nChannel " + chnl.selectableChannel());
            }
            
            if(chnl != null && chnl.state() == ChannelState.ACTIVE)
            {
                System.out.println("Active!");
                consInfo.channelActive = true;
                break;
            }

            /* Wait for channel to become active.  This finalizes the three-way handshake. */
            while (chnl != null && chnl.state() != ChannelState.ACTIVE)
            {
                if (chnl.state() == ChannelState.INITIALIZING)
                {
                    if ((ret = chnl.init(inProg, error)) < TransportReturnCodes.SUCCESS)
                    {
                        return;
                    }
                    else 
                    {
                        switch (ret)
                        {
                        case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            if (inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                            {
                                System.out.println("\nChannel In Progress - New Channel: " + chnl.selectableChannel() + " Old Channel: " + inProg.oldSelectableChannel());
                            }
                            //else
                            //{
                            //  System.out.println("\nChannel " + chnl.getScktChannel() + " In Progress...");
                            //}
                            break;
                        case TransportReturnCodes.SUCCESS:
                            System.out.println("\nChannel " + chnl.selectableChannel() + " Is Active");
                            /* reset should recover connection flag */
                            consInfo.channelActive = true;
                            break;
                        default:
                            System.out.println("\nBad return value channel=" + chnl.selectableChannel() + " <" + error.text() + ">");
                            return;
                        }
                    }
                }
            }
        }
        
        // if shutdown, return here
        if (consInfo.shutdown == true)
        {
        	return;
        }
        
        if (chnl.pingTimeout() != 0)
        {
            consInfo.pingTimeoutClient = chnl.pingTimeout() / 3;
        }

        try
        {
            sendLoginRequest(chnl);
            readLoop(chnl);
        }
        catch(Exception e)
        {
            System.err.println("Caught an unexpected exception: " + e.toString());
            e.printStackTrace(System.err);
            System.err.println("Exiting");
            System.exit(1);
        }
        finally
        {
            System.out.println("Exiting Read Handler");
        }
        
    }

	
	private void readLoop(Channel chnl)
	{
        TransportBuffer msgBuf = null;
        ReadArgs readArgs = TransportFactory.createReadArgs();
        Msg msg = CodecFactory.createMsg();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        PerfMarketPriceHandler mpHandler = new PerfMarketPriceHandler();
        PerfMarketByOrderHandler mboHandler = new PerfMarketByOrderHandler();
        Error error = TransportFactory.createError();
        int ret = 0;
        int requestThrottle = 0;
	    
        while (consInfo.shutdown != true)
        {
            do
            {
                if ((msgBuf = chnl.read(readArgs, error)) != null)
                {
                    consInfo.stats.totalMsgs++;
                    /* clear decode iterator */
                    dIter.clear();

                    /* set version info */
                    dIter.setBufferAndRWFVersion(msgBuf, chnl.majorVersion(),
                                                 chnl.minorVersion());

                    ret = msg.decode(dIter);

                    if (ret != TransportReturnCodes.SUCCESS)
                    {
                        System.out.println("ERROR: msg.decode failed (" + ret + "). Exiting.");
                        System.exit(-1);
                    }

                    switch (msg.domainType())
                    {
                        case DomainTypes.MARKET_PRICE:
                        	if (msg.msgClass() == MsgClasses.REFRESH)
                        	{
                        		consInfo.imagesReceived++;
                        	}
                            mpHandler.decodeUpdate(consInfo.decodeType, dIter, msg, consInfo);
                            requestThrottle++;
                            if (requestThrottle % 1000 == 0 && _itemHandler.itemCount() > 0)
                            {
                            	_itemHandler.sendItemRequests(chnl, _srcDirHandler.serviceId(), _loginConsumer.responseInfo(), _srcDirHandler.responseInfo());
                            }
                            break;
                        case DomainTypes.MARKET_BY_ORDER:
                        	if (msg.msgClass() == MsgClasses.REFRESH)
                        	{
                        		consInfo.imagesReceived++;
                        	}
                            mboHandler.decodeUpdate(consInfo.decodeType, dIter, msg, consInfo);
                            requestThrottle++;
                            if (requestThrottle % 1000 == 0 && _itemHandler.itemCount() > 0)
                            {
                            	_itemHandler.sendItemRequests(chnl, _srcDirHandler.serviceId(), _loginConsumer.responseInfo(), _srcDirHandler.responseInfo());
                            }
                            break;
                        case DomainTypes.LOGIN:
                            _loginConsumer.processResponse(chnl, msg, dIter);
                            break;
                        case DomainTypes.SOURCE:
                            _srcDirHandler.processResponse(chnl, msg, dIter);
                            _itemHandler.sendItemRequests(chnl, _srcDirHandler.serviceId(), _loginConsumer.responseInfo(), _srcDirHandler.responseInfo());
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    switch (readArgs.readRetVal())
                    {
                        case TransportReturnCodes.FAILURE:
                            System.out.println("FAILURE: " + error.text());
                            return;
                        case TransportReturnCodes.READ_FD_CHANGE:
                            System.out.println("\nRead() Channel Change - Old Channel: "
                                    + chnl.oldSelectableChannel() + " New Channel: "
                                    + chnl.selectableChannel());
                            break;
                        case TransportReturnCodes.READ_PING:
                            System.out.println("Read ping");
                            break;
                        default:
                            if (readArgs.readRetVal() < 0
                                    && readArgs.readRetVal() != TransportReturnCodes.READ_WOULD_BLOCK)
                            {
                                System.out.println("\nreadArgs.getReadRetVal(): "
                                        + readArgs.readRetVal());
                            }
                            break;
                    }
                }
            }
            while (readArgs.readRetVal() > 0);

        }
	}
	
    /**
     * Send Login Request to the Provider.
     * @return null on success or a String representing an error message.
     * @see {@link Consumer#readAndProcessResponse(int)}
     */
    private void sendLoginRequest(Channel chnl)
    {
        /* Send login request message */
        int retVal;
        if ((retVal = _loginConsumer.sendRequest(chnl,
                                                 "ConsumerPerf",
                                                 LoginConsumer.CONSUMER,
                                                 this)) != CodecReturnCodes.SUCCESS)
        {
            System.out.println("failed to send Login request, retVal=" + retVal
                    + " responseInfo=" + _loginConsumer.responseInfo().toString());
            System.exit(-1);
        }
    }
    
    private void sendSrcDirRequest(Channel chnl)
    {
        if (_srcDirHandler.sendRequest(chnl) != CodecReturnCodes.SUCCESS)
        {
            System.out.println("failed to send source directory request");
            System.exit(-1);
        }        
    }

    /**
     * Called upon successful login. Sends source directory request to the
     * channel
     * 
     * @param chnl the channel on which the login request was sent
     */
    public void loginSucceeded(Channel chnl)
    {
        sendSrcDirRequest(chnl);
    }
}
