/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Flags for the InProgress state.
 * 
 * @see InProgInfo
 */
public class InProgFlags
{
    // InProgFlags class cannot be instantiated
    private InProgFlags()
    {
        throw new AssertionError();
    }

    /**
     * Indicates that the channel initialization is still in progress and
     * subsequent calls to {@link Channel#init(InProgInfo, Error)} are required
     * to complete it. No channel change has occurred as a result to this call.
     */
    public static final int NONE = 0;

    /**
     * Indicates that a channel change has occurred as a result of this call.
     * The previous channel has been stored in
     * {@link InProgInfo#oldSelectableChannel()} so it can be unregistered with the
     * I/O notification mechanism. The new channel has been stored in
     * {@link InProgInfo#newSelectableChannel()} so it can be registered with the I/O
     * notification mechanism. The channel initialization is still in progress
     * and subsequent calls to {@link Channel#init(InProgInfo, Error)} are
     * required to complete it.
     */
    public static final int SCKT_CHNL_CHANGE = 1;
}
