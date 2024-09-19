/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.TransportReturnCodes;

class RestReactorOptions {

	final static int DEFAULT_SHUTDOWN_TIMEOUT_IN_MILLISECONDS = 500;
	final static int DEFAULT_SELECT_INTERVAL_IN_MILLISECONDS = 1000;

    /**
     * Determines time interval in milliseconds at which the I/O reactor wakes up to check for
     * timed out sessions and session requests.
     */
    private long _selectInterval;
    /*Determines grace period in milliseconds the I/O reactors are expected to block waiting
     * for individual worker threads to terminate cleanly.*/
    private long _shutdownGracePeriod;
    /*The maximum number of connections allowed for a host that has not been specified otherwise by a call to setMaxPerRoute.
     *Use setMaxPerRoute when you know the route ahead of time and setDefaultMaxPerRoute when you do not.
     */
	private int _defaultMaxPerRoute; 
	 /**
     * Determines the number of I/O dispatch threads to be used by the I/O reactor.
     */
    private int _ioThreadCount;
    /**
     * Determines the default socket timeout value for non-blocking I/O operations.
     * his is the elapsed time since the client has sent request to the server before server responds.
     */
    private int _soTimeout; 
    /**
     * Determines the default connect timeout value for non-blocking connection requests.
     * This is the time elapsed before the connection established or Server responded to connection request.
     * <p/>
     */
    private int _connectTimeout; 
    /**
     * The maximum number of connections allowed across all hosts
     * <p/>
     */
    private int _maxConnectTotal; 
    private int _sndBufSize; //not use any more
    private int _rcvBufSize; //not use any more
    private boolean _soKeepAlive;
    private boolean _tcpNoDelay;
    private int _bufferSize;
    private int _fragmentSizeHint;
	
    public RestReactorOptions()
    {
    	clear();
    }

    public void clear()
	{
		_selectInterval = DEFAULT_SELECT_INTERVAL_IN_MILLISECONDS;
	    _shutdownGracePeriod = DEFAULT_SHUTDOWN_TIMEOUT_IN_MILLISECONDS;
	    _ioThreadCount = 2;
	    _soTimeout = 0;
	    _soKeepAlive = false;
	    _tcpNoDelay = true;
	    _connectTimeout = 0;
	    _sndBufSize = 0;
	    _rcvBufSize = 0;
	    _maxConnectTotal = 2;
	    _defaultMaxPerRoute = 2;
		_bufferSize = 8 * 1024;
		_fragmentSizeHint = -1;
	}

	public void selectInterval(long selectInterval)
	{
		_selectInterval = selectInterval;
	}
	
	public long selectInterval()
	{
		return _selectInterval;
	}
	
	public void shutdownGracePeriod(long shutdownGracePeriod)
	{
		_shutdownGracePeriod = shutdownGracePeriod;
	}
	
	public long shutdownGracePeriod()
	{
		return _shutdownGracePeriod;
	}
	
	public void ioThreadCount(int ioThreadCount)
	{
		_ioThreadCount = ioThreadCount;
	}
	
	public int ioThreadCount()
	{
		return _ioThreadCount;
	}
	
	public void soTimeout(int soTimeout)
	{
		_soTimeout = soTimeout;
	}
	
	public int soTimeout()
	{
		return _soTimeout;
	}
	
	public void soKeepAlive(boolean soKeepAlive)
	{
		_soKeepAlive = soKeepAlive;
	}
	
	public boolean soKeepAlive()
	{
		return _soKeepAlive;
	}
	
	public void tcpNoDelay(boolean tcpNoDelay)
	{
		_tcpNoDelay = tcpNoDelay;
	}
	
	public boolean tcpNoDelay()
	{
		return _tcpNoDelay;
	}
	
	public void maxConnectTotal(int maxConnectTotal)
	{
		_maxConnectTotal = maxConnectTotal;
	}
	
	public int maxConnectTotal()
	{
		return _maxConnectTotal;
	}
	
	public void defaultMaxPerRoute(int defaultMaxPerRoute)
	{
		_defaultMaxPerRoute = defaultMaxPerRoute;
	}
	
	public int defaultMaxPerRoute()
	{
		return _defaultMaxPerRoute;
	}
	
	public void connectTimeout(int connectTimeout)
	{
		_connectTimeout = connectTimeout;
	}
	
	public int connectTimeout()
	{
		return _connectTimeout;
	}
    
    public String toString()
	{
		 return "RestReactorOptions" + "\n" + 
	               "\t_selectInterval: " + _selectInterval + "\n" +
	               "\t_shutdownGracePeriod: " + _shutdownGracePeriod + "\n" + 
	               "\tioThreadCount: " + _ioThreadCount + "\n" + 
	               "\tsoTimeout: " + _soTimeout + "\n" + 
	               "\tsoKeepAlive: " + _soKeepAlive + "\n" + 
	               "\ttcpNoDelay: " + _tcpNoDelay + "\n" + 
	               "\tconnectTimeout: " + _connectTimeout+ "\n" + 
	               "\tsndBufSize: " + _sndBufSize + "\n" + 
	               "\trcvBufSize: " + _rcvBufSize+ "\n" +
	               "\tbufferSize: " + _bufferSize + "\n" + 
	               "\tfragmentSizeHint: " + _fragmentSizeHint + "\n" +
	               "\tdefaultMaxPerRoute: " + _defaultMaxPerRoute+ "\n" + 
	               "\tmaxConnectTotal: " + _maxConnectTotal + "\n";
	}
    
    public int copy(RestReactorOptions destOpts)
    {
        if (destOpts == null)
            return TransportReturnCodes.FAILURE;

        destOpts._selectInterval = _selectInterval;
        destOpts._shutdownGracePeriod = _shutdownGracePeriod;
        destOpts._ioThreadCount = _ioThreadCount;
        destOpts._soTimeout = _soTimeout;
        destOpts._soKeepAlive = _soKeepAlive;
        destOpts._tcpNoDelay = _tcpNoDelay;
        destOpts._connectTimeout = _connectTimeout;
        destOpts._sndBufSize = _sndBufSize;
        destOpts._rcvBufSize = _rcvBufSize;
        destOpts._defaultMaxPerRoute = _defaultMaxPerRoute;
        destOpts._maxConnectTotal = _maxConnectTotal;
        
        return TransportReturnCodes.SUCCESS;
    }
    
	public void bufferSize(int bufferSize)
	{
		_bufferSize = bufferSize;
	}
	
	public int bufferSize()
	{
		return _bufferSize;
	}
	
	public void fragmentSizeHint(int fragmentSizeHint)
	{
		_bufferSize = fragmentSizeHint;
	}
	
	public int fragmentSizeHint()
	{
		return _fragmentSizeHint;
	}
}

