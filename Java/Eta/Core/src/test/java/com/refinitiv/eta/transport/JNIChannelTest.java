package com.refinitiv.eta.transport;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeTrue;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.concurrent.CyclicBarrier;

import com.refinitiv.eta.codec.Codec;
import org.junit.AssumptionViolatedException;
import org.junit.Before;
import org.junit.Test;

/**
 * JNI Channel Unit Tests
 *
 * These tests exercise ETA JNI (C-based transport) channel functionality via the
 * RELIABLE_MCAST connection type. Two prerequisites must be met:
 *
 * 1. JNI Libraries: rsslEtaJNI.dll (Windows) / librsslEtaJNI.so (Linux) must be
 *    present in Eta/Libs. Tests are skipped with an informative message if missing.
 *
 * 2. Multicast Network: A multicast-capable network interface must be available on
 *    the test host. Tests are skipped gracefully if multicast connects fail.
 *
 * Tests cover:
 * - Unified multicast channel operations
 * - Segmented multicast channel operations
 * - Buffer packing and fragmentation
 * - Blocking I/O operations
 */
public class JNIChannelTest
{
    // Barrier synchronization for multi-threaded blocking test
    private final CyclicBarrier startBarrier = new CyclicBarrier(2);
    private final CyclicBarrier connectBarrier = new CyclicBarrier(2);
    private final CyclicBarrier readWriteBarrier = new CyclicBarrier(2);

    /**
     * Verifies that the primary ETA JNI library (rsslEtaJNI) is present in Eta/Libs
     * and loads the ETA native DLL set directly by absolute path so the test JVM does
     * not depend on java.library.path ordering.
     */
    @Before
    public void checkJNILibrariesAvailable()
    {
        Path etaLibsDir = resolveEtaLibsDir();
        String[] requiredLibraries = getExpectedJNILibraryFileNames();

        assumeTrue("Expected ETA native libraries not found. They should exist in Eta/Libs "
                        + "or be supplied via -Deta.jni.libs.dir.",
                etaLibsDir != null && Files.isDirectory(etaLibsDir));

        for (String libraryName : requiredLibraries)
        {
            Path libraryPath = etaLibsDir.resolve(libraryName);
            assumeTrue("Expected ETA native library not found: " + libraryPath.toAbsolutePath(),
                    Files.isRegularFile(libraryPath));

            try
            {
                System.load(libraryPath.toAbsolutePath().toString());
            }
            catch (UnsatisfiedLinkError e)
            {
                String msg = e.getMessage() == null ? "" : e.getMessage().toLowerCase();
                if (!msg.contains("already loaded"))
                {
                    assumeTrue("ETA native library found at " + libraryPath.toAbsolutePath()
                            + " but could not be loaded: " + e.getMessage()
                            + ". Ensure the library matches your JVM architecture (64-bit).", false);
                }
            }
        }
    }

    /**
     * Resolves the directory that should contain ETA native libraries.
     *
     * Search order:
     * 1) -Deta.jni.libs.dir (if explicitly provided)
     * 2) Common relative paths from the current working directory
     * 3) Walk up parent directories and look for Eta/Libs
     */
    private Path resolveEtaLibsDir()
    {
        String explicitDir = System.getProperty("eta.jni.libs.dir");
        if (explicitDir != null && !explicitDir.trim().isEmpty())
        {
            Path candidate = Paths.get(explicitDir.trim()).toAbsolutePath().normalize();
            if (Files.isDirectory(candidate))
            {
                return candidate;
            }
        }

        Path[] relativeCandidates = new Path[]
        {
            Paths.get("../Libs"),
            Paths.get("Eta/Libs"),
            Paths.get("./Eta/Libs")
        };
        for (Path candidate : relativeCandidates)
        {
            try
            {
                Path normalized = candidate.toAbsolutePath().normalize();
                if (Files.isDirectory(normalized))
                {
                    return normalized;
                }
            }
            catch (Exception ignored)
            {
                // keep searching
            }
        }

        Path current = Paths.get(System.getProperty("user.dir")).toAbsolutePath().normalize();
        while (current != null)
        {
            Path candidate = current.resolve("Eta").resolve("Libs");
            if (Files.isDirectory(candidate))
            {
                return candidate;
            }
            current = current.getParent();
        }

        return null;
    }

    /** Returns the ETA native library file names that should be preloaded. */
    private static String[] getExpectedJNILibraryFileNames()
    {
        String os = System.getProperty("os.name").toLowerCase();
        if (os.contains("windows"))
        {
            return new String[] { "librsslRelMcast.dll", "rsslVACacheJNI.dll", "rsslEtaJNI.dll" };
        }
        return new String[] { "librsslRelMcast.so", "rsslVACacheJNI.so", "librsslEtaJNI.so" };
    }

    /**
     * Calls {@link Transport#connect} and fails fast with a descriptive message if
     * the connection cannot be established. This keeps the tests honest: if the JNI
     * libraries are present, the multicast setup must also be valid for the test to run.
     *
     * @param opts  connect options (already configured)
     * @param error error object populated on failure
     * @return a non-null JNIChannel ready for use
     */
    private JNIChannel connectRequired(ConnectOptions opts, Error error)
    {
        JNIChannel ch = (JNIChannel) Transport.connect(opts, error);
        assertNotNull("Transport.connect() returned null. Multicast setup may be unavailable on this host: "
                        + error.text()
                        + ". These tests require a multicast-capable network interface "
                        + "(for example a loopback multicast route or a dedicated multicast NIC).",
                ch);
        return ch;
    }

    /**
     * Initializes the ETA Transport layer.
     *
     * @param globalLocking true to enable global locking for thread safety
     * @throws IOException if Transport.initialize() fails
     */
    private void initTransport(boolean globalLocking) throws IOException
    {
        final Error error = TransportFactory.createError();
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(globalLocking);
        if (Transport.initialize(initArgs, error) != TransportReturnCodes.SUCCESS)
            throw new IOException("Transport.initialize() failed: " + error.text());
    }

    /**
     * Builds ConnectOptions for a unified (single address/port) multicast channel.
     * Unified mode uses one multicast address with separate unicast ports per endpoint.
     */
    private ConnectOptions getUnifiedMulticastConnectOptions(String host, String port, String unicastPort)
    {
        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.userSpecObject("TEST UNIFIED CHANNEL");
        opts.connectionType(ConnectionTypes.RELIABLE_MCAST);
        opts.unifiedNetworkInfo().address(host);
        opts.unifiedNetworkInfo().serviceName(port);
        opts.unifiedNetworkInfo().unicastServiceName(unicastPort);
        opts.majorVersion(Codec.majorVersion());
        opts.minorVersion(Codec.minorVersion());
        opts.protocolType(Codec.protocolType());
        return opts;
    }

    /**
     * Builds ConnectOptions for a segmented (separate send/receive) multicast channel.
     * Segmented mode supports asymmetric send and receive multicast groups.
     */
    private ConnectOptions getSegmentedMulticastConnectOptions(
            String sendAddr, String sendPort, String recvAddr, String recvPort, String unicastPort)
    {
        ConnectOptions opts = TransportFactory.createConnectOptions();
        opts.userSpecObject("TEST SEGMENTED CHANNEL");
        opts.connectionType(ConnectionTypes.RELIABLE_MCAST);
        opts.segmentedNetworkInfo().sendAddress(sendAddr);
        opts.segmentedNetworkInfo().sendServiceName(sendPort);
        opts.segmentedNetworkInfo().recvAddress(recvAddr);
        opts.segmentedNetworkInfo().recvServiceName(recvPort);
        opts.segmentedNetworkInfo().unicastServiceName(unicastPort);
        opts.majorVersion(Codec.majorVersion());
        opts.minorVersion(Codec.minorVersion());
        opts.protocolType(Codec.protocolType());
        return opts;
    }

    // -------------------------------------------------------------------------
    // Tests
    // -------------------------------------------------------------------------

    /**
     * Test: Unified Multicast Channel Operations
     *
     * Tests basic unified multicast channel functionality:
     * - Channel connection and initialization
     * - Channel info retrieval
     * - Buffer allocation and release
     * - Basic write/read operations
     * - Buffer copy operations
     * - Ping, flush, and reconnect operations
     * - Error handling with ioctl
     * - Channel closure
     */
    @Test
    public void unifiedMulticastTest()
    {
        Error error = TransportFactory.createError();
        ConnectOptions ops1 = getUnifiedMulticastConnectOptions("235.5.5.5", "15002", "15005");
        ConnectOptions ops2 = getUnifiedMulticastConnectOptions("235.5.5.5", "15002", "15006");
        InProgInfo inProg = TransportFactory.createInProgInfo();
        TransportBuffer writeBuffer, readBuffer = null;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ReadArgs readArgs = TransportFactory.createReadArgs();

        try
        {
            initTransport(false);
            JNIChannel jniChannel1 = connectRequired(ops1, error);
            JNIChannel jniChannel2 = connectRequired(ops2, error);

            assertEquals(ChannelState.INITIALIZING, jniChannel1.state());
            assertEquals(ChannelState.INITIALIZING, jniChannel2.state());

            while (true)
            {
                jniChannel1.init(inProg, error);
                jniChannel2.init(inProg, error);
                if (jniChannel1.state() == ChannelState.ACTIVE &&
                        jniChannel2.state() == ChannelState.ACTIVE)
                    break;
            }

            assertEquals(ChannelState.ACTIVE, jniChannel1.state());
            assertEquals(ChannelState.ACTIVE, jniChannel2.state());

            assertEquals("TEST UNIFIED CHANNEL", jniChannel1.userSpecObject());
            assertEquals("TEST UNIFIED CHANNEL", jniChannel2.userSpecObject());

            // test Channel.info()
            ChannelInfo info = TransportFactory.createChannelInfo();
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.info(info, error));
            info.clear();
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.info(info, error));

            // test Channel.bufferUsage()
            assertEquals(4, jniChannel1.bufferUsage(error));
            assertEquals(4, jniChannel2.bufferUsage(error));

            writeBuffer = jniChannel1.getBuffer(100, false, error);
            assertNotNull(writeBuffer);
            // test releaseBuffer()
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.releaseBuffer(writeBuffer, error));
            writeBuffer = jniChannel1.getBuffer(100, false, error);
            assertNotNull(writeBuffer);
            writeBuffer.data().put("this is a test...".getBytes());
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.write(writeBuffer, writeArgs, error));

            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('t', readBuffer.data().get(0));
            assertEquals('h', readBuffer.data().get(1));
            assertEquals('i', readBuffer.data().get(2));
            assertEquals('s', readBuffer.data().get(3));
            assertEquals(' ', readBuffer.data().get(4));
            assertEquals('i', readBuffer.data().get(5));
            assertEquals('s', readBuffer.data().get(6));
            assertEquals(' ', readBuffer.data().get(7));
            assertEquals('a', readBuffer.data().get(8));
            assertEquals(' ', readBuffer.data().get(9));
            assertEquals('t', readBuffer.data().get(10));
            assertEquals('e', readBuffer.data().get(11));
            assertEquals('s', readBuffer.data().get(12));
            assertEquals('t', readBuffer.data().get(13));
            assertEquals('.', readBuffer.data().get(14));
            assertEquals('.', readBuffer.data().get(15));
            assertEquals('.', readBuffer.data().get(16));

            // test buffer copy
            ByteBuffer copyBuf = ByteBuffer.allocate(100);
            readBuffer.copy(copyBuf);
            assertEquals('t', copyBuf.get(0));
            assertEquals('h', copyBuf.get(1));
            assertEquals('i', copyBuf.get(2));
            assertEquals('s', copyBuf.get(3));
            assertEquals(' ', copyBuf.get(4));
            assertEquals('i', copyBuf.get(5));
            assertEquals('s', copyBuf.get(6));
            assertEquals(' ', copyBuf.get(7));
            assertEquals('a', copyBuf.get(8));
            assertEquals(' ', copyBuf.get(9));
            assertEquals('t', copyBuf.get(10));
            assertEquals('e', copyBuf.get(11));
            assertEquals('s', copyBuf.get(12));
            assertEquals('t', copyBuf.get(13));
            assertEquals('.', copyBuf.get(14));
            assertEquals('.', copyBuf.get(15));
            assertEquals('.', copyBuf.get(16));

            // test ping and flush
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.ping(error));
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.flush(error));

            // test reconnectClient()
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.reconnectClient(error));

            // test ioctl() – code 1000 is intentionally invalid
            assertEquals(TransportReturnCodes.FAILURE, jniChannel1.ioctl(1000, 0, error));

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.close(error));
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.close(error));
        }
        catch (AssumptionViolatedException e)
        {
            throw e; // propagate so JUnit records the test as skipped, not failed
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

    /**
     * Test: Segmented Multicast Channel Operations
     *
     * Tests segmented (asymmetric send/receive) multicast channel functionality:
     * - Channel connection with separate send and receive addresses
     * - Channel initialization with segmented network configuration
     * - Write/read operations with separate multicast groups
     * - Channel closure
     */
    @Test
    public void segmentedMulticastTest()
    {
        Error error = TransportFactory.createError();
        ConnectOptions ops1 = getSegmentedMulticastConnectOptions("235.5.5.6", "15003", "235.5.5.6", "15003", "15007");
        ConnectOptions ops2 = getSegmentedMulticastConnectOptions("235.5.5.6", "15003", "235.5.5.6", "15003", "15008");
        InProgInfo inProg = TransportFactory.createInProgInfo();
        TransportBuffer writeBuffer, readBuffer = null;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ReadArgs readArgs = TransportFactory.createReadArgs();

        try
        {
            initTransport(false);
            JNIChannel jniChannel1 = connectRequired(ops1, error);
            JNIChannel jniChannel2 = connectRequired(ops2, error);

            assertEquals(ChannelState.INITIALIZING, jniChannel1.state());
            assertEquals(ChannelState.INITIALIZING, jniChannel2.state());

            while (true)
            {
                jniChannel1.init(inProg, error);
                jniChannel2.init(inProg, error);
                if (jniChannel1.state() == ChannelState.ACTIVE &&
                        jniChannel2.state() == ChannelState.ACTIVE)
                    break;
            }

            assertEquals(ChannelState.ACTIVE, jniChannel1.state());
            assertEquals(ChannelState.ACTIVE, jniChannel2.state());

            assertEquals("TEST SEGMENTED CHANNEL", jniChannel1.userSpecObject());
            assertEquals("TEST SEGMENTED CHANNEL", jniChannel2.userSpecObject());

            writeBuffer = jniChannel1.getBuffer(100, false, error);
            assertNotNull(writeBuffer);
            writeBuffer.data().put("this is a test...".getBytes());
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.write(writeBuffer, writeArgs, error));

            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('t', readBuffer.data().get(0));
            assertEquals('h', readBuffer.data().get(1));
            assertEquals('i', readBuffer.data().get(2));
            assertEquals('s', readBuffer.data().get(3));
            assertEquals(' ', readBuffer.data().get(4));
            assertEquals('i', readBuffer.data().get(5));
            assertEquals('s', readBuffer.data().get(6));
            assertEquals(' ', readBuffer.data().get(7));
            assertEquals('a', readBuffer.data().get(8));
            assertEquals(' ', readBuffer.data().get(9));
            assertEquals('t', readBuffer.data().get(10));
            assertEquals('e', readBuffer.data().get(11));
            assertEquals('s', readBuffer.data().get(12));
            assertEquals('t', readBuffer.data().get(13));
            assertEquals('.', readBuffer.data().get(14));
            assertEquals('.', readBuffer.data().get(15));
            assertEquals('.', readBuffer.data().get(16));

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.close(error));
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.close(error));
        }
        catch (AssumptionViolatedException e)
        {
            throw e;
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

    /**
     * Test: Multicast Channel Buffer Packing (Scenario 1)
     *
     * Tests buffer packing functionality with an unpacked final segment:
     * - Packing multiple segments into a single buffer
     * - Sending without a final packBuffer() call on the last segment
     * - Verifying received messages arrive as individual packed chunks
     */
    @Test
    public void multicastChnlPackingTest1()
    {
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        TransportBuffer writeBuffer, readBuffer = null;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ReadArgs readArgs = TransportFactory.createReadArgs();

        try
        {
            initTransport(false);
            ConnectOptions ops1 = getUnifiedMulticastConnectOptions("235.5.5.7", "15004", "15009");
            ConnectOptions ops2 = getUnifiedMulticastConnectOptions("235.5.5.7", "15004", "15010");
            JNIChannel jniChannel1 = connectRequired(ops1, error);
            JNIChannel jniChannel2 = connectRequired(ops2, error);

            assertEquals(ChannelState.INITIALIZING, jniChannel1.state());
            assertEquals(ChannelState.INITIALIZING, jniChannel2.state());

            while (true)
            {
                jniChannel1.init(inProg, error);
                jniChannel2.init(inProg, error);
                if (jniChannel1.state() == ChannelState.ACTIVE &&
                        jniChannel2.state() == ChannelState.ACTIVE)
                    break;
            }

            assertEquals(ChannelState.ACTIVE, jniChannel1.state());
            assertEquals(ChannelState.ACTIVE, jniChannel2.state());

            writeBuffer = jniChannel1.getBuffer(100, true, error);
            assertNotNull(writeBuffer);
            ByteBuffer bb = writeBuffer.data();
            bb.put("this ".getBytes());
            assertTrue(jniChannel1.packBuffer(writeBuffer, error) > 0);

            bb.put("is a ".getBytes());
            assertTrue(jniChannel1.packBuffer(writeBuffer, error) > 0);

            bb.put("test...".getBytes());
            // intentionally omitting final packBuffer() – tests partial-pack behaviour

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.write(writeBuffer, writeArgs, error));

            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('t', readBuffer.data().get(0));
            assertEquals('h', readBuffer.data().get(1));
            assertEquals('i', readBuffer.data().get(2));
            assertEquals('s', readBuffer.data().get(3));
            assertEquals(' ', readBuffer.data().get(4));
            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('i', readBuffer.data().get(0));
            assertEquals('s', readBuffer.data().get(1));
            assertEquals(' ', readBuffer.data().get(2));
            assertEquals('a', readBuffer.data().get(3));
            assertEquals(' ', readBuffer.data().get(4));
            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('t', readBuffer.data().get(0));
            assertEquals('e', readBuffer.data().get(1));
            assertEquals('s', readBuffer.data().get(2));
            assertEquals('t', readBuffer.data().get(3));
            assertEquals('.', readBuffer.data().get(4));
            assertEquals('.', readBuffer.data().get(5));
            assertEquals('.', readBuffer.data().get(6));

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.close(error));
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.close(error));
        }
        catch (AssumptionViolatedException e)
        {
            throw e;
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

    /**
     * Test: Multicast Channel Buffer Packing (Scenario 2)
     *
     * Tests buffer packing with all segments explicitly packed:
     * - Packing multiple segments including a final packBuffer() call
     * - Verifying all messages arrive properly packed
     * - Confirming data integrity across all packed messages
     */
    @Test
    public void multicastChnlPackingTest2()
    {
        Error error = TransportFactory.createError();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        TransportBuffer writeBuffer, readBuffer = null;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ReadArgs readArgs = TransportFactory.createReadArgs();

        try
        {
            initTransport(false);
            ConnectOptions ops1 = getUnifiedMulticastConnectOptions("235.5.5.8", "15012", "15015");
            ConnectOptions ops2 = getUnifiedMulticastConnectOptions("235.5.5.8", "15012", "15016");
            JNIChannel jniChannel1 = connectRequired(ops1, error);
            JNIChannel jniChannel2 = connectRequired(ops2, error);

            assertEquals(ChannelState.INITIALIZING, jniChannel1.state());
            assertEquals(ChannelState.INITIALIZING, jniChannel2.state());

            while (true)
            {
                jniChannel1.init(inProg, error);
                jniChannel2.init(inProg, error);
                if (jniChannel1.state() == ChannelState.ACTIVE &&
                        jniChannel2.state() == ChannelState.ACTIVE)
                    break;
            }

            assertEquals(ChannelState.ACTIVE, jniChannel1.state());
            assertEquals(ChannelState.ACTIVE, jniChannel2.state());

            writeBuffer = jniChannel1.getBuffer(100, true, error);
            assertNotNull(writeBuffer);
            ByteBuffer bb = writeBuffer.data();
            bb.put("this ".getBytes());
            assertTrue(jniChannel1.packBuffer(writeBuffer, error) > 0);

            bb.put("is a ".getBytes());
            assertTrue(jniChannel1.packBuffer(writeBuffer, error) > 0);

            bb.put("test...".getBytes());
            assertTrue(jniChannel1.packBuffer(writeBuffer, error) > 0);

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.write(writeBuffer, writeArgs, error));

            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('t', readBuffer.data().get(0));
            assertEquals('h', readBuffer.data().get(1));
            assertEquals('i', readBuffer.data().get(2));
            assertEquals('s', readBuffer.data().get(3));
            assertEquals(' ', readBuffer.data().get(4));
            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('i', readBuffer.data().get(0));
            assertEquals('s', readBuffer.data().get(1));
            assertEquals(' ', readBuffer.data().get(2));
            assertEquals('a', readBuffer.data().get(3));
            assertEquals(' ', readBuffer.data().get(4));
            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            assertEquals('t', readBuffer.data().get(0));
            assertEquals('e', readBuffer.data().get(1));
            assertEquals('s', readBuffer.data().get(2));
            assertEquals('t', readBuffer.data().get(3));
            assertEquals('.', readBuffer.data().get(4));
            assertEquals('.', readBuffer.data().get(5));
            assertEquals('.', readBuffer.data().get(6));

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.close(error));
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.close(error));
        }
        catch (AssumptionViolatedException e)
        {
            throw e;
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

    /**
     * Test: Multicast Channel Fragmentation
     *
     * Tests handling of large messages that exceed the default buffer size:
     * - Allocating a 10 KB buffer
     * - Writing data that spans multiple fragmented packets
     * - Handling WRITE_CALL_AGAIN and explicit flush operations
     * - Receiving reassembled data and validating byte-level integrity
     */
    @Test
    public void multicastChnlFragmentationTest()
    {
        Error error = TransportFactory.createError();
        ConnectOptions ops1 = getUnifiedMulticastConnectOptions("235.5.5.9", "15013", "15017");
        ConnectOptions ops2 = getUnifiedMulticastConnectOptions("235.5.5.9", "15013", "15018");
        InProgInfo inProg = TransportFactory.createInProgInfo();
        TransportBuffer writeBuffer, readBuffer = null;
        WriteArgs writeArgs = TransportFactory.createWriteArgs();
        ReadArgs readArgs = TransportFactory.createReadArgs();

        try
        {
            initTransport(false);
            JNIChannel jniChannel1 = connectRequired(ops1, error);
            JNIChannel jniChannel2 = connectRequired(ops2, error);

            assertEquals(ChannelState.INITIALIZING, jniChannel1.state());
            assertEquals(ChannelState.INITIALIZING, jniChannel2.state());

            while (true)
            {
                jniChannel1.init(inProg, error);
                jniChannel2.init(inProg, error);
                if (jniChannel1.state() == ChannelState.ACTIVE &&
                        jniChannel2.state() == ChannelState.ACTIVE)
                    break;
            }

            assertEquals(ChannelState.ACTIVE, jniChannel1.state());
            assertEquals(ChannelState.ACTIVE, jniChannel2.state());

            // Allocate and fill a 10 KB write buffer
            writeBuffer = jniChannel1.getBuffer(10000, false, error);
            assertNotNull(writeBuffer);
            for (int i = 0; i < 10000; i++)
                writeBuffer.data().put((byte) i);

            // Write with fragmentation – handle WRITE_CALL_AGAIN and flush
            int ret = jniChannel1.write(writeBuffer, writeArgs, error);
            while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
                ret = jniChannel1.write(writeBuffer, writeArgs, error);
            while (ret > TransportReturnCodes.SUCCESS)
                ret = jniChannel1.flush(error);
            assertEquals(TransportReturnCodes.SUCCESS, ret);

            // Read reassembled data and verify byte integrity
            while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
            assertNotNull(readBuffer);
            for (int i = 0; i < 10000; i++)
                assertEquals((byte) i, readBuffer.data().get(i));

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.close(error));
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.close(error));
        }
        catch (AssumptionViolatedException e)
        {
            throw e;
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

    /**
     * Test: Blocking Multicast Channel Operations
     *
     * Tests blocking-mode channel behaviour using two threads:
     * - Main thread acts as the blocking sender (unicast port 15025)
     * - Helper thread acts as the blocking receiver (unicast port 15026)
     * - CyclicBarriers synchronise connection establishment and data exchange
     *
     * Multicast availability is pre-verified before spawning the helper thread to
     * avoid indefinite barrier waits if the network is unavailable.
     */
    @Test
    public void blockingMulticastTest()
    {
        Error error = TransportFactory.createError();
        WriteArgs writeArgs = TransportFactory.createWriteArgs();

        try
        {
            initTransport(false);

            // Pre-verify multicast connectivity before spawning the helper thread.
            // If multicast is not available the test is skipped here, avoiding
            // CyclicBarrier hangs that would occur if the thread were started first.
            ConnectOptions preCheckOpts = getUnifiedMulticastConnectOptions("235.6.6.6", "15002", "15099");
            JNIChannel preCheck = connectRequired(preCheckOpts, error);
            preCheck.close(error);

            // Multicast confirmed – start the blocking receiver thread.
            (new Thread(new BlockingClientConnector())).start();

            ConnectOptions ops1 = getUnifiedMulticastConnectOptions("235.6.6.6", "15002", "15025");
            ops1.blocking(true);
            startBarrier.await();
            JNIChannel jniChannel1 = connectRequired(ops1, error);

            assertEquals(ChannelState.ACTIVE, jniChannel1.state());
            assertEquals("TEST UNIFIED CHANNEL", jniChannel1.userSpecObject());

            TransportBuffer writeBuffer = jniChannel1.getBuffer(100, false, error);
            assertNotNull(writeBuffer);
            writeBuffer.data().put("this is a test...".getBytes());
            connectBarrier.await();
            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.write(writeBuffer, writeArgs, error));
            readWriteBarrier.await();

            assertEquals(TransportReturnCodes.SUCCESS, jniChannel1.close(error));
        }
        catch (AssumptionViolatedException e)
        {
            throw e;
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

    /**
     * Helper class for {@link #blockingMulticastTest}.
     *
     * Runs in a separate thread to simulate a blocking receiver. CyclicBarrier
     * synchronisation with the main test thread ensures:
     * 1. Both threads start connecting at the same time (startBarrier)
     * 2. Both channels are active before any I/O (connectBarrier)
     * 3. Main thread does not close the transport before the read is verified (readWriteBarrier)
     */
    private class BlockingClientConnector implements Runnable
    {
        public void run()
        {
            Error error = TransportFactory.createError();
            ConnectOptions ops2 = getUnifiedMulticastConnectOptions("235.6.6.6", "15002", "15026");
            TransportBuffer readBuffer = null;
            ReadArgs readArgs = TransportFactory.createReadArgs();

            try
            {
                initTransport(false);
                ops2.blocking(true);
                startBarrier.await();
                Thread.sleep(1000);
                JNIChannel jniChannel2 = connectRequired(ops2, error);

                assertEquals(ChannelState.ACTIVE, jniChannel2.state());
                assertEquals("TEST UNIFIED CHANNEL", jniChannel2.userSpecObject());

                connectBarrier.await();
                while ((readBuffer = jniChannel2.read(readArgs, error)) == null) { /* spin */ }
                assertNotNull(readBuffer);
                assertEquals('t', readBuffer.data().get(0));
                assertEquals('h', readBuffer.data().get(1));
                assertEquals('i', readBuffer.data().get(2));
                assertEquals('s', readBuffer.data().get(3));
                assertEquals(' ', readBuffer.data().get(4));
                assertEquals('i', readBuffer.data().get(5));
                assertEquals('s', readBuffer.data().get(6));
                assertEquals(' ', readBuffer.data().get(7));
                assertEquals('a', readBuffer.data().get(8));
                assertEquals(' ', readBuffer.data().get(9));
                assertEquals('t', readBuffer.data().get(10));
                assertEquals('e', readBuffer.data().get(11));
                assertEquals('s', readBuffer.data().get(12));
                assertEquals('t', readBuffer.data().get(13));
                assertEquals('.', readBuffer.data().get(14));
                assertEquals('.', readBuffer.data().get(15));
                assertEquals('.', readBuffer.data().get(16));
                readWriteBarrier.await();

                assertEquals(TransportReturnCodes.SUCCESS, jniChannel2.close(error));
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
    }
}
