package com.thomsonreuters.upa.transport;

/* a dummy implementation of RsslLock */
class DummyLock implements Lock
{
    @Override
    public void lock()
    {
    }

    @Override
    public boolean trylock()
    {
        return true;
    }

    @Override
    public void unlock()
    {
    }
}
