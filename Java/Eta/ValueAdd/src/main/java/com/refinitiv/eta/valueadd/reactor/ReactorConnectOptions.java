/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.util.ArrayList;
import java.util.List;

/**
 * ReactorConnectOptions to be used in the {@link Reactor#connect(ReactorConnectOptions,
 * ReactorRole, ReactorErrorInfo)} call.
 */
public class ReactorConnectOptions
{
    private final int DEFAULT_DELAY = 1000;
    
    List<ReactorConnectInfo> _connectionList;
    int _reconnectAttemptLimit;
    int _reconnectMinDelay = DEFAULT_DELAY;
    int _reconnectMaxDelay = DEFAULT_DELAY;

    ReactorConnectOptions()
    {
        _connectionList = new ArrayList<ReactorConnectInfo>(10);
    }

    /** 
     * A list of connections. Add at least one connection to the list.
     * Additional connections are used as backups for fail-over in case
     * the first connection is lost. Each connection in the list will be
     * tried with each reconnection attempt. 
     * 
     * @return the list of connections
     */
    public List<ReactorConnectInfo> connectionList()
	{
		return _connectionList;
	}

    /**
     * Returns the reconnectAttemptLimit value.
     * 
     * @return the reconnectAttemptLimit value
     */
	public int reconnectAttemptLimit()
	{
		return _reconnectAttemptLimit;
	}

	/**
	 * The maximum number of times the Reactor will attempt to
	 * reconnect a channel. If set to -1, there is no limit.
	 * Must be in the range of -1 - 2147483647. Default is 0.
     * 
     * @param limit the maximum number of reconnect attempts
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the limit is valid,
     *         otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE} if
     *         the limit is out of range
     */
	public int reconnectAttemptLimit(int limit)
	{
        if (limit < -1)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;

        _reconnectAttemptLimit = limit;
        return ReactorReturnCodes.SUCCESS;
	}

    /**
     * Returns the reconnectMinDelay value.
     * 
     * @return the reconnectMinDelay value
     */
	public int reconnectMinDelay()
	{
		return _reconnectMinDelay;
	}

	/**
	 * The minimum time the Reactor will wait before attempting
	 * to reconnect, in milliseconds. Must be in the range of 1000
	 * - 2147483647. Default is 1000. Default value is used if
	 * value is greater than or equal to zero and less than 1000.
	 *
     * @param delay the minimum reconnect delay
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the delay is valid,
     *         otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE} if
     *         the delay is less than zero
	 */
	public int reconnectMinDelay(int delay)
	{
        if (delay < 0)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;
        
        if (delay < DEFAULT_DELAY)
            _reconnectMinDelay = DEFAULT_DELAY;
        else
            _reconnectMinDelay = delay;
        
        // make sure _reconnectMaxDelay is at least _reconnectMinDelay
        if (_reconnectMaxDelay < _reconnectMinDelay)
            _reconnectMaxDelay = _reconnectMinDelay;
        
        return ReactorReturnCodes.SUCCESS;
	}

    /**
     * Returns the reconnectMaxDelay value.
     * 
     * @return the reconnectMaxDelay value
     */
	public int reconnectMaxDelay()
	{
		return _reconnectMaxDelay;
	}

	/**
	 * The maximum time the Reactor will wait before attempting
	 * to reconnect, in milliseconds. Must be in the range of 1000
	 * - 2147483647 and must be greater than or equal to
	 * reconnectMinDelay. Default is 1000. reconnectMinDelay value
	 * is used if value is less than reconnectMinDelay.
	 *
     * @param delay the maximum reconnect delay
     * 
     * @return {@link ReactorReturnCodes#SUCCESS} if the delay is valid,
     *         otherwise {@link ReactorReturnCodes#PARAMETER_OUT_OF_RANGE} if
     *         the delay is less than zero
	 */
	public int reconnectMaxDelay(int delay)
	{
        if (delay < 0 || delay < _reconnectMinDelay)
            return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;

        if (_reconnectMaxDelay < _reconnectMinDelay)
            _reconnectMaxDelay = _reconnectMinDelay;
        else
            _reconnectMaxDelay = delay;
        
        return ReactorReturnCodes.SUCCESS;
	}

    /**
     * Clears this object for reuse.
     */
    public void clear()
    {
    	_connectionList.clear();
        _reconnectAttemptLimit = 0;
        _reconnectMinDelay = DEFAULT_DELAY;
        _reconnectMaxDelay = DEFAULT_DELAY;
    }

    /**
     * This method will perform a deep copy into the passed in parameter's 
     *          members from the Object calling this method.
     * 
     * @param destOpts the value getting populated with the values of the calling Object
     *  
     * @return {@link ReactorReturnCodes#SUCCESS} on success,
     *         {@link ReactorReturnCodes#FAILURE} if the destOpts is null. 
     */
    public int copy(ReactorConnectOptions destOpts)
    {
        if (destOpts == null)
            return ReactorReturnCodes.FAILURE;

        destOpts._connectionList.clear();
        for (int i = 0; i < _connectionList.size(); ++i)
        {
            ReactorConnectInfo reactorConnectInfo = new ReactorConnectInfo();
            _connectionList.get(i).copy(reactorConnectInfo);
            destOpts._connectionList.add(reactorConnectInfo);
        }

        destOpts._reconnectAttemptLimit = _reconnectAttemptLimit;
        destOpts._reconnectMinDelay = _reconnectMinDelay;
        destOpts._reconnectMaxDelay = _reconnectMaxDelay;
        return ReactorReturnCodes.SUCCESS;
    }
}
