/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import com.refinitiv.eta.JUnitConfigVariables;
import com.refinitiv.eta.RetryRule;
import com.refinitiv.eta.codec.*;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

import java.nio.ByteBuffer;
import java.util.Base64;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

public class XmlTraceDumpJunit {

    private static final int TEST_BUFFER_SIZE = 512;

    private static final String PREFIX = "\nTest Ongoing Msg\n";
    private static final String RWF_MSG_DUMP_LINE = "<CLOSE domainType=\"0\" streamId=\"0\" containerType=\"NO_DATA\" flags=\"0x01 (HAS_EXTENDED_HEADER)\" dataSize=\"0\">";
    private static final String JSON_MSG = "{\"key1\": \"value\", \"name\": \"ok\", \"flag\": true}";
    private static final String EXPECTED_JSON_DUMP = PREFIX + JSON_MSG;


    final EncodeIterator encodeIterator = CodecFactory.createEncodeIterator();
    final XmlTraceDump xmlTraceDump = CodecFactory.createXmlTraceDump();

    @Rule
    public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

    @Rule
    public TestName testName = new TestName();

    @Before
    public void printTestName() {
        System.out.println(">>>>>>>>>>>>>>>>>>>>  " + testName.getMethodName() + " Test <<<<<<<<<<<<<<<<<<<<<<<");
    }

    @Test
    public void testTraceDumpTransportForJsonProtocol() {
        final XmlTraceDump xmlTraceDump = CodecFactory.createXmlTraceDump();
        final Error error = TransportFactory.createError();
        Transport._globalLock = new DummyLock();
        final RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
        initializeChannel(channel);

        //Prepare JSON msgs for transportBuffer.
        final TransportBuffer transportBuffer = channel.getBuffer(TEST_BUFFER_SIZE, false, error);
        final ByteBuffer dataBuffer = transportBuffer.data();
        dataBuffer.put(JSON_MSG.getBytes());

        final StringBuilder stringBuilder = new StringBuilder("\nTest Ongoing Msg\n");
        xmlTraceDump.dumpBuffer(channel, Codec.JSON_PROTOCOL_TYPE, transportBuffer, null, stringBuilder, error);

        final String completeXmlTrace = stringBuilder.toString();
        assertEquals(errorText(completeXmlTrace), completeXmlTrace, EXPECTED_JSON_DUMP);
    }

    @Test
    public void testTraceDumpTransportForRwfProtocol() {
        final Error error = TransportFactory.createError();
        Transport._globalLock = new DummyLock();
        final RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        initializeChannel(channel);

        final TransportBuffer transportBuffer = channel.getBuffer(TEST_BUFFER_SIZE, false, error);
        encodeIterator.clear();
        encodeIterator.setBufferAndRWFVersion(transportBuffer, channel.majorVersion(), channel.minorVersion());
        prepareMsg(encodeIterator);

        final StringBuilder stringBuilder = new StringBuilder(PREFIX);
        xmlTraceDump.dumpBuffer(channel, Codec.RWF_PROTOCOL_TYPE, transportBuffer, null, stringBuilder, error);

        final String completeXmlTrace = stringBuilder.toString();
        assertTrue(errorText(completeXmlTrace), completeXmlTrace.startsWith(PREFIX));
        assertTrue(errorText(completeXmlTrace), completeXmlTrace.contains(RWF_MSG_DUMP_LINE));
    }

    @Test
    public void testTraceDumpCodecForJsonProtocol() {
        final XmlTraceDump xmlTraceDump = CodecFactory.createXmlTraceDump();
        final Error error = TransportFactory.createError();
        Transport._globalLock = new DummyLock();
        final RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
        initializeChannel(channel);

        //Prepare JSON msgs for transportBuffer.
        final Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.wrap(JSON_MSG.getBytes()));

        final StringBuilder stringBuilder = new StringBuilder("\nTest Ongoing Msg\n");
        xmlTraceDump.dumpBuffer(Codec.majorVersion(), Codec.minorVersion(), Codec.JSON_PROTOCOL_TYPE, buffer, null, stringBuilder, error);

        final String completeXmlTrace = stringBuilder.toString();
        assertEquals(errorText(completeXmlTrace), completeXmlTrace, EXPECTED_JSON_DUMP);
    }

    @Test
    public void testTraceDumpCodecForRwfProtocol() {
        final String expectedLine = "<CLOSE domainType=\"0\" streamId=\"0\" containerType=\"NO_DATA\" flags=\"0x01 (HAS_EXTENDED_HEADER)\" dataSize=\"0\">";
        final Error error = TransportFactory.createError();
        Transport._globalLock = new DummyLock();
        final RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.SOCKET, Codec.RWF_PROTOCOL_TYPE);
        initializeChannel(channel);

        final Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(TEST_BUFFER_SIZE));
        EncodeIterator encodeIterator = CodecFactory.createEncodeIterator();
        encodeIterator.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        prepareMsg(encodeIterator);

        final StringBuilder stringBuilder = new StringBuilder(PREFIX);
        xmlTraceDump.dumpBuffer(Codec.majorVersion(), Codec.minorVersion(), Codec.RWF_PROTOCOL_TYPE, buffer, null, stringBuilder, error);

        final String completeXmlTrace = stringBuilder.toString();
        assertTrue(errorText(completeXmlTrace), completeXmlTrace.startsWith(PREFIX));
        assertTrue(errorText(completeXmlTrace), completeXmlTrace.contains(expectedLine));
    }

    private void initializeChannel(RsslSocketChannel channel) {
        channel._state = ChannelState.ACTIVE;
        channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
        channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
        if (channel.protocolType() == Codec.RWF_PROTOCOL_TYPE) {
            channel._protocolFunctions = new RipcProtocolFunctions(channel);
        } else {
            channel._protocolFunctions = new WSProtocolFunctions(channel);
        }
        channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
        channel._transport = new SocketProtocol();
        channel.growGuaranteedOutputBuffers(10);
        channel._majorVersion = Codec.majorVersion();
        channel._minorVersion = Codec.minorVersion();
    }

    private String errorText(String result) {
        return String.format("Result of tracing:\n\n%s\n" +
                             "<!---------------------->\n", result);
    }

    private void prepareMsg(EncodeIterator encodeIterator) {
        final CloseMsg close = (CloseMsg) CodecFactory.createMsg();
        close.msgClass(MsgClasses.CLOSE);
        close.applyHasExtendedHdr();
        close.extendedHeader().data(
                Base64.getEncoder().encode(ByteBuffer.wrap("base64string".getBytes()))
        );
        close.encode(encodeIterator);
    }
}
