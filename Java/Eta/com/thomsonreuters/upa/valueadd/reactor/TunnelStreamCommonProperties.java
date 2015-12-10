package com.thomsonreuters.upa.valueadd.reactor;

/**
 * Tunnel stream common properties. Part of tunnel stream class of service.
 *
 * @see TunnelStreamClassOfService
 */
public class TunnelStreamCommonProperties
{
    int _maxMsgSize;
    int _protocolType;
    int _protocolMajorVersion;
    int _protocolMinorVersion;
    int _streamVersion;

    /**
     * Returns the maximum message size of the TunnelStream. Set by providers
     * to convey the maximum tunnel stream message size it supports.
     */
    public int maxMsgSize()
    {
        return _maxMsgSize;
    }

    /**
     * Sets the stream version of the TunnelStream. Set by providers to convey
     * the maximum tunnel stream message size it supports. Must be in the range
     * of 1 - 2,147,483,647.
     */
    public void maxMsgSize(int maxMsgSize)
    {
        _maxMsgSize = maxMsgSize;
    }

    /**
     * Returns the protocol type of the TunnelStream.
     */
    public int protocolType()
    {
        return _protocolType;
    }

    /**
     * Sets the protocol type of the TunnelStream.
     */
    public void protocolType(int protocolType)
    {
        _protocolType = protocolType;
    }

    /**
     * Returns the protocol major version of the TunnelStream.
     */
    public int protocolMajorVersion()
    {
        return _protocolMajorVersion;
    }

    /**
     * Sets the protocol major version of the TunnelStream.
     */
    public void protocolMajorVersion(int protocolMajorVersion)
    {
        _protocolMajorVersion = protocolMajorVersion;
    }

    /**
     * Returns the protocol minor version of the TunnelStream.
     */
    public int protocolMinorVersion()
    {
        return _protocolMinorVersion;
    }

    /**
     * Sets the protocol minor version of the TunnelStream.
     */
    public void protocolMinorVersion(int protocolMinorVersion)
    {
        _protocolMinorVersion = protocolMinorVersion;
    }

    /**
     * Returns the stream version of the TunnelStream.
     */
    public int streamVersion()
    {
        return _streamVersion;
    }
    
    /**
     * Sets the stream version of the TunnelStream.
     */
    public void streamVersion(int streamVersion)
    {
        _streamVersion = streamVersion;
    }

    /**
     * Clears the TunnelStreamCommonProperties for re-use.
     */
    public void clear()
    {
        _maxMsgSize = 0;
        _protocolType = 0;
        _protocolMajorVersion = 0;
        _protocolMinorVersion = 0;
        _streamVersion = 0;
    }
}
