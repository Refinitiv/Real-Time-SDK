/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.Iterator;

/* TestServer for single-threaded or multi-threaded unit tests */
public class TestServer implements Runnable
{
    final static boolean DEBUG = true;
    final static int TIMEOUTMS = 10000; // 10 seconds.
    final static int SLEEPTIMEMS = 25; // ms
    
    ServerSocketChannel _ssc = null;
    SocketChannel _socketChannel = null;
    Selector _selector = null;
    ByteBuffer _buffer = ByteBuffer.allocateDirect(6144);
    int _port;
    int _timeoutMs = TIMEOUTMS; // 5 second timeout for actions.
    volatile boolean _running = false;
    volatile boolean _setup = false;
    volatile boolean _accepted = false;
    volatile boolean _readable = false;
    volatile boolean _acceptable = false;

    public enum State
    {
        SETUP, ACCEPTED, READABLE
    }
    
    public TestServer(int port)
    {
        _port = port;

        // clear out the bytebuffer */
        for (int idx = 0; idx < _buffer.capacity(); idx++)
        {
            _buffer.put(idx, (byte)0);
        }
    }

    /* waits for test server to transition to the specified state */
    public void wait(State state)
    {
        long currentTimeMs = System.currentTimeMillis();
        long endTimeMs = currentTimeMs + _timeoutMs;

        try
        {
            while (System.currentTimeMillis() < endTimeMs)
            {
                Thread.sleep(SLEEPTIMEMS);
                if (check(state))
                    return;
            }
        }
        catch (InterruptedException e)
        {
            fail("exception occured during wait(State), state=" + state.toString()
                    + " exception=" + e.toString());
        }
        fail("timeout while wait(State), state=" + state.toString());
    }

    /* waits for test server to transition to the specified state */
    public void waitWithException(State state) throws Exception
    {
        long currentTimeMs = System.currentTimeMillis();
        long endTimeMs = currentTimeMs + _timeoutMs;

        try
        {
            while (System.currentTimeMillis() < endTimeMs)
            {
                Thread.sleep(SLEEPTIMEMS);
                if (check(state))
                    return;
            }
        }
        catch (InterruptedException e) {}
        throw new Exception();
    }

    /* checks if the specified state has occurred */
    public boolean check(State state)
    {

        switch (state)
        {
            case SETUP:
                if (_setup)
                    return true;
                break;
            case ACCEPTED:
                if (_accepted)
                    return true;
                break;
            case READABLE:
                if (_readable)
                    return true;
                break;
            default:
                break;
        }
        return false;
    }

    /* returns the test server's internal buffer */
    public ByteBuffer buffer()
    {
        return _buffer;
    }

    /*
     * Sets up a non-blocking server socket, listening on the port
     * specified.
     * 
     * @param port local port number to listen on.
     */
    public void setupServerSocket()
    {
        try
        {
            _selector = SelectorProvider.provider().openSelector();
            _ssc = ServerSocketChannel.open();
            _ssc.configureBlocking(false);
            
            InetSocketAddress isa = new InetSocketAddress(_port);
            _ssc.socket().bind(isa);

            _ssc.register(_selector, SelectionKey.OP_ACCEPT, _ssc);
            _setup = true;
        }
        catch (IOException e)
        {
            fail("server socket setup failed, exception=" + e.toString());
        }
    }

    /*
     * Attempt to accept a socket on the server side. This will block until
     * a socket is accepted or a timeout occurs.
     * 
     * @param timeoutMs
     * @return Channel
     */
    public void acceptSocket()
    {
        assertNotNull(_ssc);
        assertEquals("a 2nd connection is ready to be accepted, we expect only one active connection.",
                     null, _socketChannel);

        try
        {
            _socketChannel = _ssc.accept();
            if (_socketChannel != null)
            {
                _socketChannel.configureBlocking(false);
                _socketChannel.register(_selector, SelectionKey.OP_READ, _socketChannel);
                _accepted = true;
                if (DEBUG)
                {
                    System.out.println("DEBUG: accepted socket and registered for OP_READ");
                }
                return;
            }
        }
        catch (IOException e)
        {
            fail("accept failed, exception=" + e.toString());
        }

        fail("accpet failed, the connection could not be accepted, however the selectionKey was acceptable.");
    }

    /* attempt to read an entire message */
    public int readMessageFromSocket()
    {
        int bytesRead = 0;
        long currentTimeMs = System.currentTimeMillis();
        long endTimeMs = currentTimeMs + _timeoutMs;

        _buffer.clear();

        try
        {
            while (System.currentTimeMillis() < endTimeMs)
            {
                _readable = false;
                bytesRead = _socketChannel.read(_buffer);
                if (bytesRead > 0)
                {
                    // note that we could cache the msgLen, but normally
                    // we should be reading an entire ConnectAck/ConnectNak
                    // here.
                    if (_buffer.position() > 2)
                    {
                        int messageLength = _buffer.getShort(0);
                        if (_buffer.position() >= messageLength)
                        {
                            // we have at least one complete message
                            return _buffer.position();
                        }
                    }
                }
                Thread.sleep(25);
            }
        }
        catch (IOException | InterruptedException e)
        {
            fail("socket channel read() failed. exception=" + e.toString());
        }

        fail("socket channel read() timed out after " + _timeoutMs + "ms");
        return 0;
    }

    /*
     * Close the socket.
     */
    public void closeSocket()
    {
        _accepted = false;
        _readable = false;
        
        try
        {
            if (_socketChannel != null)
            {
                _socketChannel.close();
            }
            else
            {
                return;
            }
            if (_selector != null && _selector.isOpen())
            {
                // unregister the selection key
                for (SelectionKey key : _selector.keys())
                    if (key.isValid() && key.attachment() == _socketChannel)
                        key.cancel();
            }
        }
        catch (IOException e)
        {
            fail("exception occurred while closing socket, exception=" + e.toString());
        }
        _socketChannel = null;
    }

    /*
     * Close the server socket.
     */
    private void closeServerSocket()
    {
        if (_ssc == null && _selector == null)
            return;

        try
        {
            // close server socket
            _ssc.close();

            // unregister the selection key
        	if (_selector.isOpen())
        	{
	            for (SelectionKey key : _selector.keys())
	                if (key.isValid() && key.attachment() == _ssc)
	                    key.cancel();
	            _selector.close();
        	}
        }
        catch (IOException e)
        {
            fail("exception occurred while closing server socket, exception=" + e.toString());
        }
        _ssc = null;
        _selector = null;
    }

    /* shutdown test server */
    public void shutDown()
    {
        _running = false;
        closeSocket();
        closeServerSocket();
    }

    /*
     * flips the buffer and then writes it on the socket.
     * 
     * @param buffer
     * @return number of bytes written
     */
    public int writeMessageToSocket(byte[] byteArray)
    {
        assertNotNull(byteArray);

        _buffer.clear();
        _buffer.put(byteArray);
        _buffer.flip();

        int bytesWritten = 0;
        try
        {
            bytesWritten = _socketChannel.write(_buffer);
        }
        catch (IOException e)
        {
            fail("exception occurred while writing to socket, exception=" + e.toString());
        }

        return bytesWritten;
    }

    @Override
    public void run()
    {
        _running = true;
        if (DEBUG)
        {
            System.out.println("DEBUG: TestServer thread running");
        }
        setupServerSocket();
        if (DEBUG)
        {
            System.out
                    .println("DEBUG: TestServer thread setup and ready to accept connections");
        }
        try
        {
            while (_running)
            {
                int keyNum = _selector.select(100);

                if (keyNum > 0)
                {
                    Iterator<SelectionKey> keyIterator = _selector.selectedKeys().iterator();
                    while (keyIterator.hasNext())
                    {
                        SelectionKey selectionKey = keyIterator.next();
                        keyIterator.remove();
                        if (selectionKey.isValid())
                        {
                            if (selectionKey.isAcceptable())
                            {
                                if (DEBUG)
                                {
                                    System.out.println("DEBUG: run(): key is acceptable");
                                }
                                acceptSocket();
                                continue;
                            }
                            if (selectionKey.isReadable())
                            {
                                // don't tight loop when ready to read.
                                _readable = true;
                                Thread.sleep(SLEEPTIMEMS);
                            }
                        }
                    }
                }
            }
        }
        catch (IOException e)
        {
            fail("Exception occurred during run() loop of TestServer, exception="
                    + e.toString());
            return;
        }
        catch (InterruptedException e)
        {
            // do nothing.
        }
        finally
        {
            closeSocket();
            closeServerSocket();
        }
        if (DEBUG)
        {
            System.out.println("***TestServer: run() finished.");
        }
    }
    
    /* waits for selector to become acceptable */
    public void waitForAcceptable()
    {
        try
        {
            while (!_acceptable)
            {
                int keyNum = _selector.select(100);

                if (keyNum > 0)
                {
                    Iterator<SelectionKey> keyIterator = _selector.selectedKeys().iterator();
                    while (keyIterator.hasNext())
                    {
                        SelectionKey selectionKey = keyIterator.next();
                        keyIterator.remove();
                        if (selectionKey.isAcceptable())
                        {
                            _acceptable = true;
                            Thread.sleep(SLEEPTIMEMS);
                        }
                    }
                }
            }
        }
        catch (IOException e)
        {
            fail("Exception occurred during waitForReadable() loop of TestClient, exception="
                    + e.toString());
            return;
        }
        catch (InterruptedException e)
        {
            // do nothing.
        }
    }

    /* waits for selector to become readable */
    public void waitForReadable()
    {
        try
        {
            while (!_readable)
            {
                int keyNum = _selector.select(100);

                if (keyNum > 0)
                {
                    Iterator<SelectionKey> keyIterator = _selector.selectedKeys().iterator();
                    while (keyIterator.hasNext())
                    {
                        SelectionKey selectionKey = keyIterator.next();
                        keyIterator.remove();
                        if (selectionKey.isReadable())
                        {
                            _readable = true;
                            Thread.sleep(SLEEPTIMEMS);
                        }
                    }
                }
            }
        }
        catch (IOException e)
        {
            fail("Exception occurred during waitForReadable() loop of TestClient, exception="
                    + e.toString());
            return;
        }
        catch (InterruptedException e)
        {
            // do nothing.
        }
    }
}
