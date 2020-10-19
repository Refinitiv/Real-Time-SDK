package com.refinitiv.eta.valueadd.reactor;

/**
 * Callback used for processing all consumer events and messages.
 * 
 * @see ReactorChannelEventCallback
 * @see DefaultMsgCallback
 * @see RDMLoginMsgCallback
 * @see RDMDirectoryMsgCallback
 * @see RDMDictionaryMsgCallback
 */
public interface ConsumerCallback extends ReactorChannelEventCallback, DefaultMsgCallback,
        RDMLoginMsgCallback, RDMDirectoryMsgCallback, RDMDictionaryMsgCallback
{

}
