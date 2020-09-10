package com.rtsdk.eta.valueadd.reactor;

/**
 * The default message callback is used to communicate message events
 * to the application.
 */
public interface DefaultMsgCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * message events to the application.
     * 
     * @param event A ReactorMsgEvent containing event information. The
     *            ReactorMsgEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see ReactorMsgEvent
     * @see ReactorCallbackReturnCodes
     */
    public int defaultMsgCallback(ReactorMsgEvent event);
}
