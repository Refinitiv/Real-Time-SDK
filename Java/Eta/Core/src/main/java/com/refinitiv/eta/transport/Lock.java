/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/* The Lock interface is implemented internally to provide various thread safety constructs. */
interface Lock
{
    /* Expected to return immediately if a lock is obtained, blocks otherwise */
    public void lock();

    /* Returns true immediately if a lock is obtained, returns false otherwise */
    public boolean trylock();

    /* Releases the lock if held by the caller */
    public void unlock();
}
