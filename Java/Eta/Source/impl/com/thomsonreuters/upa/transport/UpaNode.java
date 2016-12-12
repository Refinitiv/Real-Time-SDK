package com.thomsonreuters.upa.transport;

class UpaNode
{
    UpaNode _next = null;
    Pool _pool = null;
    boolean _inPool = false;

    UpaNode next()
    {
        return _next;
    }

    void next(UpaNode next)
    {
        _next = next;
    }

    Pool pool()
    {
        return _pool;
    }

    void pool(Pool pool)
    {
        _pool = pool;
    }

    void returnToPool()
    {
        if (!_inPool)
            _pool.add(this);
    }

}
