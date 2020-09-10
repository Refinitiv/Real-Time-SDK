package com.rtsdk.eta.transport;

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
