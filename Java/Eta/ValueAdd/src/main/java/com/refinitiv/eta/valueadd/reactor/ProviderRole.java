/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Class representing the role of an OMM Interactive Provider.
 * 
 * @see ReactorRole
 * @see ReactorRoleTypes
 */

public class ProviderRole extends ReactorRole
{
    RDMLoginMsgCallback _loginMsgCallback = null;
    RDMDirectoryMsgCallback _directoryMsgCallback = null;
    RDMDictionaryMsgCallback _dictionaryMsgCallback = null;
    TunnelStreamListenerCallback _tunnelStreamListenerCallback = null;
    
    /**
     * Instantiates a new provider role.
     */
    public ProviderRole()
    {
        _type = ReactorRoleTypes.PROVIDER;
    }
    
    /**
     *  A callback function for processing RDMLoginMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     *
     * @param callback the callback
     * @see RDMLoginMsgCallback
     * @see RDMLoginMsgEvent
     */
    public void loginMsgCallback(RDMLoginMsgCallback callback)
    {
        _loginMsgCallback = callback;
    }

    /** A callback function for processing RDMLoginMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @return the loginMsgCallback
     */
    public RDMLoginMsgCallback loginMsgCallback()
    {
        return _loginMsgCallback;
    }
    
    /**
     *  A callback function for processing RDMDirectoryMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     *
     * @param callback the callback
     * @see RDMDirectoryMsgCallback
     * @see RDMDirectoryMsgEvent
     */
    public void directoryMsgCallback(RDMDirectoryMsgCallback callback)
    {
        _directoryMsgCallback = callback;
    }

    /** A callback function for processing RDMDirectoryMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @return the directoryMsgCallback
     */
    public RDMDirectoryMsgCallback directoryMsgCallback()
    {
        return _directoryMsgCallback;
    }
    
    /**
     *  A callback function for processing RDMDictionaryMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     *
     * @param callback the callback
     * @see RDMDictionaryMsgCallback
     * @see RDMDictionaryMsgEvent
     */
    public void dictionaryMsgCallback(RDMDictionaryMsgCallback callback)
    {
        _dictionaryMsgCallback = callback;
    }
    
    /** A callback function for processing RDMDictionaryMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @return the dictionaryMsgCallback
     */
    public RDMDictionaryMsgCallback dictionaryMsgCallback()
    {
        return _dictionaryMsgCallback;
    }
    
    /**
     * A callback function to listen for incoming tunnel stream requests.
     *
     * @param callback the callback
     * @see TunnelStreamListenerCallback
     * @see TunnelStreamRequestEvent
     */
    public void tunnelStreamListenerCallback(TunnelStreamListenerCallback callback)
    {
        _tunnelStreamListenerCallback = callback;
    }

    /** A callback function to listen for incoming tunnel stream requests.
     * 
     * @return the tunnelStreamListenerCallback
     */
    public TunnelStreamListenerCallback tunnelStreamListenerCallback()
    {
        return _tunnelStreamListenerCallback;
    }

    /*
     * Performs a deep copy from a specified ProviderRole into this ProviderRole.
     * Only public facing attributes are copied.
     */
    void copy(ProviderRole role)
    {
        super.copy(role);
        _loginMsgCallback = role.loginMsgCallback();
        _directoryMsgCallback = role.directoryMsgCallback();
        _dictionaryMsgCallback = role.dictionaryMsgCallback();
        _tunnelStreamListenerCallback = role.tunnelStreamListenerCallback();
    }
}
