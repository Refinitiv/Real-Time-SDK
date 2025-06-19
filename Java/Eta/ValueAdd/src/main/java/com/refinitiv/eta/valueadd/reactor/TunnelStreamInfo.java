/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

/**
 * Tunnel stream Info available through {@link TunnelStream#info(TunnelStreamInfo, ReactorErrorInfo)} method call.
 *
 * @see TunnelStream
 */
public interface TunnelStreamInfo {

    /**
     * Get buffers used count.
     *
     * @return total buffers used - both big and ordinary
     */
    int buffersUsed();

    /**
     * Get buffers used count.
     *
     * @return ordinary buffers used
     */
    int ordinaryBuffersUsed();

    /**
     * Get buffers used count.
     *
     * @return big buffers used
     */
    int bigBuffersUsed();

    /**
     * Clears buffer Info.
     */
    void clear();
}
