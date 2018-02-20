package com.thomsonreuters.upa.valueadd.reactor;

/**
 * The RDM login message callback is used to communicate login
 * message events to the application.
 */
public interface RDMLoginMsgCallback
{
    /**
     * A callback function that the {@link Reactor} will use to communicate
     * login message events to the application.
     * 
     * @param event A ReactorMsgEvent containing event information. The
     *            ReactorMsgEvent is valid only during callback
     *            
     * @return ReactorCallbackReturnCodes A callback return code that can
     *         trigger specific Reactor behavior based on the outcome of the
     *         callback function
     *         
     * @see RDMLoginMsgEvent
     * @see ReactorCallbackReturnCodes
     */
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event);
}
