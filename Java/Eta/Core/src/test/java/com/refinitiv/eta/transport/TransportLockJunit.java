///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import org.junit.Test;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.transport.Ripc.CompressionTypes;

public class TransportLockJunit
{
    String DEFAULT_PORT_NUMBER = "15000";

    enum RunningState
    {
        INACTIVE, INITIALIZING, RUNNING, TERMINATED
    };

    /**
     * This server will accept connects and process messages until instructed to
     * terminate. It will keep track of the following statistics:
     * <ul>
     * <li>writeCount per priority.
     * <li>messageCount per priority (verify if this is different from
     * writeCount).
     * <li>bytesWritten per priority.
     * <li>uncompressedBytesWritten per priority.
     * </ul>
     */
    class EtajServer implements Runnable
    {
        boolean DEBUG = false;

        BindOptions _bindOptions;
        AcceptOptions _acceptOptions;
        Server _server;
        Selector _selector;
        String _errorMsg;
        boolean _globalLocking;

        // statistics
        long _messageCount = 0;
        long _bytesRead = 0;
        long _uncompressedBytesRead = 0;

        volatile boolean _running = true;
        RunningState _runningState = RunningState.INACTIVE;

        final Error _error = TransportFactory.createError();
        final ReadArgs _readArgs = TransportFactory.createReadArgs();
        final InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
        final static int TIMEOUTMS = 1000; // 100 milliseconds

        /**
         * A blocking EtajServer is not supported for junits.
         * 
         * @param bindOptions
         * @param acceptOptions
         * @param globalLocking
         */
        public EtajServer(BindOptions bindOptions, AcceptOptions acceptOptions,
                TestArgs args)
        {
            assert (bindOptions != null);
            assert (acceptOptions != null);

            _bindOptions = bindOptions;
            _acceptOptions = acceptOptions;
            _globalLocking = args.globalLocking;
        }

        /**
         * Check if the server is running.
         * 
         * @return true if running.
         */
        public RunningState state()
        {
            return _runningState;
        }

        /**
         * Instructs the server to terminate.
         */
        public void terminate()
        {
            if (DEBUG)
                System.out.println("EtajServer: terminate() entered.");

            _running = false;
        }

        /**
         * the number of messages received.
         * @return
         */
        public long messageCount()
        {
            return _messageCount;
        }

        /**
         * the number of bytes read.
         * 
         * @return
         */
        public long bytesRead()
        {
            return _bytesRead;
        }

        /**
         * the number of uncompressed bytes read.
         * 
         * @return
         */
        public long uncompressedBytesRead()
        {
            return _uncompressedBytesRead;
        }

        /**
         * Returns the error message that caused the server to abort.
         * 
         * @return String containing error message.
         */
        public String errorMsg()
        {
            return _errorMsg;
        }

        /*
         * Sets up the Server by calling bind, and if non-blocking registers for
         * OP_ACCEPT.
         */
        private boolean setupServer()
        {
            if (DEBUG)
                System.out.println("EtajServer: setupServer() entered");

            if (_bindOptions.channelsBlocking())
            {
                _errorMsg = "Blocking EtajServer is not supported in junits.";
                return false;
            }

            if (DEBUG)
                System.out.println("EtajServer: setupServer() binding");

            _server = Transport.bind(_bindOptions, _error);
            if (_server == null)
            {
                _errorMsg = "errorCode=" + _error.errorId() + " errorText=" + _error.text();
                return false;
            }

            if (DEBUG)
                System.out.println("EtajServer: setupServer() opening selector");

            try
            {
                _selector = Selector.open();
                _server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT);
            }
            catch (Exception e)
            {
                _errorMsg = e.toString();
                return false;
            }

            if (DEBUG)
                System.out.println("EtajServer: setupServer() completed successfully");

            return true;
        }

        private void closeSockets()
        {
            if (DEBUG)
                System.out.println("EtajServer: closeSockets() entered");

            if (_selector == null)
                return;

            if (_server != null)
            {
                _server.close(_error);
                _server = null;
            }

            for (SelectionKey key : _selector.keys())
            {
                if (key.attachment() != null)
                    ((Channel)key.attachment()).close(_error);
                key.cancel();
            }

            try
            {
                _selector.close();
            }
            catch (IOException e)
            {
            }

            _selector = null;
        }

        private boolean verifyChannelInfoCompressionType(Channel channel)
        {
            ChannelInfo channelInfo = TransportFactory.createChannelInfo();
            if (channel.info(channelInfo, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "verifyChannelInfoCompressionType() channel.info() failed. error="
                        + _error.text();
                return false;
            }

            if (channelInfo.compressionType() != _bindOptions.compressionType())
            {
                _errorMsg = "verifyChannelInfoCompressionType() channelInfo.compressionType("
                        + channelInfo.compressionType()
                        + ") did not match bindOptions.compressionType("
                        + _bindOptions.compressionType() + ")";
                return false;
            }

            return true;
        }

        private boolean initializeChannel(Channel channel)
        {
            if (DEBUG)
                System.out.println("EtajServer: initializeChannel() entered");

            _inProgInfo.clear();
            int ret = channel.init(_inProgInfo, _error);
            if (ret == TransportReturnCodes.SUCCESS)
            {
                if (DEBUG)
                    System.out.println("EtajServer: initializeChannel() SUCCESS - verifying compressionType is "
                            + _bindOptions.compressionType());

                // SUCCESS, channel is ready, first verify compression Type
                // is what the server specified in BindOptions.
                return verifyChannelInfoCompressionType(channel);
            }
            else if (ret == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
            {
                return true;
            }
            else
            {
                _errorMsg = "initializeChannel failed, TransportReturnCode=" + ret;
                return false;
            }
        }

        /**
         * Process the message and store the statistics.
         * 
         * @param msgBuf
         * @param compressedBytes Number of bytes from the pre-processed
         *            message. We don't know if the message was actually
         *            compressed.
         */
        private void processStatistics(TransportBuffer msgBuf, ReadArgs readArgs)
        {
            int msgLen = msgBuf.length();

            ++_messageCount;

            if (DEBUG)
                System.out.println("EtajServer: processStatistics() processed message byte="
                        + msgLen + " messageCount=" + _messageCount);
        }

        private boolean processRead(Channel channel)
        {
            if (DEBUG)
                System.out.println("EtajServer: processRead() entered");

            int runningExpiredCount = 0;
            do
            {
                _readArgs.clear();
                TransportBuffer msgBuf = channel.read(_readArgs, _error);
                _bytesRead += _readArgs.bytesRead();
                _uncompressedBytesRead += _readArgs.uncompressedBytesRead();

                if (msgBuf != null)
                {
                    processStatistics(msgBuf, _readArgs);
                }
                else
                {
                    if (_readArgs.readRetVal() == TransportReturnCodes.READ_PING)
                    {
                        // Note that we are not tracking client pings.
                        return true;
                    }
                    else if (_readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK)
                    {
                        return true;
                    }
                }
                if (!_running)
                {
                    try
                    {
                        Thread.sleep(1);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    if (++runningExpiredCount > 10000)
                    {
                        System.out.println("EtajServer: processRead() channel.read() is returning more to read, but runtime has expired.");
                        break;
                    }
                    else
                    {
                        System.out.println("EtajServer: processRead() runtime expired, _messageCount="
                                + _messageCount);
                    }

                }
            }
            while (_readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            if (_readArgs.readRetVal() != TransportReturnCodes.SUCCESS && _running)
            {
                // ignore this error if we are terminating, which would mean
                // that the client terminated as expected.
                _errorMsg = "processRead(): channel.read() returned " + _readArgs.readRetVal()
                        + ", " + _error.text();
                return false;
            }
            else
            {
                return true;
            }
        }

        private boolean processSelector()
        {
            if (DEBUG)
                System.out.println("EtajServer: processSelector() entered");

            Set<SelectionKey> keySet = null;

            try
            {
                if (_selector.select(TIMEOUTMS) > 0)
                {
                    keySet = _selector.selectedKeys();
                    if (keySet != null)
                    {
                        Iterator<SelectionKey> iter = keySet.iterator();
                        while (iter.hasNext())
                        {
                            SelectionKey key = iter.next();
                            iter.remove();

                            if (key.isAcceptable())
                            {
                                if (DEBUG)
                                    System.out.println("EtajServer: processSelector() accepting a connection");

                                Channel channel = _server.accept(_acceptOptions, _error);
                                if (channel != null)
                                {
                                    channel.selectableChannel().register(_selector, SelectionKey.OP_READ,
                                                                   channel);
                                }
                                else
                                {
                                    _errorMsg = "server.accept() failed to return a valid Channel, error="
                                            + _error.text();
                                    return false;
                                }
                            }

                            if (key.isReadable())
                            {
                                Channel channel = (Channel)key.attachment();
                                if (DEBUG)
                                    System.out.println("EtajServer: processSelector() channel is readable, channelState="
                                            + channel.state());

                                if (channel.state() == ChannelState.ACTIVE)
                                {
                                    if (!processRead(channel))
                                        return false;
                                }
                                else if (channel.state() == ChannelState.INITIALIZING)
                                {
                                    if (!initializeChannel(channel))
                                        return false;
                                }
                                else
                                {
                                    _errorMsg = "channel state (" + channel.state()
                                            + ") is no longer ACTIVE, aborting.";
                                    return false;
                                }

                            }
                        }
                    }
                }

            }
            catch (IOException e)
            {
                assertTrue("server failure during select, error=" + e.toString(), false);
            }

            return true;
        }

        @Override
        public void run()
        {
            _runningState = RunningState.INITIALIZING;

            if (DEBUG)
                System.out.println("EtajServer: run() entered");

            // Initialize Transport
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(_globalLocking);
            if (Transport.initialize(initArgs, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "errorCode=" + _error.errorId() + " errorText=" + _error.text();
                _running = false;
            }
            else if (!setupServer())
            {
                _running = false;
            }
            else
            {
                _runningState = RunningState.RUNNING;
            }

            while (_running)
            {
                if (!processSelector())
                    _running = false;
            }

            closeSockets();

            if (_errorMsg != null)
                System.out.println("EtajServer: run(): error occurred, " + _errorMsg);

            if (Transport.uninitialize() != TransportReturnCodes.SUCCESS && _errorMsg == null)
            {
                _errorMsg = "Transport.uninitialize() failed.";
                return;
            }

            _runningState = RunningState.TERMINATED;
            if (DEBUG)
                System.out.println("EtajServer: run() completed");
        }

    } // end of EtajServer class

    /**
     * This client take a channel and send messages until instructed to
     * terminate. It will keep track of the following statistics:
     * <ul>
     * <li>writeCount per priority.
     * <li>messageCount per priority (verify if this is different from
     * writeCount).
     * <li>bytesWritten per priority.
     * <li>uncompressedBytesWritten per priority.
     * </ul>
     */
    class EtajClient implements Runnable
    {
        boolean DEBUG = false;

        Channel _channel;

        String _errorMsg;
        int _id;
        int _priority;
        int _messageSize;
        int _variablePackedMessageSize;
        int _flushInterval;
        int _maxMessageCount;
        boolean _globalLocking;
        boolean _packing;
        boolean _callWriteWithPackedMessage;
        

        // statistics
        long _messageCount = 0;
        long _packedMessageCount = 0;
        long _bytesWritten = 0;
        long _uncompressedBytesWritten = 0;

        volatile boolean _running = true;
        RunningState _runningState = RunningState.INACTIVE;

        final Error _error = TransportFactory.createError();
        final ReadArgs _readArgs = TransportFactory.createReadArgs();
        final InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
        final WriteArgs _writeArgs = TransportFactory.createWriteArgs();
        final static int TIMEOUTMS = 1000; // 100 milliseconds

        /**
         * .
         * 
         * @param bindOptions
         * @param acceptOptions
         * @param globalLocking
         */
        public EtajClient(int id, int priority, Channel channel, TestArgs args)
        {
            assert (channel != null);

            _id = id;

            if (priority <= WritePriorities.LOW)
                _priority = priority;
            else
                _priority = WritePriorities.LOW;

            _messageSize = args.messageSize;

            // start _variablePackedMessageSize one greater since it's
            // immediately decremented in writePackedMessage().
            _variablePackedMessageSize = _messageSize + 1;

            _flushInterval = args.flushInterval;
            _channel = channel;
            _globalLocking = args.globalLocking;
            _packing = args.packing;
            _callWriteWithPackedMessage = args.callWriteWithPackedMessage;
            _maxMessageCount = args.maxMessageCount;

            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": created. priority=" + _priority);
        }

        /**
         * Return server is running.
         * 
         * @return true if running.
         */
        public RunningState state()
        {
            return _runningState;
        }

        /**
         * Instructs the server to terminate.
         */
        public void terminate()
        {
            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": terminate() entered.");

            _running = false;
        }

        public long messageCount()
        {
            return _messageCount;
        }
        
        public long bytesWritten()
        {
            return _bytesWritten;
        }
        
        public long uncompressedBytesWritten()
        {
            return _uncompressedBytesWritten;
        }

        /**
         * Returns the error message that caused the server to abort.
         * 
         * @return String containing error message.
         */
        public String errorMsg()
        {
            return _errorMsg;
        }

        void populateBufferWithData(ByteBuffer buffer, int count)
        {
            // populate msgBuf with data.
            for (int idx = 0; idx < count; idx++)
            {
                buffer.put((byte)_priority);
            }
        }

        TransportBuffer getBuffer(int size, boolean packed)
        {
            int retVal;
            TransportBuffer msgBuf = null;

            while (msgBuf == null)
            {
                msgBuf = _channel.getBuffer(size, packed, _error);
                if (msgBuf == null)
                {
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": getBuffer(): no msgBufs available, errorId="
                                + _error.errorId() + " error=" + _error.text()
                                + ", attempting to flush.");

                    retVal = _channel.flush(_error);
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": getBuffer(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCodes.SUCCESS)
                    {
                        _errorMsg = "getBuffer(): no msgBufs available to write and flush failed, retVal="
                                + retVal + " error=" + _error.text();
                        _error.errorId(retVal);
                        _error.text(_errorMsg);
                        if (DEBUG)
                            System.out.println("EtajClient id=" + _id
                                    + ": getBuffer(): flush failed, error=" + _errorMsg);
                        return null;
                    }

                    try
                    {
                        if (DEBUG)
                            Thread.sleep(250);
                        else
                            Thread.sleep(10);
                    }
                    catch (InterruptedException e)
                    {
                    }

                    if (!_running)
                    {
                        _errorMsg = "getBuffer(): no msgBufs available to write and run time expired";
                        _error.errorId(TransportReturnCodes.FAILURE);
                        _error.text(_errorMsg);
                        return null;
                    }
                }
            }
            return msgBuf;
        }

        // returns TransportReturnCodes
        private int writeMessage()
        {
            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": writeMessage() entered");

            int retVal;

            // get a buffer
            TransportBuffer msgBuf = getBuffer(_messageSize, false);
            if (msgBuf == null)
            {
                return _error.errorId();
            }

            // populate msgBuf with data.
            populateBufferWithData(msgBuf.data(), _messageSize);

            // write
            _writeArgs.clear();
            _writeArgs.priority(_priority);

            retVal = 0;
            long timeout = -1;
            do
            {
                retVal = _channel.write(msgBuf, _writeArgs, _error);
                if (!_running)
                {
                    System.out.println("EtajClient id=" + _id
                            + ": writeMessage(): write() FYI: retVal=" + retVal
                            + " while run time expired.");
                    if (timeout == -1)
                    {
                        timeout = System.currentTimeMillis() + 20000; // 20s
                    }
                    else if (timeout < System.currentTimeMillis())
                    {
                        _errorMsg = "writeMessage(): write() returning retVal=" + retVal
                                + " 20s after run time expired, aborting";
                        System.out.println("EtajClient id=" + _id + " error=" + _errorMsg);
                        return TransportReturnCodes.FAILURE;
                    }
                }
                try
                {
                    if (DEBUG)
                        Thread.sleep(250);
                    else
                        Thread.sleep(1);
                }
                catch (InterruptedException e)
                {
                }
            }
            while (retVal == TransportReturnCodes.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCodes.SUCCESS)
            {
                ++_messageCount;
                _bytesWritten += _writeArgs.bytesWritten();
                _uncompressedBytesWritten += _writeArgs.uncompressedBytesWritten();

                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writeMessage(): _messageCount="
                            + _messageCount + " _flushInterval=" + _flushInterval
                            + " _messageCount % _flushInterval = "
                            + (_messageCount % _flushInterval));

                // flush on flushInterval
                if ((_messageCount % _flushInterval) == 0)
                {
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": writeMessage(): calling flush()");
                    retVal = _channel.flush(_error);
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": writeMessage(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCodes.SUCCESS)
                    {
                        _errorMsg = "writeMessage(): flush failed, retVal=" + retVal + " error="
                                + _error.text();
                        if (DEBUG)
                            System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                                    + _errorMsg);
                        return retVal;
                    }
                }
            }
            else
            {
                _errorMsg = "writeMessage(): write failed, retVal=" + retVal + " error="
                        + _error.text();
                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                            + _errorMsg);
                return retVal;
            }
            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": writeMessage(): completed normally");

            return 0;
        }

        // a packed message cannot exceed maxFragmentSize.
        // returns TransportReturnCodes
        private int writePackedMessage(int availableBytes, boolean callWriteWithPackedMessage)
        {
            final Error error = TransportFactory.createError();

            if (--_variablePackedMessageSize == 0)
                _variablePackedMessageSize = _messageSize;

            if (DEBUG)
                System.out.println("EtajClient id=" + _id
                        + ": writePackedMessage() entered, _variablePackedMessageSize="
                        + _variablePackedMessageSize);

            int retVal;

            // get a buffer
            TransportBuffer msgBuf = getBuffer(availableBytes, true);
            if (msgBuf == null)
            {
                return _error.errorId();
            }

            /*
             * if "callWriteWithPackedMessage" is true, we will call
             * channel.write() with the last packed message in the buffer.
             * Otherwise, all messages will be packed with channel.packBuffer()
             * and channel.write() will be called with no additional data
             * written to the buffer (i.e. an empty buffer).
             */
            int bufferSpaceNeeded = _variablePackedMessageSize;
            if (callWriteWithPackedMessage)
            {
                /*
                 * ensure there is enough space remaining for two messages. The
                 * first message will be packed with channel.packBuffer(). The
                 * second message will be packed with channel.write(). Since
                 * this is a junit, we know to leave space for the PACKED_HDR
                 * which is needed for the last message.
                 */
                bufferSpaceNeeded += _variablePackedMessageSize
                        + RsslSocketChannel.RIPC_PACKED_HDR_SIZE;
            }

            // pack message
            while (availableBytes >= bufferSpaceNeeded)
            {
                // populate msgBuf with data.
                populateBufferWithData(msgBuf.data(), _variablePackedMessageSize);

                // pack message
                availableBytes = _channel.packBuffer(msgBuf, error);
                ++_messageCount;
                if (DEBUG)
                    System.out.println("EtajClient id=" + _id
                            + ": writePackedMessage(): _messageCount=" + _messageCount
                            + " _variablePackedMessageSize=" + _variablePackedMessageSize);

                if (_maxMessageCount > 0 && _messageCount >= _maxMessageCount)
                {
                    System.out.println("EtajClient id=" + _id
                            + ": writePackedMessage(): _messageCount=" + _messageCount
                            + " has exceeded _maxMessageCount=" + _maxMessageCount);
                    break;
                }
            }

            if (callWriteWithPackedMessage)
            {
                // populate msgBuf with a message so that write will have to
                // pack the last message.
                for (int idx = 0; idx < _variablePackedMessageSize; idx++)
                {
                    if (msgBuf.data().position() == msgBuf.data().limit())
                        System.out.println("EtajClient.writePackedMessage(): position == limit!!!");
                    msgBuf.data().put((byte)_priority);
                }
                ++_messageCount;
            }

            if (DEBUG)
                System.out.println("EtajClient id=" + _id
                        + ": writePackedMessage(): _messageCount=" + _messageCount
                        + " _variablePackedMessageSize=" + _variablePackedMessageSize);

            // write the packed message
            _writeArgs.clear();
            _writeArgs.priority(_priority);
            retVal = _channel.write(msgBuf, _writeArgs, _error);
            if (retVal >= TransportReturnCodes.SUCCESS)
            {
                ++_packedMessageCount;
                _bytesWritten += _writeArgs.bytesWritten();
                _uncompressedBytesWritten += _writeArgs.uncompressedBytesWritten();
                
                if (DEBUG)
                    System.out.println("EtajClient id=" + _id
                            + ": writePackedMessage(): _packedMessageCount=" + _packedMessageCount
                            + " _flushInterval=" + _flushInterval
                            + " _packedMessageCount % _flushInterval = "
                            + (_packedMessageCount % _flushInterval));

                // flush on flushInterval
                if ((_packedMessageCount % _flushInterval) == 0 || _maxMessageCount > 0 && _messageCount >= _maxMessageCount)
                {
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": writePackedMessage(): calling flush()");
                    retVal = _channel.flush(_error);
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": writePackedMessage(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCodes.SUCCESS)
                    {
                        _errorMsg = "writePackedMessage(): flush failed, retVal=" + retVal
                                + " error=" + _error.text();
                        if (DEBUG)
                            System.out.println("EtajClient id=" + _id
                                    + ": writePackedMessage(): error=" + _errorMsg);
                        return retVal;
                    }
                }
            }
            else
            {
                _errorMsg = "writePackedMessage(): write failed, retVal=" + retVal + " error="
                        + _error.text();
                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writePackedMessage(): error="
                            + _errorMsg);
                return retVal;
            }
            if (DEBUG)
                System.out.println("EtajClient id=" + _id
                        + ": writePackedMessage(): completed normally");

            return 0;
        }

        @Override
        public void run()
        {
            _runningState = RunningState.INITIALIZING;

            if (DEBUG)
                System.out.println("EtajClient id=" + _id + ": run() running");

            // Initialize Transport
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(_globalLocking);
            if (Transport.initialize(initArgs, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "run(): errorCode=" + _error.errorId() + " errorText=" + _error.text();
                _running = false;
            }

            // get channel info for maxFragmentSize.
            ChannelInfo channelInfo = TransportFactory.createChannelInfo();
            if (_channel.info(channelInfo, _error) != TransportReturnCodes.SUCCESS)
            {
                _errorMsg = "run(): channel.info() failed. errorCode=" + _error.errorId()
                        + " errorText=" + _error.text();
                _running = false;
            }

            _runningState = RunningState.RUNNING;

            int retVal;
            int sleepTime = 1;
            if (DEBUG)
                sleepTime = 250;

            while (_running)
            {
                if (!_packing)
                    retVal = writeMessage();
                else
                    retVal = writePackedMessage(channelInfo.maxFragmentSize(), _callWriteWithPackedMessage);

                if (retVal < 0)
                {
                    _running = false;
                }
                else if (_maxMessageCount > 0 && _messageCount >= _maxMessageCount)
                {
                    System.out.println("EtajClient id=" + _id + ": run(): _messageCount="
                            + _messageCount + " has exceeded _maxMessageCount=" + _maxMessageCount
                            + ", stopping");
                    _running = false;
                }

                // Go to sleep to let others run once in a while.
                try
                {
                    Thread.sleep(sleepTime);
                }
                catch (InterruptedException e)
                {
                }
            }

            if (_errorMsg != null)
                System.out.println("EtajClient id=" + _id + ": run(): error occurred, " + _errorMsg);

            if (Transport.uninitialize() != TransportReturnCodes.SUCCESS && _errorMsg == null)
            {
                _errorMsg = "EtajClient id=" + _id + ": Transport.uninitialize() failed.";
            }

            _runningState = RunningState.TERMINATED;
            // if (DEBUG)
            System.out.println("EtajClient id=" + _id + ": run() complete. messageCount="
                    + _messageCount);

        }
    } // end of EtajClient class

    public BindOptions defaultBindOptions()
    {
        BindOptions bindOptions = TransportFactory.createBindOptions();
        bindOptions.majorVersion(Codec.majorVersion());
        bindOptions.minorVersion(Codec.minorVersion());
        bindOptions.protocolType(Codec.protocolType());
        bindOptions.connectionType(ConnectionTypes.SOCKET);
        bindOptions.serviceName(DEFAULT_PORT_NUMBER);
        bindOptions.serverToClientPings(false);
        return bindOptions;
    }

    public AcceptOptions defaultAcceptOptions()
    {
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        return acceptOptions;
    }

    public ConnectOptions defaultConnectOptions()
    {
        ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());
        connectOptions.protocolType(Codec.protocolType());
        connectOptions.connectionType(ConnectionTypes.SOCKET);
        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(DEFAULT_PORT_NUMBER);
        return connectOptions;
    }

    /**
     * Start and initialize the client channels.
     * 
     * @param channelCount
     * @param blocking
     * @param compressionType
     * @return
     */
    public Channel[] startClientChannels(int channelCount, int threadsPerChannels,
            int guaranteedOutputBuffers, boolean blocking, boolean writeLocking, int compressionType)
    {
        System.out.println("startClientChannels(): entered, channelCount=" + channelCount);

        Channel[] channels = new Channel[channelCount];
        ConnectOptions connectOptions = defaultConnectOptions();
        connectOptions.blocking(blocking);
        connectOptions.compressionType(compressionType);
        connectOptions.channelWriteLocking(writeLocking);
        connectOptions.guaranteedOutputBuffers(guaranteedOutputBuffers);

        Error error = TransportFactory.createError();
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        // make all of the connections
        for (int idx = 0; idx < channels.length; idx++)
        {
            if ((channels[idx] = Transport.connect(connectOptions, error)) == null)
            {
                System.out.println("startClientChannels(): Transport.connect() for channels[" + idx
                        + "] failed, errorId=" + error.errorId() + " error=" + error.text());
                return null;
            }
            System.out.println("startClientChannels(): Channel[" + idx + "] is INITIALIZING");
        }

        // loop until all connections are ACTIVE.
        long timeout = System.currentTimeMillis() + 20000; // 20 second timeout
        boolean initializing = false;
        boolean[] channelActive = new boolean[channels.length];

        do
        {
            initializing = false;
            for (int idx = 0; idx < channels.length; idx++)
            {
                int retVal;
                if (channels[idx].state() != ChannelState.INITIALIZING)
                {
                    continue;
                }

                if ((retVal = channels[idx].init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                {
                    System.out.println("startClientChannels(): channel.init() for channels[" + idx
                            + "] failed, errorId=" + error.errorId() + " error=" + error.text());
                    return null;
                }
                else if (retVal == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
                {
                    initializing = true;
                }
                else if (!channelActive[idx])
                {
                    // print once for each channel
                    channelActive[idx] = true;
                    System.out.println("startClientChannels(): Channel[" + idx + "] is ACTIVE");
                }
            }
        }
        while (initializing && System.currentTimeMillis() < timeout);

        if (!initializing)
        {
            System.out.println("startClientChannels(): all (" + channelCount
                    + ") channels initiailized");
            return channels;
        }
        else
        {
            System.out.println("startClientChannels(): failed to initialize all channels, channelActive="
                            + channelActive.toString());
            return null;
        }
    }

    /**
     * Create EtajClients for the specified number of threads for the specified
     * number of channels.
     * 
     * @param threadsPerChannels
     * @param channels created from caller.
     * @param etajClients populated with new clients
     * @param clientThreads populated with the new clients' threads.
     */
    public void startClientThreads(TestArgs args, Channel[] channels, EtajClient[] etajClients, Thread[] clientThreads)
    {
        int threadsPerChannels = args.threadsPerChannels;
        
        assert (channels != null);
        assert (etajClients != null);
        assert (clientThreads != null);

        int etajClientCount = -1;

        for (int channelIdx = 0; channelIdx < channels.length; channelIdx++)
        {
            for (int threadIdx = 0; threadIdx < threadsPerChannels; threadIdx++)
            {
                ++etajClientCount;
                etajClients[etajClientCount] = new EtajClient(etajClientCount, threadIdx,
                        channels[channelIdx], args);
                clientThreads[etajClientCount] = new Thread(etajClients[etajClientCount]);
                clientThreads[etajClientCount].start();
            }
        }
        return;
    }

    /**
     * Wait for server state of RUNNING or TERMINATED.
     * 
     * @param server
     * @return true if RunningState.RUNNING, false if RunningState.TERMINATED.
     */
    public boolean waitForStateRunning(EtajServer server)
    {
        try
        {
            while (true)
            {
                if (server.state() == RunningState.RUNNING)
                    return true;
                else if (server.state() == RunningState.TERMINATED)
                    return false;
                else
                    Thread.sleep(100);
            }
        }
        catch (InterruptedException e)
        {
        }
        return false;
    }

    private void terminateServerAndClients(Thread serverThread, EtajServer server,
            Thread[] clientThreads, EtajClient[] etajClients, Channel[] channels)
    {
        System.out.println("terminateServerAndClients(): stopping clients");

        // instruct clients to stop
        for (EtajClient client : etajClients)
        {
            client.terminate();
        }

        System.out.println("terminateServerAndClients(): waiting for clients to finish");

        // wait for all clients to finish
        long timeout = System.currentTimeMillis() + 120000; // 30 second timeout
        boolean stillRunning;
        do
        {
            stillRunning = false;
            for (EtajClient client : etajClients)
            {
                if (client.state() == RunningState.RUNNING)
                    stillRunning = true;
            }

            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
            }

            if (System.currentTimeMillis() > timeout)
            {
                System.out.println("terminateServerAndClients(): failed to stop clients after 10 seconds.");
                break;
            }
        }
        while (stillRunning);

        if (!stillRunning)
        {
            System.out.println("terminateServerAndClients(): flushing client channels.");
            Error error = TransportFactory.createError();
            timeout = System.currentTimeMillis() + 30000; // 30 second timeout
            for (Channel channel : channels)
            {
                int retVal;
                do
                {
                    retVal = channel.flush(error);
                    try
                    {
                        Thread.sleep(10);
                    }
                    catch (InterruptedException e)
                    {
                    }
                    if (System.currentTimeMillis() > timeout)
                    {
                        System.out.println("terminateServerAndClients(): failed to flush client channels after 10 seconds.");
                        break;
                    }
                }
                while (retVal > TransportReturnCodes.SUCCESS);

                if (retVal != TransportReturnCodes.SUCCESS)
                    System.out.println("terminateServerAndClients(): channel.flush() failed. retVal="
                            + retVal + " errorId=" + error.errorId() + " error="
                            + error.text());
            }
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the flushing client channels, due to non-responsive client threads.");
        }

        System.out.println("terminateServerAndClients(): terminating server");
        // allow extra time for server to receive client packets
        try
        {
            Thread.sleep(3000);
        }
        catch (InterruptedException e)
        {
        }
        server.terminate();

        System.out.println("terminateServerAndClients(): waiting for server to finish");
        // wait for server to terminate
        timeout = System.currentTimeMillis() + 30000; // 30 second timeout
        boolean serverStillRunning = false;
        do
        {
            serverStillRunning = false;
            if (server.state() == RunningState.RUNNING)
                serverStillRunning = true;

            try
            {
                Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
            }
            if (System.currentTimeMillis() > timeout)
            {
                System.out.println("terminateServerAndClients(): timeout waiting for server to finish.");
                break;
            }
        }
        while (serverStillRunning);

        // join all client threads
        if (!stillRunning)
        {
            System.out.println("terminateServerAndClients(): joining client threads");
            for (Thread clientThread : clientThreads)
            {
                try
                {
                    clientThread.join();
                }
                catch (InterruptedException e)
                {
                    e.printStackTrace();
                }
            }
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the joining of client threads, due to non-responsive client threads.");
        }

        try
        {
            serverThread.join();
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
        }

        System.out.println("terminateServerAndClients(): closing channels");

        // close all channels
        if (!stillRunning)
        {
            Error error = TransportFactory.createError();
            for (Channel channel : channels)
            {
                channel.close(error); // ignore return code since server will
                                      // close the channels as well, just prior
                                      // to this call.
            }
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the closing of client channels, due to non-responsive client threads.");
        }

        System.out.println("terminateServerAndClients(): completed");
    }

    private String compareStatistics(boolean packing, int clientMessageSize, EtajServer server,
            EtajClient[] etajClients)
    {
        long clientMessageCount = 0;
        long clientBytesWritten = 0;
        long clientUncompressedBytesWritten = 0;
        long serverMessageCount = 0;
        long serverBytesRead = 0;
        long serverUncompressedBytesRead = 0;
        StringBuilder sb = new StringBuilder();

        // collect and combine all client stats
        for (EtajClient client : etajClients)
        {
            clientMessageCount += client.messageCount();
            clientBytesWritten += client.bytesWritten();
            clientUncompressedBytesWritten += client.uncompressedBytesWritten();
        }

        // collect server stats
        serverMessageCount = server.messageCount();
        serverBytesRead = server.bytesRead();
        serverUncompressedBytesRead = server.uncompressedBytesRead();

        // compare server vs client stats
        System.out.println("MessageCount: server=" + serverMessageCount + " clients="
                + clientMessageCount + " diff=" + (serverMessageCount - clientMessageCount));
        System.out.println("Bytes Read vs Written: server=" + serverBytesRead + " clients="
                + clientBytesWritten + " diff=" + (serverBytesRead - clientBytesWritten));
        System.out.println("Uncompressed Bytes Read vs Written: server="
                + serverUncompressedBytesRead + " clients=" + clientUncompressedBytesWritten
                + " diff=" + (serverUncompressedBytesRead - clientUncompressedBytesWritten));

        if (serverMessageCount != clientMessageCount)
        {
            sb.append("MessageCount mismatch: serverMessageCount=");
            sb.append(serverMessageCount);
            sb.append(" clientMessageCount=");
            sb.append(clientMessageCount);
            sb.append(". ");
        }
        
        if (serverBytesRead != clientBytesWritten)
        {
            sb.append("ServerBytesRead mismatch ClientBytesWritten: serverBytesRead=");
            sb.append(serverBytesRead);
            sb.append(" clientBytesWritten=");
            sb.append(clientBytesWritten);
            sb.append(". ");
        }

        if (serverUncompressedBytesRead != clientUncompressedBytesWritten)
        {
            sb.append("ServerUncompressedBytesRead mismatch ClientUncompressedBytesWritten: serverUncompressedBytesRead=");
            sb.append(serverUncompressedBytesRead);
            sb.append(" clientUncompressedBytesWritten=");
            sb.append(clientUncompressedBytesWritten);
            sb.append(". ");
        }
        
        if (sb.length() == 0)
            return null; // no errors
        else
            return sb.toString();
    }

    public class TestArgs
    {
        // set default test args
        int runTime = 30;
        int clientSessionCount = 1;
        int threadsPerChannels = 1;
        int messageSize = 75;
        int flushInterval = 10;
        int guaranteedOutputBuffers = 7000;
        boolean globalLocking = true;
        boolean writeLocking = true;
        boolean blocking = false;
        boolean packing = false;
        int compressionType = CompressionTypes.NONE;
        int compressionLevel = 6;
        int ripcMaxUserMsgSize = 6144;
        int maxMessageCount = 0;

        /*
         * when enough messages are packed, write is called. The following
         * options allows write to be called with data to be packed or without.
         */
        boolean callWriteWithPackedMessage = false;

        public String toString()
        {
            return String.format("runTime=%-4d\t\tclientSessionCount=%-2d\tthreadsPerChannels=%-2d\nmessageSize=%-4d\tflushInterval=%-4d\tguaranteedOutputBuffers=%-5d\nglobalLocking=%-5s\twriteLocking=%-5s\tblocking=%-5s\npacking=%-5s\t\tcallWriteWithPackedMessage=%-5s\ncompressionType=%-1d\tcompressionLevel=%-1d\tmaxMessageCount=%d",
                                 runTime, clientSessionCount, threadsPerChannels, messageSize,
                                 flushInterval, guaranteedOutputBuffers, globalLocking, writeLocking,
                                 blocking, packing, callWriteWithPackedMessage, compressionType,
                                 compressionLevel, maxMessageCount);
        }
    }

    /**
     * One session, one thread per channel, global lock false, non-blocking
     * client.
     */
    @Test
    public void lockTest1()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 1;
        args.threadsPerChannels = 1;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = false;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest1", args);
    }

    /**
     * One session, one thread per channel, global lock false, blocking client.
     */
    @Test
    public void lockTest2()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 1;
        args.threadsPerChannels = 1;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = false;
        args.blocking = true;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest2", args);
    }

    /**
     * Two sessions, one thread per channel, global lock true, non-blocking
     * clients.
     */
    @Test
    public void lockTest3()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 2;
        args.threadsPerChannels = 1;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = false;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest3", args);
    }

    /**
     * Two sessions, one thread per channel, global lock false, blocking
     * clients.
     */
    @Test
    public void lockTest4()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 2;
        args.threadsPerChannels = 1;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = false;
        args.blocking = true;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest4", args);
    }

    /**
     * Three sessions, three thread per channel, global lock true, writeLock
     * true, non-blocking clients.
     */
    @Test
    public void lockTest5()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest5", args);
    }

    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients
     */
    @Test
    public void lockTest6()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest6", args);
    }

    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients, packing (callWriteWithPackedMessage = false).
     */
    @Test
    public void lockTest8()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest8", args);
    }
	
    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients, packing (callWriteWithPackedMessage = true).
     */
    @Test
    public void lockTest10()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = true;
        args.callWriteWithPackedMessage = true;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest10", args);
    }

    /**
     * Three sessions, three thread per channel, global lock true, writeLock
     * true, non-blocking clients, ZLIB compression with compression level 6.
     */
    @Test
    public void lockTest11()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        testRunner("lockTest11", args);
    }

    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients, ZLIB compression with compression level 6.
     */
    @Test
    public void lockTest12()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        testRunner("lockTest12", args);
    }



    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients, ZLIB compression with compression level 6 and
     * packing.
     */
    @Test
    public void lockTest14()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        testRunner("lockTest14", args);
    }



    /**
     * Three sessions, three thread per channel, global lock true, writeLock
     * true, non-blocking clients, LZ4 compression with compression level 6.
     */
    @Test
    public void lockTest16()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        testRunner("lockTest16", args);
    }

    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients, ZL4 compression with compression level 6.
     */
    @Test
    public void lockTest17()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        testRunner("lockTest17", args);
    }

    /**
     * Three sessions, three thread per channel, global lock false, writeLock
     * true, blocking clients, LZ4 compression with compression level 6 and
     * packing.
     */
    @Test
    public void lockTest20()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 75;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        testRunner("lockTest20", args);
    }

    /**
     * One session, one thread per channel, global lock false, non-blocking
     * client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest21()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 1;
        args.threadsPerChannels = 1;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = false;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest21", args);
    }

    /**
     * One session, one thread per channel, global lock false, non-blocking
     * client. MessageSize of 6145 (three byte larger than
     * Channel.info.maxFragmentSize + RIPC_HDR_SIZE), BigBuffers will need to be
     * used.
     */
    @Test
    public void lockTest22()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 1;
        args.threadsPerChannels = 1;
        args.messageSize = 6145;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = false;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest22", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * non-packing, non-compression, non-fragmented, single client scenario.
     */
    @Test
    public void lockTest23()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 1;
        args.threadsPerChannels = 1;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = false;
        args.writeLocking = false;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest23", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * non-packing, non-compression, non-fragmented, multi-client scenario.
     */
    @Test
    public void lockTest24()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest24", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * packing, non-compression, non-fragmented, multi-client scenario.
     */
    @Test
    public void lockTest25()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest25", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * packing (callWriteWithPackedMessage), non-compression, non-fragmented,
     * multi-client scenario.
     */
    @Test
    public void lockTest26()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = true;
        args.callWriteWithPackedMessage = true;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest26", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * Non-packing, compression (ZLIB), non-fragmented, multi-client scenario.
     */
    @Test
    public void lockTest27()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        testRunner("lockTest27", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * packing, compression (ZLIB), non-fragmented, multi-client scenario.
     */
    @Test
    public void lockTest28()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        testRunner("lockTest28", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * Non-packing, compression (LZ4), non-fragmented, multi-client scenario.
     */
    @Test
    public void lockTest29()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 1022;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        testRunner("lockTest29", args);
    }

    /**
     * Test with non-default RIPC Max User Message Size, which the server will
     * negotiate with the client(s). Select the max message size the client can
     * use before fragmentation kicks in. That value is RIPC MaxUserMsgSize -
     * PACKED_HDR (2).
     * 
     * Non-packing, non-compression, fragmented, multi-client scenario.
     */
    @Test
    public void lockTest30()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 3000;
        args.ripcMaxUserMsgSize = 1024;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.NONE;

        testRunner("lockTest30", args);
    }

    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, ZLIB compression with compression level 0, non-packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest31()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        testRunner("lockTest31", args);
    }



    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, ZLIB compression with compression level 9, non-packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest33()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 9;

        testRunner("lockTest33", args);
    }

    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, ZLIB compression with compression level 9, packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest34()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 9;

        testRunner("lockTest34", args);
    }

    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, LZ4 compression with compression level 0, non-packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest35()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        testRunner("lockTest35", args);
    }

    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, LZ4 compression with compression level 0, packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest36()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        testRunner("lockTest36", args);
    }

    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, LZ4 compression with compression level 9, non-packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest37()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = false;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 9;

        testRunner("lockTest37", args);
    }

    /**
     * Three sessions, three threads per channel, global lock true, write
     * locking true, LZ4 compression with compression level 9, packing,
     * non-blocking client. MessageSize of 6142 (Channel.info.maxFragmentSize)
     */
    @Test
    public void lockTest38()
    {
        TestArgs args = new TestArgs();

        args.runTime = 5;
        args.clientSessionCount = 3;
        args.threadsPerChannels = 3;
        args.messageSize = 6142;
        args.flushInterval = 10;
        args.guaranteedOutputBuffers = 7000;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.packing = true;
        args.callWriteWithPackedMessage = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 9;

        testRunner("lockTest38", args);
    }



    public void testRunner(String testName, TestArgs args)
    {
        System.out.println("--------------------------------------------------------------------------------");
        System.out.println("Test: " + testName);
        System.out.println(args.toString());
        System.out.println("--------------------------------------------------------------------------------");
        long endTime = System.currentTimeMillis() + (args.runTime * 1000);

        assertTrue("Cannot have client blocking and globalLocking=true with server using same JVM, globalLocking will deadlock.",
                   (args.blocking && args.globalLocking) == false);

        if (args.packing)
            assertTrue("Cannot perform packing on message sizes greater than 6142.",
                       args.messageSize <= 6142);

        // BindOptions
        BindOptions bindOptions = defaultBindOptions();
        bindOptions.maxFragmentSize(args.ripcMaxUserMsgSize);
        bindOptions.compressionType(args.compressionType);
        if (args.compressionType > CompressionTypes.NONE)
            bindOptions.compressionLevel(args.compressionLevel);

        // AcceptOptions
        AcceptOptions acceptOptions = defaultAcceptOptions();

        // Client sessions and threads
        EtajClient[] etajClients = new EtajClient[args.clientSessionCount * args.threadsPerChannels];
        Thread[] clientThreads = new Thread[args.clientSessionCount * args.threadsPerChannels];

        // create the server thread and start it
        EtajServer server = new EtajServer(bindOptions, acceptOptions, args);
        Thread serverThread = new Thread(server);
        serverThread.start();

        if (!waitForStateRunning(server))
        {
            assertTrue("server terminated while waiting for RUNNING state, error="
                               + server.errorMsg(), false);
        }
        else
        {
            // start the channels that represent client sessions
            Channel[] channels = startClientChannels(args.clientSessionCount,
                                                     args.threadsPerChannels,
                                                     args.guaranteedOutputBuffers, args.blocking,
                                                     args.writeLocking, args.compressionType);
            assertNotNull("startClientChannels failed, check output", channels);

            // start the client threads.
            startClientThreads(args, channels, etajClients, clientThreads);

            // run until timeout or server.state() is RunningState.TERMINATED
            try
            {
                while (server.state() == RunningState.RUNNING
                        && System.currentTimeMillis() < endTime)
                    Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
            }

            terminateServerAndClients(serverThread, server, clientThreads, etajClients, channels);

            /*
             * Server errors will take precedence over the statistics
             * comparison. If both the statistics comparison and the server
             * failed, fail the test based on the server failure. Otherwise,
             * fail the test on the statistics comparison failure.
             */
            String result = compareStatistics(args.packing, args.messageSize, server, etajClients);
            if (result != null)
            {
                if (server.errorMsg() == null)
                    assertTrue(result, false);
                else
                    System.out.println("server failed (errorMsg to follow) and comparison of stats failed with: "
                                        + result);
            }
        }

        // If a server failure occurred, fail the test.
        if (server.errorMsg() != null)
            assertTrue(server.errorMsg(), false);
    }

}
