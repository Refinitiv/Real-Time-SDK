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
