/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Information for the In Progress Connection State of {@link Channel#init(InProgInfo, Error)}.<BR>
 * If a backward compatibility reconnection occurs, the socket channel may change.
 * This is how that information is relayed.
 * 
 * @see InProgFlags
 * @see Channel
 */
public interface InProgInfo
{
    /**
     * Combination of bit values to indicate special behaviors and presence of
     * optional {@link InProgInfo} content.
     * 
     * @return the flags
     * 
     * @see InProgFlags
     */
    public int flags();

    /**
     * Old java.nio.channels.SocketChannel of this ETA channel - used in Read Channel Change events.
     * 
     * @return the oldScktChannel
     * 
     * @deprecated use {@link #oldSelectableChannel()} instead
     */
    @Deprecated
    public java.nio.channels.SocketChannel oldScktChannel();

    /**
     * java.nio.channels.SocketChannel of new ETA channel.
     * 
     * @return the newScktChannel
     * 
     * @deprecated use {@link #newSelectableChannel()} instead
     */
    @Deprecated
    public java.nio.channels.SelectableChannel newScktChannel();

    /**
     * Old java.nio.channels.SelectableChannel of this ETA channel - used in Read Channel Change events.
     * 
     * @return the oldSelectableChannel
     */
    public java.nio.channels.SelectableChannel oldSelectableChannel();

    /**
     * java.nio.channels.SelectableChannel of new ETA channel.
     * 
     * @return the newSelectableChannel
     */
    public java.nio.channels.SelectableChannel newSelectableChannel();

    /**
     * Clears Information for the In Progress Connection State.
     */
    public void clear();
}