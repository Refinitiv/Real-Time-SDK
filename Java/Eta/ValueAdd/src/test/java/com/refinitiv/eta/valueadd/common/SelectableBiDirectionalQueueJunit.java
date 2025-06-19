/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.SelectorProvider;
import java.util.Iterator;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadLocalRandom;

import org.junit.Test;

public class SelectableBiDirectionalQueueJunit
{
    public static int _id = 0;

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

    public class Worker implements Runnable
    {
        VaPool _pool = null;
        SelectableBiDirectionalQueue _queue = null;
        volatile boolean _running = true;
        boolean _shutdown = false;
        long _writeCount = 0;
        long _readCount = 0;
        int _id = 0;
        int _selectTime = 1;

        public Worker(VaPool pool, SelectableBiDirectionalQueue queue, int selectTime)
        {
            _pool = pool;
            _queue = queue;
            _selectTime = selectTime;
        }

        public void shutdown()
        {
            System.out.println("Worker: Shutdown requested");
            _running = false;
        }

        public boolean isShutdown()
        {
            return _shutdown;
        }

        public long writeCount()
        {
            return _writeCount;
        }

        public long readCount()
        {
            return _readCount;
        }

        public void readQueue()
        {
            while (_queue.readQueueSize() > 0)
            {
                if (_queue.isShutDown())
                {
                    System.out.println("Worker.readQueue: queue is shutdown???");
                    return;
                }
                VaNode node = _queue.read();
                if (node != null)
                {
                    _readCount++;
                    node.returnToPool();
                }
                else
                {
                    System.out.println("Worker.readQueue: readQueueSize=" + _queue.readQueueSize()
                            + " but read() returned null");
                }
            }
        }

        public void writeQueue()
        {
            if (_queue.isShutDown())
            {
                System.out.println("Worker.writeQueue: queue is shutdown???");
                return;
            }

            VaNode node = _pool.poll();
            if (node == null)
            {
                node = new TestObject(++_id);
                node.pool(_pool);
            }

            if (_queue.write(node))
                _writeCount++;
            else
                System.out.println("Worker.writeQueue: queue.write failed?");
        }

        @Override
        public void run()
        {
            // take objects from the pool and enqueue.
            int selectCount;

            try
            {
                Selector selector = SelectorProvider.provider().openSelector();
                _queue.readChannel().register(selector, SelectionKey.OP_READ);

                // continue to read until it's time to shutdown and the read
                // queue size is zero.
                while (_running || _queue.readQueueSize() > 0)
                {
                    selectCount = selector.select(_selectTime);
                    if (selectCount > 0)
                    {
                        Iterator<SelectionKey> iter = selector.selectedKeys().iterator();
                        while (iter.hasNext())
                        {
                            SelectionKey key = iter.next();
                            iter.remove();
                            if (key.isValid() == false)
                                continue;
                            if (key.isReadable())
                                readQueue();
                        }
                    }

                    // don't write if we are shutting down
                    if (_running)
                    {
                        if (ThreadLocalRandom.current().nextInt(0, 10) == 0)
                            writeQueue();
                    }
                }
            }
            catch (IOException e)
            {
                System.out.println("Worker.run: IOException=" + e.getLocalizedMessage());
            }

            System.out.println("Worker.run: shutdown complete, write count = " + _writeCount
                    + " read count = " + _readCount);
            _shutdown = true;
        }
    }

    public int readQueue(SelectableBiDirectionalQueue queue)
    {
        int readCount = 0;
        while (queue.readQueueSize() > 0)
        {
            if (queue.isShutDown())
            {
                System.out.println("main.readQueue: queue is shutdown???");
                return 0;
            }
            VaNode node = queue.read();
            if (node != null)
            {
                readCount++;
                node.returnToPool();
            }
            else
            {
                System.out.println("main.readQueue: readQueueSize=" + queue.readQueueSize()
                        + " but read() returned null");
            }
        }
        return readCount;
    }

    public int writeQueue(SelectableBiDirectionalQueue queue, VaPool pool)
    {
        if (queue.isShutDown())
        {
            System.out.println("main.writeQueue: queue is shutdown???");
            return 0;
        }

        VaNode node = pool.poll();
        if (node == null)
        {
            node = new TestObject(++_id);
            node.pool(pool);
        }

        if (queue.write(node))
            return 1;
        else
        {
            System.out.println("main.writeQueue(): queue.write failed?");
            return 0;
        }
    }

    @Test
    public void queueTest()
    {
        SelectableBiDirectionalQueue queue = new SelectableBiDirectionalQueue();
        TestObject objL = new TestObject(1);
        assertNotNull(queue);
        assertNotNull(objL);

        // write an object on this side of the queue and verify that the
        // remote side can read it.
        assertTrue(queue.write(objL));
        assertEquals(0, queue.remote().writeQueueSize());
        assertEquals(0, queue.readQueueSize());
        assertEquals(1, queue.remote().readQueueSize());
        assertEquals(1, queue.writeQueueSize());
        assertTrue(queue._writeNotifier.isSet());
        assertFalse(queue._readNotifier.isSet());
        assertFalse(queue.remote()._writeNotifier.isSet());
        assertTrue(queue.remote()._readNotifier.isSet());
        TestObject objR = (TestObject)queue.remote().read();
        assertEquals(objL, objR);
        assertEquals(0, queue.remote().readQueueSize());
        assertEquals(0, queue.writeQueueSize());
        assertFalse(queue._writeNotifier.isSet());
        assertFalse(queue.remote()._readNotifier.isSet());

        // write an object on the remote side of the queue and verify that this
        // side can read it.
        assertTrue(queue.remote().write(objR));
        assertEquals(1, queue.remote().writeQueueSize());
        assertEquals(1, queue.readQueueSize());
        assertEquals(0, queue.remote().readQueueSize());
        assertEquals(0, queue.writeQueueSize());
        assertFalse(queue._writeNotifier.isSet());
        assertTrue(queue._readNotifier.isSet());
        assertTrue(queue.remote()._writeNotifier.isSet());
        assertFalse(queue.remote()._readNotifier.isSet());
        objL = (TestObject)queue.read();
        assertEquals(objL, objR);
        assertFalse(queue._readNotifier.isSet());
        assertFalse(queue.remote()._writeNotifier.isSet());
        assertEquals(0, queue.remote().writeQueueSize());
        assertEquals(0, queue.readQueueSize());

    }

    @Test
    public void queueTestWithSelector()
    {
        int SELECT_TIME = 100;
        SelectableBiDirectionalQueue queue = new SelectableBiDirectionalQueue();
        TestObject obj = new TestObject(1);
        assertNotNull(queue);
        assertNotNull(obj);

        Selector localSelector = null;
        Selector remoteSelector = null;
        try
        {
            localSelector = SelectorProvider.provider().openSelector();
            queue.readChannel().register(localSelector, SelectionKey.OP_READ);
            remoteSelector = SelectorProvider.provider().openSelector();
            queue.remote().readChannel().register(remoteSelector, SelectionKey.OP_READ);
        }
        catch (IOException e)
        {
            System.out.println("testRandom: execption=" + e.getLocalizedMessage());
            assertTrue(e.getLocalizedMessage(), false);
        }

        /* test writing on the local side and reading on the remote side */

        // write on the local side of the queue
        assertEquals(true, queue.write(obj));
        assertEquals(true, queue._writeNotifier.isSet());
        assertEquals(true, queue.remote()._readNotifier.isSet());
        assertEquals(false, queue._readNotifier.isSet());
        assertEquals(false, queue.remote()._writeNotifier.isSet());

        // select on the remote side of the queue, to read on the remote side of
        // queue.
        try
        {
            remoteSelector.select(SELECT_TIME);
        }
        catch (IOException e)
        {
            assertTrue("remoteSelector.select failed, exception=" + e.getLocalizedMessage(), false);
        }
        Iterator<SelectionKey> iter = remoteSelector.selectedKeys().iterator();
        assertTrue(iter.hasNext());
        SelectionKey key = iter.next();
        iter.remove();
        assertTrue(key.isValid());
        assertTrue(key.isReadable());
        assertEquals(1, queue.remote().readQueueSize());
        TestObject rcvedObj = (TestObject)queue.remote().read();
        assertNotNull(rcvedObj);
        assertEquals(0, queue.remote().readQueueSize());
        assertEquals(false, queue.remote()._readNotifier.isSet());
        assertEquals(false, queue._writeNotifier.isSet());

        /* test writing on the remote side and reading on the local side */

        // write on the remote side of the queue
        assertEquals(true, queue.remote().write(obj));
        assertEquals(true, queue.remote()._writeNotifier.isSet());
        assertEquals(true, queue._readNotifier.isSet());
        assertEquals(false, queue.remote()._readNotifier.isSet());
        assertEquals(false, queue._writeNotifier.isSet());

        // select on the remote side of the queue, to read on the remote side of
        // queue.
        try
        {
            localSelector.select(SELECT_TIME);
        }
        catch (IOException e)
        {
            assertTrue("localSelector.select failed, exception=" + e.getLocalizedMessage(), false);
        }
        iter = localSelector.selectedKeys().iterator();
        assertTrue(iter.hasNext());
        key = iter.next();
        iter.remove();
        assertTrue(key.isValid());
        assertTrue(key.isReadable());
        assertEquals(1, queue.readQueueSize());
        rcvedObj = (TestObject)queue.read();
        assertNotNull(rcvedObj);
        assertEquals(0, queue.readQueueSize());
        assertEquals(false, queue._readNotifier.isSet());
        assertEquals(false, queue.remote()._writeNotifier.isSet());
    }

    /**
     * Test the SelectableBiDirectionalQueue by having two threads randomly
     * writing data to each other and selecting on the readChannel. The main
     * thread starts the worker, then later terminates the worker and waits for
     * the worker to finish.
     */
	@Test
    public void randomTest()
    {
        int SELECT_TIME = 1; // ms
        int runTime = 10; // seconds.
        int loopCount = 1;

        // create a pool of objects.
        VaPool pool = new VaPool(true);

        // create queue to be used by threads.
        SelectableBiDirectionalQueue queue = new SelectableBiDirectionalQueue();
        Selector selector = null;
        try
        {
            selector = SelectorProvider.provider().openSelector();
            queue.readChannel().register(selector, SelectionKey.OP_READ);
        }
        catch (IOException e)
        {
            System.out.println("testRandom: execption=" + e.getLocalizedMessage());
            assertTrue(e.getLocalizedMessage(), false);
        }

        long errorCount = 0;
        long writeCount = 0;
        long readCount = 0;
        long totalWriteCount = 0;
        long totalReadCount = 0;
        long totalRemoteWriteCount = 0;
        long totalRemoteReadCount = 0;
        int count = 0;
        int selectCount;

        ExecutorService _executorService = Executors.newSingleThreadExecutor();

        while (++count <= loopCount)
        {
            writeCount = 0;
            readCount = 0;

            // start threads
            Worker worker = new Worker(pool, queue.remote(), SELECT_TIME);
            _executorService.execute(worker);

            boolean signaledWorkerShutdown = false;
            boolean writeToWorker = true;
            long endTime = System.currentTimeMillis() + (runTime * 1000);

            try
            {
                while (true)
                {
                    // break out of this while loop if the runTime expired,
                    // the read queue size is zero and the worker is shutdown.
                    if (System.currentTimeMillis() > endTime)
                    {
                        writeToWorker = false;
                        if (worker.isShutdown() && queue.readQueueSize() == 0)
                            break;
                    }

                    selectCount = selector.select(SELECT_TIME);
                    if (selectCount > 0)
                    {
                        Iterator<SelectionKey> iter = selector.selectedKeys().iterator();
                        while (iter.hasNext())
                        {
                            SelectionKey key = iter.next();
                            iter.remove();
                            if (key.isValid() == false)
                                continue;
                            if (key.isReadable())
                                readCount += readQueue(queue);
                        }
                    }
                    // randomly write if runtime has not expired
                    if (writeToWorker)
                    {
                        int cnt = ThreadLocalRandom.current().nextInt(0, 10);
                        while (--cnt >= 0)
                            writeCount += writeQueue(queue, pool);
                    }
                    else if (signaledWorkerShutdown == false)
                    {
                        worker.shutdown();
                        signaledWorkerShutdown = true;
                    }
                }
            }
            catch (IOException e)
            {
                System.out.println("testRandom: exception=" + e.getLocalizedMessage());
                errorCount++;
            }

            if (writeCount != worker.readCount() || readCount != worker.writeCount())
                errorCount++;

            totalWriteCount += writeCount;
            totalReadCount += readCount;
            totalRemoteWriteCount += worker.writeCount();
            totalRemoteReadCount += worker.readCount();
            System.out.format("main thread: count %d of %d, poolSize=%d errorCount=%d\n", count,
                              loopCount, pool.size(), errorCount);
            System.out
                    .format("\t\tlocal: writeCount=%d readCount=%d\n\t\tremote: writeCount=%d readCount=%d\n",
                            writeCount, readCount, worker.writeCount(), worker.readCount());
            System.out
                    .format("\t\ttotal local: writeCount=%d readCount=%d\n\t\ttotal remote: writeCount=%d readCount=%d\n",
                            totalWriteCount, totalReadCount, totalRemoteWriteCount,
                            totalRemoteReadCount);

            System.out.format("\t\tqueue size: local_read=%d remote_write=%d\n",
                              queue.readQueueSize(), queue.remote().writeQueueSize());
            System.out.format("\t\tqueue size: local_write=%d remote_read=%d\n",
                              queue.writeQueueSize(), queue.remote().readQueueSize());
        }

        queue.shutdown();

        System.out.println("main thread: test complete, loopCount=" + loopCount + " poolSize="
                + pool.size() + " errorCount=" + errorCount);
        assertEquals(0, errorCount);
    }

}
