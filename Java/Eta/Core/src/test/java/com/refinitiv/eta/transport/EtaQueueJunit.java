///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import org.junit.Test;

public class EtaQueueJunit
{
    public class TestObject extends EtaNode
    {
        int _id = 0;

        TestObject(int id)
        {
            _id = id;
        }

        int id()
        {
            return _id;
        }
    }

    @Test
    public void dequeueFromEmptyQueueTest()
    {
        EtaQueue queue = new EtaQueue();
        assertEquals(null, queue.poll());
    }

    @Test
    public void enqueueDequeueTest()
    {
        EtaQueue queue = new EtaQueue();

        // create some objects and add the to the queue.
        TestObject obj1 = new TestObject(1);
        TestObject obj2 = new TestObject(2);
        TestObject obj3 = new TestObject(3);
        TestObject obj4 = new TestObject(4);

        queue.add(obj1);
        TestObject obj = (TestObject)queue.poll();
        assertEquals(obj1, obj);
        assertEquals(null, queue.poll());

        queue.add(obj1);
        queue.add(obj2);
        obj = (TestObject)queue.poll();
        assertEquals(obj1, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj2, obj);
        assertEquals(null, queue.poll());

        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        obj = (TestObject)queue.poll();
        assertEquals(obj1, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj2, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj3, obj);
        assertEquals(null, queue.poll());

        // add objs to the queue
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        obj = (TestObject)queue.poll();
        assertEquals(obj1, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj2, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj3, obj);
        // add obj2 and obj1 back to the queue
        queue.add(obj2);
        queue.add(obj1);
        obj = (TestObject)queue.poll();
        assertEquals(obj4, obj);
        // add obj4 and obj3 back to the queue
        queue.add(obj4);
        queue.add(obj3);
        // expect the order to be obj2, obj1, obj4, obj3
        obj = (TestObject)queue.poll();
        assertEquals(obj2, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj1, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj4, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj3, obj);
        assertEquals(null, queue.poll());
        assertEquals(null, queue.poll());

        // create some objects and add the to the queue.
        TestObject obj11 = new TestObject(1);
        TestObject obj12 = new TestObject(2);
        TestObject obj13 = new TestObject(3);
        TestObject obj14 = new TestObject(4);

        queue.add(obj11);
        obj = (TestObject)queue.poll();
        assertEquals(obj11, obj);
        assertEquals(null, queue.poll());

        queue.add(obj11);
        queue.add(obj12);
        obj = (TestObject)queue.poll();
        assertEquals(obj11, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj12, obj);
        assertEquals(null, queue.poll());

        queue.add(obj11);
        queue.add(obj12);
        queue.add(obj13);
        obj = (TestObject)queue.poll();
        assertEquals(obj11, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj12, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj13, obj);
        assertEquals(null, queue.poll());

        // add objs to the queue
        queue.add(obj11);
        queue.add(obj12);
        queue.add(obj13);
        queue.add(obj14);
        obj = (TestObject)queue.poll();
        assertEquals(obj11, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj12, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj13, obj);
        // add obj12 and obj11 back to the queue
        queue.add(obj12);
        queue.add(obj11);
        obj = (TestObject)queue.poll();
        assertEquals(obj14, obj);
        // add obj14 and obj13 back to the queue
        queue.add(obj14);
        queue.add(obj13);
        // expect the order to be obj12, obj11, obj14, obj13
        obj = (TestObject)queue.poll();
        assertEquals(obj12, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj11, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj14, obj);
        obj = (TestObject)queue.poll();
        assertEquals(obj13, obj);
        assertEquals(null, queue.poll());
        assertEquals(null, queue.poll());
    }
    
    @Test
    public void removeTest()
    {
        EtaQueue queue = new EtaQueue();

        // create some objects and add the to the queue.
        TestObject obj1 = new TestObject(1);
        TestObject obj2 = new TestObject(2);
        TestObject obj3 = new TestObject(3);
        TestObject obj4 = new TestObject(4);
        TestObject obj5 = new TestObject(5);

        // test remove on an empty queue.
        assertEquals(0, queue.size());
        assertEquals(false, queue.remove(obj1));

        // test queue.remove() on a queue with one node.
        queue.add(obj1);
        assertEquals(1, queue.size());
        assertEquals(false, queue.remove(obj2));
        assertEquals(1, queue.size());
        assertEquals(true, queue.remove(obj1));
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with two nodes (node being removed is first).
        queue.add(obj1);
        queue.add(obj2);
        assertEquals(2, queue.size());
        assertEquals(false, queue.remove(obj3));
        assertEquals(2, queue.size());
        assertEquals(true, queue.remove(obj1));
        assertEquals(1, queue.size());
        assertEquals(obj2, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with two nodes (node being removed is
        // last).
        queue.add(obj1);
        queue.add(obj2);
        assertEquals(2, queue.size());
        assertEquals(false, queue.remove(obj4));
        assertEquals(2, queue.size());
        assertEquals(true, queue.remove(obj2));
        assertEquals(1, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with three nodes (node being removed is first).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        assertEquals(3, queue.size());
        assertEquals(false, queue.remove(obj4));
        assertEquals(3, queue.size());
        assertEquals(true, queue.remove(obj1));
        assertEquals(2, queue.size());
        assertEquals(obj2, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj3, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with three nodes (node being removed is second).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        assertEquals(3, queue.size());
        assertEquals(false, queue.remove(obj4));
        assertEquals(3, queue.size());
        assertEquals(true, queue.remove(obj2));
        assertEquals(2, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj3, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with three nodes (node being removed is last).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        assertEquals(3, queue.size());
        assertEquals(false, queue.remove(obj4));
        assertEquals(3, queue.size());
        assertEquals(true, queue.remove(obj3));
        assertEquals(2, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj2, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with four nodes (node being removed is first).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        assertEquals(4, queue.size());
        assertEquals(false, queue.remove(obj5));
        assertEquals(4, queue.size());
        assertEquals(true, queue.remove(obj1));
        assertEquals(3, queue.size());
        assertEquals(obj2, queue.poll());
        assertEquals(2, queue.size());
        assertEquals(obj3, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj4, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with four nodes (node being removed is second).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        assertEquals(4, queue.size());
        assertEquals(false, queue.remove(obj5));
        assertEquals(4, queue.size());
        assertEquals(true, queue.remove(obj2));
        assertEquals(3, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(2, queue.size());
        assertEquals(obj3, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj4, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with four nodes (node being removed is third).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        assertEquals(4, queue.size());
        assertEquals(false, queue.remove(obj5));
        assertEquals(4, queue.size());
        assertEquals(true, queue.remove(obj3));
        assertEquals(3, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(2, queue.size());
        assertEquals(obj2, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj4, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // test queue.remove() on a queue with four nodes (node being removed is last).
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        assertEquals(4, queue.size());
        assertEquals(false, queue.remove(obj5));
        assertEquals(4, queue.size());
        assertEquals(true, queue.remove(obj4));
        assertEquals(3, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(2, queue.size());
        assertEquals(obj2, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj3, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // remove two nodes and dequeue the other two.
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        assertEquals(4, queue.size());
        assertEquals(true, queue.remove(obj4));
        assertEquals(3, queue.size());
        assertEquals(true, queue.remove(obj2));
        assertEquals(2, queue.size());
        assertEquals(obj1, queue.poll());
        assertEquals(1, queue.size());
        assertEquals(obj3, queue.poll());
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());

        // remove all (4) nodes.
        queue.add(obj1);
        queue.add(obj2);
        queue.add(obj3);
        queue.add(obj4);
        assertEquals(4, queue.size());
        assertEquals(true, queue.remove(obj4));
        assertEquals(3, queue.size());
        assertEquals(true, queue.remove(obj2));
        assertEquals(2, queue.size());
        assertEquals(true, queue.remove(obj1));
        assertEquals(1, queue.size());
        assertEquals(true, queue.remove(obj3));
        assertEquals(0, queue.size());
        assertEquals(null, queue.poll());
    }    
}
