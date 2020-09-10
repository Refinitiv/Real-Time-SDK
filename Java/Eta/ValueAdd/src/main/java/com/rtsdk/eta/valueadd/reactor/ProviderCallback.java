package com.rtsdk.eta.valueadd.reactor;

/**
 * Callback used for processing all interactive provider events and messages.
 * 
 * @see ReactorChannelEventCallback
 * @see DefaultMsgCallback
 * @see RDMLoginMsgCallback
 * @see RDMDirectoryMsgCallback
 * @see RDMDictionaryMsgCallback
 */
public interface ProviderCallback extends ReactorChannelEventCallback, DefaultMsgCallback,
        RDMLoginMsgCallback, RDMDirectoryMsgCallback, RDMDictionaryMsgCallback

{

}
