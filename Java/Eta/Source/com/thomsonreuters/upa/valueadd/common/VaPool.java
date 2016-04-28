package com.thomsonreuters.upa.valueadd.common;

/**
 * Value Add Pool class.
 */
public class VaPool
{
    private VaQueue _queue;

    /**
     * Creates a pool. This pool is not thread safe.
     */
    public VaPool()
    {
        _queue = new VaQueue();
    }

    /**
     * Creates a pool. useConcurrent can be used to make this pool thread safe.
     * 
     * @param useConcurrent if true, the pool is backed by a
     *            {@link VaConcurrentQueue}.
     */
    public VaPool(boolean useConcurrent)
    {
        if (useConcurrent)
            _queue = new VaConcurrentQueue();
        else
            _queue = new VaQueue();
    }

    /**
     * Adds a node to the pool.
     * 
     * @param node the node to add
     */
    public void add(VaNode node)
    {
        if (node.inPool())
            return; // already in pool.

        if (node.pool() != this)
            node.pool(this);

        node.inPool(true);
        _queue.add(node);
    }

    /**
     * Updates the specified node to point to this pool. This would be called
     * when the node is created and assigns a pool to the node.
     * 
     * @param node
     */
    public void updatePool(VaNode node)
    {
        node.pool(this);
    }

    /**
     * Removes and returns the first item pooled.
     * 
     * @return the first item pooled
     */
    public VaNode poll()
    {
        VaNode node = _queue.poll();

        if (node != null)
            node.inPool(false);
        return node;
    }

    /**
     * Returns the size of the pool.
     * 
     * @return the size of the pool
     */
    public int size()
    {
        return _queue.size();
    }
}
