/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

/** An iteratable version of the Value Add FIFO queue. */
public class VaIteratableQueue extends VaQueue
{
    VaNode _currentNode = null;
    VaNode _lastNode = null;
    
    /**
     * Adds to the tail of the queue.
     * 
     * @param node the node to add
     */
    public void add(VaNode node)
    {
    	super.add(node);
    	if (_currentNode == null)
    	{
    		_currentNode = _head;
    	}
    }
    
    /** Rewinds the iterator to the head of the queue. */
    public void rewind()
    {
        _currentNode = _head;
        _lastNode = null;
    }
    
    /**
     * Returns whether or not the iterator has more nodes.
     * 
     * @return true if the iterator has more nodes, or false if the iterator doesn't
     */
    public boolean hasNext()
    {
        if (_currentNode != null)
            return true;
        else
            return false;
    }

    /**
     * Returns the next node in the queue.
     * 
     * @return the next node
     */
    public VaNode next()
    {
        _lastNode = _currentNode;
        if(_lastNode != null)
            _currentNode = _lastNode.next();
        return _lastNode;
    }

    /** Removes the last node from the queue. */
    public void remove()
    {
        super.remove(_lastNode);
    }

}
