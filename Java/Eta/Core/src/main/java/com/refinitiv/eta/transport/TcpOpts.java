/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Options used for configuring TCP specific transport options
 * (ConnectionTypes.SOCKET, ConnectionTypes.ENCRYPTED, ConnectionTypes.HTTP).
 * 
 * @see ConnectOptions
 */
public interface TcpOpts
{
    /**
     * Only used with connectionType of SOCKET. If true, disables Nagle's Algorithm.
     * 
     * @param tcpNoDelay the tcpNoDelay to set
     */
    public void tcpNoDelay(boolean tcpNoDelay);

    /**
     * Only used with connectionType of SOCKET. If true, disables Nagle's Algorithm.
     * 
     * @return the tcpNoDelay
     */
    public boolean tcpNoDelay();
}