/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.test.network.replay;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Simulates data read from the network
 */
class NetworkReplayImpl implements NetworkReplay, Runnable
{
    final private Queue<byte[]> _queue = new ConcurrentLinkedQueue<byte[]>();
    private byte[] _remainingData = null;
    private int    _remainingPos = 0;
    private String _nextReadFailureErrorMsg = null;
    
    private boolean _acceptConnections = false;
    private boolean _isEndOfStream = false;
    
    // The channel on which we'll accept connections
    private ServerSocketChannel _serverChannel;
    
    // The selector we'll be monitoring
    private Selector _selector;
    
    /**
     * Constructs a NetworkReplayImpl 
     */
    NetworkReplayImpl()
    {
    }
       
    @Override
    public byte[] read()
    {
        byte[] data;

        if (_remainingData == null)
        {
            data = _queue.poll();
        }
        else
        {
            data = _remainingData.clone();
            clearRemaining();
        }

        return data;
    }
    
    @Override
    public int read(ByteBuffer dst) throws IOException
    {
        int bytesRead = 0;
        int pos = 0;
        byte[] data;

        if (_nextReadFailureErrorMsg != null)
        {
            IOException error = new IOException(_nextReadFailureErrorMsg);
            _nextReadFailureErrorMsg = null;
            throw (error);
        }
        else if (_isEndOfStream)
        {
            return -1;
        }

        if (_remainingData == null)
        {
            data = _queue.poll(); // returns null if no data is in the queue
        }
        else
        {
            // a previous call to read had more data than could be fit into the destination buffer
            data = _remainingData;
            pos = _remainingPos;
        }

        if (data != null)
        {
            if (dst.remaining() >= data.length - pos)
            {
                // we have ample room in the destination buffer

                if (pos == 0)
                {
                    bytesRead = data.length;
                    dst.put(data);
                }
                else
                {
                    // copy just the remaining data
                    bytesRead = (data.length - pos);
                    dst.put(Arrays.copyOfRange(data, pos, (pos + bytesRead)));
                }

                clearRemaining(); // always called
            }
            else
            {
                // fill up as much of the buffer as we can, and save the remainder
                bytesRead = dst.remaining();
                dst.put(data, pos, bytesRead);
                _remainingData = data;
                _remainingPos = (pos + bytesRead);
            }
        }

        return bytesRead;
    }
    
    private void clearRemaining()
    {
        _remainingData = null;
        _remainingPos = 0;
    }

    @Override
    public void enqueue(byte[] data)
    {
        _queue.offer(data);
    }
    
    @Override
    public int parseFile(String name) throws IOException
    {
        int nonCommentBytesRead = 0;

        try
        {
            List<byte[]> multiLineRecord = new ArrayList<byte[]>();

            FileInputStream fstream = new FileInputStream(name);
            DataInputStream in = new DataInputStream(fstream);
            BufferedReader br = new BufferedReader(new InputStreamReader(in));

            String line;

            // Read File Line By Line
            while ((line = br.readLine()) != null)
            {
                //System.out.println(line);

                byte[] parsed = NetworkReplayParser.parse(line);
                if (parsed.length > 0)
                {
                    multiLineRecord.add(parsed);
                    nonCommentBytesRead += parsed.length;
                }
                else
                {
                    if (multiLineRecord.size() > 0)
                    {
                        enqueue(multiLineRecord);
                        multiLineRecord = new ArrayList<byte[]>();
                    }
                }
            }

            // enqueue any remaining data
            if (multiLineRecord.size() > 0)
            {
                enqueue(multiLineRecord);
            }

            in.close();
        }
        catch (Exception e)
        {
            throw new IOException("Error parsing replay file: " + e.getMessage(), e);
        }

        return nonCommentBytesRead;
    }
    
    /**
     * Combines a multi-part record into a single record, and enqueues it
     * 
     * @param multiPartRecord A multi-part record
     */
    private void enqueue(List<byte[]> multiPartRecord)
    {
        if (multiPartRecord != null)
        {
            int length = 0;

            // calculate the total length of the record
            for (byte[] partialRecord : multiPartRecord)
            {
                if (partialRecord != null)
                {
                    length += partialRecord.length;
                }
            }

            // combine the parts into a single record
            byte[] completeRecord = new byte[length];

            int pos = 0;
            for (byte[] partialRecord : multiPartRecord)
            {
                if (partialRecord != null)
                {
                    System.arraycopy(partialRecord, 0, completeRecord, pos, partialRecord.length);
                    pos += partialRecord.length;
                }
            }

            _queue.offer(completeRecord); // enqueue the complete record
        }
    }

    @Override
    public void startListener(int portNumber) throws IOException
    {
        if (_selector == null)
        {
            _selector = initSelector(portNumber);

            _acceptConnections = true;

            new Thread(this).start(); // invokes run
        }
    }    
    
    @Override
    public void stopListener()
    {
        _acceptConnections = false;

        if (_selector != null)
        {
            try
            {
                _selector.close();
            }
            catch (IOException e)
            {
                // ignored
            }
        }

        if (((_serverChannel != null) && _serverChannel.isOpen()))
        {
            try
            {
                _serverChannel.close();
            }
            catch (IOException e)
            {
                // ignored
            }
        }
    }

    /**
     * IMPORTANT: Do NOT invoke directly or via {@code Thread.start()}; invoke the {@link #startListener(int)} method instead.
     */
    @Override
    public void run()
    {
        try
        {
            if (_selector != null)
            {
                while (_acceptConnections)
                {
                    // wait for an event on one of the registered channels
                    _selector.select();

                    // Iterate over the set of keys for which events are available
                    Iterator<SelectionKey> selectedKeys = _selector.selectedKeys().iterator();
                    while (selectedKeys.hasNext())
                    {
                        SelectionKey key = selectedKeys.next();
                        selectedKeys.remove();

                        if (!key.isValid())
                        {
                            continue;
                        }

                        if (key.isAcceptable())
                        {
                            try
                            {
                                accept(key);
                            }
                            catch (IOException e)
                            {
                                System.out.println("Network Replay - Error accepting connection: " + e.getLocalizedMessage());
                            }
                        }
                    }
                }
            }
        }
        catch (Exception e)
        {
            // ignored
        }
    }
    
    /**
     * Returns a new selector, that accepts connections on the specified port
     * 
     * @param portNumber The port to listen on
     * 
     * @return a new selector, that accepts connections on the specified port
     * 
     * @throws IOException If the selector could not be initialized
     */
    private Selector initSelector(int portNumber) throws IOException
    {
        Selector socketSelector = SelectorProvider.provider().openSelector();

        // Create a new non-blocking server socket channel
        _serverChannel = ServerSocketChannel.open();
        _serverChannel.configureBlocking(false);

        // Bind the server socket to the specified address and port
        InetSocketAddress isa = new InetSocketAddress(portNumber);
        _serverChannel.socket().bind(isa);

        // Register the server socket channel, indicating an interest in accepting new connections
        _serverChannel.register(socketSelector, SelectionKey.OP_ACCEPT);

        return socketSelector;
    }
    
    /**
     * Accepts a pending connection
     * 
     * @param key The selection key from a server socket channel
     * 
     * @throws IOException
     */
    private void accept(SelectionKey key) throws IOException
    {
        // For an accept to be pending, the channel must be a server socket channel
        ServerSocketChannel serverSocketChannel = (ServerSocketChannel)key.channel();

        // Accept the connection and make it non-blocking
        java.nio.channels.SocketChannel socketChannel = serverSocketChannel.accept();
        socketChannel.configureBlocking(false);

        // We will NOT register the newjava.nio.channels.SocketChannel with our Selector,
        // because we don't really want to be notified when there's data waiting to be read
    }

    @Override
    public void forceNextReadToFail(String error)
    {
        if (error == null)
        {
            throw new NullPointerException("an error message must be specified");
        }

        _nextReadFailureErrorMsg = error;
    }

    @Override
    public void forceEndOfStream()
    {
        _isEndOfStream = true;
    }

    @Override
    public int recordsInQueue()
    {
        return _queue.size();
    }
}
