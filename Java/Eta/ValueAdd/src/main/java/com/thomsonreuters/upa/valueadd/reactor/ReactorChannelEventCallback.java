package com.thomsonreuters.upa.valueadd.reactor;

/**
 * The Reactor channel event callback is used to communicate ReactorChannel and
 * connection state information to the application.
 */
public interface ReactorChannelEventCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * {@link ReactorChannel} and connection state information to the
     * application.
     * 
     * @param event A ReactorChannelEvent containing event information. The
     *            ReactorChannelEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see ReactorChannelEvent
     * @see ReactorCallbackReturnCodes
     */
    public int reactorChannelEventCallback(ReactorChannelEvent event);
}
