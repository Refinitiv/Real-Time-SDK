/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Options used for configuring shared memory specific transport options
 * ({@link ConnectionTypes#UNIDIR_SHMEM}).
 * 
 * @see ConnectOptions
 */
public interface ShmemOpts
{
    /**
     * Maximum number of messages that the client can have waiting to read.
     * If the client "lags" the server by more than this amount, ETA will
     * disconnect the client.
     * 
     * @param maxReaderLag the maxReaderLag to set
     */
    public void maxReaderLag(long maxReaderLag);

    /**
     * Maximum number of messages that the client can have waiting to read.
     * If the client "lags" the server by more than this amount, ETA will
     * disconnect the client.
     * 
     * @return the maxReaderLag
     */
    public long maxReaderLag();
}
