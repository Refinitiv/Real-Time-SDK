///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.transport;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Iterator;

/* TestClient for single-threaded or multi-threaded unit tests */
public class TestClient implements Runnable
{
    final static boolean DEBUG = true;
    final static int TIMEOUTMS = 10000; // 10 seconds.
    final static int SLEEPTIMEMS = 25; // ms
    
    SocketChannel _socketChannel = null;
    Selector _selector = null;
    ByteBuffer _buffer = ByteBuffer.allocateDirect(6144);
    int _port;
    int _timeoutMs = TIMEOUTMS; // 5 second timeout for actions.
    volatile boolean _running = false;
    volatile boolean _setup = false;
    volatile boolean _connected = false;
    volatile boolean _readable = false;

    public enum State
    {
        SETUP, READABLE
    }

    public TestClient(int port)
    {
        _port = port;

        for (int idx = 0; idx < _buffer.capacity(); idx++)
        {
            _buffer.put(idx, (byte)0);
        }
    }

    /* waits for test client to transition to the specified state */
    public void wait(State state)
    {
        long currentTimeMs = System.currentTimeMillis();
        long endTimeMs = currentTimeMs + _timeoutMs;

        try
        {
            while (System.currentTimeMillis() < endTimeMs)
            {
                if (check(state))
                    return;
                Thread.sleep(100);
            }
        }
        catch (InterruptedException e)
        {
            fail("exception occured during wait(State), state=" + state.toString()
                    + " exception=" + e.toString());
        }
        fail("timeout while wait(State), state=" + state.toString());
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
     * Connects a non-blocking client socket to the port specified.
     * State.SETUP is set after this socket is connected. However,
     * this method will fail if the connect does not finish, so there
     * is no need to check for State.SETUP.
     * 
     * @param port local port number to connect to.
     */
    public void connect()
    {
        try
        {
            _selector = Selector.open();
            _socketChannel = SocketChannel.open();
            _socketChannel.configureBlocking(false);

            InetSocketAddress isa = new InetSocketAddress(_port);

            _socketChannel.connect(isa);

            int sleepTime = 25; // ms
            int finishConnectAttempt = 10000 / sleepTime;
            ; // 10 seconds.
            while ((_setup = _socketChannel.finishConnect()) == false
                    && finishConnectAttempt-- > 0)
            {
                Thread.sleep(sleepTime);
            }

            assertEquals("timeout waiting to finish the connection", true, _setup);

            _socketChannel.register(_selector, SelectionKey.OP_READ, _socketChannel);
        }
        catch (IOException | InterruptedException e)
        {
            fail("client socket setup failed, exception=" + e.toString());
        }
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
        _setup = false;
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

    /* shutdown test client */
    public void shutDown()
    {
        _running = false;
        closeSocket();
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
            try
            {
                Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
        }
        catch (IOException e)
        {
            fail("exception occurred while writing to socket, exception=" + e.toString());
        }

        return bytesWritten;
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

    @Override
    public void run()
    {
        _running = true;
        if (DEBUG)
        {
            System.out.println("DEBUG: TestClient thread running");
        }
        connect();
        if (DEBUG)
        {
            System.out
                    .println("DEBUG: TestClient thread setup and ready to accept connections");
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
        catch (IOException e)
        {
            fail("Exception occurred during run() loop of TestClient, exception="
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
        }
        
        if (DEBUG)
        {
            System.out.println("DEBUG: TestClient thread finished");
        }
    }
}
