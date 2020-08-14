package com.thomsonreuters.upa.perftools.upajconsperf;

import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.ClassesOfService;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStream;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamDefaultMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamOpenOptions;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamStatusEvent;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamStatusEventCallback;

class TunnelStreamHandler implements TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback
{
	final static int TUNNEL_STREAM_STREAM_ID = 1001;
	private boolean _tunnelAuth;
	private int _tunnelDomain;
	private Service _service;
	private ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
	private TunnelStreamOpenOptions _tunnelStreamOpenOptions = ReactorFactory.createTunnelStreamOpenOptions();
	private ReactorChannel _reactorChannel;
	private boolean _tunnelStreamOpenSent;
	private TunnelStream _tunnelStream;
	private ConsumerThread _consumerThread;
	
	TunnelStreamHandler(ConsumerThread consumerThread, boolean tunnelAuth, int tunnelDomain)
	{
		_tunnelAuth = tunnelAuth;
		_tunnelDomain = tunnelDomain;
		_consumerThread = consumerThread;
	}
	
	public TunnelStream tunnelStream()
	{
		return _tunnelStream;
	}
	
	public boolean tunnelStreamOpenSent()
	{
		return _tunnelStreamOpenSent;
	}
	
	/*
     * Used by the Consumer to open a tunnel stream once the Client's channel
     * is opened and the desired service identified.
     */
    public int openStream(ReactorChannel reactorChannel, Service service, ReactorErrorInfo errorInfo)
    {
    	_service = service;
    	_reactorChannel = reactorChannel;
        
        _tunnelStreamOpenOptions.clear();
        _tunnelStreamOpenOptions.name("ConsPerf");
        _tunnelStreamOpenOptions.classOfService().flowControl().type(ClassesOfService.FlowControlTypes.BIDIRECTIONAL);
        _tunnelStreamOpenOptions.classOfService().dataIntegrity().type(ClassesOfService.DataIntegrityTypes.RELIABLE);
        _tunnelStreamOpenOptions.streamId(TUNNEL_STREAM_STREAM_ID);
        _tunnelStreamOpenOptions.domainType(_tunnelDomain);
        _tunnelStreamOpenOptions.serviceId(_service.serviceId());
        _tunnelStreamOpenOptions.defaultMsgCallback(this);
        _tunnelStreamOpenOptions.statusEventCallback(this);
        _tunnelStreamOpenOptions.guaranteedOutputBuffers(_consumerThread.consumerPerfConfig().tunnelStreamGuaranteedOutputBuffers());

		if (_tunnelAuth)
			_tunnelStreamOpenOptions.classOfService().authentication().type(ClassesOfService.AuthenticationTypes.OMM_LOGIN);
        
        if (_reactorChannel.openTunnelStream(_tunnelStreamOpenOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
        {       
            return ReactorReturnCodes.FAILURE;
        }
        
        _tunnelStreamOpenSent = true;
    	
    	return ReactorReturnCodes.SUCCESS;
    }
    
    /*
     * Used by the Consumer to close any tunnel streams it opened
     * for the reactor channel.
     */
    public int closeStreams(ReactorErrorInfo errorInfo)
    {
        int ret;

        if (_tunnelStream == null)
            return ReactorReturnCodes.SUCCESS;

        if ((ret =  _tunnelStream.close(true, errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            System.out.println("TunnelStream.close() failed with return code: " + ret);
            return ReactorCallbackReturnCodes.SUCCESS;
        }

		_tunnelStream = null;
        
        return ReactorReturnCodes.SUCCESS;
    }
	
	
	@Override
	public int defaultMsgCallback(TunnelStreamMsgEvent event) 
	{
		if(event.containerType() == DataTypes.MSG)
		{
			_consumerThread.handleTunnelStreamMsgCallback(event.msg(), event.reactorChannel());
		}
		else 
		{
			System.out.println("TunnelStreamHandler received unsupported container type: " + event.containerType());
		}
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	@Override
	public int statusEventCallback(TunnelStreamStatusEvent event) 
	{
		State state = event.state();
        int ret;
        
        System.out.println("Received TunnelStreamStatusEvent for Stream ID " + event.tunnelStream().streamId() + " with " + state + "\n");
        
        switch(state.streamState())
        {
            case StreamStates.OPEN:
                if (state.dataState() == DataStates.OK && _tunnelStream == null)
                {
                    // Stream is open and ready for use.

                	_tunnelStream = event.tunnelStream();
                }
                break;

            case StreamStates.CLOSED_RECOVER:
            case StreamStates.CLOSED:
            default:
                // For other stream states such as Closed & ClosedRecover, close the tunnel stream. 
                if ((ret = event.tunnelStream().close(true, _errorInfo)) < ReactorReturnCodes.SUCCESS)
                {
                    System.out.println("Failed to close TunnelStream:"
                            + ReactorReturnCodes.toString(ret) + "(" + _errorInfo.error().text() + ")");
                }
                
                // Remove our tunnel information if the tunnel was open.
                _tunnelStreamOpenSent = false;
                _tunnelStream = null;
                
                break;
        }

		
		return ReactorCallbackReturnCodes.SUCCESS;
	}

}
