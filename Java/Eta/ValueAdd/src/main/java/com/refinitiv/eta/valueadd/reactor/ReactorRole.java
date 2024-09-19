/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/** ReactorRole base class. Used by all other role classes. */ 
public class ReactorRole
{
    int _type = 0; // ReactorRoleTypes
    ReactorChannelEventCallback _channelEventCallback = null;
    DefaultMsgCallback _defaultMsgCallback = null;

    /**
     * The role type.
     * 
     * @return the role type
     * 
     * @see ReactorRoleTypes
     */
    public int type()
    {
        return _type;
    }

    /**
     * The ReactorChannelEventCallback associated with this role. Handles channel
     * events. Must be provided for all roles.
     *
     * @param callback the callback
     */
    public void channelEventCallback(ReactorChannelEventCallback callback)
    {
        _channelEventCallback = callback;
    }
    
    /**
     * The ReactorChannelEventCallback associated with this role. Handles channel
     * events. Must be provided for all roles.
     * 
     * @return the channelEventCallback
     */
    public ReactorChannelEventCallback channelEventCallback()
    {
        return _channelEventCallback;
    }

    /**
     * The DefaultMsgCallback associated with this role. Handles message events
     * that aren't handled by a specific domain callback. Must be provided for
     * all roles.
     *
     * @param callback the callback
     */
    public void defaultMsgCallback(DefaultMsgCallback callback)
    {
        _defaultMsgCallback = callback;
    }
    
    /**
     * The DefaultMsgCallback associated with this role. Handles message events
     * that aren't handled by a specific domain callback. Must be provided for
     * all roles.
     * 
     * @return the defaultMsgCallback
     */
    public DefaultMsgCallback defaultMsgCallback()
    {
        return _defaultMsgCallback;
    }

    /**
     * Returns a String representation of this object.
     * 
     * @return a String representation of this object
     */
    public String toString()
    {
        return "ReactorRole: " + ReactorRoleTypes.toString(_type);
    }
    
    /*
     * Performs a deep copy from a specified ReactorRole into this ReactorRole.
     */
    void copy(ReactorRole role)
    {
        _channelEventCallback = role._channelEventCallback;
        _defaultMsgCallback =  role._defaultMsgCallback;
    }
}
