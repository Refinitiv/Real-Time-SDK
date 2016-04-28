package com.thomsonreuters.upa.valueadd.reactor;

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
    
    public ProviderRole()
    {
        _type = ReactorRoleTypes.PROVIDER;
    }
    
    /** A callback function for processing RDMLoginMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @param callback
     * 
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
    
    /** A callback function for processing RDMDirectoryMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @param callback
     * 
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
    
    /** A callback function for processing RDMDictionaryMsgEvents received. If not present,
     * the received message will be passed to the defaultMsgCallback.
     * 
     * @param callback
     * 
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
     * @param callback
     * 
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
}
