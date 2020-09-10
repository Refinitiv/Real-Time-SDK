///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.valueadd.common;

import static org.junit.Assert.*;

import java.util.concurrent.ThreadLocalRandom;

import org.junit.Test;

public class VaConcurrentQueueJunit
{
    public class TestObject extends VaNode
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

    public class Enqueuer implements Runnable
    {
        VaPool _pool = null;
        VaQueue _queue = null;
        volatile boolean _running = true;
        long _queued = 0;

        Enqueuer(VaPool pool, VaQueue queue)
        {
            _pool = pool;
            _queue = queue;
        }

        public void shutdown()
        {
            System.out.println("Enqueuer: Shutdown requested");
            _running = false;
        }

        @Override
        public void run()
        {
            // take objects from the pool and enqueue.
            try
            {
                while (_running)
                {

                    VaNode node = _pool.poll();
                    if (node != null)
                    {
                        _queued++;
                        _queue.add(node);
                    }
                    else
                    {
                        System.out.print(".");
                    }

                    Thread.sleep(0, ThreadLocalRandom.current().nextInt(0, 10));
                }
            }
            catch (InterruptedException e)
            {
                System.out.println("Engueuer: exception=" + e.getLocalizedMessage());
            }
            System.out.println("Enqueuer: shutdown complete, enqueue count = " + _queued);
        }
    }

    public class Dequeuer implements Runnable
    {
        VaQueue _queue = null;
        volatile boolean _running = true;
        long _dequeued = 0;

        Dequeuer(VaQueue queue)
        {
            _queue = queue;
        }

        public void shutdown()
        {
            System.out.println("Dequeuer: Shutdown requested");
            _running = false;
        }

        @Override
        public void run()
        {
            // take objects from the pool and enqueue.
            try
            {
                while (_running || _queue.size() > 0)
                {

                    VaNode node = _queue.poll();

                    Thread.sleep(0, ThreadLocalRandom.current().nextInt(0, 5));

                    if (node != null)
                    {
                        _dequeued++;
                        node.returnToPool();
                    }
                }
            }
            catch (InterruptedException e)
            {
                System.out.println("Degueuer: exception=" + e.getLocalizedMessage());
            }
            System.out.println("Dequeuer: shutdown complete, dequeue count = " + _dequeued);
        }
    }

    /**
     * Test the VaConcurrentQueue by having one thread randomly enqueuing and
     * another thread randomly dequeuing.
     */
    @Test
    public void testRandom()
    {
        int numOfTestObjects = 8;
        int runTime = 5; // seconds.
        int loopCount = 2;

        // create a pool of objects.
        VaPool pool = new VaPool(true);

        for (int i = 0; i < numOfTestObjects; i++)
        {
            TestObject obj = new TestObject(i);
            pool.add(obj);
        }

        // create queue to be used by threads.
        VaQueue queue = new VaConcurrentQueue();

        int errorCount = 0;
        int count = 0;
        while (count++ < loopCount)
        {

            // start threads
            Enqueuer enqueuer = new Enqueuer(pool, queue);
            Dequeuer dequeuer = new Dequeuer(queue);
            Thread tEnqueuer = new Thread(enqueuer);
            Thread tDequeuer = new Thread(dequeuer);
            tEnqueuer.start();
            tDequeuer.start();

            // handle run time
            try
            {
                Thread.sleep(runTime * 1000);
            }
            catch (InterruptedException e)
            {
                System.out.println("testRandom: Thread sleep was interrupted.");
            }
            finally
            {
                // terminate threads.
                enqueuer.shutdown();
            }

            // give the threads time to shutdown and print it's final message.
            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
            }

            dequeuer.shutdown();

            // give the threads time to shutdown and print it's final message.
            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
            }

            if (pool.size() != numOfTestObjects)
            {
                System.out.println("ERROR main thread: complete, error!!!  starting pool size = "
                        + numOfTestObjects + " ending pool size = " + pool.size() + " count "
                        + count + " of " + loopCount + " errorCount=" + (++errorCount));
            }
            else
                System.out.println("main thread: complete, pool size = " + pool.size() + " count "
                        + count + " of " + loopCount + " errorCount=" + errorCount);
        }
        System.out.println("main thread: test complete, errorCount=" + errorCount);
        assertEquals(0, errorCount);
    }

    @Test
    public void removeTest()
    {
        VaQueue queue = new VaConcurrentQueue();

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

        // test queue.remove() on a queue with two nodes (node being removed is last).
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
