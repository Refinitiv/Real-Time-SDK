package com.thomsonreuters.upa.transport;

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
