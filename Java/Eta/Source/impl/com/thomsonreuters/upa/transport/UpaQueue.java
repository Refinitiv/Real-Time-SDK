package com.thomsonreuters.upa.transport;

class UpaQueue 
{
    UpaNode _head = null;
    UpaNode _tail = null;
    int _size;
    private static final boolean DEBUG = false; // for turning on debug

    void add(UpaNode node)
    {
        if (DEBUG)
            verifyQueue();

        if (node == null)
            return;

        assert (node.next() == null);

        if (_tail == null)
        {
            assert (_head == null) : "UpaQueue.add(): unexpectedly found _tail null but head was not null!";
            assert (_size == 0);
            // Queue is empty, simply add node.
            _head = node;
        }
        else
        {
            // Add node to next of the current tail
            _tail._next = node;
        }

        // update tail to be the node passed in
        _tail = node;
        _tail._next = null;
        ++_size;

        if (DEBUG)
            verifyQueue();
    }

    UpaNode poll()
    {
        if (DEBUG)
            verifyQueue();

        if (_head == null)
        {
            return null;
        }

        UpaNode node = _head;
        --_size;

        if (_head.next() == null)
        {
            assert (_size == 0);
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

        node.next(null);

        if (DEBUG)
            verifyQueue();

        return node;
    }

    boolean remove(UpaNode node)
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
            node.next(null);
            return true;
        }

        UpaNode previousNode = _head;
        UpaNode currentNode = _head.next();
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
                node.next(null);
                return true;
            }

            // move to the next node
            previousNode = currentNode;
            currentNode = currentNode.next();
        }
        return false;
    }

    int size()
    {
        return _size;
    }

    void clear()
    {
        while (_size != 0)
        {
            poll();
        }
    }

    boolean contains(UpaNode node)
    {
        boolean ret = false;

        for (UpaNode qNode = _head; qNode != null; qNode = qNode.next())
        {
            if (qNode == node)
            {
                ret = true;
                break;
            }
        }

        return ret;
    }

    void verifyQueue()
    {
        int i = 0;
        assert (_size >= 0);
        for (UpaNode node = _head; node != null; node = node.next())
        {
            ++i;
            if (node.next() == null)
                assert (node == _tail);
        }

        assert (i == _size);
    }

}
