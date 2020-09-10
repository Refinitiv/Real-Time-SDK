package com.rtsdk.eta.transport;

class EtaNode
{
    EtaNode _next = null;
    Pool _pool = null;
    boolean _inPool = false;

    EtaNode next()
    {
        return _next;
    }

    void next(EtaNode next)
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
