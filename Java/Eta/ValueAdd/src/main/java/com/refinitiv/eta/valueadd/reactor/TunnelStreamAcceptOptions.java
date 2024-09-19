/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * The options for accepting a TunnelStream.
 * 
 * @see TunnelStream
 * @see ReactorChannel#acceptTunnelStream(TunnelStreamRequestEvent, TunnelStreamAcceptOptions, ReactorErrorInfo)
 *
 */
public class TunnelStreamAcceptOptions
{
    int DEFAULT_OUTPUT_BUFFERS = 50;
    ClassOfService _classOfService = new ClassOfService();
    TunnelStreamDefaultMsgCallback _defaultMsgCallback;
    TunnelStreamStatusEventCallback _statusEventCallback;
    int _guaranteedOutputBuffers = DEFAULT_OUTPUT_BUFFERS;
    Object _userSpecObject;

    /**
     * Returns the class of service of the TunnelStream.
     * Use to set class of service for the TunnelStream.
     *
     * @return the class of service
     * @see ClassOfService
     */
    public ClassOfService classOfService()
    {
        return _classOfService;
    }
    
    /**
     * The TunnelStreamStatusEventCallback of the accepted TunnelStream. Handles stream events
     * for tunnel stream.
     *
     * @param callback the callback
     */
    public void statusEventCallback(TunnelStreamStatusEventCallback callback)
    {
        _statusEventCallback = callback;
    }
    
    /**
     * The TunnelStreamStatusEventCallback of the accepted TunnelStream. Handles stream events
     * for tunnel stream.
     * 
     * @return the tunnelStreamDefaultMsgCallback
     */
    public TunnelStreamStatusEventCallback statusEventCallback()
    {
        return _statusEventCallback;
    }

    /**
     * The TunnelStreamDefaultMsgCallback of the accepted TunnelStream. Handles message events
     * for tunnel stream.
     *
     * @param callback the callback
     */
    public void defaultMsgCallback(TunnelStreamDefaultMsgCallback callback)
    {
        _defaultMsgCallback = callback;
    }
    
    /**
     * The TunnelStreamDefaultMsgCallback of the accepted TunnelStream. Handles message events
     * for tunnel stream.
     * 
     * @return the tunnelStreamDefaultMsgCallback
     */
    public TunnelStreamDefaultMsgCallback defaultMsgCallback()
    {
        return _defaultMsgCallback;
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
     * Returns the user specified object to be set on the TunnelStream.
     *
     * @return the object
     */
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    /**
     * Sets a user specified object to be set on the TunnelStream.
     *
     * @param userSpecObject the user spec object
     */
    public void userSpecObject(Object userSpecObject)
    {
        _userSpecObject = userSpecObject;
    }
    
    /**
     * Clears the TunnelStreamAcceptOptions for re-use.
     */
    public void clear()
    {
        _classOfService.clear();
        _defaultMsgCallback = null;
        _statusEventCallback = null;
        _guaranteedOutputBuffers = DEFAULT_OUTPUT_BUFFERS;
        _userSpecObject = null;
    }
}
