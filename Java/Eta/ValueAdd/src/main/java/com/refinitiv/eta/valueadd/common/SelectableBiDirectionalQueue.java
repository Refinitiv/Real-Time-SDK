/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.Pipe;
import java.nio.channels.SelectableChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.Predicate;

/**
 * A selectable bidirectional queue used to communicate events between the Reactor
 * and Worker threads. To be useful this queue needs a local and remote end. Initially
 * this class operates on the local end of the queues. To operate on the remote end
 * of the queues, call {@link #remote()}.
 */
public class SelectableBiDirectionalQueue
{
    int DEFAULT_BUFFER_SIZE = 2;
    SelectableBiDirectionalQueue _remote;
    
    // Queues are used to pass events.
    VaQueue _writeQueue;
    VaQueue _readQueue;

    // Pipes are used to trigger selectors to notify when to read.
    Pipe _writePipe;
    Pipe _readPipe;
    ByteBuffer _writeBuffer;
    ByteBuffer _readBuffer;

    // Locks are used for concurrency.
    Lock _writeLock;
    Lock _readLock;

    // inner class to share a boolean between threads
    class NotifiedState
    {
        boolean _notified = false;

        public void set()
        {
            _notified = true;
        }

        public boolean isSet()
        {
            return _notified;
        }

        public void clear()
        {
            _notified = false;
        }
    }

    NotifiedState _writeNotifier;
    NotifiedState _readNotifier;
    volatile NotifiedState _shutdown;

    /**
     * Normal constructor.
     */
    public SelectableBiDirectionalQueue()
    {
        try
        {
            /*
             * create a Pipe which will be used to trigger the selector as
             * OP_READ.
             */
            _writePipe = SelectorProvider.provider().openPipe();
            _readPipe = SelectorProvider.provider().openPipe();
            _writePipe.sink().configureBlocking(false);
            _writePipe.source().configureBlocking(false);
            _readPipe.sink().configureBlocking(false);
            _readPipe.source().configureBlocking(false);
        }
        catch (IOException e)
        {
            System.out
                    .println("SelectableBiDirectionalQueue.constructor: failed to create a Pipe, exception="
                            + e.getLocalizedMessage());
            return;
        }

        _writeBuffer = ByteBuffer.allocateDirect(DEFAULT_BUFFER_SIZE);
        _readBuffer = ByteBuffer.allocateDirect(DEFAULT_BUFFER_SIZE);

        /* create queues used to pass VaNodes. */
        _writeQueue = new VaQueue();
        _readQueue = new VaQueue();

        /* create locks */
        _writeLock = new ReentrantLock();
        _readLock = new ReentrantLock();

        _writeNotifier = new NotifiedState();
        _readNotifier = new NotifiedState();
        _shutdown = new NotifiedState();
    }

    /**
     * Returns whether or not the queue is shutdown.
     * 
     * @return true if the queue is shutdown, or false if it isn't
     */
    public boolean isShutDown()
    {
        return _shutdown.isSet();
    }

    /**
     * This constructor is used to create a new SelectableBiDirectionalQueue
     * that owns the remote end of a specified SelectableBiDirectionalQueue. The
     * specified SelectableBiDriectionalQueue owns the local end. One thread
     * will use local end queue and another thread will use the remote end
     * queue.
     * 
     * @param sq A SelectableBiDirectionalQueue (a.k.a the local end queue).
     */
    SelectableBiDirectionalQueue(SelectableBiDirectionalQueue sq)
    {
        // this is called from remote, copy and flip from sq.
        _writePipe = sq._readPipe;
        _readPipe = sq._writePipe;
        _writeBuffer = sq._readBuffer;
        _readBuffer = sq._writeBuffer;
        _writeQueue = sq._readQueue;
        _readQueue = sq._writeQueue;
        _writeLock = sq._readLock;
        _readLock = sq._writeLock;
        _writeNotifier = sq._readNotifier;
        _readNotifier = sq._writeNotifier;
        _shutdown = sq._shutdown;
    }

    /**
     * Returns the SelectableChannel of the queue that can be registered
     * with a selector.
     * 
     * @return SelectableChannel of the queue
     */
    public SelectableChannel readChannel()
    {
        return _readPipe.source();
    }

    /**
     * Returns the size of the read queue.
     * 
     * @return the size of the read queue
     */
    public int readQueueSize()
    {
        return _readQueue.size();
    }

    /**
     * Returns the size of the write queue.
     * 
     * @return the size of the write queue
     */
    public int writeQueueSize()
    {
        return _writeQueue.size();
    }

    /**
     * Reads a ReactorEvent sent from the remote end of the queue.
     * 
     * Once the selector fires for the key (registered on {@link #readChannel()}
     * ), read should be called until it returns null or
     * {@link #readQueueSize()} is zero. This will reset the internal notifier
     * and allow the selector to fire again later.
     * 
     * @return ReactorEvent
     */
    public VaNode read()
    {
        if (_shutdown.isSet())
            return null;

        VaNode node = null;

        _readLock.lock();
        
        if (_shutdown.isSet())
        {
        	_readLock.unlock();
            return null;
        }
        
        try
        {
            node = _readQueue.poll();
            if (node == null)
                System.out
                        .println("SelectableBiDirectionalQueue.read: node was unexpectedly null?");

            if (_readQueue.size() == 0)
            {
                // _readQueue is empty, clear the readNotifier.
                _readBuffer.clear();

                /*
                 * Note: Windows OS actually sets the notifier (for the
                 * SelectableChannel) before the data may be available, so
                 * sometimes read will return zero. ignore it and clear the
                 * _readNotifier so the far end will be forced to send anothe
                 * byte on the Pipe and cause the SelectableChannel to be
                 * notified again.
                 */
                int cnt = _readPipe.source().read(_readBuffer);
                if (cnt == -1)
                {
                    System.out.println("SelectableBiDirectionalQueue.read: return returned -1");
                    shutdown();
                }
                _readNotifier.clear();
            }
        }
        catch (IOException e)
        {
            System.out.println("SelectableBiDirectionalQueue.read: failed, execption="
                    + e.getLocalizedMessage());
            e.printStackTrace();
            shutdown();
        }
        finally
        {
            _readLock.unlock();
        }

        return node;
    }

    /**
     * Writes a ReactorEvent to the remote end of the queue.
     * 
     * @param node ReactorEvent to write to remote end of queue
     * 
     * @return true if write succeeded, false otherwise
     */
    public boolean write(VaNode node)
    {
        if (_shutdown.isSet())
            return false;

        _writeLock.lock();
        
        if (_shutdown.isSet())
        {
        	_writeLock.unlock();
            return false;
        }
        
        try
        {
            _writeQueue.add(node);
            if (_writeNotifier.isSet() == false)
            {
                // the writeNotifier was not set, need to send a byte and set
                // the notifier.
                _writeNotifier.set();
                _writeBuffer.clear();
                _writeBuffer.put((byte)0);
                _writeBuffer.flip();
                int cnt = _writePipe.sink().write(_writeBuffer);
                if (cnt != 1)
                {
                    System.out
                            .println("SelectableBiDirectionalQueue.write: expected to write 1 byte but wrote "
                                    + cnt);
                    shutdown();
                }
            }
        }
        catch (IOException e)
        {
            System.out.println("SelectableBiDirectionalQueue.write: failed, execption="
                    + e.getLocalizedMessage() + " stacktrace=" + e.getStackTrace().toString());
            shutdown();
        }
        finally
        {
            _writeLock.unlock();
        }
        return true;
    }

    /**
     * Helper method which will create a new SelectableBiDirectionalQueue that
     * uses the remote end of this queue. Repeated calls will return the same
     * object, since there is only one remote end of this queue. The user can
     * obtain the same result by calling the
     * {@link SelectableBiDirectionalQueue#SelectableBiDirectionalQueue(SelectableBiDirectionalQueue)}
     * constructor.
     * 
     * @return a SelectableBiDirectionalQueue that uses the remote end of this
     *         queue
     */
    public SelectableBiDirectionalQueue remote()
    {
        if (_remote == null)
            _remote = new SelectableBiDirectionalQueue(this);
        return _remote;
    }

    /** Shuts down the selectable bidirectional queue. */
    public void shutdown()
    {
        if (_shutdown.isSet() == false)
        {
            _shutdown.set();
            try
            {
                _writePipe.sink().close();
                _writePipe.source().close();
                _readPipe.sink().close();
                _readPipe.source().close();
                _remote._writePipe.sink().close();
                _remote._writePipe.source().close();
                _remote._readPipe.sink().close();
                _remote._readPipe.source().close();
                
                _writeBuffer = null;
                _writeNotifier = null;
                _writePipe = null;
                _writeQueue = null;
                _readBuffer = null;
                _readNotifier = null;
                _readPipe = null;
                _readQueue = null;
                _remote._writeBuffer = null;
                _remote._writeNotifier = null;
                _remote._writePipe = null;
                _remote._writeQueue = null;
                _remote._readBuffer = null;
                _remote._readNotifier = null;
                _remote._readPipe = null;
                _remote._readQueue = null;
            }
            catch (IOException e)
            {
                System.out
                        .println("SelectableBiDirectionalQueue.shutdown: closing of pipes failed, execption="
                                + e.getLocalizedMessage()
                                + " stacktrace="
                                + e.getStackTrace().toString());
            }
        }
    }

    /**
     * Counts the number of elements in the queue that satisfy the given condition
     * @param filter the condition that the counted elements must satisfy
     * @return the number of suitable elements in the queue
     */
    public int countNumberOfReadQueueElements(Predicate<VaNode> filter) {
        _readLock.lock();
        try {
            int result = 0;
            VaNode current = _readQueue._head;
            while (current != null) {
                if (filter.test(current)) {
                    result++;
                }
                current = current.next();
            }
            return result;
        } finally {
            _readLock.unlock();
        }
    }
}
