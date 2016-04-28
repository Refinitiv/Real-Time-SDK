package com.thomsonreuters.upa.valueadd.common;

/**
 * Represents a Node in a Queue. Used with VaConcurrentQueue or VaQueue.
 */
public class VaNode
{
    private VaNode _next = null;
    private VaPool _pool = null;
    private boolean _inPool = false;
    
    /**
     * The next node.
     * 
     * @return the next node
     */
    public VaNode next()
    {
        return _next;
    }
    
    /**
     * The next node.
     *
     * @param next the next node to set
     */
    public void next(VaNode next)
    {
        _next = next;
    }
    
    /**
     * Returns this node to the pool.
     */
    public void returnToPool()
    {
        if (_pool != null)
        {
            _pool.add(this);
        }
    }
    
    void pool(VaPool pool)
    {
        _pool = pool;
    }
    
    VaPool pool()
    {
        return _pool;
    }
    
    boolean inPool()
    {
        return _inPool;
    }
    
    void inPool(boolean in)
    {
        _inPool = in;
    }
    
}
