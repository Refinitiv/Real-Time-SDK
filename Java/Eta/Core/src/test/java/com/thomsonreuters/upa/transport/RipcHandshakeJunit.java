package com.thomsonreuters.upa.transport;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.test.network.replay.NetworkReplay;
import com.thomsonreuters.upa.test.network.replay.NetworkReplayFactory;
import com.thomsonreuters.upa.transport.Ripc.CompressionTypes;

public class RipcHandshakeJunit
{
    final static boolean DEBUG = false;
    final static int SLEEPTIMEMS = 25; // ms
    final static int TIMEOUTMS = 10000; // 10 seconds.
    final static int MAX_EXPECTED_COMPONENT_VERSION_SIZE = 253;
    final static String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/thomsonreuters/upa/transport/RipcHandshakeJunit";
    final static int DEFAULT_LISTEN_PORT = 4322;
    final static String DEFAULT_LISTEN_PORT_AS_STRING = Integer.toString(DEFAULT_LISTEN_PORT);
    final static int KEY_EXCHANGE = 8;

    /*
     * This field denotes the current (i.e. default) RIPC version.
     */
    final static int CURRENT_RIPC_VERSION = 14;

    final static int RIPC_VERSION_13 = 13;

    /*
     * Client component Version. receivedComponentVersion and
     * sentComponentVersion are hard coded in the TestData files.
     * userSpecifiedComponentVersion is the user specified component version.
     */
    final static String receivedComponentVersion = "UPA Java Edition";
    final static String sentComponentVersion13 = "Test Client Version 1.23";
    final static String sentComponentVersion14 = "UPA Java Edition";
    final static String userSpecifiedComponentVersion = "User Specified Client Version 5.43";

    StringBuilder _sb = new StringBuilder();

    /*
     * Invokes RsslTransport.initialize()
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
     * Parses a network replay file and returns a populated
     * NetworkReplay
     * 
     * @param fileName The name of the network replay file to parse
     * @return A populated NetworkReplay
     * @throws IOException Thrown if the file could not be parsed
     */
    static NetworkReplay parseReplayFile(String fileName) throws IOException
    {
        NetworkReplay fileReplay = NetworkReplayFactory.create();
        fileReplay.parseFile(fileName);

        return fileReplay;
    }

    /*
     * Connects to the RSSL server and returns the channel to the caller.
     */
    static Channel connectToRsslServer(String hostname, String portno, Error error)
    {
        return connectToRsslServer(hostname, portno, Codec.protocolType(), Ripc.CompressionTypes.NONE, error);
    }

    /*
     * Connects to the RSSL server and returns the channel to the caller.
     */
    static Channel connectToRsslServer(String hostname, String portno, int protocolType, int compressionType, Error error)
    {
        Channel chnl;
        ConnectOptions copts = TransportFactory.createConnectOptions();
        copts.unifiedNetworkInfo().address(hostname);
        copts.unifiedNetworkInfo().serviceName(portno);

        copts.guaranteedOutputBuffers(500);
        copts.majorVersion(Codec.majorVersion());
        copts.minorVersion(Codec.minorVersion());
        copts.protocolType(protocolType);
        copts.compressionType(compressionType);

        if ((chnl = Transport.connect(copts, error)) != null)
        {
            System.out.println("\nChannel " + chnl.selectableChannel());
        }

        return chnl;
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
                        System.out
                                .println("\nChannel " + channel.selectableChannel() + " In Progress...");
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

    /* returns the connection version for a specified ripc version */
    static int connectionVersion(long ripcVersion)
    {
        int connectionVersion = 0;

        if (ripcVersion == 14)
            connectionVersion = Ripc.ConnectionVersions.VERSION14;
        else if (ripcVersion == 13)
            connectionVersion = Ripc.ConnectionVersions.VERSION13;
        else if (ripcVersion == 12)
            connectionVersion = Ripc.ConnectionVersions.VERSION12;
        else if (ripcVersion == 11)
            connectionVersion = Ripc.ConnectionVersions.VERSION11;

        return connectionVersion;
    }

    /* returns the ipc version for a specified ripc version */
    private int ipcVersion(long ripcVersion)
    {
        int ipcVersion = 0;

        if (ripcVersion == 14)
            ipcVersion = Ripc.RipcVersions.VERSION14;
        else if (ripcVersion == 13)
            ipcVersion = Ripc.RipcVersions.VERSION13;
        else if (ripcVersion == 12)
            ipcVersion = Ripc.RipcVersions.VERSION12;
        else if (ripcVersion == 11)
            ipcVersion = Ripc.RipcVersions.VERSION11;

        return ipcVersion;
    }

    /* compares the ByteBuffer to the expected ripc connection request */
    static void compareExpectedConnectReq(ByteBuffer buffer, int ripcVersion, String expectedComponentVersion)
    {
        compareExpectedConnectReq(buffer, ripcVersion, Codec.protocolType(), Ripc.CompressionTypes.NONE, expectedComponentVersion);
    }
    
    /* compares the ByteBuffer to the expected ripc connection request */
    static void compareExpectedConnectReq(ByteBuffer buffer, int ripcVersion, int protocolType, int compressionType, String expectedComponentVersion)
    {
        int idx = 0;

        /* Message Length - two bytes */
        int messageLength = buffer.getShort(idx);
        idx += 2;

        /* Flags */
        assertEquals(0x00, buffer.get(idx++));

        /* Connection Version */
        int version = buffer.getInt(idx);
        idx += 4;

        assertEquals(connectionVersion(ripcVersion), version);

        /* Flags */
        if (ripcVersion == 14)
        {
            assertEquals(0x08, buffer.get(idx++));
        }
        else
        {
            assertEquals(0x0, buffer.get(idx++));
        }

        /* Header Length - same as length */
        int headerLength = buffer.get(idx++) & 0xFF;
        
        /* Compression Bitmap Size */
        if(compressionType == Ripc.CompressionTypes.NONE)
        {
            assertEquals(compressionType, buffer.get(idx++));
        }
        else if (compressionType == Ripc.CompressionTypes.ZLIB)
        {
            assertEquals(1, buffer.get(idx++));
            assertEquals(compressionType, buffer.get(idx++));
        }

        /* Ping Timeout */
        assertEquals(0x3C, buffer.get(idx++));

        /* Session Flags */
        assertEquals(0x00, buffer.get(idx++));

        /* Protocol Type */
        if (ripcVersion >= 12)
        {
            assertEquals(protocolType, buffer.get(idx++));
        }

        /* Major Version */
        assertEquals(0x0e, buffer.get(idx++));

        /* Minor Version */
        int minorVersion = buffer.get(idx++);
        if (ripcVersion == 14)
        {
            assertEquals(0x01, minorVersion);
        }
        else
        {
            if (minorVersion != 0 && minorVersion != 1)
            {
                fail();
            }
        }

        String hostname = null;
        String ipAddress = null;

        try
        {
            hostname = InetAddress.getLocalHost().getHostName();
            ipAddress = InetAddress.getLocalHost().getHostAddress();
        }
        catch (UnknownHostException e)
        {
            fail("exception while determining host and ipaddress, exception=" + e.toString());
        }

        /* HostName length and String */
        assertEquals(hostname.length(), buffer.get(idx++));
        for (int stringIdx = 0; stringIdx < hostname.length(); stringIdx++, idx++)
        {
            assertEquals("mismatch at idx=" + idx, hostname.charAt(stringIdx), buffer.get(idx));
        }

        /* IpAddress length and String */
        assertEquals(ipAddress.length(), buffer.get(idx++));
        for (int stringIdx = 0; stringIdx < ipAddress.length(); stringIdx++, idx++)
        {
            assertEquals("mismatch at idx=" + idx, ipAddress.charAt(stringIdx), buffer.get(idx));
        }

        /* Component Version Container */
        if (ripcVersion >= 13)
        {
            /* Connected Component Version - container length (u15-rb) */
            int componentVersionContainerLen = buffer.get(idx++) & 0xFF;
            int componentVersionLen = 0;
            if (componentVersionContainerLen > 0)
            {
                /* Connected Component Version - name length (u15-rb) */
                componentVersionLen = buffer.get(idx++) & 0xFF;
                if (componentVersionLen > 0)
                {
                    byte[] ccnBytes = new byte[componentVersionLen];
                    buffer.position(idx);
                    buffer.get(ccnBytes, 0, componentVersionLen);
                    idx += componentVersionLen;
                    assertEquals(expectedComponentVersion, new String(ccnBytes));
                }
            }

            // add componentContainer length to HeaderLength for comparison which follows.
            headerLength += componentVersionLen + 2;
        }
        
         
        assertEquals(messageLength, headerLength);

        /* verify that the decoded length equals the specified message length */
        assertEquals(messageLength, idx);
    }

    /* compares the ByteBuffer to the expected ripc connection ack */
    public void compareExpectedConnectAck(ByteBuffer buffer, int ripcVersion, String expectedComponentVersion)
    {
        compareExpectedConnectAck(buffer, ripcVersion, expectedComponentVersion, 0x3C,
                                  Codec.majorVersion(), Codec.minorVersion(), Ripc.CompressionTypes.NONE, 0);
    }
    
    /* compares the ByteBuffer to the expected ripc connection ack */
    public void compareExpectedConnectAck(ByteBuffer buffer, int ripcVersion, String expectedComponentVersion, int pingTimeout, int majorVersion, int minorVersion, int compressionType, int compressionLevel)
    {
        int idx = 0;

        /* Message Length - two bytes */
        int messageLength = buffer.getShort(idx);
        idx += 2;

        /* Flags */
        assertEquals(0x01, buffer.get(idx++));

        /* Extended Header Flags - ConnectAck */
        assertEquals(0x01, buffer.get(idx++));

        /* Header Length */
        int expectedHeaderLength = 0;
        switch (ripcVersion)
        {
            case 14:
                expectedHeaderLength = Ripc14Protocol.CONNECT_ACK_HEADER;
                break;
            case 13:
                expectedHeaderLength = Ripc13Protocol.CONNECT_ACK_HEADER;
                break;
            case 12:
                expectedHeaderLength = Ripc12Protocol.CONNECT_ACK_HEADER;
                break;
            case 11:
                expectedHeaderLength = Ripc11Protocol.CONNECT_ACK_HEADER;
                break;
        }
        assertEquals(expectedHeaderLength, buffer.get(idx++));

        /* unknown/unused byte */
        assertEquals(0x00, buffer.get(idx++));

        /* Ipc Version */
        int version = buffer.getInt(idx);
        idx += 4;
        assertEquals(ipcVersion(ripcVersion), version);

        /* Max User Msg Size */
        assertEquals(0x1800, (buffer.getShort(idx) & 0xFFFF));
        idx += 2;

        /* Session Flags */
        assertEquals(0x03, buffer.get(idx++) & 0xFF);

        /* Ping Timeout */
        assertEquals(pingTimeout, buffer.get(idx++) & 0xFF);

        /* Major Version */
        assertEquals(majorVersion, buffer.get(idx++) & 0xFF);

        /* Minor Version */
        int versionRead = buffer.get(idx++) & 0xFF;
        if (ripcVersion == 14)
        {
            if (versionRead != minorVersion && versionRead != 1)
            {
                fail();
            }
        }
        else
        {
            if (versionRead != minorVersion && versionRead != 0 && versionRead != 1)
            {
                fail();
            }
        }

        /* Compression Type */
        assertEquals(compressionType, (buffer.getShort(idx) & 0xFFFF));
        idx += 2;

        /* Compression Level */
        assertEquals(compressionLevel, buffer.get(idx++) & 0xFF);
        
        /* new RIPC handshake stuff for encryption */
        if (ripcVersion == 14)
        {
            assertEquals(KEY_EXCHANGE, buffer.get(idx++)); // verify KEY_EXCHANGE flag
            idx += 1; // skip encryption type
            // get key info length
            int keyInfoLen = buffer.get(idx++);
            idx += keyInfoLen; // skip key info
        }

        /* Component Version Container */
        if (ripcVersion >= 13)
        {
            /* Connected Component Version - container length (u15-rb) */
            int componentVersionContainerLen = buffer.get(idx++) & 0xFF;
            
            /* Connected Component Version - name length (u15-rb) */
            int componentVersionLen = buffer.get(idx++) & 0xFF;
            if (componentVersionLen > 0)
            {
                byte[] ccnBytes = new byte[componentVersionLen];
                buffer.position(idx);
                buffer.get(ccnBytes, 0, componentVersionLen);
                idx += componentVersionLen;
                assertEquals(expectedComponentVersion, new String(ccnBytes));
            }
            
            assertEquals(componentVersionLen+1, componentVersionContainerLen);
        }

        /* verify that the decoded length equals the specified message length */
        assertEquals(messageLength, idx);
    }
    
    /* compares the ByteBuffer to the expected ripc connection nak */
    public void compareExpectedConnectNak(ByteBuffer buffer, int ripcVersion)
    {
        int idx = 0;

        /* Message Length - two bytes */
        int messageLength = buffer.getShort(idx);
        idx += 2;

        /* Flags */
        assertEquals(0x01, buffer.get(idx++));

        /* Extended Header Flags - ConnectNak */
        assertEquals(0x02, buffer.get(idx++));

        /* Header Length - verify later */
        int actualHeaderLength = buffer.get(idx++) & 0xFF;

        /* unknown/unused byte */
        assertEquals(0x00, buffer.get(idx++));

        /* Text Length */
        int textLen = buffer.getShort(idx) & 0xFF;
        idx += 2;

        /* verify Header Length */
        int expectedHeaderLength = 0;
        byte[] expectedTextBytes = null;

        switch (ripcVersion)
        {
            case 14:
                expectedHeaderLength += Ripc14Protocol.CONNECT_NAK_HEADER;
                expectedTextBytes = Ripc14Protocol.CONNECTION_REFUSED_BYTES;
                break;
            case 13:
                expectedHeaderLength += Ripc13Protocol.CONNECT_NAK_HEADER;
                expectedTextBytes = Ripc13Protocol.CONNECTION_REFUSED_BYTES;
                break;
            case 12:
                expectedHeaderLength += Ripc12Protocol.CONNECT_NAK_HEADER;
                expectedTextBytes = Ripc12Protocol.CONNECTION_REFUSED_BYTES;
                break;
            case 11:
                expectedHeaderLength += Ripc11Protocol.CONNECT_NAK_HEADER;
                expectedTextBytes = Ripc11Protocol.CONNECTION_REFUSED_BYTES;
                break;
        }

        /* Text */
        if (textLen > 0)
        {
            byte[] ccnBytes = new byte[textLen];
            buffer.position(idx);
            buffer.get(ccnBytes, 0, textLen);
            idx += textLen;
            
            /* verify each byte - Add one byte to expectedTextBytes to include NULL for C */
            assertEquals(expectedTextBytes.length + 1, ccnBytes.length);
            int i = 0;
            for(; i < expectedTextBytes.length; i++)
                assertEquals(expectedTextBytes[i], ccnBytes[i]);
            /* verify NULL for C */
            assertEquals(0, ccnBytes[i]);
        }

        assertEquals(expectedHeaderLength, actualHeaderLength);

        /* verify that the decoded length equals the specified message length */
        assertEquals(messageLength, idx);
        assertEquals(expectedHeaderLength + expectedTextBytes.length + 1, messageLength);
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

        if ((consumerChannel != null) && (consumerChannel.state() != ChannelState.INACTIVE))
        {
            int ret = consumerChannel.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                fail("consumerChannel.close() failed with return code " + ret
                        + ", error=" + error.text());
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
    private void cleanupForTestsWithTestClient(NetworkReplay replay, Server server,
            Channel serverChannel, TestClient client, Selector selector, Error error)
    {
        int ret;

        if (replay != null)
        {
            replay.stopListener();
            replay = null;
        }

        if (server != null)
        {
            ret = server.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                fail("server.close() failed with return code " + ret + ", error="
                        + error.text());
            }
            server = null;
        }

        if ((serverChannel != null) && ( ((RsslSocketChannel)serverChannel)._state != ChannelState.INACTIVE))
        {
            ret = serverChannel.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                fail("serverChannel.close() failed with return code " + ret
                        + ", error=" + error.text());
            }
            serverChannel = null;
        }
        
        assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());

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
    }

    /* clean up the client and server */
    private void cleanup(Server server, Channel serverChannel, Channel clientChannel, Selector selector, Error error)
    {
        assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());

        // clean up the server
        int ret;
        
        if (server != null)
        {
            ret = server.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                System.out.println("server.close() failed with return code " + ret + ", error="
                        + error.text());
            }
            server = null;
        }

        if (serverChannel != null)
        {
            ret = serverChannel.flush(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                System.out.println("serverChannel.flush() failed with return code " + ret
                        + error.text());
            }
            
            ret = serverChannel.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                System.out.println("serverChannel.close() failed with return code " + ret
                        + error.text());
            }
            serverChannel = null;
        }

        // clean up the client
        if (clientChannel != null)
        {
            ret = clientChannel.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                System.out.println("clientChannel.close() failed with return code " + ret
                        + ", error=" + error.text());
            }
            clientChannel = null;
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
        
        // give client time to shut down.
        try
        {
            Thread.sleep(100);
        }
        catch (InterruptedException e)
        {
        }
    }

    /* compare the componentVersion string to the received ComponentInfo */
    static void compareComponentVersionToReceivedComponentInfo(String componentVersion, Channel consumerChannel, Error error)
    {
        ChannelInfo info = TransportFactory.createChannelInfo();
        int ret = consumerChannel.info(info, error);
        assertEquals("channel.info() failed, error=" + error.text(), TransportReturnCodes.SUCCESS,
                     ret);
        System.out.println("ChannelInfo = \n" + info.toString());

        List<ComponentInfo> ciList = info.componentInfo();
        assertNotNull(ciList);
        assertEquals(1, ciList.size());

        // compare a String to a Buffer.
        int stringLen = componentVersion.length();
        Buffer cvBuffer = ciList.get(0).componentVersion();
        assertNotNull(cvBuffer);

        int cvPos = cvBuffer.position();
        int cvLen = cvBuffer.length();
        assertEquals(stringLen, cvLen);

        ByteBuffer cvBB = cvBuffer.data();
        for (int i = 0; i < stringLen; i++)
        {
            assertEquals("mismatch at index " + i + " expected " + componentVersion.charAt(i)
                                 + " but was " + cvBB.get(cvPos + i), componentVersion.charAt(i),
                         cvBB.get(cvPos + i));
        }
    }

    /*
     * This test assumes that the current RIPC version is 13. In this test, the
     * UPAJ client will connect to the TestServer and send a ConnectReq. The
     * TestServer will verify the ConnectReq (as RIPC13) along with the
     * component version. The TestServer will send a complete ConnectAck in a
     * single packet. initChannel() will be called until the channel is active
     * (indicating that the client received a valid ConnectAck) in which case
     * the received Component Info will be verified, or until a timeout occurs.
     */
    @Test
    public void clientRcvSingleConnectAckTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/10_input_connectAck_Ripc14.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.waitForAcceptable();
            server.acceptSocket();

            // initChannel should send connectReq to our server.
            // wait for the channel to become active.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            server.waitForReadable();
            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + server.buffer().position() + " hex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            compareExpectedConnectReq(server.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion());

            // write ConnectAck to our client.
            server.writeMessageToSocket(replay.read());

            int retry = 5;
            while (retry-- > 0)
            {
                // initChannel should read connectAck.
                // wait for channel to become active.
                channelState = initChannel(consumerChannel, error, inProg);
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.ACTIVE, channelState);
            assertNull(error.text());

            compareComponentVersionToReceivedComponentInfo(receivedComponentVersion, consumerChannel, error);

        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, null, error);
        }
    }

    /*
     * This test assumes that the current RIPC version is 13. In this test, the
     * UPAJ client will connect to the TestServer and send a ConnectReq with a
     * user specified component version. The TestServer will verify the
     * ConnectReq (as RIPC13) along with the user specified component version.
     * The TestServer will send a complete ConnectAck in a single packet.
     * initChannel() will be called until the channel is active (indicating that
     * the client received a valid ConnectAck) in which case the received
     * Component Info will be verified, or until a timeout occurs.
     */
    @Test
    public void clientUserSpecifiedComponentVersion_clientRcvSingleConnectAckTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/10_input_connectAck_Ripc14.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            // specify ComponentVersion
            ByteBuffer componentVersionBB = ByteBuffer.wrap(userSpecifiedComponentVersion.getBytes());
            ComponentInfo myCi = TransportFactory.createComponentInfo();
            myCi.componentVersion().data(componentVersionBB);
            int ret = consumerChannel.ioctl(IoctlCodes.COMPONENT_INFO, myCi, error);
            assertEquals(TransportReturnCodes.SUCCESS, ret);

            server.waitForAcceptable();
            server.acceptSocket();

            // initChannel should send connectReq to our server.
            // wait for the channel to become active.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            server.waitForReadable();
            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + server.buffer().position() + " hex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }
            
            compareExpectedConnectReq(server.buffer(), CURRENT_RIPC_VERSION, userSpecifiedComponentVersion);

            // write ConnectAck to our client.
            server.writeMessageToSocket(replay.read());

            int retry = 5;
            while (retry-- > 0)
            {
                // initChannel should read connectAck.
                // wait for channel to become active.
                channelState = initChannel(consumerChannel, error, inProg);
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.ACTIVE, channelState);
            assertNull(error.text());

            compareComponentVersionToReceivedComponentInfo(receivedComponentVersion, consumerChannel, error);

        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, null, error);
        }
    }

    /*
     * This test assumes that the current RIPC version is 13. In this test, the
     * UPAJ client will connect to the TestServer and send a ConnectReq. The
     * TestServer will verify the ConnectReq (as RIPC13) along with the
     * component version. The TestServer will send a ConnectAck with each byte
     * in it's own packet. This will verify that the client can keep reading
     * until a complete message is received. initChannel() will then be called
     * until the channel is active (indicating that the client received a valid
     * ConnectAck), or until a timeout occurs.
     */
    @Test
    public void clientRcvAllSplitConnectAckTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/20_input_connectAck_Ripc14_all_split.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.waitForAcceptable();
            server.acceptSocket();

            // initChannel should send connectReq to our server.
            // wait for the channel to become active.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            server.waitForReadable();
            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + server.buffer().position() + " hex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            compareExpectedConnectReq(server.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion());

            /* now write the split ConnectAck to client in pieces. */
            int retry = 67; // 64 bytes in file.
            while (retry-- > 0)
            {
                byte[] bytesToWrite = replay.read();
                int bytesWritten = 0;
                if (bytesToWrite != null)
                    bytesWritten = server.writeMessageToSocket(bytesToWrite);

                // initChannel should read connectAck.
                // wait for the channel to become active.
                channelState = initChannel(consumerChannel, error, inProg);
                if (DEBUG)
                {
                    System.out.println("DEBUG: retry=" + retry + " bytesWritten=" + bytesWritten
                            + " channelState=" + ChannelState.toString(channelState));
                }
                
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.ACTIVE, channelState);
            assertNull(error.text());

            compareComponentVersionToReceivedComponentInfo(receivedComponentVersion, consumerChannel, error);
        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, null, error);
        }
    }

    /*
     * This test assumes that the current RIPC version is 13. In this test, the
     * UPAJ client will connect to the TestServer and send a ConnectReq. The
     * TestServer will verify the ConnectReq (as RIPC13) along with the
     * component version. The TestServer will send a complete ConnectAck in a
     * single packet with the wrong RIPC version. initChannel() will be called
     * until the channel is inactive (indicating that the client received
     * ConnectAck with wrong RIPC version), or until a timeout occurs.
     */
    @Test
    public void clientRcvSingleConnectAckWithWrongRipcVersionTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/30_input_connectAck_with_wrong_ripcVersion.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.waitForAcceptable();
            server.acceptSocket();

            // initChannel should send connectReq to our server.
            // wait for the channel to become active.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            server.waitForReadable();
            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + server.buffer().position() + " hex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            compareExpectedConnectReq(server.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion());

            // write ConnectAck to our client.
            server.writeMessageToSocket(replay.read());

            int retry = 5;
            while (retry-- > 0)
            {
                // initChannel should read connectAck and fail since the
                // RipcVersion is wrong.
                channelState = initChannel(consumerChannel, error, inProg);
                if (error.text() != null)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.INACTIVE, channelState);
            assertNotNull(error.text());

        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, null, error);
        }
    }

    /*
     * This test assumes that the current RIPC version is 13. In this test, the
     * UPAJ client will connect to the TestServer and send a ConnectReq with
     * ZLIB compression requested. The TestServer will verify the ConnectReq (as
     * RIPC13) along with the component version. The TestServer will send a
     * complete ConnectAck in a single packet, with ZLIB compression type and
     * the default compression level 6. initChannel() will be called until the
     * channel is active (indicating that the client received a valid
     * ConnectAck) in which case the received Component Info will be verified,
     * or until a timeout occurs.
     */
    @Test
    public void clientRcvSingleConnectAckWithCompressionTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/110_input_connectAck_Ripc14_Zlib_compression.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT),
                                                       Codec.protocolType(), Ripc.CompressionTypes.ZLIB, error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.waitForAcceptable();
            server.acceptSocket();

            // initChannel should send connectReq to our server.
            // wait for the channel to become active.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            server.waitForReadable();
            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + server.buffer().position() + " hex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            compareExpectedConnectReq(server.buffer(), CURRENT_RIPC_VERSION, Codec.protocolType(), Ripc.CompressionTypes.ZLIB, Transport.queryVersion().productVersion());

            // write ConnectAck to our client.
            server.writeMessageToSocket(replay.read());

            int retry = 5;
            while (retry-- > 0)
            {
             // initChannel should read connectAck.
                // wait for channel to become active.
                channelState = initChannel(consumerChannel, error, inProg);
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.ACTIVE, channelState);
            assertNull(error.text());

            compareComponentVersionToReceivedComponentInfo(receivedComponentVersion, consumerChannel, error);
        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, null, error);
        }
    }

    /*
     * In this test, the UPAJ client will connect to the TestServer and send a
     * ConnectReq. The TestServer will verify the ConnectReq as the current RIPC
     * version, for example version 13, then close the connection. The client
     * will fall back to RIPC version 12 and send a ConnectReq. The TestServer
     * will verify the ConnectReq and send a complete ConnectAck in a single
     * packet. initChannel() will be called until the channel is active
     * (indicating that the client received a valid ConnectAck), or until a
     * timeout occurs.
     */
    @Test
    public void fallBackToRipc12Test()
    {
        ripcFallBack(12, BASE_TEST_DATA_DIR_NAME + "/40_input_connectAck_Ripc12.txt");
    }

    /*
     * In this test, the UPAJ client will connect to the TestServer and send a
     * ConnectReq. The TestServer will verify the ConnectReq as the current RIPC
     * version, for example version 13, then close the connection. The client
     * will fall back to RIPC version 12 and send a ConnectReq. The TestServer
     * will verify the ConnectReq as version 12, then close the connection. The
     * client will fall back to RIPC version 11 and send a ConnectReq. The
     * TestServer will verify the ConnectReq as version 11 and send a complete
     * ConnectAck in a single packet. initChannel() will be called until the
     * channel is active (indicating that the client received a valid
     * ConnectAck), or until a timeout occurs.
     */
    @Test
    public void fallBackToRipc11Test()
    {
        ripcFallBack(11, BASE_TEST_DATA_DIR_NAME + "/40_input_connectAck_Ripc11.txt");
    }

    /*
     * In this test, the UPAJ client will connect to the TestServer and send a
     * ConnectReq. The TestServer will verify the ConnectReq as the current RIPC
     * version, for example version 13, then close the connection. The client
     * will fall back to RIPC version 12 and send a ConnectReq. The TestServer
     * will verify the ConnectReq as version 12, then close the connection. The
     * client will fall back to RIPC version 11 and send a ConnectReq. The
     * TestServer will verify the ConnectReq as version 11, then close the
     * connection. initChannel() will be called until the channel is inactive
     * (indicating that the client is out of protocols to try), or until a
     * timeout occurs.
     */
    @Test
    public void fallBackToNoSupportedProtocolTest()
    {
        ripcFallBack(10, BASE_TEST_DATA_DIR_NAME + "/40_input_connectAck_Ripc11.txt", true);
    }

    /* generalized version of the fallBack tests */
    private void ripcFallBack(int fallBackRipcVersion, final String inputFile)
    {
        ripcFallBack(fallBackRipcVersion, inputFile, false);
    }

    /*
     * This is a generalized version of the fallBack tests that several other
     * tests will call. It allows the fallBackRicpVersion and inputFile to be
     * specified. It also supports a flag to indicate that it expects to fall
     * back to no protocol. This method will have a the UPAJ client connect to
     * the TestServer and send a ConnectReq. The TestServer will verify the
     * ConnectReq as the current RIPC version, for example version 13, then
     * close the connection. The client will fall back to RIPC version 12 and
     * send a ConnectReq. The TestServer will verify the ConnectReq as version
     * 12, this will continue until the expected version is reached or
     * "no protocols left" is reached.
     */
    private void ripcFallBack(int fallBackRipcVersion, final String inputFile, boolean noProtocolsExpectedFlag)
    {
        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);
        Thread tServer = new Thread(server);
        tServer.start();

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.wait(TestServer.State.SETUP);

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.wait(TestServer.State.ACCEPTED);

            /* we want to force the client to fall back to fallBackRipcVersion */
            int fallBackCount = CURRENT_RIPC_VERSION - fallBackRipcVersion;
            int expectedRipcVersion = CURRENT_RIPC_VERSION;
            int channelState = ChannelState.INACTIVE;
            int retry;

            /*
             * this outside loop controls how many times we need to close the
             * connection to reject the client's connectReq
             */

            while (fallBackCount-- >= 0)
            {
                /*
                 * call initChannel() until we have something to read (the
                 * ConnectReq)
                 */
                retry = 5;
                while (retry-- > 0)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: fallBackcount=" + fallBackCount + " retry="
                                + retry);
                    }
                    // initChannel should send connectReq to our server.
                    channelState = initChannel(consumerChannel, error, inProg);
                    assertEquals(ChannelState.INITIALIZING, channelState);

                    if (server.check(TestServer.State.READABLE))
                        break;
                    else
                        Thread.sleep(SLEEPTIMEMS);
                }

                server.wait(TestServer.State.READABLE);

                if (fallBackCount == -1)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: fallBackCount == -1, getting out of while loop");
                    }
                    /* this should be the ConnectReq we want */
                    break;
                }

                server.readMessageFromSocket();
                if (DEBUG)
                {
                    System.out
                            .println("DEBUG: expectedRipcVersion="
                                    + expectedRipcVersion
                                    + "\nhex:"
                                    + Transport.toHexString(server.buffer(), 0, server.buffer()
                                            .position()));
                }

                compareExpectedConnectReq(server.buffer(), expectedRipcVersion--, Transport.queryVersion().productVersion());

                if (DEBUG)
                {
                    System.out.println("DEBUG: closing socket");
                }
                server.closeSocket();

                if (DEBUG)
                {
                    System.out.println("DEBUG: waiting for server to accept new connection");
                }

                /* UPAJ should establish a new connection. */

                retry = 1000 / SLEEPTIMEMS;
                while (retry-- > 0)
                {
                    if (DEBUG)
                    {
                        System.out
                                .println("DEBUG: waiting for server to accept new connection, retry="
                                        + retry);
                    }

                    // initChannel should send connectReq to our server.
                    channelState = initChannel(consumerChannel, error, inProg);
                    if (noProtocolsExpectedFlag && fallBackCount == 0
                            && channelState == ChannelState.INACTIVE)
                    {
                        // This test should fall back to no more protocols to
                        // try.
                        return;
                    }
                    else
                    {
                        assertEquals(ChannelState.INITIALIZING, channelState);

                        if (server.check(TestServer.State.ACCEPTED))
                            break;
                        else
                            Thread.sleep(SLEEPTIMEMS);
                    }
                }

                if (noProtocolsExpectedFlag && fallBackCount == 0)
                {
                    fail("test is expecting to fall back to no more protocols, but the channelState never transitioned to INACTIVE.");
                }

                if (DEBUG)
                {
                    System.out.println("DEBUG: server accepted new connection");
                }
            }

            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG2: expectedRipcVersion=" + expectedRipcVersion + "\nhex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            compareExpectedConnectReq(server.buffer(), expectedRipcVersion, Transport.queryVersion().productVersion());

            if (DEBUG)
            {
                System.out.println("DEBUG: writing ConnectAck to our client");
            }
            // write ConnectAck to our client.
            server.writeMessageToSocket(replay.read());

            retry = 5;
            while (retry-- > 0)
            {
                // initChannel should read connectAck.
                // wait for the channel to become active
                channelState = initChannel(consumerChannel, error, inProg);
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.ACTIVE, channelState);
            assertNull(error.text());

            if (fallBackRipcVersion >= 13)
            {
                compareComponentVersionToReceivedComponentInfo(receivedComponentVersion, consumerChannel, error);
            }
        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, tServer, error);
        }

        try
        {
            tServer.join();
        }
        catch (InterruptedException e)
        {
            fail("tServer.join() failed, error=" + e.toString());
        }
    }
    
    /*
     * This test will use a non RWF protocol type and the test server will cause
     * the client to fall back from RIPC13, then to RIPC12, then to no protocol
     * rather than RIPC11 since the client is using a non RWF protocol type.
     * This method will have a the UPAJ client connect to the TestServer and
     * send a ConnectReq. The TestServer will verify the ConnectReq as the
     * current RIPC version, for example version 13, then close the connection.
     * The client will fall back to RIPC version 12 and send a ConnectReq. The
     * TestServer will verify the ConnectReq as version 12 and close the
     * connection. Since the client's protocol type does not match the RWF
     * protocol type, the client cannot fall back to RIPC11 and the connection
     * is aborted.
     */
    @Test
    public void fallBackWithNonRwfProtocolType()
    {
        int fallBackRipcVersion = 11; // RIPC11
        boolean noProtocolsExpectedFlag = true;
        
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);
        Thread tServer = new Thread(server);
        tServer.start();

        try
        {
            initTransport(false); // initialize RSSL

            server.wait(TestServer.State.SETUP);

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), 
                                                       123, CompressionTypes.NONE, error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.wait(TestServer.State.ACCEPTED);

            /* we want to force the client to fall back to fallBackRipcVersion */
            int fallBackCount = CURRENT_RIPC_VERSION - fallBackRipcVersion;
            int expectedRipcVersion = CURRENT_RIPC_VERSION;
            int channelState = ChannelState.INACTIVE;
            int retry;

            /*
             * this outside loop controls how many times we need to close the
             * connection to reject the client's connectReq
             */

            while (fallBackCount-- >= 0)
            {
                /*
                 * call initChannel() until we have something to read (the
                 * ConnectReq)
                 */
                retry = 5;
                while (retry-- > 0)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: fallBackcount=" + fallBackCount + " retry="
                                + retry);
                    }
                    // initChannel should send connectReq to our server.
                    channelState = initChannel(consumerChannel, error, inProg);
                    assertEquals(ChannelState.INITIALIZING, channelState);

                    if (server.check(TestServer.State.READABLE))
                        break;
                    else
                        Thread.sleep(SLEEPTIMEMS);
                }

                server.wait(TestServer.State.READABLE);

                if (fallBackCount == -1)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: fallBackCount == -1, getting out of while loop");
                    }
                    /* this should be the ConnectReq we want */
                    break;
                }

                server.readMessageFromSocket();
                if (DEBUG)
                {
                    System.out
                            .println("DEBUG: expectedRipcVersion="
                                    + expectedRipcVersion
                                    + "\nhex:"
                                    + Transport.toHexString(server.buffer(), 0, server.buffer()
                                            .position()));
                }

                compareExpectedConnectReq(server.buffer(), expectedRipcVersion--, 123,
                                          CompressionTypes.NONE, Transport.queryVersion()
                                                  .productVersion());

                if (DEBUG)
                {
                    System.out.println("DEBUG: closing socket");
                }
                server.closeSocket();

                if (DEBUG)
                {
                    System.out.println("DEBUG: waiting for server to accept new connection");
                }

                /* UPAJ should establish a new connection. */

                retry = 1000 / SLEEPTIMEMS;
                while (retry-- > 0)
                {
                    if (DEBUG)
                    {
                        System.out
                                .println("DEBUG: waiting for server to accept new connection, retry="
                                        + retry);
                    }

                    // initChannel should send connectReq to our server.
                    channelState = initChannel(consumerChannel, error, inProg);
                    if (noProtocolsExpectedFlag && fallBackCount == 0
                            && channelState == ChannelState.INACTIVE)
                    {
                        // This test should fall back to no more protocols to
                        // try.
                        return;
                    }
                    else
                    {
                        assertEquals(ChannelState.INITIALIZING, channelState);

                        if (server.check(TestServer.State.ACCEPTED))
                            break;
                        else
                            Thread.sleep(SLEEPTIMEMS);
                    }
                }

                if (noProtocolsExpectedFlag && fallBackCount == 0)
                {
                    fail("test is expecting to fall back to no more protocols, but the channelState never transitioned to INACTIVE.");
                }

                if (DEBUG)
                {
                    System.out.println("DEBUG: server accepted new connection");
                }
            }
            
            assertTrue("This test was expected to fall back to RIPC11, but then abort with no more protocols, due to client specified protocol does not match Codec.protocolType()",
                       false);

        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(null, consumerChannel, server, tServer, error);
            tServer.interrupt();
        }

        try
        {
            tServer.join();
        }
        catch (InterruptedException e)
        {
            fail("tServer.join() failed, error=" + e.toString());
        }
    }

    /*
     * This test assumes that the current RIPC version is 13. In this test, the
     * UPAJ client will connect to the TestServer and send a ConnectReq. The
     * TestServer will verify the ConnectReq (as RIPC13). The TestServer will
     * send a complete ConnectNak in a single packet. initChannel() will be
     * called until the initChannel() call fails and error.text() is populated
     * with the expected error test, or until a timeout occurs.
     */
    @Test
    public void clientRcvSingleConnectNakTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/50_input_connectNak_Ripc13.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            server.setupServerSocket();

            // make UPA call to connect to server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.waitForAcceptable();
            server.acceptSocket();

            // initChannel should send connectReq to our server.
            // wait for the channel to become active.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            server.waitForReadable();
            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + server.buffer().position() + " hex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            compareExpectedConnectReq(server.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion());

            // write ConnectNak to our client.
            server.writeMessageToSocket(replay.read());

            int retry = 5;
            while (retry-- > 0)
            {
                // initChannel should read connectNak.
                // wait for the channel to become active.
                channelState = initChannel(consumerChannel, error, inProg);
                if (error.text() != null)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertTrue("Did not rcv expected error text.", error.text().contains("Error occurred during connection process."));
        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanupForTestsWithTestServer(replay, consumerChannel, server, null, error);
        }
    }

    /* Helper method to bind UPAJ server socket. */
    private Server serverBind(Selector selector, Error error)
    {
        return serverBind(selector, 60, 30, Ripc.CompressionTypes.NONE, 0 /* compression level */,
                          Codec.protocolType(), Codec.majorVersion(), Codec.minorVersion(),
                          false, error);
    }

    /*
     * Helper method to bind UPAJ server socket. 
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
    private Server serverBind(Selector selector, int pingTimeout, int minPingTimeout,
            int compressionType, int compressionLevel, int protocolType, int majorVersion,
            int minorVersion, boolean blocking, Error error)
    {
        BindOptions bOpts = TransportFactory.createBindOptions();

        bOpts.serviceName(DEFAULT_LISTEN_PORT_AS_STRING);
        bOpts.connectionType(ConnectionTypes.SOCKET);
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
    private Channel serverAccept(Selector selector, boolean nakMount, Error error)
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
                            AcceptOptions aOpts = TransportFactory.createAcceptOptions();
                            aOpts.nakMount(nakMount);
                            if (DEBUG)
                                System.out.println("DEBUG: setting nakMount=" + nakMount);
                            Channel channel = server.accept(aOpts, error);
                            if (channel != null)
                            {
                                channel.selectableChannel().register(selector, SelectionKey.OP_READ,
                                                               channel);
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
     * Calls channel.init() until the specified state is reached or timeout
     * occurs.
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
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq. initChannel()
     * will be called on the UPAJ server until the ChannelState is ACTIVE, in
     * which case the UPAJ server will have processed the ConnectReq and sent a
     * ConnectAck. The TestClient will read and verify the ConnectAck (along
     * with component version).
     */
    @Test
    public void serverRcvSingleRipc13ConnectReqTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc13.txt";
        testServerRcvSingleConnectReq(inputFile, 13);
    }

    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC12 ConnectReq. initChannel()
     * will be called on the UPAJ server until the ChannelState is ACTIVE, in
     * which case the UPAJ server will have processed the ConnectReq and sent a
     * ConnectAck. The TestClient will read and verify the ConnectAck (along
     * with component version).
     */
    @Test
    public void serverRcvSingleRipc12ConnectReqTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc12.txt";
        testServerRcvSingleConnectReq(inputFile, 12);
    }

    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC11 ConnectReq. initChannel()
     * will be called on the UPAJ server until the ChannelState is ACTIVE, in
     * which case the UPAJ server will have processed the ConnectReq and sent a
     * ConnectAck. The TestClient will read and verify the ConnectAck (along
     * with component version).
     */
    @Test
    public void serverRcvSingleRipc11ConnectReqTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc11.txt";
        testServerRcvSingleConnectReq(inputFile, 11);
    }
    
    /*
     * This is a generalized version of the
     * serverRcvSingleRipcXConnectReqTest().
     * 
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a ConnectReq. initChannel() will be
     * called on the UPAJ server until the ChannelState is ACTIVE, in which case
     * the UPAJ server will have processed the ConnectReq and sent a ConnectAck.
     * The TestClient will read and verify the ConnectAck (along with component
     * version).
     * 
     * @param inputFile
     * @param ripcVersion
     */
    public void testServerRcvSingleConnectReq(String inputFile, int ripcVersion)
    {
        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            // verify that the UPAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectAck(client.buffer(), ripcVersion, Transport.queryVersion().productVersion());

            if (ripcVersion >= 13)
            {
                if (ripcVersion == 14)
                {
                    compareComponentVersionToReceivedComponentInfo(sentComponentVersion14, serverChannel, error);
                }
                else if (ripcVersion == 13)
                {
                    compareComponentVersionToReceivedComponentInfo(sentComponentVersion13, serverChannel, error);
                }
            }
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }

    /*
     * In this test, the UPAJ server will be configured with
     * AcceptOptions.nakMount(boolean) true. The UPAJ server will accept
     * a connection from the TestClient. The TestClient will send a RIPC13
     * ConnectReq. initChannel() will be called on the UPAJ server until the
     * ChannelState is INACTIVE, in which case the UPAJ server will have
     * processed the ConnectReq and sent a ConnectNak. The TestClient will read
     * and verify the ConnectNak.
     */
    @Test
    public void serverRcvSingleRipc13ConnectReqSendNakTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc13.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client
            // but specify NakMount.
            serverChannel = serverAccept(selector, true, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our UPAJ server process the ConnectReq and send a
            // ConnectNakk
            // verify that the UPAJ server's channel state goes to INACTIVE.
            initChannelWaitState(serverChannel, ChannelState.INACTIVE, error, inProg);

            // have our test client read and verify the ConnectNak.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectNak(client.buffer(), CURRENT_RIPC_VERSION);
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }

    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq with a protocol
     * type version that the UPAJ server does not supports. initChannel() will
     * be called on the UPAJ server until the ChannelState is INACTIVE, in which
     * case the UPAJ server will have processed the ConnectReq and sent a
     * ConnectNak. The TestClient will read and verify the ConnectNak.
     */
    @Test
    public void serverRcvSingleRipc13ConnectReqWithWrongProtocolTypeTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/80_input_connectReq_Ripc13_nonDefaultProtocolType.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, 60, 30, Ripc.CompressionTypes.NONE, 0, Codec.protocolType(),
                                Codec.majorVersion(), Codec.minorVersion(), false, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our UPAJ server process the ConnectReq. The protocolType will not match
            // so the UPAJ server will send a ConnectNak.
            // verify that the UPAJ server's channel state goes to INACTIVE.
            initChannelWaitState(serverChannel, ChannelState.INACTIVE, error, inProg);

            // have our test client read and verify the ConnectNak.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectNak(client.buffer(), CURRENT_RIPC_VERSION);
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }

    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq with a non protocol
     * type version that the UPAJ server supports. initChannel() will be called
     * on the UPAJ server until the ChannelState is ACTIVE, in which case the
     * UPAJ server will have processed the ConnectReq and sent a ConnectAck. The
     * TestClient will read and verify the ConnectAck.
     */
    @Test
    public void serverRcvSingleRipc13ConnectReqWithNonDefaultProtocolTypeTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/80_input_connectReq_Ripc14_nonDefaultProtocolType.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, 60, 30, Ripc.CompressionTypes.NONE, 0, 255, Codec.majorVersion(),
                                Codec.minorVersion(), false, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            // verify that the UPAJ server's channel state goes to active.
            initChannel(serverChannel, error, inProg); // receive ConnectReq
            client.writeMessageToSocket(replay.read()); // send clientKey to server
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg); // process clientKey

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectAck(client.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion());

            compareComponentVersionToReceivedComponentInfo(sentComponentVersion14, serverChannel, error);
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }

    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq with a non
     * default major and minor version that the UPAJ server supports.
     * initChannel() will be called on the UPAJ server until the ChannelState is
     * ACTIVE, in which case the UPAJ server will have processed the ConnectReq
     * and sent a ConnectAck. The TestClient will read and verify the
     * ConnectAck.
     */
    @Test
    public void serverRcvSingleRipc13ConnectReqWithNonDefaultMajorMinorVersionTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/90_input_connectReq_Ripc14_nonDefaultMajorMinorVersion.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, 60, 30, CompressionTypes.NONE, 0, Codec.protocolType(), 255, 255,
                                false, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());
            
            // have our UPAJ server process the ConnectReq and send a ConnectAck
            // verify that the UPAJ server's channel state goes to active.
            initChannel(serverChannel, error, inProg);
            client.writeMessageToSocket(replay.read()); // send clientKey to server
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg); // process clientKey

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectAck(client.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion(),
                                      0x3C, 255, 255, CompressionTypes.NONE, 0);

            compareComponentVersionToReceivedComponentInfo(sentComponentVersion14, serverChannel, error);
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }
    
    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq with ZLIB
     * compression requested. initChannel() will be called on the UPAJ server
     * until the ChannelState is ACTIVE, in which case the UPAJ server will have
     * processed the ConnectReq and sent a ConnectAck. The TestClient will read
     * and verify the ConnectAck (along with compression type and level).
     */
    @Test
    public void serverRcvSingleRipc13ConnectReqWithCompressionTest()
    {
        String inputFile = BASE_TEST_DATA_DIR_NAME + "/100_input_connectReq_Ripc13_Zlib_compression.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, 60, 30, Ripc.CompressionTypes.ZLIB, 6, Codec.protocolType(),
                                Codec.majorVersion(), Codec.minorVersion(), false, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            // verify that the UPAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectAck(client.buffer(), RIPC_VERSION_13, Transport.queryVersion().productVersion(),
                                      60, Codec.majorVersion(), Codec.minorVersion(), Ripc.CompressionTypes.ZLIB, 6);

            compareComponentVersionToReceivedComponentInfo(sentComponentVersion13, serverChannel, error);
        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }
    
    /*
     * In this test, the UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq with each byte
     * in it's own packet. initChannel() will be called on the UPAJ server until
     * the ChannelState is ACTIVE, in which case the UPAJ server will have
     * processed the ConnectReq and sent a ConnectAck. The TestClient will read
     * and verify the ConnectAck (along with component version).
     */
    @Test
    public void serverRcvAllSplitConnectReqTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/70_input_connectReq_Ripc14_all_split.txt";
        final String inputFile2 = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc14.txt";
        
        NetworkReplay replay = null;
        NetworkReplay replay2 = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);
            replay2 = parseReplayFile(inputFile2);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);
            int channelState = ChannelState.INACTIVE;
            
            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());


            /* now write the split ConnectReq to server in pieces. */
            int retry = 70; // 62 bytes in file.
            while (retry-- > 0)
            {
                byte[] bytesToWrite = replay.read();
                int bytesWritten = 0;
                if (bytesToWrite != null)
                    bytesWritten = client.writeMessageToSocket(bytesToWrite);

                // initChannel should read connectReq.
                // wait for the server's channel to become active.
                channelState = initChannel(serverChannel, error, inProg);
                if (DEBUG)
                {
                    System.out.println("DEBUG: retry=" + retry + " bytesWritten=" + bytesWritten
                            + " channelState=" + ChannelState.toString(channelState)
                            + ", error=" + error.text());
                }
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            
            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }
            compareExpectedConnectAck(client.buffer(), CURRENT_RIPC_VERSION, Transport.queryVersion().productVersion());

            // have our UPAJ client send the clientKey
            replay2.read();
            client.writeMessageToSocket(replay2.read());
            
            // have our UPAJ server process the clientKey and
            // verify that the UPAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);            
            assertNull(error.text());
            
            compareComponentVersionToReceivedComponentInfo(sentComponentVersion14, serverChannel, error);
        }
        catch (IOException | InterruptedException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }
    
    /*
     * In this test, after binding the UPAJ server socket, the user specifies a
     * component version. The UPAJ server will accept a connection from the
     * TestClient. The TestClient will send a RIPC13 ConnectReq. initChannel()
     * will be called on the UPAJ server until the ChannelState is ACTIVE, in
     * which case the UPAJ server will have processed the ConnectReq and sent a
     * ConnectAck with the user specified component version. The TestClient will
     * read and verify the ConnectAck (along with component version).
     */
    @Test
    public void serverUserSpecifiedComponentVersion_serverRcvSingleConnectReqTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc14.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);

            // specify ComponentVersion
            ByteBuffer componentVersionBB = ByteBuffer.wrap(userSpecifiedComponentVersion
                    .getBytes());
            ComponentInfo myCi = TransportFactory.createComponentInfo();
            myCi.componentVersion().data(componentVersionBB);
            server.ioctl(IoctlCodes.COMPONENT_INFO, myCi, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            // verify that the UPAJ server's channel state goes to active.
            initChannel(serverChannel, error, inProg); // receive ConnectReq
            client.writeMessageToSocket(replay.read()); // send clientKey to server
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg); // process clientKey

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectAck(client.buffer(), CURRENT_RIPC_VERSION, userSpecifiedComponentVersion);

            compareComponentVersionToReceivedComponentInfo(sentComponentVersion14, serverChannel, error);

        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }

    /*
     * In this test, the UAPJ server will bind the server socket. The UPAJ
     * server will accept a connection from the TestClient. The TestClient will
     * send a RIPC13 ConnectReq. Before initChannel() is called, the user
     * specifies a component version. initChannel() will be called on the UPAJ
     * server until the ChannelState is ACTIVE, in which case the UPAJ server
     * will have processed the ConnectReq and sent a ConnectAck with the user
     * specified component version. The TestClient will read and verify the
     * ConnectAck (along with component version).
     */
    @Test
    public void serverUserSpecifiedComponentVersionAfterAccept_serverRcvSingleConnectReqTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/60_input_connectReq_Ripc14.txt";

        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        TestClient client = new TestClient(DEFAULT_LISTEN_PORT);

        try
        {
            replay = parseReplayFile(inputFile);

            initTransport(false); // initialize RSSL

            // set the bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);

            // have our test client connect to UPAJ server
            client.connect();

            // have the UPAJ server accept the connection from our test client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // have our test client send a ConnectReq to the server
            client.writeMessageToSocket(replay.read());

            // specify ComponentVersion
            ByteBuffer componentVersionBB = ByteBuffer.wrap(userSpecifiedComponentVersion
                    .getBytes());
            ComponentInfo myCi = TransportFactory.createComponentInfo();
            myCi.componentVersion().data(componentVersionBB);
            serverChannel.ioctl(IoctlCodes.COMPONENT_INFO, myCi, error);
            
            // have our UPAJ server process the ConnectReq and send a ConnectAck
            // verify that the UPAJ server's channel state goes to active.
            initChannel(serverChannel, error, inProg); // receive ConnectReq
            client.writeMessageToSocket(replay.read()); // send clientKey to server
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg); // process clientKey

            // have our test client read and verify the ConnectAck.
            client.waitForReadable();
            client.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG: buffer pos=" + client.buffer().position() + " hex:"
                        + Transport.toHexString(client.buffer(), 0, client.buffer().position()));
            }

            compareExpectedConnectAck(client.buffer(), CURRENT_RIPC_VERSION, userSpecifiedComponentVersion);

            compareComponentVersionToReceivedComponentInfo(sentComponentVersion14, serverChannel, error);

        }
        catch (IOException e)
        {
            fail("exception occurred during test, exception=" + e.toString());
        }
        finally
        {
            cleanupForTestsWithTestClient(replay, server, serverChannel, client, selector, error);
        }
    }

    /*
     * This test will set up a UPAJ server with no compression. A UPAJ client
     * will be set up and connected to the UPAJ server. The UPAJ server will
     * accept the connection. initChannel() will be called on the UPAJ client to
     * send the ConnectReq. initChannel() will be called on the UPAJ server to
     * send a ConnectAck. The received component versiOn from the ConnectReq and
     * ConnectAck will be verified.
     */
    @Test
    public void defaultComponentInfoTest()
    {
        testDefaultComponentInfo(Ripc.CompressionTypes.NONE);
        testDefaultPingTimeout();
    }
    
    /*
     * This test will set up a UPAJ server with ZLIB compression. A UPAJ client
     * will be set up and connected to the UPAJ server. The UPAJ server will
     * accept the connection. initChannel() will be called on the UPAJ client to
     * send the ConnectReq (with ZLIB compression). initChannel() will be called
     * on the UPAJ server to send a ConnectAck (with ZLIB compression). The
     * received component version from the ConnectReq and ConnectAck will be
     * verified.
     */
    @Test
    public void defaultComponentInfoWithCompressionTest()
    {
        testDefaultComponentInfo(Ripc.CompressionTypes.ZLIB);
    }
    
    /*
     * This is a generalized version of the defaultComponentInfoXTest().
     * 
     * This test will set up a UPAJ server with the specified compression type.
     * A UPAJ client will be set up and connected to the UPAJ server. The UPAJ
     * server will accept the connection. initChannel() will be called on the
     * UPAJ client to send the ConnectReq. initChannel() will be called on the
     * UPAJ server to send a ConnectAck. The received component version from the
     * ConnectReq and ConnectAck will be verified.
     * 
     * @param compressionType
     */
    @SuppressWarnings("deprecation")
	private void testDefaultComponentInfo(int compressionType)
    {
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();

        /* consumer */
        Channel consumerChannel = null;

        /* server */
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        try
        {
            initTransport(false); // initialize RSSL

            // bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, 0, 30, compressionType, 6, Codec.protocolType(),
                                Codec.majorVersion(), Codec.minorVersion(), false, error);

            // UPAJ client to connect to UPAJ server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT),
                                                       Codec.protocolType(), compressionType, error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            // have the UPAJ server accept the connection from the UPAJ client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // initChannel should send connectReq to our server.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            channelState = initChannel(serverChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            // have our UPAJ client process the ConnectAck, send the clientKey and
            // verify that the UPAJ client's channel state goes to active.
            initChannelWaitState(consumerChannel, ChannelState.ACTIVE, error, inProg);
            
            // have our UPAJ server process the clientKey and
            // verify that the UPAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);            
            
            int rcvdBufSize = ((RsslSocketChannel)consumerChannel).scktChannel().socket().getReceiveBufferSize();
            int sendBufSize = ((RsslSocketChannel)consumerChannel).scktChannel().socket().getSendBufferSize();

            String defaultComponentVersion = Transport.queryVersion().productVersion();
            compareComponentVersionToReceivedComponentInfo(defaultComponentVersion, serverChannel, error);
            compareComponentVersionToReceivedComponentInfo(defaultComponentVersion, consumerChannel, error);
            
            /* verify server is honoring client's requested compression */
            ChannelInfo info = TransportFactory.createChannelInfo();
            int ret = consumerChannel.info(info, error);
            assertEquals("channel.info() failed, error=" + error.text(), TransportReturnCodes.SUCCESS,
                         ret);
            assertEquals(rcvdBufSize, info.sysRecvBufSize());
            assertEquals(sendBufSize, info.sysSendBufSize());
            assertEquals(1, info.pingTimeout());
            assertEquals(compressionType, info.compressionType());
        }
        catch (IOException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanup(server, serverChannel, consumerChannel, selector, error);
        }
    }

    /*
     * This test will set up a UPAJ server with the pingTimeout greater than 255.
     * A UPAJ client will be set up and connected to the UPAJ server. The UPAJ
     * server will accept the connection. initChannel() will be called on the
     * UPAJ client to send the ConnectReq. initChannel() will be called on the
     * UPAJ server to send a ConnectAck. The pingTimeout will be verified to be
     * set to 255.
     * 
     * @param compressionType
     */
    private void testDefaultPingTimeout()
    {
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();

        /* consumer */
        Channel consumerChannel = null;

        /* server */
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        try
        {
            initTransport(false); // initialize RSSL

            // bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, 3000, 30, 0, 6, Codec.protocolType(),
                                Codec.majorVersion(), Codec.minorVersion(), false, error);

            assertEquals(255, ((ServerImpl)server)._bindOpts.pingTimeout());
            // UPAJ client to connect to UPAJ server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT),
                                                       Codec.protocolType(), 0, error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            // have the UPAJ server accept the connection from the UPAJ client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // initChannel should send connectReq to our server.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            channelState = initChannel(serverChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            // have our UPAJ client process the ConnectAck, send the clientKey and
            // verify that the UPAJ client's channel state goes to active.
            initChannelWaitState(consumerChannel, ChannelState.ACTIVE, error, inProg);
            
            // have our UPAJ server process the clientKey and
            // verify that the UPAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);            
            
            @SuppressWarnings("deprecation")
			int rcvdBufSize = ((RsslSocketChannel)consumerChannel).scktChannel().socket().getReceiveBufferSize();
            @SuppressWarnings("deprecation")
			int sendBufSize = ((RsslSocketChannel)consumerChannel).scktChannel().socket().getSendBufferSize();

            String defaultComponentVersion = Transport.queryVersion().productVersion();
            compareComponentVersionToReceivedComponentInfo(defaultComponentVersion, serverChannel, error);
            compareComponentVersionToReceivedComponentInfo(defaultComponentVersion, consumerChannel, error);
            
            /* verify server is honoring client's requested compression */
            ChannelInfo info = TransportFactory.createChannelInfo();
            int ret = consumerChannel.info(info, error);
            assertEquals("channel.info() failed, error=" + error.text(), TransportReturnCodes.SUCCESS,
                         ret);
            assertEquals(rcvdBufSize, info.sysRecvBufSize());
            assertEquals(sendBufSize, info.sysSendBufSize());
            assertEquals(0, info.compressionType());
        }
        catch (IOException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanup(server, serverChannel, consumerChannel, selector, error);
        }
    }

    /*
     * This test will set up a UPAJ server with a user specified component
     * version. A UPAJ client will be set up with a user specified component
     * version, then connect to the UPAJ server. The UPAJ server will accept the
     * connection. initChannel() will be called on the UPAJ client to send the
     * ConnectReq. initChannel() will be called on the UPAJ server to send a
     * ConnectAck. The received component version from the ConnectReq and
     * ConnectAck will be verified.
     */
    @Test
    public void userSpecifiedComponentInfoTest()
    {
        String clientComponentVersion = "Client connected component version information v2012";
        String serverComponentVersion = "Server Connected Component version information star date 66225.4";

        testUserSpecifiedComponentInfo(clientComponentVersion, clientComponentVersion,
                                   serverComponentVersion, serverComponentVersion);
    }

    /*
     * This test will set up a UPAJ server with a user specified component
     * version 127 in length. A UPAJ client will be set up with a user specified
     * component version 127 in length, then connect to the UPAJ server. The
     * UPAJ server will accept the connection. initChannel() will be called on
     * the UPAJ client to send the ConnectReq. initChannel() will be called on
     * the UPAJ server to send a ConnectAck. The received component version from
     * the ConnectReq and ConnectAck will be verified.
     */
    @Test
    public void userSpecifiedComponentInfo127Test()
    {
        userSpecifiedComponentInfoSizeTest(127);
    }

    /*
     * This test will set up a UPAJ server with a user specified component
     * version 128 in length. A UPAJ client will be set up with a user specified
     * component version 128 in length, then connect to the UPAJ server. The UPAJ
     * server will accept the connection. initChannel() will be called on the
     * UPAJ client to send the ConnectReq. initChannel() will be called on the
     * UPAJ server to send a ConnectAck. The received component version from the
     * ConnectReq and ConnectAck will be verified.
     */
    @Test
    public void userSpecifiedComponentInfo128Test()
    {
        userSpecifiedComponentInfoSizeTest(128);
    }

    /*
     * This test will set up a UPAJ server with a user specified component
     * version 255 in length. A UPAJ client will be set up with a user specified
     * component version 255 in length, then connect to the UPAJ server. The UPAJ
     * server will accept the connection. initChannel() will be called on the
     * UPAJ client to send the ConnectReq. initChannel() will be called on the
     * UPAJ server to send a ConnectAck. The received component version from the
     * ConnectReq and ConnectAck will be verified.
     */
    @Test
    public void userSpecifiedComponentInfo255Test()
    {
        userSpecifiedComponentInfoSizeTest(255);
    }

    /*
     * This test will set up a UPAJ server with a user specified component
     * version 256 in length. A UPAJ client will be set up with a user specified
     * component version 256 in length, then connect to the UPAJ server. The UPAJ
     * server will accept the connection. initChannel() will be called on the
     * UPAJ client to send the ConnectReq. initChannel() will be called on the
     * UPAJ server to send a ConnectAck. The received component version from the
     * ConnectReq and ConnectAck will be verified.
     */
    @Test
    public void userSpecifiedComponentInfo256Test()
    {
        userSpecifiedComponentInfoSizeTest(256);
    }

    /*
     * This test will set up a UPAJ server with a user specified component
     * version 2K in length. A UPAJ client will be set up with a user specified
     * component version 2K in length, then connect to the UPAJ server. The UPAJ
     * server will accept the connection. initChannel() will be called on the
     * UPAJ client to send the ConnectReq. initChannel() will be called on the
     * UPAJ server to send a ConnectAck. The received component version from the
     * ConnectReq and ConnectAck will be verified.
     */
    @Test
    public void userSpecifiedComponentInfo2KTest()
    {
        userSpecifiedComponentInfoSizeTest(2048);
    }
    
    /*
     * Helper method to generate a string of characters of the specified size.
     * 
     * @param charIndex specifies the character to start the string with.
     * @param size of the string
     * @return String of the specified size, starting with the specified
     *         character.
     */
    private String generateString(int charIndex, int size)
    {
        _sb.setLength(0);

        if (charIndex < 32)
            charIndex = 32;
        else if (charIndex > 122)
            charIndex = 122;

        for (int i = 0; i < size; i++)
        {
            _sb.append((char)charIndex++);

            if (charIndex > 122)
                charIndex = 32;
        }

        return _sb.toString();
    }

    /*
     * This is a generalized version of the userSpecifiedComponentInfoXTest()
     * that accepts a size, which is used to specify the size of the user
     * specified component version. It will create client and server component
     * versions as well as client and server expected component versions that
     * are trimmed to the MAX_EXPECTED_COMPONENT_VERSION_SIZE.
     * testUserSpecifiedComponentInfo() will then be called with the
     * clientSentComponentVersion, clientExpectedComponentVersion,
     * serverSentComponentVersion and serverExpectedComponentVersion.
     * 
     * @param size
     */
    private void userSpecifiedComponentInfoSizeTest(int size)
    {
        int expectedSize = size;
        if (expectedSize > MAX_EXPECTED_COMPONENT_VERSION_SIZE)
            expectedSize = MAX_EXPECTED_COMPONENT_VERSION_SIZE;

        String clientSentComponentVersion = generateString(65, size);
        String clientExpectedComponentVersion = generateString(65, expectedSize);
        String serverSentComponentVersion = generateString(97, size);
        String serverExpectedComponentVersion = generateString(97, expectedSize);

        testUserSpecifiedComponentInfo(clientSentComponentVersion, clientExpectedComponentVersion,
                                   serverSentComponentVersion, serverExpectedComponentVersion);
    }

    /*
     * This is a generalized version of the userSpecifiedComponentInfoXTest().
     * 
     * This test will set up a UPAJ server with the user specified component
     * version. A UPAJ client will be set up with a user specified component
     * version, then connect to the UPAJ server. The UPAJ server will accept the
     * connection. initChannel() will be called on the UPAJ client to send the
     * ConnectReq. initChannel() will be called on the UPAJ server to send a
     * ConnectAck. The received component version from the ConnectReq and
     * ConnectAck will be verified.
     * 
     * @param clientSentComponentVersion
     * @param clientExpectedComponentVersion
     * @param serverSentComponentVersion
     * @param serverExpectedComponentVersion
     */
    private void testUserSpecifiedComponentInfo(String clientSentComponentVersion,
            String clientExpectedComponentVersion, String serverSentComponentVersion,
            String serverExpectedComponentVersion)
    {
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();

        /* consumer */
        Channel consumerChannel = null;

        /* server */
        Server server = null;
        Channel serverChannel = null;
        Selector selector = null;

        try
        {
            initTransport(false); // initialize RSSL

            // bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);

            // specify server ComponentVersion
            ByteBuffer serverComponentVersionBB = ByteBuffer.wrap(serverSentComponentVersion
                    .getBytes());
            ComponentInfo myServerCi = TransportFactory.createComponentInfo();
            myServerCi.componentVersion().data(serverComponentVersionBB);
            int ret = server.ioctl(IoctlCodes.COMPONENT_INFO, myServerCi, error);
            assertEquals(TransportReturnCodes.SUCCESS, ret);

            // UPAJ client to connect to UPAJ server.
            if ((consumerChannel = connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            // specify client ComponentVersion
            ByteBuffer clientComponentVersionBB = ByteBuffer.wrap(clientSentComponentVersion
                    .getBytes());
            ComponentInfo myClientCi = TransportFactory.createComponentInfo();
            myClientCi.componentVersion().data(clientComponentVersionBB);
            ret = consumerChannel.ioctl(IoctlCodes.COMPONENT_INFO, myClientCi, error);
            assertEquals(TransportReturnCodes.SUCCESS, ret);

            // have the UPAJ server accept the connection from the UPAJ client.
            serverChannel = serverAccept(selector, false, error);
            assertNotNull(serverChannel.selectableChannel());

            // initChannel should send connectReq to our server.
            int channelState = initChannel(consumerChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            // have our UPAJ server process the ConnectReq and send a ConnectAck
            channelState = initChannel(serverChannel, error, inProg);
            assertEquals(ChannelState.INITIALIZING, channelState);

            // have our UPAJ client process the ConnectAck, send the clientKey and
            // verify that the UPAJ client's channel state goes to active.
            initChannelWaitState(consumerChannel, ChannelState.ACTIVE, error, inProg);
            
            // have our UPAJ server process the clientKey and
            // verify that the UPAJ server's channel state goes to active.
            initChannelWaitState(serverChannel, ChannelState.ACTIVE, error, inProg);            

            // verify that the server and consumer received the correct
            // component version
            compareComponentVersionToReceivedComponentInfo(clientExpectedComponentVersion, serverChannel, error);
            compareComponentVersionToReceivedComponentInfo(serverExpectedComponentVersion, consumerChannel, error);
        }
        catch (IOException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            cleanup(server, serverChannel, consumerChannel, selector, error);
        }
    }

    /*
     * This test will verify Server.ioctl(int, Object, Error) and
     * Channel.ioctl(int, Object, Error).
     */
    @Test
    public void componentInfoIoctlTest()
    {

        ComponentInfo componentInfo = TransportFactory.createComponentInfo();
        ByteBuffer componentVersionBB = ByteBuffer.wrap("This is a test".getBytes());
        componentInfo.componentVersion().data(componentVersionBB);

        Channel clientChannel = null;
        Server server = null;
        Selector selector = null;
        Error error = TransportFactory.createError();

        try
        {
            initTransport(false); // initialize RSSL

            // bind the UPAJ server
            selector = Selector.open();
            server = serverBind(selector, error);

            // attempt to set a componentInfo using server.ioctl. expect
            // success.
            assertEquals(TransportReturnCodes.SUCCESS,
                         server.ioctl(IoctlCodes.COMPONENT_INFO, componentInfo, error));

            // attempt to set a ByteBuffer using server.ioctl. expect failure
            assertEquals(TransportReturnCodes.FAILURE,
                         server.ioctl(IoctlCodes.COMPONENT_INFO, componentVersionBB, error));

            // attempt to set a Buffer using server.ioctl. expect failure
            assertEquals(TransportReturnCodes.FAILURE, server.ioctl(IoctlCodes.COMPONENT_INFO,
                                                                    componentInfo
                                                                            .componentVersion(),
                                                                    error));

            // UPAJ client to connect to UPAJ server.
            if ((clientChannel = connectToRsslServer("localhost",
                                                     String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            // attempt to set a componentInfo using clientChannel.ioctl. expect
            // success.
            assertEquals(TransportReturnCodes.SUCCESS,
                         clientChannel.ioctl(IoctlCodes.COMPONENT_INFO, componentInfo, error));

            // attempt to set a ByteBuffer using clientChannel.ioctl. expect
            // failure
            assertEquals(TransportReturnCodes.FAILURE,
                         clientChannel.ioctl(IoctlCodes.COMPONENT_INFO, componentVersionBB, error));

            // attempt to set a Buffer using clientChannel.ioctl. expect failure
            assertEquals(TransportReturnCodes.FAILURE,
                         clientChannel.ioctl(IoctlCodes.COMPONENT_INFO,
                                             componentInfo.componentVersion(), error));
        }
        catch (IOException e)
        {
            fail("Exception caught, exception=" + e.toString());
        }
        finally
        {
            cleanup(server, null, clientChannel, selector, error);
        }
    }
}
