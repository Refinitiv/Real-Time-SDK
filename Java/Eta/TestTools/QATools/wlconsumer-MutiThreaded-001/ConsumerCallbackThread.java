/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2024-2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

//APIQA
package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.refinitiv.eta.codec.AckMsg;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.rdm.Dictionary.VerbosityValues;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;
import com.refinitiv.eta.valueadd.examples.common.ItemArg;
import com.refinitiv.eta.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig.ItemInfo;
import com.refinitiv.eta.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig;
import com.refinitiv.eta.valueadd.reactor.ConsumerCallback;
import com.refinitiv.eta.valueadd.reactor.RDMDictionaryMsgEvent;
import com.refinitiv.eta.valueadd.reactor.RDMDirectoryMsgEvent;
import com.refinitiv.eta.valueadd.reactor.RDMLoginMsgEvent;
import com.refinitiv.eta.valueadd.reactor.Reactor;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEventTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorDispatchOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorMsgEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorOptions;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/**
 * <p>
 * This interface implements callbacks that process information
 * received by the provider. It creates the Reactor, creates the desired
 * connections, then dispatches from the Reactor for events and messages.
 * Once it has received the event indicating that the channel is ready,
 * it will trigger ConsumerRequestThread to send the desired item requests (snapshot or streaming) to a
 * provider. The resulting decoded responses from the provided are displayed on the console.
 * </p>
 */
public class ConsumerCallbackThread implements Runnable, ConsumerCallback
{
    private final String FIELD_DICTIONARY_DOWNLOAD_NAME = "RWFFld";
    private final String ENUM_TABLE_DOWNLOAD_NAME = "RWFEnum";
    private final String FIX_FIELD_DICTIONARY_FILE_NAME = "FDMFixFieldDictionary";
    private final String FIX_ENUM_TABLE_FILE_NAME = "FDMenumtypes.def";
    
    private Reactor reactor;
    private ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
    private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
    WatchlistConsumerConfig watchlistConsumerConfig;
    private Selector selector;
    
    private Error error;    // error information
    
    private DataDictionary fixdictionary;
    
	ArrayList<ChannelInfo> chnlInfoList = new ArrayList<ChannelInfo>();

    TunnelStreamHandler tunnelStreamHandler;
    private String tsServiceName;
   
	StringBuilder cacheDisplayStr;
	Buffer cacheEntryBuffer;
	
	boolean _finalStatusEvent;
	boolean closeHandled;

	private int FIELD_DICTIONARY_STREAM_ID = 3;
	private int ENUM_DICTIONARY_STREAM_ID = 4;
	
	ItemDecoder itemDecoder;
	boolean itemsRequested = false;

    public static final int MAX_MSG_SIZE = 1024;

	public static int TRANSPORT_BUFFER_SIZE_CLOSE = MAX_MSG_SIZE;

    boolean fieldDictionaryLoaded;
    boolean enumDictionaryLoaded;
    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
    private CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
    
    boolean shutDown = false;
	ChannelInfo chnlInfo;
	WatchlistConsumer mtWatchlistConsumer;
	ConsumerRequestThread consumerReqeustThread;
    
    public ConsumerCallbackThread(ChannelInfo channelInfo, WatchlistConsumer consumer)
    {
        fixdictionary = CodecFactory.createDataDictionary();
        
        error = TransportFactory.createError();
      
        dispatchOptions.maxMessages(1);
        _finalStatusEvent = true;

 		itemDecoder = new ItemDecoder();
                        
        itemsRequested = false;
 
        chnlInfo = channelInfo;
        mtWatchlistConsumer = consumer;
        consumerReqeustThread = new ConsumerRequestThread(chnlInfo, this);
        watchlistConsumerConfig = mtWatchlistConsumer.watchlistConsumerConfig();
        
        try
        {
            selector = Selector.open();
        }
        catch (Exception e)
        {
        	System.out.println("Selector.open() failed: " + e.getLocalizedMessage());
        	System.exit(ReactorReturnCodes.FAILURE);
        }
    }

    /* Initializes the Value Add consumer application. */
    private void init()
    {
        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("ConsumerCallbackThread " + this.hashCode() + " initializing...");
  
        // enable Reactor XML tracing if specified
        if (watchlistConsumerConfig.enableXmlTracing())
        {
        	reactorOptions.enableXmlTracing();
        }
         
        itemDecoder.init();
        
		// create reactor
	    reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
	    if (errorInfo.code() != ReactorReturnCodes.SUCCESS)
	    {
        	System.out.println("createReactor() failed: " + errorInfo.toString());
        	System.exit(ReactorReturnCodes.FAILURE);	    	
	    }
	    
        // register selector with reactor's reactorChannel.
        try
        {
			reactor.reactorChannel().selectableChannel().register(selector,
																SelectionKey.OP_READ,
																reactor.reactorChannel());
		}
        catch (ClosedChannelException e)
        {
        	System.out.println("selector register failed: " + e.getLocalizedMessage());
        	System.exit(ReactorReturnCodes.FAILURE);
		}
        
    	// initialize channel info
    	initChannelInfo(chnlInfo);   
    	
        // connect channel
        int ret;
        if ((ret = reactor.connect(chnlInfo.connectOptions, chnlInfo.consumerRole, errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
        	System.out.println("Reactor.connect failed with return code: " + ret + " error = " + errorInfo.error().text());
        	System.exit(ReactorReturnCodes.FAILURE);
        }
        
		new Thread(consumerReqeustThread).start();
    }

    /* Runs the Value Add consumer application. */
    public void run()		
	{		
    	// initialize the test data from configuration and xml files
		init();

		int selectRetVal, selectTime = 1000;
		while (true)
		{
	        Set<SelectionKey> keySet = null;
	        try
	        {
	        	selectRetVal = selector.select(selectTime);
	            if (selectRetVal > 0)
	            {
	                keySet = selector.selectedKeys();
	            }
	        }
	        catch (IOException e)
	        {
	        	System.out.println("select failed: " + e.getLocalizedMessage());
	        	System.exit(ReactorReturnCodes.FAILURE);
	        }
	
	        // nothing to read
	        if (keySet != null)
			{
				Iterator<SelectionKey> iter = keySet.iterator();
				int ret = ReactorReturnCodes.SUCCESS;
				while (iter.hasNext())
				{
					SelectionKey key = iter.next();
					iter.remove();
					try
					{
						if (key.isReadable())
						{
							// retrieve associated reactor channel and dispatch on that channel 
							ReactorChannel reactorChnl = (ReactorChannel)key.attachment();


							// dispatch until no more messages
							while ((ret = reactorChnl.dispatch(dispatchOptions, errorInfo)) > 0) {}
							if (ret == ReactorReturnCodes.FAILURE)
							{
								if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
										reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
								{
									System.out.println("ReactorChannel dispatch failed: " + ret + "(" + errorInfo.error().text() + ")");
									uninitialize();
									System.exit(ReactorReturnCodes.FAILURE);
								}
							}
						}
					}
					catch (CancelledKeyException e)
					{
					} // key can be canceled during shutdown
				}
			}
	        
            // Handle run-time
            if (shutDown && !closeHandled)
            {
                System.out.println("ConsumerCallbackThread  " + this.hashCode() + " closing now...");
                mtWatchlistConsumer.shutDown = true;
                handleClose();
                closeHandled = true;                
            }
           
	        if (!closeHandled)
	        {
	        	handlePosting();
	        	handleTunnelStream();
	        	
		        // send login reissue if login reissue time has passed
        		if (chnlInfo.reactorChannel == null ||
        	    	(chnlInfo.reactorChannel.state() != ReactorChannel.State.UP && 
        	    	chnlInfo.reactorChannel.state() != ReactorChannel.State.READY))
    	    	{
    	    		continue;
    	    	}
        		
        		if (chnlInfo.canSendLoginReissue &&
        			System.currentTimeMillis() >= chnlInfo.loginReissueTime)
        		{
					LoginRequest loginRequest = chnlInfo.consumerRole.rdmLoginRequest();
					submitOptions.clear();
					if (chnlInfo.reactorChannel.submit(loginRequest, submitOptions, errorInfo) !=  CodecReturnCodes.SUCCESS)
					{
						System.out.println("Login reissue failed. Error: " + errorInfo.error().text());
					}
					else
					{
						System.out.println("Login reissue sent");
					}
					chnlInfo.canSendLoginReissue = false;
        		}
	        }	        
	        		 
	        if(closeHandled && tunnelStreamHandler != null && tunnelStreamHandler._chnlInfo != null &&
	           !tunnelStreamHandler._chnlInfo.isTunnelStreamUp) 
	        	break;
		}	
		
		try
		{
			Thread.sleep(3000);
		}
		catch (InterruptedException e)
		{
			System.out.printf("Thread.sleep(1000) failed\n");
			System.exit(-1);
		}
		
		if (closeHandled)
			uninitialize();
	}

	private void requestDictionaries(ReactorChannel channel, ChannelInfo chnlInfo)
	{
		RequestMsg msg = (RequestMsg) CodecFactory.createMsg();

		/* set-up message */
		msg.msgClass(MsgClasses.REQUEST);
		
		msg.applyStreaming();
		msg.streamId(FIELD_DICTIONARY_STREAM_ID);
		chnlInfo.fieldDictionaryStreamId = FIELD_DICTIONARY_STREAM_ID;
		msg.domainType(DomainTypes.DICTIONARY);
		msg.containerType(DataTypes.NO_DATA);
		msg.msgKey().applyHasNameType();
		msg.msgKey().applyHasName();
		msg.msgKey().applyHasFilter();		
		msg.msgKey().filter(VerbosityValues.NORMAL);		
		msg.msgKey().name().data(FIELD_DICTIONARY_DOWNLOAD_NAME);

		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		submitOptions.serviceName(watchlistConsumerConfig.serviceName());
		
        channel.submit(msg, submitOptions, errorInfo);
         
 		msg.streamId(ENUM_DICTIONARY_STREAM_ID);
 		chnlInfo.enumDictionaryStreamId = ENUM_DICTIONARY_STREAM_ID;
 		msg.msgKey().name().data(ENUM_TABLE_DOWNLOAD_NAME);
 		
        channel.submit(msg, submitOptions, errorInfo);
		
	}

    @Override
	public int reactorChannelEventCallback(ReactorChannelEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		
		switch(event.eventType())
		{				
    		case ReactorChannelEventTypes.CHANNEL_UP:
    		{
    			if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("Channel Up Event: " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("Channel Up Event");
    	        // register selector with channel event's reactorChannel
    	        try
    	        {
    				event.reactorChannel().selectableChannel().register(selector,
    																	SelectionKey.OP_READ,
    																	event.reactorChannel());
    			}
    	        catch (ClosedChannelException e)
    	        {
    	        	System.out.println("selector register failed: " + e.getLocalizedMessage());
    	        	return ReactorCallbackReturnCodes.SUCCESS;
    			}    			
 
    	        break;
    		}
    		case ReactorChannelEventTypes.FD_CHANGE:
    		{  
    	        System.out.println("Channel Change - Old Channel: "
    	                + event.reactorChannel().oldSelectableChannel() + " New Channel: "
    	                + event.reactorChannel().selectableChannel());
    	        
    	        // cancel old reactorChannel select
                SelectionKey key = event.reactorChannel().oldSelectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();
    
    	        // register selector with channel event's new reactorChannel
    	        try
    	        {
    	        	event.reactorChannel().selectableChannel().register(selector,
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
    			if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("Channel Ready Event: " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("Channel Ready Event");
    			if (isRequestedServiceUp(chnlInfo))
    			{
        			checkAndInitPostingSupport(chnlInfo); 
    			}
    			
                break;   
                
			}
			case ReactorChannelEventTypes.CHANNEL_OPENED:
    		{
    			// set ReactorChannel on ChannelInfo, again need this?
    			chnlInfo.reactorChannel = event.reactorChannel();

				if (!(fieldDictionaryLoaded && enumDictionaryLoaded) &&
						!(itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile))
	                requestDictionaries(event.reactorChannel(), chnlInfo);
				else
					itemsRequested = false;
    			
                break;
    		}
    		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
    		{
    			if (event.reactorChannel().selectableChannel() != null)
                    System.out.println("\nConnection down reconnecting: Channel " + event.reactorChannel().selectableChannel());
                else
                    System.out.println("\nConnection down reconnecting");
    
    			if (event.errorInfo() != null && event.errorInfo().error().text() != null)
    				System.out.println("	Error text: " + event.errorInfo().error().text() + "\n");
    						    			
                // unregister selectableChannel from Selector
                if (event.reactorChannel().selectableChannel() != null)
                {
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }
    			
    			// reset dictionary if not loaded from file
    	        if (itemDecoder.fieldDictionaryLoadedFromFile == false &&
    	            itemDecoder.enumTypeDictionaryLoadedFromFile == false)
    	        {
    	        	if (chnlInfo.dictionary != null)
    	        	{
    	        		chnlInfo.dictionary.clear();
    	        	}
    	        }
    	            	        
    	        itemsRequested = false;
    	        chnlInfo.hasServiceInfo = false;
    	        chnlInfo.hasTunnelStreamServiceInfo = false;
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
                    SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                    if (key != null)
                        key.cancel();
                }

                // close ReactorChannel
                if (chnlInfo.reactorChannel != null)
                {
                    chnlInfo.reactorChannel.close(errorInfo);
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
	    String itemName = null;
	    State itemState = null;
	    ItemInfo item = null;

		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		Msg msg = event.msg();
		
        if (msg == null)
        {
            /* The message is not present because an error occurred while decoding it. Print 
             * the error and close the channel. If desired, the un-decoded message buffer
             * is available in event.transportBuffer(). */
            System.out.printf("defaultMsgCallback: %s(%s)\n", event.errorInfo().error().text(), event.errorInfo().location());
            // unregister selectableChannel from Selector
            if (event.reactorChannel().selectableChannel() != null)
            {
                SelectionKey key = event.reactorChannel().selectableChannel().keyFor(selector);
                if (key != null)
                    key.cancel();
            }

            // close ReactorChannel
            if (chnlInfo.reactorChannel != null)
            {
                chnlInfo.reactorChannel.close(errorInfo);
            }
            return ReactorCallbackReturnCodes.SUCCESS;
        }
        
        item = watchlistConsumerConfig.getItemInfo(msg.streamId());

        switch (msg.msgClass())
        {
            case MsgClasses.REFRESH:
            	
                RefreshMsg refreshMsg = (RefreshMsg)msg; 
                if ( refreshMsg.checkHasMsgKey())
                {
                	if (refreshMsg.msgKey().checkHasName())
                	{
                		itemName = refreshMsg.msgKey().name().toString(); // Buffer
                	    if ( item == null && refreshMsg.streamId() < 0)
                	    {
                	    	watchlistConsumerConfig.addProvidedItemInfo(refreshMsg.streamId(), refreshMsg.msgKey(), 
                	    			refreshMsg.domainType());
                	    }
                	}
                }
                else if (item != null)
                {
                	itemName = item.name();
                }
                   				
    		    System.out.println("DefaultMsgCallback Refresh ItemName: " + itemName + " Domain: " + DomainTypes.toString(refreshMsg.domainType()) + ", StreamId: " + refreshMsg.streamId());

				System.out.println("                      State: "  + refreshMsg.state());
 
				itemState = refreshMsg.state();
    			/* Decode data body according to its domain. */
    			itemDecoder.decodeDataBody(event.reactorChannel(), refreshMsg);            	
            	break;
                               
            case MsgClasses.UPDATE:
                      
                UpdateMsg updateMsg = (UpdateMsg)msg; 
                if (updateMsg.checkHasMsgKey() && updateMsg.msgKey().checkHasName())
                {
                    itemName = updateMsg.msgKey().name().toString();
                }
                else if (item != null)
                {
                	itemName = item.name();
                }
                
    		    System.out.println("DefaultMsgCallback Update ItemName: " + itemName + " Domain: " + DomainTypes.toString(updateMsg.domainType()) + ", StreamId: " + updateMsg.streamId());

    			/* Decode data body according to its domain. */
    			itemDecoder.decodeDataBody(event.reactorChannel(), updateMsg);            	
            	break;

            case MsgClasses.STATUS:
                StatusMsg statusMsg = (StatusMsg)msg; 
                if (statusMsg.checkHasMsgKey())
                {
                	if (statusMsg.msgKey().checkHasName())
                	{
                		itemName = statusMsg.msgKey().name().toString();
                   	    if ( item != null && statusMsg.streamId() < 0)
                	    {
                	    	watchlistConsumerConfig.addProvidedItemInfo(statusMsg.streamId(), statusMsg.msgKey(), 
                	    			statusMsg.domainType());
                	    }
                	}
                }
                else if (item != null)
                {
                	itemName = item.name();
                }
                 				
    		    System.out.println("DefaultMsgCallback Status -- ItemName: " + itemName + " Domain: " + DomainTypes.toString(statusMsg.domainType()) + ", StreamId: " + statusMsg.streamId());

    		    if ( statusMsg.checkHasState())
    		    {
    				System.out.println(statusMsg.state());
    				 
    				itemState = statusMsg.state();
    		    } 
    		    
            	break;
            case MsgClasses.ACK:
            	
                AckMsg ackMsg = (AckMsg)msg; 
                if (ackMsg.checkHasMsgKey())
                {
                	if (ackMsg.msgKey().checkHasName())
                	{
                		itemName = ackMsg.msgKey().name().toString();
                	}
                }
                else if (item != null)
                {
                	itemName = item.name();
                }
    		    System.out.println("DefaultMsgCallback Ack --  ItemName: " + itemName + " Domain: " + DomainTypes.toString(ackMsg.domainType()) + ", StreamId: " + ackMsg.streamId());
    		    System.out.println(" ackId: " + ackMsg.ackId());
    		    if ( ackMsg.checkHasSeqNum())
    		    {
    		    	 System.out.println(" seqNum: " + ackMsg.seqNum()); 
    		    }
    		    if ( ackMsg.checkHasNakCode())
    		    {
    		    	 System.out.println(" nakCode: " + ackMsg.nakCode()); 
    		    }
    		    if ( ackMsg.checkHasText())
    		    {
    		    	 System.out.println(" text: " + ackMsg.text()); 
    		    }
    		    break;
 
            default:
            	System.out.println("Received Unhandled Item Msg Class: " + msg.msgClass());
                break;
                
        }

        if (itemState != null && item != null) 
        {
            /* Check state of any provider-driven streams.
             * If the state indicates the item was closed, remove it from our list. */
            if (msg.streamId() < 0 && itemState.streamState() != StreamStates.OPEN)
                watchlistConsumerConfig.removeProvidedItemInfo(item);

            /* Update item state. */
            else
                itemState.copy(item.state());
        }

    
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		LoginMsgType msgType = event.rdmLoginMsg().rdmMsgType();

		switch (msgType)
		{
			case REFRESH:
				System.out.println("Received Login Refresh for Username: " + ((LoginRefresh)event.rdmLoginMsg()).userName());
				System.out.println(event.rdmLoginMsg().toString());
				
				// save loginRefresh
				((LoginRefresh)event.rdmLoginMsg()).copy(chnlInfo.loginRefresh);
					
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.LOGIN) + ", StreamId: " + event.rdmLoginMsg().streamId());
				
				System.out.println(" State: "  + chnlInfo.loginRefresh.state());
				if ( chnlInfo.loginRefresh.checkHasUserName()) 
					System.out.println(" UserName: " + chnlInfo.loginRefresh.userName().toString());
				
				// get login reissue time from authenticationTTReissue
				if (chnlInfo.loginRefresh.checkHasAuthenticationTTReissue())
				{
					chnlInfo.loginReissueTime = chnlInfo.loginRefresh.authenticationTTReissue() * 1000;
					chnlInfo.canSendLoginReissue = true;
				}
				
				break;
				
			case STATUS:
				LoginStatus loginStatus = (LoginStatus)event.rdmLoginMsg();
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.LOGIN) + ", StreamId: " + event.rdmLoginMsg().streamId());
				System.out.println("Received Login StatusMsg");
				if (loginStatus.checkHasState())
		    	{
					System.out.println("	" + loginStatus.state());
		    	}
				if (loginStatus.checkHasUserName()) 
					System.out.println(" UserName: " + loginStatus.userName().toString());

				break;
			default:
				System.out.println("Received Unhandled Login Msg Type: " + msgType);
				break;
		}
		
		System.out.println("");
		
        return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		DirectoryMsgType msgType = event.rdmDirectoryMsg().rdmMsgType();
		List<Service> serviceList = null;

		switch (msgType)
		{
			case REFRESH:
				DirectoryRefresh directoryRefresh = (DirectoryRefresh)event.rdmDirectoryMsg();
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.REFRESH));
				System.out.println(directoryRefresh.state().toString());
		    					
				serviceList = directoryRefresh.serviceList();	
				String serviceName = chnlInfo.connectionArg.service();

				for (Service service : serviceList)
				{
					if(service.info().serviceName().toString() != null)
			        {
						if (service.info().serviceName().toString().equals(serviceName))
						{
							// save serviceInfo associated with requested service name
							if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
							{
								System.out.println("Service.copy() failure");
								uninitialize();
								System.exit(ReactorReturnCodes.FAILURE);                    
			                	}
								chnlInfo.hasServiceInfo = true;
						}
		                if (service.info().serviceName().toString().equals(tsServiceName))
		                {
		                    // save serviceInfo associated with requested service name
		                    if (service.copy(chnlInfo.tsServiceInfo) < CodecReturnCodes.SUCCESS)
		                    {
		                        System.out.println("Service.copy() failure");
		                        uninitialize();
		                        System.exit(ReactorReturnCodes.FAILURE);                    
		                    }
		                    
		                    chnlInfo.hasTunnelStreamServiceInfo = true;
		                }
			        }
				}
				break;
			case UPDATE:
				DirectoryUpdate directoryUpdate = (DirectoryUpdate)event.rdmDirectoryMsg();

			    serviceName = chnlInfo.connectionArg.service();
			    String tsServiceName = chnlInfo.connectionArg.tsService();
			    System.out.println("Received Source Directory Update");
			    System.out.println(directoryUpdate.toString());
			    
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.UPDATE));

				serviceList = directoryUpdate.serviceList();
				
			    for (Service service : serviceList)
			    {
			    	if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.serviceInfo.serviceId() ) 
			    	{
			    		chnlInfo.serviceInfo.action(MapEntryActions.DELETE);
			    	}
			            
			    	if (service.action() == MapEntryActions.DELETE && service.serviceId() == chnlInfo.tsServiceInfo.serviceId() ) 
			    	{
			    		chnlInfo.tsServiceInfo.action(MapEntryActions.DELETE);
			    	}
			            
			    	boolean updateServiceInfo = false, updateQServiceInfo = false;
			    	if(service.info().serviceName().toString() != null)
			    	{
			    		System.out.println("Received serviceName: " + service.info().serviceName() + "\n");
			    		// update service cache - assume cache is built with previous refresh message
			    		if (service.info().serviceName().toString().equals(serviceName) ||
			    				service.serviceId() == chnlInfo.serviceInfo.serviceId())
			    		{
			    			updateServiceInfo = true;
			    		}
			    		if (service.info().serviceName().toString().equals(tsServiceName) ||
			    				service.serviceId() == chnlInfo.tsServiceInfo.serviceId())
			    		{
			    			updateQServiceInfo = true;
			    		}
			    	}
			    	else
			    	{
			    		if (service.serviceId() == chnlInfo.serviceInfo.serviceId())
			    		{
			    			updateServiceInfo = true;
			    		}
			    		if (service.serviceId() == chnlInfo.tsServiceInfo.serviceId())
			    		{
			    			updateQServiceInfo = true;
			    		}
			    	}
			            
			    	if (updateServiceInfo)
			    	{
			    		// update serviceInfo associated with requested service name
			    		if (service.copy(chnlInfo.serviceInfo) < CodecReturnCodes.SUCCESS)
			    		{
			    			System.out.println("Service.copy() failure");
			    			uninitialize();
			    			System.exit(ReactorReturnCodes.FAILURE);                    
			    		}
			    		chnlInfo.hasServiceInfo = true;
			    	}
			    	if (updateQServiceInfo)
			    	{
			    		// update serviceInfo associated with requested service name
			    		if (service.copy(chnlInfo.tsServiceInfo) < CodecReturnCodes.SUCCESS)
			    		{
			    			System.out.println("Service.copy() failure");
			    			uninitialize();
			    			System.exit(ReactorReturnCodes.FAILURE);                    
			    		}

			    		chnlInfo.hasTunnelStreamServiceInfo = true;                
			    	}
			    }
				
				break;
			case CLOSE:
				System.out.println("Received Source Directory Close");
				break;
			case STATUS:
				DirectoryStatus directoryStatus = (DirectoryStatus)event.rdmDirectoryMsg();
				System.out.println("Received Source Directory StatusMsg");
				System.out.println("Domain: " + DomainTypes.toString(DomainTypes.SOURCE));
				System.out.println("Stream: " + event.rdmDirectoryMsg().streamId() + " Msg Class: " + MsgClasses.toString(MsgClasses.STATUS));
				System.out.println(directoryStatus.state().toString());
				if (directoryStatus.checkHasState())
		    	{
					System.out.println("	" + directoryStatus.state());
		    	}
				break;
			default:
				System.out.println("Received Unhandled Source Directory Msg Type: " + msgType);
		    	break;
		}
		
		/* Refresh and update messages contain updates to service information. */		
		if ( serviceList != null ) 
		{
			for (Service service : serviceList)
			{
				System.out.println(" Service = " + service.serviceId() + " Action: " + MapEntryActions.toString(service.action()));
				
				if (chnlInfo.connectionArg.tunnel()) 
				{
					tunnelStreamHandler.processServiceUpdate(chnlInfo.connectionArg.tsService(), service);        	
				}
			}
		}
        
		if (chnlInfo.connectionArg.tunnel()) 
		{
			if (!tunnelStreamHandler.isServiceFound())
			{
				System.out.println(" Directory response does not contain service name for tunnel streams: \n " 
						+ chnlInfo.connectionArg.tsService());
			}
			else if (!tunnelStreamHandler.isServiceSupported())
			{
				System.out.println(" Service in use for tunnel streams does not support them: \n"						 
						+ chnlInfo.connectionArg.tsService());
			}
            else if (isRequestedTunnelStreamServiceUp(chnlInfo))
            {
                if (tunnelStreamHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
                            chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
                    {
                        uninitialize();
                        System.exit(ReactorReturnCodes.FAILURE);
                    }
                }
            }
		}
		
		System.out.println("");
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
    @Override
	public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
	{
		ChannelInfo chnlInfo = (ChannelInfo)event.reactorChannel().userSpecObj();
		DictionaryMsgType msgType = event.rdmDictionaryMsg().rdmMsgType();
		
		// initialize dictionary
		if (chnlInfo.dictionary == null)
		{
			chnlInfo.dictionary = CodecFactory.createDataDictionary();
		}
		
		switch (msgType)
		{
			case REFRESH:
				DictionaryRefresh dictionaryRefresh = (DictionaryRefresh)event.rdmDictionaryMsg();

				if (dictionaryRefresh.checkHasInfo())
				{
					/* The first part of a dictionary refresh should contain information about its type.
					 * Save this information and use it as subsequent parts arrive. */
					switch(dictionaryRefresh.dictionaryType())
					{
						case Dictionary.Types.FIELD_DEFINITIONS:
							fieldDictionaryLoaded = false;
							chnlInfo.fieldDictionaryStreamId = dictionaryRefresh.streamId();
							break;
						case Dictionary.Types.ENUM_TABLES:
							enumDictionaryLoaded = false;
							chnlInfo.enumDictionaryStreamId = dictionaryRefresh.streamId();
							break;
						default: 
							System.out.println("Unknown dictionary type " + dictionaryRefresh.dictionaryType() + " from message on stream " + dictionaryRefresh.streamId());
							chnlInfo.reactorChannel.close(errorInfo);
							return ReactorCallbackReturnCodes.SUCCESS;
					}
				}

				/* decode dictionary response */

		        // clear decode iterator
				chnlInfo.dIter.clear();

		        // set buffer and version info
				chnlInfo.dIter.setBufferAndRWFVersion(dictionaryRefresh.dataBody(),
													event.reactorChannel().majorVersion(),
													event.reactorChannel().minorVersion());

				System.out.println("Received Dictionary Response: " + dictionaryRefresh.dictionaryName());

				if (dictionaryRefresh.streamId() == chnlInfo.fieldDictionaryStreamId)
				{
					if (chnlInfo.dictionary.decodeFieldDictionary(chnlInfo.dIter, Dictionary.VerbosityValues.VERBOSE, error) == CodecReturnCodes.SUCCESS)
					{
						if (dictionaryRefresh.checkRefreshComplete())
						{
							fieldDictionaryLoaded = true;
							itemDecoder.fieldDictionaryDownloadedFromNetwork =  true;
							itemDecoder.dictionary = chnlInfo.dictionary;
							System.out.println("Field Dictionary complete.");
						}
					}
					else
		    		{
		    			System.out.println("Decoding Field Dictionary failed: " + error.text());
						chnlInfo.reactorChannel.close(errorInfo);
		    		}
				}
				else if (dictionaryRefresh.streamId() == chnlInfo.enumDictionaryStreamId)
				{
					if (chnlInfo.dictionary.decodeEnumTypeDictionary(chnlInfo.dIter, Dictionary.VerbosityValues.VERBOSE, error) == CodecReturnCodes.SUCCESS)
					{
						if (dictionaryRefresh.checkRefreshComplete())
						{
							enumDictionaryLoaded = true;
	                        itemDecoder.enumTypeDictionaryDownloadedFromNetwork =  true;
	                        itemDecoder.dictionary = chnlInfo.dictionary;
							System.out.println("EnumType Dictionary complete.");
						}
					}
					else
		    		{
		    			System.out.println("Decoding EnumType Dictionary failed: " + error.text());
						chnlInfo.reactorChannel.close(errorInfo);
		    		}
				}
				else
				{
					System.out.println("Received unexpected dictionary message on stream " + dictionaryRefresh.streamId());
				}
				
				if (fieldDictionaryLoaded  && enumDictionaryLoaded)
					itemsRequested = false;
								
				break;
			case STATUS:
				DictionaryStatus dictionaryStatus = (DictionaryStatus)event.rdmDictionaryMsg();

				if (dictionaryStatus.streamId() == chnlInfo.fieldDictionaryStreamId)
				{
					System.out.println("Received Dictionary StatusMsg for RWFFld, streamId: " + chnlInfo.fieldDictionaryStreamId);
				}
				else if (dictionaryStatus.streamId() == chnlInfo.enumDictionaryStreamId)
				{
					System.out.println("Received Dictionary StatusMsg for RWFEnum, streamId: " + chnlInfo.enumDictionaryStreamId);
				}
				if (dictionaryStatus.checkHasState())
                {
                    System.out.println(dictionaryStatus.state());
                }
				break;
			default:
				System.out.println("Received Unhandled Dictionary Msg Type: " + msgType);
				break;		
		}
		
		System.out.println("");
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	

    public boolean isRequestedServiceUp(ChannelInfo chnlInfo)
    {
        return  chnlInfo.hasServiceInfo &&
			chnlInfo.serviceInfo.checkHasState() && (!chnlInfo.serviceInfo.state().checkHasAcceptingRequests() ||
                chnlInfo.serviceInfo.state().acceptingRequests() == 1) && chnlInfo.serviceInfo.state().serviceState() == 1;
    }

    public boolean isRequestedTunnelStreamServiceUp(ChannelInfo chnlInfo)
    {
        return  chnlInfo.hasTunnelStreamServiceInfo &&
            chnlInfo.tsServiceInfo.checkHasState() && (!chnlInfo.tsServiceInfo.state().checkHasAcceptingRequests() ||
                chnlInfo.tsServiceInfo.state().acceptingRequests() == 1) && chnlInfo.tsServiceInfo.state().serviceState() == 1;
    }

    private void checkAndInitPostingSupport(ChannelInfo chnlInfo)
    {
        if (!(chnlInfo.shouldOnStreamPost || chnlInfo.shouldOffStreamPost))
            return;

        // set up posting if its enabled 
        
        // ensure that provider supports posting - if not, disable posting
        if (!chnlInfo.loginRefresh.checkHasFeatures() ||
                !chnlInfo.loginRefresh.features().checkHasSupportPost() ||
                chnlInfo.loginRefresh.features().supportOMMPost() == 0)
        {
            // provider does not support posting, disable it
        	chnlInfo.shouldOffStreamPost = false;
        	chnlInfo.shouldOnStreamPost = false;
        	chnlInfo.postHandler.enableOnstreamPost(false);
        	chnlInfo.postHandler.enableOffstreamPost(false);
            System.out.println("Connected Provider does not support OMM Posting.  Disabling Post functionality.");
            return;
        }
        
        if ( watchlistConsumerConfig.publisherId() != null && watchlistConsumerConfig.publisherAddress() != null)
        	chnlInfo.postHandler.setPublisherInfo(watchlistConsumerConfig.publisherId(), watchlistConsumerConfig.publisherAddress());
    }
	
    // on and off stream posting if enabled
    private void handlePosting()
    {   
    	for (ChannelInfo chnlInfo : chnlInfoList)
    	{
	    	if (chnlInfo.loginRefresh == null ||
	    		chnlInfo.serviceInfo == null ||
	    		chnlInfo.reactorChannel == null ||
	    		chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
	    	{	    		    		
	    		continue;
	    	}

        	chnlInfo.postItemName.clear();
	    	
	        if (chnlInfo.postHandler.enableOnstreamPost())
	        {	        	
                ItemInfo postingItem = null;

                // Find a first MarketPrice item 
                // If found, send on-stream posts on it.
                for (int i = 0; i < watchlistConsumerConfig.itemCount(); i++)
                {
                    if (watchlistConsumerConfig.itemList().get(i).domain() == DomainTypes.MARKET_PRICE)
                    {
                        postingItem = watchlistConsumerConfig.itemList().get(i);
                        if(watchlistConsumerConfig.itemList().get(i).state().streamState() != StreamStates.OPEN  ||
                        		watchlistConsumerConfig.itemList().get(i).state().dataState() != DataStates.OK)
                        {
                        	System.out.println("No currently available Market Price streams to on-stream post to.  Will retry shortly.");
                        	return;
                        }
                        break;
                    }
                    
                }	        	

                if (postingItem == null)
                {
                    System.out.println("No currently available Market Price streams to on-stream post to.  Will retry shortly.\n");	            
                    return;
                }
	            	           
                chnlInfo.postHandler.streamId(postingItem.streamId());
                chnlInfo.postHandler.postItemName().data(postingItem.name());
	            chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
	            chnlInfo.postHandler.dictionary(chnlInfo.dictionary);

	
	            int ret = chnlInfo.postHandler.handlePosts(chnlInfo.reactorChannel, errorInfo);
	            if (ret < CodecReturnCodes.SUCCESS)
	                System.out.println("Error posting onstream: " + errorInfo.error().text());
	        }
	        if (chnlInfo.postHandler.enableOffstreamPost())
	        {
	        	chnlInfo.postHandler.streamId(chnlInfo.loginRefresh.streamId());
	        	chnlInfo.postHandler.postItemName().data("OFFPOST");
	        	chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
	        	chnlInfo.postHandler.dictionary(chnlInfo.dictionary);
	            int ret = chnlInfo.postHandler.handlePosts(chnlInfo.reactorChannel, errorInfo);
	            if (ret < CodecReturnCodes.SUCCESS)
	                System.out.println("Error posting offstream: " + errorInfo.error().text());
	        }
    	}
    }

    private void handleTunnelStream()
    {
        for (ChannelInfo chnlInfo : chnlInfoList)
        {
            if (chnlInfo.loginRefresh == null ||
                chnlInfo.serviceInfo == null ||
                chnlInfo.reactorChannel == null ||
                chnlInfo.reactorChannel.state() != ReactorChannel.State.READY)
            {
                continue;
            }
            
            if (tunnelStreamHandler != null)
            {
                tunnelStreamHandler.sendMsg(chnlInfo.reactorChannel);                 
            }
        }
    }

	private void initChannelInfo(ChannelInfo chnlInfo)
	{
        // set up consumer role   
		chnlInfo.consumerRole.defaultMsgCallback(this);
		chnlInfo.consumerRole.channelEventCallback(this);
		chnlInfo.consumerRole.loginMsgCallback(this);
		chnlInfo.consumerRole.directoryMsgCallback(this);
		chnlInfo.consumerRole.watchlistOptions().enableWatchlist(true);
		chnlInfo.consumerRole.watchlistOptions().itemCountHint(4);
		chnlInfo.consumerRole.watchlistOptions().maxOutstandingPosts(5);
		chnlInfo.consumerRole.watchlistOptions().obeyOpenWindow(true);
		chnlInfo.consumerRole.watchlistOptions().channelOpenCallback(this);
		
        if (itemDecoder.fieldDictionaryLoadedFromFile == false &&
        	itemDecoder.enumTypeDictionaryLoadedFromFile == false)
        {
        	chnlInfo.consumerRole.dictionaryMsgCallback(this);
        }
        
        // initialize consumer role to default
        chnlInfo.consumerRole.initDefaultRDMLoginRequest();
        chnlInfo.consumerRole.initDefaultRDMDirectoryRequest();
        
		// use command line login user name if specified
        if (watchlistConsumerConfig.userName() != null && !watchlistConsumerConfig.userName().equals(""))
        {
            chnlInfo.consumerRole.rdmLoginRequest().userName().data(watchlistConsumerConfig.userName());
        }        
        
        // use command line authentication token and extended authentication information if specified
        if (watchlistConsumerConfig.authenticationToken() != null && !watchlistConsumerConfig.authenticationToken().equals(""))
        {
            chnlInfo.consumerRole.rdmLoginRequest().userNameType(Login.UserIdTypes.AUTHN_TOKEN);
            chnlInfo.consumerRole.rdmLoginRequest().userName().data(watchlistConsumerConfig.authenticationToken());

            if (watchlistConsumerConfig.authenticationExtended() != null && !watchlistConsumerConfig.authenticationExtended().equals(""))
            {
            	chnlInfo.consumerRole.rdmLoginRequest().applyHasAuthenticationExtended();
                chnlInfo.consumerRole.rdmLoginRequest().authenticationExtended().data(watchlistConsumerConfig.authenticationExtended());
            }
        }
        
        // use command line application id if specified
        if (watchlistConsumerConfig.applicationId() != null && !watchlistConsumerConfig.applicationId().equals(""))
        {
            chnlInfo.consumerRole.rdmLoginRequest().attrib().applicationId().data(watchlistConsumerConfig.applicationId());
        }
        
        chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
        chnlInfo.consumerRole.rdmLoginRequest().attrib().singleOpen(1);
        chnlInfo.consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
        chnlInfo.consumerRole.rdmLoginRequest().attrib().allowSuspectData(1);
                
        if (itemDecoder.fieldDictionaryLoadedFromFile == true &&
        	itemDecoder.enumTypeDictionaryLoadedFromFile == true)
        {
        	chnlInfo.dictionary = itemDecoder.getDictionary();
        }
        chnlInfo.shouldOffStreamPost = watchlistConsumerConfig.enableOffpost();
        chnlInfo.shouldOnStreamPost = watchlistConsumerConfig.enablePost();
        
        if (chnlInfo.shouldOnStreamPost)
        {
        	boolean mpItemFound = false;
        	if (chnlInfo.connectionArg.itemList() != null)
        	{
	        	for (ItemArg itemArg  : chnlInfo.connectionArg.itemList())
	        	{
	        		if (itemArg.domain() == DomainTypes.MARKET_PRICE)
	        		{
	        			mpItemFound = true;
	        			break;
	        		}
	        	}
        	}
            if (mpItemFound == false)
            {
                System.out.println("\nPosting will not be performed for this channel as no Market Price items were requested");
                chnlInfo.shouldOnStreamPost = false;
            }
        }

                 
        chnlInfo.postHandler.enableOnstreamPost(chnlInfo.shouldOnStreamPost);
        chnlInfo.postHandler.enableOffstreamPost(chnlInfo.shouldOffStreamPost);

        // This sets up our basic timing so post messages will be sent
        // periodically
        chnlInfo.postHandler.initPostHandler();
        
        
        // set up reactor connect options
        chnlInfo.connectOptions.reconnectAttemptLimit(-1); // attempt to recover forever
        chnlInfo.connectOptions.reconnectMinDelay(500); // 0.5 second minimum
        chnlInfo.connectOptions.reconnectMaxDelay(3000); // 3 second maximum
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().connectionType(chnlInfo.connectionArg.connectionType());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(chnlInfo.connectionArg.port());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(chnlInfo.connectionArg.hostname());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().unifiedNetworkInfo().interfaceName(chnlInfo.connectionArg.interfaceName());
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().userSpecObject(chnlInfo);
        chnlInfo.connectOptions.connectionList().get(0).connectOptions().guaranteedOutputBuffers(1000);
        
        // handler encrypted or http connection 
        chnlInfo.shouldEnableEncrypted = watchlistConsumerConfig.enableEncrypted();
        chnlInfo.shouldEnableHttp = watchlistConsumerConfig.enableHttp(); 
         
        if (chnlInfo.shouldEnableEncrypted)
        {
        	ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
        	cOpt.connectionType(ConnectionTypes.ENCRYPTED);
            cOpt.tunnelingInfo().tunnelingType("encrypted"); 
            setEncryptedConfiguration(cOpt);        	           	        	
        }        
        else if (chnlInfo.shouldEnableHttp)
        {
        	ConnectOptions cOpt = chnlInfo.connectOptions.connectionList().get(0).connectOptions();
            cOpt.connectionType(ConnectionTypes.HTTP);
            cOpt.tunnelingInfo().tunnelingType("http"); 
            setHTTPConfiguration(cOpt);
        } 
        
        // handle basic tunnel stream configuration
        if (chnlInfo.connectionArg.tunnel() && tunnelStreamHandler == null)
        {
            tsServiceName = chnlInfo.connectionArg.tsService();
            tunnelStreamHandler = new TunnelStreamHandler(chnlInfo.connectionArg.tunnelAuth(), chnlInfo.connectionArg.tunnelDomain());
        }
	}
	
	// load FIX dictionary to support FIX Protocol
	public void loadFixDictionary()
	{      
        fixdictionary.clear();
        if (fixdictionary.loadFieldDictionary(FIX_FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("\nUnable to load FIX field dictionary. \n\tText: "
                        + error.text() + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }

        if (fixdictionary.loadEnumTypeDictionary(FIX_ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("\nUnable to load FIX enum dictionary. \n\tText: "
                        + error.text() + "\n");
            uninitialize();
            System.exit(ReactorReturnCodes.FAILURE);
        }	    
	}    

    private void closeItemStreams(ChannelInfo chnlInfo)
    {
        // have offstream posting post close status
    	if (chnlInfo.shouldOffStreamPost)
    	{
    		chnlInfo.postHandler.streamId(chnlInfo.loginRefresh.streamId());
    		chnlInfo.postHandler.postItemName().data("OFFPOST");
    		chnlInfo.postHandler.serviceId(chnlInfo.serviceInfo.serviceId());
    		chnlInfo.postHandler.dictionary(chnlInfo.dictionary);
    		chnlInfo.postHandler.closeOffStreamPost(chnlInfo.reactorChannel, errorInfo);
    	}
    	
		for(int itemListIndex = 0; itemListIndex < watchlistConsumerConfig.itemCount(); ++itemListIndex)
		{			        
			int domainType = watchlistConsumerConfig.itemList().get(itemListIndex).domain();
		        
			int streamId = watchlistConsumerConfig.itemList().get(itemListIndex).streamId;
	           
			/* encode item close */
			closeMsg.clear();
			closeMsg.msgClass(MsgClasses.CLOSE);
			closeMsg.streamId(streamId);
			closeMsg.domainType(domainType);
			closeMsg.containerType(DataTypes.NO_DATA); 

		    if ( (chnlInfo.reactorChannel.submit(closeMsg, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS)
		    {
		    	System.out.println("Close itemStream of " + streamId + " Failed.");
		    }
		}
	}

    /* Uninitializes the Value Add consumer application. */
	private void uninitialize()
	{
		System.out.println("ConsumerCallbackThread  " + this.hashCode() + " unitializing and exiting...");
        
    	for (ChannelInfo chnlInfo : chnlInfoList)
    	{
	        // close items streams
	        closeItemStreams(chnlInfo);
	
            // close tunnel streams
            if (tunnelStreamHandler != null &&
                chnlInfo.reactorChannel != null)
            {
                if (tunnelStreamHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("tunnelStreamHandler.closeStream() failed with errorText: " + errorInfo.error().text());
                }
            }
    
	        // close ReactorChannel
	        if (chnlInfo.reactorChannel != null)
	        {
	        	chnlInfo.reactorChannel.close(errorInfo);
	        }
    	}
        
        // shutdown reactor
    	if (reactor != null)
    	{
    	    reactor.shutdown(errorInfo);
    	}
	}
		
	private void handleClose()
	{
		 System.out.println("ConsumerCallbackThread  " + this.hashCode() + " closing streams...");
	        		 		                
		for (ChannelInfo chnlInfo : chnlInfoList)
		{
			closeItemStreams(chnlInfo);
		
			// close tunnel streams
			if (tunnelStreamHandler != null && chnlInfo.reactorChannel != null)
			{
				if (tunnelStreamHandler.closeStreams(chnlInfo, _finalStatusEvent, errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					System.out.println("tunnelStreamHandler.closeStream() failed with errorText: " + errorInfo.error().text());
	            }
	        }	
		}
	}	
		   
    private void setEncryptedConfiguration(ConnectOptions options)
    {
    	setHTTPConfiguration(options);
    	
    	String keyFile = watchlistConsumerConfig.keyStoreFile();
    	String keyPasswd = watchlistConsumerConfig.keystorePassword();
        if (keyFile == null)
        {
        	System.err.println("Error: Keystore file not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }                   
        if (keyPasswd == null)
        {
        	System.err.println("Error: Keystore password not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }          
    	    	
    	options.tunnelingInfo().KeystoreFile(keyFile);
        options.tunnelingInfo().KeystorePasswd(keyPasswd);   
    }

    
    private void setHTTPConfiguration(ConnectOptions options)
    {    	
        options.tunnelingInfo().objectName("");
        options.tunnelingInfo().KeystoreType("JKS");
        options.tunnelingInfo().SecurityProtocol("TLS");
        options.tunnelingInfo().SecurityProvider("SunJSSE");
        options.tunnelingInfo().KeyManagerAlgorithm("SunX509");
        options.tunnelingInfo().TrustManagerAlgorithm("PKIX");
    	
        if (watchlistConsumerConfig.enableProxy())
        {
         	String proxyHostName = watchlistConsumerConfig.proxyHostname();
            if ( proxyHostName == null)
            {
            	System.err.println("Error: Proxy hostname not provided.");  
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }           
            String proxyPort = watchlistConsumerConfig.proxyPort();
            if ( proxyPort == null)
            {
            	System.err.println("Error: Proxy port number not provided.");  
            	System.exit(CodecReturnCodes.FAILURE);        		        		        		
            }                             	

  
            options.tunnelingInfo().HTTPproxy(true);
            options.tunnelingInfo().HTTPproxyHostName(proxyHostName);
            try
            {
            	options.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort));
            }
            catch(Exception e)
            {
               	System.err.println("Error: Proxy port number not provided.");  
            	System.exit(CodecReturnCodes.FAILURE);            	
            }
        }
   
        // credentials
        if (options.tunnelingInfo().HTTPproxy())
        {
            setCredentials(options);           
        }
    }    
    
    /*
     * For BASIC authentication we need: HTTPproxyUsername, HTTPproxyPasswd For
     * NTLM authentication we need: HTTPproxyUsername, HTTPproxyPasswd,
     * HTTPproxyDomain, HTTPproxyLocalHostname For Negotiate/Kerberos we need:
     * HTTPproxyUsername, HTTPproxyPasswd, HTTPproxyDomain,
     * HTTPproxyKRB5configFile
     */
    private void setCredentials(ConnectOptions options)
    {
        String localIPaddress = null;
        String localHostName = null;

    	String proxyUsername = watchlistConsumerConfig.proxyUsername();
        if ( proxyUsername == null)
        {
        	System.err.println("Error: Proxy username not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }    
              
        String proxyPasswd = watchlistConsumerConfig.proxyPassword();
        if ( proxyPasswd == null)
        {
        	System.err.println("Error: Proxy password not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }     
        String proxyDomain = watchlistConsumerConfig.proxyDomain();
        if ( proxyDomain == null)
        {
        	System.err.println("Error: Proxy domain not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }             
    	    	
        options.credentialsInfo().HTTPproxyUsername(proxyUsername);
        options.credentialsInfo().HTTPproxyPasswd(proxyPasswd);
        options.credentialsInfo().HTTPproxyDomain(proxyDomain);
        
        try
        {
            localIPaddress = InetAddress.getLocalHost().getHostAddress();
            localHostName = InetAddress.getLocalHost().getHostName();
        }
        catch (UnknownHostException e)
        {
            localHostName = localIPaddress;
        }
        options.credentialsInfo().HTTPproxyLocalHostname(localHostName);
        
        String proxyKrbfile = watchlistConsumerConfig.krbFile();
        if (proxyKrbfile == null)
        {
        	System.err.println("Error: Proxy krbfile not provided.");  
        	System.exit(CodecReturnCodes.FAILURE);        		        		        		
        }                              
        options.credentialsInfo().HTTPproxyKRB5configFile(proxyKrbfile);          
             
    }	
    
    String mapServiceActionString(int action)
    {
    	if (action == MapEntryActions.DELETE) return "DELETE";
    	else if ( action == MapEntryActions.ADD) return "ADD";
    	else if (action == MapEntryActions.UPDATE) return "UPDATE";
    	else 
    		return null;
    }
    			

	/** Signals thread to shutdown. */
	public void shutDown(boolean shutDown)
	{
		this.shutDown = shutDown;
		mtWatchlistConsumer.shutDown = true;
	}
	
	boolean isDictionaryReady()
	{
		return 	(fieldDictionaryLoaded && enumDictionaryLoaded) || (itemDecoder.fieldDictionaryLoadedFromFile && itemDecoder.enumTypeDictionaryLoadedFromFile);
	}
}
