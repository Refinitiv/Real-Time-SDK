package com.rtsdk.eta.valueadd.reactor;

import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.rdm.ClassesOfService;

/**
 * Common class of service properties.
 * 
 * @see ClassOfService
 * @see ClassesOfService
 */
public class CosCommon
{
    /** The current version of the class of service. */
    
    static final int CURRENT_STREAM_VERSION = 2;
    
    public static final int DEFAULT_MAX_MSG_SIZE = 614400;
    public static final int DEFAULT_MAX_FRAGMENT_SIZE = 6144;
    public static final int SUPPS_FRAGMENTATION = 1;
    
    
    int _maxMsgSize = DEFAULT_MAX_MSG_SIZE;
    int _maxFragmentSize = DEFAULT_MAX_FRAGMENT_SIZE;
    int _supportFragmentation = SUPPS_FRAGMENTATION;
    
    int _protocolType = Codec.protocolType();
    int _protocolMajorVersion = Codec.majorVersion();
    int _protocolMinorVersion = Codec.minorVersion();
    int _streamVersion = CURRENT_STREAM_VERSION;
    
    /**
     * Returns the maximum message size. Set by providers to convey the
     * maximum message size it supports.
     *
     * @return the int
     */
    public int maxMsgSize()
    {
        return _maxMsgSize;
    }

    /**
     * Sets the maximum message size. Set by providers to convey the
     * maximum message size it supports. Must be a positive number up
     * to 2,147,483,647.
     *
     * @param maxMsgSize the max msg size
     */
    public void maxMsgSize(int maxMsgSize)
    {
        _maxMsgSize = maxMsgSize;
    }

    /**
     * Returns the maximum fragment size. Set by providers to convey the
     * maximum fragment size it supports.
     *
     * @return the int
     */
    public int maxFragmentSize()
    {
        return _maxFragmentSize;
    }
    
    /**
     * Sets the maximum fragment size. Set by providers to convey the
     * maximum fragment size it supports. Must be a positive number up
     * to 2,147,483,647.
     *
     * @param maxFragmentSize the max fragment size
     */
    public void maxFragmentSize(int maxFragmentSize)
    {
        _maxFragmentSize = maxFragmentSize;
    }

    /**
     * Returns the protocol type of the TunnelStream.
     *
     * @return the int
     */
    public int protocolType()
    {
        return _protocolType;
    }

    /**
     * Sets the protocol type of the TunnelStream.
     *
     * @param protocolType the protocol type
     */
    public void protocolType(int protocolType)
    {
        _protocolType = protocolType;
    }

    /**
     * Returns the protocol major version of the TunnelStream.
     *
     * @return the int
     */
    public int protocolMajorVersion()
    {
        return _protocolMajorVersion;
    }

    /**
     * Sets the protocol major version of the TunnelStream.
     *
     * @param protocolMajorVersion the protocol major version
     */
    public void protocolMajorVersion(int protocolMajorVersion)
    {
        _protocolMajorVersion = protocolMajorVersion;
    }

    /**
     * Returns the protocol minor version of the TunnelStream.
     *
     * @return the int
     */
    public int protocolMinorVersion()
    {
        return _protocolMinorVersion;
    }

    /**
     * Sets the protocol minor version of the TunnelStream.
     *
     * @param protocolMinorVersion the protocol minor version
     */
    public void protocolMinorVersion(int protocolMinorVersion)
    {
        _protocolMinorVersion = protocolMinorVersion;
    }

    /**
     * Stream version.
     *
     * @return the int
     */
    int streamVersion()
    {
        return _streamVersion;
    }
    
    /**
     * Stream version.
     *
     * @param streamVersion the stream version
     */
    void streamVersion(int streamVersion)
    {
        _streamVersion = streamVersion;
    }

    /**
     * Support fragmentation.
     *
     * @return the int
     */
    int supportFragmentation()
    {
		return _supportFragmentation;
	}

	/**
	 * Support fragmentation.
	 *
	 * @param supportFragmentation the support fragmentation
	 */
	void supportFragmentation(int supportFragmentation)
	{
		_supportFragmentation = supportFragmentation;
	}

    /**
     * Clears the CosCommon for re-use.
     */
    public void clear()
    {
        _maxMsgSize = DEFAULT_MAX_MSG_SIZE;
        _maxFragmentSize = DEFAULT_MAX_FRAGMENT_SIZE;
        _supportFragmentation = SUPPS_FRAGMENTATION;
        _protocolType = Codec.protocolType();
        _protocolMajorVersion = Codec.majorVersion();
        _protocolMinorVersion = Codec.minorVersion();
        _streamVersion = CURRENT_STREAM_VERSION;
    }
}
