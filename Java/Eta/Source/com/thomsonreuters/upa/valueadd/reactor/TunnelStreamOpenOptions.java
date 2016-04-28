package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;

/**
 * The options for opening a TunnelStream.
 * 
 * @see TunnelStream
 * @see ReactorChannel#openTunnelStream(TunnelStreamOpenOptions, ReactorErrorInfo)
 *
 */
public class TunnelStreamOpenOptions
{
    int DEFAULT_TIMEOUT = 60;
    int DEFAULT_OUTPUT_BUFFERS = 50;
    int _domainType;
    int _streamId;
    int _serviceId;
    int _responseTimeout = DEFAULT_TIMEOUT;
    int _guaranteedOutputBuffers = DEFAULT_OUTPUT_BUFFERS;
    ClassOfService _classOfService = new ClassOfService();
    TunnelStreamDefaultMsgCallback _defaultMsgCallback;
    TunnelStreamStatusEventCallback _statusEventCallback;
    TunnelStreamQueueMsgCallback _queueMsgCallback;
    LoginRequest _authLoginRequest;
    String _name;
	Object _userSpecObject;
    
    /**
     * Returns the domain type of the TunnelStream.
     */
    public int domainType()
    {
        return _domainType;
    }

    /**
     * Sets the domain type of the TunnelStream.
     */
    public void domainType(int domainType)
    {
        _domainType = domainType;
    }

    /**
     * Returns the stream id of the TunnelStream.
     */
    public int streamId()
    {
        return _streamId;
    }

    /**
     * Sets the stream id of the TunnelStream.
     */
    public void streamId(int streamId)
    {
        _streamId = streamId;
    }

    /**
     * Returns the service identifier of the TunnelStream.
     */
    public int serviceId()
    {
        return _serviceId;
    }

    /**
     * Sets the service identifier of the TunnelStream.
     */
    public void serviceId(int serviceId)
    {
        _serviceId = serviceId;
    }

    /**
     * The amount of time (in seconds) to wait for a provider to respond to a
     * TunnelStream open request. If the provider does not respond in time,
     * a TunnelStreamStatusEvent is sent to the application to indicate the
     * TunnelStream could not be opened. Timeout must be greater than zero.
     * Default is 60 seconds.
     * 
     * @param timeout the response timeout in seconds
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the timeout is valid,
     *         otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE} if
     *         the timeout is out of range
     */
    public int responseTimeout(int timeout)
    {
        if (timeout < 1)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;

        _responseTimeout = timeout;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the response timeout value.
     * 
     * @return the response timeout value
     */
    public int responseTimeout()
    {
        return _responseTimeout;
    }

    /**
     * Sets the number of guaranteed output buffers that will be available
     * for the tunnel stream. Must be greater than 0.
     * 
     * @param numBuffers the number of guaranteed output buffers
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the number of guaranteed
     *         output buffers is valid, otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE}
     *         if the number of guaranteed output buffers is out of range
     */
    public int guaranteedOutputBuffers(int numBuffers)
    {
        if (numBuffers < 1)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;

        _guaranteedOutputBuffers = numBuffers;
        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Returns the number of guaranteed output buffers that will be available
     * for the tunnel stream.
     * 
     * @return the number of guaranteed output buffers
     */
    public int guaranteedOutputBuffers()
    {
        return _guaranteedOutputBuffers;
    }

    /**
     * The TunnelStreamStatusEventCallback of the TunnelStream. Handles stream events
     * for tunnel stream.
     * 
     * @param callback
     */
    public void statusEventCallback(TunnelStreamStatusEventCallback callback)
    {
        _statusEventCallback = callback;
    }
    
    /**
     * The TunnelStreamStatusEventCallback of the TunnelStream. Handles stream events
     * for tunnel stream.
     * 
     * @return the tunnelStreamDefaultMsgCallback
     */
    public TunnelStreamStatusEventCallback statusEventCallback()
    {
        return _statusEventCallback;
    }

    /**
     * The TunnelStreamDefaultMsgCallback of the TunnelStream. Handles message events
     * for tunnel stream.
     * 
     * @param callback
     */
    public void defaultMsgCallback(TunnelStreamDefaultMsgCallback callback)
    {
        _defaultMsgCallback = callback;
    }
    
    /**
     * The TunnelStreamDefaultMsgCallback of the TunnelStream. Handles message events
     * for tunnel stream.
     * 
     * @return the tunnelStreamDefaultMsgCallback
     */
    public TunnelStreamDefaultMsgCallback defaultMsgCallback()
    {
        return _defaultMsgCallback;
    }
    
    /**
     * The QueueMsgCallback of the TunnelStream. Handles message events
     * for queue message streams.
     * 
     * @param callback
     */
    public void queueMsgCallback(TunnelStreamQueueMsgCallback callback)
    {
        _queueMsgCallback = callback;
    }
    
    /**
     * The QueueMsgCallback of the TunnelStream. Handles message events
     * for queue message streams.
     * 
     * @return the queueMsgCallback
     */
    public TunnelStreamQueueMsgCallback queueMsgCallback()
    {
        return _queueMsgCallback;
    }
    
    /**
     * Returns the login request to send, if using authentication.
     */
    public LoginRequest authLoginRequest()
    {
        return _authLoginRequest;
    }

    /**
     * Sets the login request to send, if using authentication.
     */
    public void authLoginRequest(LoginRequest loginRequest)
    {
        _authLoginRequest = loginRequest;
    }

    /**
     * Returns the name of the TunnelStream.
     */
    public String name()
    {
        return _name;
    }

    /**
     * Sets the name of the TunnelStream.
     */
    public void name(String name)
    {
        _name = name;
    }

    /**
     * Returns the user specified object to be set on the TunnelStream.
     */
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    /**
     * Sets a user specified object to be set on the TunnelStream.
     */
    public void userSpecObject(Object userSpecObject)
    {
        _userSpecObject = userSpecObject;
    }
    
    /**
     * Returns the class of service of the TunnelStream.
     * Use to set class of service for the TunnelStream.
     * 
     * @see ClassOfService
     */
    public ClassOfService classOfService()
    {
        return _classOfService;
    }

    /**
     * Clears the TunnelStreamOpenOptions for re-use.
     */
    public void clear()
    {
        _domainType = 0;
        _streamId = 0;
        _serviceId = 0;
        _responseTimeout = DEFAULT_TIMEOUT;
        _guaranteedOutputBuffers = DEFAULT_OUTPUT_BUFFERS;
        _classOfService.clear();
        _defaultMsgCallback = null;
        _statusEventCallback = null;
        _queueMsgCallback = null;
        _authLoginRequest = null;
        _name = null;
		_userSpecObject = null;
    }
}
