package com.thomsonreuters.upa.valueadd.examples.consumer;

import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.ClassesOfService;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.reactor.ReactorCallbackReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamDefaultMsgCallback;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamMsgEvent;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamOpenOptions;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamStatusEvent;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamStatusEventCallback;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamSubmitOptions;

/**
 * This is the tunnel stream handler for the UPA Value Add consumer application. It sends and receives
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
	// APIQA: buffer size and fill size
	private int _tunnelBufSize;
	private int _tunnelFillSize;
    // END APIQA: 

	public TunnelStreamHandler(boolean tunnelAuth, int tunnelDomain)
	{
		_tunnelAuth = tunnelAuth;
		_tunnelDomain = tunnelDomain;
	}

	// APIQA: set buffer size and fill size
	public void setTunnelBufSize(int tunnelBufSize)
	{
		_tunnelBufSize = tunnelBufSize;
	}
	public void setTunnelFillSize(int tunnelFillSize)
	{
		_tunnelFillSize = tunnelFillSize;
	}	
    // END APIQA: 

    /*
     * Used by the Consumer to open a tunnel stream once the Consumer's channel
     * is opened and the desired service identified.
     */
    public int openStream(ChannelInfo chnlInfo, ReactorErrorInfo errorInfo)
    {
        int ret;

        _serviceId = chnlInfo.qServiceInfo.serviceId();
        
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
        	// APIQA: get a buffer 
        	//TransportBuffer buffer = _chnlInfo.tunnelStream.getBuffer(1024, _errorInfo);
        	TransportBuffer buffer = _chnlInfo.tunnelStream.getBuffer(_tunnelBufSize, _errorInfo);
            // END APIQA: 
            if (buffer == null)
            {
	           	 System.out.println("TunnelStream.getBuffer() failed: " + CodecReturnCodes.toString(_errorInfo.error().errorId())
	             + "(" + _errorInfo.error().text() + ")");
                return;
            }
            
            // put basic text message in buffer
            // APIQA:  Fill in buffer with values 0 to 255 and repeat
            //buffer.data().put(new String("TunnelStream message " + ++_msgCount).getBytes());
            if (_tunnelFillSize == 0)
            {
            	_tunnelFillSize = _tunnelBufSize;
            }
            for (int i = 0, b = 0; i < _tunnelFillSize; i++)
            {
                if (b == 256)
                {
                    b = 0;
                }
                buffer.data().put((byte)b++);
            }
            //END APIQA: 
            
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
                
                // reset message count
                _msgCount = 0;
                
                break;
        }

        return ReactorCallbackReturnCodes.SUCCESS;
    }
}
