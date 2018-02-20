package com.thomsonreuters.upa.valueadd.reactor;

/**
 * The queue message callback is used to communicate queue
 * message events to the application.
 */
public interface TunnelStreamQueueMsgCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * queue message events to the application.
     * 
     * @param event A TunnelStreamQueueMsgEvent containing event information. The
     *            TunnelStreamQueueMsgEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see TunnelStream
     * @see TunnelStreamQueueMsgEvent
     * @see ReactorCallbackReturnCodes
     */
    public int queueMsgCallback(TunnelStreamQueueMsgEvent event);

}
