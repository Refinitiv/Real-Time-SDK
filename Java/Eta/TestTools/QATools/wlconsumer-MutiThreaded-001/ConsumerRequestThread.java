/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.valueadd.examples.watchlistconsumer.WatchlistConsumerConfig.ItemInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;

/**
 * <p>
 * This interface is responsible for sending item requests to provider.
 * </p>
 */
public class ConsumerRequestThread implements Runnable
{
    private ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private EncodeIterator encIter = CodecFactory.createEncodeIterator();
    private final List<Integer> viewFieldList;
    private ItemRequest itemRequest;
    private ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	boolean _finalStatusEvent;
	ChannelInfo chnlInfo;
	ConsumerCallbackThread consumerCallbackThread;
	Buffer payload;
	ArrayList<ItemInfo> itemList = new ArrayList<ItemInfo>();
	 
    public ConsumerRequestThread(ChannelInfo channelInfo, ConsumerCallbackThread consumer)
    {
    	 itemRequest = createItemRequest();
    	 viewFieldList = new ArrayList<Integer>();
        chnlInfo = channelInfo;
        consumerCallbackThread = consumer;
    }

    protected ItemRequest createItemRequest()
    {
        return new ItemRequest();
    }
    
    /* Runs the Value Add consumer application. */
    public void run()		
	{		
		while (!consumerCallbackThread.shutDown)
		{
	        //send  item requests
			if (consumerCallbackThread.isRequestedServiceUp(chnlInfo) && consumerCallbackThread.isDictionaryReady()
					&& !consumerCallbackThread.itemsRequested)
			{
    			for (ItemInfo itemInfo : itemList)
    			{
    				try 
    				{
    					Thread.sleep(10);
    					 requestItems(itemInfo);
    				}
    				catch (InterruptedException e)
    				{
    					e.printStackTrace();
    				}
    			}
    			consumerCallbackThread.itemsRequested = true;
    		}
    			
			if (consumerCallbackThread.isRequestedTunnelStreamServiceUp(chnlInfo)  && !consumerCallbackThread.itemsRequested)
			{	
					if (chnlInfo.connectionArg.tunnel()) 
					{
						if (!consumerCallbackThread.tunnelStreamHandler.isServiceFound())
						{
							System.out.println(" Directory response does not contain service name for tunnel streams: \n " 
									+ chnlInfo.connectionArg.tsService());
						}
						else if (!consumerCallbackThread.tunnelStreamHandler.isServiceSupported())
						{
							System.out.println(" Service in use for tunnel streams does not support them: \n"						 
									+ chnlInfo.connectionArg.tsService());
						}
			            else if (consumerCallbackThread.isRequestedTunnelStreamServiceUp(chnlInfo))
			            {
			                if (consumerCallbackThread.tunnelStreamHandler.openStream(chnlInfo, errorInfo) != ReactorReturnCodes.SUCCESS)
			                {
			                    if (chnlInfo.reactorChannel.state() != ReactorChannel.State.CLOSED &&
			                            chnlInfo.reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING)
			                    {
			                    	consumerCallbackThread.shutDown(true);
			                        System.exit(ReactorReturnCodes.FAILURE);
			                    }
			                }
			            }
					}			
			}
			
	        try
			{
				Thread.sleep(10);
			} catch (InterruptedException e)
			{
				e.printStackTrace();
			}
		}
		
		 // Handle run-time
        if (consumerCallbackThread.shutDown)
        {
        	System.out.println("ConsumerRequestThread  " + this.hashCode() + " closing now...");
        }
	}
    
	int requestItems(ItemInfo itemInfo)
	{
	    int ret = CodecReturnCodes.SUCCESS;
	    
		itemRequest.clear();	
		
		if (!consumerCallbackThread.watchlistConsumerConfig.enableSnapshot())
			itemRequest.applyStreaming();

		itemRequest.addItem(itemInfo.name);
		
		int domainType = itemInfo.domain();
		
		itemRequest.domainType(domainType);
		itemRequest.streamId(itemInfo.streamId);

		if (itemInfo.isPrivateStream())
			itemRequest.applyPrivateStream();
					
		if (domainType == DomainTypes.SYMBOL_LIST && itemInfo.symbolListData)
		{
			itemRequest.symbolListData(true);
		    payload = CodecFactory.createBuffer(); 
		    payload.data(ByteBuffer.allocate(1024));

			if (payload == null)
			{
				return CodecReturnCodes.FAILURE;
			}

			encIter.clear();
			encIter.setBufferAndRWFVersion(payload, chnlInfo.reactorChannel.majorVersion(), chnlInfo.reactorChannel.minorVersion());
			
			
			ret = itemRequest.encode(encIter);
	         
			if (ret < CodecReturnCodes.SUCCESS)
			{
				errorInfo.error().text("RequestItem.encode() failed");
				errorInfo.error().errorId(ret);
				return ret;
			}

			itemRequest.requestMsg.encodedDataBody(payload);
		}
		else if (domainType == DomainTypes.MARKET_PRICE && consumerCallbackThread.watchlistConsumerConfig.enableView())
		{			
		    payload = CodecFactory.createBuffer(); // move it to the top to share 
		    payload.data(ByteBuffer.allocate(1024));

			if (payload == null)
			{
				return CodecReturnCodes.FAILURE;
			}
			
			encIter.clear();
			encIter.setBufferAndRWFVersion(payload, chnlInfo.reactorChannel.majorVersion(), chnlInfo.reactorChannel.minorVersion());

			itemRequest.applyHasView();				
			viewFieldList.add(22);
			viewFieldList.add(25);
			viewFieldList.add(30);
			viewFieldList.add(31);
			viewFieldList.add(1025);
			itemRequest.viewFields(viewFieldList);				
			ret = itemRequest.encode(encIter);
											         
			if (ret < CodecReturnCodes.SUCCESS)
			{
				errorInfo.error().text("RequestItem.encode() failed");
				errorInfo.error().errorId(ret);
				return ret;
			}							
			itemRequest.requestMsg.encodedDataBody(payload);				
		}
		else
		{
		    itemRequest.encode();
		}
				
		submitOptions.clear();
		if (consumerCallbackThread.watchlistConsumerConfig.serviceName() != null)
		{
		    submitOptions.serviceName(consumerCallbackThread.watchlistConsumerConfig.serviceName());
		}

		ret = chnlInfo.reactorChannel.submit(itemRequest.requestMsg, submitOptions, errorInfo);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("\nReactorChannel.submit() failed: " + ret + "(" + errorInfo.error().text() + ")\n");
			System.exit(ReactorReturnCodes.FAILURE);
		}
		
		return CodecReturnCodes.SUCCESS;
	}	
}
