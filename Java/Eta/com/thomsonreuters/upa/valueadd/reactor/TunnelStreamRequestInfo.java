package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.RequestMsg;

/** Tunnel stream request information for a tunnel stream listener. */
public class TunnelStreamRequestInfo
{
    ReactorChannel _reactorChannel;
    int _domainType;
    int _streamId;
    int _serviceId;
    String _name;
    RequestMsg _msg;
    TunnelStreamClassOfService _classOfService = new TunnelStreamClassOfService();
    ReactorErrorInfo _errorInfo;
    
    /**
     * Returns the ReactorChannel of the TunnelStream request.
     */
    public ReactorChannel reactorChannel()
    {
        return _reactorChannel;
    }

    void reactorChannel(ReactorChannel reactorChannel)
    {
        _reactorChannel = reactorChannel;
    }

    /**
     * Returns the domain type of the TunnelStream request.
     */
    public int domainType()
    {
        return _domainType;
    }

    void domainType(int domainType)
    {
        _domainType = domainType;
    }

    /**
     * Returns the stream id of the TunnelStream request.
     */
    public int streamId()
    {
        return _streamId;
    }

    void streamId(int streamId)
    {
        _streamId = streamId;
    }

    /**
     * Returns the service identifier of the TunnelStream request.
     */
    public int serviceId()
    {
        return _serviceId;
    }

    void serviceId(int serviceId)
    {
        _serviceId = serviceId;
    }
    
    /**
     * Returns the name of the TunnelStream request.
     */
    public String name()
    {
        return _name;
    }

    void name(String name)
    {
        _name = name;
    }

    /**
     * Returns the TunnelStream request message.
     */
    public RequestMsg msg()
    {
        return _msg;
    }

    void msg(RequestMsg msg)
    {
        _msg = msg;
    }

    /**
     * The ReactorErrorInfo associated with this TunnelStream request.
     * 
     * @return ReactorErrorInfo
     */
    public ReactorErrorInfo errorInfo()
    {
        return _errorInfo;
    }
    
    void errorInfo(ReactorErrorInfo errorInfo)
    {
        _errorInfo = errorInfo;
    }
    
    /**
     * Returns the class of service of the TunnelStream request.
     * 
     * @see TunnelStreamClassOfService
     */
    public TunnelStreamClassOfService classOfService()
    {
        return _classOfService;
    }

    /**
     * Clears the TunnelStreamRequestInfo for re-use.
     */
    public void clear()
    {
        _domainType = 0;
        _streamId = 0;
        _serviceId = 0;
        _name = null;
        _classOfService.clear();
    }
}
