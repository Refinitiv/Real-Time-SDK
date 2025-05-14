/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/* An implementation of RsslLock that uses the Java concurrent ReentrantLock */
class ReentrantLock implements Lock
{
    java.util.concurrent.locks.ReentrantLock _lock;

    ReentrantLock()
    {
        _lock = new java.util.concurrent.locks.ReentrantLock();
    }

    @Override
    public void lock()
    {
        _lock.lock();
    }

    @Override
    public boolean trylock()
    {
        return _lock.tryLock();
    }

    @Override
    public void unlock()
    {
        _lock.unlock();
    }
}
