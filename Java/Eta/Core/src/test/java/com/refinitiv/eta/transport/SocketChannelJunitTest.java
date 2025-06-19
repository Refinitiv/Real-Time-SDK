/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.lang.reflect.Field;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import static org.mockito.Mockito.*;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.ParseHexFile;
import com.refinitiv.eta.test.network.replay.NetworkReplay;
import com.refinitiv.eta.test.network.replay.NetworkReplayFactory;
import com.refinitiv.eta.test.network.replay.NetworkReplayUtil;

public class SocketChannelJunitTest
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

    final static int TIMEOUTMS = 10000; // 10 seconds.

    /*
     * The default port NetworkReplay will listen on
     */
    private static final int DEFAULT_LISTEN_PORT = 4323;
    final static String DEFAULT_LISTEN_PORT_AS_STRING = Integer.toString(DEFAULT_LISTEN_PORT);

    /*
     * The base directory name where the test data files are located
     */
    private static final String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/refinitiv/eta/transport/SocketChannelJunit";

    /*
     * This file contains just the RIPC ConnectAck handshake message.
     */
    public static final String RIPC_CONNECT_ACK_HANDSHAKE_FILE = BASE_TEST_DATA_DIR_NAME + "/010_just_ripc_handshake.txt";

    /*
     * This file contains just the RIPC ConnectReq handshake message.
     */
    public static final String RIPC_CONNECT_REQ_HANDSHAKE_FILE = "src/test/resources/com/refinitiv/eta/transport/RipcHandshakeJunit/60_input_connectReq_Ripc13.txt";

    // for blocking client/server test
    private volatile boolean _clientReady = false;
    private volatile boolean _doReadWrite = false;
    private volatile boolean _serverReady = false;

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

    /*
     * Return the length of the additional packed header, which included the
     * packed header(2).
     *
     * @param channel
     * @return the length of the additional packed header.
     */
    int additionalPackedHeaderLength()
    {
        return Ripc.Offsets.PACKED_MSG_DATA;
    }

    /*
     * Return the length of the first fragment header, which included the RIPC
     * header (3), Extended Flags (1), total message length (4), and the
     * fragmentId(2(RIPC13)|1(RIPC12)).
     *
     * @param channel
     * @return the length of the first fragment header.
     */
    int firstFragmentHeaderLength(RsslSocketChannel channel)
    {
        return 8 + channel._readBufStateMachine._fragIdLen;
    }

    /*
     * Return the length of the additional fragment header, which included the
     * RIPC header (3), Extended Flags (1), and the
     * fragmentId(2(RIPC13)|1(RIPC12)).
     *
     * @param channel
     * @return the length of the additional fragment header.
     */
    int additionalFragmentHeaderLength(RsslSocketChannel channel)
    {
        return 4 + channel._readBufStateMachine._fragIdLen;
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
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * and then invoke it again
     * THEN the first call to read returns the expected message
     * AND the first call to read returns exactly SUCCESS
     * AND the second call to return returns null
     * AND the second call to read returns WOULD_BLOCK
     */
    @Test
    public void readSingleCompleteMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

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
     * WHEN the user invokes RsslSocketChannel.read(ReadArgs, Error)
     * AND there is no data to be read from the network
     * THEN RsslSocketChannel.read(ReadArgs, Error) will return a {@code null} buffer
     * AND TransportReturnCodes.READ_WOULD_BLOCK will be returned
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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

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
     * GIVEN a newly-connected socket channel
     * WHEN the user invokes RsslSocketChannel.read(ReadArgs, Error)
     * AND the socket channel (internally) encounters a failure resulting in an IOException
     * THEN RsslSocketChannel.read(ReadArgs, Error) will return a {@code null} buffer
     * AND TransportReturnCodes.FAILURE will be returned
     */
    @Test
    public void ioExceptionHandling()
    {
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // load the messages to replay
            replay = parseReplayFile(RIPC_CONNECT_ACK_HANDSHAKE_FILE);

            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();

            // force an IOException to be thrown when the RsslSocketChannel
            //(internally) attempts to read from the network
            replay.forceNextReadToFail("this will cause the channel read to fail");

            @SuppressWarnings("unused")
            TransportBuffer msgBuf;

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.FAILURE);
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
     * RsslSocketChannel reads from the network
     * WHEN the user invokes
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN two separate messages will be returned
     */
    @Test
    public void twoMessagesSingleNetworkRead()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/030_expected_two_msgs_single_net_read.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/030_input_two_msgs_single_net_read.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

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
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER + expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.SUCCESS); // no more data
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(0, readArgs.bytesRead());
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

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
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN the first invocation will return a {@code null} message
     * and a value greater than SUCCESS
     * AND the second invocation will return the expected message
     * and a value exactly equal to success
     * AND the third invocation will return READ_WOULD_BLOCK
     */
    @Test
    public void twoPartKnownIncomplete()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/040_expected_two_part_known_incomplete.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/040_input_two_part_known_incomplete.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(385, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, cumulativeBytesRead);
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, cumulativeUncompressedBytesRead);

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
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN the first invocation will return a {@code null} message
     * and a value greater than SUCCESS
     * AND the second invocation will return the expected message
     * and a value exactly equal to success
     * AND the third invocation will return READ_WOULD_BLOCK
     */
    @Test
    public void twoPartUnknownIncomplete()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/050_expected_two_part_unknown_incomplete.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/050_input_two_part_unknown_incomplete.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[1].length, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() == TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(385, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, cumulativeBytesRead);
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, cumulativeUncompressedBytesRead);

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
     * This test reproduces an MR (upaj0243) where we received an
     * "unknown message class" error during decoding. In this
     * scenario, a single call to read() read a complete message,
     * plus the first two bytes of the next message. An off-by-one
     * error (<= instead of <) caused us to also read an additional
     * byte containing junk data (left over from previously read data)
     * as the RIPC flag. This in-turn caused the state machine to
     * incorrectly interpret the message as being "packed".
     *
     * The input file contains:<br/>
     * a) The RIPC handshake<br/>
     * b) The first message, with a total length of 34 bytes, plus
     * the first two bytes of the second message, with a total
     * length of 10 bytes.<br/>
     * c) The remaining 8 bytes of the second message
     */
    @Test
    public void unknownIncompleteUpaj0243()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/051_expected_unknown_incomplete.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/051_input_unknown_incomplete.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // We need to simulate a buffer that is "dirty", with previously-
            // read data. We will use reflection to gain access to the private
            // read IO buffer, and initialize it with a value '12' that indicates
            // a packed message.
            Field privateReadIoBuffer = RsslSocketChannel.class.getDeclaredField("_readIoBuffer");
            privateReadIoBuffer.setAccessible(true);
            ByteBufferPair _readIoBuffer = (ByteBufferPair) privateReadIoBuffer.get(consumerChannel);
            byte packedFlag = 18; // 12 in hex
            for (int i = 0; i < _readIoBuffer.buffer().limit(); i++)
            {
                _readIoBuffer.buffer().put(packedFlag);
            }
            _readIoBuffer.buffer().rewind(); // get read for "regular" reads

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER + 2, readArgs.bytesRead());
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());


            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER - 2, readArgs.bytesRead());
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

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
        catch (NoSuchFieldException e)
        {
            fail(e.getLocalizedMessage());
        }
        catch (SecurityException e)
        {
            fail(e.getLocalizedMessage());
        }
        catch (IllegalArgumentException e)
        {
            fail(e.getLocalizedMessage());
        }
        catch (IllegalAccessException e)
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
     * GIVEN an input file that tests the KNOWN_INSUFFICIENT state:
     *
     * (Note: when the RsslSocketChannel is in the KNOWN_INSUFFICIENT
     * state, it knows the length of the "next" message, but there is
     * insufficient space in the read buffer to accommodate it.
     *
     * For this unit test, we will override the capacity of the
     * RsslSocketChannel read buffer to be 35 bytes.
     * The input file will contain two complete messages that, combined,
     * will exceeded the length of the input buffer.
     * Specifically, the first message we receive will have a total
     * length of 32 bytes. The second message we receive will
     * have a total length of 10 bytes. When the RsslSocketChannel
     * (internally) performs a network read, the entire first message, plus
     * the first three bytes of the second message will fill the entire read buffer.
     *
     * After the first message is processed (by the application), the state
     * machine will transition from the KNOWN_COMPLETE state to the
     * KNOWN_INSUFFICENT state. This will require the buffer to be compacted
     * before the rest of the second message can be read by the
     * RsslSocketChannel.)
     *
     * WHEN the user invokes
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN the first invocation will return the expected message
     * and a value greater than SUCCESS
     * AND the second invocation will return the expected message
     * and a value exactly equal to success
     * AND the third invocation will return READ_WOULD_BLOCK
     */
    @Test
    public void knownInsufficient()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/055_expected_known_insufficient.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/055_input_known_insufficient.txt";
        final int readBufferCapacity = 35; // bytes
        knownInsufficient(expectedFile, inputFile, readBufferCapacity);
    }

    /*
     * This is essentially the same as the knownInsufficient() test
     * case, but the readBufferCapacity and input to NetworkReplay has been
     * adjusted so that the initial read leaves a single byte free in the buffer.
     * This test ensures that the extra byte is not copied when we compact()
     * the ByteBuffer.
     */
    @Test
    public void knownInsufficientRemainingBuf()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/056_expected_known_insufficient_remain_buf.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/056_input_known_insufficient_remain_buf.txt";
        final int readBufferCapacity = 36; // bytes

        knownInsufficient(expectedFile, inputFile, readBufferCapacity);
    }

    /* general knownInsufficient test */
    private void knownInsufficient(String expectedFile, String inputFile, final int readBufferCapacity)
    {

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);
            consumerChannel.overrideReadBufferCapacity(readBufferCapacity);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            if (readBufferCapacity == 35)
            {
                assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER + expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            }
            else
            {
                assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER + 3, readArgs.bytesRead());
            }
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            if (readBufferCapacity == 35)
            {
                assertEquals(0, readArgs.bytesRead());
            }
            else
            {
                assertEquals(7, readArgs.bytesRead());
            }
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

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
     * GIVEN an input file that tests the UNKNOWN_INSUFFICIENT state:
     *
     * (Note: when the RsslSocketChannel is in the UNKNOWN_INSUFFICIENT
     * state, it dosn't know the length of the "next" message, and there is
     * insufficient space in the read buffer to accommodate the rest of the
     * RIPC header.
     *
     * For this unit test, we will override the capacity of the
     * RsslSocketChannel buffer to be 35 bytes. The input file will
     * contain two complete messages that, combined, will exceeded the length
     * of the input buffer. Specifically, the first message we receive will
     * have a total length of 34 bytes. The second message we receive will
     * have a total length of 10 bytes. When the RsslSocketChannel
     * (internally) performs a network read, the entire first message, plus
     * the first byte of the second message will fill the entire read buffer.
     *
     * After the first message is processed (by the application), the state
     * machine will transition from the KNOWN_COMPLETE state to the
     * UNKNOWN_INSUFFICENT state. This will require the buffer to be compacted
     * before the rest of the second message can be read.
     *
     * WHEN the user invokes
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN the first invocation will return the expected message
     * and a value greater than SUCCESS
     * AND the second invocation will return the expected message
     * and a value exactly equal to success
     * AND the third invocation will return READ_WOULD_BLOCK
     */
    @Test
    public void unknownInsufficient()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/058_expected_unknown_insufficient.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/058_input_unknown_insufficient.txt";
        final int readBufferCapacity = 35; // bytes

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);
            consumerChannel.overrideReadBufferCapacity(readBufferCapacity);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null); // we have a partial message
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER + expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            assertEquals(expectedMessages[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());


            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            assertEquals(0, readArgs.bytesRead());
            assertEquals(expectedMessages[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());

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
     * GIVEN an input file containing a single ping message
     * WHEN the user invokes
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN the first invocation will return a {@code null} message
     * and a value equal to READ_PING
     * AND the second invocation will return  READ_WOULD_BLOCK
     */
    @Test
    public void testPingMessage()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/ping_message.txt";

        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // load the messages to replay
            replay = parseReplayFile(inputFile);

            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

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
     * RsslSocketChannel.read(ReadArgs, Error)
     * AND another thread has already acquired a read lock
     * THEN the first invocation will return a {@code null} message
     * AND a value equal to READ_IN_PROGRESS
     * AND the second invocation will return  READ_WOULD_BLOCK
     */
    @Test
    public void testReadLockFailure()
    {
        NetworkReplay replay = null;

        try
        {
            initTransport(true); // initialize RSSL

            replay = parseReplayFile(RIPC_CONNECT_ACK_HANDSHAKE_FILE);
            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            // create a mock read lock
            Lock readLock = Mockito.mock(Lock.class);
            // the first call to trylock() will fail, the second will succeed
            when(readLock.trylock()).thenReturn(false).thenReturn(true);

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
     * RsslSocketChannel.read(ReadArgs, Error)
     * THEN the first call will return the expected message
     * AND the first call will return (exactly) RsslTransportReturnCodes.SUCCESS
     * AND after the first call the state of the channel will NOT be closed
     * AND the second call will return null
     * AND the second call will return FAILURE
     * AND the state of the channel will be closed after the second call is complete
     */
    @Test
    public void endOfStream()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

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
     * {@code XXYZZAAAAAAAAAAAA}
     *
     * In the message above:
     * <ul>
     *     <li>{@code XX} is the two-byte RIPC message length (for the entire message)</li>
     *     <li>{@code Y} is the one-byte RIPC message flag</li>
     *     <li>{@code ZZ} is the two-byte length of the first (and only) packed message</li>
     *     <li>{@code AAAAAAAAAAAA} represents the first (and only) packed message</li>
     * </ul>
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * and then invoke it again
     * THEN the first call to read returns the expected message
     * AND the first call to read returns exactly SUCCESS
     * AND the second call to return returns null
     * AND the second call to read returns WOULD_BLOCK
     */
    @Test
    public void readSingleComplete1PartPackedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/060_expected_single_complete_1part_packed.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/060_input_single_complete_1part_packed.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain transport headers, so add it for
             * the comparison.
             */
            int headerAndDataLen = firstPackedHeaderLength()+ expectedMessages[1].length;
            assertEquals(headerAndDataLen, readArgs.bytesRead());
            assertEquals(headerAndDataLen, readArgs.uncompressedBytesRead());

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
     *
     * GIVEN a file containing and a single (complete) packed message,
     * such that the packed message has only one part, and the length
     * of said part is zero bytes. The message format is as follows:
     *
     * {@code XXYZZ}
     *
     * In the message above:
     * <ul>
     *     <li>{@code XX} is the two-byte RIPC message length (for the entire message)</li>
     *     <li>{@code Y} is the one-byte RIPC message flag</li>
     *     <li>{@code ZZ} is the two-byte length of the first (and only) packed message</li>
     * </ul>
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * and then invoke it again
     * THEN the first call to read returns a message with a length of zero
     * AND the first call to read returns exactly SUCCESS
     * AND the second call to return returns null
     * AND the second call to read returns WOULD_BLOCK
     */
    @Test
    public void readSinglePackedEmptyPayload()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/070_single_packed_with_empty_payload.txt";

        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // load the messages to replay
            replay = parseReplayFile(inputFile);

            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            @SuppressWarnings("unused")
            TransportBuffer msgBuf;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

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
     * GIVEN a file containing and a single (complete) packed message,
     * where the packed message has two parts. The message format is as follows:
     *
     * {@code XXYZZAAAAAAAAAAAANNBBBBBBBBBBBB}
     *
     * In the message above:
     * <ul>
     *     <li>{@code XX} is the two-byte RIPC message length (for the entire message)</li>
     *     <li>{@code Y} is the one-byte RIPC message flag</li>
     *     <li>{@code ZZ} is the two-byte length of the first packed message</li>
     *     <li>{@code AAAAAAAAAAAA} represents the first packed message</li>
     *     <li>{@code NN} is the two-byte length of the second packed message</li>
     *     <li>{@code BBBBBBBBBBBB} represents the second packed message</li>
     * </ul>
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns 1st packed message
     * AND the first call to read returns a value greater than SUCCESS
     * AND the second call to return returns the second packed message
     * AND the second call to read returns a value exactly equal to SUCCESS
     * AND the third call to return returns null
     * AND the third call to read returns WOULD_BLOCK
     */
    @Test
    public void readSingleComplete2PartPackedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/080_expected_single_complete_2part_packed.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/080_input_single_complete_2part_packed.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain transport headers, so add it for
             * the comparison.
             */
            int headerAndDataLen = firstPackedHeaderLength() + expectedMessages[1].length +
                                   additionalPackedHeaderLength()+ expectedMessages[2].length;
            assertEquals(headerAndDataLen, readArgs.bytesRead());
            assertEquals(headerAndDataLen, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain transport headers, so add it for
             * the comparison. This is an additional packed message.
             */
            assertEquals(0, readArgs.bytesRead());
            assertEquals(0, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
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
     * GIVEN a file containing the RIPC handshake, and a single (complete)
     * packed message, where the packed message has three parts. The last
     * part is "empty" (it has a length of zero.) The message format is
     * as follows:
     *
     * {@code XXYZZAAAAAAAAAAAANNBBBBBBBBBBBBOO}
     *
     * In the message above:
     * <ul>
     *     <li>{@code XX} is the two-byte RIPC message length (for the entire message)</li>
     *     <li>{@code Y} is the one-byte RIPC message flag</li>
     *     <li>{@code ZZ} is the two-byte length of the first packed message</li>
     *     <li>{@code AAAAAAAAAAAA} represents the first packed message</li>
     *     <li>{@code NN} is the two-byte length of the second packed message</li>
     *     <li>{@code BBBBBBBBBBBB} represents the second packed message</li>
     *     <li>{@code OO} is the two-byte length of the third packed message</li>
     * </ul>
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns 1st packed message
     * AND the first call to read returns a value greater than SUCCESS
     * AND the second call to return returns the second packed message
     * AND the second call to read returns a value greater than SUCCESS
     * AND the third call to return returns null
     * AND the third call to read returns WOULD_BLOCK
     */
    @Test
    public void read3PartLastPartEmptyPackedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/090_expected_3part_packed_last_part_empty.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/090_input_3part_packed_last_part_empty.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain transport headers, so add it for
             * the comparison.
             */
            int headerAndDataLen = firstPackedHeaderLength()+ expectedMessages[1].length +
                                   additionalPackedHeaderLength()+ expectedMessages[2].length + additionalPackedHeaderLength();
            assertEquals(headerAndDataLen, readArgs.bytesRead());
            assertEquals(headerAndDataLen, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain transport headers, so add it for
             * the comparison. This is an additional packed message.
             */
            assertEquals(0, readArgs.bytesRead());
            assertEquals(0, readArgs.uncompressedBytesRead());

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * The last packed message had a length of zero, so just account
             * for the Packed Header.
             */
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
     * GIVEN a file containing the RIPC handshake, and a fragmented message
     * consisting of one "fragment header" message, and one "fragment"
     * message where:
     *
     * The format of the message containing the fragment header is as follows:
     * {@code AABCDDDDEFFFFFFFFFF}
     *
     * In the message above:
     * <ul>
     *     <li>{@code AA} is the two-byte RIPC message length (for the entire message)</li>
     *     <li>{@code B} is the one-byte RIPC message flag</li>
     *     <li>{@code C} is the one-byte extended flag</li>
     *     <li>{@code DDDD} is the 4 byte total length of the fragmented message data (does not include RIPC headers)</li>
     *     <li>{@code E} is the one-byte fragment ID</li>
     *     <li>{@code FFFFFFF} is the data</li>
     * </ul>
     *
     * The format of the message containing the fragment is as follows:
     * {@code AABCDEFFF}
     *
     * In the message above:
     * <ul>
     *     <li>{@code AA} is the two-byte RIPC message length (for the entire message)</li>
     *     <li>{@code B} is the one-byte RIPC message flag</li>
     *     <li>{@code C} is the one-byte extended flag</li>
     *     <li>{@code D} is the one-byte fragment id</li>
     *     <li>{@code EEE} is the data</li>
     * </ul>
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value equal to SUCCESS
     * AND the second call to read returns the (reassembled) fragmented message
     * AND the second call to read returns a value equal to SUCCESS
     * AND the third call to return returns null
     * AND the third call to read returns WOULD_BLOCK
     */
    @Test
    public void fragmentedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/100_expected_fragmented.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/100_input_fragmented.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + additionalFragmentHeaderLength(consumerChannel) + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeBytesRead);
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
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
     * GIVEN an input file containing the RIPC handshake, and a fragmented
     * message consisting of one "fragment header" message, and one "fragment"
     * message, where BOTH the fragment header message and the fragment message
     * will be read from the network in a single call tojava.nio.channels.SocketChannel.read()
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value greater than SUCCESS
     * AND the second call to read returns the (reassembled) fragmented message
     * AND the second call to read returns a value equal to SUCCESS
     * AND the third call to return returns null
     * AND the third call to read returns WOULD_BLOCK
     */
    @Test
    public void fragmentedHeaderAndFragInSingleReadIO()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/105_expected_fragment_header+frag_single_read.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/105_input_fragment_header+frag_single_read.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertTrue(readArgs.readRetVal() > TransportReturnCodes.SUCCESS);
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(17, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(9, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + additionalFragmentHeaderLength(consumerChannel) + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
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
     * GIVEN an input file containing the RIPC handshake, and two fragmented
     * messages such that the smaller one follows the larger one. (The
     * purpose of this test is to exercise the code related to pooling
     * fragment buffers. Since the first message is larger than the second
     * one, we should be able to re-use the same fragment buffer allocated
     * for the first message.)
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value equal to SUCCESS
     * AND the second call to read returns the (reassembled) first fragmented message
     * AND the second call to read returns a value equal to SUCCESS
     * THEN the third call to read returns null
     * AND the third call to read returns a value equal to SUCCESS
     * THEN the fourth call returns the (reassembled) first fragmented message
     * AND the fourth call to read returns a value equal to SUCCESS
     * AND the fifth call to return returns null
     * AND the fifth call to read returns WOULD_BLOCK
     */
    @Test
    public void smallerFragFollowsLarger()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/110_expected_smaller_frag_follows_larger.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/110_input_smaller_frag_follows_larger.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            // compare the first (larger) message
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + additionalFragmentHeaderLength(consumerChannel) + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeBytesRead);
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);

            readArgs.clear();
            cumulativeUncompressedBytesRead = cumulativeBytesRead = 0;
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[4].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[4].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            // compare the second (smaller) message
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             */
            headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                  + additionalFragmentHeaderLength(consumerChannel) + expectedMessages[2].length;
            assertEquals(headerAndDataLength, cumulativeBytesRead);
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);

            readArgs.clear();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
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
     * GIVEN an input file containing the RIPC handshake, an "out-of-sequence"
     * fragment such that we receive the fragment, but we never received its
     * corresponding fragment header, and a "normal" RIPC message. The purpose
     * of this test is to verify we can recover from the scenario where we
     * receive an out-of-sequence fragment.
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value equal to SUCCESS
     * AND the second call to read returns the regular message
     * AND the second call to read returns a value equal to SUCCESS
     * AND the third call to return returns null
     * AND the third call to read returns WOULD_BLOCK
     */
    @Test
    public void outOfSequenceFragment()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/115_expected_frag_out_of_sequence.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/115_input_frag_out_of_sequence.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            final byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
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
     * GIVEN an input file containing the RIPC handshake, a fragment header
     * with a "total message length" (data length) that exceeds Java's MAX_INT,
     * a normal RIPC message, a fragment associated with the fragment
     * header, and finally, another normal message.
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value equal to SUCCESS
     * AND the second call to read returns the first normal message
     * AND the second call to read returns a value equal to SUCCESS
     * AND the third call to read returns null
     * AND the third call to read returns a value equal to SUCCESS
     * AND the fourth call to read returns the second normal message
     * AND the fourth call to read returns a value equal to SUCCESS
     * AND the fifth call to return returns null
     * AND the fifth call to read returns WOULD_BLOCK
     */
    @Test
    public void fragmentExceedesMaxInt()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/120_expected_frag_max_int.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/120_input_frag_max_int.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            // this message has a "total message length" that exceeds MAX_INT
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

            // the first normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // this message is a fragment associated with the above fragment header
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

            // the second normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            // read should block
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
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
     * GIVEN an input file containing the RIPC handshake, a fragment header,
     * a normal RIPC message, a second fragment header with the
     * same ID as the first fragment header, and a fragment.
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value equal to SUCCESS
     * AND the second call to read returns the normal message
     * AND the second call to read returns a value equal to SUCCESS
     * AND the third call to read returns null
     * AND the third call to read returns a value equal to SUCCESS
     * AND the fourth call to read returns the reassembled fragmented message
     * AND the fourth call to read returns a value equal to SUCCESS
     * AND the fifth call to return returns null
     * AND the fifth call to read returns WOULD_BLOCK
     */
    @Test
    public void duplicateFragmentHeader()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/125_expected_dupe_frag_headers.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/125_input_dupe_frag_headers.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            // this call reads the first fragment header
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

            // the first normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // read the second fragment header (with the duplicate fragment id)
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());

            // read the fragment (associated with the second fragment header)
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            // read should block
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
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
     * GIVEN an input file containing the RIPC handshake, a fragment header,
     * a normal RIPC message, a second fragment header of the same size with
     * a different frag ID as the first fragment header, and one fragment
     * for each fragID.
     *
     * WHEN we invoke RsslSocketChannel.read(ByteBuffer)
     * several times
     * THEN the first call to read returns null
     * AND the first call to read returns a value equal to SUCCESS
     * AND the second call to read returns the normal message
     * AND the second call to read returns a value equal to SUCCESS
     * AND the third call to read returns null
     * AND the third call to read returns a value equal to SUCCESS
     * AND the fourth call to read returns the first reassembled fragmented message
     * AND the fourth call to read returns a value equal to SUCCESS
     * AND the fifth call to return returns the second reassembled fragmented message
     * AND the fifth call to read returns a value equal to SUCCESS
     * AND the sixth call to return returns null
     * AND the sixth call to read returns WOULD_BLOCK
     */
    @Test
    public void twoFragmentedMessagesOfSameSizeAtSameTime()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/130_expected_two_frag_msgs_same_size_diff_ids.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/130_input_two_frag_msgs_same_size_diff_ids.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            // this call reads the first fragment header
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            // the first normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // read the second fragment header (same size as first fragment)
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            // read the first fragment (associated with the first fragment header)
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[4].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[4].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            // read the second fragment (associated with the second fragment header)
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[5].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[5].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[3], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             * Expect two fragment headers, two additional fragments and one normal msg.
             */
            int headerAndDataLength = (firstFragmentHeaderLength(consumerChannel) * 2)
                                      + (additionalFragmentHeaderLength(consumerChannel) * 2) + Ripc.Lengths.HEADER
                                      + expectedMessages[1].length + expectedMessages[2].length
                                      + expectedMessages[3].length;
            assertEquals(headerAndDataLength, cumulativeBytesRead);
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);

            // read should block
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
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

    @Test
    public void normalMessageFollowsFragmentedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/140_expected_normal_msg_follow_frag_msg.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/140_input_normal_msg_follow_frag_msg.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read from the channel
            // the first normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // this call reads the first fragment header
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            // the second normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[3].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            // read the first fragment (associated with the first fragment header)
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[4].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[4].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[3], msg); //first array element is initial RIPC message

            // the third normal RIPC message
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(expectedInput[5].length + Ripc.Lengths.HEADER, readArgs.bytesRead());
            cumulativeBytesRead += readArgs.bytesRead();
            assertEquals(expectedInput[5].length + Ripc.Lengths.HEADER, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[4], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             * Expect one fragment header, one additional fragments and three normal msgs.
             */
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + additionalFragmentHeaderLength(consumerChannel) + (Ripc.Lengths.HEADER * 3)
                                      + expectedMessages[1].length + expectedMessages[2].length
                                      + expectedMessages[3].length + expectedMessages[4].length;
            assertEquals(headerAndDataLength, cumulativeBytesRead);
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);

            // read should block
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal()); // no more data
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
     * Returns an array of messages that we would expect to receive from a call
     * to RsslSocketChannel.read(ReadArgs, Error). NOTE:
     * The RIPC headers will be stripped from these messages.
     *
     * @param replayFile The replay file containing the messages
     * @return An array of messages that we would expect to receive from a call
     *         to RsslSocketChannel.read(ReadArgs, Error)
     *         . NOTE: The RIPC headers will be stripped from these messages.
     * @throws IOException Thrown if an error occurs while parsing the replay
     *             file
     */
    private byte[][] parseExpectedMessages(String replayFile) throws IOException
    {
        assertTrue(replayFile != null);

        // these are the messages we expect (without the RIPC headers)
        final byte[][] expectedMessages = NetworkReplayUtil.parseAndStripRipcHeaders(replayFile);
        assertTrue(expectedMessages != null && expectedMessages.length > 0);

        return expectedMessages;
    }

    /*
     * Invokes RsslTransport.initialize(
     *
     * @throws IOException
     */
    static void initTransport(boolean globalLocking) throws IOException
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
     * Connect the provided channel (to the {@code NetworkReplay})
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
     * Creates a new instance of a RsslSocketChannel and overrides its
     * RsslSocketChannel.read(ByteBuffer) method
     *
     * @param replay
     * @return
     */
    private RsslSocketChannel createReplaySocketChannel(final NetworkReplay replay)
    {
        assertTrue(replay != null);

        // allocate a channel and override it's read() to read from our
        // NetworkReplay
        // NOTE: a RsslSocketChannel is *normally* constructed by via its
        // factory
        RsslSocketChannel consumerChannel = new RsslSocketChannel()
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

        assertTrue(consumerChannel.connectionType() == ConnectionTypes.SOCKET);

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
        // Wait for channel to become active. This finalizes the three-way handshake.
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
     * @param msg The message to print (may be {@code null})
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

    ////// START basic write(), flush() and ping() tests //////
    /*
     * 1. Call write method with
     *
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns all bytes sent
     *
     * Expected Result:
     *
     * Buffer directly written to network and write method returns 0.
     */
    @Test
    public void basicWFPTestCase1()
    {
        String testData = "basicWFPTestCase1";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.position();
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();


        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }
                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;
            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen+3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            //java.nio.channels.SocketChannel.write() returns all bytes sent
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)(bufLen + 3));

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // write call should return success
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertEquals(TransportReturnCodes.SUCCESS, rsslChnl.write(transBuf, writeArgs, error));
            // bytesWritten should be bufLen
            assertTrue(writeArgs.bytesWritten() == bufLen + 3);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 2. Call write method with
     *
     * - HIGH priority
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent
     *
     * Expected Result:
     *
     * Buffer put in flush buffer and write method returns the remaining bytes queued.
     */
    @Test
    public void basicWFPTestCase2()
    {
        String testData = "basicWFPTestCase2";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.position(0);
        byteBuf.put(testData.getBytes());
        String partialTestData = "WFPTestCase2";
        int partialBufLen = partialTestData.length();
        ByteBuffer partialByteBuf = ByteBuffer.allocate(partialBufLen);
        partialByteBuf.put(partialTestData.getBytes());
        partialByteBuf.position(0);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }
            }
                    ;

            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen+3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            //java.nio.channels.SocketChannel.write() returns 5 bytes sent (3 is for RIPC header)
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)(5 + 3));

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // write should return bufLen - 5
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) == (bufLen - 5));
            // bytesWritten should be 5

            assertTrue(writeArgs.bytesWritten() == 5 + 3);
            // _totalBytesQueued should be partialBufLen
            assertTrue(rsslChnl._totalBytesQueued == partialBufLen);

            // create SocketBuffer to compare to partial buffer in queue
            TransportBufferImpl partBuf = new TransportBufferImpl(partialBufLen);
            partBuf.data().put(partialByteBuf);
            partBuf._length = partBuf.data().limit();
            partBuf._data.position(0);

            // flush buffer should contain partial message
            assertEquals(0, compareAsString(rsslChnl._releaseBufferArray[0], partBuf));
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 3. Call write method with
     *
     * - MEDIUM priority
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent
     *
     * Expected Result: Buffer put in flush buffer and write method returns the remaining bytes queued.
     */
    @Test
    public void basicWFPTestCase3()
    {
        String testData = "basicWFPTestCase3";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.position(0);
        byteBuf.put(testData.getBytes());
        String partialTestData = "WFPTestCase3";
        int partialBufLen = partialTestData.length();
        ByteBuffer partialByteBuf = ByteBuffer.allocate(partialBufLen);
        partialByteBuf.put(partialTestData.getBytes());
        partialByteBuf.position(0);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }
            }
                    ;

            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen+3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            //java.nio.channels.SocketChannel.write() returns 5 bytes sent (3 is for RIPC header)
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)(5 + 3));

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // write should return bufLen - 5
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) == bufLen - 5);
            // bytesWritten should be 5
            assertTrue(writeArgs.bytesWritten() == 5 + 3);
            // _totalBytesQueued should be partialBufLen
            assertTrue(rsslChnl._totalBytesQueued == partialBufLen);

            // create SocketBuffer to compare to partial buffer in queue
            TransportBufferImpl partBuf = new TransportBufferImpl(partialBufLen);
            partBuf.data().put(partialByteBuf);
            partBuf._length = partBuf.data().limit();
            partBuf._data.position(0);

            // flush buffer should contain partial message
            assertTrue(compareAsString(rsslChnl._releaseBufferArray[0], partBuf) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 4. Call write method with
     *
     * - LOW priority
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent
     *
     * Expected Result: Buffer put in flush buffer and write method returns the remaining bytes queued.
     */
    @Test
    public void basicWFPTestCase4()
    {
        String testData = "basicWFPTestCase4";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.position(0);
        byteBuf.put(testData.getBytes());
        String partialTestData = "WFPTestCase4";
        int partialBufLen = partialTestData.length();
        ByteBuffer partialByteBuf = ByteBuffer.allocate(partialBufLen);
        partialByteBuf.put(partialTestData.getBytes());
        partialByteBuf.position(0);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }
            }
                    ;

            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen+3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            //java.nio.channels.SocketChannel.write() returns 5 bytes sent (3 is for RIPC header)
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)(5 + 3));

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // write should return bufLen - 5
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) == bufLen - 5);
            // bytesWritten should be 5
            assertTrue(writeArgs.bytesWritten() == 5 + 3);
            // _totalBytesQueued should be partialBufLen
            assertTrue(rsslChnl._totalBytesQueued == partialBufLen);

            // create SocketBuffer to compare to partial buffer in queue
            TransportBufferImpl partBuf = new TransportBufferImpl(partialBufLen);
            partBuf.data().put(partialByteBuf);
            partBuf._length = partBuf.data().limit();
            partBuf._data.position(0);

            // flush buffer should contain partial message
            assertTrue(compareAsString(rsslChnl._releaseBufferArray[0], partBuf) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 5. Call write method with
     *
     * - HIGH priority
     * - DIRECT_SOCKET_WRITE flag set
     * - buffers queued
     *
     * Expected Result: Buffer put in HIGH priority queue and flush method called.
     * Returns 0 if flush returns 0 or remaining bytes queued if flush returns a positive number.
     */
    @Test
    public void basicWFPTestCase5()
    {
        String testData = "basicWFPTestCase5";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    return 17;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            transBuf._isWriteBuffer = true;
            transBuf.data(byteBuf);

            // write should return remaining bytes queued
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == 17);

            // end of queue should contain message
            assertEquals((ScktChnl._highPriorityQueue)._tail, transBuf);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 6. Call write method with
     *
     * - MEDIUM priority
     * - DIRECT_SOCKET_WRITE flag set
     * - buffers queued
     *
     * Expected Result: Buffer put in MEDIUM priority queue and flush method called.
     * Returns 0 if flush returns 0 or remaining bytes queued if flush returns a positive number.
     */
    @Test
    public void basicWFPTestCase6()
    {
        String testData = "basicWFPTestCase6";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    return 17;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            transBuf._isWriteBuffer = true;
            transBuf.data(byteBuf);

            // write should return remaining bytes queued
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == 17);

            // end of queue should contain message
            assertEquals((ScktChnl._mediumPriorityQueue)._tail, transBuf);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 7. Call write method with
     *
     * - LOW priority
     * - DIRECT_SOCKET_WRITE flag set
     * - buffers queued
     *
     * Expected Result: Buffer put in LOW priority queue and flush method called.
     * Returns 0 if flush returns 0 or remaining bytes queued if flush returns a positive number.
     */
    @Test
    public void basicWFPTestCase7()
    {
        String testData = "basicWFPTestCase7";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    return 17;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            transBuf._isWriteBuffer = true;
            transBuf.data(byteBuf);

            // write should return remaining bytes queued
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == 17);

            // end of queue should contain message
            assertEquals((ScktChnl._lowPriorityQueue)._tail, transBuf);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 8. Call write method with
     *
     * - HIGH priority
     * - DIRECT_SOCKET_WRITE not set
     * - high water mark not reached
     * - no buffers queued
     *
     * Expected Result: Buffer put in HIGH priority queue and write method returns the total bytes queued.
     */
    @Test
    public void basicWFPTestCase8()
    {
        String testData = "basicWFPTestCase8";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl scktBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            scktBuf._isWriteBuffer = true;
            scktBuf.data(byteBuf);

            // write should return the total bytes queued
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(scktBuf, writeArgs, error) > 0);

            // end of queue should contain message
            assertEquals((ScktChnl._highPriorityQueue)._tail, scktBuf);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 9. Call write method with
     *
     * - HIGH priority
     * - DIRECT_SOCKET_WRITE not set
     * - high water mark reached
     * - buffers queued
     *
     * Expected Result: Buffer put in HIGH priority queue and flush method called.
     * Returns 0 if flush returns 0 or WRITE_FLUSH_FAILED if flush returns a positive number.
     */
    @Test
    public void basicWFPTestCase9()
    {
        String testData = "basicWFPTestCase9";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to empty queue and return 0
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    _highPriorityQueue.clear();
                    return 0;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            transBuf._isWriteBuffer = true;
            transBuf.data(byteBuf);
            // change high water mark so next call exceeds it
            ScktChnl._highWaterMark = 10;

            // write should return SUCCESS
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == TransportReturnCodes.SUCCESS);

            // queue should be empty
            assertEquals((ScktChnl._highPriorityQueue)._tail, null);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 10. Call write method with
     *
     * - MEDIUM priority
     * - DIRECT_SOCKET_WRITE not set
     * - high water mark not reached
     * - no buffers queued
     *
     * Expected Result: Buffer put in MEDIUM priority queue and write method returns the total bytes queued.
     */
    @Test
    public void basicWFPTestCase10()
    {
        String testData = "basicWFPTestCase10";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);


            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            transBuf.data().put(testData.getBytes());
            // write should return the total bytes queued
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) > 0);

            // end of queue should contain message
            assertEquals((ScktChnl._mediumPriorityQueue)._tail, transBuf);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 11. Call write method with
     *
     * - MEDIUM priority
     * - DIRECT_SOCKET_WRITE not set
     * - high water mark reached
     * - buffers queued
     *
     * Expected Result: Buffer put in MEDIUM priority queue and flush method called.
     * Returns 0 if flush returns 0 or WRITE_FLUSH_FAILED if flush returns a positive number.
     */
    @Test
    public void basicWFPTestCase11()
    {
        String testData = "basicWFPTestCase11";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to empty queue and return 0
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    _mediumPriorityQueue.clear();
                    return 0;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);


            TransportBufferImpl transBuf= sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            transBuf.data().put(testData.getBytes());

            // change high water mark so next call exceeds it
            ScktChnl._highWaterMark = 10;

            // write should return SUCCESS
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == TransportReturnCodes.SUCCESS);

            // queue should be empty
            assertEquals((ScktChnl._mediumPriorityQueue)._tail, null);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 12 Call write method with
     *
     * - LOW priority
     * - DIRECT_SOCKET_WRITE not set
     * - high water mark not reached
     * - no buffers queued
     *
     * Expected Result: Buffer put in LOW priority queue and write method returns the total bytes queued.
     */
    @Test
    public void basicWFPTestCase12()
    {
        String testData = "basicWFPTestCase12";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);

            TransportBufferImpl scktBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            scktBuf.data().put(testData.getBytes());

            // write should return the total bytes queued
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(scktBuf, writeArgs, error) > 0);

            // end of queue should contain message
            assertEquals((ScktChnl._lowPriorityQueue)._tail, scktBuf);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 13. Call write method with
     *
     * - LOW priority
     * - DIRECT_SOCKET_WRITE not set
     * - high water mark reached
     * - buffers queued
     *
     * Expected Result: Buffer put in LOW priority queue and flush method called.
     * Returns 0 if flush returns 0 or WRITE_FLUSH_FAILED if flush returns a positive number.
     */
    @Test
    public void basicWFPTestCase13()
    {
        String testData = "basicWFPTestCase13";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to empty queue and return 0
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    _lowPriorityQueue.clear();
                    return 0;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);

            TransportBufferImpl scktBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
            scktBuf.data().put(testData.getBytes());
            // change high water mark so next call exceeds it
            ScktChnl._highWaterMark = 10;

            // write should return SUCCESS
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(scktBuf, writeArgs, error) == TransportReturnCodes.SUCCESS);

            // queue should be empty
            assertEquals((ScktChnl._lowPriorityQueue)._tail, null);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 14. Call flush method with
     *
     * -java.nio.channels.SocketChannel.write() returns all bytes sent
     *
     * Expected Result: Buffers written per priority flush strategy and flush method returns 0.
     */
    @Test
    public void basicWFPTestCase14()
    {
        String testDataHigh = "basicWFPTestCase14High";
        String testDataMedium = "basicWFPTestCase14Medium";
        String testDataLow = "basicWFPTestCase14Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh = ByteBuffer.allocate(bufLenHigh);
        byteBufHigh.put(testDataHigh.getBytes());
        ByteBuffer byteBufMedium = ByteBuffer.allocate(bufLenMedium);
        byteBufMedium.put(testDataMedium.getBytes());
        ByteBuffer byteBufLow = ByteBuffer.allocate(bufLenLow);
        byteBufLow.put(testDataLow.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;
            TransportBufferImpl [] transBufHigh = new TransportBufferImpl[4];
            TransportBufferImpl [] transBufMedium = new TransportBufferImpl[4];
            TransportBufferImpl [] transBufLow = new TransportBufferImpl[4];
            for (int i=0; i<4; i++)
            {
                transBufHigh[i] = new TransportBufferImpl(bufLenHigh+3);

                transBufHigh[i]._isWriteBuffer = true;
                transBufHigh[i]._data.position(3);
                byteBufHigh.flip();
                transBufHigh[i].data().put(byteBufHigh);

                transBufMedium[i] = new TransportBufferImpl(bufLenMedium+3);

                transBufMedium[i]._isWriteBuffer = true;
                transBufMedium[i]._data.position(3);
                byteBufMedium.flip();
                transBufMedium[i].data().put(byteBufMedium);

                transBufLow[i] = new TransportBufferImpl(bufLenLow+3);
                transBufLow[i]._isWriteBuffer = true;
                transBufLow[i]._data.position(3);
                byteBufLow.flip();
                transBufLow[i].data().put(byteBufLow);
            }

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            for (int i=0; i<4; i++)
            {
                // queue several buffers by calling the write method several times with no write flags
                cumulativeBytesQueued += (bufLenHigh + 3);
                writeArgs.priority(WritePriorities.HIGH);
                writeArgs.flags(WriteFlags.NO_FLAGS);
                writeReturnVal = rsslChnl.write(transBufHigh[i], writeArgs, error);
                System.out.println("writeReturnVal, i "+ writeReturnVal + " " + i);
                // write return value should be cumulative bytes queued
                assertEquals(cumulativeBytesQueued, writeReturnVal);
                // bytesWritten should be scktBufHigh.getLength()
                assertTrue(writeArgs.bytesWritten() == transBufHigh[i].data().limit()-transBufHigh[i].data().position());
                cumulativeBytesQueued += (bufLenMedium + 3);
                writeArgs.priority(WritePriorities.MEDIUM);
                writeArgs.flags(WriteFlags.NO_FLAGS);
                writeReturnVal = rsslChnl.write(transBufMedium[i], writeArgs, error);
                // write return value should be cumulative bytes queued
                assertTrue(writeReturnVal == cumulativeBytesQueued);
                // bytesWritten should be scktBufMedium.getLength()
                assertTrue(writeArgs.bytesWritten() == transBufMedium[i].data().limit()-transBufMedium[i].data().position());
                cumulativeBytesQueued += (bufLenLow + 3);
                writeArgs.priority(WritePriorities.LOW);
                writeArgs.flags(WriteFlags.NO_FLAGS);
                writeReturnVal = rsslChnl.write(transBufLow[i], writeArgs, error);
                // write return value should be cumulative bytes queued
                assertTrue(writeReturnVal == cumulativeBytesQueued);
                // bytesWritten should be scktBufLow.getLength()
                assertTrue(writeArgs.bytesWritten() == transBufLow[i].data().limit()-transBufLow[i].data().position());
            }


            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
            assertTrue(rsslChnl._totalBytesQueued == cumulativeBytesQueued);

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            //java.nio.channels.SocketChannel.write() returns all bytes sent
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)cumulativeBytesQueued);

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // flush should return SUCCESS
            assertTrue(rsslChnl.flush(error) == TransportReturnCodes.SUCCESS);

            // _totalBytesQueued in RsslSocketChannel should be 0
            assertTrue(rsslChnl._totalBytesQueued == 0);

            // priority queues in RsslSocketChannel should be empty
            assertEquals((rsslChnl._highPriorityQueue)._tail, null);
            assertEquals((rsslChnl._mediumPriorityQueue)._tail, null);
            assertEquals((rsslChnl._lowPriorityQueue)._tail, null);

            // verify that the _gatherWriteArray's order matches expected flush order.
            assertEquals(transBufHigh[0].data(),   rsslChnl._gatheringWriteArray[0]); // H
            assertEquals(transBufMedium[0].data(), rsslChnl._gatheringWriteArray[1]); // M
            assertEquals(transBufHigh[1].data(),   rsslChnl._gatheringWriteArray[2]); // H
            assertEquals(transBufLow[0].data(),    rsslChnl._gatheringWriteArray[3]); // L
            assertEquals(transBufHigh[2].data(),   rsslChnl._gatheringWriteArray[4]); // H
            assertEquals(transBufMedium[1].data(), rsslChnl._gatheringWriteArray[5]); // M
            assertEquals(transBufHigh[3].data(),   rsslChnl._gatheringWriteArray[6]); // H
            assertEquals(transBufMedium[2].data(), rsslChnl._gatheringWriteArray[7]); // M
            assertEquals(transBufLow[1].data(),    rsslChnl._gatheringWriteArray[8]); // L
            assertEquals(transBufMedium[3].data(), rsslChnl._gatheringWriteArray[9]); // M
            assertEquals(transBufLow[2].data(),    rsslChnl._gatheringWriteArray[10]); // L
            assertEquals(transBufLow[3].data(),    rsslChnl._gatheringWriteArray[11]); // L
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 15. Call flush method with
     *
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent
     *
     * Expected Result: Buffers remain in gathering write array and flush method
     * returns the remaining bytes queued. The remainder of buffers (some possibly
     * partial) can be sent later.
     */
    @Test
    public void basicWFPTestCase15()
    {
        String testDataHigh = "basicWFPTestCase15High";
        String testDataMedium = "basicWFPTestCase15Medium";
        String testDataLow = "basicWFPTestCase15Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh1 = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufHigh2 = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufHigh3 = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufMedium1 = ByteBuffer.allocate(bufLenMedium);
        ByteBuffer byteBufMedium2 = ByteBuffer.allocate(bufLenMedium);
        ByteBuffer byteBufLow = ByteBuffer.allocate(bufLenLow);
        byteBufHigh1.position(0);
        byteBufHigh2.position(0);
        byteBufHigh3.position(0);
        byteBufMedium1.position(0);
        byteBufMedium2.position(0);
        byteBufLow.position(0);
        byteBufHigh1.put(testDataHigh.getBytes());
        byteBufHigh2.put(testDataHigh.getBytes());
        byteBufHigh3.put(testDataHigh.getBytes());
        byteBufMedium1.put(testDataMedium.getBytes());
        byteBufMedium2.put(testDataMedium.getBytes());
        byteBufLow.put(testDataLow.getBytes());
        String partialTestData = "Case15Low";
        int partialBufLen = partialTestData.length();
        ByteBuffer partialByteBuf = ByteBuffer.allocate(partialBufLen);
        partialByteBuf.put(partialTestData.getBytes());
        partialByteBuf.position(0);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                void releaseBufferInternal(TransportBuffer bufferInt)
                {
                    return;
                }
            }
                    ;

            TransportBufferImpl transBufHigh1 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh1._isWriteBuffer = true;
            transBufHigh1._data.position(3);
            byteBufHigh1.flip();
            transBufHigh1.data().put(byteBufHigh1);

            TransportBufferImpl transBufHigh2 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh2._isWriteBuffer = true;
            transBufHigh2._data.position(3);
            byteBufHigh2.flip();
            transBufHigh2.data().put(byteBufHigh2);

            TransportBufferImpl transBufHigh3 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh3._isWriteBuffer = true;
            transBufHigh3._data.position(3);
            byteBufHigh3.flip();
            transBufHigh3.data().put(byteBufHigh3);

            TransportBufferImpl transBufMedium1 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium1._isWriteBuffer = true;
            transBufMedium1._data.position(3);
            byteBufMedium1.flip();
            transBufMedium1.data().put(byteBufMedium1);

            TransportBufferImpl transBufMedium2 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium2._isWriteBuffer = true;
            transBufMedium2._data.position(3);
            byteBufMedium2.flip();
            transBufMedium2.data().put(byteBufMedium2);

            TransportBufferImpl transBufLow = new TransportBufferImpl(bufLenLow+3);
            transBufLow._isWriteBuffer = true;
            transBufLow._data.position(3);
            byteBufLow.flip();
            transBufLow.data().put(byteBufLow);


            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            // queue several buffers by calling the write method several times with no write flags
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufHigh1, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh1.data().limit()-transBufHigh1.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufMedium1, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium1.data().limit()-transBufMedium1.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufHigh2, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh2.data().limit()-transBufHigh2.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufLow, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow.data().limit()-transBufLow.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufHigh3, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh3.data().limit()-transBufHigh3.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufMedium2, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium2.data().limit()-transBufMedium2.data().position());

            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
            assertTrue(rsslChnl._totalBytesQueued == cumulativeBytesQueued);

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            //java.nio.channels.SocketChannel.write() returns partial bytes sent
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)92);

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // flush should return bytes remaining to be sent
            assertTrue(rsslChnl.flush(error) == (cumulativeBytesQueued - 92));

            // _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
            assertTrue(rsslChnl._totalBytesQueued == (cumulativeBytesQueued - 92));

            // _isFlushPending in RsslSocketChannel should be true
            assertTrue(rsslChnl._isFlushPending == true);

            // _writeArrayPosition in RsslSocketChannel should be 3
            assertTrue(rsslChnl._writeArrayPosition == 3);

            // create SocketBuffer to compare to partial buffer in queue
            TransportBufferImpl partialBuf = new TransportBufferImpl(partialBufLen);
            partialBuf.data().put(partialByteBuf);
            partialBuf._length = partialBuf.data().limit();
            partialBuf._data.position(0);

            // _gatheringWriteArray/_releaseBufferArray in RsslSocketChannel should contain remaining bytes sent
            assertTrue(rsslChnl._gatheringWriteArray[rsslChnl._writeArrayPosition] == rsslChnl._releaseBufferArray[rsslChnl._writeArrayPosition].data());
            assertTrue(compareAsString(rsslChnl._releaseBufferArray[rsslChnl._writeArrayPosition], partialBuf) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 16. Call flush method twice with
     *
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent for first flush
     * -java.nio.channels.SocketChannel.write() returns all bytes sent for second flush
     *
     * Expected Result: Buffers written per priority flush strategy and flush method returns 0.
     * Check that priority flush strategy is maintained from start of test to end of test.
     */
    @Test
    public void basicWFPTestCase16()
    {
        String testDataHigh = "basicWFPTestCase16High";
        String testDataMedium = "basicWFPTestCase16Medium";
        String testDataLow = "basicWFPTestCase16Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh1 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh2 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh3 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh4 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh5 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh6 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh7 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh8 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh9 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh10 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh11 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh12 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh13 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh14 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh15 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh16 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh17 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh18 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh19 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh20 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh21 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh22 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh23 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufHigh24 = ByteBuffer.allocate(bufLenHigh );
        ByteBuffer byteBufMedium1 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium2 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium3 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium4 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium5 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium6 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium7 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium8 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium9 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium10 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium11 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium12 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium13 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium14 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium15 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufMedium16 = ByteBuffer.allocate(bufLenMedium );
        ByteBuffer byteBufLow1 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow2 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow3 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow4 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow5 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow6 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow7 = ByteBuffer.allocate(bufLenLow );
        ByteBuffer byteBufLow8 = ByteBuffer.allocate(bufLenLow );
        byteBufHigh1.put(testDataHigh.getBytes());
        byteBufHigh2.put(testDataHigh.getBytes());
        byteBufHigh3.put(testDataHigh.getBytes());
        byteBufHigh4.put(testDataHigh.getBytes());
        byteBufHigh5.put(testDataHigh.getBytes());
        byteBufHigh6.put(testDataHigh.getBytes());
        byteBufHigh7.put(testDataHigh.getBytes());
        byteBufHigh8.put(testDataHigh.getBytes());
        byteBufHigh9.put(testDataHigh.getBytes());
        byteBufHigh10.put(testDataHigh.getBytes());
        byteBufHigh11.put(testDataHigh.getBytes());
        byteBufHigh12.put(testDataHigh.getBytes());
        byteBufHigh13.put(testDataHigh.getBytes());
        byteBufHigh14.put(testDataHigh.getBytes());
        byteBufHigh15.put(testDataHigh.getBytes());
        byteBufHigh16.put(testDataHigh.getBytes());
        byteBufHigh17.put(testDataHigh.getBytes());
        byteBufHigh18.put(testDataHigh.getBytes());
        byteBufHigh19.put(testDataHigh.getBytes());
        byteBufHigh20.put(testDataHigh.getBytes());
        byteBufHigh21.put(testDataHigh.getBytes());
        byteBufHigh22.put(testDataHigh.getBytes());
        byteBufHigh23.put(testDataHigh.getBytes());
        byteBufHigh24.put(testDataHigh.getBytes());
        byteBufMedium1.put(testDataMedium.getBytes());
        byteBufMedium2.put(testDataMedium.getBytes());
        byteBufMedium3.put(testDataMedium.getBytes());
        byteBufMedium4.put(testDataMedium.getBytes());
        byteBufMedium5.put(testDataMedium.getBytes());
        byteBufMedium6.put(testDataMedium.getBytes());
        byteBufMedium7.put(testDataMedium.getBytes());
        byteBufMedium8.put(testDataMedium.getBytes());
        byteBufMedium9.put(testDataMedium.getBytes());
        byteBufMedium10.put(testDataMedium.getBytes());
        byteBufMedium11.put(testDataMedium.getBytes());
        byteBufMedium12.put(testDataMedium.getBytes());
        byteBufMedium13.put(testDataMedium.getBytes());
        byteBufMedium14.put(testDataMedium.getBytes());
        byteBufMedium15.put(testDataMedium.getBytes());
        byteBufMedium16.put(testDataMedium.getBytes());
        byteBufLow1.put(testDataLow.getBytes());
        byteBufLow2.put(testDataLow.getBytes());
        byteBufLow3.put(testDataLow.getBytes());
        byteBufLow4.put(testDataLow.getBytes());
        byteBufLow5.put(testDataLow.getBytes());
        byteBufLow6.put(testDataLow.getBytes());
        byteBufLow7.put(testDataLow.getBytes());
        byteBufLow8.put(testDataLow.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // overridejava.nio.channels.SocketChannel write
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                @Override
                public long write(ByteBuffer[] srcs, int offset, int length)
                {
                    int bytesWritten = 92, cumulativeBytes = 0;

                    // write 92 bytes to _junitTestBuffer for each call
                    for (int i = offset; i < offset + length; i++)
                    {
                        for (int j = srcs[i].position(); j < srcs[i].limit(); j++)
                        {
                            _junitTestBuffer[_junitTestBufPosition++] = srcs[i].get(j);
                            cumulativeBytes++;
                            if (cumulativeBytes == bytesWritten)
                            {
                                break;
                            }
                        }
                        if (cumulativeBytes == bytesWritten)
                        {
                            break;
                        }
                    }

                    return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
                }

                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;

            ScktChnl._junitTestBuffer = new byte[1224];
            ScktChnl._junitTestBufPosition = 0;

            TransportBufferImpl transBufHigh1 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh1._isWriteBuffer = true;
            transBufHigh1._data.position(3);
            byteBufHigh1.flip();
            transBufHigh1.data().put(byteBufHigh1);

            TransportBufferImpl transBufHigh2 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh2._isWriteBuffer = true;
            transBufHigh2._data.position(3);
            byteBufHigh2.flip();
            transBufHigh2.data().put(byteBufHigh2);

            TransportBufferImpl transBufHigh3 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh3._isWriteBuffer = true;
            transBufHigh3._data.position(3);
            byteBufHigh3.flip();
            transBufHigh3.data().put(byteBufHigh3);

            TransportBufferImpl transBufHigh4 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh4._isWriteBuffer = true;
            transBufHigh4._data.position(3);
            byteBufHigh4.flip();
            transBufHigh4.data().put(byteBufHigh4);

            TransportBufferImpl transBufHigh5 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh5._isWriteBuffer = true;
            transBufHigh5._data.position(3);
            byteBufHigh5.flip();
            transBufHigh5.data().put(byteBufHigh5);

            TransportBufferImpl transBufHigh6 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh6._isWriteBuffer = true;
            transBufHigh6._data.position(3);
            byteBufHigh6.flip();
            transBufHigh6.data().put(byteBufHigh6);

            TransportBufferImpl transBufHigh7 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh7._isWriteBuffer = true;
            transBufHigh7._data.position(3);
            byteBufHigh7.flip();
            transBufHigh7.data().put(byteBufHigh7);

            TransportBufferImpl transBufHigh8 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh8._isWriteBuffer = true;
            transBufHigh8._data.position(3);
            byteBufHigh8.flip();
            transBufHigh8.data().put(byteBufHigh8);

            TransportBufferImpl transBufHigh9 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh9._isWriteBuffer = true;
            transBufHigh9._data.position(3);
            byteBufHigh9.flip();
            transBufHigh9.data().put(byteBufHigh9);

            TransportBufferImpl transBufHigh10 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh10._isWriteBuffer = true;
            transBufHigh10._data.position(3);
            byteBufHigh10.flip();
            transBufHigh10.data().put(byteBufHigh10);

            TransportBufferImpl transBufHigh11 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh11._isWriteBuffer = true;
            transBufHigh11._data.position(3);
            byteBufHigh11.flip();
            transBufHigh11.data().put(byteBufHigh11);

            TransportBufferImpl transBufHigh12 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh12._isWriteBuffer = true;
            transBufHigh12._data.position(3);
            byteBufHigh12.flip();
            transBufHigh12.data().put(byteBufHigh12);

            TransportBufferImpl transBufHigh13 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh13._isWriteBuffer = true;
            transBufHigh13._data.position(3);
            byteBufHigh13.flip();
            transBufHigh13.data().put(byteBufHigh13);

            TransportBufferImpl transBufHigh14 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh14._isWriteBuffer = true;
            transBufHigh14._data.position(3);
            byteBufHigh14.flip();
            transBufHigh14.data().put(byteBufHigh14);

            TransportBufferImpl transBufHigh15 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh15._isWriteBuffer = true;
            transBufHigh15._data.position(3);
            byteBufHigh15.flip();
            transBufHigh15.data().put(byteBufHigh15);

            TransportBufferImpl transBufHigh16 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh16._isWriteBuffer = true;
            transBufHigh16._data.position(3);
            byteBufHigh16.flip();
            transBufHigh16.data().put(byteBufHigh16);

            TransportBufferImpl transBufHigh17 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh17._isWriteBuffer = true;
            transBufHigh17._data.position(3);
            byteBufHigh17.flip();
            transBufHigh17.data().put(byteBufHigh17);

            TransportBufferImpl transBufHigh18 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh18._isWriteBuffer = true;
            transBufHigh18._data.position(3);
            byteBufHigh18.flip();
            transBufHigh18.data().put(byteBufHigh18);

            TransportBufferImpl transBufHigh19 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh19._isWriteBuffer = true;
            transBufHigh19._data.position(3);
            byteBufHigh19.flip();
            transBufHigh19.data().put(byteBufHigh19);

            TransportBufferImpl transBufHigh20 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh20._isWriteBuffer = true;
            transBufHigh20._data.position(3);
            byteBufHigh20.flip();
            transBufHigh20.data().put(byteBufHigh20);

            TransportBufferImpl transBufHigh21 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh21._isWriteBuffer = true;
            transBufHigh21._data.position(3);
            byteBufHigh21.flip();
            transBufHigh21.data().put(byteBufHigh21);

            TransportBufferImpl transBufHigh22 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh22._isWriteBuffer = true;
            transBufHigh22._data.position(3);
            byteBufHigh22.flip();
            transBufHigh22.data().put(byteBufHigh22);

            TransportBufferImpl transBufHigh23 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh23._isWriteBuffer = true;
            transBufHigh23._data.position(3);
            byteBufHigh23.flip();
            transBufHigh23.data().put(byteBufHigh23);

            TransportBufferImpl transBufHigh24 = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh24._isWriteBuffer = true;
            transBufHigh24._data.position(3);
            byteBufHigh24.flip();
            transBufHigh24.data().put(byteBufHigh24);

            TransportBufferImpl transBufMedium1 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium1._isWriteBuffer = true;
            transBufMedium1._data.position(3);
            byteBufMedium1.flip();
            transBufMedium1.data().put(byteBufMedium1);

            TransportBufferImpl transBufMedium2 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium2._isWriteBuffer = true;
            transBufMedium2._data.position(3);
            byteBufMedium2.flip();
            transBufMedium2.data().put(byteBufMedium2);

            TransportBufferImpl transBufMedium3 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium3._isWriteBuffer = true;
            transBufMedium3._data.position(3);
            byteBufMedium3.flip();
            transBufMedium3.data().put(byteBufMedium3);

            TransportBufferImpl transBufMedium4 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium4._isWriteBuffer = true;
            transBufMedium4._data.position(3);
            byteBufMedium4.flip();
            transBufMedium4.data().put(byteBufMedium4);

            TransportBufferImpl transBufMedium5 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium5._isWriteBuffer = true;
            transBufMedium5._data.position(3);
            byteBufMedium5.flip();
            transBufMedium5.data().put(byteBufMedium5);

            TransportBufferImpl transBufMedium6 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium6._isWriteBuffer = true;
            transBufMedium6._data.position(3);
            byteBufMedium6.flip();
            transBufMedium6.data().put(byteBufMedium6);

            TransportBufferImpl transBufMedium7 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium7._isWriteBuffer = true;
            transBufMedium7._data.position(3);
            byteBufMedium7.flip();
            transBufMedium7.data().put(byteBufMedium7);

            TransportBufferImpl transBufMedium8 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium8._isWriteBuffer = true;
            transBufMedium8._data.position(3);
            byteBufMedium8.flip();
            transBufMedium8.data().put(byteBufMedium8);

            TransportBufferImpl transBufMedium9 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium9._data.position(3);
            byteBufMedium9.flip();
            transBufMedium9.data().put(byteBufMedium9);
            transBufMedium9._isWriteBuffer = true;

            TransportBufferImpl transBufMedium10 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium10._data.position(3);
            byteBufMedium10.flip();
            transBufMedium10.data().put(byteBufMedium10);
            transBufMedium10._isWriteBuffer = true;

            TransportBufferImpl transBufMedium11 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium11._data.position(3);
            byteBufMedium11.flip();
            transBufMedium11.data().put(byteBufMedium11);
            transBufMedium11._isWriteBuffer = true;

            TransportBufferImpl transBufMedium12 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium12._data.position(3);
            byteBufMedium12.flip();
            transBufMedium12.data().put(byteBufMedium12);
            transBufMedium12._isWriteBuffer = true;

            TransportBufferImpl transBufMedium13 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium13._data.position(3);
            byteBufMedium13.flip();
            transBufMedium13.data().put(byteBufMedium13);
            transBufMedium13._isWriteBuffer = true;

            TransportBufferImpl transBufMedium14 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium14._data.position(3);
            byteBufMedium14.flip();
            transBufMedium14.data().put(byteBufMedium14);
            transBufMedium14._isWriteBuffer = true;

            TransportBufferImpl transBufMedium15 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium15._data.position(3);
            byteBufMedium15.flip();
            transBufMedium15.data().put(byteBufMedium15);
            transBufMedium15._isWriteBuffer = true;

            TransportBufferImpl transBufMedium16 = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium16._data.position(3);
            byteBufMedium16.flip();
            transBufMedium16.data().put(byteBufMedium16);
            transBufMedium16._isWriteBuffer = true;

            TransportBufferImpl transBufLow1 = new TransportBufferImpl(bufLenLow+3);
            transBufLow1._data.position(3);
            byteBufLow1.flip();
            transBufLow1.data().put(byteBufLow1);
            transBufLow1._isWriteBuffer = true;

            TransportBufferImpl transBufLow2 = new TransportBufferImpl(bufLenLow+3);
            transBufLow2._data.position(3);
            byteBufLow2.flip();
            transBufLow2.data().put(byteBufLow2);
            transBufLow2._isWriteBuffer = true;

            TransportBufferImpl transBufLow3 = new TransportBufferImpl(bufLenLow+3);
            transBufLow3._data.position(3);
            byteBufLow3.flip();
            transBufLow3.data().put(byteBufLow3);
            transBufLow3._isWriteBuffer = true;

            TransportBufferImpl transBufLow4 = new TransportBufferImpl(bufLenLow+3);
            transBufLow4._data.position(3);
            byteBufLow4.flip();
            transBufLow4.data().put(byteBufLow4);
            transBufLow4._isWriteBuffer = true;

            TransportBufferImpl transBufLow5 = new TransportBufferImpl(bufLenLow+3);
            transBufLow5._data.position(3);
            byteBufLow5.flip();
            transBufLow5.data().put(byteBufLow5);
            transBufLow5._isWriteBuffer = true;

            TransportBufferImpl transBufLow6 = new TransportBufferImpl(bufLenLow+3);
            transBufLow6._data.position(3);
            byteBufLow6.flip();
            transBufLow6.data().put(byteBufLow6);
            transBufLow6._isWriteBuffer = true;

            TransportBufferImpl transBufLow7 = new TransportBufferImpl(bufLenLow+3);
            transBufLow7._data.position(3);
            byteBufLow7.flip();
            transBufLow7.data().put(byteBufLow7);
            transBufLow7._isWriteBuffer = true;

            TransportBufferImpl transBufLow8 = new TransportBufferImpl(bufLenLow+3);
            transBufLow8._data.position(3);
            byteBufLow8.flip();
            transBufLow8.data().put(byteBufLow8);
            transBufLow8._isWriteBuffer = true;

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            // queue several buffers by calling the write method several times with no write flags
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh1, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh1.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh1.data().limit()-transBufHigh1.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium1, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium1.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium1.data().limit()-transBufMedium1.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh2, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh2.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh2.data().limit()-transBufHigh2.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow1, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow1.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow1.data().limit()-transBufLow1.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh3, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh3.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh3.data().limit()-transBufHigh3.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium2, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium2.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium2.data().limit()-transBufMedium2.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh4, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh4.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh4.data().limit()-transBufHigh4.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium3, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium3.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium3.data().limit()-transBufMedium3.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh5, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh5.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh5.data().limit()-transBufHigh5.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow2, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow2.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow2.data().limit()-transBufLow2.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh6, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh6.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh6.data().limit()-transBufHigh6.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium4, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium4.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium4.data().limit()-transBufMedium4.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh7, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh7.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh7.data().limit()-transBufHigh7.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium5, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium5.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium5.data().limit()-transBufMedium5.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh8, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh8.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh8.data().limit()-transBufHigh8.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow3, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow3.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow3.data().limit()-transBufLow3.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh9, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh9.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh9.data().limit()-transBufHigh9.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium6, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium6.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium6.data().limit()-transBufMedium6.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh10, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh10.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh10.data().limit()-transBufHigh10.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium7, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium7.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium7.data().limit()-transBufMedium7.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh11, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh11.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh11.data().limit()-transBufHigh11.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow4, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow4.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow4.data().limit()-transBufLow4.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh12, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh12.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh12.data().limit()-transBufHigh12.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium8, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium8.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium8.data().limit()-transBufMedium8.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh13, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh13.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh13.data().limit()-transBufHigh13.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium9, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium9.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium9.data().limit()-transBufMedium9.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh14, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh14.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh14.data().limit()-transBufHigh14.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow5, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow5.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow5.data().limit()-transBufLow5.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh15, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh15.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh15.data().limit()-transBufHigh15.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium10, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium10.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium10.data().limit()-transBufMedium10.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh16, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh16.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh16.data().limit()-transBufHigh16.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium11, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium11.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium11.data().limit()-transBufMedium11.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh17, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh17.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh17.data().limit()-transBufHigh17.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow6, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow6.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow6.data().limit()-transBufLow6.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh18, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh18.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh18.data().limit()-transBufHigh18.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium12, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium12.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium12.data().limit()-transBufMedium12.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh19, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh19.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh19.data().limit()-transBufHigh19.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium13, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium13.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium13.data().limit()-transBufMedium13.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh20, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh20.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh20.data().limit()-transBufHigh20.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow7, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow7.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow7.data().limit()-transBufLow7.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh21, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh21.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh21.data().limit()-transBufHigh21.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium14, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium14.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium14.data().limit()-transBufMedium14.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh22, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh22.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh22.data().limit()-transBufHigh22.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium15, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium15.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium15.data().limit()-transBufMedium15.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh23, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh23.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh23.data().limit()-transBufHigh23.data().position());
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow8, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufLow8.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufLow8.data().limit()-transBufLow8.data().position());
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh24, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufHigh24.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufHigh24.data().limit()-transBufHigh24.data().position());
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium16, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued);
            // bytesWritten should be scktBufMedium16.getLength()
            assertTrue(writeArgs.bytesWritten() == transBufMedium16.data().limit()-transBufMedium16.data().position());

            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
            assertTrue(ScktChnl._totalBytesQueued == cumulativeBytesQueued);

            while (cumulativeBytesQueued > 0)
            {
                int bytesFlushed = 0, flushRetVal;

                flushRetVal = ScktChnl.flush(error);
                bytesFlushed = cumulativeBytesQueued - flushRetVal;

                // _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
                assertTrue(ScktChnl._totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

                // decrement cumulativeBytesQueued by bytesFlushed
                cumulativeBytesQueued -= bytesFlushed;
            }

            // _totalBytesQueued in RsslSocketChannel should be 0
            assertTrue(ScktChnl._totalBytesQueued == 0);

            // verify bytes are written in correct order
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 3, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 28, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 55, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 80, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 104, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 129, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 156, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 181, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 208, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 233, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 257, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 282, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 309, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 334, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 361, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 386, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 410, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 435, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 462, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 487, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 514, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 539, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 563, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 588, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 615, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 640, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 667, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 692, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 716, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 741, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 768, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 793, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 820, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 845, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 869, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 894, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 921, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 946, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 973, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 998, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 1022, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 1047, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 1074, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 1099, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 1126, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 1151, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 1175, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 1200, testDataMedium.length())) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 17. Call ping method with
     *
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns all bytes sent
     *
     * Expected Result: Ping message written to network and ping method returns 0.
     */
    @Test
    public void basicWFPTestCase17()
    {
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            when(scktChnl.write(Mockito.any(ByteBuffer.class))).thenReturn(3);

            // set RsslSocketChannel internal socket channel to mock socket channel
            ScktChnl._scktChannel = scktChnl;

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // ping call should return success
            assertTrue(ScktChnl.ping(error) == TransportReturnCodes.SUCCESS);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 18. Call ping method with
     *
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent
     *
     * Expected Result: Ping message not written to network and ping method returns a positive number.
     */
    @Test
    public void basicWFPTestCase18()
    {
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            when(scktChnl.write(Mockito.any(ByteBuffer.class))).thenReturn(1);

            // set RsslSocketChannel internal socket channel to mock socket channel
            ScktChnl._scktChannel = scktChnl;

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // ping call should return partial bytes sent
            assertTrue(ScktChnl.ping(error) > 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 19. Call ping method with
     *
     * - buffers queued
     *
     * Expected Result: Flush method called (returns flush method's return value).
     */
    @Test
    public void basicWFPTestCase19()
    {
        Error error = TransportFactory.createError();
        final int flushRetVal = 57;

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return 0
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                protected int flushInternal(Error error)
                {
                    return flushRetVal;
                }
            };

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued greater than 0 to simulate buffers queued
            ScktChnl._totalBytesQueued = 19;

            // ping call should return flush method return value
            assertTrue(ScktChnl.ping(error) == flushRetVal);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 20. Call write method with
     *
     * - Channel not in ACTIVE state
     *
     * Expected Result: Returns FAILURE.
     */
    @Test
    public void basicWFPTestCase20()
    {
        String testData = "basicWFPTestCase20";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBuffer transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION) ;

            // set RsslSocketChannel state to INITIALIZING
            ScktChnl._state = ChannelState.INITIALIZING;

            // write call should return failure
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == TransportReturnCodes.FAILURE);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 21. Call flush method with
     *
     * - Channel not in ACTIVE state
     *
     * Expected Result: Returns FAILURE.
     */
    @Test
    public void basicWFPTestCase21()
    {
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            // set RsslSocketChannel state to INITIALIZING
            ScktChnl._state = ChannelState.INITIALIZING;

            // flush call should return failure
            assertTrue(ScktChnl.flush(error) == TransportReturnCodes.FAILURE);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 22. Call write method with
     *
     * - Channel closed (SocketChannel throws exception)
     * - no buffers queued
     * - DIRECT_SOCKET_WRITE flag set
     *
     * Expected Result: Write method returns FAILURE.
     */
    @Test
    public void basicWFPTestCase22()
    {
        String testData = "basicWFPTestCase22";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();
            // reset all queues and total bytes written
            ScktChnl._highPriorityQueue.clear();
            ScktChnl._mediumPriorityQueue.clear();
            ScktChnl._lowPriorityQueue.clear();
            ScktChnl._totalBytesQueued = 0;

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION) ;
            transBuf._isWriteBuffer = true;
            transBuf.data(byteBuf);

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenThrow(new IOException());

            ScktChnl._scktChannel = scktChnl;

            // write should return FAILURE
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == TransportReturnCodes.WRITE_FLUSH_FAILED);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 23. Call write method with
     *
     * - Channel closed (SocketChannel throws exception)
     * - high water mark reached
     *
     * Expected Result: Flush method called and returns WRITE_FLUSH_FAILED.
     */
    @Test
    public void basicWFPTestCase23()
    {
        String testData = "basicWFPTestCase23";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;

            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen+3);
            transBuf._isWriteBuffer = true;
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);

            // reset all queues
            rsslChnl._highPriorityQueue.clear();
            rsslChnl._mediumPriorityQueue.clear();
            rsslChnl._lowPriorityQueue.clear();

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0
            rsslChnl._totalBytesQueued = 0;

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenThrow(new IOException());

            // set RsslSocketChannel internal socket channel to mock socket channel
            rsslChnl._scktChannel = scktChnl;

            // write should return socket buffer testData length
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) == (transBuf.data().limit()-transBuf.data().position()));

            // change high water mark so next call exceeds it
            rsslChnl._highWaterMark = 10;


            TransportBufferImpl transBuf1 = new TransportBufferImpl(bufLen+3);
            transBuf1._data.position(3);
            byteBuf.flip();
            transBuf1.data().put(byteBuf);
            transBuf1._isWriteBuffer = true;
//	        transBuf.data().position(transBuf.data().limit());
            // write should return WRITE_FLUSH_FAILED
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf1, writeArgs, error) == TransportReturnCodes.WRITE_FLUSH_FAILED);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 24. Call flush method with
     *
     * - Channel closed (SocketChannel throws exception)
     *
     * Expected Result: Returns FAILURE.
     */
    @Test
    public void basicWFPTestCase24()
    {
        String testData = "basicWFPTestCase24";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();
            // reset all queues
            ScktChnl._highPriorityQueue.clear();
            ScktChnl._mediumPriorityQueue.clear();
            ScktChnl._lowPriorityQueue.clear();

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0
            ScktChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            SocketBuffer sBuf = new SocketBuffer(null, 6144);
            TransportBufferImpl transBuf = sBuf.getBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION) ;
            transBuf._isWriteBuffer = true;
            transBuf.data(byteBuf);

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenThrow(new IOException());

            // set RsslSocketChannel internal socket channel to mock socket channel
            ScktChnl._scktChannel = scktChnl;

            // write should return socket buffer testData length
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(ScktChnl.write(transBuf, writeArgs, error) == (transBuf.data().limit()-transBuf.data().position()));

            // flush call should return failure
            assertTrue(ScktChnl.flush(error) == TransportReturnCodes.WRITE_FLUSH_FAILED);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 25. Call ping method with
     *
     * - Channel not in ACTIVE state
     *
     * Expected Result: Returns FAILURE.
     */
    @Test
    public void basicWFPTestCase25()
    {
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            // set RsslSocketChannel state to INITIALIZING
            ScktChnl._state = ChannelState.INITIALIZING;

            // ping call should return failure
            assertTrue(ScktChnl.ping(error) == TransportReturnCodes.FAILURE);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 26. Call ping method with
     *
     * - Channel closed (SocketChannel throws exception)
     *
     * Expected Result: Returns FAILURE.
     */
    @Test
    public void basicWFPTestCase26()
    {
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel ScktChnl = new RsslSocketChannel();

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            when(scktChnl.write(Mockito.any(ByteBuffer.class))).thenThrow(new IOException());

            // set RsslSocketChannel internal socket channel to mock socket channel
            ScktChnl._scktChannel = scktChnl;

            //set RsslSocketChannel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 so ping directly calls socket channel write
            ScktChnl._totalBytesQueued = 0;

            // ping call should return failure
            assertTrue(ScktChnl.ping(error) == TransportReturnCodes.FAILURE);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 27. Call write method and then flush method with
     *
     * - HIGH priority
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent for write call
     * -java.nio.channels.SocketChannel.write() returns all bytes sent for flush call
     *
     * Expected Result: Write method with DIRECT_SOCKET_WRITE flag set always flushed first.
     * Check that priority flush strategy is maintained from start of test to end of test.
     */
    @Test
    public void basicWFPTestCase27()
    {
        String testDataHigh = "basicWFPTestCase27High";
        String testDataMedium = "basicWFPTestCase27Medium";
        String testDataLow = "basicWFPTestCase27Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufMedium = ByteBuffer.allocate(bufLenMedium);
        ByteBuffer byteBufLow = ByteBuffer.allocate(bufLenLow);
        byteBufHigh.put(testDataHigh.getBytes());
        byteBufMedium.put(testDataMedium.getBytes());
        byteBufLow.put(testDataLow.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // overridejava.nio.channels.SocketChannel write
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public long write(ByteBuffer[] srcs, int offset, int length)
                {
                    int bytesWritten = 16, cumulativeBytes = 0;

                    // write 16 bytes to _junitTestBuffer for each call
                    for (int i = offset; i < offset + length; i++)
                    {
                        for (int j = srcs[i].position(); j < srcs[i].limit(); j++)
                        {
                            _junitTestBuffer[_junitTestBufPosition++] = srcs[i].get(j);
                            cumulativeBytes++;
                            if (cumulativeBytes == bytesWritten)
                            {
                                break;
                            }
                        }
                        if (cumulativeBytes == bytesWritten)
                        {
                            break;
                        }
                    }

                    return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
                }

                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;

            rsslChnl._junitTestBuffer = new byte[1224];
            rsslChnl._junitTestBufPosition = 0;

            TransportBufferImpl transBufHigh = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh._data.position(3);
            transBufHigh._isWriteBuffer = true;
            byteBufHigh.flip();
            transBufHigh.data().put(byteBufHigh);

            TransportBufferImpl transBufMedium = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium._data.position(3);
            transBufMedium._isWriteBuffer = true;
            byteBufMedium.flip();
            transBufMedium.data().put(byteBufMedium);

            TransportBufferImpl transBufLow = new TransportBufferImpl(bufLenLow+3);
            transBufLow._data.position(3);
            transBufLow._isWriteBuffer = true;
            byteBufLow.flip();
            transBufLow.data().put(byteBufLow);

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            // call the write method with DIRECT_SOCKET_WRITE write flag
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            writeReturnVal = rsslChnl.write(transBufHigh, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be 16
            assertTrue(writeArgs.bytesWritten() == 16);

            // queue a couple more buffers by calling the write method with no write flags
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufMedium, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertEquals(cumulativeBytesQueued - 16, writeReturnVal);
            // bytesWritten should be scktBufMedium.getLength()
            assertTrue(writeArgs.bytesWritten() == (transBufMedium.data().limit()-transBufMedium.data().position()));
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBufLow, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be scktBufLow.getLength()
            assertTrue(writeArgs.bytesWritten() == (transBufLow.data().limit()-transBufLow.data().position()));

            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
            assertTrue(rsslChnl._totalBytesQueued == cumulativeBytesQueued - 16);

            while (cumulativeBytesQueued > 0)
            {
                int bytesFlushed = 0, flushRetVal;

                flushRetVal = rsslChnl.flush(error);
                bytesFlushed = cumulativeBytesQueued - flushRetVal;

                // _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
                assertTrue(rsslChnl._totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

                // decrement cumulativeBytesQueued by bytesFlushed
                cumulativeBytesQueued -= bytesFlushed;
            }

            // _totalBytesQueued in RsslSocketChannel should be 0
            assertTrue(rsslChnl._totalBytesQueued == 0);

            // verify bytes are written in correct order
            assertTrue(testDataHigh.compareTo(new String(rsslChnl._junitTestBuffer, 3, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(rsslChnl._junitTestBuffer, 28, testDataMedium.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(rsslChnl._junitTestBuffer, 55, testDataLow.length())) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 28. Call write method and then flush method with
     *
     * - MEDIUM priority
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent for write call
     * -java.nio.channels.SocketChannel.write() returns all bytes sent for flush call
     *
     * Expected Result: Write method with DIRECT_SOCKET_WRITE flag set always flushed first.
     * Check that priority flush strategy is maintained from start of test to end of test.
     */
    @Test
    public void basicWFPTestCase28()
    {
        String testDataHigh = "basicWFPTestCase28High";
        String testDataMedium = "basicWFPTestCase28Medium";
        String testDataLow = "basicWFPTestCase28Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufMedium = ByteBuffer.allocate(bufLenMedium);
        ByteBuffer byteBufLow = ByteBuffer.allocate(bufLenLow);
        byteBufHigh.put(testDataHigh.getBytes());
        byteBufMedium.put(testDataMedium.getBytes());
        byteBufLow.put(testDataLow.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // overridejava.nio.channels.SocketChannel write
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                @Override
                public long write(ByteBuffer[] srcs, int offset, int length)
                {
                    int bytesWritten = 16, cumulativeBytes = 0;

                    // write 16 bytes to _junitTestBuffer for each call
                    for (int i = offset; i < offset + length; i++)
                    {
                        for (int j = srcs[i].position(); j < srcs[i].limit(); j++)
                        {
                            _junitTestBuffer[_junitTestBufPosition++] = srcs[i].get(j);
                            cumulativeBytes++;
                            if (cumulativeBytes == bytesWritten)
                            {
                                break;
                            }
                        }
                        if (cumulativeBytes == bytesWritten)
                        {
                            break;
                        }
                    }

                    return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
                }

                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            };

            ScktChnl._junitTestBuffer = new byte[1224];
            ScktChnl._junitTestBufPosition = 0;

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            TransportBufferImpl transBufHigh = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh._data.position(3);
            byteBufHigh.flip();
            transBufHigh.data().put(byteBufHigh);
            transBufHigh._isWriteBuffer = true;

            TransportBufferImpl transBufMedium = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium._data.position(3);
            byteBufMedium.flip();
            transBufMedium.data().put(byteBufMedium);
            transBufMedium._isWriteBuffer = true;

            TransportBufferImpl transBufLow = new TransportBufferImpl(bufLenLow+3);
            transBufLow._data.position(3);
            byteBufLow.flip();
            transBufLow.data().put(byteBufLow);
            transBufLow._isWriteBuffer = true;

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            // call the write method with DIRECT_SOCKET_WRITE write flag
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            writeReturnVal = ScktChnl.write(transBufMedium, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be 16
            assertTrue(writeArgs.bytesWritten() == 16);

            // queue a couple more buffers by calling the write method with no write flags
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be scktBufHigh.getLength()
            assertTrue(writeArgs.bytesWritten() == (transBufHigh.data().limit()-transBufHigh.data().position()));
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufLow, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be scktBufLow.getLength()
            assertTrue(writeArgs.bytesWritten() == (transBufLow.data().limit()-transBufLow.data().position()));

            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
            assertTrue(ScktChnl._totalBytesQueued == cumulativeBytesQueued - 16);

            while (cumulativeBytesQueued > 0)
            {
                int bytesFlushed = 0, flushRetVal;

                flushRetVal = ScktChnl.flush(error);
                bytesFlushed = cumulativeBytesQueued - flushRetVal;

                // _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
                assertTrue(ScktChnl._totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

                // decrement cumulativeBytesQueued by bytesFlushed
                cumulativeBytesQueued -= bytesFlushed;
            }

            // _totalBytesQueued in RsslSocketChannel should be 0
            assertTrue(ScktChnl._totalBytesQueued == 0);

            // verify bytes are written in correct order
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 3, testDataMedium.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 30, testDataHigh.length())) == 0);
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 55, testDataLow.length())) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * 29. Call write method and then flush method with
     *
     * - LOW priority
     * - DIRECT_SOCKET_WRITE flag set
     * - no buffers queued
     * -java.nio.channels.SocketChannel.write() returns partial bytes sent for write call
     * -java.nio.channels.SocketChannel.write() returns all bytes sent for flush call
     *
     * Expected Result: Write method with DIRECT_SOCKET_WRITE flag set always flushed first.
     * Check that priority flush strategy is maintained from start of test to end of test.
     */
    @Test
    public void basicWFPTestCase29()
    {
        String testDataHigh = "basicWFPTestCase29High";
        String testDataMedium = "basicWFPTestCase29Medium";
        String testDataLow = "basicWFPTestCase29Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufMedium = ByteBuffer.allocate(bufLenMedium);
        ByteBuffer byteBufLow = ByteBuffer.allocate(bufLenLow);
        byteBufHigh.put(testDataHigh.getBytes());
        byteBufMedium.put(testDataMedium.getBytes());
        byteBufLow.put(testDataLow.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // overridejava.nio.channels.SocketChannel write
            RsslSocketChannel ScktChnl = new RsslSocketChannel()
            {
                @Override
                public long write(ByteBuffer[] srcs, int offset, int length)
                {
                    int bytesWritten = 16, cumulativeBytes = 0;

                    // write 16 bytes to _junitTestBuffer for each call
                    for (int i = offset; i < offset + length; i++)
                    {
                        for (int j = srcs[i].position(); j < srcs[i].limit(); j++)
                        {
                            _junitTestBuffer[_junitTestBufPosition++] = srcs[i].get(j);
                            cumulativeBytes++;
                            if (cumulativeBytes == bytesWritten)
                            {
                                break;
                            }
                        }
                        if (cumulativeBytes == bytesWritten)
                        {
                            break;
                        }
                    }

                    return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
                }

                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            };

            ScktChnl._junitTestBuffer = new byte[1224];
            ScktChnl._junitTestBufPosition = 0;

            // set channel state to ACTIVE
            ScktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ScktChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            TransportBufferImpl transBufHigh = new TransportBufferImpl(bufLenHigh+3);
            transBufHigh._data.position(3);
            byteBufHigh.flip();
            transBufHigh.data().put(byteBufHigh);
            transBufHigh._isWriteBuffer = true;

            TransportBufferImpl transBufMedium = new TransportBufferImpl(bufLenMedium+3);
            transBufMedium._data.position(3);
            byteBufMedium.flip();
            transBufMedium.data().put(byteBufMedium);
            transBufMedium._isWriteBuffer = true;

            TransportBufferImpl transBufLow = new TransportBufferImpl(bufLenLow+3);
            transBufLow._data.position(3);
            byteBufLow.flip();
            transBufLow.data().put(byteBufLow);
            transBufLow._isWriteBuffer = true;

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            // call the write method with DIRECT_SOCKET_WRITE write flag
            cumulativeBytesQueued += (bufLenLow + 3);
            writeArgs.priority(WritePriorities.LOW);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            writeReturnVal = ScktChnl.write(transBufLow, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be 16
            assertTrue(writeArgs.bytesWritten() == 16);

            // queue a couple more buffers by calling the write method with no write flags
            cumulativeBytesQueued += (bufLenHigh + 3);
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufHigh, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be scktBufHigh.getLength()
            assertTrue(writeArgs.bytesWritten() == (transBufHigh.data().limit()-transBufHigh.data().position()));
            cumulativeBytesQueued += (bufLenMedium + 3);
            writeArgs.priority(WritePriorities.MEDIUM);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = ScktChnl.write(transBufMedium, writeArgs, error);
            // write return value should be cumulative bytes queued
            assertTrue(writeReturnVal == cumulativeBytesQueued - 16);
            // bytesWritten should be scktBufMedium.getLength()
            assertTrue(writeArgs.bytesWritten() == (transBufMedium.data().limit()-transBufMedium.data().position()));

            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
            assertTrue(ScktChnl._totalBytesQueued == cumulativeBytesQueued - 16);

            while (cumulativeBytesQueued > 0)
            {
                int bytesFlushed = 0, flushRetVal;

                flushRetVal = ScktChnl.flush(error);
                bytesFlushed = cumulativeBytesQueued - flushRetVal;

                // _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
                assertTrue(ScktChnl._totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

                // decrement cumulativeBytesQueued by bytesFlushed
                cumulativeBytesQueued -= bytesFlushed;
            }

            // _totalBytesQueued in RsslSocketChannel should be 0
            assertTrue(ScktChnl._totalBytesQueued == 0);

            // verify bytes are written in correct order
            assertTrue(testDataLow.compareTo(new String(ScktChnl._junitTestBuffer, 3, testDataLow.length())) == 0);
            assertTrue(testDataHigh.compareTo(new String(ScktChnl._junitTestBuffer, 27, testDataHigh.length())) == 0);
            assertTrue(testDataMedium.compareTo(new String(ScktChnl._junitTestBuffer, 52, testDataMedium.length())) == 0);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }
    ////// END basic write(), flush() and ping() tests //////

    /*
     * Create a TransportBuffer with zero length
     * write the buffer via channel
     * Also create a TransportBuffer with a fix length
     * write the buffer via channel without anything actual written
     *
     * Expected Result: TransportReturnCodes.FAILURE
     */
    @Test
    public void writeZeroLenghtBufferTest()
    {
        int BUF_SIZE = 0;
        int BUF_SIZE1 = 100;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            // overridejava.nio.channels.SocketChannel write
            RsslSocketChannel scktChnl = new RsslSocketChannel();
            // set channel state to ACTIVE
            scktChnl._state = ChannelState.ACTIVE;

            TransportBufferImpl transBuf = new TransportBufferImpl(BUF_SIZE);
            transBuf._isWriteBuffer = true;
            assertEquals(TransportReturnCodes.FAILURE, scktChnl.write(transBuf, writeArgs, error));

            transBuf = new TransportBufferImpl(BUF_SIZE1);
            transBuf._isWriteBuffer = true;
            assertEquals(TransportReturnCodes.FAILURE, scktChnl.write(transBuf, writeArgs, error));



            TransportBufferImpl transBuf1 = new TransportBufferImpl(BUF_SIZE1);
            transBuf1._isWriteBuffer = true;
            transBuf1._isPacked = true;

            ByteBuffer bb = transBuf1.data();
            bb.put("packed test #11".getBytes());
            assertTrue(scktChnl.packBuffer(transBuf1, error) > 0);


            bb.put("packed test #22".getBytes());
            assertTrue(scktChnl.packBuffer(transBuf1, error) > 0);

            bb.put("final packing test #3".getBytes());


            assertTrue(scktChnl.write(transBuf1, writeArgs, error) > 0);


        }
        catch(Exception e)
        {
            assertTrue(false);
        }
    }




    /*
     * Compressed Write Test - compares to expected results from ETAC
     *
     * Writes three times since stream produces different output for same data.
     */
    @Test
    public void compressedWriteTest()
    {
        String testData = "aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzz";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.put(testData.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();
        RsslSocketChannel rsslChnl = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number

            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));
            RsslSocketChannel initChannel = (RsslSocketChannel) Transport.connect(connectOpts, error);
            assertNotNull(initChannel);
            rsslChnl = getNetworkReplayChannel(Transport._transports[0],2);
            assertNotNull(rsslChnl);

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set-up channel for compression
            rsslChnl._sessionOutCompression = 1;
            rsslChnl._sessionCompLevel = 9;
            rsslChnl._compressor = rsslChnl._ZlibCompressor;
            rsslChnl._compressor.compressionLevel(rsslChnl._sessionCompLevel);

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // create SocketBuffer and set to test data
            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen+3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;

            /* FIRST WRITE */
            // write should return the total bytes queued
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) > 0);
            assertEquals(74, writeArgs.bytesWritten());
            assertEquals(81, writeArgs.uncompressedBytesWritten());

            // end of queue should contain compressed message
            TransportBufferImpl buf = (TransportBufferImpl) (rsslChnl._highPriorityQueue)._tail;

            // should equal output form ETAC
            byte[] expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/Compressed1.txt");
            assertNotNull(expected);
            byte[] bytes = new byte[buf.data().limit()];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);

            /* SECOND WRITE */
            // write should return the total bytes queued
            byteBuf.clear();
            byteBuf.put(testData.getBytes());

            TransportBufferImpl transBuf1 = new TransportBufferImpl(bufLen+3);
            transBuf1._data.position(3);
            byteBuf.flip();
            transBuf1.data().put(byteBuf);
            transBuf1._isWriteBuffer = true;

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf1, writeArgs, error) > 0);
            assertEquals(13, writeArgs.bytesWritten());
            assertEquals(81, writeArgs.uncompressedBytesWritten());

            // end of queue should contain compressed message
            buf = (TransportBufferImpl) (rsslChnl._highPriorityQueue)._tail;
            // should equal output form ETAC
            expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/Compressed2.txt");
            assertNotNull(expected);
            bytes = new byte[buf.data().limit()];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);

            /* THIRD WRITE */
            // write should return the total bytes queued
            byteBuf.clear();
            byteBuf.put(testData.getBytes());
            TransportBufferImpl transBuf2 = new TransportBufferImpl(bufLen+3);
            transBuf2._data.position(3);
            byteBuf.flip();
            transBuf2.data().put(byteBuf);
            transBuf2._isWriteBuffer = true;

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf2, writeArgs, error) > 0);
            assertEquals(12, writeArgs.bytesWritten());
            assertEquals(81, writeArgs.uncompressedBytesWritten());

            // end of queue should contain compressed message
            buf = (TransportBufferImpl) (rsslChnl._highPriorityQueue)._tail;

            // should equal output form ETAC
            expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/Compressed3.txt");
            assertNotNull(expected);
            bytes = new byte[buf.data().limit()];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);

            rsslChnl.close(error);
            initChannel.close(error);
        }
        catch(Exception e)
        {
            assertTrue(false);
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Packed Write Test - compares to expected results from ETAC
     *
     * Writes the same sequence as TransportTest writes.
     */
    @SuppressWarnings("deprecation")
    @Test
    public void packedWriteTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(5);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));
            // need to create ProtocolInt for Socket, so need to connect
            Channel initChannel = Transport.connect(connectOpts, error);
            assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 5);
            assertNotNull(channel);
            try
            {
                while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}

            // set channel state to ACTIVE
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            ((RsslSocketChannel)channel)._totalBytesQueued = 0;

            // change high water mark so cannot exceed it
            ((RsslSocketChannel)channel)._highWaterMark = 10000;

            // get 500 byte buffer
            TransportBuffer sendbuffer = channel.getBuffer(500, true, error);
            assertEquals(500, sendbuffer.length());

            ByteBuffer bb = sendbuffer.data();
            bb.put("packed test #1".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);


            bb.put("packed test #2".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("final packing test #3".getBytes());

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(channel.write(sendbuffer, writeArgs, error) > 0);
            assertEquals(58, writeArgs.uncompressedBytesWritten());
            assertEquals(58, writeArgs.bytesWritten());

            // end of queue should contain packed message
            TransportBufferImpl buf = (TransportBufferImpl)((RsslSocketChannel)channel)._highPriorityQueue._tail;

            // should equal output from ETAC
            byte[] expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/Packed1.txt");
            assertNotNull(expected);
            byte[] bytes = new byte[buf._length];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);

            sendbuffer = channel.getBuffer(500, true, error);

            bb = sendbuffer.data();
            bb.put("packing2 test #1".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("packing2 test #2".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("packing3 final test #3".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(channel.write(sendbuffer, writeArgs, error) > 0);
            assertEquals(63, writeArgs.uncompressedBytesWritten());
            assertEquals(63, writeArgs.bytesWritten());

            // end of queue should contain packed message
            buf = (TransportBufferImpl)((RsslSocketChannel)channel)._highPriorityQueue._tail;

            // should equal output from ETAC
            expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/Packed2.txt");
            assertNotNull(expected);
            bytes = new byte[buf._length];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Compressed Packed Write Test - compares to expected results from ETAC
     *
     * Writes the same sequence as TransportTest writes.
     */
    @SuppressWarnings("deprecation")
    @Test
    public void compressedPackedWriteTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(5);
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));
            // need to create ProtocolInt for Socket, so need to connect
            Channel initChannel = Transport.connect(connectOpts, error);
            assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 5);
            assertNotNull(channel);
            try
            {
                while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}

            // set channel state to ACTIVE
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;

            // set-up channel for compression
            ((RsslSocketChannel)channel)._sessionOutCompression = 1;
            ((RsslSocketChannel)channel)._sessionCompLevel = 9;
            ((RsslSocketChannel)channel)._compressor = ((RsslSocketChannel)channel)._ZlibCompressor;
            ((RsslSocketChannel)channel)._compressor.compressionLevel(((RsslSocketChannel)channel)._sessionCompLevel);

            // set _totalBytesQueued to 0 for no buffers queued
            ((RsslSocketChannel)channel)._totalBytesQueued = 0;

            // change high water mark so cannot exceed it
            ((RsslSocketChannel)channel)._highWaterMark = 10000;

            // get 500 byte buffer
            TransportBuffer sendbuffer = channel.getBuffer(500, true, error);

            ByteBuffer bb = sendbuffer.data();
            bb.put("packed test #1".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("packed test #2".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("final packing test #3".getBytes());

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(channel.write(sendbuffer, writeArgs, error) > 0);
            assertEquals(58, writeArgs.uncompressedBytesWritten());
            assertEquals(47, writeArgs.bytesWritten());

            // end of queue should contain packed message
            TransportBufferImpl buf = (TransportBufferImpl) ((RsslSocketChannel)channel)._highPriorityQueue._tail;

            // should equal output from ETAC
            byte[] expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/PackedCompressed1.txt");
            assertNotNull(expected);
            byte[] bytes = new byte[buf._length];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);

            sendbuffer = channel.getBuffer(500, true, error);

            bb = sendbuffer.data();
            bb.put("packing2 test #1".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("packing2 test #2".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            bb.put("packing3 final test #3".getBytes());
            assertTrue(channel.packBuffer(sendbuffer, error) > 0);

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(channel.write(sendbuffer, writeArgs, error) > 0);
            assertEquals(63, writeArgs.uncompressedBytesWritten());
            assertEquals(29, writeArgs.bytesWritten());

            // end of queue should contain packed message
            buf = (TransportBufferImpl) ((RsslSocketChannel)channel)._highPriorityQueue._tail;

            // should equal output from ETAC
            expected =
                    ParseHexFile.parse(BASE_TEST_DATA_DIR_NAME + "/PackedCompressed2.txt");
            assertNotNull(expected);
            bytes = new byte[buf._length];
            buf.data().get(bytes);
            assertArrayEquals(expected, bytes);
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Decompress compressed packed message.
     */
    @Test
    public void decompressPackedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Packed.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedPacked.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed packed message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(1, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(58, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // read compressed packed message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(1, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[2], msg); //first array element is initial RIPC message

            // read compressed packed message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[3], msg); //first array element is initial RIPC message

            // read compressed packed message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(1, readArgs.readRetVal());
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(63, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[4], msg); //first array element is initial RIPC message

            // read compressed packed message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(1, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[5], msg); //first array element is initial RIPC message

            // read compressed packed message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(0, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[6], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison.
             * Expect two packed headers, and four additional packed messages.
             */
            int headerAndDataLength = (firstPackedHeaderLength() * 2)
                                      + (additionalPackedHeaderLength() * 4) + expectedMessages[1].length
                                      + expectedMessages[2].length + expectedMessages[3].length
                                      + expectedMessages[4].length + expectedMessages[5].length
                                      + expectedMessages[6].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Decompress arbitrary length compressed fragmented message.
     */
    @Test
    public void decompressFragmentedMessage()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(22, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6138, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(1038, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // verify cumulative ReadArgs.uncompressedBytesRead()
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + additionalFragmentHeaderLength(consumerChannel) + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Decompress compressed fragmented message of size 6144 compression level 0.
     */
    @Test
    public void decompressFragmented6144L0Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6144.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6144L0.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(23, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(3, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6147, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect two RIPC messages.
             */
            int headerAndDataLength = Ripc.Lengths.HEADER * 2 + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Decompress compressed fragmented message of size 6144 compression level 9.
     */
    @Test
    public void decompressFragmented6144L9Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6144.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6144L9.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect one RIPC message.
             */
            int headerAndDataLength = Ripc.Lengths.HEADER + expectedMessages[1].length;
            assertEquals(337, readArgs.bytesRead()); // compressed bytes read
            assertEquals(headerAndDataLength, readArgs.uncompressedBytesRead());
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
     * Decompress compressed fragmented message of size 6131 compression level 0.
     */
    @Test
    public void decompressFragmented6131L0Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6131.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6131L0.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(15, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(3, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6134, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect two RIPC message.
             */
            int headerAndDataLength = (Ripc.Lengths.HEADER * 2) + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Decompress compressed fragmented message of size 6131 compression level 9.
     */
    @Test
    public void decompressFragmented6131L9Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6131.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6131L9.txt";

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify ReadArgs.bytesRead() and ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect one RIPC message.
             */
            int headerAndDataLength = Ripc.Lengths.HEADER + expectedMessages[1].length;
            assertEquals(337, readArgs.bytesRead()); // compressed bytes read
            assertEquals(headerAndDataLength, readArgs.uncompressedBytesRead());
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
     * Decompress two back-to-back compressed fragmented messages of size 6131
     * compression level 0.
     */
    @Test
    public void decompressFragmented6131L0X2Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6131.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6131L0X2.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read first compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(15, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(3, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6134, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            // read second compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(13, readArgs.readRetVal());
            assertEquals(expectedInput[2].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(3, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6134, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect one firstFragment followed by split message(compFrag),
             * and one additional fragment followed by split message(compFrag).
             */
            int headerAndDataLength = (Ripc.Lengths.HEADER * 4) + (expectedMessages[1].length * 2);
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Decompress compressed fragmented message of size 6145 compression level 0.
     */
    @Test
    public void decompressFragmented6145L0Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6145.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6145L0.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(40, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6137, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(23, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(7, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(23, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect one firstFragment (compressed),
             * followed by an additionalFragment (compressed), followed by an
             * additionalFragment (uncompressed).
             */
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + (additionalFragmentHeaderLength(consumerChannel) * 2)
                                      + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Decompress compressed fragmented message of size 6145 compression level 9.
     */
    @Test
    public void decompressFragmented6145L9Message()
    {
        final String expectedFile = BASE_TEST_DATA_DIR_NAME + "/Fragmented6145.txt";
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedFragmented6145L9.txt";

        NetworkReplay replay = null;

        int cumulativeUncompressedBytesRead = 0;

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
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectChannel(consumerChannel, DEFAULT_LISTEN_PORT); // connect to the NetworkReplay
            waitForChannelActive(consumerChannel); // wait for the channel to become active

            // initialize variables required for reading from a channel
            final ReadArgs readArgs = TransportFactory.createReadArgs();
            final Error error = TransportFactory.createError();
            TransportBuffer msgBuf = null;

            // read compressed fragmented message from the channel
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) == null);
            assertEquals(23, readArgs.readRetVal());
            assertEquals(expectedInput[1].length + Ripc.Lengths.HEADER, readArgs.bytesRead()); // compressed bytes read
            assertEquals(6138, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();
            assertTrue((msgBuf = consumerChannel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertEquals(0, readArgs.bytesRead()); // compressed bytes read
            assertEquals(23, readArgs.uncompressedBytesRead());
            cumulativeUncompressedBytesRead += readArgs.uncompressedBytesRead();

            assertEquals(msgBuf.data().limit() - msgBuf.data().position(), msgBuf.data().remaining());

            // compare with expected output
            byte[] msg = getBytesFromBuffer(msgBuf);
            printMessage(msg, consumerChannel);
            assertArrayEquals(expectedMessages[1], msg); //first array element is initial RIPC message

            /*
             * verify cumulative ReadArgs.uncompressedBytesRead()
             * Expected file does not contain the Transport headers, so add it
             * back on for the comparison. Expect one firstFragment (compressed),
             * followed by an additionalFragment (uncompressed).
             */
            int headerAndDataLength = firstFragmentHeaderLength(consumerChannel)
                                      + additionalFragmentHeaderLength(consumerChannel)
                                      + expectedMessages[1].length;
            assertEquals(headerAndDataLength, cumulativeUncompressedBytesRead);
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
     * Blocked connect test.
     *
     * Channel must be in ACTIVE state when connect call returns.
     */
    @Test
    public void blockedConnectTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/CompressedConsumerMessages.txt";
        NetworkReplay replay = null;

        try
        {
            initTransport(false); // initialize RSSL

            // load the messages to replay
            replay = parseReplayFile(inputFile);

            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel that reads from our NetworkReplay
            RsslSocketChannel consumerChannel = createReplaySocketChannel(replay);

            connectOpts.blocking(true);

            consumerChannel._scktChannel = new SocketHelper();
            assertEquals(TransportReturnCodes.SUCCESS, consumerChannel.connect(connectOpts, error));
            assertEquals(ChannelState.ACTIVE, consumerChannel.state());
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
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Blocked accept test.
     *
     * Channel must be in ACTIVE state when accept call returns.
     */
    @Test
    public void blockedAcceptTest()
    {
        final Error error = TransportFactory.createError();
        RsslSocketChannel channel = null;
        Server server = null;
        BindOptions bindOpts = TransportFactory.createBindOptions();
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();

        try
        {
            initTransport(false); // initialize RSSL

            bindOpts.serviceName("14002");
            bindOpts.channelsBlocking(true);
            bindOpts.serverBlocking(true);
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.protocolType(Codec.protocolType());

            // bind server
            assertTrue((server = Transport.bind(bindOpts, error)) != null);

            // start client thread to connect to this server
            (new Thread(new ClientConnector())).start();

            // wait until client is ready
            while (!_clientReady) {}

            assertTrue((channel = (RsslSocketChannel)server.accept(acceptOptions, error)) != null);
            assertEquals(ChannelState.ACTIVE, channel.state());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Server buffer usage test.
     */
    @Test
    public void serverBufferUsageTest()
    {
        final Error error = TransportFactory.createError();
        RsslSocketChannel channel = null;
        Server server = null;
        BindOptions bindOpts = TransportFactory.createBindOptions();
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();

        try
        {
            initTransport(false); // initialize RSSL

            bindOpts.serviceName("14002");
            bindOpts.channelsBlocking(true);
            bindOpts.serverBlocking(true);
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.protocolType(Codec.protocolType());
            bindOpts.maxOutputBuffers(10);
            bindOpts.sharedPoolSize(100);
            bindOpts.guaranteedOutputBuffers(5);
            bindOpts.maxFragmentSize(6144);

            // bind server
            assertTrue((server = Transport.bind(bindOpts, error)) != null);

            // start client thread to connect to this server
            (new Thread(new ClientConnector())).start();

            // wait until client is ready
            while (!_clientReady) {}

            assertTrue((channel = (RsslSocketChannel)server.accept(acceptOptions, error)) != null);
            assertEquals(ChannelState.ACTIVE, channel.state());

            // Make sure buffer usage is equal to how many buffers are actually used
            @SuppressWarnings("unused")
            TransportBuffer tBuffer;

            for  (int i = 0; i < 5; ++i)
            {
                tBuffer = channel.getBuffer(6142, false, error);
            }

            int ret = server.bufferUsage(error);
            assertEquals(0, ret);

            for  (int i = 5; i < 10; ++i)
            {
                tBuffer = channel.getBuffer(6142, false, error);
            }

            ret = server.bufferUsage(error);
            assertEquals(5, ret);

            // Close channel and server
            channel.close(error);
            server.close(error);
        }
        catch (Exception e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            _doReadWrite = false;
            if (channel != null)
                channel.close(error);
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Blocked read/write test.
     *
     * Correct data must be returned after one call to read/write.
     *
     * Includes tests for buffer usage - Checks if buffers used = output of bufferUsage()
     */
    @Test
    public void blockedReadWriteTest()
    {
        final Error error = TransportFactory.createError();
        RsslSocketChannel channel = null;
        Server server = null;
        BindOptions bindOpts = TransportFactory.createBindOptions();
        AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
        ReadArgs readArgs = TransportFactory.createReadArgs();
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
            initTransport(false); // initialize RSSL

            bindOpts.serviceName("14002");
            bindOpts.channelsBlocking(true);
            bindOpts.serverBlocking(true);
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.protocolType(Codec.protocolType());

            // bind server
            assertTrue((server = Transport.bind(bindOpts, error)) != null);

            // start client thread to connect to this server
            (new Thread(new ClientConnector())).start();

            // wait until client is ready
            while (!_clientReady) {}

            assertTrue((channel = (RsslSocketChannel)server.accept(acceptOptions, error)) != null);
            assertEquals(ChannelState.ACTIVE, channel.state());

            // tell client to do read/write
            _doReadWrite = true;

            Thread.sleep(1000);
            TransportBuffer readBuf = null;
            assertTrue((readBuf = channel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertArrayEquals("transport blocking read/write test 1".getBytes(), getBytesFromBuffer(readBuf));
            Thread.sleep(1000); // delay so the other side blocks on read
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
            TransportBuffer writeBuf = null;
            assertTrue((writeBuf = channel.getBuffer(100, false, error)) != null);
            writeBuf.data().put("transport blocking read/write test 2".getBytes());
            assertEquals(0, channel.write(writeBuf, writeArgs, error));
            assertTrue((readBuf = channel.read(readArgs, error)) != null);
            assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
            assertArrayEquals("got it!!!".getBytes(), getBytesFromBuffer(readBuf));
        }
        catch (Exception e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            _doReadWrite = false;
            if (channel != null)
                channel.close(error);
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Blocked connect test with RIPC VERSION12 fallback.
     *
     * Channel must be in ACTIVE state when connect call returns.
     */
    @Test
    public void blockedConnectRIPCVersion12FallbackTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        IpcProtocolManager ipcProtocolManager = new IpcProtocolManager();

        try
        {
            initTransport(false); // initialize RSSL

            // start RIPC version fallback server to connect to
            (new Thread(new RIPCFallbackServer())).start();

            connectOpts.unifiedNetworkInfo().address("localhost");
            connectOpts.unifiedNetworkInfo().serviceName("14002");
            connectOpts.majorVersion(Codec.majorVersion());
            connectOpts.minorVersion(Codec.minorVersion());
            connectOpts.protocolType(Codec.protocolType());
            connectOpts.blocking(true);

            // set starting RIPC version to be VERSION12
            ipcProtocolManager.startingVersion(Ripc.ConnectionVersions.VERSION12);

            // wait until server is ready
            while (!_serverReady) {}

            assertTrue((channel = Transport.connect(connectOpts, error)) != null);
            assertEquals(ChannelState.ACTIVE, channel.state());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            // set starting RIPC version back to VERSION13
            ipcProtocolManager.startingVersion(Ripc.ConnectionVersions.VERSION13);
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Blocked connect test with RIPC VERSION11 fallback.
     *
     * Channel must be in ACTIVE state when connect call returns.
     */
    @Test
    public void blockedConnectRIPCVersion11FallbackTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        IpcProtocolManager ipcProtocolManager = new IpcProtocolManager();

        try
        {
            initTransport(false); // initialize RSSL

            // start RIPC version fallback server to connect to
            (new Thread(new RIPCFallbackServer())).start();

            connectOpts.unifiedNetworkInfo().address("localhost");
            connectOpts.unifiedNetworkInfo().serviceName("14002");
            connectOpts.majorVersion(Codec.majorVersion());
            connectOpts.minorVersion(Codec.minorVersion());
            connectOpts.protocolType(Codec.protocolType());
            connectOpts.blocking(true);

            // set starting RIPC version to be VERSION11
            ipcProtocolManager.startingVersion(Ripc.ConnectionVersions.VERSION11);

            // wait until server is ready
            while (!_serverReady) {}

            assertTrue((channel = Transport.connect(connectOpts, error)) != null);
            assertEquals(ChannelState.ACTIVE, channel.state());
        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            // set starting RIPC version back to VERSION13
            ipcProtocolManager.startingVersion(Ripc.ConnectionVersions.VERSION13);
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Verify NIC binding on the consumer side
     * i.e RsslSocketChannel class must create InetSocketAddress with the
     * specified interface name and call bind on the socket channel
     */
    @Test
    public void consumerNICBindTest()
    {
        final String inputFile = RIPC_CONNECT_ACK_HANDSHAKE_FILE;
        NetworkReplay replay = null;

        ConnectOptions connectOpts = getConnectOptions("localhost", DEFAULT_LISTEN_PORT_AS_STRING);

        // set the interface name on consumer
        connectOpts.unifiedNetworkInfo().interfaceName("127.0.0.1");

        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel channel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            assertNotNull(channel = Transport.connect(connectOpts, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            server.waitForAcceptable();
            server.acceptSocket();

            assertEquals(TransportReturnCodes.CHAN_INIT_IN_PROGRESS, channel.init(inProg, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            // read ConnectReq and send ConnectAck
            server.waitForReadable();
            server.readMessageFromSocket();
            server.writeMessageToSocket(replay.read());

            Thread.sleep(1000);

            // call channel.init to read ConnectAck
            assertEquals(TransportReturnCodes.SUCCESS, channel.init(inProg, error));
            assertEquals(ChannelState.ACTIVE, channel.state());
        }
        catch (IOException | InterruptedException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, channel, server, null, error);
        }
    }

    /*
     * Verify NIC binding on the provider side
     * i.e SocketServer class must create InetSocketAddress with the
     * specified interface name use this with bind()
     */
    @Test
    public void providerNICBindTest()
    {
        String inputFile = RIPC_CONNECT_REQ_HANDSHAKE_FILE;

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        int recvBufSize = 50000;
        int sendBufSize = 40000;
        BindOptions bOpts = TransportFactory.createBindOptions();

        bOpts.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bOpts.connectionType(ConnectionTypes.SOCKET);

        bOpts.sysRecvBufSize(recvBufSize);
        AcceptOptions aOpts = TransportFactory.createAcceptOptions();
        aOpts.sysSendBufSize(sendBufSize);

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the ETAJ server
            selector = Selector.open();
            server = serverBind(selector, bOpts, error);

            // have our test client connect to ETAJ server
            client.connect();

            // have the ETAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, aOpts, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our ETAJ server process the ConnectReq and send a ConnectAck
            // verify that the ETAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, null, selector, error);
        }
    }

    public RsslSocketChannel getNetworkReplayChannel(Protocol transport, int numBuffers)
    {
        final Error error = TransportFactory.createError();
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";
        final NetworkReplay replay = NetworkReplayFactory.create();

        ConnectOptions opts = TransportFactory.createConnectOptions();

        opts.userSpecObject("TEST CHANNEL");
        opts.unifiedNetworkInfo().address("localhost");
        opts.unifiedNetworkInfo().serviceName("4323");
        opts.majorVersion(Codec.majorVersion());
        opts.minorVersion(Codec.minorVersion());
        opts.protocolType(Codec.protocolType());
        opts.guaranteedOutputBuffers(numBuffers);

        try
        {
            replay.parseFile(inputFile);

            replay.startListener(DEFAULT_LISTEN_PORT);

            RsslSocketChannel channel = new RsslSocketChannel()
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
                @Override
                public int close(Error error)
                {
                    int ret = TransportReturnCodes.SUCCESS;

                    // first set channel state to closed
                    try
                    {
                        if (_state != ChannelState.CLOSED)
                        {
                            _state = ChannelState.CLOSED;
                            _scktChannel.close();
                        }
                        else
                        {
                            error.channel(this);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("socket channel is already closed ");
                            ret = TransportReturnCodes.FAILURE;
                        }
                    }
                    catch (IOException e)
                    {
                        error.channel(this);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("socket channel close failed ");
                        ret = TransportReturnCodes.FAILURE;
                    }
                    finally
                    {
                        if (ret == TransportReturnCodes.FAILURE)
                        {
                            return ret;
                        }
                    }

                    return ret;
                }

            };

            channel._transport = (SocketProtocol) transport;

            channel._scktChannel = new SocketHelper();
            channel.connect(opts, error);
            assertTrue(channel.state() == ChannelState.INITIALIZING);

            waitForChannelActive(channel); // wait for the channel to become active
            return channel;

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
        }
        return null;
    }

    private ConnectOptions getDefaultConnectOptions()
    {
        ConnectOptions connectOpts = TransportFactory.createConnectOptions();
        connectOpts.userSpecObject("TEST CHANNEL");
        connectOpts.unifiedNetworkInfo().address("localhost");
        connectOpts.unifiedNetworkInfo().serviceName("4323");
        connectOpts.majorVersion(Codec.majorVersion());
        connectOpts.minorVersion(Codec.minorVersion());
        connectOpts.protocolType(Codec.protocolType());
        return connectOpts;
    }

    private ConnectOptions getConnectOptions(String host, String port)
    {
        ConnectOptions connectOpts = TransportFactory.createConnectOptions();
        connectOpts.userSpecObject("TEST CHANNEL");
        connectOpts.unifiedNetworkInfo().address(host);
        connectOpts.unifiedNetworkInfo().serviceName(port);
        connectOpts.majorVersion(Codec.majorVersion());
        connectOpts.minorVersion(Codec.minorVersion());
        connectOpts.protocolType(Codec.protocolType());
        return connectOpts;
    }

    // used for client to server testing
    private class ClientConnector implements Runnable
    {
        public void run()
        {
            final Error error = TransportFactory.createError();
            ConnectOptions connectOpts = TransportFactory.createConnectOptions();
            RsslSocketChannel channel = null;

            try
            {
                initTransport(false); // initialize RSSL

                connectOpts.unifiedNetworkInfo().address("localhost");
                connectOpts.unifiedNetworkInfo().serviceName("14002");
                connectOpts.majorVersion(Codec.majorVersion());
                connectOpts.minorVersion(Codec.minorVersion());
                connectOpts.protocolType(Codec.protocolType());
                connectOpts.blocking(true);

                _clientReady = true; // tell server that client is ready
                assertTrue((channel = (RsslSocketChannel)Transport.connect(connectOpts, error)) != null);
                assertEquals(ChannelState.ACTIVE, channel.state());
                Thread.sleep(1000);
                // do read/write if necessary
                if (_doReadWrite)
                {
                    Thread.sleep(1000); // delay so the other side blocks on read
                    ReadArgs readArgs = TransportFactory.createReadArgs();
                    WriteArgs writeArgs = TransportFactory.createWriteArgs();
                    writeArgs.priority(WritePriorities.HIGH);
                    writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
                    TransportBuffer writeBuf = channel.getBuffer(100, false, error);
                    writeBuf.data().put("transport blocking read/write test 1".getBytes());
                    assertEquals(0, channel.write(writeBuf, writeArgs, error));
                    TransportBuffer readBuf = null;
                    assertTrue((readBuf = channel.read(readArgs, error)) != null);
                    assertEquals(TransportReturnCodes.SUCCESS, readArgs.readRetVal());
                    assertArrayEquals("transport blocking read/write test 2".getBytes(), getBytesFromBuffer(readBuf));
                    Thread.sleep(1000); // delay so the other side blocks on read
                    writeBuf = channel.getBuffer(100, false, error);
                    writeBuf.data().put("got it!!!".getBytes());
                    assertTrue(channel.write(writeBuf, writeArgs, error) == 0);
                }
            }
            catch (Exception e)
            {
                fail(e.getLocalizedMessage());
            }
            finally
            {
                _clientReady = false;
                if (channel != null)
                    channel.close(error);
                assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
            }
        }
    }

    // used for blocking RIPC version fallback testing
    private class RIPCFallbackServer implements Runnable
    {
        public void run()
        {
            final Error error = TransportFactory.createError();
            BindOptions bindOpts = TransportFactory.createBindOptions();
            AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
            Server server = null;
            RsslSocketChannel channel = null;

            try
            {
                initTransport(false); // initialize RSSL

                bindOpts.serviceName("14002");
                bindOpts.majorVersion(Codec.majorVersion());
                bindOpts.minorVersion(Codec.minorVersion());
                bindOpts.protocolType(Codec.protocolType());
                bindOpts.channelsBlocking(true);
                bindOpts.serverBlocking(true);

                // bind server
                assertTrue((server = Transport.bind(bindOpts, error)) != null);

                _serverReady = true; // tell client that server is ready

                // accept connection
                assertTrue((channel = (RsslSocketChannel)server.accept(acceptOptions, error)) != null);
                assertEquals(ChannelState.ACTIVE, channel.state());
            }
            catch (Exception e)
            {
                fail(e.getLocalizedMessage());
            }
            finally
            {
                _serverReady = false;
                if (channel != null)
                    channel.close(error);
                if (server != null)
                    server.close(error);
                assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
            }
        }
    }

    /*
     *
     * @return RsslBindOptions populated with default options.
     */
    private BindOptions getDefaultBindOptions()
    {
        BindOptions bindOpts = TransportFactory.createBindOptions();
        bindOpts.userSpecObject("TEST SERVER");
        bindOpts.serviceName("14005");
        bindOpts.majorVersion(Codec.majorVersion());
        bindOpts.minorVersion(Codec.minorVersion());
        bindOpts.protocolType(Codec.protocolType());
        return bindOpts;
    }

    void getAndVerifyServerInfo(Server server, ServerInfo serverInfo, Error error, int current, int peak)
    {
        assertEquals(current, server.bufferUsage(error));
        assertEquals(TransportReturnCodes.SUCCESS, server.info(serverInfo, error));
        assertNotNull(serverInfo);
        assertEquals(current, serverInfo.currentBufferUsage());
        assertEquals(peak, serverInfo.peakBufferUsage());
    }

    /*
     * Verify that a invalid Ioctl code returns failure.
     */
    @Test
    public void ioctlInvalidCodeTest()
    {
        final Error error = TransportFactory.createError();

        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.sharedPoolSize(3);

        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);

        Server server = null;

        try
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                          + error.sysError(), server);

            // test Server.ioctl(int code, Object object, Error error)
            assertEquals(TransportReturnCodes.FAILURE, server.ioctl(99999, null, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertTrue("Code is not valid.".equals(error.text()));

            // test Server.ioctl(int code, int value, Error error)
            assertEquals(TransportReturnCodes.FAILURE, server.ioctl(99999, 1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertTrue("Code is not valid.".equals(error.text()));

            // create the RsslSocketChannel to test
            RsslSocketChannel scktChnl = new RsslSocketChannel();

            // set channel state to ACTIVE
            scktChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            scktChnl._totalBytesQueued = 0;

            // test Channel.ioctl(int code, int value, Error error)
            assertEquals(TransportReturnCodes.FAILURE, scktChnl.ioctl(99999, 1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertTrue("Code is not valid.".equals(error.text()));

            // test Channel.ioctl(int code, Object value, Error error)
            assertEquals(TransportReturnCodes.FAILURE, scktChnl.ioctl(99999, "non null", error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertTrue("Code is not valid.".equals(error.text()));
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Verify that an invalid IoctlCodes.COMPONENT_INFO value returns failure.
     */
    @Test
    public void ioctlInvalidComponentInfoValueTest()
    {
        final Error error = TransportFactory.createError();

        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.sharedPoolSize(3);

        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);

        Server server = null;

        try
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                          + error.sysError(), server);

            // Server.ioctl(IoctlCodes.COMPONENT_INFO, Object object, Error
            // error)
            assertEquals(TransportReturnCodes.FAILURE, server.ioctl(IoctlCodes.COMPONENT_INFO, server, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertTrue("Code is not valid.".equals(error.text()));
            assertEquals(TransportReturnCodes.FAILURE, server.ioctl(IoctlCodes.COMPONENT_INFO, null, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertTrue("Code is not valid.".equals(error.text()));
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Create a SocketProtocol with empty global buffer pool, and
     * RsslSocketChannel with a set number of guaranteedOutputBuffers. Grow and
     * shrink the guaranteedOutputBuffer and verify that the size is adjusted
     * correctly as well as the global buffer pool grows as expected when the
     * guaranteedOutputBuffer is shrunk. Also verify that maxOutputBuffers grows
     * when guaranteedOutputBuffer grows past maxOutputBuffers.
     */
    @Test
    public void ioctlNumGuaranteedBuffersTest()
    {
        int initialBufferCount = 20, bufferCount = 20;
        int growCount = 10;
        int shrinkCount = 10;

        final Error error = TransportFactory.createError();
        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.guaranteedOutputBuffers(initialBufferCount);

        // setup the RsslSocketChannel
        SocketProtocol transport = new SocketProtocol(opts);
        RsslSocketChannel rsslChnl = new RsslSocketChannel(transport, transport._channelPool);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._writeLock = new DummyLock();
        rsslChnl.dataFromOptions(opts);
        rsslChnl.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        // verify setup.
        Pool bufferPool = transport.getPool(rsslChnl._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // shrink guaranteedOutputBuffers
        bufferCount = initialBufferCount - shrinkCount;
        assertEquals(bufferCount, rsslChnl.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, bufferCount, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(shrinkCount, bufferPool.size());
        assertEquals(bufferCount, rsslChnl._availableBuffers.size());
        assertEquals(bufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // grow guaranteedOutputBuffers
        bufferCount = initialBufferCount + growCount;
        assertEquals(bufferCount, rsslChnl.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, bufferCount, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(0, bufferPool.size()); // should be drained
        assertEquals(bufferCount, rsslChnl._availableBuffers.size());
        assertEquals(bufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(bufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // call ioctl with  NUM_GUARANTEED_BUFFERS unchanged.
        assertEquals(bufferCount, rsslChnl.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, bufferCount, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(0, bufferPool.size()); // should be drained
        assertEquals(bufferCount, rsslChnl._availableBuffers.size());
        assertEquals(bufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(bufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // shrink guaranteedOutputBuffers to Zero
        assertEquals(0, rsslChnl.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, 0, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(bufferCount, bufferPool.size());
        assertEquals(0, rsslChnl._availableBuffers.size());
        assertEquals(0, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(bufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // try using negative number
        assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, -1, error));
        assertEquals(TransportReturnCodes.FAILURE, error.errorId());
    }

    /*
     * Create a SocketProtocol with empty global buffer pool, and
     * RsslSocketChannel with a set number of guaranteedOutputBuffers. While
     * some buffers are allocated, shrink the guaranteedOutputBuffer to less
     * than what is _availableBuffers. Verify that it shrank to what could be
     * freed from _availableBuffers and not to the shrink size specified.
     * Initially 25 available, use 15 buffers, then try to shrink by 20,
     * however, since only 10 buffers are available, it's actually shrunk by 10.
     * Post shrink, _availableBuffers would be 0, global buffers would be 10,
     * and guraranteedOutputBuffers would be 15. Verify that maxOutputBuffers
     * does not change.
     */
    @Test
    public void ioctlNumGuaranteedBuffersWithUsedBuffersTest()
    {
        int initialBufferCount = 25, bufferCount = 25;
        int shrinkCount = 20;
        int inUseCount = 15;
        int postShrinkBufferCount = 15;
        int postShrinkGlobalBufferCount = 10;

        final Error error = TransportFactory.createError();
        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.guaranteedOutputBuffers(initialBufferCount);

        // setup the RsslSocketChannel
        SocketProtocol transport = new SocketProtocol(opts);
        RsslSocketChannel rsslChnl = new RsslSocketChannel(transport, transport._channelPool);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._writeLock = new DummyLock();
        rsslChnl.dataFromOptions(opts);
        rsslChnl.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        // verify setup.
        Pool bufferPool = transport.getPool(rsslChnl._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // allocate inUseCount buffers
        for(int i = 0; i < inUseCount; i++)
        {
            assertNotNull(rsslChnl.getBuffer((rsslChnl._internalMaxFragmentSize - 3), false, error));
            assertEquals(0, error.errorId());
        }

        // shrink guaranteedOutputBuffers, expect it to return a different value (postShrinkBufferCount) since there were not.
        bufferCount = initialBufferCount - shrinkCount;
        assertEquals(postShrinkBufferCount, rsslChnl.ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS, bufferCount, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(postShrinkGlobalBufferCount, bufferPool.size());
        assertEquals(0, rsslChnl._availableBuffers.size());
        assertEquals(postShrinkBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());
    }

    /*
     * Test setting MAX_NUM_BUFFERS to various values. Verify that it changes as expected.
     */
    @Test
    public void ioctlMaxNumBuffersTest()
    {
        int initialBufferCount = 50;
        int newMaxNumBuffers = 100;
        int newMinNumBuffers = 20;

        final Error error = TransportFactory.createError();
        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.guaranteedOutputBuffers(initialBufferCount);

        // setup the RsslSocketChannel
        SocketProtocol transport = new SocketProtocol(opts);
        RsslSocketChannel rsslChnl = new RsslSocketChannel(transport, transport._channelPool);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._writeLock = new DummyLock();
        rsslChnl.dataFromOptions(opts);

        // verify setup.
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // set MAX_NUM_BUFFERS to the same value.
        assertEquals(initialBufferCount, rsslChnl.ioctl(IoctlCodes.MAX_NUM_BUFFERS, initialBufferCount, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // modify MAX_NUM_BUFFERS to a larger value.
        assertEquals(newMaxNumBuffers, rsslChnl.ioctl(IoctlCodes.MAX_NUM_BUFFERS, newMaxNumBuffers, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(newMaxNumBuffers, rsslChnl._channelInfo.maxOutputBuffers());

        // modify MAX_NUM_BUFFERS to a smaller value.
        assertEquals(initialBufferCount + 5, rsslChnl.ioctl(IoctlCodes.MAX_NUM_BUFFERS, initialBufferCount + 5, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount + 5, rsslChnl._channelInfo.maxOutputBuffers());

        // modify MAX_NUM_BUFFERS to a value smaller than guaranteedOutputBuffers
        assertEquals(initialBufferCount, rsslChnl.ioctl(IoctlCodes.MAX_NUM_BUFFERS, newMinNumBuffers, error));
        assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl._channelInfo.maxOutputBuffers());

        // modify MAX_NUM_BUFFERS to a negative value, expect it to fail.
        assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.MAX_NUM_BUFFERS, -1, error));
        assertEquals(TransportReturnCodes.FAILURE, error.errorId());
    }

    /*
     * Verify that the ServerInfo Peak Buffer count is correct and can be reset.
     * This test will create a ServerSocket (via Transport.bind()), then get
     * some SocketBuffers from the (shared) server pool and verify that the
     * ServerInfo reports the correct usage. Then
     * IoctlCodes.SERVER_PEAK_BUF_RESET will be used to reset the peak buffer
     * count and ServerInfo will again be verified.
     */
    @Test
    public void ioctlServerPeakBufferResetTest()
    {
        final Error error = TransportFactory.createError();

        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.sharedPoolSize(3);

        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);

        Server server = null;
        ServerInfo serverInfo = TransportFactory.createServerInfo();
        SocketBuffer[] socketBuffer = new SocketBuffer[5];

        try
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                          + error.sysError(), server);

            getAndVerifyServerInfo(server, serverInfo, error, 0, 0);

            socketBuffer[0] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[0]);
            getAndVerifyServerInfo(server, serverInfo, error, 1, 1);

            socketBuffer[1] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[1]);
            getAndVerifyServerInfo(server, serverInfo, error, 2, 2);

            socketBuffer[2] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[2]);
            getAndVerifyServerInfo(server, serverInfo, error, 3, 3);

            // attempt to get more than shared pool size
            socketBuffer[3] = ((ServerImpl)server).getBufferFromServerPool();
            assertNull(socketBuffer[3]);
            getAndVerifyServerInfo(server, serverInfo, error, 3, 3);

            socketBuffer[2].returnToPool();
            getAndVerifyServerInfo(server, serverInfo, error, 2, 3);

            // reset the server's peak buffer count
            assertEquals(TransportReturnCodes.SUCCESS, server.ioctl(IoctlCodes.SERVER_PEAK_BUF_RESET, null, error));
            getAndVerifyServerInfo(server, serverInfo, error, 2, 2);

            socketBuffer[1].returnToPool();
            getAndVerifyServerInfo(server, serverInfo, error, 1, 2);

            socketBuffer[0].returnToPool();
            getAndVerifyServerInfo(server, serverInfo, error, 0, 2);

            // reset the server's peak buffer count
            assertEquals(TransportReturnCodes.SUCCESS, server.ioctl(IoctlCodes.SERVER_PEAK_BUF_RESET, null, error));
            getAndVerifyServerInfo(server, serverInfo, error, 0, 0);
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Verify that the Server.sharedPool and sharedPoolSize
     * can be modified
     * with Channel.ioctl and IoctlCodes.SERVER_NUM_POOL_BUFFER.
     * This test will allocate several buffers from the sharedPool,
     * then increase and reduce the sharedPoolSize. It verifies that
     * the sharedPool grows and shrinks correctly, and that the global
     * pool gets populated correctly when the sharedPool shrinks.
     */
    @Test
    public void ioctlServerNumPoolBuffersTest()
    {
        final Error error = TransportFactory.createError();
        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.sharedPoolSize(3);
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);
        Server server = null;
        SocketBuffer[] socketBuffer = new SocketBuffer[6];
        try
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                          + error.sysError(), server);

            ServerImpl socketServer = (ServerImpl)server;
            // The "global" pool.
            Pool bufferPool = socketServer._transport.getPool(socketServer.bufferSize());
            assertEquals(3, socketServer._bindOpts._sharedPoolSize);
            assertEquals(0, socketServer._sharedPool.size());
            assertEquals(0, bufferPool.size());

            /*
             * allocate four SocketBuffers from sharedPool. The fourth should be
             * null since it passes the sharedPoolSize.
             */
            assertEquals(0, socketServer._sharedPool.size()); // allocated on demand.
            socketBuffer[0] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[0]);
            socketBuffer[1] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[1]);
            assertTrue(socketBuffer[0] != socketBuffer[1]);
            socketBuffer[2] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[2]);
            assertTrue(socketBuffer[2] != socketBuffer[0]);
            assertTrue(socketBuffer[2] != socketBuffer[1]);
            socketBuffer[3] = ((ServerImpl)server).getBufferFromServerPool();
            assertNull(socketBuffer[3]);

            assertEquals(3, server.bufferUsage(error));
            assertEquals(3, socketServer._bindOpts._sharedPoolSize);
            assertEquals(0, socketServer._sharedPool.size());
            assertEquals(0, bufferPool.size());

            /*
             * use IoctlCodes.SERVER_NUM_POOL_BUFFERS to increase the size of
             * the sharedPoolSize to 5, then allocate three more SocketBuffers,
             * the third should fail since the sharedPoolSize is exceeded.
             */
            assertEquals(5, server.ioctl(IoctlCodes.SERVER_NUM_POOL_BUFFERS, 5, error));
            assertEquals(5, socketServer._bindOpts._sharedPoolSize);
            assertEquals(0, socketServer._sharedPool.size());
            assertEquals(0, bufferPool.size());

            socketBuffer[3] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[3]);
            assertTrue(socketBuffer[3] != socketBuffer[0]);
            assertTrue(socketBuffer[3] != socketBuffer[1]);
            assertTrue(socketBuffer[3] != socketBuffer[2]);
            socketBuffer[4] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[4]);
            assertTrue(socketBuffer[4] != socketBuffer[0]);
            assertTrue(socketBuffer[4] != socketBuffer[1]);
            assertTrue(socketBuffer[4] != socketBuffer[2]);
            assertTrue(socketBuffer[4] != socketBuffer[3]);
            socketBuffer[5] = ((ServerImpl)server).getBufferFromServerPool();
            assertNull(socketBuffer[5]);
            assertEquals(5, server.bufferUsage(error));

            /*
             * use IoctlCodes.SERVER_NUM_POOL_BUFFERS to decrease the size of
             * the sharedPoolSize to 4, then allocate one more SocketBuffers,
             * which should fail since the sharedPoolSize is exceeded.
             */
            assertEquals(5, server.ioctl(IoctlCodes.SERVER_NUM_POOL_BUFFERS, 4, error));
            // sharedPoolSize did not change since all buffers are in use.
            assertEquals(5, socketServer._bindOpts._sharedPoolSize);
            assertEquals(0, socketServer._sharedPool.size());
            assertEquals(0, bufferPool.size());
            socketBuffer[5] = ((ServerImpl)server).getBufferFromServerPool();
            assertNull(socketBuffer[5]);
            assertEquals(5, server.bufferUsage(error));

            /*
             * release two SocketBuffers, then allocate one more SocketBuffer
             * which should succeeded since the SharedPoolSize is not exceeded.
             */
            socketBuffer[4].returnToPool();
            socketBuffer[4] = null;
            assertEquals(4, server.bufferUsage(error));
            socketBuffer[3].returnToPool();
            socketBuffer[3] = null;
            assertEquals(3, server.bufferUsage(error));
            assertEquals(5, socketServer._bindOpts._sharedPoolSize);
            assertEquals(2, socketServer._sharedPool.size());
            assertEquals(0, bufferPool.size());

            socketBuffer[3] = ((ServerImpl)server).getBufferFromServerPool();
            assertNotNull(socketBuffer[3]);
            assertTrue(socketBuffer[3] != socketBuffer[0]);
            assertTrue(socketBuffer[3] != socketBuffer[1]);
            assertTrue(socketBuffer[3] != socketBuffer[2]);

            assertEquals(4, server.bufferUsage(error));
            assertEquals(5, socketServer._bindOpts._sharedPoolSize);
            assertEquals(1, socketServer._sharedPool.size());
            assertEquals(0, bufferPool.size());

            /*
             * decrease the sharedPoolSize to 2. Since there is only one free buffer in sharedPool,
             * the sharedPoolSize should be reduced to 4.
             */
            assertEquals(4, server.ioctl(IoctlCodes.SERVER_NUM_POOL_BUFFERS, 2, error));
            // sharedPoolSize did not change since all buffers are in use.
            assertEquals(4, socketServer._bindOpts._sharedPoolSize);
            assertEquals(0, socketServer._sharedPool.size());
            assertEquals(1, bufferPool.size());

            /*
             * release all of the buffers, then decrease the sharedPoolSize to 2. Since there are four
             * free buffers in sharedPool, the sharedPoolSize should be reduced to 2, and the global
             * pool will increase from 1 to 3.
             */
            socketBuffer[3].returnToPool();
            socketBuffer[3] = null;
            assertEquals(3, server.bufferUsage(error));
            socketBuffer[2].returnToPool();
            socketBuffer[2] = null;
            assertEquals(2, server.bufferUsage(error));
            socketBuffer[1].returnToPool();
            socketBuffer[1] = null;
            assertEquals(1, server.bufferUsage(error));
            socketBuffer[0].returnToPool();
            socketBuffer[0] = null;
            assertEquals(0, server.bufferUsage(error));

            assertEquals(2, server.ioctl(IoctlCodes.SERVER_NUM_POOL_BUFFERS, 2, error));
            assertEquals(2, socketServer._bindOpts._sharedPoolSize);
            assertEquals(2, socketServer._sharedPool.size());
            assertEquals(3, bufferPool.size());

            // call ioctl to set SERVER_NUM_POOL_BUFFERS to the same value that it already is.
            assertEquals(2, server.ioctl(IoctlCodes.SERVER_NUM_POOL_BUFFERS, 2, error));
            assertEquals(2, socketServer._bindOpts._sharedPoolSize);

        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Use Channel.ioctl() to change Priority Flush Order. Perform several
     * priority writes of High, Medium and Low. Call flush method with
     * java.nio.channels.SocketChannel.write() returns all bytes sent.
     *
     * Expected Result: Buffers written per priority flush strategy and flush
     * method returns 0.
     */
    @Test
    public void ioctlPriorityFlushOrderTest()
    {
        String testDataHigh = "High";
        String testDataMedium = "Medium";
        String testDataLow = "Low";
        int bufLenHigh = testDataHigh.length();
        int bufLenMedium = testDataMedium.length();
        int bufLenLow = testDataLow.length();
        ByteBuffer byteBufHigh = ByteBuffer.allocate(bufLenHigh);
        ByteBuffer byteBufMedium = ByteBuffer.allocate(bufLenMedium);
        ByteBuffer byteBufLow = ByteBuffer.allocate(bufLenLow);
        byteBufHigh.put(testDataHigh.getBytes());
        byteBufMedium.put(testDataMedium.getBytes());
        byteBufLow.put(testDataLow.getBytes());
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;

            TransportBufferImpl [] transBufHigh = new TransportBufferImpl[4];
            TransportBufferImpl [] transBufMedium = new TransportBufferImpl[4];
            TransportBufferImpl [] transBufLow = new TransportBufferImpl[4];
            for (int i=0; i<4; i++)
            {
                transBufHigh[i] = new TransportBufferImpl(bufLenHigh+3);
                transBufHigh[i]._data.position(3);
                byteBufHigh.flip();
                transBufHigh[i].data().put(byteBufHigh);
                transBufHigh[i]._isWriteBuffer = true;

                transBufMedium[i] = new TransportBufferImpl(bufLenMedium+3);
                transBufMedium[i]._data.position(3);
                byteBufMedium.flip();
                transBufMedium[i].data().put(byteBufMedium);
                transBufMedium[i]._isWriteBuffer = true;

                transBufLow[i] = new TransportBufferImpl(bufLenLow+3);
                transBufLow[i]._data.position(3);
                byteBufLow.flip();
                transBufLow[i].data().put(byteBufLow);
                transBufLow[i]._isWriteBuffer = true;
            }

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // Use Channel.ioctl() to change Priority Flush Order to something invalid.
            // (Missing H)
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.PRIORITY_FLUSH_ORDER, "LMLM", error));

            // Use Channel.ioctl() to change Priority Flush Order to something invalid.
            // (Missing M)
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.PRIORITY_FLUSH_ORDER, "LHHL", error));

            // Use Channel.ioctl() to change Priority Flush Order to something invalid.
            // (larger than 32 characters)
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.PRIORITY_FLUSH_ORDER, "HMLHMLHLMHLMHLMHLHLMHLMHLMHLMHLMHLMHLMHL", error));

            // Use Channel.ioctl() to change Priority Flush Order
            assertEquals(TransportReturnCodes.SUCCESS, rsslChnl.ioctl(IoctlCodes.PRIORITY_FLUSH_ORDER, "LMLHLM", error));

            int cumulativeBytesQueued = 0, writeReturnVal = 0;

            // queue several buffers by calling the write method several times
            // with no write flags
            for (int i=0; i<4; i++)
            {
                // queue several buffers by calling the write method several times with no write flags
                cumulativeBytesQueued += (bufLenHigh + 3);
                writeArgs.priority(WritePriorities.HIGH);
                writeArgs.flags(WriteFlags.NO_FLAGS);
                writeReturnVal = rsslChnl.write(transBufHigh[i], writeArgs, error);
                System.out.println("writeReturnVal, i "+ writeReturnVal + " " + i);
                // write return value should be cumulative bytes queued
                assertEquals(cumulativeBytesQueued, writeReturnVal);
                // bytesWritten should be scktBufHigh.getLength()
                assertTrue(writeArgs.bytesWritten() == transBufHigh[i].data().limit()-transBufHigh[i].data().position());
                cumulativeBytesQueued += (bufLenMedium + 3);
                writeArgs.priority(WritePriorities.MEDIUM);
                writeArgs.flags(WriteFlags.NO_FLAGS);
                writeReturnVal = rsslChnl.write(transBufMedium[i], writeArgs, error);
                // write return value should be cumulative bytes queued
                assertTrue(writeReturnVal == cumulativeBytesQueued);
                // bytesWritten should be scktBufMedium.getLength()
                assertTrue(writeArgs.bytesWritten() == transBufMedium[i].data().limit()-transBufMedium[i].data().position());
                cumulativeBytesQueued += (bufLenLow + 3);
                writeArgs.priority(WritePriorities.LOW);
                writeArgs.flags(WriteFlags.NO_FLAGS);
                writeReturnVal = rsslChnl.write(transBufLow[i], writeArgs, error);
                // write return value should be cumulative bytes queued
                assertTrue(writeReturnVal == cumulativeBytesQueued);
                // bytesWritten should be scktBufLow.getLength()
                assertTrue(writeArgs.bytesWritten() == transBufLow[i].data().limit()-transBufLow[i].data().position());
            }

            // _totalBytesQueued in RsslSocketChannel should be cumulative bytes
            // queued
            assertTrue(rsslChnl._totalBytesQueued == cumulativeBytesQueued);

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            // java.nio.channels.SocketChannel.write() returns all bytes sent
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)cumulativeBytesQueued);

            // set RsslSocketChannel internal socket channel to mock socket
            // channel
            rsslChnl._scktChannel = scktChnl;

            // flush should return SUCCESS
            assertTrue(rsslChnl.flush(error) == TransportReturnCodes.SUCCESS);

            // _totalBytesQueued in RsslSocketChannel should be 0
            assertTrue(rsslChnl._totalBytesQueued == 0);

            // priority queues in RsslSocketChannel should be empty
            assertTrue(rsslChnl._highPriorityQueue._head == null);
            assertTrue(rsslChnl._mediumPriorityQueue._head == null);
            assertTrue(rsslChnl._lowPriorityQueue._head == null);

            // verify that the _gatherWriteArray's order matches expected flush order.
            assertEquals(transBufLow[0].data(), rsslChnl._gatheringWriteArray[0]); // L
            assertEquals(transBufMedium[0].data(), rsslChnl._gatheringWriteArray[1]); // M
            assertEquals(transBufLow[1].data(), rsslChnl._gatheringWriteArray[2]); // L
            assertEquals(transBufHigh[0].data(), rsslChnl._gatheringWriteArray[3]); // H
            assertEquals(transBufLow[2].data(), rsslChnl._gatheringWriteArray[4]); // L
            assertEquals(transBufMedium[1].data(), rsslChnl._gatheringWriteArray[5]); // M
            assertEquals(transBufLow[3].data(), rsslChnl._gatheringWriteArray[6]); // L
            assertEquals(transBufMedium[2].data(), rsslChnl._gatheringWriteArray[7]); // M
            assertEquals(transBufHigh[1].data(), rsslChnl._gatheringWriteArray[8]); // H
            assertEquals(transBufMedium[3].data(), rsslChnl._gatheringWriteArray[9]); // M
            assertEquals(transBufHigh[2].data(), rsslChnl._gatheringWriteArray[10]); // H
            assertEquals(transBufHigh[3].data(), rsslChnl._gatheringWriteArray[11]); // H
        }
        catch (Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * Set some ioctl codes and verify that they are reset to defaults when
     * Channel is re-allocated from the pool. The following IoctlCodes are
     * covered in this test.
     * <ul>
     * <li>COMPONENT_INFO
     * <li>PRIORITY_FLUSH_ORDER
     * <li>SYSTEM_READ_BUFFERS
     * <li>SYSTEM_WRITE_BUFFERS
     * </ul>
     *
     */
    @SuppressWarnings("deprecation")
    @Test
    public void ioctlVerifyChannelReset()
    {
        final Error error = TransportFactory.createError();
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);
        ConnectOptions connectOpts = buildConnectOptions("localhost", "14002");
        Channel chnl = null;
        Channel chnl2 = null;
        String myFlushPriority = "LMLHLM";

        try
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));

            chnl = Transport.connect(connectOpts, error);
            RsslSocketChannel socketChannel = (RsslSocketChannel)chnl;
            assertTrue(RsslSocketChannel.DEFAULT_PRIORITY_FLUSH_ORDER.equals(socketChannel._channelInfo._priorityFlushStrategy));
            assertEquals(0, socketChannel._componentInfo.componentVersion().length());

            // specify ComponentVersion
            String userSpecifiedComponentVersion = "User Specified Client Version 5.43";
            ByteBuffer componentVersionBB = ByteBuffer.wrap(userSpecifiedComponentVersion.getBytes());
            ComponentInfo myCi = TransportFactory.createComponentInfo();
            myCi.componentVersion().data(componentVersionBB);
            assertEquals(TransportReturnCodes.SUCCESS, chnl.ioctl(IoctlCodes.COMPONENT_INFO, myCi, error));

            // change flush priority order
            assertEquals(TransportReturnCodes.SUCCESS, chnl.ioctl(IoctlCodes.PRIORITY_FLUSH_ORDER, myFlushPriority, error));
            assertTrue(myFlushPriority.equals(socketChannel._channelInfo._priorityFlushStrategy));

            // change High Water Mark to a small value
            assertEquals(TransportReturnCodes.SUCCESS, chnl.ioctl(IoctlCodes.HIGH_WATER_MARK, 5, error));
            assertEquals(5, socketChannel._highWaterMark);

            // change System Read Buffer Size
            assertEquals(TransportReturnCodes.SUCCESS, chnl.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 50000, error));
            assertEquals(50000, ((RsslSocketChannel)chnl).scktChannel().socket().getReceiveBufferSize());

            // change System Write Buffer Size
            assertEquals(TransportReturnCodes.SUCCESS, chnl.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 40000, error));
            assertEquals(40000, ((RsslSocketChannel)chnl).scktChannel().socket().getSendBufferSize());

            /*
             * release Channel back to pool
             */
            chnl.close(error);

            // attempt to get the same Channel back.
            int attempt = 100;
            while (attempt-- > 0)
            {
                chnl2 = Transport.connect(connectOpts, error);
                if (chnl2 == chnl)
                    break;
                else
                    chnl2.close(error);
            }

            /*
             * We have the original channel back.
             */

            assertEquals(chnl2, chnl);
            RsslSocketChannel socketChannel2 = (RsslSocketChannel)chnl;

            // Verify that the component version info is reset.
            assertEquals(0, socketChannel2._componentInfo.componentVersion().length());
            // Verify that the flush priority has been reset to the default value.
            assertTrue(RsslSocketChannel.DEFAULT_PRIORITY_FLUSH_ORDER.equals(socketChannel2._channelInfo._priorityFlushStrategy));
            // Verify that the high water mark has been reset to the default value.
            assertEquals(6144, socketChannel._highWaterMark);
            // Verify that the System Read Buffer size has been reset to the default value.
            assertEquals(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE, ((RsslSocketChannel)chnl).scktChannel().socket().getReceiveBufferSize());
            assertEquals(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE, ((RsslSocketChannel)chnl).scktChannel().socket().getSendBufferSize());
        }
        catch (SocketException e)
        {
            e.printStackTrace();
        }
        finally
        {
            if (chnl != null)
                chnl.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Verify that ETAJ flushes data once high water mark is hit/exceeded.
     */
    @Test
    public void ioctlHighWaterMarkTest()
    {
        // this testData will be used to verify
        // RsslSocketChannel.writeWithNoBuffersQueued(()
        String testData = "FlushMe";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen);
        byteBuf.put(testData.getBytes());

        // create two testDatas that will look identical to testDat,
        // the first will have one char from testData, the second will
        // have the rest. These testDatas will be used to verify
        // RsslSocketChannel.writeWithBuffersQueued(()
        String smallTestData1 = "F";
        int smallBufLen1 = smallTestData1.length();
        ByteBuffer smallByteBuf1 = ByteBuffer.allocate(smallBufLen1);
        smallByteBuf1.put(smallTestData1.getBytes());
        String smallTestData2 = "lushMe";
        int smallBufLen2 = smallTestData2.length();
        ByteBuffer smallByteBuf2 = ByteBuffer.allocate(smallBufLen2);
        smallByteBuf2.put(smallTestData2.getBytes());

        int writeReturnVal = 0, cumulativeBytesQueued = bufLen + 3;
        int smallCumulativeBytesQueued = smallBufLen1 + 3 + smallBufLen2 + 3;

        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();

        try
        {
            // create the RsslSocketChannel to test
            RsslSocketChannel rsslChnl = new RsslSocketChannel()
            {
                @Override
                public int releaseBuffer(TransportBuffer bufferInt, Error error)
                {
                    return 0;
                }

                @Override
                void releaseBufferInternal(TransportBuffer bufferInt)
                {

                }
            }
                    ;

            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            SocketHelper scktChnl = Mockito.mock(SocketHelper.class);

            // java.nio.channels.SocketChannel.write() returns all bytes sent
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)cumulativeBytesQueued);

            // set RsslSocketChannel internal socket channel to mock socket
            // channel
            rsslChnl._scktChannel = scktChnl;

            // Use Channel.ioctl() to change High Water Mark to a small value
            assertEquals(TransportReturnCodes.SUCCESS, rsslChnl.ioctl(IoctlCodes.HIGH_WATER_MARK, 5, error));

            // create SocketBuffer and set to test data
            TransportBufferImpl transBuf = new TransportBufferImpl(bufLen + 3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;

            // call the write method with no write flags, queued bytes should
            // exceed high water mark.
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(transBuf, writeArgs, error);

            // write return value should be 0 since all queued bytes were written.
            assertEquals(0, writeReturnVal);

            // The high water mark should have been hit.
            // Verify there are no bytes queued.
            assertEquals(0, rsslChnl._totalBytesQueued);

            /*
             * Do this again, but this time call write twice. The high water
             * mark should be exceeded on the second write.
             */

            // java.nio.channels.SocketChannel.write() returns all bytes sent
            when(scktChnl.write(Mockito.any(ByteBuffer[].class), Mockito.anyInt(), Mockito.anyInt())).thenReturn((long)smallCumulativeBytesQueued);

            // create SocketBuffer and set to test data
            TransportBufferImpl smallTransBuf1 = new TransportBufferImpl(smallBufLen1 + 3);
            smallTransBuf1._data.position(3);
            smallByteBuf1.flip();
            smallTransBuf1.data().put(smallByteBuf1);
            smallTransBuf1._isWriteBuffer = true;

            TransportBufferImpl smallTransBuf2 = new TransportBufferImpl(smallBufLen2 + 3);
            smallTransBuf2._data.position(3);
            smallByteBuf2.flip();
            smallTransBuf2.data().put(smallByteBuf2);
            smallTransBuf2._isWriteBuffer = true;

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(smallTransBuf1, writeArgs, error);
            assertEquals(smallBufLen1 + 3, writeReturnVal);

            // this write should exceed the high water mark
            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            writeReturnVal = rsslChnl.write(smallTransBuf2, writeArgs, error);

            // The high water mark should have been hit.
            // Verify there are no bytes queued.
            assertEquals(0, rsslChnl._totalBytesQueued);

            // test invalid value
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.HIGH_WATER_MARK, -9999, error));

        }
        catch (Exception e)
        {
            assertTrue(false);
        }
    }

    /*
     * Uninitialize the transport, stop replay, close consumerChannel, shutdown
     * TestServer and stop TestServer thread.
     */
    static void cleanupForTestsWithTestServer(NetworkReplay replay, Channel consumerChannel, TestServer server, Thread tServer, Error error)
    {
        assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        if (replay != null)
        {
            replay.stopListener();
            replay = null;
        }

        if ((consumerChannel != null) && ((RsslSocketChannel)consumerChannel).state() != ChannelState.INACTIVE)
        {
            int ret = consumerChannel.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                fail("consumerChannel.close() failed with return code " + ret + ", error="
                     + error.text());
            }
            consumerChannel = null;
        }

        server.shutDown();
        if (tServer != null)
        {
            tServer.interrupt();
            // give server time to shut down.
            try
            {
                Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
                fail("cleanupForTestsWithTestServer, exception=" + e.toString());
            }
        }
    }

    /*
     * Uninitialize the transport, stop replay, close serverChannel, shutdown
     * TestServer and stop TestServer thread.
     */
    private void cleanupForTestsWithTestClient(NetworkReplay replay, Server server, Channel serverChannel, TestClient client, Thread tClient, Selector selector, Error error)
    {
        int ret;

        assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        if (replay != null)
        {
            replay.stopListener();
            replay = null;
        }

        if ((server != null) && ((ServerImpl)server)._state != ChannelState.INACTIVE)
        {
            ret = server.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                fail("server.close() failed with return code " + ret + ", error="
                     + error.text());
            }
            server = null;
        }

        if ((serverChannel != null) && ((RsslSocketChannel)serverChannel)._state != ChannelState.INACTIVE)
        {
            ret = serverChannel.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                fail("serverChannel.close() failed with return code " + ret
                     + ", error=" + error.text());
            }
            serverChannel = null;
        }

        if (selector != null)
        {
            try
            {
                selector.close();
            }
            catch (IOException e)
            {
                fail("failed to close selector, exception=" + e.toString());
            }
            selector = null;
        }

        client.shutDown();
        if (tClient != null)
        {
            tClient.interrupt();

            // give client time to shut down.
            try
            {
                Thread.sleep(100);
            }
            catch (InterruptedException e)
            {
            }
        }

    }

    static Server serverBind(Selector selector, BindOptions bOpts, Error error)
    {
        return serverBind(selector, bOpts, 60, 30, Ripc.CompressionTypes.NONE, 0 /* compression level */,
                Codec.protocolType(), Codec.majorVersion(), Codec.minorVersion(), false, error);
    }

    /*
     * Helper method to bind ETAJ server socket.
     *
     * @param selector
     * @param pingTimeout
     * @param minPingTimeout
     * @param compressionType
     * @param compressionLevel
     * @param protocolType
     * @param majorVersion
     * @param minorVersion
     * @param error
     * @param channelsBlocking
     * @return Server
     */
    static Server serverBind(Selector selector, BindOptions bOpts, int pingTimeout, int minPingTimeout, int compressionType,
                             int compressionLevel, int protocolType, int majorVersion, int minorVersion, boolean blocking, Error error)
    {
        bOpts.pingTimeout(pingTimeout);
        bOpts.minPingTimeout(minPingTimeout);
        bOpts.guaranteedOutputBuffers(10);
        bOpts.compressionType(compressionType);
        if(compressionType > Ripc.CompressionTypes.NONE)
            bOpts.compressionLevel(compressionLevel);
        bOpts.protocolType(protocolType);
        bOpts.majorVersion(majorVersion);
        bOpts.minorVersion(minorVersion);
        bOpts.channelsBlocking(blocking);
        bOpts.serverBlocking(blocking);

        Server server = Transport.bind(bOpts, error);
        if (server == null)
            assertNotNull("failed to bind server, reason=" + error.text(), server);

        if(!blocking)
        {
            try
            {
                server.selectableChannel().register(selector, SelectionKey.OP_ACCEPT, server);
            }
            catch (Exception e)
            {
                assertTrue("failed to register server for OP_ACCEPT, exception=" + e.toString(), false);
            }
        }

        return server;
    }

    /*
     * accept a connection or fail after timeout.
     *
     * @param selector
     * @param nakMount boolean indicating
     *            AcceptOptions.nakMount(boolean)
     * @param error
     * @return Channel
     */
    static Channel serverAccept(Selector selector, AcceptOptions aOpts, Error error)
    {

        Set<SelectionKey> keySet = null;

        try
        {
            if (selector.select(TIMEOUTMS) > 0)
            {
                keySet = selector.selectedKeys();
                if (keySet != null)
                {
                    Iterator<SelectionKey> iter = keySet.iterator();
                    while (iter.hasNext())
                    {
                        SelectionKey key = iter.next();
                        iter.remove();
                        if (key.isAcceptable())
                        {
                            Server server = (Server)key.attachment();
                            Channel channel = server.accept(aOpts, error);
                            if (channel != null)
                            {
                                channel.selectableChannel().register(selector, SelectionKey.OP_READ, channel);
                                return channel;
                            }
                            else
                            {
                                assertTrue("server.accept() failed to return a valid Channel, error="
                                           + error.text(), false);
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

        assertTrue("server timed out waiting to accept a connection", false);
        return null;
    }

    /*
     * Calls channel.init() one time and returns the current channel state.
     *
     * @param channel
     * @throws IOException
     */
    static int initChannel(Channel channel, Error error, InProgInfo inProg)
    {
        assertTrue(channel != null);
        int retval;

        // This finalizes the three-way handshake.

        if (channel.state() == ChannelState.INITIALIZING)
        {
            if ((retval = channel.init(inProg, error)) < TransportReturnCodes.SUCCESS)
            {
                // fail("\nChannel " + channel.selectableChannel() + " inactive: "
                // + error.text());
                System.out.println("initChannel(): Channel " + channel.selectableChannel()
                                   + " inactive: " + error.text());
                // return channelState as inactive for our junits.
                return ChannelState.INACTIVE;
            }
            else
            {
                switch (retval)
                {
                    case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                        if (inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                        {
                            System.out.println("\nChannel In Progress - New Channel: "
                                               + channel.selectableChannel() + " Old Channel: "
                                               + inProg.oldSelectableChannel());
                        }
                        System.out.println("\nChannel " + channel.selectableChannel() + " In Progress...");
                        break;
                    case TransportReturnCodes.SUCCESS:
                        System.out.println("\nChannel " + channel.selectableChannel() + " Is Active");
                        break;
                    default:
                        fail("\nBad return value channel=" + channel.selectableChannel() + " <"
                             + error.text() + ">");
                }
            }
        }

        return channel.state();
    }

    /*
     * Calls channel.init() until the specified state is reached or timeout occurs.
     *
     * @param channel
     * @param state to be reached.
     * @param inProg
     */
    private void initChannelWaitState(Channel channel, int state, Error error, InProgInfo inProg)
    {
        assertTrue(channel != null);

        long currentTimeMs = System.currentTimeMillis();
        long endTimeMs = currentTimeMs + TIMEOUTMS;
        int channelState;

        try
        {
            while (System.currentTimeMillis() < endTimeMs)
            {
                if ((channelState = initChannel(channel, error, inProg)) == state)
                    return;
                else if (channelState == ChannelState.INACTIVE)
                    fail("waiting for channel state of " + ChannelState.toString(state)
                         + " but received state of " + ChannelState.toString(channelState)
                         + ", error=" + error.text());
                Thread.sleep(100);
            }
        }
        catch (InterruptedException e)
        {
        }
        fail("timeout waiting for channel state " + ChannelState.toString(state)
             + " current channel state is " + ChannelState.toString(channel.state()));
    }

    /*
     * Verify that the System Read and Write Buffer size is set to the default
     * values (as defined in RsslSocketChannel, when the channel is created.
     * Also verify that the values can be changed with Channel.ioctl() after
     * the channel is connected.
     */
    @Test
    public void ioctlSystemReadWriteBuffersDefaultTest()
    {
        final String inputFile = RIPC_CONNECT_ACK_HANDSHAKE_FILE;
        NetworkReplay replay = null;

        ConnectOptions connectOpts = getConnectOptions("localhost", DEFAULT_LISTEN_PORT_AS_STRING);

        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel channel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            assertNotNull(channel = Transport.connect(connectOpts, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            server.waitForAcceptable();
            server.acceptSocket();

            assertEquals(TransportReturnCodes.CHAN_INIT_IN_PROGRESS, channel.init(inProg, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            // read ConnectReq and send ConnectAck
            server.waitForReadable();
            server.readMessageFromSocket();
            server.writeMessageToSocket(replay.read());

            Thread.sleep(1000);

            // call channel.init to read ConnectAck
            assertEquals(TransportReturnCodes.SUCCESS, channel.init(inProg, error));
            assertEquals(ChannelState.ACTIVE, channel.state());

            // verify that the buf size specified in connectOpts, was set on the socket.
            assertEquals(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            assertEquals(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            /*
             * set the System Read/Write Buffers to larger than 64K. Note that
             * this is done after the connect, so TCP may not take the hint, but
             * the java socket will have the new value.
             */
            channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 123456, error);
            assertEquals(123456, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 123456, error);
            assertEquals(123456, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            /*
             * set the System Read/Write Buffers to 54321 and verify that the socket's
             * receive buffer size was modified.
             */
            channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 54321, error);
            assertEquals(54321, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 54321, error);
            assertEquals(54321, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

        }
        catch (IOException | InterruptedException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, channel, server, null, error);
        }
    }

    /*
     * Verify that the System Read and Write Buffer size can be set via
     * ConnectOpts when the channel is created. Also verify that the values can
     * be changed with Channel.ioctl() after the channel is connected.
     */
    @Test
    public void ioctlSystemReadWriteWriteBuffersTest()
    {
        final String inputFile = RIPC_CONNECT_ACK_HANDSHAKE_FILE;
        final int myDefaultBufSize = 60000;
        NetworkReplay replay = null;

        ConnectOptions connectOpts = getConnectOptions("localhost", DEFAULT_LISTEN_PORT_AS_STRING);
        connectOpts.sysRecvBufSize(myDefaultBufSize);
        connectOpts.sysSendBufSize(myDefaultBufSize);

        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel channel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            assertNotNull(channel = Transport.connect(connectOpts, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            server.waitForAcceptable();
            server.acceptSocket();

            assertEquals(TransportReturnCodes.CHAN_INIT_IN_PROGRESS, channel.init(inProg, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            // read ConnectReq and send ConnectAck
            server.waitForReadable();
            server.readMessageFromSocket();
            server.writeMessageToSocket(replay.read());

            Thread.sleep(1000);

            // call channel.init to read ConnectAck
            assertEquals(TransportReturnCodes.SUCCESS, channel.init(inProg, error));
            assertEquals(ChannelState.ACTIVE, channel.state());

            // verify that the buf size specified in connectOpts, was set on the socket.
            assertEquals(myDefaultBufSize, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            assertEquals(myDefaultBufSize, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            /*
             * set the System Read/Write Buffer to 10240 and verify that the socket's
             * receive buffer size was modified.
             */
            channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 10240, error);
            assertEquals(10240, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 10240, error);
            assertEquals(10240, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            /*
             * set the System Read/Write Buffers to larger than 64K. Note that
             * this is done after the connect, so TCP may not take the hint, but
             * the java socket will have the new value.
             */
            channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 165537, error);
            assertEquals(165537, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 165537, error);
            assertEquals(165537, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            // set ioctl SYSTEM_READ_BUFFERS with invalid data.
            assertEquals(TransportReturnCodes.FAILURE, channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            // set ioctl SYSTEM_WRITE_BUFFERS with invalid data.
            assertEquals(TransportReturnCodes.FAILURE, channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
        }
        catch (IOException | InterruptedException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, channel, server, null, error);
        }
    }

    /*
     * Verify that the System Read and Write Buffer size can be set via
     * ConnectOpts to a value larger than 64K, when the channel is created. Also
     * verify that the values can be changed with Channel.ioctl() after the
     * channel is connected.
     */
    @Test
    public void ioctlSystemReadWriteBuffersLargerThan64KTest()
    {
        final String inputFile = RIPC_CONNECT_ACK_HANDSHAKE_FILE;
        final int myDefaultBufSize = 200000;
        NetworkReplay replay = null;

        ConnectOptions connectOpts = getConnectOptions("localhost", DEFAULT_LISTEN_PORT_AS_STRING);
        connectOpts.sysRecvBufSize(myDefaultBufSize);
        connectOpts.sysSendBufSize(myDefaultBufSize);

        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel channel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            assertNotNull(channel = Transport.connect(connectOpts, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            server.waitForAcceptable();
            server.acceptSocket();

            assertEquals(TransportReturnCodes.CHAN_INIT_IN_PROGRESS, channel.init(inProg, error));
            assertEquals(ChannelState.INITIALIZING, channel.state());

            // read ConnectReq and send ConnectAck
            server.waitForReadable();
            server.readMessageFromSocket();
            server.writeMessageToSocket(replay.read());

            Thread.sleep(1000);

            // call channel.init to read ConnectAck
            assertEquals(TransportReturnCodes.SUCCESS, channel.init(inProg, error));
            assertEquals(ChannelState.ACTIVE, channel.state());

            // verify that the buf size specified in connectOpts, was set on the socket.
            assertEquals(myDefaultBufSize, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            assertEquals(myDefaultBufSize, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            /*
             * set the System Read Buffer to 10240 and verify that the socket's
             * receive buffer size was modified.
             */
            channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 10240, error);
            assertEquals(10240, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 10240, error);
            assertEquals(10240, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());

            /*
             * set the System Read/Write Buffers to larger than 64K. Note that
             * this is done after the connect, so TCP may not take the hint, but
             * the java socket will have the new value.
             */
            channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, 165537, error);
            assertEquals(165537, ((RsslSocketChannel)channel)._scktChannel.socket().getReceiveBufferSize());
            channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, 165537, error);
            assertEquals(165537, ((RsslSocketChannel)channel)._scktChannel.socket().getSendBufferSize());
        }
        catch (IOException | InterruptedException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, channel, server, null, error);
        }
    }

    /*
     * Verify that the System Read/Write Buffer size are set to default values
     * after bind() and accept(). Also verify that the values can be changed
     * with Channel.ioctl() after the channel is connected.
     */
    @SuppressWarnings("deprecation")
    @Test
    public void ioctlSystemReadWriteBuffersDefaultServerTest()
    {
        String inputFile = RIPC_CONNECT_REQ_HANDSHAKE_FILE;

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        BindOptions bOpts = TransportFactory.createBindOptions();
        bOpts.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bOpts.connectionType(ConnectionTypes.SOCKET);
        AcceptOptions aOpts = TransportFactory.createAcceptOptions();

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the ETAJ server
            selector = Selector.open();
            server = serverBind(selector, bOpts, error);

            // have our test client connect to ETAJ server
            client.connect();

            // have the ETAJ server accept the connection from our test client.
            aOpts.sysSendBufSize(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE);
            serverChannel = serverAccept(selector, aOpts, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our ETAJ server process the ConnectReq and send a ConnectAck
            // verify that the ETAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();

            /*
             * Verify default System Read/Write Buffer size.
             */
            assertEquals(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE, ((RsslSocketChannel)serverChannel).scktChannel().socket().getReceiveBufferSize());
            assertEquals(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE, ((RsslSocketChannel)serverChannel).scktChannel().socket().getSendBufferSize());

            /*
             * Use ioctl to change System Read Buffer size on server and System
             * Read/Write Buffer size channel.
             */
            int recvBufSize = 42000;
            int sendBufSize = 45000;
            assertEquals(TransportReturnCodes.SUCCESS, server.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, recvBufSize, error));
            assertEquals(recvBufSize, ((ServerImpl)server).srvrScktChannel().socket().getReceiveBufferSize());
            assertEquals(TransportReturnCodes.SUCCESS, serverChannel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, recvBufSize, error));
            assertEquals(recvBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getReceiveBufferSize());
            assertEquals(TransportReturnCodes.SUCCESS, serverChannel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, error));
            assertEquals(sendBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getSendBufferSize());
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, null, selector, error);
        }
    }

    /*
     * Verify that the System Read Buffer size can be set to a non-default value
     * via BindOpts prior to bind(), and Write Buffer size can be set to a
     * non-default value via AcceptOpts prior to accept().
     */
    @SuppressWarnings("deprecation")
    @Test
    public void ioctlSystemReadWriteBuffersServerTest()
    {
        String inputFile = RIPC_CONNECT_REQ_HANDSHAKE_FILE;

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        int recvBufSize = 50000;
        int sendBufSize = 40000;
        BindOptions bOpts = TransportFactory.createBindOptions();
        bOpts.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bOpts.connectionType(ConnectionTypes.SOCKET);
        bOpts.sysRecvBufSize(recvBufSize);
        AcceptOptions aOpts = TransportFactory.createAcceptOptions();
        aOpts.sysSendBufSize(sendBufSize);

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the ETAJ server
            selector = Selector.open();
            server = serverBind(selector, bOpts, error);

            // have our test client connect to ETAJ server
            client.connect();

            // have the ETAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, aOpts, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our ETAJ server process the ConnectReq and send a ConnectAck
            // verify that the ETAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();

            /*
             * Verify default System Read/Write Buffer size.
             */
            assertEquals(recvBufSize, ((ServerImpl)server).srvrScktChannel().socket().getReceiveBufferSize());
            assertEquals(recvBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getReceiveBufferSize());
            assertEquals(sendBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getSendBufferSize());

            // set ioctl SYSTEM_READ_BUFFERS with invalid data.
            assertEquals(TransportReturnCodes.FAILURE, server.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertEquals(TransportReturnCodes.FAILURE, serverChannel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            // set ioctl SYSTEM_WRITE_BUFFERS with invalid data.
            assertEquals(TransportReturnCodes.FAILURE, serverChannel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, null, selector, error);
        }
    }

    /*
     * Verify that the System Read Buffer size can be set to a value larger than 64K
     * via BindOpts prior to bind(), and Write Buffer size can be set to a
     * value larger than 64K via AcceptOpts prior to accept().
     */
    @SuppressWarnings("deprecation")
    @Test
    public void ioctlSystemReadWriteBuffersLargerThan64KServerTest()
    {
        String inputFile = RIPC_CONNECT_REQ_HANDSHAKE_FILE;

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        int recvBufSize = 100000;
        int sendBufSize = 110000;
        BindOptions bOpts = TransportFactory.createBindOptions();
        bOpts.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bOpts.connectionType(ConnectionTypes.SOCKET);
        bOpts.sysRecvBufSize(recvBufSize);
        AcceptOptions aOpts = TransportFactory.createAcceptOptions();
        aOpts.sysSendBufSize(sendBufSize);

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the ETAJ server
            selector = Selector.open();
            server = serverBind(selector, bOpts, error);

            // have our test client connect to ETAJ server
            client.connect();

            // have the ETAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, aOpts, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our ETAJ server process the ConnectReq and send a ConnectAck
            // verify that the ETAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();

            /*
             * Verify default System Read/Write Buffer size.
             */
            assertEquals(recvBufSize, ((ServerImpl)server).srvrScktChannel().socket().getReceiveBufferSize());
            assertEquals(recvBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getReceiveBufferSize());
            assertEquals(sendBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getSendBufferSize());

            /*
             * Use ioctl to change System Read Buffer size on server and System
             * Read/Write Buffer size channel.
             */
            recvBufSize = 52000;
            sendBufSize = 55000;
            assertEquals(TransportReturnCodes.SUCCESS, server.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, recvBufSize, error));
            assertEquals(recvBufSize, ((ServerImpl)server).srvrScktChannel().socket().getReceiveBufferSize());
            assertEquals(TransportReturnCodes.SUCCESS, serverChannel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, recvBufSize, error));
            assertEquals(recvBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getReceiveBufferSize());
            assertEquals(TransportReturnCodes.SUCCESS, serverChannel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, error));
            assertEquals(sendBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getSendBufferSize());

            /*
             * Use ioctl to change System Read Buffer size on server and System
             * Read/Write Buffer size channel.
             */
            recvBufSize = 128000;
            sendBufSize = 127000;
            assertEquals(TransportReturnCodes.SUCCESS, server.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, recvBufSize, error));
            assertEquals(recvBufSize, ((ServerImpl)server).srvrScktChannel().socket().getReceiveBufferSize());
            assertEquals(TransportReturnCodes.SUCCESS, serverChannel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, recvBufSize, error));
            assertEquals(recvBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getReceiveBufferSize());
            assertEquals(TransportReturnCodes.SUCCESS, serverChannel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, error));
            assertEquals(sendBufSize, ((RsslSocketChannel)serverChannel).scktChannel().socket().getSendBufferSize());
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, null, selector, error);
        }
    }

    /*
     * Verify that the Compression Threshold can be modified with
     * Channnel.ioctl(). This test will write a buffer that is under the default
     * compression threshold and verify that the data was not compressed. Then
     * it will write two buffers that are over the default compression threshold
     * and verify that the data was compressed. Finally, it will use
     * Channel.ioctl() to change the compression threshold and write a buffer
     * lower than the compression threshold and verify that the data was not compressed.
     */
    @Test
    public void ioctlCompressionThresholdWriteTest()
    {
        String testData20 = "aaabbbcccdddeeefffgg";
        String testData80 = "gghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzzaaabbbcccdddeeefffggg";

        // populate byteBuf with testData20
        ByteBuffer byteBuf = ByteBuffer.allocate(100);
        byteBuf.put(testData20.getBytes());

        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        Error error = TransportFactory.createError();
        RsslSocketChannel rsslChnl = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            // set channel state to ACTIVE

            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));
            RsslSocketChannel initChannel = (RsslSocketChannel) Transport.connect(connectOpts, error);
            assertNotNull(initChannel);
            rsslChnl = getNetworkReplayChannel(Transport._transports[0],2);
            assertNotNull(rsslChnl);


            TransportBufferImpl transBuf = new TransportBufferImpl(testData80.length() + 3);
            transBuf._data.position(3);
            byteBuf.flip();
            transBuf.data().put(byteBuf);
            transBuf._isWriteBuffer = true;


            // set channel state to ACTIVE
            rsslChnl._state = ChannelState.ACTIVE;

            // set-up channel for compression
            rsslChnl._sessionOutCompression = CompressionTypes.ZLIB;
            rsslChnl._sessionCompLevel = 9;
            rsslChnl._compressor = rsslChnl._ZlibCompressor;
            rsslChnl._compressor.compressionLevel(rsslChnl._sessionCompLevel);
            rsslChnl._sessionCompLowThreshold = rsslChnl.ZLIB_COMPRESSION_THRESHOLD;
            rsslChnl._channelInfo._compressionType = CompressionTypes.ZLIB;

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            /*
             * Write under default (30) compression threshold
             */

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) > 0);
            assertEquals(23, writeArgs.bytesWritten());
            assertEquals(23, writeArgs.uncompressedBytesWritten());

            // end of queue should contain compressed message
            TransportBufferImpl buf = (TransportBufferImpl) rsslChnl._highPriorityQueue._tail;
            ByteBuffer bb = buf.data();
            // verify that the RIPC header has RipcFlags.Data (0x02) and is not RipcFlags.CompressedData (0x04).
            assertEquals(0x02, bb.get(2));

            /*
             * write over the default (30) compression threshold. The first
             * compressed message is larger because it contains the full
             * compression table.
             */

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // populate byteBuf with testData80
            byteBuf.limit(byteBuf.capacity());
            byteBuf.position(0);
            byteBuf.limit(80);
            byteBuf.put(testData80.getBytes());
            byteBuf.flip();
            transBuf._data.position(3);
            transBuf._data.limit(83);
            transBuf.data().put(byteBuf);
            transBuf._isOwnedByApp = true;

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) > 0);
            assertEquals(75, writeArgs.bytesWritten());
            assertEquals(83, writeArgs.uncompressedBytesWritten());
            assertEquals(false, transBuf._isOwnedByApp);

            // end of queue should contain compressed message
            buf = (TransportBufferImpl) rsslChnl._highPriorityQueue._tail;
            bb = buf.data();
            // verify that the RIPC header has RipcFlags.CompressedData (0x04)
            // and is not RipcFlags.Data (0x02).
            assertEquals(0x04, bb.get(2));

            /*
             * write over the default (30) compression threshold once more so
             * that we see the compression in effect.
             */

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // populate byteBuf with testData80
            byteBuf.position(0);
            byteBuf.limit(80);
            byteBuf.put(testData80.getBytes());
            byteBuf.flip();
            transBuf._data.position(3);
            transBuf._data.limit(83);
            transBuf.data().put(byteBuf);
            transBuf._isOwnedByApp = true;

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) > 0);
            assertEquals(13, writeArgs.bytesWritten());
            assertEquals(83, writeArgs.uncompressedBytesWritten());
            assertEquals(false, transBuf._isOwnedByApp);

            // end of queue should contain compressed message
            buf = (TransportBufferImpl) rsslChnl._highPriorityQueue._tail;
            bb = buf.data();
            // verify that the RIPC header has RipcFlags.CompressedData (0x04)
            // and is not RipcFlags.Data (0x02).
            assertEquals(0x04, bb.get(2));

            /*
             * Adjust the compression threshold to 100 and write under the
             * compression threshold. Verify that the data was not compressed.
             */

            assertEquals(TransportReturnCodes.SUCCESS, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 100, error));

            // set _totalBytesQueued to 0 for no buffers queued
            rsslChnl._totalBytesQueued = 0;

            // populate byteBuf with testData80
            byteBuf.position(0);
            byteBuf.limit(80);
            byteBuf.put(testData80.getBytes());
            byteBuf.flip();
            transBuf._data.position(3);
            transBuf._data.limit(83);
            transBuf.data().put(byteBuf);
            transBuf._isOwnedByApp = true;

            writeArgs.priority(WritePriorities.HIGH);
            writeArgs.flags(WriteFlags.NO_FLAGS);
            assertTrue(rsslChnl.write(transBuf, writeArgs, error) > 0);
            assertEquals(83, writeArgs.bytesWritten());
            assertEquals(83, writeArgs.uncompressedBytesWritten());
            assertEquals(false, transBuf._isOwnedByApp);

            // end of queue should contain compressed message
            buf = (TransportBufferImpl) rsslChnl._highPriorityQueue._tail;
            bb = buf.data();
            // verify that the RIPC header has RipcFlags.Data (0x02)
            // and is not RipcFlags.CompressedData (0x05).
            assertEquals(0x02, bb.get(2));


            // verify that Channel.ioctl(COMPRESSION_THRESHOLD) passes if value equals 30 (the minimum).
            error.clear();
            assertEquals(TransportReturnCodes.SUCCESS, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 30, error));
            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());

            // verify that Channel.ioctl(COMPRESSION_THRESHOLD) fails if value
            // is less than 30 (the minimum).
            error.clear();
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 29, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            error.clear();
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 0, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            error.clear();
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            // change compression type to LZ4 and try same COMPRESSION_THRESHOLD tests which should fail
            rsslChnl._sessionOutCompression = CompressionTypes.LZ4;
            rsslChnl._sessionCompLevel = 9;
            rsslChnl._compressor = rsslChnl._Lz4Compressor;
            rsslChnl._compressor.compressionLevel(rsslChnl._sessionCompLevel);
            rsslChnl._sessionCompLowThreshold = rsslChnl.LZ4_COMPRESSION_THRESHOLD;

            error.clear();
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 29, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            error.clear();
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 0, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            error.clear();
            assertEquals(TransportReturnCodes.FAILURE, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, -1, error));
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());

            // now try 30 with LZ4 which should pass
            error.clear();
            assertEquals(TransportReturnCodes.SUCCESS, rsslChnl.ioctl(IoctlCodes.COMPRESSION_THRESHOLD, 30, error));
            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());

            rsslChnl.close(error);
            initChannel.close(error);
        }
        catch (Exception e)
        {
            assertTrue(e.toString(), false);
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     */
    @Test
    public void releaseBufferTest()
    {
        String testData = "releaseBufferTest";
        int bufLen = testData.length();
        ByteBuffer byteBuf = ByteBuffer.allocate(bufLen + 3);
        byteBuf.position(3);
        byteBuf.put(testData.getBytes());
        Error error = TransportFactory.createError();
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ConnectOptions ops = getDefaultConnectOptions();
        ops.guaranteedOutputBuffers(8);

        try
        {
            // create the RsslSocketChannel to test
            // override flush to return positive number
            // set channel state to ACTIVE

            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));
            RsslSocketChannel initChannel = (RsslSocketChannel) Transport.connect(ops, error);
            assertNotNull(initChannel);
            RsslSocketChannel channel = getNetworkReplayChannel(Transport._transports[0],8);
            channel._isJunitTest = true;

            TransportBufferImpl buf1 = (TransportBufferImpl)channel.getBuffer(6100, false, error);
            buf1._isWriteBuffer = true;

            assertEquals(6100, buf1.length());
            assertEquals(1, channel.bufferUsage(error));
            TransportBufferImpl buf2 = (TransportBufferImpl)channel.getBuffer(6100, false, error);

            assertEquals(2, channel.bufferUsage(error));

            TransportBufferImpl buf3 = (TransportBufferImpl)channel.getBuffer(6100, false, error);


            assertEquals(3, channel.bufferUsage(error));
            TransportBufferImpl buf4 = (TransportBufferImpl)channel.getBuffer(6100, false, error);

            assertEquals(4, channel.bufferUsage(error));

            TransportBufferImpl buf5 = (TransportBufferImpl)channel.getBuffer(6100, false, error);

            assertEquals(5, channel.bufferUsage(error));

            TransportBufferImpl buf6 = (TransportBufferImpl)channel.getBuffer(6100, false, error);

            assertEquals(6, channel.bufferUsage(error));

            TransportBufferImpl buf7 = (TransportBufferImpl)channel.getBuffer(6100, false, error);

            assertEquals(7, channel.bufferUsage(error));

            buf1.data().put(testData.getBytes());
            channel.write(buf1, writeArgs, error);
            channel.flush(error);

            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
            assertEquals(6, channel.bufferUsage(error));

            channel.releaseBuffer(buf2, error);
            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
            assertEquals(5, channel.bufferUsage(error));

            TransportBuffer buf8 = channel.getBuffer(6100, false, error);

            TransportBuffer buf = channel.getBuffer(6100, false, error);

            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
            assertEquals(7, channel.bufferUsage(error));
            assertEquals(buf1, buf);

            buf = channel.getBuffer(6100, false, error);
            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
            assertEquals(8, channel.bufferUsage(error));
            assertEquals(buf2, buf);

            channel.releaseBuffer(buf1, error);
            assertEquals(7, channel.bufferUsage(error));
            channel.releaseBuffer(buf2, error);	// is not released because is current Buffer
            assertEquals(7, channel.bufferUsage(error));
            channel.releaseBuffer(buf3, error);
            assertEquals(6, channel.bufferUsage(error));
            channel.releaseBuffer(buf4, error);
            assertEquals(5, channel.bufferUsage(error));
            channel.releaseBuffer(buf5, error);
            assertEquals(4, channel.bufferUsage(error));
            channel.releaseBuffer(buf6, error);
            assertEquals(3, channel.bufferUsage(error));
            channel.releaseBuffer(buf7, error);
            assertEquals(2, channel.bufferUsage(error));
            channel.releaseBuffer(buf8, error);

            assertEquals(TransportReturnCodes.SUCCESS, error.errorId());
            assertEquals(1, channel.bufferUsage(error));

            channel.releaseBuffer(buf1, error);
            channel.releaseBuffer(buf2, error);
            channel.releaseBuffer(buf3, error);
            channel.releaseBuffer(buf4, error);
            channel.releaseBuffer(buf5, error);
            channel.releaseBuffer(buf6, error);
            channel.releaseBuffer(buf7, error);
            channel.releaseBuffer(buf8, error);

            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertEquals(1, channel.bufferUsage(error));

            TransportBufferImpl buf11 = (TransportBufferImpl)channel.getBuffer(60000, false, error);
            buf11._isWriteBuffer = true;
            assertEquals(60000, buf11.length());
            assertEquals(2, channel.bufferUsage(error));
            TransportBuffer buf12 = channel.getBuffer(60000, false, error);
            assertEquals(3, channel.bufferUsage(error));
            channel.releaseBuffer(buf11, error);
            assertEquals(2, channel.bufferUsage(error));
            channel.releaseBuffer(buf11, error);
            assertEquals(2, channel.bufferUsage(error));
            channel.releaseBuffer(buf12, error);
            assertEquals(1, channel.bufferUsage(error));
            channel.releaseBuffer(buf11, error);
            assertEquals(1, channel.bufferUsage(error));
            buf = channel.getBuffer(60000, false, error);
            assertEquals(buf, buf11);
            assertEquals(2, channel.bufferUsage(error));
            buf = channel.getBuffer(60000, false, error);
            assertEquals(buf, buf12);
            assertEquals(3, channel.bufferUsage(error));


            channel.close(error);
            initChannel.close(error);
        }
        catch (Exception e)
        {
            assertTrue(e.toString(), false);
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * Create a SocketProtocol with empty global buffer pool, and 2 RsslSocketChannel
     * objects with a set number of guaranteedOutputBuffers. Release buffers from both
     * channels back to the pool and get them back a couple of times. Verify buffers
     * both after release and when they are back. This was added to test the issue
     * found in MR upaj1185.
     */
    @Test
    public void numGuaranteedBuffersTwoChannelTest()
    {
        int initialBufferCount = 20;

        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.guaranteedOutputBuffers(initialBufferCount);

        // setup the RsslSocketChannel
        SocketProtocol transport = new SocketProtocol(opts);
        RsslSocketChannel rsslChnl1 = new RsslSocketChannel(transport, transport._channelPool);
        RsslSocketChannel rsslChnl2 = new RsslSocketChannel(transport, transport._channelPool);
        rsslChnl1._state = ChannelState.ACTIVE;
        rsslChnl1._writeLock = new DummyLock();
        rsslChnl1.dataFromOptions(opts);
        rsslChnl1.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());
        rsslChnl2._state = ChannelState.ACTIVE;
        rsslChnl2._writeLock = new DummyLock();
        rsslChnl2.dataFromOptions(opts);
        rsslChnl2.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        // verify setup.
        Pool bufferPool = transport.getPool(rsslChnl1._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl1._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl1._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl1._channelInfo.maxOutputBuffers());

        // verify setup.
        bufferPool = transport.getPool(rsslChnl2._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl2._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl2._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl2._channelInfo.maxOutputBuffers());

        // release all buffers back to pool
        rsslChnl1.shrinkGuaranteedOutputBuffers(rsslChnl1._availableBuffers.size());
        rsslChnl2.shrinkGuaranteedOutputBuffers(rsslChnl2._availableBuffers.size());

        // verify release
        bufferPool = transport.getPool(rsslChnl1._internalMaxFragmentSize);
        assertEquals(initialBufferCount*2, bufferPool.size());
        assertEquals(0, rsslChnl1._availableBuffers.size());
        bufferPool = transport.getPool(rsslChnl2._internalMaxFragmentSize);
        assertEquals(initialBufferCount*2, bufferPool.size());
        assertEquals(0, rsslChnl2._availableBuffers.size());

        // get the buffers back
        rsslChnl1.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());
        rsslChnl2.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        // verify buffers are back
        bufferPool = transport.getPool(rsslChnl1._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl1._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl1._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl1._channelInfo.maxOutputBuffers());

        // verify buffers are back
        bufferPool = transport.getPool(rsslChnl2._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl2._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl2._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl2._channelInfo.maxOutputBuffers());

        // release all buffers back to pool
        rsslChnl1.shrinkGuaranteedOutputBuffers(rsslChnl1._availableBuffers.size());
        rsslChnl2.shrinkGuaranteedOutputBuffers(rsslChnl2._availableBuffers.size());

        // verify release
        bufferPool = transport.getPool(rsslChnl1._internalMaxFragmentSize);
        assertEquals(initialBufferCount*2, bufferPool.size());
        assertEquals(0, rsslChnl1._availableBuffers.size());
        bufferPool = transport.getPool(rsslChnl2._internalMaxFragmentSize);
        assertEquals(initialBufferCount*2, bufferPool.size());
        assertEquals(0, rsslChnl2._availableBuffers.size());

        // get the buffers back
        rsslChnl1.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());
        rsslChnl2.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        // verify buffers are back
        bufferPool = transport.getPool(rsslChnl1._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl1._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl1._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl1._channelInfo.maxOutputBuffers());

        // verify buffers are back
        bufferPool = transport.getPool(rsslChnl2._internalMaxFragmentSize);
        assertEquals(0, bufferPool.size());
        assertEquals(initialBufferCount, rsslChnl2._availableBuffers.size());
        assertEquals(initialBufferCount, rsslChnl2._channelInfo.guaranteedOutputBuffers());
        assertEquals(initialBufferCount, rsslChnl2._channelInfo.maxOutputBuffers());
    }

    @Test
    public void SocketBufferTest()
    {
        Error error = TransportFactory.createError();
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.unifiedNetworkInfo().address("localhost");
        opts.unifiedNetworkInfo().serviceName("14002");
        opts.guaranteedOutputBuffers(1);

        // setup the RsslSocketChannel
        Transport._globalLock = new DummyLock();
        SocketProtocol transport = new SocketProtocol(opts);

        RsslSocketChannel rsslChnl = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._highWaterMark = 100000;
        rsslChnl._readLock = new DummyLock();
        rsslChnl._writeLock = new DummyLock();
        rsslChnl._protocolFunctions = rsslChnl.ripcProtocolFunctions;
        rsslChnl.dataFromOptions(opts);
        rsslChnl.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        TransportBufferImpl transBuf1 = (TransportBufferImpl)rsslChnl.getBuffer(2000, false, error);
        transBuf1.data().put("Buffer 1".getBytes());
        TransportBufferImpl transBuf2 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf2.data().put("Buffer 2".getBytes());
        rsslChnl.write(transBuf2, writeArgs, error);
        rsslChnl.write(transBuf1, writeArgs, error);
        TransportBufferImpl transBuf3 = (TransportBufferImpl)rsslChnl.getBuffer(2000, false, error);
        transBuf3.data().put("Buffer 3".getBytes());
        TransportBufferImpl transBuf4 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf4.data().put("Buffer 4".getBytes());
        TransportBufferImpl transBuf5 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf5.data().put("Buffer 5".getBytes());
        rsslChnl.write(transBuf5, writeArgs, error);
        rsslChnl.write(transBuf3, writeArgs, error);
        rsslChnl.write(transBuf4, writeArgs, error);
        rsslChnl.close(error);

        RsslSocketChannel rsslChnl2 = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl2._state = ChannelState.ACTIVE;
        rsslChnl2._highWaterMark = 100000;
        rsslChnl2._readLock = new DummyLock();
        rsslChnl2._writeLock = new DummyLock();
        rsslChnl2._protocolFunctions = rsslChnl.ripcProtocolFunctions;
        rsslChnl2.dataFromOptions(opts);
        rsslChnl2.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        TransportBufferImpl transBuf21 = (TransportBufferImpl)rsslChnl2.getBuffer(2000, false, error);
        transBuf21.data().put("Buffer 21".getBytes());
        TransportBufferImpl transBuf22 = (TransportBufferImpl)rsslChnl2.getBuffer(3000, false, error);
        transBuf22.data().put("Buffer 22".getBytes());
        rsslChnl2.write(transBuf22, writeArgs, error);
        rsslChnl2.write(transBuf21, writeArgs, error);
        TransportBufferImpl transBuf23 = (TransportBufferImpl)rsslChnl2.getBuffer(2000, false, error);
        transBuf23.data().put("Buffer 23".getBytes());
        TransportBufferImpl transBuf24 = (TransportBufferImpl)rsslChnl2.getBuffer(4000, false, error);
        transBuf24.data().put("Buffer 4".getBytes());
        TransportBufferImpl transBuf25 = (TransportBufferImpl)rsslChnl2.getBuffer(3000, false, error);
        transBuf25.data().put("Buffer 25".getBytes());
        rsslChnl2.write(transBuf25, writeArgs, error);
        rsslChnl.write(transBuf23, writeArgs, error);
        rsslChnl.write(transBuf24, writeArgs, error);
        rsslChnl2.close(error);

        RsslSocketChannel rsslChnl3 = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl3._state = ChannelState.ACTIVE;
        rsslChnl3._highWaterMark = 100000;
        rsslChnl3._readLock = new DummyLock();
        rsslChnl3._writeLock = new DummyLock();
        rsslChnl3._protocolFunctions = rsslChnl.ripcProtocolFunctions;
        rsslChnl3.dataFromOptions(opts);
        rsslChnl3.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        TransportBufferImpl transBuf31 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf31.data().put("Buffer 21".getBytes());
        TransportBufferImpl transBuf32 = (TransportBufferImpl)rsslChnl3.getBuffer(4000, false, error);
        transBuf32.data().put("Buffer 22".getBytes());
        rsslChnl3.write(transBuf32, writeArgs, error);
        rsslChnl3.write(transBuf31, writeArgs, error);
        TransportBufferImpl transBuf33 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf33.data().put("Buffer 23".getBytes());
        TransportBufferImpl transBuf34 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf34.data().put("Buffer 4".getBytes());
        TransportBufferImpl transBuf35 = (TransportBufferImpl)rsslChnl3.getBuffer(4000, false, error);
        transBuf35.data().put("Buffer 25".getBytes());
        rsslChnl3.write(transBuf35, writeArgs, error);
        rsslChnl.write(transBuf33, writeArgs, error);
        rsslChnl.write(transBuf34, writeArgs, error);
        rsslChnl3.close(error);

        rsslChnl = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._highWaterMark = 100000;
        rsslChnl._readLock = new DummyLock();
        rsslChnl._writeLock = new DummyLock();
        rsslChnl.dataFromOptions(opts);
        rsslChnl.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        transBuf1 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf1.data().put("Buffer 1".getBytes());
        transBuf2 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf2.data().put("Buffer 2".getBytes());
        rsslChnl.write(transBuf2, writeArgs, error);
        rsslChnl.write(transBuf1, writeArgs, error);
        transBuf3 = (TransportBufferImpl)rsslChnl.getBuffer(2000, false, error);
        transBuf3.data().put("Buffer 3".getBytes());
        transBuf4 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf4.data().put("Buffer 4".getBytes());
        rsslChnl.write(transBuf3, writeArgs, error);
        rsslChnl.write(transBuf4, writeArgs, error);
        transBuf5 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf5.data().put("Buffer 5".getBytes());
        rsslChnl.write(transBuf5, writeArgs, error);
        rsslChnl.close(error);

        rsslChnl3 = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl3._state = ChannelState.ACTIVE;
        rsslChnl3._highWaterMark = 100000;
        rsslChnl3._readLock = new DummyLock();
        rsslChnl3._writeLock = new DummyLock();
        rsslChnl3.dataFromOptions(opts);
        rsslChnl3.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        transBuf31 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf31.data().put("Buffer 1".getBytes());
        transBuf32 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf32.data().put("Buffer 2".getBytes());
        transBuf33 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf33.data().put("Buffer 3".getBytes());
        transBuf34 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf34.data().put("Buffer 4".getBytes());
        rsslChnl3.write(transBuf33, writeArgs, error);
        rsslChnl3.write(transBuf34, writeArgs, error);
        transBuf35 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf35.data().put("Buffer 25".getBytes());
        rsslChnl3.write(transBuf35, writeArgs, error);
        rsslChnl3.write(transBuf31, writeArgs, error);
        rsslChnl3.write(transBuf32, writeArgs, error);
        rsslChnl3.close(error);

        rsslChnl = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._highWaterMark = 100000;
        rsslChnl._readLock = new DummyLock();
        rsslChnl._writeLock = new DummyLock();
        rsslChnl.dataFromOptions(opts);
        rsslChnl.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        transBuf1 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf1.data().put("Buffer 1".getBytes());
        transBuf2 = (TransportBufferImpl)rsslChnl.getBuffer(2000, false, error);
        transBuf2.data().put("Buffer 2".getBytes());
        rsslChnl.write(transBuf1, writeArgs, error);
        rsslChnl.write(transBuf2, writeArgs, error);
        transBuf3 = (TransportBufferImpl)rsslChnl.getBuffer(2000, false, error);
        transBuf3.data().put("Buffer 3".getBytes());
        transBuf4 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf4.data().put("Buffer 4".getBytes());
        rsslChnl.write(transBuf4, writeArgs, error);
        rsslChnl.write(transBuf3, writeArgs, error);
        transBuf5 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf5.data().put("Buffer 5".getBytes());
        rsslChnl.write(transBuf5, writeArgs, error);
        rsslChnl.close(error);

        rsslChnl3 = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl3._state = ChannelState.ACTIVE;
        rsslChnl3._highWaterMark = 100000;
        rsslChnl3._readLock = new DummyLock();
        rsslChnl3._writeLock = new DummyLock();
        rsslChnl3.dataFromOptions(opts);
        rsslChnl3.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        transBuf31 = (TransportBufferImpl)rsslChnl3.getBuffer(4000, false, error);
        transBuf31.data().put("Buffer 1".getBytes());
        rsslChnl3.write(transBuf31, writeArgs, error);
        transBuf32 = (TransportBufferImpl)rsslChnl3.getBuffer(4000, false, error);
        transBuf32.data().put("Buffer 2".getBytes());
        rsslChnl3.write(transBuf32, writeArgs, error);
        transBuf33 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf33.data().put("Buffer 3".getBytes());
        rsslChnl3.write(transBuf33, writeArgs, error);
        transBuf34 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf34.data().put("Buffer 4".getBytes());
        rsslChnl3.write(transBuf34, writeArgs, error);
        transBuf35 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf35.data().put("Buffer 25".getBytes());
        rsslChnl3.write(transBuf35, writeArgs, error);
        rsslChnl3.close(error);

        rsslChnl = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl._state = ChannelState.ACTIVE;
        rsslChnl._highWaterMark = 100000;
        rsslChnl._readLock = new DummyLock();
        rsslChnl._writeLock = new DummyLock();
        rsslChnl.dataFromOptions(opts);
        rsslChnl.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        transBuf1 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf1.data().put("Buffer 1".getBytes());
        transBuf2 = (TransportBufferImpl)rsslChnl.getBuffer(3000, false, error);
        transBuf2.data().put("Buffer 2".getBytes());
        rsslChnl.write(transBuf1, writeArgs, error);
        rsslChnl.write(transBuf2, writeArgs, error);
        transBuf3 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf3.data().put("Buffer 3".getBytes());
        transBuf4 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf4.data().put("Buffer 4".getBytes());
        rsslChnl.write(transBuf3, writeArgs, error);
        rsslChnl.write(transBuf4, writeArgs, error);
        transBuf5 = (TransportBufferImpl)rsslChnl.getBuffer(4000, false, error);
        transBuf5.data().put("Buffer 5".getBytes());
        rsslChnl.write(transBuf5, writeArgs, error);
        rsslChnl.close(error);

        rsslChnl3 = (RsslSocketChannel) transport.channel(opts, error);
        rsslChnl3._state = ChannelState.ACTIVE;
        rsslChnl3._highWaterMark = 100000;
        rsslChnl3._readLock = new DummyLock();
        rsslChnl3._writeLock = new DummyLock();
        rsslChnl3.dataFromOptions(opts);
        rsslChnl3.growGuaranteedOutputBuffers(opts.guaranteedOutputBuffers());

        transBuf31 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf31.data().put("Buffer 1".getBytes());
        transBuf32 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf32.data().put("Buffer 2".getBytes());
        transBuf33 = (TransportBufferImpl)rsslChnl3.getBuffer(4000, false, error);
        transBuf33.data().put("Buffer 3".getBytes());
        transBuf34 = (TransportBufferImpl)rsslChnl3.getBuffer(3000, false, error);
        transBuf34.data().put("Buffer 4".getBytes());
        rsslChnl3.write(transBuf31, writeArgs, error);
        rsslChnl3.write(transBuf32, writeArgs, error);
        transBuf35 = (TransportBufferImpl)rsslChnl3.getBuffer(2000, false, error);
        transBuf35.data().put("Buffer 25".getBytes());
        rsslChnl3.write(transBuf35, writeArgs, error);
        rsslChnl3.write(transBuf33, writeArgs, error);
        rsslChnl3.write(transBuf34, writeArgs, error);
        rsslChnl3.close(error);
    }
    
    /*
     * Verify that when ServerSocketChannel.accept returns null 
     * ServerImpl.accept properly populates error data and returns null. 
     * This test will create server in non blocking mode (via Transport.bind()), 
     * then invoke accept method then verify that this method returns bull
     * and populates error object with correct data.
     */
    @Test
    public void ifAcceptReceivesNullItReturnsNullTooTest()
    {
        final Error error = TransportFactory.createError();

        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.sharedPoolSize(3);
        bindOpts.serverBlocking(false);

        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(false);

        Server server = null;

        try
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                                  + error.sysError(), server);

            AcceptOptions acceptOp = TransportFactory.createAcceptOptions();
            
            assertNull(server.accept(acceptOp, error));  
            assertNull(error.channel());
            assertEquals(TransportReturnCodes.FAILURE, error.errorId());
            assertEquals(0, error.sysError());
            assertEquals("This channel is in non-blocking mode and no connection is available to be accepted", error.text());
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
}

