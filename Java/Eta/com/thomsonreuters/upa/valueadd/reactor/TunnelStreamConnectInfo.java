package com.thomsonreuters.upa.valueadd.reactor;

/** Tunnel stream connect information for a tunnel stream listener. */
public class TunnelStreamConnectInfo
{
    ReactorChannel _reactorChannel;
    int _domainType;
    int _streamId;
    int _serviceId;
    String _name;
    int _flags;
    ReactorErrorInfo _errorInfo;
    
    /**
     * Returns the ReactorChannel of the TunnelStream connection request.
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
     * Returns the domain type of the TunnelStream connection request.
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
     * Returns the stream id of the TunnelStream connection request.
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
     * Returns the service identifier of the TunnelStream connection request.
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
     * Returns the name of the TunnelStream connection request.
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
     * Returns the flags of the TunnelStream connection request.
     * 
     * @see TunnelStreamFlags
     */
    public int flags()
    {
        return _flags;
    }

    void flags(int flags)
    {
        _flags = flags;
    }
    
    /**
     * The ReactorErrorInfo associated with this TunnelStream connection request.
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
     * Checks if this TunnelStream has authentication or not.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if TunnelStream has authentication, false - if not.
     */
    public boolean checkAuthentication()
    {
        return (_flags & TunnelStreamFlags.AUTHENTICATION) != 0;
    }
    
    void applyAuthentication()
    {
        _flags |= TunnelStreamFlags.AUTHENTICATION;
    }

    /** 
     * Checks if this TunnelStream has flow control or not.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if TunnelStream has flow control, false - if not.
     */
    public boolean checkFlowControl()
    {
        return (_flags & TunnelStreamFlags.FLOW_CONTROL) != 0;
    }
    
    void applyFlowControl()
    {
        _flags |= TunnelStreamFlags.FLOW_CONTROL;
    }

    /** 
     * Checks if this TunnelStream has reliability or not.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if TunnelStream has reliability, false - if not.
     */
    public boolean checkReliability()
    {
        return (_flags & TunnelStreamFlags.RELIABILITY) != 0;
    }
    
    void applyReliability()
    {
        _flags |= TunnelStreamFlags.RELIABILITY;
    }

    /** 
     * Checks if this TunnelStream has guaranteed messaging or not.
     * 
     * This flag can also be bulk-get by {@link #flags()}
     * 
     * @return true - if TunnelStream has guaranteed messaging, false - if not.
     */
    public boolean checkGuaranteedMessaging()
    {
        return (_flags & TunnelStreamFlags.GUARANTEED_MESSAGING) != 0;
    }
    
    void applyGuaranteedMessaging()
    {
        _flags |= TunnelStreamFlags.GUARANTEED_MESSAGING;
    }

    /**
     * Clears the TunnelStreamConnectInfo for re-use.
     */
    public void clear()
    {
        _domainType = 0;
        _streamId = 0;
        _serviceId = 0;
        _flags = 0;
        _name = null;
    }
}
