/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.json.util;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.Supplier;

public class UtilQueueConcurrent<T> extends UtilQueue<T> {
    private Lock lock = new ReentrantLock();

    public UtilQueueConcurrent(int initialSize, Supplier<T> supplier) {
        super(initialSize, supplier);
    }

    @Override
    public T get() {
        lock.lock();
        try {
            return super.get();
        } finally {
            lock.unlock();
        }
    }

    @Override
    public void add(T element) {
        lock.lock();
        try {
            super.add(element);
        } finally {
            lock.unlock();
        }
    }
}
