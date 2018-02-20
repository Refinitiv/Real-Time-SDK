package com.thomsonreuters.upa.valueadd.reactor;

/**
 * The RDM dictionary message callback is used to communicate dictionary
 * message events to the application.
 */
public interface RDMDictionaryMsgCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * dictionary message events to the application.
     * 
     * @param event A ReactorMsgEvent containing event information. The
     *            ReactorMsgEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see RDMDictionaryMsgEvent
     * @see ReactorCallbackReturnCodes
     */
	public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event);
}
