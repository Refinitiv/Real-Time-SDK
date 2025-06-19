/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * ReactorChannelEventTypes used with ReactorChannelEvents.
 */
public class ReactorChannelEventTypes
{
	/** Channel event initialization value. This should not be used by the application or returned to the application. */
    public static final int INIT = 0;
    /** Channel has successfully initialized and can be dispatched. If the application presented any messages for setting
	 * up the session in their ReactorRole object in {@link Reactor#connect(ReactorConnectOptions, ReactorRole, ReactorErrorInfo)}
	 * or {@link Reactor#accept(com.refinitiv.eta.transport.Server, ReactorAcceptOptions, ReactorRole, ReactorErrorInfo)},
	 * they will now be sent. */
    public static final int CHANNEL_UP = 1;
    /** ReactorChannel has failed (e.g. the connection was lost or a ping timeout expired) and can no longer send or receive data.
     * There is no connection recovery for this event. The application should call {@link ReactorChannel#close(ReactorErrorInfo)}
     * to clean up the channel. */
    public static final int CHANNEL_DOWN = 2;
    /** ReactorChannel has failed and connection recovery has started. This only occurs on client connections since there is
     * no connection recovery for server connections. The application should should release any resources associated with the
     * channel, such as TransportBuffers, and unregister its selectableChannel, if valid, from any select notifiers. */
    public static final int CHANNEL_DOWN_RECONNECTING = 3;
    /** Channel has sent and received all messages expected for setting up the session. Normal use (such as item requests)
     * can now be done. */
	public static final int CHANNEL_READY = 4;
    /** An event has occurred that did not result in channel failure, but may require attention by the application. */
    public static final int WARNING = 5;
    /** The SelectableChannel representing this channel has changed. The new and old SelectableChannel can be found on
     * the ReactorChannel. */
    public static final int FD_CHANGE = 6;
    /** Channel was opened by the application and can be used (occurs when watchlist is enabled and only appears
     * in the channelOpenCallback). */
    public static final int CHANNEL_OPENED = 7;

    /**
     * Returns a String representation of the specified ReactorChannelEventTypes
     * type.
     *
     * @param type the type
     * @return String representation of the specified ReactorChannelEventTypes
     *         type
     */
    public static String toString(int type)
    {
        switch (type)
        {
            case 0:
                return "ReactorChannelEventTypes.INIT";
            case 1:
                return "ReactorChannelEventTypes.CHANNEL_UP";
            case 2:
                return "ReactorChannelEventTypes.CHANNEL_DOWN";
            case 3:
                return "ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING";
            case 4:
                return "ReactorChannelEventTypes.CHANNEL_READY";
            case 5:
                return "ReactorChannelEventTypes.WARNING";
            case 6:
                return "ReactorChannelEventTypes.FD_CHANGE";
            case 7:
                return "ReactorChannelEventTypes.CHANNEL_OPENED";
            default:
                return "ReactorChannelEventTypes " + type + " - undefined.";
        }
    }
}
