///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019,2024 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;
import java.util.function.Function;
import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

import org.junit.Test;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.transport.Ripc.CompressionTypes;
import org.junit.runner.RunWith;
import org.junit.runners.Parameterized;

/**
 * 
 * The {@link TransportMessageJunit} creates a Channel between
 * a client and server. Messages are sent from client to server,
 * and the received message is verified to be identical to what
 * was sent. 
 * <p>
 * The test options include <ul>
 * <li>An array of messages sizes; one message will be sent of each message size.</li> 
 * <li>The compression type and level for the channel.</li>
 * <li>The content of the message from {@link MessageContentType}:
 * UNIFORM (all 1's), SEQUENCE (1,2,3...), or RANDOM (good for 
 * simulating poor compression). </li>
 * </ul>
 * <p>
 * <p>
 * The client/server framework for this test was based on {@link TransportLockJunit}.
 *
 */
@RunWith(Parameterized.class)
public class TransportMessageJunit
{
    private final String serverSecurityProvider;
    private final String clientSecurityProvider;

    @Parameterized.Parameters
    public static Object[][] data()
    {
        return new Object[][]{
                {"SunJSSE", "SunJSSE"},
                {"Conscrypt", "Conscrypt"},
                {"SunJSSE", "Conscrypt"},
                {"Conscrypt", "SunJSSE"}
        };
    }

    public TransportMessageJunit(String serverSecurityProvider, String clientSecurityProvider)
    {
        this.serverSecurityProvider = serverSecurityProvider;
        this.clientSecurityProvider = clientSecurityProvider;
    }

    enum RunningState
    {
        INACTIVE, INITIALIZING, RUNNING, TERMINATED
    };
    
    enum MessageContentType
    {
        UNIFORM, SEQUENCE, RANDOM, MIXED_RANDOM_START, MIXED_UNIFORM_START
    };

    static int PRINT_NUM_BYTES = 80;
    
    interface DataHandler
    {
        void handleMessage(TransportBuffer buffer);
        int receivedCount();
        void setExpectedMessage(ByteBuffer b);
        ArrayList<Boolean> comparisonResults();
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
        DataHandler _dataHandler;

        // statistics
        long _messageCount = 0;
        long _compressedBytesRead = 0;
        long _bytesRead = 0;

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
                boolean globalLocking, DataHandler dataHandler)
        {
            assert (bindOptions != null);
            assert (acceptOptions != null);

            _bindOptions = bindOptions;
            _acceptOptions = acceptOptions;
            _globalLocking = globalLocking;
            _dataHandler = dataHandler;
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

        public long messageCount()
        {
            return _messageCount;
        }

        public long compressedBytesRead()
        {
            return _compressedBytesRead;
        }

        public long bytesRead()
        {
            return _bytesRead;
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

        private boolean processRead(Channel channel)
        {
            _readArgs.clear();
            do
            {
                TransportBuffer msgBuf = channel.read(_readArgs, _error);
                if (msgBuf != null)
                {
                    if (DEBUG)
                        System.out.println("Message read: " + msgBuf.length());
                    _dataHandler.handleMessage(msgBuf);
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
                        System.out.println("Channel READ_WOULD_BLOCK");
                        return true;
                    }
                }
            }
            while (_readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            if (_readArgs.readRetVal() != TransportReturnCodes.SUCCESS && _running)
            {
                // ignore this error if we are terminating, which would mean that the client terminated as expected.
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
                                    System.out
                                            .println("EtajServer: processSelector() accepting a connection");

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
                                    System.out
                                            .println("EtajServer: processSelector() channel is readable, channelState="
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
        boolean _globalLocking;

        // statistics
        long _messageCount = 0;
        long _uncompressedBytesWritten = 0;
        long _bytesWritten = 0;

        volatile boolean _running = true;
        RunningState _runningState = RunningState.INACTIVE;

        final Error _error = TransportFactory.createError();
        final ReadArgs _readArgs = TransportFactory.createReadArgs();
        final InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
        final WriteArgs _writeArgs = TransportFactory.createWriteArgs();
        final static int TIMEOUTMS = 1000; // 100 milliseconds

        java.util.Random _gen;
        
        /**
         * .
         */
        public EtajClient(int id, int priority, 
                Channel channel, boolean globalLocking)
        {
            assert (channel != null);

            _id = id;

            if (priority <= WritePriorities.LOW)
                _priority = priority;
            else
                _priority = WritePriorities.LOW;

            _channel = channel;
            _globalLocking = globalLocking;

            _gen = new java.util.Random(2289374);
            
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

        public long uncompressedBytesWritten()
        {
            return _uncompressedBytesWritten;
        }

        public long bytesWritten()
        {
            return _bytesWritten;
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

        // dataBuf: user data to put in each of the messages
        // packCount: number of packed messages to send
        private int writePackedMessages(PackedTestArgs args)
        {
            int retVal = 0;
            TransportBuffer msgBuf = null;
            int userBytes = 0; 
            final Error error = TransportFactory.createError();

            int payloadSize = (args.packedMessageSize + RsslSocketChannel.RIPC_PACKED_HDR_SIZE) * args.packedCount;
			// Packed buffer
            msgBuf = _channel.getBuffer(payloadSize, true, _error);
            
            int availableRemaining = payloadSize - msgBuf.length();
            if (args.debug)
                System.out.println("[Packing] start payloadSize=" + payloadSize + " availableRemaining=" + availableRemaining);
            for (int p = 1; 
                    p <= args.packedCount; 
                    p++)
            {
                if (args.debug)
                    System.out.println("[Packing] #" + p + " availableRemaining=" + availableRemaining);
                
                // populate buffer with payload
                ByteBuffer dataBuf = ByteBuffer.allocate(args.packedMessageSize);
                dataBuf.clear();
                populateMessage(dataBuf, dataBuf.capacity(), args.messageContent);

                args.setExpectedMessage(dataBuf); // for comparison on the receive side
                dataBuf.position(0);
                for( int n = 0; n < dataBuf.limit(); n++ )
                {
                    msgBuf.data().put(dataBuf.get());
                }
                userBytes += dataBuf.limit();
                
                // pack
                availableRemaining = _channel.packBuffer(msgBuf, error);
            }
            if (args.debug)
                System.out.println("[Packing] availableRemaining at end of packing:" + availableRemaining);
            
            _writeArgs.clear();
            
            do
            {
                if (retVal != 0)
                    System.out.println("writePackedMessages(): last retVal=" + retVal 
                                       + " msgBufPos=" + msgBuf.data().position()
                                      + " msgBufLimit=" + msgBuf.data().limit());
                retVal = _channel.write(msgBuf, _writeArgs, _error);
                if(!_running)
                {
                    System.out.println("EtajClient id=" + _id + ": writePackedMessages(): write() retVal=" + retVal + " while run time expired.");
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
            }
            while (retVal == TransportReturnCodes.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCodes.SUCCESS)
            {
                ++_messageCount;
                _bytesWritten += _writeArgs.bytesWritten();
                _uncompressedBytesWritten += _writeArgs.uncompressedBytesWritten();

                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writePackedMessages(): _bytesWritten="
                            + _bytesWritten + " _uncompressedBytesWritten="
                            + _uncompressedBytesWritten 
                            + " userBytes=" + userBytes);

                retVal = _channel.flush(_error);

                if (retVal < TransportReturnCodes.SUCCESS)
                {
                    _errorMsg = "writeMessage(): flush failed, retVal=" + retVal + " error="
                            + _error.text();

                    System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                                + _errorMsg);
                    return retVal;
                }
            }
            else
            {
                _errorMsg = "writeMessage(): write failed, retVal=" + retVal + " error="
                        + _error.text();

                System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                            + _errorMsg);
                return retVal;
            }

            return retVal;
        }
        
        ////////////////////////////////////////////
        // returns TransportReturnCodes
        private int writeMessage(ByteBuffer dataBuf)
        {
            int retVal;
            TransportBuffer msgBuf = null;

            while (msgBuf == null)
            {
				// Not packed 
                msgBuf = _channel.getBuffer(dataBuf.limit(), false, _error);
                if (msgBuf == null)
                {
                    System.out.println("EtajClient id=" + _id
                                           + ": writeMessage(): no msgBufs available, errorId=" + _error.errorId()
                                           + " error=" + _error.text() + ", attempting to flush.");

                    retVal = _channel.flush(_error);
                    if (DEBUG)
                        System.out.println("EtajClient id=" + _id
                                + ": writeMessage(): flush() returned retVal=" + retVal);

                    if (retVal < TransportReturnCodes.SUCCESS)
                    {
                        _errorMsg = "writeMessage(): no msgBufs available to write and flush failed, retVal="
                                + retVal + " error=" + _error.text();

                        System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                                    + _errorMsg);
                        return retVal;
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
                    
                    if (!_running)
                    {
                        _errorMsg = "writeMessage(): no msgBufs available to write and run time expired";
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            // populate msgBuf with data.
            dataBuf.position(0);
            for( int n = 0; n < dataBuf.limit(); n++ )
            {
                msgBuf.data().put(dataBuf.get());
            }

            // write
            _writeArgs.clear();
            _writeArgs.priority(_priority);
            
            int origLen=msgBuf.length(), origPos=msgBuf.data().position(), origLimit=msgBuf.data().limit();
            retVal = 0;

            do
            {
                if (retVal !=0)
                    System.out.println("writeMessage(): last retVal=" + retVal + " origLen="
                                       + origLen + " origPos=" + origPos + " origLimit="
                                       + origLimit + " msgBufPos=" + msgBuf.data().position()
                                      + " msgBufLimit=" + msgBuf.data().limit());
                retVal = _channel.write(msgBuf, _writeArgs, _error);
                if(!_running)
                {
                    System.out.println("EtajClient id=" + _id + ": writeMessage(): write() retVal=" + retVal + " while run time expired.");
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
            }
            while (retVal == TransportReturnCodes.WRITE_CALL_AGAIN);

            if (retVal >= TransportReturnCodes.SUCCESS)
            {
                ++_messageCount;
                _bytesWritten += _writeArgs.bytesWritten();
                _uncompressedBytesWritten += _writeArgs.uncompressedBytesWritten();

                if (DEBUG)
                    System.out.println("EtajClient id=" + _id + ": writeMessage(): _bytesWritten="
                            + _bytesWritten + " _uncompressedBytesWritten="
                            + _uncompressedBytesWritten 
                            + " userBytes=" + dataBuf.limit());

                retVal = _channel.flush(_error);

                if (retVal < TransportReturnCodes.SUCCESS)
                {
                    _errorMsg = "writeMessage(): flush failed, retVal=" + retVal + " error="
                            + _error.text();

                    System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                                + _errorMsg);
                    return retVal;
                }
            }
            else
            {
                _errorMsg = "writeMessage(): write failed, retVal=" + retVal + " error="
                        + _error.text();

                System.out.println("EtajClient id=" + _id + ": writeMessage(): error="
                            + _errorMsg);
                return retVal;
            }

            return 0;
        }
        
        private void populateMessage(ByteBuffer buf, int messageSize, MessageContentType type)
        {
            switch (type)
            {
                case RANDOM:
                    
                    for (int i=0; i < messageSize; i++)
                    {
                        buf.put((byte)(_gen.nextInt() % 255));
                    }
                    break;
                    
                case UNIFORM:
                default:
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        buf.put((byte)1);
                    }
                    break;
                    
                case SEQUENCE:
                    for (int idx = 0; idx < messageSize; idx++)
                    {
                        buf.put((byte)(idx % 255));
                    }
                    break;

                case MIXED_RANDOM_START:
                case MIXED_UNIFORM_START:
                    boolean random = false;
                    if (type == MessageContentType.MIXED_RANDOM_START)
                        random = true;
                    for (int idx = 1; idx <= messageSize; idx++)
                    {
                        if (random)
                        {
                            buf.put((byte)(_gen.nextInt() % 255));
                        }
                        else
                        {
                            buf.put((byte)7);
                        }
                        // approximately the amount of data for a fragment
                        if (idx % 6137 == 0)
                            random = !random;
                    }
                    break;
                    
            }
            
            buf.position(0);
            buf.limit(messageSize);
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

            while (_running); // DRT for now not doing anything in client thread
            
            if (Transport.uninitialize() != TransportReturnCodes.SUCCESS && _errorMsg == null)
            {
                _errorMsg = "EtajClient id=" + _id + ": Transport.uninitialize() failed.";
            }

            _runningState = RunningState.TERMINATED;
            System.out.println("EtajClient id=" + _id + ": run() complete. messageCount=" + _messageCount);

        }
    } // end of EtajClient class

    public BindOptions defaultBindOptions(String portNumber)
    {
        BindOptions bindOptions = TransportFactory.createBindOptions();
        bindOptions.majorVersion(Codec.majorVersion());
        bindOptions.minorVersion(Codec.minorVersion());
        bindOptions.protocolType(Codec.protocolType());
        bindOptions.connectionType(ConnectionTypes.SOCKET);
        bindOptions.serviceName(portNumber);
        bindOptions.serverToClientPings(false);
        return bindOptions;
    }

    public BindOptions encryptedBindOptions(String portNumber, String[] securityProtocolVersions)
    {
        BindOptions bindOptions = TransportFactory.createBindOptions();
        bindOptions.connectionType(ConnectionTypes.ENCRYPTED);
        initEncryptionOptions(bindOptions.encryptionOptions(), securityProtocolVersions);
        bindOptions.serviceName(portNumber);
        bindOptions.sysRecvBufSize(64 * 1024);

        return bindOptions;
    }

    public AcceptOptions defaultAcceptOptions()
    {
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        return acceptOptions;
    }

    public ConnectOptions defaultConnectOptions(String portNumber)
    {
        ConnectOptions connectOptions = TransportFactory.createConnectOptions();
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());
        connectOptions.protocolType(Codec.protocolType());
        connectOptions.connectionType(ConnectionTypes.SOCKET);
        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(portNumber);
        return connectOptions;
    }

    public ConnectOptions encryptedConnectOptions(String portNumber)
    {
        ConnectOptions connectOptions = TransportFactory.createConnectOptions();

        connectOptions.connectionType(ConnectionTypes.ENCRYPTED);
        connectOptions.encryptionOptions().connectionType(ConnectionTypes.SOCKET);
        initEncryptionOptions(connectOptions.encryptionOptions(), new String[] {"1.3", "1.2"});
        connectOptions.tunnelingInfo().tunnelingType("None");
        connectOptions.unifiedNetworkInfo().address("localhost");
        connectOptions.unifiedNetworkInfo().serviceName(portNumber);
        connectOptions.majorVersion(Codec.majorVersion());
        connectOptions.minorVersion(Codec.minorVersion());
        connectOptions.protocolType(Codec.protocolType());

        return connectOptions;
    }

    private void initEncryptionOptions(EncryptionOptions encryptionOptions, String[] securityProtocolVersions)
    {
        encryptionOptions.KeystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
        encryptionOptions.KeystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
        encryptionOptions.SecurityProtocolVersions(securityProtocolVersions);
        encryptionOptions.SecurityProvider(clientSecurityProvider);
    }

    private void initEncryptionOptions(ServerEncryptionOptions encryptionOptions, String[] securityProtocolVersions)
    {
        encryptionOptions.keystoreFile(CryptoHelperTest.VALID_CERTIFICATE);
        encryptionOptions.keystorePasswd(CryptoHelperTest.KEYSTORE_PASSWORD);
        encryptionOptions.securityProtocolVersions(securityProtocolVersions);
        encryptionOptions.securityProvider(serverSecurityProvider);
    }

    /**
     * Start and initialize the client channels.
     * 
     * @param blocking
     * @param compressionType
     * @return
     */
    public Channel startClientChannel(int guaranteedOutputBuffers, 
            boolean blocking, boolean writeLocking, int compressionType, String portNumber, boolean encrypted)
    {
        System.out.println("startClientChannel(): entered");

        Channel channel;
        ConnectOptions connectOptions = encrypted ? encryptedConnectOptions(portNumber) : defaultConnectOptions(portNumber);
        connectOptions.blocking(blocking);
        connectOptions.compressionType(compressionType);
        connectOptions.channelWriteLocking(writeLocking);
        connectOptions.guaranteedOutputBuffers(guaranteedOutputBuffers);

        Error error = TransportFactory.createError();
        InProgInfo inProgInfo = TransportFactory.createInProgInfo();

        if ((channel = Transport.connect(connectOptions, error)) == null)
        {
            System.out.println("startClientChannel(): Transport.connect() for channel "
                  +  "failed, errorId=" + error.errorId() + " error=" + error.text());
            return null;
        }
        System.out.println("startClientChannel(): Channel is INITIALIZING");


        // loop until all connections are ACTIVE.
        long timeout = System.currentTimeMillis() + 20000; // 20 second timeout
        boolean initializing = false;

        do
        {
            initializing = false;
                int retVal;
                if (channel.state() != ChannelState.INITIALIZING)
                {
                    continue;
                }
                
                if ((retVal = channel.init(inProgInfo, error)) < TransportReturnCodes.SUCCESS)
                {
                    System.out.println("startClientChannel(): channel.init() "
                            + " failed, errorId=" + error.errorId() + " error=" + error.text());
                    return null;
                }
                else if (retVal == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
                {
                    initializing = true;
                }
                else 
                {
                    System.out.println("startClientChannel(): Channel is ACTIVE");
                }
        }
        while (initializing && System.currentTimeMillis() < timeout);

        if (!initializing)
        {
            System.out.println("startClientChannel() initialized");
            return channel;
        }
        else
        {
            System.out.println("startClientChannel(): failed to initialize channel");
            return null;
        }
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

    private void terminateServerAndClients(Thread serverThread, 
            EtajServer server,
            Thread clientThread, 
            EtajClient etajClient, 
            Channel channel)
    {
        System.out.println("terminateServerAndClients(): stopping clients");

        // instruct clients to stop
        etajClient.terminate();

        System.out.println("terminateServerAndClients(): waiting for clients to finish");

        // wait for all clients to finish
        long timeout = System.currentTimeMillis() + 12000000; // 20 second timeout
        boolean stillRunning;
        do
        {
            stillRunning = false;
            if (etajClient.state() == RunningState.RUNNING)
                stillRunning = true;

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
                int retVal;
                do
                {
                    retVal = channel.flush(error);
                    try
                    {
                        Thread.sleep(1);
                    }
                    catch (InterruptedException e)
                    {
                    }
                }
                while (retVal > TransportReturnCodes.SUCCESS);
    
                if (retVal != TransportReturnCodes.SUCCESS)
                    System.out.println("terminateServerAndClients(): channel.flush() failed. retVal="
                            + retVal + " errorId=" + error.errorId() + " error=" + error.text());
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the flushing client channels, due to non-responsive client threads.");
        }

        
        System.out.println("terminateServerAndClients(): terminating server");
        server.terminate();

        System.out.println("terminateServerAndClients(): waiting for server to finish");
        // wait for server to terminate
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
        }
        while (serverStillRunning);


        // join all client threads
        if (!stillRunning)
        {
            System.out.println("terminateServerAndClients(): joining client threads");
                try
                {
                    clientThread.join();
                }
                catch (InterruptedException e)
                {
                    e.printStackTrace();
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
                channel.close(error); // ignore return code since server will close the channels as well, just prior to this call.
        }
        else
        {
            System.out.println("terminateServerAndClients(): skipping the closing of client channels, due to non-responsive client threads.");
        }
        
        System.out.println("terminateServerAndClients(): completed");
    }
    
    public static class PackedTestArgs implements DataHandler
    {
        // set default test args
        int runTime = 30;
        int guaranteedOutputBuffers = 7000;
        boolean globalLocking = true;
        boolean writeLocking = true;
        boolean blocking = false;
        int compressionType = CompressionTypes.NONE;
        int compressionLevel = 6;
        int packedMessageSize = 100;
        int packedCount = 1;
        MessageContentType messageContent = MessageContentType.UNIFORM;
        boolean printReceivedData = false;
        boolean debug = false;
        int expectedTotalBytes = -1;
        int expectedUncompressedBytes = -1;


        private static int portNumber = 15100;
        String PORT_NUMBER;

        private PackedTestArgs(){}

        public static PackedTestArgs getInstance()
        {
                PackedTestArgs args = new PackedTestArgs();
                args.PORT_NUMBER = Integer.toString(portNumber++);
                System.out.println ("PACKED PORT NUMBER:" + args.PORT_NUMBER);
                return args;
        }


        // DataHandler support
        int _receivedCount = 0;
        ArrayList<ByteBuffer> _expectedMessages = new ArrayList<ByteBuffer>(20);
        ArrayList<Boolean> _compareResults = new ArrayList<Boolean>(20);

        public String toString()
        {
            return String.format("runTime=%-4d\nguaranteedOutputBuffers=%-5d\nglobalLocking=%-5s\twriteLocking=%-5s\tblocking=%-5s\ncompressionType=%-1d\tcompressionLevel=%-1d\tdataType=%s",
                                 runTime, guaranteedOutputBuffers, globalLocking, writeLocking, blocking, compressionType, compressionLevel, messageContent);
        }

        @Override
        public void handleMessage(TransportBuffer buffer)
        {
            ++_receivedCount;
            if (debug)
                System.out.println("[Packed] receivedCount=" + _receivedCount + " len=" + buffer.length() + " pos=" + buffer.data().position());

            int receivedPos = buffer.data().position();
            if (printReceivedData)
            {
                System.out.print("RECEIVE: ");
                int num = buffer.length();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", buffer.data().get()));
                }
                System.out.println();
            }

            ByteBuffer receivedMsg = buffer.data();
            
            ByteBuffer expected = _expectedMessages.get(_receivedCount - 1);
            
            if (printReceivedData)
            {
                System.out.print("EXPECT:  ");
                expected.position(0);
                int num = expected.limit() - expected.position();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", expected.get()));
                }
                System.out.println();
                receivedMsg.position(receivedPos);
            }
            
            expected.position(0);
            if (expected.compareTo(receivedMsg) == 0)
            {
                if(printReceivedData)
                    System.out.println("\tTRUE");
                _compareResults.add(Boolean.TRUE);
            }
            else
            {
                if(printReceivedData)
                    System.out.println("\tFALSE");
                _compareResults.add(Boolean.FALSE);
            }

        }

        @Override
        public int receivedCount()
        {
            return _receivedCount;
        }
        
        @Override
        public void setExpectedMessage(ByteBuffer b)
        {
            _expectedMessages.add(b);
        }
        
        @Override
        public ArrayList<Boolean> comparisonResults()
        {
            return _compareResults;
        }

    }
    
    public static class TestArgs implements DataHandler
    {
        // set default test args
        int runTime = 30;
        int guaranteedOutputBuffers = 7000;
        boolean globalLocking = true;
        boolean writeLocking = true;
        boolean blocking = false;
        int compressionType = CompressionTypes.NONE;
        int compressionLevel = 6;
        int messageSizes[];
        MessageContentType messageContent = MessageContentType.UNIFORM;
        boolean printReceivedData = false;
        boolean debug = false;
        int expectedTotalBytes = -1;
        int expectedUncompressedBytes = -1;
        boolean encrypted = false;
        String[] securityProtocolVersions = {"1.2", "1.3"};

	private static int portNumber = 15200;
	String PORT_NUMBER;

	private TestArgs(){}

	public static TestArgs getInstance()
	{
		TestArgs args = new TestArgs();
		args.PORT_NUMBER = Integer.toString(portNumber++);
		System.out.println ("PORT NUMBER:" + args.PORT_NUMBER);
		return args;
	}


        // DataHandler support
        int _receivedCount = 0;
        ArrayList<ByteBuffer> _expectedMessages = new ArrayList<ByteBuffer>(20);
        ArrayList<Boolean> _compareResults = new ArrayList<Boolean>(20);

        public String toString()
        {
            return String.format("runTime=%-4d\nguaranteedOutputBuffers=%-5d\nglobalLocking=%-5s\twriteLocking=%-5s\tblocking=%-5s\ncompressionType=%-1d\tcompressionLevel=%-1d\tdataType=%s",
                                 runTime, guaranteedOutputBuffers, globalLocking, writeLocking, blocking, compressionType, compressionLevel, messageContent);
        }

        @Override
        public void handleMessage(TransportBuffer buffer)
        {
            ++_receivedCount;
            if (debug)
                System.out.println("receivedCount=" + _receivedCount + " len=" + buffer.length() + " pos=" + buffer.data().position());

            int receivedPos = buffer.data().position();
            if (printReceivedData)
            {
                System.out.print("RECEIVE: ");
                int num = buffer.length();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", buffer.data().get()));
                }
                System.out.println();
            }

            ByteBuffer receivedMsg = buffer.data();
            
            ByteBuffer expected = _expectedMessages.get(_receivedCount - 1);
            
            if (printReceivedData)
            {
                System.out.print("EXPECT:  ");
                expected.position(0);
                int num = expected.limit() - expected.position();
                if (num > PRINT_NUM_BYTES)
                    num = PRINT_NUM_BYTES;
                for (int n = 0; n < num; n++)
                {
                    System.out.print(String.format("%02X ", expected.get()));
                }
                System.out.println();
                receivedMsg.position(receivedPos);
            }
                     
            expected.position(0);
            if (expected.compareTo(receivedMsg) == 0)
            {
                if(printReceivedData)
                    System.out.println("\tTRUE");
                _compareResults.add(Boolean.TRUE);
            }
            else
            {
                if(printReceivedData)
                    System.out.println("\tFALSE");
                _compareResults.add(Boolean.FALSE);
            }
        }

        @Override
        public int receivedCount()
        {
            return _receivedCount;
        }

        @Override
        public void setExpectedMessage(ByteBuffer b)
        {
            _expectedMessages.add(b);
        }

        @Override
        public ArrayList<Boolean> comparisonResults()
        {
            return _compareResults;
        }

    }

    @Test
    // Primarily for debugging during development
    public void test0()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        
        int[] sizes = { 6140};
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.debug = true;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test0: debugging", args);
    }

    @Test
    // Verify zlib level 0 split/re-combine with CompFragment
    public void test1()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        
        int[] sizes = { 6142 };
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.messageContent = MessageContentType.UNIFORM;
        
        testRunner("test1: basic", args);
    }

    // No compression: message sizes from no-frag to fragmentation
    @Test
    public void test2()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;
        
        int[] sizes = { 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.UNIFORM;
        
        testRunner("test2: no compression fragmentation boundary", args);
    }
    
    // lz4 compression growth: messages sizes from no-frag to fragmentation
    @Test
    public void test3()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        
        int[] sizes = { 6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110};
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test3: lz4 fragmentation boundary test poor compression", args);
    }

    // zlib compression growth: message sizes from no-frag to fragmentation
    @Test
    public void test4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        
        int[] sizes = { 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test4: zlib fragmentation boundary test poor compression", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Random forces compression fragmentation on the first fragment
    @Test
    public void test5()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_RANDOM_START;
        
        testRunner("test5: mixed data random start", args);
    }

    @Test
    public void test5z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_RANDOM_START;
        
        testRunner("test5: mixed data random start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test6()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;
        
        testRunner("test6: mised data uniform start", args);
    }

    @Test
    public void test6z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;
        
        testRunner("test6z: mixed data uniform start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test7()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 500000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;
        
        testRunner("test7: mixed data large message", args);
    }

    @Test
    public void test8()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // frag 1: 6147
        // frag 2:  869
        args.expectedTotalBytes = 7016;
        
        testRunner("test8: ", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8lz4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // frag 1: 6147
        // frag 1b: 29
        // frag 2: 874
        args.expectedTotalBytes = 7050;
        
        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;
        
        testRunner("test8lz4: fragment with compFragment testing write args bytes written", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.debug = true;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // frag 1: 6147
        // frag 1b: 15
        // frag 2: 879
        args.expectedTotalBytes = 7041;
        
        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;
        
        testRunner("test8z: zlib fragment with compFragment testing write args bytes written", args);
    }

    // compressed frag normal: testing writeArgs bytes written
    @Test
    public void test9lz4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        // 1: 6147
        // 1b: 25
        args.expectedTotalBytes = 6172;
        
        // 1: 3
        // 1b: 3
        // data: 6140
        args.expectedUncompressedBytes = 6146;
        
        testRunner("test9lz4: compFragment normal testing write args bytes written", args);
    }
    
    @Test
    public void test10()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test10: ", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10lz4()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test10lz4: last fragment not compressed", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test10z: last fragment not compressed", args);
    }

    @Test
    public void test11z()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        int[] sizes = { 6128, 6129 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunner("test11z: ", args);
    }

    @Test
    public void ptest1()
    {
        PackedTestArgs args = PackedTestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        args.packedMessageSize = 612;
        args.packedCount = 10;
        args.printReceivedData = false;
        args.debug = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunnerPacked("ptest1: packing with random data and lz4 ", args);
    }

    @Test
    public void ptest2()
    {
        PackedTestArgs args = PackedTestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;

        args.packedMessageSize = 612;
        args.packedCount = 10;
        args.printReceivedData = false;
        args.debug = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunnerPacked("ptest2: packing with random data and zlib ", args);
    }

    @Test
    public void ptest3()
    {
        PackedTestArgs args = PackedTestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 6;

        args.packedMessageSize = 612;
        args.packedCount = 10;
        args.printReceivedData = false;
        args.debug = false;
        args.messageContent = MessageContentType.RANDOM;
        
        testRunnerPacked("ptest3: packing with random data and no compression ", args);
    }

    @Test
    public void test0_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;

        int[] sizes = { 6140};
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.debug = true;
        args.messageContent = MessageContentType.RANDOM;
        args.encrypted = true;

        testRunner("test0_encrypted: debugging", args);
    }

    @Test
    public void test1_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;

        int[] sizes = { 6142 };
        args.messageSizes = sizes;
        args.printReceivedData = true;
        args.messageContent = MessageContentType.UNIFORM;
        args.encrypted = true;

        testRunner("test1_encrypted: basic", args);
    }

    // No compression: message sizes from no-frag to fragmentation
    @Test
    public void test2_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.encrypted = true;

        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;

        int[] sizes = { 6140, 6141, 6142, 6143, 6144, 6145, 6146, 6147, 6148, 6149 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.UNIFORM;

        testRunner("test2_encrypted: no compression fragmentation boundary", args);
    }

    // lz4 compression growth: messages sizes from no-frag to fragmentation
    @Test
    public void test3_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = false;
        args.writeLocking = true;
        args.blocking = true;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = {6100, 6101, 6102, 6103, 6104, 6105, 6106, 6107, 6108, 6109, 6110};
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        testRunner("test3_encrypted: lz4 fragmentation boundary test poor compression", args);
    }

    // zlib compression growth: message sizes from no-frag to fragmentation
    @Test
    public void test4_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = { 6123, 6124, 6125, 6126, 6127, 6128, 6129, 6130, 6131, 6132, 6133, 6134, 6135, 6136, 6137, 6138, 6139, 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        testRunner("test4_encrypted: zlib fragmentation boundary test poor compression", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Random forces compression fragmentation on the first fragment
    @Test
    public void test5_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_RANDOM_START;

        testRunner("test5_encrypted: mixed data random start", args);
    }

    @Test
    public void test5z_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_RANDOM_START;

        testRunner("test5_encrypted: mixed data random start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test6_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;

        testRunner("test6_encrypted: mised data uniform start", args);
    }

    @Test
    public void test6z_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = { 50000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;

        testRunner("test6z_encrypted: mixed data uniform start", args);
    }

    // Alternate fragments with Random and Uniform data.
    // Starting with Uniform forces compression fragmentation on the second fragment
    @Test
    public void test7_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 6;
        args.encrypted = true;

        int[] sizes = { 500000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.MIXED_UNIFORM_START;

        testRunner("test7_encrypted: mixed data large message", args);
    }

    @Test
    public void test8_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;
        args.encrypted = true;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 2:  869
        args.expectedTotalBytes = 7016;

        testRunner("test8_encrypted: ", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8lz4_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;
        args.encrypted = true;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 29
        // frag 2: 874
        args.expectedTotalBytes = 7050;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;

        testRunner("test8lz4_encrypted: fragment with compFragment testing write args bytes written", args);
    }

    // fragment + compressed frag testing writeArgs bytes written
    @Test
    public void test8z_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.debug = true;
        args.encrypted = true;

        int[] sizes = { 7000 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        // frag 1: 6147
        // frag 1b: 15
        // frag 2: 879
        args.expectedTotalBytes = 7041;

        // frag 1: 10
        // frag 1b: 3
        // frag 2: 6
        // data: 7000
        args.expectedUncompressedBytes = 7019;

        testRunner("test8z_encrypted: zlib fragment with compFragment testing write args bytes written", args);
    }

    // compressed frag normal: testing writeArgs bytes written
    @Test
    public void test9lz4_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;

        int[] sizes = { 6140 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;
        args.encrypted = true;

        // 1: 6147
        // 1b: 25
        args.expectedTotalBytes = 6172;

        // 1: 3
        // 1b: 3
        // data: 6140
        args.expectedUncompressedBytes = 6146;

        testRunner("test9lz4_encrypted: compFragment normal testing write args bytes written", args);
    }

    @Test
    public void test10_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.NONE;
        args.compressionLevel = 0;
        args.encrypted = true;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        testRunner("test10_encrypted: ", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10lz4_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.LZ4;
        args.compressionLevel = 0;
        args.encrypted = true;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        testRunner("test10lz4_encrypted: last fragment not compressed", args);
    }

    // Designed so that the last fragment size is below the compression threshold
    @Test
    public void test10z_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.encrypted = true;

        int[] sizes = { 12300 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        testRunner("test10z_encrypted: last fragment not compressed", args);
    }

    @Test
    public void test11z_encrypted()
    {
        TestArgs args = TestArgs.getInstance();

        args.runTime = 30;
        args.guaranteedOutputBuffers = 700;
        args.globalLocking = true;
        args.writeLocking = true;
        args.blocking = false;
        args.compressionType = CompressionTypes.ZLIB;
        args.compressionLevel = 0;
        args.encrypted = true;

        int[] sizes = { 6128, 6129 };
        args.messageSizes = sizes;
        args.printReceivedData = false;
        args.messageContent = MessageContentType.RANDOM;

        testRunner("test11z_encrypted: ", args);
    }

    public void testRunner(String testName, TestArgs args)
    {
        System.out.println("--------------------------------------------------------------------------------");
        System.out.println("Test: " + testName);
        System.out.println(args.toString());
        System.out.println("--------------------------------------------------------------------------------");
        long endTime = System.currentTimeMillis() + (args.runTime * 1000);

        assertTrue("Cannot have client blocking and globalLocking=true with server using same JVM, globalLocking will deadlock.", (args.blocking && args.globalLocking) == false);

        // BindOptions
        BindOptions bindOptions = args.encrypted ? encryptedBindOptions(args.PORT_NUMBER, args.securityProtocolVersions) : defaultBindOptions(args.PORT_NUMBER);
        bindOptions.compressionType(args.compressionType);
        if (args.compressionType > CompressionTypes.NONE)
            bindOptions.compressionLevel(args.compressionLevel);

        // AcceptOptions
        AcceptOptions acceptOptions = defaultAcceptOptions();

        // create the server thread and start it
        EtajServer server = new EtajServer(bindOptions, acceptOptions, args.globalLocking, args);
        Thread serverThread = new Thread(server);
        serverThread.start();

        if (!waitForStateRunning(server))
        {
            assertTrue("server terminated while waiting for RUNNING state, error="
                               + server.errorMsg(), false);
        }
        else
        {
            System.out.println("Server bound");
            // start the channels that represent client session
            Channel clientChannel = startClientChannel(args.guaranteedOutputBuffers,
                                                       args.blocking,
                                                       args.writeLocking,
                                                       args.compressionType,
							args.PORT_NUMBER, args.encrypted);
            assertNotNull("startClientChannel failed, check output", clientChannel);

            EtajClient etajClient = new EtajClient(1, // etajClientCount
                                                   0, // priority
                                                   clientChannel,
                                                   args.globalLocking);
            Thread clientThread = new Thread(etajClient);
            clientThread.start();

            int messageCount = 0;
            boolean testFailed = false;
            for (int testMessageSize : args.messageSizes)
            {
                ++messageCount;
                if (server.state() != RunningState.RUNNING)
                {
                    System.out.println("Terminating message loop (Server !RUNNING): at message " 
                                + messageCount + " of " + args.messageSizes.length);
                    break;
                }
                
                ByteBuffer msgData = ByteBuffer.allocate(testMessageSize);
                args.setExpectedMessage(msgData);
                etajClient.populateMessage(msgData, msgData.capacity(), args.messageContent);
                int writeReturn = etajClient.writeMessage(msgData);
                if (writeReturn < TransportReturnCodes.SUCCESS)
                {
                    System.out.println("Terminating message loop (writeMessage return code " 
                            + writeReturn + "): at message " 
                            + messageCount + " of " + args.messageSizes.length);
                    testFailed = true;
                    break;
                }
                if (args.debug)
                    System.out.println("writeArgs.bytesWritten()=" + etajClient._writeArgs.bytesWritten()
                                       + " writeArgs.uncompressedBytesWritten=" + etajClient._writeArgs.uncompressedBytesWritten());

                if (args.expectedTotalBytes != -1)
                {
                    assertTrue(args.expectedTotalBytes == etajClient._writeArgs.bytesWritten());
                }
                if (args.expectedUncompressedBytes != -1)
                {
                    assertTrue(args.expectedUncompressedBytes == etajClient._writeArgs.uncompressedBytesWritten());
                }
                
                try
                {
                    // wait for message to be received to compare with message sent
                    boolean messageTestComplete = false;
                    while (!messageTestComplete
                            && server.state() == RunningState.RUNNING
                            && System.currentTimeMillis() < endTime)
                    {
                        Thread.sleep(1000);
                    
                        if (messageCount == args.receivedCount())
                        {
                            ArrayList<Boolean> results = args.comparisonResults();
                            if (results != null)
                            {
                                System.out.println(messageCount + " of " + args.messageSizes.length
                                      + ": message comparison " + results.get(messageCount-1));
                                
                               // if (!results.get(messageCount-1).booleanValue())
                               //     testFailed = true;
                                messageTestComplete = true; // next
                                break;
                            }

                        }
                    }
                }
                catch (InterruptedException e)
                {
                }
            }

            // verify test made it through all messages before exit
            assertTrue(messageCount == args.messageSizes.length);
            assertTrue(!testFailed);
            
            terminateServerAndClients(serverThread, server, clientThread, etajClient, clientChannel);

        }

        // If a server failure occurred, fail the test.
        if (server.errorMsg() != null)
            assertTrue(server.errorMsg(), false);
    }

    public void testRunnerPacked(String testName, PackedTestArgs args)
    {
        System.out.println("--------------------------------------------------------------------------------");
        System.out.println("Packed Test: " + testName);
        System.out.println(args.toString());
        System.out.println("--------------------------------------------------------------------------------");
        long endTime = System.currentTimeMillis() + (args.runTime * 1000);

        assertTrue("Cannot have client blocking and globalLocking=true with server using same JVM, globalLocking will deadlock.", (args.blocking && args.globalLocking) == false);

        // BindOptions
        BindOptions bindOptions = defaultBindOptions(args.PORT_NUMBER);
        bindOptions.compressionType(args.compressionType);
        if (args.compressionType > CompressionTypes.NONE)
            bindOptions.compressionLevel(args.compressionLevel);

        // AcceptOptions
        AcceptOptions acceptOptions = defaultAcceptOptions();

        // create the server thread and start it
        EtajServer server = new EtajServer(bindOptions, acceptOptions, args.globalLocking, args);
        Thread serverThread = new Thread(server);
        serverThread.start();

        if (!waitForStateRunning(server))
        {
            assertTrue("server terminated while waiting for RUNNING state, error="
                               + server.errorMsg(), false);
        }
        else
        {
            // start the channels that represent client session
            Channel clientChannel = startClientChannel(args.guaranteedOutputBuffers,
                                                       args.blocking,
                                                       args.writeLocking,
                                                       args.compressionType,
							args.PORT_NUMBER, false);
            assertNotNull("startClientChannel failed, check output", clientChannel);

            EtajClient etajClient = new EtajClient(1, // etajClientCount
                                                   0, // priority
                                                   clientChannel, 
                                                   args.globalLocking);
            Thread clientThread = new Thread(etajClient);
            clientThread.start();

                if (server.state() != RunningState.RUNNING)
                {
                    System.out.println("Terminating message loop (Server !RUNNING)");
                    return;
                }
                
                System.out.println("[testRunnerPacked] sending " + args.packedCount 
                                   + " packed messages of size " + args.packedMessageSize);
                int writeReturn = etajClient.writePackedMessages(args);
                if (writeReturn < TransportReturnCodes.SUCCESS)
                {
                    System.out.println("Terminating message loop writePackedMessages return code " 
                            + writeReturn); 
                    
                    return;
                }
                System.out.println("writeArgs.bytesWritten()=" + etajClient._writeArgs.bytesWritten()
                                   + " writeArgs.uncompressedBytesWritten=" + etajClient._writeArgs.uncompressedBytesWritten());

                try
                {
                    // wait for message to be received to compare with message sent
                    boolean messageTestComplete = false;
                    while (!messageTestComplete
                            && server.state() == RunningState.RUNNING
                            && System.currentTimeMillis() < endTime)
                    {
                        Thread.sleep(100);
                    
                        if (args.receivedCount() == args.packedCount)
                            messageTestComplete = true;
                    }
                }
                catch (InterruptedException e)
                {
                }

            assertTrue(args.packedCount == args.receivedCount());
            
            ArrayList<Boolean> results = args.comparisonResults();
            if (results != null)
            {
                int count = 0;
                for (Boolean b : results)
                {
                    ++count;
                    if (!b.booleanValue())
                        System.out.println("[testRunnerPacked] message #" + count + " comparison failed");
                    
                    assertTrue(b.booleanValue());
                }
                System.out.println("[testRunnerPacked] verified " + results.size()
                                   + " of " + args.packedCount + " received messages");
            }
            
            terminateServerAndClients(serverThread, server, clientThread, etajClient, clientChannel);

        }

        // If a server failure occurred, fail the test.
        if (server.errorMsg() != null)
            assertTrue(server.errorMsg(), false);
    }

}
