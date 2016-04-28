package com.thomsonreuters.upa.valueadd.reactor;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.rdm.ClassesOfService;

/**
 * Common class of service properties.
 * 
 * @see ClassOfService
 * @see ClassesOfService
 */
public class CosCommon
{
    /** The current version of the class of service. */
    static final int CURRENT_STREAM_VERSION = 1;
    
    public static final int DEFAULT_MAX_MSG_SIZE = 6144;
    
    int _maxMsgSize = DEFAULT_MAX_MSG_SIZE;
    int _protocolType = Codec.protocolType();
    int _protocolMajorVersion = Codec.majorVersion();
    int _protocolMinorVersion = Codec.minorVersion();
    int _streamVersion = CURRENT_STREAM_VERSION;
    
    /**
     * Returns the maximum message size. Set by providers to convey the
     * maximum message size it supports.
     */
    public int maxMsgSize()
    {
        return _maxMsgSize;
    }

    /**
     * Sets the maximum message size. Set by providers to convey the
     * maximum message size it supports. Must be a positive number up
     * to 2,147,483,647.
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

    int streamVersion()
    {
        return _streamVersion;
    }
    
    void streamVersion(int streamVersion)
    {
        _streamVersion = streamVersion;
    }

    /**
     * Clears the CosCommon for re-use.
     */
    public void clear()
    {
        _maxMsgSize = DEFAULT_MAX_MSG_SIZE;
        _protocolType = Codec.protocolType();
        _protocolMajorVersion = Codec.majorVersion();
        _protocolMinorVersion = Codec.minorVersion();
        _streamVersion = CURRENT_STREAM_VERSION;
    }
}
