package com.thomsonreuters.upa.transport;

class Pool 
{
    final UpaQueue _queue = new UpaQueue();
    Object _poolOwner;
    boolean _isSharedPoolBuffer;
    boolean _isProtocolBuffer;

    Pool(Object o)
    {
        _poolOwner = o;
    }

    UpaNode poll()
    {
        UpaNode node = _queue.poll();
        if (node != null)
            node._inPool = false;
        return node;
    }

    void add(UpaNode node)
    {
        node._inPool = true;
        _queue.add(node);
    }

    int size()
    {
        return _queue.size();
    }

    /* Add a number of UpaNodes from queue to this pool.
     * 
     * queue is the queue containing the nodes.
     * count is the number of nodes to add from queue to this pool.
     * 
     * Returns the number of nodes added to this pool.
     */
    int add(Pool pool, int numToAdd)
    {
        // adds the nodes from queue to this pool
        int numAdded = 0;
        UpaNode tempNode = null;
        while ((numAdded < numToAdd) && ((tempNode = pool._queue.poll()) != null))
        {
            tempNode._inPool = false;
            tempNode.pool(this);
            tempNode.returnToPool();
            numAdded++;
        }
        return numAdded;
    }

    /* Try to get numOfNodes number of nodes from this pool and add it to the pool
     * 
     * Returns number of added nodes
     */
    int poll(Pool pool, int numOfNodes)
    {
        assert (pool != this);
        int numAdded = 0;
        UpaNode tempNode = poll();
        while (tempNode != null)
        {
            tempNode.pool(pool);
            tempNode.returnToPool();
            numAdded++;
            if (numAdded == numOfNodes)
                break;
            tempNode = poll();
        }
        return numAdded;
    }

    void clear()
    {
        // This method is used during uninitialize.
        UpaNode node = null;
        while ((node = poll()) != null)
        {
            node.next(null);
        }
        _isSharedPoolBuffer = false;
        _isProtocolBuffer = false;
    }

}
