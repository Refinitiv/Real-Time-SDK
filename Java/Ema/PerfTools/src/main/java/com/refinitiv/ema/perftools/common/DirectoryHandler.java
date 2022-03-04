/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.access.AckMsg;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.FilterList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmConsumerClient;
import com.refinitiv.ema.access.OmmConsumerEvent;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.EmaRdm;

/**
 * This is the source directory handler for the EMA consumer application. It
 * provides methods for sending the source directory request to a provider and
 * processing the response. Methods for setting the service name, getting the
 * service information, and closing a source directory stream are also provided.
 */
public class DirectoryHandler implements OmmConsumerClient
{
    private static final long FILTER_TO_REQUEST = EmaRdm.SERVICE_INFO_FILTER  |
    		EmaRdm.SERVICE_STATE_FILTER | EmaRdm.SERVICE_GROUP_FILTER;
    
    private String _serviceName ;    // requested service
    private int _serviceId;	 //requested service id
    class ServiceState
    {
    	int streamState;
    	int dataState;
    	int statusCode;
    	String text;
    }
    private ServiceState _state;            // requested service state
    private ReqMsg _directoryRequest = EmaFactory.createReqMsg();
    private boolean _acceptRequest;
    
    /**
     * Instantiates a new directory handler.
     */
    public DirectoryHandler()
    {
        _state = new ServiceState();
    }

    /**
     * Sets the service name requested by the application.
     * 
     * @param servicename - The service name requested by the application
     */
    public void serviceName(String servicename)
    {
        _serviceName = servicename;
    }

    /**
     * Checks if is requested service up.
     *
     * @return true if service requested by application is up, false if not.
     */
    public boolean isRequestedServiceUp()
    {
        return  _acceptRequest && _state.streamState == OmmState.StreamState.OPEN && _state.dataState == OmmState.DataState.OK;
    }

    /**
     * Service id.
     *
     * @return service id associated with the service name requested by application.
     */
    public int serviceId()
    {
        return _serviceId;
    }

    /**
     * Sends a source directory request to a channel. This consists of getting a
     * message buffer, encoding the source directory request, and sending the
     * source directory request to the server.
     *
     * @return the request
     */
    public ReqMsg getRequest()
    {
        //initialize directory _state
        //this will be updated as refresh and status messages are received
        _state.dataState = OmmState.DataState.NO_CHANGE;
        _state.streamState = OmmState.StreamState.CLOSED;
        
        _directoryRequest.clear();
        _directoryRequest.domainType(EmaRdm.MMT_DIRECTORY).serviceName(_serviceName).filter(FILTER_TO_REQUEST).interestAfterRefresh(true);

        return _directoryRequest;
    }
    
	private void decode(Map map)
	{
		FilterList filterList;
		boolean findMapEntry = false;
		for(MapEntry mapEntry : map)
		{
				if (mapEntry.loadType() == DataTypes.FILTER_LIST)
				{
					filterList = mapEntry.filterList();
					for(FilterEntry filterEntry : filterList)
					{
							if (filterEntry.loadType() == DataTypes.ELEMENT_LIST )
							{
								if (decode(filterEntry.elementList()))
								{
									_serviceId = (int) mapEntry.key().uintValue();
									findMapEntry = true;
								}
							}
					}
				}
				
				if (findMapEntry)
					return;
		}
	}

	private boolean decode(ElementList elementList)
	{
		boolean foundService = false;
		for (ElementEntry elementEntry : elementList)
		{
			if (Data.DataCode.BLANK != elementEntry.code())
			{
				switch (elementEntry.loadType())
				{
					case DataTypes.ASCII :
						if (elementEntry.name().equals("Name") && elementEntry.ascii().ascii().equals(_serviceName))
							foundService = true;
						break;
					case DataTypes.UINT :
						if (elementEntry.name().equals("AcceptingRequests"))
						{
							if (elementEntry.uintValue() == 1)
								_acceptRequest = true;
							else
								_acceptRequest = false;
						}
						break;
					default :
						break;
				}
			}
		}
		return foundService;
	}

	/* (non-Javadoc)
	 * @see com.refinitiv.ema.access.OmmConsumerClient#onRefreshMsg(com.refinitiv.ema.access.RefreshMsg, com.refinitiv.ema.access.OmmConsumerEvent)
	 */
	@Override
	public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent)
	{
		 System.out.println("Received Source Directory Refresh\n");
		 
		 _state.dataState = refreshMsg.state().dataState();
	     _state.streamState = refreshMsg.state().streamState();
	         
	     if (refreshMsg.payload().dataType() != DataTypes.MAP)
	        	System.out.println("Received invalid source directory refresh msg.");
	     else
	        	decode(refreshMsg.payload().map());
         
         if (!isRequestedServiceUp())
             System.out.println("Requested service '" + _serviceName + "' not up. Waiting for service to be up...");
	}

	/* (non-Javadoc)
	 * @see com.refinitiv.ema.access.OmmConsumerClient#onUpdateMsg(com.refinitiv.ema.access.UpdateMsg, com.refinitiv.ema.access.OmmConsumerEvent)
	 */
	@Override
	public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent)
	{
		System.out.println("Received Source Directory Update\n");

        if (updateMsg.payload().dataType() != DataTypes.MAP)
        	System.out.println("Received invalid source directory update msg.");
        else
        	decode(updateMsg.payload().map());
	}

	/* (non-Javadoc)
	 * @see com.refinitiv.ema.access.OmmConsumerClient#onStatusMsg(com.refinitiv.ema.access.StatusMsg, com.refinitiv.ema.access.OmmConsumerEvent)
	 */
	@Override
	public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent)
	{
		 System.out.println("Received Source Directory Status\n");
         
		 System.out.println(statusMsg.toString());
		 
         if (statusMsg.hasState())
         {
             _state.dataState = statusMsg.state().dataState();
             _state.streamState = statusMsg.state().streamState();
         }
	}

	/* (non-Javadoc)
	 * @see com.refinitiv.ema.access.OmmConsumerClient#onGenericMsg(com.refinitiv.ema.access.GenericMsg, com.refinitiv.ema.access.OmmConsumerEvent)
	 */
	@Override
	public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {}
	
	/* (non-Javadoc)
	 * @see com.refinitiv.ema.access.OmmConsumerClient#onAckMsg(com.refinitiv.ema.access.AckMsg, com.refinitiv.ema.access.OmmConsumerEvent)
	 */
	@Override
	public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {}
	
	/* (non-Javadoc)
	 * @see com.refinitiv.ema.access.OmmConsumerClient#onAllMsg(com.refinitiv.ema.access.Msg, com.refinitiv.ema.access.OmmConsumerEvent)
	 */
	@Override
	public void onAllMsg(com.refinitiv.ema.access.Msg msg, OmmConsumerEvent consumerEvent) {}
}
