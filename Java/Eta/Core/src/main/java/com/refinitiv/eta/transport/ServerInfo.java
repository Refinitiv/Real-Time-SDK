/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * ETA Server Info returned by {@link Server#info(ServerInfo, Error)} call.
 * 
 * @see Server
 */
public interface ServerInfo
{
    /**
     * The number of currently used shared pool buffers across all users
     * connected to the {@link Server}.
     * 
     * @return the currentBufferUsage
     */
    public int currentBufferUsage();

    /**
     * The maximum achieved number of used shared pool buffers across all users
     * connected to the {@link Server}. This value can be reset through the use
     * of {@link Server#ioctl(int, Object, Error)}, also described in this section.
     * 
     * @return the peakBufferUsage
     */
    public int peakBufferUsage();

    /**
     * Clears ETA Server Info.
     */
    public void clear();
}