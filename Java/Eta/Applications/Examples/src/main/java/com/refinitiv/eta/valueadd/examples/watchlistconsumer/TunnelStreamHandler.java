/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.ClassesOfService;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamDefaultMsgCallback;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamMsgEvent;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamOpenOptions;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamStatusEvent;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamStatusEventCallback;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamSubmitOptions;

/**
 * This is the tunnel stream handler for the ETA Value Add consumer application. It sends and receives
 * basic text messages over a tunnel stream. It only supports one ReactorChannel/TunnelStream.
 */
public class TunnelStreamHandler implements TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback
{
    final static int TUNNEL_STREAM_STREAM_ID = 1001;
    final static int TUNNEL_SEND_FREQUENCY = 1;

    private int _serviceId;
    private TunnelStreamOpenOptions _tunnelStreamOpenOptions = ReactorFactory.createTunnelStreamOpenOptions();
    ChannelInfo _chnlInfo;
    private ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    private long _nextSubmitMsgTime;
    private TunnelStreamSubmitOptions _tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
    private int _msgCount;
    private boolean _finalStatusEvent;
	private boolean _tunnelAuth;
	private int _tunnelDomain;

	private boolean _isTunnelServiceFound; 
	private boolean _tunnelServiceSupported;
	
	public TunnelStreamHandler(boolean tunnelAuth, int tunnelDomain)
	{
		_tunnelAuth = tunnelAuth;
		_tunnelDomain = tunnelDomain;
		_isTunnelServiceFound = false;
		_tunnelServiceSupported = false;
	}

    /*
     * Used by the Consumer to open a tunnel stream once the Consumer's channel
     * is opened and the desired service identified.
     */
    public int openStream(ChannelInfo chnlInfo, ReactorErrorInfo errorInfo)
    {
        int ret;
        
        if (!_tunnelServiceSupported)
        {
            System.out.println("ReactorChannel.openTunnelStream() failed: service " + chnlInfo.tsServiceInfo.serviceId() 
            		+ " not supported");
            return -1;
        }

        _serviceId = chnlInfo.tsServiceInfo.serviceId();
        
        _tunnelStreamOpenOptions.clear();
        _tunnelStreamOpenOptions.name("BasicTunnelStream");
        _tunnelStreamOpenOptions.classOfService().flowControl().type(ClassesOfService.FlowControlTypes.BIDIRECTIONAL);
        _tunnelStreamOpenOptions.classOfService().dataIntegrity().type(ClassesOfService.DataIntegrityTypes.RELIABLE);
        _tunnelStreamOpenOptions.streamId(TUNNEL_STREAM_STREAM_ID);
        _tunnelStreamOpenOptions.domainType(_tunnelDomain);
        _tunnelStreamOpenOptions.serviceId(_serviceId);
        _tunnelStreamOpenOptions.defaultMsgCallback(this);
        _tunnelStreamOpenOptions.statusEventCallback(this);

		if (_tunnelAuth)
			_tunnelStreamOpenOptions.classOfService().authentication().type(ClassesOfService.AuthenticationTypes.OMM_LOGIN);
		
        
        if ((ret = chnlInfo.reactorChannel.openTunnelStream(_tunnelStreamOpenOptions, errorInfo)) != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("ReactorChannel.openTunnelStream() failed: " + CodecReturnCodes.toString(ret)
                    + "(" + errorInfo.error().text() + ")");
        }
        
        chnlInfo.tunnelStreamOpenSent = true;
        _chnlInfo = chnlInfo;
    
        return ReactorReturnCodes.SUCCESS;
    }

    /*
     * Used by the Consumer to close any tunnel streams it opened
     * for the reactor channel.
     */
    public int closeStreams(ChannelInfo chnlInfo,  boolean finalStatusEvent,  ReactorErrorInfo errorInfo)
    {
        int ret;
        _finalStatusEvent = finalStatusEvent;

        if (chnlInfo.tunnelStream == null)
            return ReactorReturnCodes.SUCCESS;

        if ((ret =  chnlInfo.tunnelStream.close(finalStatusEvent, errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("TunnelStream.close() failed with return code: " + ret);
            return ReactorCallbackReturnCodes.SUCCESS;
        }

		if (!finalStatusEvent)
		{
			chnlInfo.tunnelStreamOpenSent = false;
			chnlInfo.tunnelStream = null;
		}
        
        return ReactorReturnCodes.SUCCESS;
    }


    /*
     * Encode and send a basic text messages over a tunnel stream.
     */
    public void sendMsg(ReactorChannel reactorChannel)
    {
        long currentTime = System.currentTimeMillis();
        int ret; 

        if (currentTime < _nextSubmitMsgTime)
        {
            return;
        }
        
        _nextSubmitMsgTime = currentTime + TUNNEL_SEND_FREQUENCY * 1000;
        
        if (_chnlInfo != null && _chnlInfo.tunnelStream != null && _chnlInfo.isTunnelStreamUp)
        {
            // get buffer to encode message into
            TransportBuffer buffer = _chnlInfo.tunnelStream.getBuffer(1024, _errorInfo);
            if (buffer == null)
            {
                System.out.println("defaultMsgCallback failed: Unable to get a buffer from TunnelStream <" + _errorInfo.error().text() + ">");
                return;
            }
            
            // put basic text message in buffer
            buffer.data().put(new String("TunnelStream message " + ++_msgCount).getBytes());

            // submit the encoded data buffer to the tunnel stream
            _tunnelStreamSubmitOptions.clear();
            _tunnelStreamSubmitOptions.containerType(DataTypes.OPAQUE);
            if ((ret = _chnlInfo.tunnelStream.submit(buffer, _tunnelStreamSubmitOptions, _errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                System.out.println("TunnelStream.submit() failed: " + CodecReturnCodes.toString(ret)
                        + "(" + _errorInfo.error().text() + ")");
                _chnlInfo.tunnelStream.releaseBuffer(buffer, _errorInfo);
                return;
            }
        }
    }

    @Override
    public int defaultMsgCallback(TunnelStreamMsgEvent event)
    {
        if (event.containerType() == DataTypes.OPAQUE)
        {
            byte[] msgBytes = new byte[event.transportBuffer().length()];
            event.transportBuffer().data().get(msgBytes);
            String msgString = new String(msgBytes);
            System.out.println("Consumer TunnelStreamHandler received OPAQUE data: " + msgString + "\n");
        }
        else // not opaque data
        {
            System.out.println("TunnelStreamHandler received unsupported container type");
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }

    /* Process a tunnel stream status event for the TunnelStreamHandler. */
    @Override
    public int statusEventCallback(TunnelStreamStatusEvent event)
    {
        State state = event.state();
        int ret;
        
        System.out.println("Received TunnelStreamStatusEvent for Stream ID " + event.tunnelStream().streamId() + " with " + state + "\n");
        
        switch(state.streamState())
        {
            case StreamStates.OPEN:
                if (state.dataState() == DataStates.OK && _chnlInfo.tunnelStream == null)
                {
                    // Stream is open and ready for use.

                    // Add it to our ChannelInfo
                    _chnlInfo.tunnelStream = event.tunnelStream();
                    
                    _chnlInfo.isTunnelStreamUp = true;
                }
                break;

            case StreamStates.CLOSED_RECOVER:
            case StreamStates.CLOSED:
            default:
                // For other stream states such as Closed & ClosedRecover, close the tunnel stream. 
                if ((ret = event.tunnelStream().close(_finalStatusEvent, _errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("Failed to close TunnelStream:"
                            + ReactorReturnCodes.toString(ret) + "(" + _errorInfo.error().text() + ")");
                }
                // Remove our tunnel information if the tunnel was open.
                _chnlInfo.tunnelStreamOpenSent = false;
                _chnlInfo.tunnelStream = null;
                _chnlInfo.isTunnelStreamUp = false;
                
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }
    
    public void processServiceUpdate(String matchServiceName, Service service)
    {
    	/* Save service information for queue messaging. */
    	if (!_isTunnelServiceFound)
    	{
    		/* Check if the name matches the service we're looking for. */
    		if (service.checkHasInfo() && service.info().serviceName().toString().equals(matchServiceName))
    		{
    		    _isTunnelServiceFound = true;
    			_serviceId = service.serviceId();
    		}
    	}

    	if (service.serviceId() == _serviceId)
    	{
    		/* Process the state of the queue messaging service. */
    		if (service.action() != MapEntryActions.DELETE)
    		{
    			_serviceId = service.serviceId();

    			/* If info is present, check if queue message support is indicated. */
    			if (service.checkHasInfo())
    			{
    				_tunnelServiceSupported = false;
    				for(int i = 0; i < service.info().capabilitiesList().size(); ++i)
    				{
    					if (service.info().capabilitiesList().get(i) == _tunnelDomain)
    					{
    						_tunnelServiceSupported = true;
    						break;
    					}
    				}
    			}
    		}
    		else
    		{
    			_isTunnelServiceFound = false;
    			_tunnelServiceSupported = false;
    		}
    	}    	
    	
    
 
    }   
    
    public boolean isServiceFound()
    {
    	return _isTunnelServiceFound;
    }
    
    public boolean isServiceSupported()
    {
    	return _tunnelServiceSupported;
    }
    
}
