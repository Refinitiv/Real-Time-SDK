/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/** A thread-safe version of the Value Add FIFO queue. */
public class VaConcurrentQueue extends VaQueue
{
    Lock _lock = new ReentrantLock();
    
    @Override
    public void add(VaNode node)
    {
        _lock.lock();
        try
        {
            super.add(node);
        }
        finally
        {
            _lock.unlock();
        }
    }
    
    @Override
    public VaNode poll()
    {
        VaNode node = null;
        _lock.lock();
        try
        {
            node = super.poll();
            return node;
        }
        finally
        {
            _lock.unlock();
        }
    }
    
    @Override
    public VaNode peek()
    {
        VaNode node = null;
        _lock.lock();
        try
        {
            node = super.peek();
            return node;
        }
        finally
        {
            _lock.unlock();
        }
    }
    
    @Override
    public boolean remove(VaNode node)
    {
        _lock.lock();
        try
        {
            return super.remove(node);
        }
        finally
        {
            _lock.unlock();
        }
    }
}
