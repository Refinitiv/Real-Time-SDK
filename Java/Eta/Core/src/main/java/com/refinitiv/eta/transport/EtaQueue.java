package com.refinitiv.eta.transport;

class EtaQueue 
{
    EtaNode _head = null;
    EtaNode _tail = null;
    int _size;
    private static final boolean DEBUG = false; // for turning on debug

    void add(EtaNode node)
    {
        if (DEBUG)
            verifyQueue();

        if (node == null)
            return;

        assert (node.next() == null);

        if (_tail == null)
        {
            assert (_head == null) : "EtaQueue.add(): unexpectedly found _tail null but head was not null!";
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

    EtaNode poll()
    {
        if (DEBUG)
            verifyQueue();

        if (_head == null)
        {
            return null;
        }

        EtaNode node = _head;
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

    boolean remove(EtaNode node)
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

        EtaNode previousNode = _head;
        EtaNode currentNode = _head.next();
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

    boolean contains(EtaNode node)
    {
        boolean ret = false;

        for (EtaNode qNode = _head; qNode != null; qNode = qNode.next())
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
        for (EtaNode node = _head; node != null; node = node.next())
        {
            ++i;
            if (node.next() == null)
                assert (node == _tail);
        }

        assert (i == _size);
    }

}
