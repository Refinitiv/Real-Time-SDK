/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * ETA Initialize Arguments used in the {@link Transport#initialize(InitArgs, Error)} call.
 * 
 * @see Transport
 */
public interface InitArgs
{
    /**
     * If locking is true, the global locking will be used by {@link Transport}.
     * 
     * @param locking the locking to set
     */
    public void globalLocking(boolean locking);

    /**
     * If true, the global lock is used.
     * 
     * @return the locking
     */
    public boolean globalLocking();

    /**
     * Sets maximum number of objects in pools that will be used by {@link Transport}.
     * 
     * @param socketProtocolPoolLimit the number to set
     */
    public void socketProtocolPoolLimit(int socketProtocolPoolLimit);

    /**
     * Gets maximum number of objects in pools that will be used by {@link Transport}.
     * 
     * @return the number
     */
    public int socketProtocolPoolLimit();

    /**
     * Clears ETA Initialize Arguments.
     */
    public void clear();
}
