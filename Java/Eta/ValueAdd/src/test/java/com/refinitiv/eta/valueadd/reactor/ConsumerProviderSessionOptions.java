///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.transport.CompressionTypes;

public class ConsumerProviderSessionOptions {
	private int _connectionType;
	private int _protocolType;
	private String _protocolList;
    private int _reconnectAttemptLimit;
    private int _reconnectMinDelay = 1000;
    private int _reconnectMaxDelay = 1000;
	private boolean _setupDefaultLoginStream;
	private boolean _setupDefaultDirectoryStream;
	private boolean _setupSecondDefaultDirectoryStream;
	private int _numStatusEvents;
	private int _pingTimeout = 60;
    private int _consumerChannelInitTimeout = 60;
    private long _openWindow = -1;
    private int numOfGuaranteedBuffers = 50;
    private int _compressionType = CompressionTypes.NONE;
    
    private int _wsbMode = ReactorWarmStandbyMode.LOGIN_BASED;
	
	/** Returns the type of connection the session will use. */
	public int connectionType() { return _connectionType; }
	
	/** Sets the type of connection the session will use. */
	public void connectionType(int connectionType) { _connectionType = connectionType; }
	
	/** Sets whether a default login stream will be setup.
	 * If set to true, the consumer's reactor role must have a preset login request. */
	public void setupDefaultLoginStream(boolean setupDefaultLoginStream)
	{
		_setupDefaultLoginStream = setupDefaultLoginStream;
	}
	
	/** Returns whether a default login stream will be setup. */
	public boolean setupDefaultLoginStream()
	{
		return _setupDefaultLoginStream;
	}
		
	/** Sets whether a default directory stream will be setup.
	 * If set to true, either the consumer's reactor role must have a preset directory request,
	 * or its watchlist is enabled (or both). */
	public void setupDefaultDirectoryStream(boolean setupDefaultDirectoryStream)
	{
		_setupDefaultDirectoryStream = setupDefaultDirectoryStream;
	}
	
	/** Returns whether a default directory stream will be setup. */
	public boolean setupDefaultDirectoryStream()
	{
		return _setupDefaultDirectoryStream;
	}

	/** Sets whether a second default directory stream will be setup.
	 * If set to true, either the consumer's reactor role must have a preset directory request,
	 * or its watchlist is enabled (or both). */
	public void setupSecondDefaultDirectoryStream(boolean setupDefaultDirectoryStream)
	{
		_setupSecondDefaultDirectoryStream = setupDefaultDirectoryStream;
	}
	
	/** Returns whether a second default directory stream will be setup. */
	public boolean setupSecondDefaultDirectoryStream()
	{
		return _setupSecondDefaultDirectoryStream;
	}

    /** Returns the reconnectAttemptLimit. */
    public int reconnectAttemptLimit()
    {
        return _reconnectAttemptLimit;
    }

    /** Sets the reconnectAttemptLimit. */
    public void reconnectAttemptLimit(int reconnectAttemptLimit)
    {
        _reconnectAttemptLimit = reconnectAttemptLimit;
    }
    
    /** Returns the reconnectMinDelay. */
    public int reconnectMinDelay()
    {
        return _reconnectMinDelay;
    }

    /** Sets the reconnectMinDelay. */
    public void reconnectMinDelay(int reconnectMinDelay)
    {
        _reconnectMinDelay = reconnectMinDelay;
    }
    
    /** Returns the reconnectMaxDelay. */
    public int reconnectMaxDelay()
    {
        return _reconnectMaxDelay;
    }

    /** Sets the reconnectMaxDelay. */
    public void reconnectMaxDelay(int reconnectMaxDelay)
    {
        _reconnectMaxDelay = reconnectMaxDelay;
    }
    
    /** Returns the pingTimeout the consumer and provider will use. */
    public int pingTimeout()
    {
        return _pingTimeout;
    }

    /** Sets the pingTimeout the consumer and provider will use. */
    public void pingTimeout(int pingTimeout)
    {
        _pingTimeout = pingTimeout;
    }

    /** Returns the consumer channel's initialization timeout. */
    public int consumerChannelInitTimeout()
    {
        return _consumerChannelInitTimeout;
    }

    /** Sets the consumer channel's initializationTimeout. */
    public void consumerChannelInitTimeout(int consumerChannelInitTimeout)
    {
        _consumerChannelInitTimeout = consumerChannelInitTimeout;
    }

	/* Returns the number of status events received before the CHANNEL_READY event. Used for watchlist channel open callback submit status messages. */
	public int numStatusEvents()
	{
	    return _numStatusEvents;
	}
	
    /* Sets the number of status events received before the CHANNEL_READY event. Used for watchlist channel open callback submit status messages. */
    public void numStatusEvents(int numStatusEvents)
    {
        _numStatusEvents = numStatusEvents;
    }
    
    /** Returns the openWindow the provider will use on its service. */
    public long openWindow()
    {
    	return _openWindow;
    }
    
    /** Sets the openWindow the provider will use on its service. By default, value is -1 and no OpenWindow is defined. */
    public void openWindow(long openWindow)
    {
    	_openWindow = openWindow;
    }

    public int getProtocolType() {
        return _protocolType;
    }

    public void setProtocolType(int _protocolType) {
        this._protocolType = _protocolType;
    }

    public String getProtocolList() {
        return _protocolList;
    }

    public void setProtocolList(String _protocolList) {
        this._protocolList = _protocolList;
    }

    public int getNumOfGuaranteedBuffers() {
        return numOfGuaranteedBuffers;
    }

    public void setNumOfGuaranteedBuffers(int numOfGuaranteedBuffers) {
        this.numOfGuaranteedBuffers = numOfGuaranteedBuffers;
    }

	public int compressionType() {
		return _compressionType;
	}

	public void compressionType(int compressionType) {
		this._compressionType = compressionType;
	}
	
	public int wsbMode() {
		return _wsbMode;
	}

	public void wsbMode(int wsbMode) {
		this._wsbMode = wsbMode;
	}
}
