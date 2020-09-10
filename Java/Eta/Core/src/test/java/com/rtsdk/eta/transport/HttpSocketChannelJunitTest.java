package com.rtsdk.eta.transport;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.test.network.replay.NetworkReplay;
import com.rtsdk.eta.test.network.replay.NetworkReplayFactory;
import com.rtsdk.eta.test.network.replay.NetworkReplayUtil;

public class HttpSocketChannelJunitTest
{
    @BeforeClass
    public static void setRunningInJunits()
    {
    	ErrorImpl._runningInJunits = true;
    }

    @AfterClass
    public static void clearRunningInJunits()
    {
    	ErrorImpl._runningInJunits = false;
    }
    
	private static final int HTTP_HEADER6 = 6;
	private static final int CHUNKEND_SIZE = 2;

    /*
     * The default port NetworkReplay will listen on
     */
    private static final int DEFAULT_LISTEN_PORT = 4321;
    
    /*
     * The base directory name where the test data files are located
     */
    private static final String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/rtsdk/eta/transport/HttpSocketChannelJunit";

    /*
     * This file contains just the RIPC ConnectAck handshake message.
     */
    public static final String RIPC_CONNECT_ACK_HANDSHAKE_FILE = BASE_TEST_DATA_DIR_NAME + "/010http_just_ripc_handshake.txt";

    /*
     * Return the length of the first packed header, which included the RIPC
     * header(3) and the packed header(2).
     * 
     * @param channel
     * @return the length of the first packed header.
     */
    int firstPackedHeaderLength()
    {
        return Ripc.Lengths.HEADER + Ripc.Offsets.PACKED_MSG_DATA;
    }

    byte[] getBytesFromBuffer(TransportBuffer buffer)
    {
        // note: I ran some micro bench marks to compare the bulk-get,
        // relative-get and absolute-get (10K buffer with 10K copies).
        // Bulk-get was fastest by far. Relative-get was twice as fast
        // as absolute-get, but was still ~26x slower than the bulk-get.
        
        byte[] msg = new byte[buffer.data().limit()-buffer.data().position()];
        buffer.data().get(msg);
        
        return msg;
    }
    
    int compareAsString(TransportBuffer thisTransportBuf, TransportBuffer thatTransportBuf)
    {
        int len = 0, thisLen = 0, thatLen = 0;
        byte b1, b2;
        
        ByteBuffer thisBBuf = thisTransportBuf.data();
        ByteBuffer thatBBuf = thatTransportBuf.data();

        // Get the shorter length
        thisLen = thisBBuf.remaining();
        thatLen = thatBBuf.remaining();
        len = (thisLen < thatLen) ? thisLen : thatLen;

        for (int i = 0; i < len; i++)
        {
            b1 = thisBBuf.get(i + thisBBuf.position());
            b2 = thatBBuf.get(i + thatBBuf.position());
            if (b1 != b2)
            {
                return b1 - b2;
            }
        }

        if (thisLen == thatLen)
            return 0; // this is equal to that.
        else if (thisLen < thatLen)
        {
            if (thatBBuf.get(len + thatBBuf.position()) == 0)
                return 0; // extra null terminator. this is equal to that.
            return -1; // this is less than that
        }
        else
        {
            if (thisBBuf.get(len + thisBBuf.position()) == 0)
                return 0; // extra null terminator. this is equal to that.
            return 1; // this is greater than that\
        }
    }

    /*
     * GIVEN a file containing a single, complete message, 
     * WHEN we invoke RsslHttpSocketChannel.read(ByteBuffer)
     * and then invoke it again 
     * THEN the first call to read returns the expected message 
     * AND the first call to read returns exactly SUCCESS 
     * AND the second call to return returns null 
     * AND the second call to read returns WOULD_BLOCK 
     */
    @Test
    public void readSingleCompleteMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/020http_single_complete_message.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020http_single_complete_message.txt";
        
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // the messages we expect from calls to RsslHttpSocketChannel.read() (does not include RIPC headers)
            final byte[][] expectedMessages = parseExpectedMessages(expectedFile);


            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE), readArgs.readRetVal());
            
            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.bytesRead());
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK); // no more data
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }  

    /*
     * GIVEN a newly-connected socket channel 
     * WHEN the user invokes RsslHttpSocketChannel.read(ReadArgs, Error)
     * AND there is no data to be read from the network 
     * THEN RsslHttpSocketChannel.read(ReadArgs, Error) will return a null buffer 
     * AND TransportReturnCodes#READ_WOULD_BLOCK will be returned 
     */
    @Test
    public void readNoData()
    {
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // load the messages to replay
            
            replay = parseReplayFile(RIPC_CONNECT_ACK_HANDSHAKE_FILE);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            
            @SuppressWarnings("unused")
            TransportBuffer msgBuf;

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK);
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }  
    
    /*
     * GIVEN a single input file containing two (concatenated) messages such 
     * that they (should) be read into the same buffer when the 
     * RsslHttpSocketChannel reads from the network 
     * WHEN the user invokes 
     * RsslHttpSocketChannel.read(ReadArgs, Error) 
     * THEN two separate messages will be returned 
     */
    @Test
    public void twoMessagesSingleNetworkRead()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/030http_expected_two_msgs_single_net_read.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/030http_input_two_msgs_single_net_read.txt";
        
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // the messages we expect from calls to RsslHttpSocketChannel.read() (does not include RIPC headers)
            final byte[][] expectedMessages = parseExpectedMessages(expectedFile);

            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS); // still more data

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message
            
            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE + expectedMessages[3].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.bytesRead());
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.uncompressedBytesRead());
            
            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE)); // no more data
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[3], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(0, readArgs.bytesRead());
            assertEquals(expectedMessages[3].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.uncompressedBytesRead());
            
            // the next call should return null data, and a "RSSL_RET_READ_WOULD_BLOCK" return code
            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            // the return value should (exactly) equal SUCCESS because there is no more data to process
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK);
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /*
     * GIVEN an input file containing a single message that is 
     * broken into two parts, such that the first part contains
     * the RIPC header, and part of the message, and the 2nd
     * part contains the rest of the message 
     * WHEN the user invokes 
     * RsslHttpSocketChannel.read(ReadArgs, Error)
     * THEN the first invocation will return a null message 
     * and a value greater than SUCCESS 
     * AND the second invocation will return the expected message 
     * and a value exactly equal to success 
     * AND the third invocation will return READ_WOULD_BLOCK 
     */
    @Test
    public void twoPartKnownIncomplete()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/040http_expected_two_part_known_incomplete.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/040http_input_two_part_known_incomplete.txt";
        
        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0, cumulativeBytesRead = 0;
        
        try
        {
            initTransport(false); // initialize RSSL

            // the messages we expect from calls to RsslHttpSocketChannel.read() (does not include RIPC headers)
            final byte[][] expectedMessages = parseExpectedMessages(expectedFile);
            final byte[][] expectedInput = parseExpectedMessages(inputFile);

            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE));
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(395, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, cumulativeBytesRead);
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, cumulativeUncompressedBytesRead);
            
            // the next call should return null data, and a "RSSL_RET_READ_WOULD_BLOCK" return code
            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            // the return value should (exactly) equal SUCCESS because there is no more data to process
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK);
            assertEquals(0, readArgs.bytesRead());
            assertEquals(0, readArgs.uncompressedBytesRead());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }

    }
    
    /*
     * GIVEN an input file containing a single message that is
     * broken into two parts, such that the first part contains
     * just the first byte of the RIPC header, and the 2nd
     * part contains the rest of the message 
     * WHEN the user invokes
     * RsslHttpSocketChannel.read(ReadArgs, Error) 
     * THEN the first invocation will return a null message 
     * and a value greater than SUCCESS 
     * AND the second invocation will return the expected message 
     * and a value exactly equal to success 
     * AND the third invocation will return READ_WOULD_BLOCK 
     */    
    @Test
    public void twoPartUnknownIncomplete()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/050http_expected_two_part_unknown_incomplete.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/050http_input_two_part_unknown_incomplete.txt";
        
        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0, cumulativeBytesRead = 0;
        
        try
        {
            initTransport(false); // initialize RSSL

            // the messages we expect from calls to RsslSocketChannel.read() (does not include RIPC headers)
            final byte[][] expectedMessages = parseExpectedMessages(expectedFile);
            final byte[][] expectedInput = parseExpectedMessages(inputFile);

            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[2].length, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE));
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(395, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, cumulativeBytesRead);
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE, cumulativeUncompressedBytesRead);
            
            // the next call should return null data, and a "RSSL_RET_READ_WOULD_BLOCK" return code
            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            // the return value should (exactly) equal SUCCESS because there is no more data to process
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK);
            assertEquals(0, readArgs.bytesRead());
            assertEquals(0, readArgs.uncompressedBytesRead());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * GIVEN an input file containing a single ping message  
     * WHEN the user invokes 
     * RsslHttpSocketChannel.read(ReadArgs, Error) 
     * THEN the first invocation will return a null message 
     * and a value equal to READ_PING 
     * AND the second invocation will return READ_WOULD_BLOCK 
     */    
    @Test
    public void testPingMessage()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/http_ping_message.txt";
        
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            
            @SuppressWarnings("unused")
            TransportBuffer msgBuf;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_PING);

            // the next call should return null data, and a "RSSL_RET_READ_WOULD_BLOCK" return code
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            // the return value should (exactly) equal SUCCESS because there is no more data to process
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK);
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /*
     * GIVEN a transport initialized to use GLOBAL_AND_CHANNEL locking 
     * WHEN the user invokes 
     * RsslHttpSocketChannel.read(ReadArgs, Error)
     * AND another thread has already acquired a read lock 
     * THEN the first invocation will return a null message 
     * AND a value equal to READ_IN_PROGRESS 
     * AND the second invocation will return READ_WOULD_BLOCK 
     */    
    @Test
    public void testReadLockFailure()
    {
        NetworkReplay replay = null;

        try
        {
            //// initTransport(RsslLockingTypes.GLOBAL_AND_CHANNEL); // initialize RSSL
            initTransport(true); // initialize RSSL

            replay = parseReplayFile(RIPC_CONNECT_ACK_HANDSHAKE_FILE);
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);
            
            // create a mock read lock
            Lock readLock = Mockito.mock(Lock.class);
            // the first call to trylock() will fail, the second will succeed
            Mockito.stub(readLock.trylock()).toReturn(false).toReturn(true);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            consumerChannel.readLock(readLock);
            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            
            @SuppressWarnings("unused")
            TransportBuffer msgBuf;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_IN_PROGRESS);
            
            // the next call should return null data, and a "RSSL_RET_READ_WOULD_BLOCK" return code
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            // the return value should (exactly) equal SUCCESS because there is no more data to process
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK);
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /*
     * GIVEN a single input file containing a single complete message 
     * WHEN we force end of stream AFTER the first time the user invokes 
     * RsslHttpSocketChannel.read(ReadArgs, Error) 
     * THEN the first call will return the expected message 
     * AND the first call will return (exactly) RsslTransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE) 
     * AND after the first call the state of the channel will NOT be closed 
     * AND the second call will return null 
     * AND the second call will return FAILURE 
     * AND the state of the channel will be closed after the second call is complete 
     */
    @Test
    public void endOfStream()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/020http_single_complete_message.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020http_single_complete_message.txt";
        
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // the messages we expect from calls to RsslSocketChannel.read() (does not include RIPC headers)
            final byte[][] expectedMessages = parseExpectedMessages(expectedFile);

            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE), readArgs.readRetVal());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message
            
            assertTrue(consumerChannel.state() != ChannelState.CLOSED);
            
            replay.forceEndOfStream();
            
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.FAILURE); // no more data
            
            assertEquals(ChannelState.CLOSED, consumerChannel.state());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * GIVEN a file containing a single, complete packed message,
     * message that has only one part where the message format
     * is as follows: 
     *  
     * XXYZZAAAAAAAAAAAA
     *  
     * In the message above:
     * 
     *     XX is the two-byte RIPC message length (for the entire message)
     *     Y is the one-byte RIPC message flag
     *     ZZ is the two-byte length of the first (and only) packed message
     *     AAAAAAAAAAAA represents the first (and only) packed message
     * 
     * WHEN we invoke RsslHttpSocketChannel.read(ByteBuffer)
     * and then invoke it again 
     * THEN the first call to read returns the expected message 
     * AND the first call to read returns exactly SUCCESS 
     * AND the second call to return returns null 
     * AND the second call to read returns WOULD_BLOCK 
     */
    @Test
    public void readSingleComplete1PartPackedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/060http_expected_single_complete_1part_packed.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/060http_input_single_complete_1part_packed.txt";
        
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // the messages we expect from calls to RsslSocketChannel.read() (does not include RIPC headers)
            final byte[][] expectedMessages = parseExpectedMessages(expectedFile);

            // load the messages to replay
            replay = parseReplayFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplayHttpSocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS + (HTTP_HEADER6+CHUNKEND_SIZE), readArgs.readRetVal());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain transport headers, so add it for
             * the comparison.
             */
            int headerAndDataLen = firstPackedHeaderLength()+ expectedMessages[2].length;
            assertEquals(headerAndDataLen + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.bytesRead());
            assertEquals(headerAndDataLen + HTTP_HEADER6 + CHUNKEND_SIZE, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.READ_WOULD_BLOCK); // no more data
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }

            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /*
     * Returns an array of messages that we would expect to receive from a call
     * to RsslHttpSocketChannel.read(ReadArgs, Error). NOTE:
     * The RIPC headers will be stripped from these messages.
     * 
     * @param replayFile The replay file containing the messages
     * @return An array of messages that we would expect to receive from a call
     *         to RsslHttpSocketChannel.read(ReadArgs, Error)
     *         . NOTE: The RIPC headers will be stripped from these messages.
     * @throws IOException Thrown if an error occurs while parsing the replay
     *             file
     */
    private byte[][] parseExpectedMessages(String replayFile) throws IOException
    {
        assertTrue(replayFile != null);

        // these are the messages we expect (without the RIPC headers)
        final byte[][] expectedMessages = NetworkReplayUtil.parseAndStripHttpAndRipcHeaders(replayFile);
        assertTrue(expectedMessages != null && expectedMessages.length > 0);

        return expectedMessages;
    }

    /*
     * Invokes RsslTransport.initialize()
     * 
     * @throws IOException
     */
    private void initTransport(boolean globalLocking) throws IOException
    {
        final Error error = TransportFactory.createError();

        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(globalLocking);
        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
        {
            throw new IOException("RsslTransport.initialize() failed: " + error.text());
        }
    }
    
    /*
     * Connect the provided channel (to the NetworkReplay)
     * @param portNumber The port number the network replay is listening on
     * @param consumerChannel The channel to connect
     */
    private void connectChannel(RsslSocketChannel consumerChannel, Integer portNumber)
    {
        assertTrue(consumerChannel != null);

        final ConnectOptions opts = buildConnectOptions("localhost", portNumber.toString());
        final Error error = TransportFactory.createError();

        consumerChannel._scktChannel = new SocketHelper();
        assertEquals(TransportReturnCodes.SUCCESS, consumerChannel.connect(opts, error));
        assertTrue(consumerChannel.state() == ChannelState.INITIALIZING);
    }

    /*
     * Creates a new instance of a RsslHttpSocketChannel and overrides its
     * RsslHttpSocketChannel.read(ByteBuffer) method
     * 
     * @param replay
     * @return new instance of a RsslHttpSocketChannel
     */
    private RsslSocketChannel createReplayHttpSocketChannel(final NetworkReplay replay)
    {
        assertTrue(replay != null);

        // allocate a channel and override it's read() to read from our NetworkReplay
        // NOTE: a RsslHttpSocketChannel is *normally* constructed by via its factory
        RsslHttpSocketChannel consumerChannel = new RsslHttpSocketChannel()
        {
            @Override
            protected int read(ByteBuffer dst) throws NotYetConnectedException, IOException
            {
                return replay.read(dst);
            }
            @Override
            protected int initChnlReadFromChannel(ByteBuffer dst, Error error) throws IOException
            {
                return replay.read(dst);
            }
        };
        
        assertTrue(consumerChannel.connectionType() == ConnectionTypes.HTTP);

        return consumerChannel;
    }

    /*
     * Waits for the specified channel to become active
     * 
     * @param channel
     * @throws IOException
     */
    private void waitForChannelActive(final RsslSocketChannel channel) throws IOException
    {
        assertTrue(channel != null);

        final int MAX_ATTEMPTS = 10;

        final Error error = TransportFactory.createError();
        final InProgInfo inProg = TransportFactory.createInProgInfo();
        int retval;

        int attempts = 0;
        // Wait for channel to become active. This finalizes the three-way
        // handshake.
        while (channel.state() != ChannelState.ACTIVE && attempts < MAX_ATTEMPTS)
        {
            if (channel.state() == ChannelState.INITIALIZING)
            {
                if ((retval = channel.init(inProg, error)) < TransportReturnCodes.SUCCESS)
                {
                    String initFail = "\nChannel " + channel.selectableChannel() + " inactive: "
                            + error.text();
                    System.err.println(initFail);
                    throw new IOException(initFail);
                }
                else
                {
                    switch (retval)
                    {
                        case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            System.out.println("\nChannel " + channel.selectableChannel()
                                    + " In Progress...");
                            break;
                        case TransportReturnCodes.SUCCESS:
                            System.out.println("\nChannel " + channel.selectableChannel()
                                    + " Is Active");
                            break;
                        default:
                            String badRet = "\nBad return value channel="
                                    + channel.selectableChannel() + " <" + error.text() + ">";
                            System.out.println(badRet);
                            throw new IOException(badRet);
                    }
                }
            }

            try
            {
                Thread.sleep(1000);
            }
            catch (Exception e)
            {
                throw new IOException("Thread.sleep() exception: " + e.getLocalizedMessage());
            }

            ++attempts;
        }

        if (channel.state() != ChannelState.ACTIVE)
        {
            throw new IOException("channel never became active");
        }
    }

    /*
     * Parses a network replay file and returns a populated NetworkReplay
     * 
     * @param fileName The name of the network replay file to parse
     * @return A populated NetworkReplay
     * @throws IOException Thrown if the file could not be parsed
     */
    private NetworkReplay parseReplayFile(String fileName) throws IOException
    {
        NetworkReplay fileReplay = NetworkReplayFactory.create();
        fileReplay.parseFile(fileName);

        return fileReplay;
    }

    /*
     * Returns a ConnectOptions for the specified address
     * 
     * @param hostname The hostname or IP address to connect to
     * @param portno The port number to connect to
     * @return A ConnectOptions for the specified address
     */
    private ConnectOptions buildConnectOptions(String hostname, String port)
    {
        ConnectOptions copts = TransportFactory.createConnectOptions();
        copts.connectionType(ConnectionTypes.HTTP);

        copts.guaranteedOutputBuffers(500);
        copts.unifiedNetworkInfo().address(hostname);
        copts.unifiedNetworkInfo().serviceName(port);
        copts.majorVersion(Codec.majorVersion());
        copts.minorVersion(Codec.minorVersion());
        copts.protocolType(Codec.protocolType());

        return copts;
    }

    /*
     * Prints the specified message to standard output
     * @param msg The message to print (may be null)
     * @param channel A channel (implements the utility method used for printing)
     */
    private void printMessage(final byte[] msg, RsslSocketChannel channel)
    {
        assertTrue(channel != null);
        
        System.out.println("-- begin --");
        if (msg != null)
        {
            ByteBuffer temp = ByteBuffer.wrap(msg);
            System.out.println(Transport.toHexString(temp, 0, temp.limit()));            
        }
        System.out.println("-- end --");
    }
}
