package com.thomsonreuters.upa.valueadd.common;

/** A FIFO queue. */
public class VaQueue
{
    VaNode _head = null;
    VaNode _tail = null;
    volatile int _size = 0;

    /**
     * Adds to the tail of the queue.
     * 
     * @param node the node to add
     */
    public void add(VaNode node)
    {
        if (node == null)
            return;
        
        if (_tail == null)
        {
            assert (_head == null) : "VaQueue.enqueue(): unexpectedly found _tail null but head was not null!";
            // Queue is empty, simply add node.
            _head = node;
        }
        else
        {
            // Add node to next of the current tail.
            _tail.next(node);
        }
        
        // update tail to be the node passed in.
        _tail = node;
        _tail.next(null);
        _size++;
    }
    
    /**
     * Removes and returns the head of the queue.
     * 
     * @return the head of the queue
     */
    public VaNode poll()
    {
        if (_head == null)
        {
            return (VaNode)null;
        }
        
        VaNode node = _head;
        _size--;
        
        if(_head.next() == null)
        {
            // queue is empty
            _head = null;
            _tail = null;
            _size = 0;
        }
        else
        {
            // make the next node the new head
            _head = _head.next();
        }
        
        return node;
    }
    
    /**
     * Returns but does not remove the head of the queue.
     * 
     * @return the head of the queue
     */
    public VaNode peek()
    {
        return _head;
    }
    
    /**
     * Removes a node from the queue.
     * 
     * @param node the node to remove
     * 
     * @return true if the node was in the queue, or false if the node wasn't
     */
    public boolean remove(VaNode node)
    {
        if (node == null || _head == null)
            return false;
        
        if (_head == node)
        {
            if (_head.next() != null)
            {
                _head = _head.next();
            }
            else
            {
                _head = null;
                _tail = null;
            }
            _size--;
            return true;
        }
        
        VaNode previousNode = _head;
        VaNode currentNode = _head.next();
        while (currentNode != null)
        {
            if (currentNode == node)
            {
                if (currentNode.next() != null)
                {
                    previousNode.next(currentNode.next());
                }
                else
                {
                    // at the tail.
                    previousNode.next(null);
                    _tail = previousNode;
                }
                _size--;
                return true;
            }
            
            // move to the next node
            previousNode = currentNode;
            currentNode = currentNode.next();
        }
        return false;
    }
    
    /**
     * Returns the size of the queue.
     * 
     * @return the size of the queue
     */
    public int size()
    {
        return _size;
    }
}
