///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import java.io.IOException;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.lang.Float;
import java.lang.Double;

import org.junit.Test;

import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.test.network.replay.NetworkReplay;
import com.refinitiv.eta.test.network.replay.NetworkReplayFactory;
import com.refinitiv.eta.transport.TransportFactory;

public class DecodersJunit
{
	private static final short TEST_DATA_PRIOIRTY_CLASS = 255; // the max value of an "unsigned byte"
	private static final int TEST_DATA_PRIOIRTY_COUNT = 35000; // a valid "unsigned short"

	private static final boolean TEST_DATA_QOS_DYNAMIC = true;
	private static final int TEST_DATA_QOS_RATE = QosRates.TIME_CONFLATED;
	private static final int TEST_DATA_QOS_RATE_INFO = 36000; // a valid "unsigned short";
	private static final int TEST_DATA_QOS_TIMELINESS = QosTimeliness.DELAYED;
	private static final int TEST_DATA_QOS_TIME_INFO = 37000; // a valid "unsigned short";

	private static final boolean TEST_DATA_WORST_QOS_DYNAMIC = true;
	private static final int TEST_DATA_WORST_QOS_RATE = QosRates.TIME_CONFLATED;
	private static final int TEST_DATA_WORST_QOS_RATE_INFO = 38000; // a valid "unsigned short";
	private static final int TEST_DATA_WORST_QOS_TIMELINESS = QosTimeliness.DELAYED;
	private static final int TEST_DATA_WORST_QOS_TIME_INFO = 39000; // a valid "unsigned short";

	private static final int TEST_DATA_MSG_KEY_NAME_TYPE = InstrumentNameTypes.RIC;
	private static final String TEST_DATA_MSG_KEY_NAME = "TRI.N";
	private static final int TEST_DATA_MSG_SERVICE_ID = 4321;

	@Test
	public void bufferReaderTest()
	{
		final int BUFFER_LENGTH = 1000;
		final ByteBuffer byteBuffer = ByteBuffer.allocate(BUFFER_LENGTH);
		final BufferReader bufferReader = new DataBufferReaderWireFormatV1();

		// populate the ByteBuffer with canned data.
		for (int i = 0; i < BUFFER_LENGTH; i++)
		{
			byteBuffer.put((byte)(i % 256));
		}
		byteBuffer.position(0);


		bufferReader.data(byteBuffer);
		bufferReader._position = 0;
		assertEquals(0, bufferReader.position());
		try
		{
			assertFalse(bufferReader.readBoolean()); // i=0, b=0, expect=false

					assertEquals(1, bufferReader.readByte()); // i=1, expect=1
					assertEquals(2, bufferReader.position());

					assertEquals(0x0203, bufferReader.readShort()); // i=2, expected=515(0x0203)
					assertEquals(4, bufferReader.position());

					assertEquals(0x04050607, bufferReader.readInt()); // i=4, expected 67438987(0x04050607)
					assertEquals(8, bufferReader.position());

					assertEquals(579005069656919567L, bufferReader.readLong()); // i=8, expected 579005069656919567L(0x08090a0b0c0d0e0f)
					assertEquals(16, bufferReader.position());

					assertEquals(Double.longBitsToDouble(1157726452361532951L), bufferReader.readDouble(), 0); // i=16, expected L(0x1011121314151617)
					assertEquals(24, bufferReader.position());

					assertEquals(Float.intBitsToFloat(404298267), bufferReader.readFloat(), (double)0); // i=24, expected 404298267(0x18191a1b)
					assertEquals(28, bufferReader.position());

					assertEquals(0x1c1d, bufferReader.readShort()); // i=28, expected 7197(0x1c1d)
					assertEquals(30, bufferReader.position());

					bufferReader.position(127);
					assertEquals(127, bufferReader.readByte()); // i=127, expect=127(0x7f)
					assertEquals(-128, bufferReader.readByte()); // i=128, expect=-128(0x80 signed)
					assertEquals(129, bufferReader.position());
					bufferReader.position(127);
					assertEquals(127, bufferReader.readUnsignedByte()); // i=127, expect=127(0x7f)
					assertEquals(128, bufferReader.readUnsignedByte()); // i=128, expect=128(0x80)
					assertEquals(129, bufferReader.position());

					assertEquals(-32382, bufferReader.readShort()); // i=129 expect=-32382(0x8182 signed)
					bufferReader.position(129);
					assertEquals(33154, bufferReader.readUnsignedShort()); // i=129 expect=33154(0x8182)
					assertEquals(131, bufferReader.position());

					assertEquals(-2088467066, bufferReader.readInt()); // i=131, expect=-2088467066(0x83848586 signed)
					bufferReader.position(131);
					assertEquals(0x83848586, bufferReader.readInt()); // i=131, expect=2206500230(0x83848586)
					assertEquals(135, bufferReader.position());

					// unsigned long > 0x7FFFFFFFFFFFFFFF (a negative value) is out of range of java long.
					bufferReader.position(127);
					assertEquals(9187485637388043654L, bufferReader.readULong()); // i=135, expect 9187485637388043654(0x7f80818283848586)
					bufferReader.position(135);
					assertEquals(-8680537053616894578L, bufferReader.readLong()); // i=135, expect 9766207020092657038(0x8788898a8b8c8d8e)
					assertEquals(143, bufferReader.position());

					// uint16ls
					bufferReader.position(257);
					assertEquals(0, bufferReader.readInt16ls(0)); // don't read anything.
					assertEquals(257, bufferReader.position());
					assertEquals(1, bufferReader.readInt16ls(1)); // i=257, expect 1(0x01)
					assertEquals(515, bufferReader.readInt16ls(2)); // i=257, expect 515(0x0203)
					assertEquals(260, bufferReader.position());
					bufferReader.position(253);
					assertEquals(-3, bufferReader.readInt16ls(1)); // i=253, expect -3(0xfd)
					assertEquals(-257, bufferReader.readInt16ls(2)); // i=254, expect -257(0xfeff)
					assertEquals(256, bufferReader.position());

					// uint16ls
					bufferReader.position(253);
					assertEquals(0, bufferReader.readUInt16ls(0)); // don't read anything.
					assertEquals(253, bufferReader.position());
					assertEquals(253, bufferReader.readUInt16ls(1)); // i=253, expect 253(0xfd)
					assertEquals(65279, bufferReader.readUInt16ls(2)); // i=254, expect 65279(0xfeff)
					assertEquals(256, bufferReader.position());

					// int32ls
					bufferReader.position(128);
					assertEquals(-128, bufferReader.readInt32ls(1)); // i=128, expect -128(0x80)
					assertEquals(-32382, bufferReader.readInt32ls(2)); // i=129, expect -32382(0x8182)
					assertEquals(-8158075, bufferReader.readInt32ls(3)); // i=131, expect -8158075(0x838485)
					assertEquals(-2037938039, bufferReader.readInt32ls(4)); // i=134, expect (0x86878889)
					assertEquals(138, bufferReader.position());

					// uint32ls
					bufferReader.position(128);
					assertEquals(128, bufferReader.readUInt32ls(1)); // i=128, expect 128(0x80)
					assertEquals(33154, bufferReader.readUInt32ls(2)); // i=129, expect 33154(0x8182)
					assertEquals(8619141, bufferReader.readUInt32ls(3)); // i=131, expect 8619141(0x838485)
					assertEquals(2257029257L, bufferReader.readUInt32ls(4)); // i=134, expect 2257029257(0x86878889)
					assertEquals(138, bufferReader.position());


					// OB = Optimized Byte

					// ushort16ob
					// if first byte is < FE, use the one byte value.
					// if first byte is FE, use the following two byte value (three bytes total).
					bufferReader.position(253);
					assertEquals(253, bufferReader.readUShort16ob()); // i=253, expect 253(0xFD)
					assertEquals(65280, bufferReader.readUShort16ob()); // i=254, expect 65280(0xFF00), since FE signifies two bytes.
					assertEquals(257, bufferReader.position());

					//uint32ob
					// < FE = no additional bytes
					// FE = two additional bytes
					// FF = three additional bytes
					bufferReader.position(253);
					assertEquals(253, bufferReader.readUInt32ob()); // i=253, expect 253(0xFD)
					assertEquals(65280, bufferReader.readUInt32ob()); // i=254, expect 65280(0xFEFF00->0xFF00), since FE signifies two bytes.
					assertEquals(257, bufferReader.position());
					bufferReader.position(255);
					assertEquals(66051, bufferReader.readUInt32ob()); // i=254, expect 66051(0xFF00010203->0x00010203), since FF signifies four bytes.
					assertEquals(260, bufferReader.position());

					// long64ls
					bufferReader.position(127);
					assertEquals(0, bufferReader.readLong64ls(0));
					assertEquals(127, bufferReader.position());
					assertEquals(127, bufferReader.readLong64ls(1));
					assertEquals(-128, bufferReader.readLong64ls(1));
					assertEquals(129, bufferReader.position());
					bufferReader.position(127);
					assertEquals(32640, bufferReader.readLong64ls(2)); // expect 32640(0x7f80)
					assertEquals(-32382, bufferReader.readLong64ls(2)); // expect -32382(0x8182)
					assertEquals(131, bufferReader.position());
					bufferReader.position(127);
					assertEquals(8355969, bufferReader.readLong64ls(3)); // expect 8355969(0x7f8081)
					assertEquals(-8223868, bufferReader.readLong64ls(3)); // expect -8223868(0x828384)
					assertEquals(133, bufferReader.position());
					bufferReader.position(127);
					assertEquals(2139128194, bufferReader.readLong64ls(4)); // expect 2139128194(0x7f808182)
					assertEquals(-2088467066, bufferReader.readLong64ls(4)); // expect -2088467066(0x83848586)
					assertEquals(135, bufferReader.position());
					bufferReader.position(127);
					assertEquals(547616817795L, bufferReader.readLong64ls(5)); // expect 547616817795L(0x7f80818283)
					assertEquals(-530335758456L, bufferReader.readLong64ls(5)); // expect -530335758456L(0x8485868788)
					assertEquals(137, bufferReader.position());
					bufferReader.position(127);
					assertEquals(140189905355652L, bufferReader.readLong64ls(6)); // expect 140189905355652L(0x7f8081828384)
					assertEquals(-134662130726518L, bufferReader.readLong64ls(6)); // expect -134662130726518L(0x85868788898a)
					assertEquals(139, bufferReader.position());
					bufferReader.position(127);
					assertEquals(35888615771047045L, bufferReader.readLong64ls(7)); // expect 35888615771047045L(0x7f808182838485)
					assertEquals(-34190926665839732L, bufferReader.readLong64ls(7)); // expect -34190926665839732L(0x868788898a8b8c)
					assertEquals(141, bufferReader.position());
					bufferReader.position(127);
					assertEquals(9187485637388043654L, bufferReader.readLong64ls(8)); // expect 9187485637388043654L(0x7f80818283848586)
					assertEquals(-8680537053616894578L, bufferReader.readLong64ls(8)); // expect -8680537053616894578L(0x8788898a8b8c8d8e)
					assertEquals(143, bufferReader.position());


					// ulong64ls

					bufferReader.position(127);
					assertEquals(0, bufferReader.readULong64ls(0));
					assertEquals(127, bufferReader.position());
					assertEquals(127, bufferReader.readULong64ls(1));
					assertEquals(128, bufferReader.readULong64ls(1));
					assertEquals(129, bufferReader.position());
					bufferReader.position(127);
					assertEquals(32640, bufferReader.readULong64ls(2)); // expect 32640(0x7f80)
					assertEquals(33154, bufferReader.readULong64ls(2)); // expect 33154(0x8182)
					assertEquals(131, bufferReader.position());
					bufferReader.position(127);
					assertEquals(8355969, bufferReader.readULong64ls(3)); // expect 8355969(0x7f8081)
					assertEquals(8553348, bufferReader.readULong64ls(3)); // expect 8553348(0x828384)
					assertEquals(133, bufferReader.position());
					bufferReader.position(127);
					assertEquals(2139128194, bufferReader.readULong64ls(4)); // expect 2139128194(0x7f808182)
					assertEquals(2206500230L, bufferReader.readULong64ls(4)); // expect 2206500230L(0x83848586)
					assertEquals(135, bufferReader.position());
					bufferReader.position(127);
					assertEquals(547616817795L, bufferReader.readULong64ls(5)); // expect 547616817795L(0x7f80818283)
					assertEquals(569175869320L, bufferReader.readULong64ls(5)); // expect 569175869320L(0x8485868788)
					assertEquals(137, bufferReader.position());
					bufferReader.position(127);
					assertEquals(140189905355652L, bufferReader.readULong64ls(6)); // expect 140189905355652L(0x7f8081828384)
					assertEquals(146812845984138L, bufferReader.readULong64ls(6)); // expect 146812845984138L(0x85868788898a)
					assertEquals(139, bufferReader.position());
					bufferReader.position(127);
					assertEquals(35888615771047045L, bufferReader.readULong64ls(7)); // expect 35888615771047045L(0x7f808182838485)
					assertEquals(37866667372088204L, bufferReader.readULong64ls(7)); // expect 37866667372088204L(0x868788898a8b8c)
					assertEquals(141, bufferReader.position());
					bufferReader.position(127);
					assertEquals(9187485637388043654L, bufferReader.readULong64ls(8)); // expect 9187485637388043654L(0x7f80818283848586)
					// cannot read an 8 byte unsigned long over 0x7fffffffffffffff, need to use BigInteger.
					assertEquals(135, bufferReader.position());


					// RB = Reserved Bit.

					bufferReader.position(127);
					// reserve bit, 15-rb high order bit signifies one byte or two.
					assertEquals(127, bufferReader.readUShort15rb()); // i=127, expect 127(0x7f)
					assertEquals(129, bufferReader.readUShort15rb()); // i=128, expect 129(0x8081)
					assertEquals(130, bufferReader.position());
					bufferReader.position(254);
					assertEquals(32511, bufferReader.readUShort15rb()); // i=254, expect 32511(0xfeff)


					bufferReader.position(0);
					assertEquals(0, bufferReader.readUInt30rb()); // i=0, expect 0(0x00)
					bufferReader.position(63);
					assertEquals(63, bufferReader.readUInt30rb()); // i=63, expect 63(0x3f)
					assertEquals(64, bufferReader.position());

					bufferReader.position(128);
					assertEquals(129, bufferReader.readUInt30rb()); // i=191, expect(0x8081->0x81)
					assertEquals(130, bufferReader.position());
					bufferReader.position(191);
					assertEquals(16320, bufferReader.readUInt30rb()); // i=128, expect 16320(0xbfc0->0x3fc0)
					assertEquals(193, bufferReader.position());

					bufferReader.position(79);
					assertEquals(1003601, bufferReader.readUInt30rb()); // i=127, expect 1003601(0x4F5051->0x0f5051)
					assertEquals(82, bufferReader.position());
					bufferReader.position(127);
					assertEquals(4161665, bufferReader.readUInt30rb()); // i=127, expect 4161665(0x7F8081->0x3f8081)
					assertEquals(130, bufferReader.position());

					bufferReader.position(207);
					assertEquals(265343442, bufferReader.readUInt30rb()); // i=127, expect 265343442(0xCFd0d1d2->0x0fd0d1d2)
					assertEquals(211, bufferReader.position());
					bufferReader.position(255);
					assertEquals(1056964866, bufferReader.readUInt30rb()); // i=127, expect 1056964866(0xFF000102->0x3f000102)
					assertEquals(259, bufferReader.position());
		}
		catch (Exception e)
		{
			assertTrue(false);
		}
    }
    
    /**
     * Prints a message as XML
     * 
     * @param msg The message to print
     */
    private void printXml(Msg msg)
    {
        assertTrue(msg != null);
        
        DecodeIterator iter = CodecFactory.createDecodeIterator();
        iter.setBufferAndRWFVersion(msg.encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion());
        System.out.println(msg.decodeToXml(iter));
    }    
    
    /**
     * Tests a message with JSON as payload 
     */
    @Test
    public void decodeMsgWithJsonPayload()
    {
    	decodeJsonMsg();
    }
    /** 
     * Tests a message with JSON as element entry in payload 
     */
    @Test
    public void decodeMsgWithJsonElement()
    {
    	decodeJsonElement();   	
    }
    
    /**
     * Verifies that a Close message with an extended header can be decoded
     */
    @Test
    public void decodeCodeMsgWithExtendedHeader()
    {
        decodeCloseMsg(true);
    }

    /**
     * Verifies that a Close message with no extended header can be decoded
     */
    @Test
    public void decodeCloseMsgNoExtendedHeader()
    {
        decodeCloseMsg(false);
    }

   
    /**
     * Decodes a close message
     * @param hasExtendedHeader If {@code true}, decode an extended header
     */
    private void decodeCloseMsg(boolean hasExtendedHeader)
    {
        final int streamId = 1234; // also the combo to my luggage
        final byte[] extendedHeader = { 9, 8, 7, 6};
        
        // build a close message to decode
        ByteBuffer bb = buildCloseMsg(streamId, (hasExtendedHeader ? extendedHeader : null));

        // create & associate an RsslBuffer with the ByteBuffer containing the close message
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // create a message and decode it
        Msg msg = CodecFactory.createMsg();
        msg.clear();
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
        
        // cast to the close-specific message, and validate it
        assertEquals(MsgClasses.CLOSE, msg.msgClass());
        CloseMsg closeMsg = (CloseMsg)msg;

        assertEquals(streamId, closeMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, closeMsg.domainType());
        assertEquals(DataTypes.NO_DATA, closeMsg.containerType());
        
        if (hasExtendedHeader)
        {
            assertEquals(true, closeMsg.checkHasExtendedHdr());
            
            final byte[] extractedExtendedHeader = new byte[closeMsg.extendedHeader().length()];
            closeMsg.extendedHeader().copy(extractedExtendedHeader);
            assertArrayEquals(extendedHeader, extractedExtendedHeader);
        }
        else
        {
            assertEquals(CloseMsgFlags.NONE, closeMsg.flags());
        }
        
        System.out.println("Input: hasExtendedHeader=" + hasExtendedHeader);
        printXml(closeMsg);
    }
    
    /**
     * Builds a close message and returns a ByteBuffer containing the
     * encoded message.
     * 
     * @param streamId The stream ID applicable to the message
     * @param extendedHeaderContent If not {@code null}, this data will
     * be encoded as an extended header
     * 
     * @return a ByteBuffer containing the encoded message
     */
    private ByteBuffer buildCloseMsg(int streamId, byte[] extendedHeaderContent)
    {
        final int MAX_MSG_SIZE = 100;
        
        // create a close message
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId);
        closeMsg.domainType(DomainTypes.MARKET_PRICE);
        closeMsg.containerType(DataTypes.NO_DATA); // data format
        
        if (extendedHeaderContent != null)
        {
            closeMsg.flags(CloseMsgFlags.HAS_EXTENDED_HEADER);
            Buffer extendedHeader = CodecFactory.createBuffer();
            extendedHeader.data(ByteBuffer.wrap(extendedHeaderContent));
            closeMsg.extendedHeader(extendedHeader);
        }
        else
        {
            closeMsg.flags(CloseMsgFlags.NONE);
        }
        
        // allocate a ByteBuffer and associate it with an RsslBuffer
        ByteBuffer bb = ByteBuffer.allocate(MAX_MSG_SIZE);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);
        
        // create an encode iterator
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // encode the message
        assertEquals(CodecReturnCodes.SUCCESS, closeMsg.encode(encodeIter));
        bb.flip(); // prepare it for immediate decoding
        
        return bb;
    }

    /**
     * Decodes a close message containing an invalid extended header
     */
    @Test
    public void decodeCloseMsgInvalidExtendedHeader()
    {
        final String fileName = "src/test/resources/com/refinitiv/eta/data/RsslDecodersJunit/CloseMsgInvalidExtHdr.txt";
        
        NetworkReplay fileReplay = NetworkReplayFactory.create();
        try
        {
            BufferImpl._runningInJunits = true; // disable assert()s
            
            fileReplay.parseFile(fileName);
            assertEquals(1, fileReplay.recordsInQueue());
            
            byte[] closeMsg = fileReplay.read();
            assertTrue(closeMsg != null && closeMsg.length > 0);
            
            // build a close message to decode
            ByteBuffer bb = ByteBuffer.wrap(closeMsg);

            // create & associate an RsslBuffer with the ByteBuffer containing the close message
            Buffer buffer = CodecFactory.createBuffer();
            buffer.data(bb);
            
            // create and associate a decode iterator with the RsslBuffer
            DecodeIterator dIter = CodecFactory.createDecodeIterator();
            dIter.clear();
            dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            
            // create a message and decode it
            Msg msg = CodecFactory.createMsg();
            msg.clear();
            assertEquals(CodecReturnCodes.INCOMPLETE_DATA, msg.decode(dIter));
        }
        catch (IOException e)
        {
            assertTrue("failed to read input file", false);
        }
        finally
        {
            BufferImpl._runningInJunits = false; // enable assert()s
        }
    }    
    
    /**
     * Test RsslCloseMsg.decode with data from ETAC.
     */
    @Test
    public void decodeCloseMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/003_closeMsg.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        CloseMsg msg = (CloseMsg) CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.CLOSE, msg.msgClass());
        assertEquals(DomainTypes.MARKET_PRICE, msg.domainType());
        assertEquals(2147483647, msg.streamId());
        assertEquals(true, msg.checkAck());
        // Data Type
        assertEquals(DataTypes.NO_DATA, msg.containerType());
        // Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        assertEquals("EXTENDED HEADER", msg.extendedHeader().toString());

        assertEquals(true, msg.isFinalMsg());
    }
    
    /**
     * Verifies that a Request message with an extended header can be decoded
     */
    @Test
    public void decodeRequestMsgWithExtendedHeader()
    {
        decodeRequestMsg(true, false, false, false);
    }
    
    /**
     * Verifies that a Request message with a priority class and priority count can be decoded
     */
    @Test
    public void decodeRequestMsgWithPriority()
    {
        decodeRequestMsg(false, true, false, false);
    }

    /**
     * Verifies that a Request message with a Qos and Worst Qos count can be decoded
     */
    @Test
    public void decodeRequestMsgWithQos()
    {
        decodeRequestMsg(false, false, true, false);
    }
    
    /**
     * Verifies that a Request message a message key
     */
    @Test
    public void decodeRequestMsgMsgKey()
    {
        decodeRequestMsg(false, false, false, true);
    }    

    /**
     * Verifies that a Request message with no optional data
     */
    @Test
    public void decodeRequestMsgNoOptionalData()
    {
        decodeRequestMsg(false, false, false, false);
    }
    
    /**
     * Verifies that a Request message with all optional data
     */
    @Test
    public void decodeRequestMsgAllOptionalData()
    {
        decodeRequestMsg(true, true, true, true);
    } 
    
    
    /**
     * Decodes a request message
     * @param hasExtendedHeader If {@code true}, decode an extended header
     * @param hasPrioirtyClass If {@code true}, decode a priority class and priority count
     * @param hasQos If {@code true}, decode a Qos and Worst Qos
     * @param hasMsgKey If {@code true}, decode a message key
     */
    private void decodeRequestMsg(boolean hasExtendedHeader, boolean hasPrioirtyClass, boolean hasQos, boolean hasMsgKey)
    {
        final int streamId = 1234; // also the combo to my luggage
        final byte[] extendedHeader = { 9, 8, 7, 6};
        boolean hasOptionalData = false;
        
        // build a request message to decode
        ByteBuffer bb = buildRequestMsg(streamId, (hasExtendedHeader ? extendedHeader : null), hasPrioirtyClass, hasQos, hasMsgKey);
        
        // create & associate an RsslBuffer with the ByteBuffer containing the request message
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // create a message and decode it
        Msg msg = CodecFactory.createMsg();
        msg.clear();
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));

        // cast to the request-specific message, and validate it
        assertEquals(MsgClasses.REQUEST, msg.msgClass());
        RequestMsg requestMsg = (RequestMsg)msg;
        
        assertEquals(streamId, requestMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, requestMsg.domainType());
        assertEquals(DataTypes.NO_DATA, requestMsg.containerType());
        
        if (hasPrioirtyClass)
        {
            hasOptionalData = true;
            
            assertEquals(true, requestMsg.checkHasPriority());
            
            assertEquals(TEST_DATA_PRIOIRTY_CLASS, requestMsg.priority().priorityClass());
            assertEquals(TEST_DATA_PRIOIRTY_COUNT, requestMsg.priority().count());
        }
        else
        {
            assertEquals(false, requestMsg.checkHasPriority());
            assertEquals(null, requestMsg.priority());
        }
        
        if (hasQos)
        {
            hasOptionalData = true;
            
            assertEquals(true, requestMsg.checkHasQos());
            
            assertEquals(TEST_DATA_QOS_DYNAMIC, requestMsg.qos().isDynamic());
            assertEquals(TEST_DATA_QOS_RATE, requestMsg.qos().rate());
            assertEquals(TEST_DATA_QOS_RATE_INFO, requestMsg.qos().rateInfo());
            assertEquals(TEST_DATA_QOS_TIMELINESS, requestMsg.qos().timeliness());
            assertEquals(TEST_DATA_QOS_TIME_INFO, requestMsg.qos().timeInfo());
            
            assertEquals(true, requestMsg.checkHasWorstQos());
            
            assertEquals(TEST_DATA_WORST_QOS_DYNAMIC, requestMsg.worstQos().isDynamic());
            assertEquals(TEST_DATA_WORST_QOS_RATE, requestMsg.worstQos().rate());
            assertEquals(TEST_DATA_WORST_QOS_RATE_INFO, requestMsg.worstQos().rateInfo());
            assertEquals(TEST_DATA_WORST_QOS_TIMELINESS, requestMsg.worstQos().timeliness());
            assertEquals(TEST_DATA_WORST_QOS_TIME_INFO, requestMsg.worstQos().timeInfo());
        }
        else
        {
            assertEquals(false, requestMsg.checkHasQos());
            assertEquals(null, requestMsg.qos());
            
            assertEquals(false, requestMsg.checkHasWorstQos());
            assertEquals(null, requestMsg.worstQos());
        }        
        
        if (hasExtendedHeader)
        {
            hasOptionalData = true;
            
            assertEquals(true, requestMsg.checkHasExtendedHdr());
            
            final byte[] extractedExtendedHeader = new byte[requestMsg.extendedHeader().length()];
            requestMsg.extendedHeader().copy(extractedExtendedHeader);
            assertArrayEquals(extendedHeader, extractedExtendedHeader);
        }
        else
        {
            assertEquals(false, requestMsg.checkHasExtendedHdr());
        }
        
        if (hasMsgKey)
        {
            assertEquals(true, requestMsg.msgKey().checkHasNameType());
            assertEquals(true, requestMsg.msgKey().checkHasName());
            assertEquals(true, requestMsg.msgKey().checkHasServiceId());
            
            assertEquals(TEST_DATA_MSG_KEY_NAME_TYPE, requestMsg.msgKey().nameType());
            assertEquals(TEST_DATA_MSG_KEY_NAME, requestMsg.msgKey().name().toString());
            assertEquals(TEST_DATA_MSG_SERVICE_ID,requestMsg.msgKey().serviceId());
        }        
        
        if (!hasOptionalData)
        {
            assertEquals(RequestMsgFlags.NONE, requestMsg.flags());
        }

        StringBuilder input = new StringBuilder();
        input.append("Input: hasExtendedHeader=");
        input.append(hasExtendedHeader);
        input.append(" hasPrioirtyClass=");
        input.append(hasPrioirtyClass);
        input.append(" hasQos=");
        input.append(hasQos);
        input.append(" hasMsgKey=");
        input.append(hasMsgKey);
        System.out.println(input.toString());
        printXml(requestMsg);
    }
    
    
    /**
     * Decodes a message with JSON content message
     */
    private void decodeJsonMsg()
    {
        final int streamId = 1234; // also the combo to my luggage
        final int MAX_MSG_SIZE = 100;          
        // build a close message to decode
        ByteBuffer bb = ByteBuffer.allocate(MAX_MSG_SIZE);
        		
        // create & associate an RsslBuffer with the ByteBuffer containing the close message
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);	
     
        
        // create a close message
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId);
        closeMsg.domainType(DomainTypes.MARKET_PRICE);
        closeMsg.containerType(DataTypes.JSON); // data format
        closeMsg.flags(CloseMsgFlags.NONE);
        closeMsg.encodedDataBody().data("This is fake JSON content");
     
                  
        // create an encode iterator
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // encode the message
        assertEquals(CodecReturnCodes.SUCCESS, closeMsg.encode(encodeIter));
        bb.flip(); // prepare it for immediate decoding        		        		
        buffer.data(bb);
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // create a message and decode it
        Msg msg = CodecFactory.createMsg();
        msg.clear();
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
        
        // cast to the close-specific message, and validate it
        assertEquals(MsgClasses.CLOSE, msg.msgClass());
        CloseMsg decCloseMsg = (CloseMsg)msg;

        assertEquals(streamId, decCloseMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, decCloseMsg.domainType());
        assertEquals(DataTypes.JSON, decCloseMsg.containerType());
        assertEquals(false, closeMsg.checkHasExtendedHdr());

        assertEquals("This is fake JSON content", closeMsg.encodedDataBody().toString());

      
        System.out.println("Input: payload=" + closeMsg.encodedDataBody().toString());
        printXml(decCloseMsg);
    }
    
    private void decodeJsonElement()
    {
        final int streamId = 1234; // also the combo to my luggage
        final int MAX_MSG_SIZE = 100;          
        // build a close message to decode
        ByteBuffer bb = ByteBuffer.allocate(MAX_MSG_SIZE);
        		
        // create & associate an RsslBuffer with the ByteBuffer containing the close message
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);	
     
        
        // create a close message
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(streamId);
        closeMsg.domainType(DomainTypes.MARKET_PRICE);
        closeMsg.containerType(DataTypes.ELEMENT_LIST); // data format
        closeMsg.flags(CloseMsgFlags.NONE);               
                  
        // create an encode iterator
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // encode the message
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, closeMsg.encodeInit(encodeIter, 50));
        
        ElementList elementList = CodecFactory.createElementList();
        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 0));
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        Buffer elementName = CodecFactory.createBuffer();
        elementName.data("TestJSON");
        elementEntry.name(elementName);
        elementEntry.dataType(DataTypes.JSON);
        elementEntry.encodedData().data("This is also fake JSON content");
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(encodeIter));        		
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, closeMsg.encodeComplete(encodeIter,  true));                      
                               
        bb.flip(); // prepare it for immediate decoding        		        		
        buffer.data(bb);
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // create a message and decode it
        Msg msg = CodecFactory.createMsg();
        msg.clear();
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
        
        // cast to the close-specific message, and validate it
        assertEquals(MsgClasses.CLOSE, msg.msgClass());
        CloseMsg decCloseMsg = (CloseMsg)msg;

        assertEquals(streamId, decCloseMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, decCloseMsg.domainType());
        assertEquals(DataTypes.ELEMENT_LIST, decCloseMsg.containerType());
        assertEquals(false, closeMsg.checkHasExtendedHdr());
        
        ElementList decelementList = CodecFactory.createElementList();        
        assertEquals(CodecReturnCodes.SUCCESS, decelementList.decode(dIter,  null));
        ElementEntry decelementEntry = CodecFactory.createElementEntry();        
        assertEquals(false, decelementList.checkHasInfo());
        assertEquals(false, decelementList.checkHasSetData());
        assertEquals(false, decelementList.checkHasSetId());
        assertEquals(true, decelementList.checkHasStandardData());
        // msgKey's elementList entry 1
        assertEquals(CodecReturnCodes.SUCCESS, decelementEntry.decode(dIter));
        assertEquals(DataTypes.JSON, decelementEntry.dataType());      
        assertEquals(elementName.toString(), decelementEntry.name().toString()); 
        assertEquals("This is also fake JSON content", decelementEntry.encodedData().toString());

      
        System.out.println("Element Entry JSON payload=" + decelementEntry.encodedData().data().toString());
        printXml(decCloseMsg);
    }
    
    /**
     * Builds a request message and returns a ByteBuffer containing the
     * encoded message.
     * 
     * @param streamId The stream ID applicable to the message
     * @param extendedHeaderContent If not {@code null}, this data will
     * be encoded as an extended header
     * @param hasPrioirtyClass If true, encode a priority class
     * @param hasQos If {@code true}, encode Qos and Worst Qos
     * @param hasMsgKey If {@code true}, encode a message key
     * 
     * @return a ByteBuffer containing the encoded message
     */
    private ByteBuffer buildRequestMsg(int streamId, byte[] extendedHeaderContent, boolean hasPrioirtyClass, boolean hasQos, boolean hasMsgKey)
    {
        final int MAX_MSG_SIZE = 100;
        boolean optionalFlagsSet = false;
        
        // create a request message
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(streamId);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.containerType(DataTypes.NO_DATA);
        
        if (hasPrioirtyClass)
        {
            optionalFlagsSet = true;
            
            requestMsg.applyHasPriority();
            requestMsg.priority().priorityClass(TEST_DATA_PRIOIRTY_CLASS);
            requestMsg.priority().count(TEST_DATA_PRIOIRTY_COUNT);
        }
        
        if (hasQos)
        {
            optionalFlagsSet = true;
            
            requestMsg.applyHasQos();
            requestMsg.qos().dynamic(TEST_DATA_QOS_DYNAMIC);
            requestMsg.qos().rate(TEST_DATA_QOS_RATE);
            requestMsg.qos().rateInfo(TEST_DATA_QOS_RATE_INFO);
            requestMsg.qos().timeliness(TEST_DATA_QOS_TIMELINESS);
            requestMsg.qos().timeInfo(TEST_DATA_QOS_TIME_INFO);
            
            requestMsg.applyHasWorstQos();
            requestMsg.worstQos().dynamic(TEST_DATA_WORST_QOS_DYNAMIC);
            requestMsg.worstQos().rate(TEST_DATA_WORST_QOS_RATE);
            requestMsg.worstQos().rateInfo(TEST_DATA_WORST_QOS_RATE_INFO);
            requestMsg.worstQos().timeliness(TEST_DATA_WORST_QOS_TIMELINESS);
            requestMsg.worstQos().timeInfo(TEST_DATA_WORST_QOS_TIME_INFO);            
        }
        
        if (extendedHeaderContent != null)
        {
            optionalFlagsSet = true;
            
            requestMsg.applyHasExtendedHdr();
            Buffer extendedHeader = CodecFactory.createBuffer();
            extendedHeader.data(ByteBuffer.wrap(extendedHeaderContent));
            requestMsg.extendedHeader(extendedHeader);
        }
        
        if (!optionalFlagsSet)
        {
            requestMsg.flags(RequestMsgFlags.NONE);
        }

        if (hasMsgKey)
        {
            // specify msgKey members
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().nameType(TEST_DATA_MSG_KEY_NAME_TYPE);
            requestMsg.msgKey().name().data(TEST_DATA_MSG_KEY_NAME);
            requestMsg.msgKey().serviceId(TEST_DATA_MSG_SERVICE_ID);            
        }
        
        // allocate a ByteBuffer and associate it with an RsslBuffer
        ByteBuffer bb = ByteBuffer.allocate(MAX_MSG_SIZE);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        // create an encode iterator
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.clear();
        encodeIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        // encode the message
        assertEquals(CodecReturnCodes.SUCCESS, requestMsg.encode(encodeIter));
        bb.flip(); // prepare it for immediate decoding
        return bb;
    }

    /**
     * Decodes a request message containing an invalid extended header
     */
    @Test
    public void decodeRequestMsgInvalidExtendedHeader()
    {
        final String fileName = "src/test/resources/com/refinitiv/eta/data/RsslDecodersJunit/RequestMsgInvalidExtHdr.txt";
        
        decodeRequestMsgFromFile(fileName, CodecReturnCodes.INCOMPLETE_DATA);
    }
    
    /**
     * Decodes a request message containing an invalid message key name length
     */
    @Test
    public void decodeRequestMsgInvalidMsgKeyNameLen()
    {
        final String fileName = "src/test/resources/com/refinitiv/eta/data/RsslDecodersJunit/RequestMsgInvalidMsgKeyNameLen.txt";
        
        decodeRequestMsgFromFile(fileName, CodecReturnCodes.INCOMPLETE_DATA);
    }
    
    /**
     * Decodes a request message containing an invalid message key length
     */
    @Test
    public void decodeRequestMsgInvalidMsgKeyLen()
    {
        final String fileName = "src/test/resources/com/refinitiv/eta/data/RsslDecodersJunit/RequestMsgInvalidMsgKeyLen.txt";
        
        decodeRequestMsgFromFile(fileName, CodecReturnCodes.INCOMPLETE_DATA);
    }    

    
    /**
     * Decodes a request message from a hex dump file and asserts that
     * of {@link Msg#decode(DecodeIterator)} returns the expected
     * result.
     * 
     * @param fileName The name of the file that contains the hex dump
     * @param expectedResult The expected result of the call to the
     * {@link Msg#decode(DecodeIterator)}
     */
    public void decodeRequestMsgFromFile(String fileName, int expectedResult)
    {
        assertTrue(fileName != null);
        
        NetworkReplay fileReplay = NetworkReplayFactory.create();
        try
        {
            BufferImpl._runningInJunits = true; // disable assert()s
            
            fileReplay.parseFile(fileName);
            assertEquals(1, fileReplay.recordsInQueue());
            
            byte[] requestMsg = fileReplay.read();
            assertTrue(requestMsg != null && requestMsg.length > 0);
            
            // build a close message to decode
            ByteBuffer bb = ByteBuffer.wrap(requestMsg);

            // create & associate an RsslBuffer with the ByteBuffer containing the close message
            Buffer buffer = CodecFactory.createBuffer();
            buffer.data(bb);
            
            // create and associate a decode iterator with the RsslBuffer
            DecodeIterator dIter = CodecFactory.createDecodeIterator();
            dIter.clear();
            dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
            
            // create a message and decode it
            Msg msg = CodecFactory.createMsg();
            msg.clear();
            assertEquals(expectedResult, msg.decode(dIter));
        }
        catch (IOException e)
        {
            assertTrue("failed to read input file", false);
        }
        finally
        {
            BufferImpl._runningInJunits = false; // enable assert()s
        }
    }
    
    /**
     * Test RsslRequestMsg.decode with data from ETAC.
     */
    @Test
    public void decodeRequestMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/001_requestMsg.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer etaBuffer = CodecFactory.createBuffer();
        etaBuffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        RequestMsg msg = (RequestMsg) CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.REQUEST, msg.msgClass());
        assertEquals(DomainTypes.MARKET_PRICE, msg.domainType());
        assertEquals(100, msg.streamId());
        assertEquals(true, msg.checkStreaming());
        assertEquals(true, msg.checkConfInfoInUpdates());
        assertEquals(true, msg.checkHasBatch());
        assertEquals(true, msg.checkHasView());
        assertEquals(true, msg.checkMsgKeyInUpdates());
        assertEquals(false, msg.checkNoRefresh());
        assertEquals(false, msg.checkPause());
        assertEquals(false, msg.checkPrivateStream());
        // data format
        assertEquals(DataTypes.ELEMENT_LIST, msg.containerType());
        // Priority
        assertEquals(true, msg.checkHasPriority());
        assertEquals(3, msg.priority().priorityClass());
        assertEquals(4, msg.priority().count());
        // Qos and Worst Qos
        assertEquals(true, msg.checkHasQos());        
        assertEquals(true, msg.checkHasWorstQos());
        assertNotNull(msg.qos());
        assertNotNull(msg.worstQos());
        assertEquals(false, msg.qos().isDynamic());
        assertEquals(QosTimeliness.REALTIME, msg.qos().timeliness());
        assertEquals(QosRates.TICK_BY_TICK, msg.qos().rate());
        assertEquals(QosTimeliness.DELAYED, msg.worstQos().timeliness());
        assertEquals(65532, msg.worstQos().timeInfo());
        assertEquals(QosRates.TIME_CONFLATED, msg.worstQos().rate());
        assertEquals(65533, msg.worstQos().rateInfo());
        // Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        assertEquals("EXTENDED HEADER", msg.extendedHeader().toString());
        assertEquals(true, msg.qos().isBetter(msg.worstQos()));
        assertEquals(false, msg.isFinalMsg());
        
        // msgKey
        MsgKey key = msg.msgKey();
        assertEquals(true, key.checkHasAttrib());
        assertEquals(false, key.checkHasFilter());
        assertEquals(false, key.checkHasIdentifier());
        assertEquals(true, key.checkHasName());
        assertEquals(true, key.checkHasNameType());
        assertEquals(false, key.checkHasServiceId());
        assertEquals(InstrumentNameTypes.RIC, key.nameType());
        assertEquals("Batch_Request", key.name().toString());
        assertEquals(DataTypes.ELEMENT_LIST, key.attribContainerType());
        assertEquals(CodecReturnCodes.SUCCESS, msg.decodeKeyAttrib(dIter, key));
        
        // decode msgKey's Attrib (ElementList with three ElementEntries)
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
        assertEquals(false, elementList.checkHasInfo());
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        assertEquals(true, elementList.checkHasStandardData());
        // msgKey's elementList entry 1
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPID.toString(), elementEntry.name().toString());
        assertEquals("256", elementEntry.encodedData().toString());
        // msgKey's elementList entry 2        
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPNAME.toString(), elementEntry.name().toString());
        assertEquals("rsslConsumer", elementEntry.encodedData().toString());
        // msgKey's elementList entry 3
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.POSITION.toString(), elementEntry.name().toString());
        assertEquals("localhost", elementEntry.encodedData().toString());
        // msgKey's elementList END_OF_CONTAINER
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        
        // decode payload (ElementList with Batch and View entries (two entries)
        elementList.clear();
        elementEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
        assertEquals(false, elementList.checkHasInfo());
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        assertEquals(true, elementList.checkHasStandardData());
        
        // payload's elementList entry 1 (Batch Item List)
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ARRAY, elementEntry.dataType());
        assertEquals(ElementNames.BATCH_ITEM_LIST.toString(), elementEntry.name().toString());
        // decode array from payload entry 1.
        Array array = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, array.primitiveType());
        assertEquals(0, array.itemLength());
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals("TRI.N", ae.encodedData().toString());
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals("IBM.N", ae.encodedData().toString());
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals("CSCO.O", ae.encodedData().toString());
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, ae.decode(dIter));
        
        // payload's elementList entry 2 (View Type)
        elementEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        assertEquals(ElementNames.VIEW_TYPE.toString(), elementEntry.name().toString());
        UInt uint = CodecFactory.createUInt();
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(1, uint.toLong());
        
        // payload's elementList entry 2 (View Data)
        elementEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ARRAY, elementEntry.dataType());
        assertEquals(ElementNames.VIEW_DATA.toString(), elementEntry.name().toString());
        array.clear();
        assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));
        assertEquals(DataTypes.UINT, array.primitiveType());
        assertEquals(2, array.itemLength());
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        uint.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(22, uint.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        uint.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(25, uint.toLong());
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, ae.decode(dIter));
        // check for end of element entry of payload.
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        // payload decoding complete.
    }
    
    /**
     * Test RsslRefreshMsg.decode with data from ETAC.
     */
    @Test
    public void decodeRefreshMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/002_refreashMsg.txt");
        assertNotNull(expected);
        
        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer etaBuffer = CodecFactory.createBuffer();
        etaBuffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        RefreshMsg msg = (RefreshMsg) CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.REFRESH, msg.msgClass());
        assertEquals(DomainTypes.MARKET_PRICE, msg.domainType());
        assertEquals(2147483647, msg.streamId());
        assertEquals(true, msg.checkClearCache());
        assertEquals(true, msg.checkDoNotCache());
        assertEquals(true, msg.checkRefreshComplete());
        assertEquals(true, msg.checkSolicited());
        assertEquals(false, msg.checkPrivateStream());
        // Data Format
        assertEquals(DataTypes.FIELD_LIST, msg.containerType());
        // verify sequence number
        assertEquals(true, msg.checkHasSeqNum());
        assertEquals(1234567890, msg.seqNum());
        // verify state
        assertEquals(StreamStates.OPEN, msg.state().streamState());
        assertEquals(DataStates.OK, msg.state().dataState());
        assertEquals(StateCodes.NONE, msg.state().code());
        assertEquals("some text info", msg.state().text().toString());
        // verify GroupId
        assertEquals("10203040", msg.groupId().toString());
        // verify Perm Expression
        assertEquals(true, msg.checkHasPermData());
        ByteBuffer buf = msg.permData().data();
        int position = msg.permData().position();
        assertEquals(0x10, buf.get(position++));
        assertEquals(0x11, buf.get(position++));
        assertEquals(0x12, buf.get(position++));
        assertEquals(0x13, buf.get(position++));
        // verify QoS
        assertEquals(true, msg.checkHasQos());
        assertNotNull(msg.qos());
        assertEquals(false, msg.qos().isDynamic());
        assertEquals(QosTimeliness.REALTIME, msg.qos().timeliness());
        assertEquals(QosRates.TICK_BY_TICK, msg.qos().rate());
        // verify Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        assertEquals("EXTENDED HEADER", msg.extendedHeader().toString());
        // verify post user info
        assertEquals(true, msg.checkHasPostUserInfo());
        PostUserInfo postUserInfo = msg.postUserInfo();
        assertEquals(4294967290L, postUserInfo.userAddr());
        assertEquals(4294967295L, postUserInfo.userId());
        // verify part number
        assertEquals(true, msg.checkHasPartNum());
        assertEquals(32767, msg.partNum());
		assertEquals(false, msg.isFinalMsg());

        // msgKey
        assertEquals(true, msg.checkHasMsgKey());
        MsgKey key = msg.msgKey();
        assertEquals(true, key.checkHasAttrib());
        assertEquals(false, key.checkHasFilter());
        assertEquals(false, key.checkHasIdentifier());
        assertEquals(true, key.checkHasName());
        assertEquals(true, key.checkHasNameType());
        assertEquals(true, key.checkHasServiceId());
        assertEquals(InstrumentNameTypes.RIC, key.nameType());
        assertEquals("TRI.N", key.name().toString());
        assertEquals(32639, key.serviceId());
        assertEquals(DataTypes.ELEMENT_LIST, key.attribContainerType());
        assertEquals(CodecReturnCodes.SUCCESS, msg.decodeKeyAttrib(dIter, key));
        
        // decode msgKey's Attrib (ElementList with three ElementEntries)
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        assertEquals(true, elementList.checkHasStandardData());
        assertEquals(false, elementList.checkHasInfo());
        // msgKey's elementList entry 1
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPID.toString(), elementEntry.name().toString());
        assertEquals("256", elementEntry.encodedData().toString());
        // msgKey's elementList entry 2        
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPNAME.toString(), elementEntry.name().toString());
        assertEquals("rsslConsumer", elementEntry.encodedData().toString());
        // msgKey's elementList entry 3
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.POSITION.toString(), elementEntry.name().toString());
        assertEquals("localhost", elementEntry.encodedData().toString());
        // msgKey's elementList END_OF_CONTAINER
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        
        // decode payload (FieldList with four entries.
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();
        UInt uint = CodecFactory.createUInt();
        Buffer buffer = CodecFactory.createBuffer();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasInfo());
        assertEquals(true, fieldList.checkHasStandardData());
        assertEquals(false, fieldList.checkHasSetData());
        assertEquals(false, fieldList.checkHasSetId());
        assertEquals(2, fieldList.dictionaryId());
        assertEquals(3, fieldList.fieldListNum());
        // decode field entry 1 (fieldId 10 encoded as a blank REAL).
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(10, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        // decode field entry 2 (fieldId 175 encoded as ASCII_STRING).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(175, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, buffer.decode(dIter));
        assertEquals("ABCDEFG", buffer.toString());
        // decode field entry 3 (fieldId 32 encoded as UINT).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(554433, uint.toLong());
        // decode field entry 4 (fieldId 111 emcpded as REAL).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(111, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(RealHints.EXPONENT_4, real.hint());
        assertEquals(867564, real.toLong());
        // decode end of container
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
        // payload decoding complete. 
    }
    
    /**
     * Test RsslUpdateMsg.decode with data from ETAC.
     */
    @Test
    public void decodeUpdateMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/004_updateMsg.txt");
        assertNotNull(expected);
        
        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer etaBuffer = CodecFactory.createBuffer();
        etaBuffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        UpdateMsg msg = (UpdateMsg) CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.UPDATE, msg.msgClass());
        assertEquals(DomainTypes.LOGIN, msg.domainType());
        assertEquals(2146290601, msg.streamId());
        assertEquals(true, msg.checkDoNotCache());
        assertEquals(true, msg.checkDoNotConflate());
        assertEquals(true, msg.checkDoNotRipple());
        assertEquals(false, msg.checkDiscardable());
        // Conflation information
        assertEquals(true, msg.checkHasConfInfo());
        assertEquals(10, msg.conflationCount());
        assertEquals(500, msg.conflationTime());
        // Data Format
        assertEquals(DataTypes.FIELD_LIST, msg.containerType());
        // verify sequence number
        assertEquals(true, msg.checkHasSeqNum());
        assertEquals(1234567890, msg.seqNum());
        // verify Perm Expression
        assertEquals(true, msg.checkHasPermData());
        ByteBuffer buf = msg.permData().data();
        int position = msg.permData().position();
        assertEquals(0x10, buf.get(position++));
        assertEquals(0x11, buf.get(position++));
        assertEquals(0x12, buf.get(position++));
        assertEquals(0x13, buf.get(position++));
        // verify Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        assertEquals("EXTENDED HEADER", msg.extendedHeader().toString());
        // verify post user info
        assertEquals(true, msg.checkHasPostUserInfo());
        PostUserInfo postUserInfo = msg.postUserInfo();
        assertEquals(4294967290L, postUserInfo.userAddr());
        assertEquals(4294967295L, postUserInfo.userId());
        assertEquals(false, msg.isFinalMsg());

        // msgKey
        assertEquals(true, msg.checkHasMsgKey());
        MsgKey key = msg.msgKey();
        assertEquals(true, key.checkHasAttrib());
        assertEquals(false, key.checkHasFilter());
        assertEquals(false, key.checkHasIdentifier());
        assertEquals(true, key.checkHasName());
        assertEquals(true, key.checkHasNameType());
        assertEquals(true, key.checkHasServiceId());
        assertEquals(InstrumentNameTypes.RIC, key.nameType());
        assertEquals("TRI.N", key.name().toString());
        assertEquals(32639, key.serviceId());
        assertEquals(DataTypes.ELEMENT_LIST, key.attribContainerType());
        assertEquals(CodecReturnCodes.SUCCESS, msg.decodeKeyAttrib(dIter, key));
        
        // decode msgKey's Attrib (ElementList with three ElementEntries)
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        assertEquals(true, elementList.checkHasStandardData());
        assertEquals(false, elementList.checkHasInfo());
        // msgKey's elementList entry 1
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPID.toString(), elementEntry.name().toString());
        assertEquals("256", elementEntry.encodedData().toString());
        // msgKey's elementList entry 2        
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPNAME.toString(), elementEntry.name().toString());
        assertEquals("rsslConsumer", elementEntry.encodedData().toString());
        // msgKey's elementList entry 3
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.POSITION.toString(), elementEntry.name().toString());
        assertEquals("localhost", elementEntry.encodedData().toString());
        // msgKey's elementList END_OF_CONTAINER
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        
        // decode payload (FieldList with four entries.
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();
        UInt uint = CodecFactory.createUInt();
        Buffer buffer = CodecFactory.createBuffer();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasInfo());
        assertEquals(true, fieldList.checkHasStandardData());
        assertEquals(false, fieldList.checkHasSetData());
        assertEquals(false, fieldList.checkHasSetId());
        assertEquals(2, fieldList.dictionaryId());
        assertEquals(3, fieldList.fieldListNum());
        // decode field entry 1 (fieldId 10 encoded as a blank REAL).
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(10, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        // decode field entry 2 (fieldId 175 encoded as ASCII_STRING).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(175, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, buffer.decode(dIter));
        assertEquals("ABCDEFG", buffer.toString());
        // decode field entry 3 (fieldId 32 encoded as UINT).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(554433, uint.toLong());
        // decode field entry 4 (fieldId 111 emcpded as REAL).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(111, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(RealHints.EXPONENT_4, real.hint());
        assertEquals(867564, real.toLong());
        // decode end of container
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
        // payload decoding complete. 
        
        printXml(msg);
    }
    
    /**
     * Test RsslStatusMsg.decode with data from ETAC.
     */
    @Test
    public void decodeStatusMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/005_statusMsg.txt");
        assertNotNull(expected);
        
        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        StatusMsg msg = (StatusMsg) CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.STATUS, msg.msgClass());
        assertEquals(DomainTypes.SOURCE, msg.domainType());
        assertEquals(24, msg.streamId());
        assertEquals(true, msg.checkClearCache());
        assertEquals(false, msg.checkPrivateStream());
        // Data Format
        assertEquals(DataTypes.NO_DATA, msg.containerType());
        // verify state
        assertEquals(true, msg.checkHasState());
        assertEquals(StreamStates.CLOSED_RECOVER, msg.state().streamState());
        assertEquals(DataStates.SUSPECT, msg.state().dataState());
        assertEquals(StateCodes.TOO_MANY_ITEMS, msg.state().code());
        assertEquals("encodeStateMsgTest", msg.state().text().toString());
        // verify GroupId
        assertEquals(true, msg.checkHasGroupId());
        ByteBuffer buf = msg.groupId().data();
        int position = msg.groupId().position();
        assertEquals(6, msg.groupId().length());
        assertEquals(0x0b, buf.get(position++));
        assertEquals(0x7b, buf.get(position++));
        assertEquals(0x08, buf.get(position++));
        assertEquals(0x03, buf.get(position++));
        assertEquals(0x4c, buf.get(position++));
        assertEquals(0x02, buf.get(position++));
        // verify Perm Expression
        assertEquals(true, msg.checkHasPermData());
        buf = msg.permData().data();
        position = msg.permData().position();
        assertEquals(4, msg.permData().length());
        assertEquals(0x0a, buf.get(position++));
        assertEquals(0x05, buf.get(position++));
        assertEquals(0x03, buf.get(position++));
        assertEquals(0x09, buf.get(position++));
        // verify Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        buf = msg.extendedHeader().data();
        position = msg.extendedHeader().position();
        assertEquals(11, msg.extendedHeader().length());
        assertEquals(0x43, buf.get(position++));
        assertEquals(0x01, buf.get(position++));
        assertEquals(0x02, buf.get(position++));
        assertEquals(0x03, buf.get(position++));
        assertEquals(0x04, buf.get(position++));
        assertEquals(0x05, buf.get(position++));
        assertEquals(0x06, buf.get(position++));
        assertEquals(0x07, buf.get(position++));
        assertEquals(0x08, buf.get(position++));
        assertEquals(0x09, buf.get(position++));
        assertEquals(0x43, buf.get(position++));
        // verify post user info
        assertEquals(true, msg.checkHasPostUserInfo());
        PostUserInfo postUserInfo = msg.postUserInfo();
        assertEquals(1234, postUserInfo.userAddr());
        assertEquals(567, postUserInfo.userId());
        assertEquals(true, msg.isFinalMsg());

        // msgKey
        assertEquals(true, msg.checkHasMsgKey());
        MsgKey key = msg.msgKey();
        assertEquals(false, key.checkHasAttrib());
        assertEquals(true, key.checkHasFilter());
        assertEquals(false, key.checkHasIdentifier());
        assertEquals(false, key.checkHasName());
        assertEquals(false, key.checkHasNameType());
        assertEquals(false, key.checkHasServiceId());
        assertEquals(7, key.filter());
        assertEquals(DataTypes.NO_DATA, key.attribContainerType());
    }

    /**
     * Test RsslDecoders.decodeFilterList().
     * <ol>
     * <li>Test for BUFFER_TOO_SMALL by setting curBufPos past ByteBuffer limit.
     * </li>
     * <li>Test for NO_DATA with empty ByteBuffer</li>
     * <li>Test for INCOMPLETE_DATA with a ByteBuffer that has less than three
     * bytes remaining (limit - pos)</li>
     * <li>Test for INCOMPLETE_DATA with the position read past the endBufPos.</li>
     * </ol>
     */
    @Test
    public void decodeFilterListTest()
    {
        ByteBuffer bb = ByteBuffer.allocate(0);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        // 1. Test for BUFFER_TOO_SMALL by setting curBufPos past ByteBuffer
        // limit.
        FilterList filterList = CodecFactory.createFilterList();
        ((DecodeIteratorImpl)dIter)._curBufPos = 1;
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, filterList.decode(dIter));
        ((DecodeIteratorImpl)dIter)._curBufPos = 0;

        // 2. Test for NO_DATA with empty ByteBuffer
        assertEquals(CodecReturnCodes.NO_DATA, filterList.decode(dIter));

        // 3. Test for INCOMPLETE_DATA with a ByteBuffer that has less than
        // three bytes remaining (limit - pos)
        bb = ByteBuffer.allocate(100);
        buffer.data(bb, 0, 1);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, filterList.decode(dIter));

        // 4. test for INCOMPLETE_DATA with the position read past the
        // endBufPos. Do this by setting totalCountHint flag. With three bytes
        // of data.
        bb.rewind();
        bb.put(0, (byte)FilterListFlags.HAS_TOTAL_COUNT_HINT); // flags
        buffer.data(bb, 0, 3);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, filterList.decode(dIter));
    }

    /**
     * Test RsslDecoders.decodeFilterEntry().
     * <ol>
     * <li>a complete successful decode with verification.</li>
     * <li>test for end of container.</li>
     * <li>cause BUFFER_TO_SMALL at Decoders.decodeFilterEntry().</li>
     * <li>cause INCOMPLETE_DATA at Decoders.decodeFilterEntry().</li>
     * </ol>
     */
    @Test
    public void decodeFilterEntryTest()
    {
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/015_filter_list_wEntries.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        FilterList filterList = CodecFactory.createFilterList();
        FilterEntry filterEntry = CodecFactory.createFilterEntry();

        // 1. a complete successful decode with verification.

        // Start decoding filterList
        assertEquals(CodecReturnCodes.SUCCESS, filterList.decode(dIter));
        assertEquals(FilterListFlags.HAS_PER_ENTRY_PERM_DATA
                | FilterListFlags.HAS_TOTAL_COUNT_HINT, filterList.flags()); //
        assertEquals(0x04, filterList.totalCountHint());

        // after decoding filterList, verify that the encodedEntries match what is
        // expected.
		
		// less the filterList header.
        byte[] expectedBytes = new byte[expected.length - 4]; 
        // less the filterList header.
		System.arraycopy(expected, 4, expectedBytes, 0, expected.length - 4); 
        byte[] actualBytes = new byte[filterList.encodedEntries().length()];
        filterList.encodedEntries().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // decode the filter entries

        // filter entry 1
        assertEquals(CodecReturnCodes.SUCCESS, filterEntry.decode(dIter));
        assertEquals(FilterEntryActions.SET, filterEntry.action());
        assertEquals(FilterEntryFlags.HAS_PERM_DATA | FilterEntryFlags.HAS_CONTAINER_TYPE,
                     filterEntry.flags());
        assertEquals(1, filterEntry.id());
        assertEquals(DataTypes.OPAQUE, filterEntry.containerType());
        // verify permData
        assertEquals(4, filterEntry.permData().length());
        expectedBytes = new byte[4];
        System.arraycopy(expected, 8, expectedBytes, 0, 4); // copy permData
        actualBytes = new byte[filterEntry.permData().length()];
        filterEntry.permData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);
        // verify encodedData
        assertEquals(11, filterEntry.encodedData().length());
        expectedBytes = new byte[11];
        System.arraycopy(expected, 13, expectedBytes, 0, 11); // copy encodedData
        actualBytes = new byte[filterEntry.encodedData().length()];
        filterEntry.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // filter entry 2
        filterEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, filterEntry.decode(dIter));
        assertEquals(FilterEntryActions.UPDATE, filterEntry.action());
        assertEquals(FilterEntryFlags.HAS_PERM_DATA, filterEntry.flags());
        assertEquals(10, filterEntry.id());
        assertEquals(DataTypes.XML, filterEntry.containerType());
        // verify permData
        assertEquals(2, filterEntry.permData().length());
        expectedBytes = new byte[2];
        System.arraycopy(expected, 27, expectedBytes, 0, 2); // copy permData
        actualBytes = new byte[filterEntry.permData().length()];
        filterEntry.permData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);
        // verify encodedData
        assertEquals(4, filterEntry.encodedData().length());
        expectedBytes = new byte[4];
        System.arraycopy(expected, 30, expectedBytes, 0, 4); // copy encodedData
        actualBytes = new byte[filterEntry.encodedData().length()];
        filterEntry.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // filter entry 3
        filterEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, filterEntry.decode(dIter));
        assertEquals(FilterEntryActions.CLEAR, filterEntry.action());
        assertEquals(FilterEntryFlags.NONE, filterEntry.flags());
        assertEquals(0x80, filterEntry.id());
        assertEquals(DataTypes.XML, filterEntry.containerType());
        // verify encodedData
        assertEquals(0, filterEntry.encodedData().length());

        // filter entry 4
        filterEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, filterEntry.decode(dIter));
        assertEquals(FilterEntryActions.SET, filterEntry.action());
        assertEquals(FilterEntryFlags.HAS_CONTAINER_TYPE, filterEntry.flags());
        assertEquals(0xff, filterEntry.id());
        assertEquals(DataTypes.OPAQUE, filterEntry.containerType());
        // verify encodedData
        assertEquals(10, filterEntry.encodedData().length());
        expectedBytes = new byte[10];
        System.arraycopy(expected, 40, expectedBytes, 0, 10); // copy encodedData
        actualBytes = new byte[filterEntry.encodedData().length()];
        filterEntry.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // 2. now that we read all of the items, test for end of container.
        filterEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, filterEntry.decode(dIter));

        // 3. cause BUFFER_TO_SMALL at Decoders.decodeFilterEntry().
        bb.rewind();
        bb.limit(4);
        buffer.data(bb, 0, 4);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        // Start decoding filterList
        assertEquals(CodecReturnCodes.SUCCESS, filterList.decode(dIter));
        assertEquals(FilterListFlags.HAS_PER_ENTRY_PERM_DATA
                | FilterListFlags.HAS_TOTAL_COUNT_HINT, filterList.flags()); //
        assertEquals(0x04, filterList.totalCountHint());
        bb.limit(3);
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, filterEntry.decode(dIter));

        // 4. cause INCOMPLETE_DATA at Decoders.decodeFilterEntry().
        bb.rewind();
        bb.limit(4);
        buffer.data(bb, 0, 4);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        // Start decoding filterList
        assertEquals(CodecReturnCodes.SUCCESS, filterList.decode(dIter));
        assertEquals(FilterListFlags.HAS_PER_ENTRY_PERM_DATA
                | FilterListFlags.HAS_TOTAL_COUNT_HINT, filterList.flags()); //
        assertEquals(0x04, filterList.totalCountHint());

        filterEntry.clear();
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, filterEntry.decode(dIter));
    }

    /**
     * Test Decoders decodeArray() and decodeArrayEntry()
     * <ol>
     * 
     * <li>Positive Case - decode item length and a valid primitive type. Verify encoded data.</li>
     * <li>Positive case - Array with encodedData, itemLength > 0. Verify encoded data. ArrayInit primitiveType=Buffer itemLength = 6.</li>
     * <li>Positive case - Array with encodedData, itemLength=0. Verify encoded data.</li>
     * <li>Positive case - ArrayEntry with itemLength > 0 and encodedData.length=0. Verify encoded data.</li>
     * <li>Positive case - ArrayEntry with itemLength > 0 and encodedData.length=0. Verify encoded data.</li>
     * <li>Positive case - Array populated with UInts. Verify encoded data.</li>
     * <li>Negative case - Test RsslDecoders.decodeArray() BLANK_DATA.</li>
     * <li>Negative case - Test RsslDecoders.decodeArray() INCOMPLETE_DATA</li
     * <li>Positive case - decode length specified UInts as RsslBuffers.</li>
     * <li>Test UNSUPPORTED_DATA_TYPE when RsslDecoders.decodeArrayEntry() calls decodePrimitiveType().</li>
     * </ol>
     */
    @Test
    public void decodeArrayAndEntriesTest()
    {
        // 1. Positive Case - decode item length and a valid primitive type. Verify encoded data.
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/008_array_entries_ascii.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        Array array = CodecFactory.createArray();
 //       RsslBuffer arrayEntryBuffer = RsslCodecFactory.createBuffer();

        assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, array.primitiveType());
        assertEquals(10, array.itemLength());

        // verify array 1
        ArrayEntry ae = CodecFactory.createArrayEntry();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(10, ae.encodedData().length());
        byte[] expectedBytes = new byte[10];
        System.arraycopy(expected, 4, expectedBytes, 0, 10); // copy data at idx=4
        byte[] actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // verify array 2
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(10, ae.encodedData().length());
        System.arraycopy(expected, 14, expectedBytes, 0, 10); // copy data at idx=14
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // verify array 3
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(10, ae.encodedData().length());
        System.arraycopy(expected, 24, expectedBytes, 0, 10); // copy data at idx=24
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // test for end of container
        ae.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER,
                     ae.decode(dIter));
        
        // 2. Positive case - Array with encodedData, itemLength > 0. Verify encoded data.
        //    ArrayInit primitiveType=Buffer itemLength = 6.
        expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/010_array_entries_encData_len2.txt");
        assertNotNull(expected);

        bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        array.clear();
        assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));
        assertEquals(DataTypes.BUFFER, array.primitiveType());
        assertEquals(5, array.itemLength());
        
        // verify array 1
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(5, ae.encodedData().length());
        expectedBytes = new byte[5];
        System.arraycopy(expected, 4, expectedBytes, 0, 5); // copy data at idx=4
        actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // verify array 2
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(5, ae.encodedData().length());
        expectedBytes = new byte[5];
        System.arraycopy(expected, 9, expectedBytes, 0, 5); // copy data at idx=9
        actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // verify array 3
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(5, ae.encodedData().length());
        expectedBytes = new byte[5];
        System.arraycopy(expected, 14, expectedBytes, 0, 5); // copy data at idx=14
        actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // test for end of container
        ae.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER,
                     ae.decode(dIter));
        
        // 2. Negative case - Test RsslDecoders.decodeArray() BLANK_DATA.
        bb.rewind();
        bb.limit(0);
        buffer.data(bb, 0, 0);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        array.clear();
        ae.clear();

        assertEquals(CodecReturnCodes.BLANK_DATA, array.decode(dIter));
        
        // 3. Negative case - Test RsslDecoders.decodeArray() INCOMPLETE_DATA
        bb.limit(2);
        buffer.data(bb, 0, 2);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, array.decode(dIter));

        // 4. Positive case - decode length specified UInts as RsslBuffers.
        expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/016_array_entries_ls_uint.txt");
        assertNotNull(expected);

        bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        array.clear();
        assertEquals(CodecReturnCodes.SUCCESS, array.decode(dIter));
        assertEquals(DataTypes.UINT, array.primitiveType());
        assertEquals(0, array.itemLength());

        // verify array 1
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(1, ae.encodedData().length());
        expectedBytes = new byte[1];
        System.arraycopy(expected, 5, expectedBytes, 0, 1); // copy data at idx=5
        actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);

        // verify array 2
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(1, ae.encodedData().length());
        expectedBytes = new byte[1];
        System.arraycopy(expected, 7, expectedBytes, 0, 1); // copy data at idx=7
        actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);
       
        // verify array 3
        ae.clear();
        assertEquals(CodecReturnCodes.SUCCESS, ae.decode(dIter));
        assertEquals(2, ae.encodedData().length());
        expectedBytes = new byte[2];
        System.arraycopy(expected, 9, expectedBytes, 0, 2); // copy data at idx=9
        actualBytes = new byte[ae.encodedData().length()];
        ae.encodedData().copy(actualBytes);
        assertArrayEquals(expectedBytes, actualBytes);
        
        // test for end of container
        ae.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, ae.decode(dIter));
    }
    
    /**
     * Test RsslDate.decode(), RsslTime.decode() and RsslDateTime.decode().
     * <ol>
     * <li>test blank time</li>
     * <li>test blank date</li>
     * <li>rwfTime: uint8 hours, uint8 minutes, uint8 seconds, uint16 milliseconds</li>
     * <li>rwfTime: uint8 hours, uint8 minutes, uint8 seconds</li>
     * <li>rwfTime: uint8 hours, uint8 minutes</li>
     * <li>rwfDate: uint8 day, uint8 month, uint16 year</li>
     * <li>rwfDateTime: rwfDate, rwfTime</li>
     * </ol>
     */
    @Test
    public void decodeDateTimeTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(0);
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        Time time = CodecFactory.createTime();
        Date date = CodecFactory.createDate();
        
        // test blank time
        buffer.data(bb, 0, 0);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, time.decode(dIter));
        
        // test blank date
        assertEquals(CodecReturnCodes.BLANK_DATA, date.decode(dIter));
        
        // rwfTime: uint8 hours, uint8 minutes, uint8 seconds, uint16 milliseconds
        byte[] expected = new byte[] { 0x09, 0x0f, 0x1e, 0x00, 0x2d };
        bb = ByteBuffer.wrap(expected);
        
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, time.decode(dIter));
        assertEquals(9, time.hour());
        assertEquals(15, time.minute());
        assertEquals(30, time.second());
        assertEquals(45, time.millisecond());
        
        // rwfTime: uint8 hours, uint8 minutes, uint8 seconds
        expected = new byte[] { 0x09, 0x0f, 0x1e };
        bb = ByteBuffer.wrap(expected);
        
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, time.decode(dIter));
        assertEquals(9, time.hour());
        assertEquals(15, time.minute());
        assertEquals(30, time.second());
        assertEquals(0, time.millisecond());
        
        // rwfTime: uint8 hours, uint8 minutes
        expected = new byte[] { 0x09, 0x0f };
        bb = ByteBuffer.wrap(expected);
        
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, time.decode(dIter));
        assertEquals(9, time.hour());
        assertEquals(15, time.minute());
        assertEquals(0, time.second());
        assertEquals(0, time.millisecond());
        
        // rwfDate: uint8 day, uint8 month, uint16 year
        expected = new byte[] { 0x17, 0x05, 0x07, (byte)0xb2};
        bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, date.decode(dIter));
        assertEquals(23, date.day());
        assertEquals(5, date.month());
        assertEquals(1970, date.year());
        
        // rwfDateTime: rwfDate, rwfTime
        expected = new byte[] { 0x17, 0x05, 0x07, (byte)0xb2, 0x09, 0x0f, 0x1e, 0x00, 0x2d };
        bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());

        DateTime dateTime = CodecFactory.createDateTime();
        assertEquals(CodecReturnCodes.SUCCESS, dateTime.decode(dIter));
        assertEquals(23, dateTime.day());
        assertEquals(5, dateTime.month());
        assertEquals(1970, dateTime.year());
        assertEquals(9, dateTime.hour());
        assertEquals(15, dateTime.minute());
        assertEquals(30, dateTime.second());
        assertEquals(45, dateTime.millisecond());
        
        // test set/get value from epoch
        DateTime dateTime2 = CodecFactory.createDateTime();
        dateTime.value(1398009952238L);
        assertEquals(20, dateTime.day());
        assertEquals(4, dateTime.month());
        assertEquals(2014, dateTime.year());
        assertEquals(16, dateTime.hour());
        assertEquals(05, dateTime.minute());
        assertEquals(52, dateTime.second());
        assertEquals(238, dateTime.millisecond());
        assertEquals(1398009952238L, dateTime.millisSinceEpoch());
        dateTime.copy(dateTime2);
        assertEquals(20, dateTime2.day());
        assertEquals(4, dateTime2.month());
        assertEquals(2014, dateTime2.year());
        assertEquals(16, dateTime2.hour());
        assertEquals(05, dateTime2.minute());
        assertEquals(52, dateTime2.second());
        assertEquals(238, dateTime2.millisecond());
        assertEquals(1398009952238L, dateTime2.millisSinceEpoch());
    }
    
    /**
     * Test RsslDecoders.decodeInt().
     * <ol>
     * <li>test blank data</li>
     * <li>test length of 1.</li>
     * <li>test length of 2, value = -1.</li>
     * <li>test length of 2, value = 32767.</li>
     * <li>test length of 3, value = -1.</li>
     * <li>test length of 4, value = 2147483647.</li>
     * <li>test length of 5, value = 549755813887L.</li>
     * <li>test length of 6, value = 140737488355327L.</li>
     * <li>test length of 7, value = 36028797018963967L.</li>
     * <li>test length of 8, value = -1L.</li>
     * <li>test length of 8, value = 9223372036854775807L.</li>
     * </ol>
     */
    @Test
    public void decodeIntTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(0);
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        Int Int = CodecFactory.createInt();
        
        // test blank data
        buffer.data(bb, 0, 0);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, Int.decode(dIter));
        
        // test length of 1.
        bb = ByteBuffer.allocate(8);
        bb.put((byte)0);
        bb.flip();
        buffer.data(bb, 0, 1);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(0, Int.toLong());
        
        // test length of 2, value = -1.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 2);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(-1, Int.toLong());
        
        // test length of 2, value = 32767.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 2);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(32767, Int.toLong());
        
        // test length of 3, value = -1.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 3);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(8388607, Int.toLong());
        
        // test length of 4, value = 2147483647.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 4);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(2147483647, Int.toLong());
        
        // test length of 5, value = 549755813887L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 5);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(549755813887L, Int.toLong());
        
        // test length of 6, value = 140737488355327L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 6);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(140737488355327L, Int.toLong());
        
        // test length of 7, value = 36028797018963967L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 7);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(36028797018963967L, Int.toLong());
        
        // test length of 8, value = -1L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 8);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(-1L, Int.toLong());
        
        // test length of 8, value = 9223372036854775807L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x7f);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 8);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Int.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Int.decode(dIter));
        assertEquals(9223372036854775807L, Int.toLong());
    }
    
    /**
     * Test RsslDecoders.decodeUInt().
     * <ol>
     * <li>test blank data</li>
     * <li>test length of 1.</li>
     * <li>test length of 2, value = 65535.</li>
     * <li>test length of 3, value = 16777215.</li>
     * <li>test length of 4, value = 4294967295L.</li>
     * <li>test length of 5, value = 1099511627775L.</li>
     * <li>test length of 6, value = 281474976710655L.</li>
     * <li>test length of 7, value = 72057594037927935L.</li>
     * <li>test length of 8, value = 18446744073709551615L.</li>
     * </ol>
     */
    @Test
    public void decodeUIntTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(0);
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        UInt uInt = CodecFactory.createUInt();
        
        // test blank data
        buffer.data(bb, 0, 0);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, uInt.decode(dIter));
        
        // test length of 1.
        bb = ByteBuffer.allocate(8);
        bb.put((byte)0);
        bb.flip();
        buffer.data(bb, 0, 1);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(0, uInt.toLong());
        
        // test length of 2, value = 65535.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 2);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(65535, uInt.toLong());
        
        // test length of 3, value = 16777215.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 3);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(16777215, uInt.toLong());
        
        // test length of 4, value = 4294967295L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 4);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(4294967295L, uInt.toLong());
        
        // test length of 5, value = 1099511627775L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 5);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(1099511627775L, uInt.toLong());
        
        // test length of 6, value = 281474976710655L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 6);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(281474976710655L, uInt.toLong());
        
        // test length of 7, value = 72057594037927935L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 7);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(72057594037927935L, uInt.toLong());

        // test length of 8, value = 18446744073709551615L.
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 8);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        uInt.clear();
        assertEquals(CodecReturnCodes.SUCCESS, uInt.decode(dIter));
        assertEquals(-1L, uInt.toLong());
        BigInteger bi = uInt.toBigInteger();
        assertEquals("18446744073709551615", bi.toString());
    }

    /**
     * Test RsslDecoders.decodeReal().
     * <ol>
     * <li>test blank data</li>
     * <li>test length of 0 (no data).</li>
     * <li>test length of 1 (hint only, but zero).</li>
     * <li>test length of 1 (hint only, isBlank set).</li>
     * <li>test length of 2 (hint + 1 byte magnitude).</li>
     * <li>test length of 2 (hint + 1 byte magnitude).</li>
     * <li>test length of 3 (hint + 2 byte magnitude).</li>
     * <li>test length of 4 (hint + 3 byte magnitude).</li>
     * <li>test length of 5 (hint + 4 byte magnitude).</li>
     * <li>test length of 6 (hint + 5 byte magnitude).</li>
     * <li>test length of 7 (hint + 6 byte magnitude).</li>
     * <li>test length of 8 (hint + 7 byte magnitude).</li>
     * <li>test length of 9 (hint + 8 byte magnitude).</li>
     * <li>test length of 9 (hint + 8 byte magnitude).</li>
     * </ol>
     */
    @Test
    public void decodeRealTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        Real real = CodecFactory.createReal();
        
        // test blank data
        ByteBuffer bb = ByteBuffer.allocate(0);
        buffer.data(bb, 0, 0);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        
        // test length of 0 (no data).
        bb = ByteBuffer.allocate(9);
        buffer.data(bb, 0, 0);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        
        // test length of 1 (hint only, but zero).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x00);
        bb.flip();
        buffer.data(bb, 0, 1);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        
        // test length of 1 (hint only, isBlank set).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)0x20); // isBlank bit set
        bb.flip();
        buffer.data(bb, 0, 1);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        
        // test length of 2 (hint + 1 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.EXPONENT_2);
        bb.put((byte)-1);
        bb.flip();
        buffer.data(bb, 0, 2);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.EXPONENT_2, real.hint());
        assertEquals(-1, real.toLong());
        
        // test length of 2 (hint + 1 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.EXPONENT_14);
        bb.put((byte)127);
        bb.flip();
        buffer.data(bb, 0, 2);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.EXPONENT_14, real.hint());
        assertEquals(127, real.toLong());
        
        // test length of 3 (hint + 2 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.EXPONENT0);
        bb.put((byte)0x7f); // 32767
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 3);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.EXPONENT0, real.hint());
        assertEquals(32767, real.toLong());
        
        // test length of 4 (hint + 3 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.EXPONENT2);
        bb.put((byte)0x7f); // 8355711
        bb.put((byte)0x7f);
        bb.put((byte)0x7f);
        bb.flip();
        buffer.data(bb, 0, 4);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.EXPONENT2, real.hint());
        assertEquals(8355711, real.toLong());
        
        // test length of 5 (hint + 4 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.EXPONENT_1);
        bb.put((byte)0x7f); // 2130837760
        bb.put((byte)0x02);
        bb.put((byte)0x01);
        bb.put((byte)0x00);
        bb.flip();
        buffer.data(bb, 0, 5);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.EXPONENT_1, real.hint());
        assertEquals(2130837760, real.toLong());
        
        // test length of 6 (hint + 5 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.FRACTION_256);
        bb.put((byte)0x01); // 4328719365L
        bb.put((byte)0x02);
        bb.put((byte)0x03);
        bb.put((byte)0x04);
        bb.put((byte)0x05);
        bb.flip();
        buffer.data(bb, 0, 6);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.FRACTION_256, real.hint());
        assertEquals(4328719365L, real.toLong());
        
        // test length of 7 (hint + 6 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.FRACTION_256);
        bb.put((byte)0x80); // -139637976727553L
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 7);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.FRACTION_256, real.hint());
        assertEquals(-139637976727553L, real.toLong());
        
        // test length of 8 (hint + 7 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.FRACTION_8);
        bb.put((byte)0x7f); // 36028797018963967L
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 8);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.FRACTION_8, real.hint());
        assertEquals(36028797018963967L, real.toLong());
        
        // test length of 9 (hint + 8 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.FRACTION_1);
        bb.put((byte)0x7f); // 9223372036854775807L
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 9);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.FRACTION_1, real.hint());
        assertEquals(9223372036854775807L, real.toLong());
        
        // test length of 9 (hint + 8 byte magnitude).
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put((byte)RealHints.FRACTION_1);
        bb.put((byte)0x80); // -9151314442816847873L
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.flip();
        buffer.data(bb, 0, 9);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        real.clear();
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(false, real.isBlank());
        assertEquals(RealHints.FRACTION_1, real.hint());
        assertEquals(-9151314442816847873L, real.toLong());
    }
    
    /**
     * Test RsslEnum.decode().
     * <ol>
     * <li>test blank data</li>
     * <li>test a one byte RsslEnum</li>
     * <li>test a two byte RsslEnum</li>
     * </ol>
     */
    
    @Test
    public void decodeEnumTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        Enum Enum = CodecFactory.createEnum();

        // test blank data
        ByteBuffer bb = ByteBuffer.allocate(0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, Enum.decode(dIter));
        assertEquals(0, Enum.toInt());

        // test a one byte RsslEnum
        bb = ByteBuffer.allocate(2);
        bb.put((byte)0x7f);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 1));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Enum.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Enum.decode(dIter));
        assertEquals(127, Enum.toInt());

        // test a two byte RsslEnum
        bb.rewind();
        bb.limit(2);
        bb.put((byte)0xff); // 65407
        bb.put((byte)0x7f);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 2));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Enum.clear();
        assertEquals(CodecReturnCodes.SUCCESS, Enum.decode(dIter));
        assertEquals(65407, Enum.toInt());
    }
    
    /**
     * Test RsslBuffer.decode().
     * <ol>
     * <li>test blank data</li>
     * <li>test with data</li>
     * </ol>
     */
    @Test
    public void decodeBufferTest()
    {
        Buffer etaBuffer = CodecFactory.createBuffer();
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();

        // test blank data
        ByteBuffer bb = ByteBuffer.allocate(0);
        assertEquals(CodecReturnCodes.SUCCESS, etaBuffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, buffer.decode(dIter));
        
        // test with data
        bb = ByteBuffer.allocate(2);
        bb.put((byte) 0x01);
        bb.put((byte) 0x02);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, etaBuffer.data(bb));
        dIter.clear();
        dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, buffer.decode(dIter));
        assertEquals(2, buffer.length());
        assertEquals(0, buffer.position());
        assertEquals(1, buffer.data().get(0));
        assertEquals(2, buffer.data().get(1));
        
    }
    
    /**
     * Test RsslMap.decode().
     * <ol>
     * <li>test no data.</li>
     * <li>test incomplete data.</li>
     * <li>test has_key_field_id with SUCCESS.</li>
     * <li>summary data with bytebuffer too small (BUFFER_TOO_SMALL).</li>
     * <li>summary data with RsslBuffer to small (INCOMPLETE_DATA).</li>
     * <li>test map with total hint count.</li>
     * </ol>
     */
    @Test
    public void decodeMapTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        Map map = CodecFactory.createMap();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();

        // test no data
        ByteBuffer bb = ByteBuffer.allocate(0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.NO_DATA, map.decode(dIter));
        
        // test incomplete data
        bb = ByteBuffer.allocate(20);
        bb.put(((byte)MapFlags.NONE));
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 1));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, map.decode(dIter));
        
        // test has_key_field_id with SUCCESS.
        bb.rewind();
        bb.limit(7);
        bb.put(((byte)MapFlags.HAS_KEY_FIELD_ID));
        bb.put((byte)DataTypes.INT);
        bb.put((byte)(DataTypes.FIELD_LIST - DataTypes.CONTAINER_TYPE_MIN));
        bb.put((byte)0x7f); // key field id = 32767
        bb.put((byte)0xff);
        bb.put((byte)0x00); // count
        bb.put((byte)0x01);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 7));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(true, map.checkHasKeyFieldId());
        assertEquals(false, map.checkHasPerEntryPermData());
        assertEquals(false, map.checkHasSetDefs());
        assertEquals(false, map.checkHasSummaryData());
        assertEquals(false, map.checkHasTotalCountHint());
        assertEquals(DataTypes.INT, map.keyPrimitiveType());
        assertEquals(DataTypes.FIELD_LIST, map.containerType());
        assertEquals(32767, map.keyFieldId());
        
        // summary data with bytebuffer too small (BUFFER_TOO_SMALL).
        bb.rewind();
        bb.limit(14);
        bb.put(0, (byte)(MapFlags.HAS_KEY_FIELD_ID | MapFlags.HAS_SUMMARY_DATA));
        bb.put(5, (byte)0x06); // summary data len = 6
        bb.put(6, (byte)0x01);
        bb.put(7, (byte)0x02);
        bb.put(8, (byte)0x03);
        bb.put(9, (byte)0x04);
        bb.put(10, (byte)0x05);
        bb.put(11, (byte)0x06);
        bb.put(12, (byte)0x00); // count
        bb.put(13, (byte)0x01);
        bb.limit(11); // set bytebuffer too small
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 11));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, map.decode(dIter));
        
        // summary data with RsslBuffer to small (INCOMPLETE_DATA).
        bb.rewind();
        bb.limit(14);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 11));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, map.decode(dIter));
        assertEquals(true, map.checkHasKeyFieldId());
        assertEquals(true, map.checkHasSummaryData());
        
        // test map with total hint count.
        bb.rewind();
        bb.limit(9);
        bb.put(((byte)MapFlags.HAS_TOTAL_COUNT_HINT));
        bb.put((byte)DataTypes.INT);
        bb.put((byte)(DataTypes.FIELD_LIST - DataTypes.CONTAINER_TYPE_MIN));
        bb.put((byte)0xff); // total hint count (uint30-rb) 0x3fffffff (encoded as 0xffffffff)
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0x00); // count
        bb.put((byte)0x01);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 9));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(false, map.checkHasKeyFieldId());
        assertEquals(false, map.checkHasPerEntryPermData());
        assertEquals(false, map.checkHasSetDefs());
        assertEquals(false, map.checkHasSummaryData());
        assertEquals(true, map.checkHasTotalCountHint());
        assertEquals(DataTypes.INT, map.keyPrimitiveType());
        assertEquals(DataTypes.FIELD_LIST, map.containerType());
        assertEquals(1073741823, map.totalCountHint());
        
    }
    
    /**
     * Test RsslMapEntry.decode().
     * <ol>
     * <li>Sucessfully decode a complete Map with Entries.</li>
     * <li>Test map entry with INCOMPLETE_DATA for encodedData.</li>
     * <li>Test map entry with BUFFER_TOO_SMALL for encodedData.</li>
     * <li>Test map entry with INCOMPLETE_DATA for encodedKey.</li>
     * <li>Test map entry with BUFFER_TOO_SMALL for encodedKey.</li>
     * <li>Test map entry with INCOMPLETE_DATA for encPerm.</li>
     * <li>Test map entry with BUFFER_TOO_SMALL for PermData.</li>
     * <li>Test map entry with INCOMPLETE_DATA at the beginning of the entry.</li>
     * <li>Test map entry with a key of dataType ASCII_STRING.</li>
     * <li>Test map entry with a key of a dataType UNSUPPORTED_DATA_TYPE</li>
     * </ol>
     */
    @Test
    public void decodeMapEntryTest()
    {
        
        // Sucessfully decode a complete Map with Entries.
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/017_map_wEntry.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        Map map = CodecFactory.createMap();
        MapEntry mapEntry = CodecFactory.createMapEntry();
        UInt keyUInt = CodecFactory.createUInt();
        
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(true, map.checkHasKeyFieldId());
        assertEquals(true, map.checkHasPerEntryPermData());
        assertEquals(true, map.checkHasSetDefs());
        assertEquals(true, map.checkHasSummaryData());
        assertEquals(true, map.checkHasTotalCountHint());
        assertEquals(DataTypes.UINT, map.keyPrimitiveType());
        assertEquals(DataTypes.FIELD_LIST, map.containerType());
        assertEquals(2, map.totalCountHint());
        assertEquals("000000001111111122222222", map.encodedSetDefs().toString());
        assertEquals("ABCDEFGHIJKLMNOPQRSTUVWXYZ", map.encodedSummaryData().toString());
        // decode map entry 1
        assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, keyUInt));
        assertEquals(5423839389739397169L, keyUInt.toLong()); 
        assertEquals(true, mapEntry.checkHasPermData());
        assertEquals(MapEntryActions.ADD, mapEntry.action());
        assertEquals("PERM", mapEntry.permData().toString());
        assertEquals("KEY00001", mapEntry.encodedKey().toString());
        assertEquals("ZYXWVUTSRQPONMLKJIHGFEDCBA", mapEntry.encodedData().toString());
        // decode map entry 2
        mapEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, keyUInt));
        assertEquals(19250, keyUInt.toLong()); 
        assertEquals(false, mapEntry.checkHasPermData());
        assertEquals("K2", mapEntry.encodedKey().toString());
        assertEquals("AAAAAAAAAAAAAAAAAAAAAAAAAA", mapEntry.encodedData().toString());
        // decode map entry 3
        mapEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, keyUInt));
        assertEquals(19251, keyUInt.toLong());
        assertEquals(false, mapEntry.checkHasPermData());
        assertEquals("K3", mapEntry.encodedKey().toString());
        assertEquals(0, mapEntry.encodedData().length());
        // decode end of container
        mapEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, mapEntry.decode(dIter, keyUInt));

        // Test map entry with INCOMPLETE_DATA for encodedData.
        bb.rewind();
        bb.limit(bb.capacity());
        buffer.data(bb, 0 , 77);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));
         
        // Test map entry with BUFFER_TOO_SMALL for encodedData.
        bb.rewind();
        bb.limit(77);
        buffer.data(bb, 0 , 77);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));
         
        // Test map entry with INCOMPLETE_DATA for encodedKey.
        bb.rewind();
        bb.limit(bb.capacity());
        buffer.data(bb, 0 , 68);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));
         
        // Test map entry with BUFFER_TOO_SMALL for encodedKey.
        bb.rewind();
        bb.limit(68);
        buffer.data(bb, 0 , 68);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));
         
        // Test map entry with INCOMPLETE_DATA for permData
        bb.rewind();
        bb.limit(bb.capacity());
        buffer.data(bb, 0 , 63);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));
         
        // Test map entry with BUFFER_TOO_SMALL for PermData.
        bb.rewind();
        bb.limit(63);
        buffer.data(bb, 0 , 63);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));
         
        
        // Test map entry with INCOMPLETE_DATA at the beginning of the entry.
        bb.rewind();
        bb.limit(60);
        buffer.data(bb, 0 , 60);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, mapEntry.decode(dIter, keyUInt));

        // Test map entry with a key of dataType ASCII_STRING.
        Buffer keyBuffer = CodecFactory.createBuffer();
        keyBuffer.data(ByteBuffer.allocate(26));
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put(1, (byte)DataTypes.ASCII_STRING); // change data type to ASCII_STRING
        buffer.data(bb);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, map.keyPrimitiveType());
        assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, keyBuffer));
        assertEquals("KEY00001", keyBuffer.toString());
        
        // Test map entry with a key of a dataType INT
        Int keyInt = CodecFactory.createInt();
        keyBuffer.data().rewind();
        keyBuffer.data().limit(keyBuffer.data().capacity());
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put(1, (byte)DataTypes.INT); // change data type to INT
        buffer.data(bb);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(DataTypes.INT, map.keyPrimitiveType());
        assertEquals(CodecReturnCodes.SUCCESS, mapEntry.decode(dIter, keyInt));

        // Test map entry with a key of a dataType UNSUPPORTED_DATA_TYPE
        keyBuffer.data().rewind();
        keyBuffer.data().limit(keyBuffer.data().capacity());
        bb.rewind();
        bb.limit(bb.capacity());
        bb.put(1, (byte)DataTypes.UNKNOWN); // change data type to UNKNOWN
        buffer.data(bb);
        map.clear();
        mapEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, map.decode(dIter));
        assertEquals(DataTypes.UNKNOWN, map.keyPrimitiveType());
        assertEquals(CodecReturnCodes.UNSUPPORTED_DATA_TYPE, mapEntry.decode(dIter, keyBuffer));
        
    }
    
    /**
     * Test RsslElementList.decode().
     * <ol>
     * <li>test blank data</li>
     * <li>test length of 0 (no data).</li>
     * <li>test INCOMPLETE_DATA with info flag, but no info data.</li>
     * <li>test INCOMPLETE_DATA with info flag and infoLen, but no info data.</li>
     * <li>test BUFFER_TO_SMALL with info flag, info data, but info len is too big.</li>
     * <li>test with no standard data.</li>
     * </ol>
     */
    @Test
    public void decodeElementListTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        ElementList elementList = CodecFactory.createElementList();
        
        // test blank data
        ByteBuffer bb = ByteBuffer.allocate(0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.NO_DATA, elementList.decode(dIter, null)); // no local set def       

        // test length of 0 (no data).
        bb = ByteBuffer.allocate(9);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        elementList.clear();
        assertEquals(CodecReturnCodes.NO_DATA, elementList.decode(dIter, null));

        // test INCOMPLETE_DATA with info flag, but no info data.
        bb.rewind();
        bb.put((byte)ElementListFlags.HAS_ELEMENT_LIST_INFO);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 1));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        elementList.clear();
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementList.decode(dIter, null));
        
        // test INCOMPLETE_DATA with info flag and infoLen, but no info data.
        bb.rewind();
        bb.limit(2);
        bb.put((byte)ElementListFlags.HAS_ELEMENT_LIST_INFO);
        bb.put((byte)2);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 2));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        elementList.clear();
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementList.decode(dIter, null));
        
        // test BUFFER_TO_SMALL with info flag, info data, but info len is too big.
        bb.rewind();
        bb.limit(5);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 5));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        bb.put((byte)ElementListFlags.HAS_ELEMENT_LIST_INFO);
        bb.put((byte)3); // infoLen incorrectly 3.
        bb.put((byte)0); // elementListNum = 1;
        bb.put((byte)1);
        elementList.clear();
        bb.limit(4); // adjust the limit to cause BUFFER_TO_SMALL
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementList.decode(dIter, null));
        
        // test with no standard data.
        bb.rewind();
        bb.put((byte)0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 1));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        elementList.clear();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
    }

    /**
     * Test RsslElementEntry.decode() and verify a complete element list decoding with decoded data from ETAC.
     * <ol>
     * <li>Test elementEntry BUFFER_TOO_SMALL</li>
     * <li>test elementEntry INCOMPLETE_DATA (the entries length is larger than the bytebuffer).</li>
     * <li>test elementEntry INCOMPLETE_DATA (after reading the entry data, the position is past the _endBufPos).</li>
     * <li>test elementEntry name too big for buffer (BUFFER_TO_SMALL).</li>
     * <li>test elementEntry data too big for buffer (BUFFER_TO_SMALL).</li>
     * <li>test entry data type of NO_DATA.</li>
     * <li>test and verify decoding of a complete elementList with three entries (UINT, ELEMENT_LIST, UINT)</li>
     * </ol> 
     */
    @Test
    public void decodeElementEntryTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        
        // Test elementEntry BUFFER_TOO_SMALL
        ByteBuffer bb = ByteBuffer.allocate(10);
        bb.put((byte)ElementListFlags.HAS_STANDARD_DATA);
        bb.put((byte)0);
        bb.put((byte)1);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 3));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        bb.limit(2); // cause BUFFER_TOO_SMALL
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementEntry.decode(dIter));
       
        // test elementEntry INCOMPLETE_DATA (the entries length is larger than the bytebuffer).
        bb.limit(4);
        bb.put(3,(byte)0x09); // first element's name's length
        bb.rewind();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 4));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementEntry.decode(dIter));
        
        // test elementEntry INCOMPLETE_DATA (after reading the entry data, the position is past the _endBufPos).
        bb.rewind();
        bb.limit(9);
        bb.put(3, (byte)0x01); // name length of 1.
        bb.put(4, (byte)0x41); // 'A';
        bb.put(5, (byte)DataTypes.ASCII_STRING); // data Type
        bb.put(6, (byte)0x02); // data length of 2.
        bb.put(7, (byte)0x41); // 'A'
        bb.put(8, (byte)0x42); // 'B'
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementEntry.decode(dIter));
        
        // test entry data type of NO_DATA.
        bb.rewind();
        bb.limit(6);
        bb.put(3, (byte)0x01); // name length of 1.
        bb.put(4, (byte)0x41); // 'A';
        bb.put(5, (byte)DataTypes.NO_DATA); // data Type
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 6));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        
        
        //test elementEntry name too big for buffer (BUFFER_TO_SMALL).</li>
        bb.rewind();
        bb.limit(7);
        bb.put(3, (byte)0x04); // name length of 4.
        bb.put(4, (byte)0x41); // 'A';
        bb.put(5, (byte)0x42); // 'D';
        bb.put(6, (byte)0x43); // 'C';
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 7));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementEntry.decode(dIter));
        
        //test elementEntry data too big for buffer (BUFFER_TO_SMALL).</li>
        bb.rewind();
        bb.limit(9);
        bb.put(3, (byte)0x01); // name length of 1.
        bb.put(4, (byte)0x41); // 'A';
        bb.put(5, (byte)DataTypes.ASCII_STRING); // data Type
        bb.put(6, (byte)0x03); // data length of 3.
        bb.put(7, (byte)0x41); // 'A'
        bb.put(8, (byte)0x42); // 'B'
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 9));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, elementEntry.decode(dIter));
        
        // test and verify decoding of a complete elementList with three entries (UINT, ELEMENT_LIST, UINT)
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/004_elementList_wthrreEntries_middleEntryHasElementList.txt");
        assertNotNull(expected);

        bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null)); // no local set def
        assertEquals(true, elementList.checkHasStandardData());
        assertEquals(false, elementList.checkHasInfo());
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        // The first entry should be UINT value=1234
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        DecodeIterator subIter = CodecFactory.createDecodeIterator();
        UInt uint = CodecFactory.createUInt();
        subIter.setBufferAndRWFVersion(elementEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(subIter));
        assertEquals(1234, uint.toLong());
        // The second entry should be an ElementList containing ASCII_STRING value="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        elementEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ELEMENT_LIST, elementEntry.dataType());
        subIter.clear();
        subIter.setBufferAndRWFVersion(elementEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        ElementList subElementList = CodecFactory.createElementList();
        assertEquals(CodecReturnCodes.SUCCESS, subElementList.decode(subIter, null));
        assertEquals(true, subElementList.checkHasStandardData());
        assertEquals(false, subElementList.checkHasInfo());
        assertEquals(false, subElementList.checkHasSetData());
        assertEquals(false, subElementList.checkHasSetId());
        // the subElementEntry should contain an ASCII_STRING value="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        ElementEntry subElementEntry = CodecFactory.createElementEntry();
        assertEquals(CodecReturnCodes.SUCCESS, subElementEntry.decode(subIter));
        assertEquals(DataTypes.ASCII_STRING, subElementEntry.dataType());
        assertEquals("ABCDEFGHIJKLMNOPQRSTUVWXYZ", subElementEntry.encodedData().toString());
        // The third entry should be a UINT value=987654321
        elementEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.UINT, elementEntry.dataType());
        uint.clear();
        subIter.clear();
        subIter.setBufferAndRWFVersion(elementEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(subIter));
        assertEquals(987654321, uint.toLong());
        // Test for end of cotainer
        elementEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
    }

    /**
     * Test RsslFieldList.decode().
     * <ol>
     * <li>Test fieldList NO_DATA</li>
     * <li>Test fieldList with FIELD_LIST_INFO</li>
     * <li>Test fieldList with FIELD_LIST_INFO with infoLen too big (INCOMPLETE_DATA)</li>
     * <li>Test fieldList with FIELD_LIST_INFO with infoLen too big, RsslBu8ffer has enough room, but ByteBuffer does not. (BUFFER_TO_SMALL).</li>
     * </ol>
     */
    @Test
    public void decodeFieldListTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        FieldList fieldList = CodecFactory.createFieldList();

        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/006_encodeFieldList_wEntries_andRollBack.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());

        // Test fieldList NO_DATA
        bb.limit(0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.NO_DATA, fieldList.decode(dIter, null));

        // Test fieldList with FIELD_LIST_INFO
        bb.limit(5);
        bb.rewind();
        bb.put((byte)FieldListFlags.HAS_FIELD_LIST_INFO);
        bb.put((byte)0x03); // infoLength = 3
        bb.put((byte)0x01); // dictionaryID=1
        bb.put((byte)0x03); // field list number 772 (0x0304)
        bb.put((byte)0x04); //
        bb.flip();
        buffer.data(bb, 0, 5);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(false, fieldList.checkHasSetData());
        assertEquals(false, fieldList.checkHasSetId());
        assertEquals(false, fieldList.checkHasStandardData());
        assertEquals(1, fieldList.dictionaryId());
        assertEquals(772, fieldList.fieldListNum());

        // Test fieldList with FIELD_LIST_INFO with infoLen too big (INCOMPLETE_DATA)
        bb.rewind();
        bb.put(1, (byte)0x05); // change infoLength to 5
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, fieldList.decode(dIter, null));

        // Test fieldList with FIELD_LIST_INFO with infoLen too big, RsslBu8ffer
        // has enough room, but ByteBuffer does not. (BUFFER_TO_SMALL).
        bb.limit(7);
        bb.rewind();
        buffer.data(bb, 0, 7);
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        // set the limit so that the INFO will be read, but then infoLen will
        // put the bytebuffer over the limit.
        bb.limit(5);
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, fieldList.decode(dIter, null));
    }
    
    /**
     * Test RsslFieldEntry.decode() and verify a complete element list decoding with decoded data from ETAC..
     * <ol>
     * <li>Test fieldList with STANDARD_DATA and INFO, verify data with ETAC.</li>
     * <li>Test fieldEntry with field data that is longer than the ByteBuffer.</li>
     * <li>Test fieldEntry with field data that is longer than the RsslBuffer.</li>
     * </ol>
     */
    @Test
    public void decodeFieldEntryTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        DecodeIterator dSubIter = CodecFactory.createDecodeIterator();
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        UInt uint = CodecFactory.createUInt();
        Real real = CodecFactory.createReal();
        
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/006_encodeFieldList_wEntries_andRollBack.txt");
        assertNotNull(expected);

        ByteBuffer bb = ByteBuffer.wrap(expected);
        buffer.data(bb, 0, bb.limit() - bb.position());
        
        // Test fieldList with STANDARD_DATA and INFO
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasStandardData());
        assertEquals(true, fieldList.checkHasInfo());
        assertEquals(false, fieldList.checkHasSetData());
        assertEquals(false, fieldList.checkHasSetId());
        assertEquals(2, fieldList.dictionaryId());
        assertEquals(3, fieldList.fieldListNum());
        // decode field entry fieldId=10, REAL Blank.
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(10, fieldEntry.fieldId());
        dSubIter.clear();
        dSubIter.setBufferAndRWFVersion(fieldEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, uint.decode(dSubIter));
        assertEquals(0, uint.toLong());
        // decode field entry fieldId=175, pre-encoded data (ABCDEFG)
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(175, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals("ABCDEFG", fieldEntry.encodedData().toString()); // encodedData as a string.
        // decode field entry fieldId=32 UInt 554433
        fieldEntry.clear();
        uint.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        dSubIter.clear();
        dSubIter.setBufferAndRWFVersion(fieldEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dSubIter));
        assertEquals(554433, uint.toLong());
        //decode field entry fieldId=111 REAL value=867564 hint=EXPONENT_4.
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(111, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        dSubIter.clear();
        dSubIter.setBufferAndRWFVersion(fieldEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dSubIter));
        assertEquals(false, real.isBlank());
        assertEquals(867564, real.toLong());
        assertEquals(RealHints.EXPONENT_4, real.hint());
        // decode field entry fieldId=54 REAL BLANK
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(54, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        dSubIter.clear();
        dSubIter.setBufferAndRWFVersion(fieldEntry.encodedData(), Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dSubIter));
        assertEquals(true, real.isBlank());
        // End of container
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
        
        // Test fieldEntry with field data that is longer than the ByteBuffer.
        bb = ByteBuffer.allocate(10);
        bb.put((byte)FieldListFlags.HAS_STANDARD_DATA); // flag
        bb.put((byte)0x00); // count = 1 (0X0001)
        bb.put((byte)0x01);
        bb.put((byte)0x02); // entry fieldId 515 (0x0203)
        bb.put((byte)0x03);
        bb.put((byte)0x04); // entry length = 4
        bb.put((byte)0x7f); // entry data (len=1)
        bb.flip();
        // Test fieldEntry with field data that is longer than the RsslBuffer
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasStandardData());
        // decode field entry fieldId=515, len of 4, but ByteBuffer has only one remaining byte, (BUFFER_TO_SMALL)
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, fieldEntry.decode(dIter));
        
        // Test fieldEntry with field data that is longer than the RsslBuffer.
        bb.limit(10); // add three bytes to this bytebuffer
        bb.put(6, (byte) 0x7e);
        bb.put(7, (byte) 0x7d);
        bb.put(8, (byte) 0x7c);
        bb.rewind();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 9));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasStandardData());
        // decode field entry fieldId=515, len of 4, but RsslBuffer has only one remaining byte, (INCOMPLETE_DATA)
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, fieldEntry.decode(dIter));

    }
    
    /**
     * Test RsslSeries.decode().
     * <ol>
     * <li>test no data.</li>
     * <li>test incomplete data.</li>
     * <li>test has_key_field_id with SUCCESS.</li>
     * <li>summary data with bytebuffer too small (BUFFER_TOO_SMALL).</li>
     * <li>summary data with RsslBuffer too small (INCOMPLETE_DATA).</li>
     * <li>set defs with bytebuffer too small (BUFFER_TOO_SMALL).</li>
     * <li>set defs with RsslBuffer too small (INCOMPLETE_DATA).</li>
     * <li>test series with total hint count.</li>
     * </ol>
     */
    @Test
    public void decodeSeriesTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        Series series = CodecFactory.createSeries();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();

        // test no data
        ByteBuffer bb = ByteBuffer.allocate(0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.NO_DATA, series.decode(dIter));
        
        // test incomplete data
        bb = ByteBuffer.allocate(20);
        bb.put(((byte)SeriesFlags.NONE));
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 1));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, series.decode(dIter));
        
        // test minimal data.
        bb.rewind();
        bb.limit(4);
        bb.put(((byte)SeriesFlags.NONE));
        bb.put((byte)(DataTypes.FIELD_LIST - DataTypes.CONTAINER_TYPE_MIN));
        bb.put((byte)0x00); // count
        bb.put((byte)0x01);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 4));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(false, series.checkHasSetDefs());
        assertEquals(false, series.checkHasSummaryData());
        assertEquals(false, series.checkHasTotalCountHint());
        assertEquals(DataTypes.FIELD_LIST, series.containerType());
        
        // summary data with bytebuffer too small (BUFFER_TOO_SMALL).
        bb.rewind();
        bb.limit(11);
        bb.put(0, (byte)(SeriesFlags.HAS_SUMMARY_DATA));
        bb.put(2, (byte)0x06); // summary data len = 6
        bb.put(3, (byte)0x01);
        bb.put(4, (byte)0x02);
        bb.put(5, (byte)0x03);
        bb.put(6, (byte)0x04);
        bb.put(7, (byte)0x05);
        bb.put(8, (byte)0x06);
        bb.put(9, (byte)0x00); // count
        bb.put(10, (byte)0x01);
        bb.limit(8); // set bytebuffer too small
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, series.decode(dIter));
        
        // summary data with RsslBuffer too small (INCOMPLETE_DATA).
        bb.rewind();
        bb.limit(11);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, series.decode(dIter));
        assertEquals(true, series.checkHasSummaryData());

        // set defs with bytebuffer too small (BUFFER_TOO_SMALL).
        bb.rewind();
        bb.limit(11);
        bb.put(0, (byte)(SeriesFlags.HAS_SET_DEFS));
        bb.put(2, (byte)0x06); // set data len = 6
        bb.put(3, (byte)0x01);
        bb.put(4, (byte)0x02);
        bb.put(5, (byte)0x03);
        bb.put(6, (byte)0x04);
        bb.put(7, (byte)0x05);
        bb.put(8, (byte)0x06);
        bb.put(9, (byte)0x00); // count
        bb.put(10, (byte)0x01);
        bb.limit(8); // set bytebuffer too small
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, series.decode(dIter));
        
        // set defs with RsslBuffer too small (INCOMPLETE_DATA).
        bb.rewind();
        bb.limit(11);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, series.decode(dIter));
        assertEquals(true, series.checkHasSetDefs());

        // test series with total hint count.
        bb.rewind();
        bb.limit(8);
        bb.put(((byte)SeriesFlags.HAS_TOTAL_COUNT_HINT));
        bb.put((byte)(DataTypes.FIELD_LIST - DataTypes.CONTAINER_TYPE_MIN));
        bb.put((byte)0xff); // total hint count (uint30-rb) 0x3fffffff (encoded as 0xffffffff)
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0x00); // count
        bb.put((byte)0x01);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(false, series.checkHasSetDefs());
        assertEquals(false, series.checkHasSummaryData());
        assertEquals(true, series.checkHasTotalCountHint());
        assertEquals(DataTypes.FIELD_LIST, series.containerType());
        assertEquals(1073741823, series.totalCountHint());
    }
    
    /**
     * Test RsslSeriesEntry.decode().
     * <ol>
     * <li>Sucessfully decode a complete Series with Entries.</li>
     * <li>Test series entry with INCOMPLETE_DATA for encodedData.</li>
     * <li>Test series entry with BUFFER_TOO_SMALL for encodedData.</li>
     * <li>Test series with container type of NO_DATA.</li>
     * </ol>
     */
    @Test
    public void decodeSeriesEntryTest()
    {
        // Sucessfully decode a complete Series with Entries.
        byte[] expected = {0x07, 0x04, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x32,
        		0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x1a, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
        		0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x02, 0x00, 0x03, 0x1a, 0x5a, 0x59,
        		0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45,
        		0x44, 0x43, 0x42, 0x41, 0x1a, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
        		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, (byte)0xfe, 0x00, 0x00};

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        Series series = CodecFactory.createSeries();
        SeriesEntry seriesEntry = CodecFactory.createSeriesEntry();
        
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(true, series.checkHasSetDefs());
        assertEquals(true, series.checkHasSummaryData());
        assertEquals(true, series.checkHasTotalCountHint());
        assertEquals(DataTypes.FIELD_LIST, series.containerType());
        assertEquals(2, series.totalCountHint());
        assertEquals("000000001111111122222222", series.encodedSetDefs().toString());
        assertEquals("ABCDEFGHIJKLMNOPQRSTUVWXYZ", series.encodedSummaryData().toString());
        // decode series entry 1
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
        assertEquals("ZYXWVUTSRQPONMLKJIHGFEDCBA", seriesEntry.encodedData().toString());
        // decode series entry 2
        seriesEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
        assertEquals("AAAAAAAAAAAAAAAAAAAAAAAAAA", seriesEntry.encodedData().toString());
        // decode series entry 3
        seriesEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
        assertEquals(0, seriesEntry.encodedData().length());
        // decode end of container
        seriesEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, seriesEntry.decode(dIter));

        // Test series entry with INCOMPLETE_DATA for encodedData.
        bb.rewind();
        bb.limit(bb.capacity());
        buffer.data(bb, 0 , 77);
        series.clear();
        seriesEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, seriesEntry.decode(dIter));
         
        // Test series entry with BUFFER_TOO_SMALL for encodedData.
        bb.rewind();
        bb.limit(77);
        buffer.data(bb, 0 , 77);
        series.clear();
        seriesEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, seriesEntry.decode(dIter));
        
        // Test series with container type of NO_DATA
        byte[] expected2 = {0x07, 0x00, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x1a, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
        0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x02, 0x00, 0x03};
        
        bb = ByteBuffer.wrap(expected2);
        buffer = CodecFactory.createBuffer();
        buffer.data(bb);
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));

        assertEquals(CodecReturnCodes.SUCCESS, series.decode(dIter));
        assertEquals(true, series.checkHasSetDefs());
        assertEquals(true, series.checkHasSummaryData());
        assertEquals(true, series.checkHasTotalCountHint());
        assertEquals(DataTypes.NO_DATA, series.containerType());
        assertEquals(2, series.totalCountHint());
        assertEquals("000000001111111122222222", series.encodedSetDefs().toString());
        assertEquals("ABCDEFGHIJKLMNOPQRSTUVWXYZ", series.encodedSummaryData().toString());
        assertEquals(CodecReturnCodes.SUCCESS, seriesEntry.decode(dIter));
    }
    
    /**
     * Test RsslVector.decode().
     * <ol>
     * <li>test no data.</li>
     * <li>test incomplete data.</li>
     * <li>test has_key_field_id with SUCCESS.</li>
     * <li>summary data with bytebuffer too small (BUFFER_TOO_SMALL).</li>
     * <li>summary data with RsslBuffer too small (INCOMPLETE_DATA).</li>
     * <li>set defs with bytebuffer too small (BUFFER_TOO_SMALL).</li>
     * <li>set defs with RsslBuffer too small (INCOMPLETE_DATA).</li>
     * <li>test vector with total hint count.</li>
     * </ol>
     */
    @Test
    public void decodeVectorTest()
    {
        Buffer buffer = CodecFactory.createBuffer();
        Vector vector = CodecFactory.createVector();
        DecodeIterator dIter = CodecFactory.createDecodeIterator();

        // test no data
        ByteBuffer bb = ByteBuffer.allocate(0);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 0));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.NO_DATA, vector.decode(dIter));
        
        // test incomplete data
        bb = ByteBuffer.allocate(20);
        bb.put(((byte)VectorFlags.NONE));
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 1));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vector.decode(dIter));
        
        // test minimal data.
        bb.rewind();
        bb.limit(4);
        bb.put(((byte)VectorFlags.NONE));
        bb.put((byte)(DataTypes.FIELD_LIST - DataTypes.CONTAINER_TYPE_MIN));
        bb.put((byte)0x00); // count
        bb.put((byte)0x01);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 4));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(false, vector.checkHasSetDefs());
        assertEquals(false, vector.checkHasSummaryData());
        assertEquals(false, vector.checkHasTotalCountHint());
        assertEquals(DataTypes.FIELD_LIST, vector.containerType());
        
        // summary data with bytebuffer too small (BUFFER_TOO_SMALL).
        bb.rewind();
        bb.limit(11);
        bb.put(0, (byte)(VectorFlags.HAS_SUMMARY_DATA));
        bb.put(2, (byte)0x06); // summary data len = 6
        bb.put(3, (byte)0x01);
        bb.put(4, (byte)0x02);
        bb.put(5, (byte)0x03);
        bb.put(6, (byte)0x04);
        bb.put(7, (byte)0x05);
        bb.put(8, (byte)0x06);
        bb.put(9, (byte)0x00); // count
        bb.put(10, (byte)0x01);
        bb.limit(8); // set bytebuffer too small
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vector.decode(dIter));
        
        // summary data with RsslBuffer too small (INCOMPLETE_DATA).
        bb.rewind();
        bb.limit(11);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vector.decode(dIter));
        assertEquals(true, vector.checkHasSummaryData());

        // set defs with bytebuffer too small (BUFFER_TOO_SMALL).
        bb.rewind();
        bb.limit(11);
        bb.put(0, (byte)(VectorFlags.HAS_SET_DEFS));
        bb.put(2, (byte)0x06); // set data len = 6
        bb.put(3, (byte)0x01);
        bb.put(4, (byte)0x02);
        bb.put(5, (byte)0x03);
        bb.put(6, (byte)0x04);
        bb.put(7, (byte)0x05);
        bb.put(8, (byte)0x06);
        bb.put(9, (byte)0x00); // count
        bb.put(10, (byte)0x01);
        bb.limit(8); // set bytebuffer too small
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vector.decode(dIter));
        
        // set defs with RsslBuffer too small (INCOMPLETE_DATA).
        bb.rewind();
        bb.limit(11);
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vector.decode(dIter));
        assertEquals(true, vector.checkHasSetDefs());

        // test vector with total hint count.
        bb.rewind();
        bb.limit(8);
        bb.put(((byte)VectorFlags.HAS_TOTAL_COUNT_HINT));
        bb.put((byte)(DataTypes.FIELD_LIST - DataTypes.CONTAINER_TYPE_MIN));
        bb.put((byte)0xff); // total hint count (uint30-rb) 0x3fffffff (encoded as 0xffffffff)
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0xff);
        bb.put((byte)0x00); // count
        bb.put((byte)0x01);
        bb.flip();
        assertEquals(CodecReturnCodes.SUCCESS, buffer.data(bb, 0, 8));
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(false, vector.checkHasSetDefs());
        assertEquals(false, vector.checkHasSummaryData());
        assertEquals(true, vector.checkHasTotalCountHint());
        assertEquals(DataTypes.FIELD_LIST, vector.containerType());
        assertEquals(1073741823, vector.totalCountHint());
    }

    /**
     * Test RsslVectorEntry.decode().
     * <ol>
     * <li>Sucessfully decode a complete Vector with Entries.</li>
     * <li>Test vector entry with INCOMPLETE_DATA for encodedData.</li>
     * <li>Test vector entry with BUFFER_TOO_SMALL for encodedData.</li>
     * <li>Test vector entry with INCOMPLETE_DATA for permData.</li>
     * <li>Test vector entry with BUFFER_TOO_SMALL for permData.</li>
     * <li>Test vector entry with INCOMPLETE_DATA at the beginning of the entry.</li>
     * </ol>
     */
    @Test
    public void decodeVectorEntryTest()
    {
        // Sucessfully decode a complete Vector with Entries.
        byte[] expected = {0x1f, 0x04, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x31, 0x32,
        		0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x1a, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
        		0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x02, 0x00, 0x03, 0x12, 0x0b, 0x04,
        		0x50, 0x45, 0x52, 0x4d, 0x1a, 0x5a, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50, 0x4f, 0x4e, 0x4d, 0x4c,
        		0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x02, 0x0c, 0x1a, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
        		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
        		0x05, 0x0d};

        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(bb);

        Vector vector = CodecFactory.createVector();
        VectorEntry vectorEntry = CodecFactory.createVectorEntry();
        
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(true, vector.checkHasPerEntryPermData());
        assertEquals(true, vector.checkHasSetDefs());
        assertEquals(true, vector.checkHasSummaryData());
        assertEquals(true, vector.checkHasTotalCountHint());
        assertEquals(true, vector.checkSupportsSorting());
        assertEquals(DataTypes.FIELD_LIST, vector.containerType());
        assertEquals(2, vector.totalCountHint());
        assertEquals("000000001111111122222222", vector.encodedSetDefs().toString());
        assertEquals("ABCDEFGHIJKLMNOPQRSTUVWXYZ", vector.encodedSummaryData().toString());
        // decode vector entry 1
        assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.decode(dIter));
        assertEquals(11, vectorEntry.index()); 
        assertEquals(true, vectorEntry.checkHasPermData());
        assertEquals(VectorEntryActions.SET, vectorEntry.action());
        assertEquals("PERM", vectorEntry.permData().toString());
        assertEquals("ZYXWVUTSRQPONMLKJIHGFEDCBA", vectorEntry.encodedData().toString());
        // decode vector entry 2
        vectorEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.decode(dIter));
        assertEquals(12, vectorEntry.index());
        assertEquals(VectorEntryActions.SET, vectorEntry.action());
        assertEquals(false, vectorEntry.checkHasPermData());
        assertEquals("AAAAAAAAAAAAAAAAAAAAAAAAAA", vectorEntry.encodedData().toString());
        // decode vector entry 3
        vectorEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, vectorEntry.decode(dIter));
        assertEquals(13, vectorEntry.index());
        assertEquals(false, vectorEntry.checkHasPermData());
        assertEquals(VectorEntryActions.DELETE, vectorEntry.action());
        // decode end of container
        vectorEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, vectorEntry.decode(dIter));

        // Test vector entry with INCOMPLETE_DATA for encodedData.
        bb.rewind();
        bb.limit(bb.capacity());
        buffer.data(bb, 0 , 77);
        vector.clear();
        vectorEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vectorEntry.decode(dIter));
         
        // Test vector entry with BUFFER_TOO_SMALL for encodedData.
        bb.rewind();
        bb.limit(77);
        buffer.data(bb, 0 , 77);
        vector.clear();
        vectorEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vectorEntry.decode(dIter));
         
        // Test vector entry with INCOMPLETE_DATA for permData
        bb.rewind();
        bb.limit(bb.capacity());
        buffer.data(bb, 0 , 63);
        vector.clear();
        vectorEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vectorEntry.decode(dIter));
         
        // Test vector entry with BUFFER_TOO_SMALL for PermData.
        bb.rewind();
        bb.limit(63);
        buffer.data(bb, 0 , 63);
        vector.clear();
        vectorEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vectorEntry.decode(dIter));
        
        // Test vector entry with INCOMPLETE_DATA at the beginning of the entry.
        bb.rewind();
        bb.limit(58);
        buffer.data(bb, 0 , 58);
        vector.clear();
        vectorEntry.clear();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, vector.decode(dIter));
        assertEquals(CodecReturnCodes.INCOMPLETE_DATA, vectorEntry.decode(dIter));
    }
    
    /**
     * Test RsslGenericMsg.decode with data from ETAC.
     */
    @Test
    public void decodeGenericMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/018_genericMsg.txt");
        assertNotNull(expected);
        
        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer etaBuffer = CodecFactory.createBuffer();
        etaBuffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        GenericMsg msg = (GenericMsg)CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.GENERIC, msg.msgClass());
        assertEquals(DomainTypes.LOGIN, msg.domainType());
        assertEquals(2146290601, msg.streamId());
        assertEquals(true, msg.checkMessageComplete());
        // Data Format
        assertEquals(DataTypes.FIELD_LIST, msg.containerType());
        // verify sequence number
        assertEquals(true, msg.checkHasSeqNum());
        assertEquals(1234567890, msg.seqNum());
        // verify secondary sequence number
        assertEquals(true, msg.checkHasSecondarySeqNum());
        assertEquals(1122334455, msg.secondarySeqNum());
        // verify part number
        assertEquals(true, msg.checkHasPartNum());
        assertEquals(12345, msg.partNum());
        // verify Perm Expression
        assertEquals(true, msg.checkHasPermData());
        ByteBuffer buf = msg.permData().data();
        int position = msg.permData().position();
        assertEquals(0x10, buf.get(position++));
        assertEquals(0x11, buf.get(position++));
        assertEquals(0x12, buf.get(position++));
        assertEquals(0x13, buf.get(position++));
        // verify Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        assertEquals("EXTENDED HEADER", msg.extendedHeader().toString());
        assertEquals(false, msg.isFinalMsg());

        // msgKey
        assertEquals(true, msg.checkHasMsgKey());
        MsgKey key = msg.msgKey();
        assertEquals(true, key.checkHasAttrib());
        assertEquals(false, key.checkHasFilter());
        assertEquals(false, key.checkHasIdentifier());
        assertEquals(true, key.checkHasName());
        assertEquals(true, key.checkHasNameType());
        assertEquals(true, key.checkHasServiceId());
        assertEquals(InstrumentNameTypes.RIC, key.nameType());
        assertEquals("TRI.N", key.name().toString());
        assertEquals(32639, key.serviceId());
        assertEquals(DataTypes.ELEMENT_LIST, key.attribContainerType());
        assertEquals(CodecReturnCodes.SUCCESS, msg.decodeKeyAttrib(dIter, key));
        
        // decode msgKey's Attrib (ElementList with three ElementEntries)
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        assertEquals(true, elementList.checkHasStandardData());
        assertEquals(false, elementList.checkHasInfo());
        // msgKey's elementList entry 1
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPID.toString(), elementEntry.name().toString());
        assertEquals("256", elementEntry.encodedData().toString());
        // msgKey's elementList entry 2        
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPNAME.toString(), elementEntry.name().toString());
        assertEquals("rsslConsumer", elementEntry.encodedData().toString());
        // msgKey's elementList entry 3
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.POSITION.toString(), elementEntry.name().toString());
        assertEquals("localhost", elementEntry.encodedData().toString());
        // msgKey's elementList END_OF_CONTAINER
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        
        // decode payload (FieldList with four entries.
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();
        UInt uint = CodecFactory.createUInt();
        Buffer buffer = CodecFactory.createBuffer();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasInfo());
        assertEquals(true, fieldList.checkHasStandardData());
        assertEquals(false, fieldList.checkHasSetData());
        assertEquals(false, fieldList.checkHasSetId());
        assertEquals(2, fieldList.dictionaryId());
        assertEquals(3, fieldList.fieldListNum());
        // decode field entry 1 (fieldId 10 encoded as a blank REAL).
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(10, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        // decode field entry 2 (fieldId 175 encoded as ASCII_STRING).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(175, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, buffer.decode(dIter));
        assertEquals("ABCDEFG", buffer.toString());
        // decode field entry 3 (fieldId 32 encoded as UINT).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(554433, uint.toLong());
        // decode field entry 4 (fieldId 111 emcpded as REAL).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(111, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(RealHints.EXPONENT_4, real.hint());
        assertEquals(867564, real.toLong());
        // decode end of container
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
        // payload decoding complete. 
    }
    
    /**
     * Test RsslPostMsg.decode with data from ETAC.
     */
    @Test
    public void decodePostMsgTest()
    {
        // test and verify decoding of a complete Request message
        byte[] expected = ParseHexFile
                .parse("src/test/resources/com/refinitiv/eta/data/RsslEncodersJunit/023_postMsg.txt");
        assertNotNull(expected);
        
        ByteBuffer bb = ByteBuffer.wrap(expected);
        Buffer etaBuffer = CodecFactory.createBuffer();
        etaBuffer.data(bb, 0, bb.limit() - bb.position());
        
        // create and associate a decode iterator with the RsslBuffer
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
                     dIter.setBufferAndRWFVersion(etaBuffer, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        
        PostMsg msg = (PostMsg) CodecFactory.createMsg();
        
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter)); 
        assertEquals(MsgClasses.POST, msg.msgClass());
        assertEquals(DomainTypes.LOGIN, msg.domainType());
        assertEquals(2146290601, msg.streamId());
        assertEquals(true, msg.checkAck());
        assertEquals(true, msg.checkPostComplete());
        // Data Format
        assertEquals(DataTypes.FIELD_LIST, msg.containerType());
        // verify sequence number
        assertEquals(true, msg.checkHasSeqNum());
        assertEquals(1234567890, msg.seqNum());
        // verify post id
        assertEquals(true, msg.checkHasPostId());
        assertEquals(12345, msg.postId());
        // verify part number
        assertEquals(true, msg.checkHasPartNum());
        assertEquals(23456, msg.partNum());
        // verify post user rights
        assertEquals(true, msg.checkHasPostUserRights());
        assertEquals(PostUserRights.MODIFY_PERM, msg.postUserRights());
        // verify Perm Expression
        assertEquals(true, msg.checkHasPermData());
        ByteBuffer buf = msg.permData().data();
        int position = msg.permData().position();
        assertEquals(0x10, buf.get(position++));
        assertEquals(0x11, buf.get(position++));
        assertEquals(0x12, buf.get(position++));
        assertEquals(0x13, buf.get(position++));
        // verify Extended Header
        assertEquals(true, msg.checkHasExtendedHdr());
        assertEquals("EXTENDED HEADER", msg.extendedHeader().toString());
        // verify post user info
        PostUserInfo postUserInfo = msg.postUserInfo();
        assertEquals(4294967290L, postUserInfo.userAddr());
        assertEquals(4294967295L, postUserInfo.userId());
        assertEquals(false, msg.isFinalMsg());

        // msgKey
        assertEquals(true, msg.checkHasMsgKey());
        MsgKey key = msg.msgKey();
        assertEquals(true, key.checkHasAttrib());
        assertEquals(false, key.checkHasFilter());
        assertEquals(false, key.checkHasIdentifier());
        assertEquals(true, key.checkHasName());
        assertEquals(true, key.checkHasNameType());
        assertEquals(true, key.checkHasServiceId());
        assertEquals(InstrumentNameTypes.RIC, key.nameType());
        assertEquals("TRI.N", key.name().toString());
        assertEquals(32639, key.serviceId());
        assertEquals(DataTypes.ELEMENT_LIST, key.attribContainerType());
        assertEquals(CodecReturnCodes.SUCCESS, msg.decodeKeyAttrib(dIter, key));
        
        // decode msgKey's Attrib (ElementList with three ElementEntries)
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.decode(dIter, null));
        assertEquals(false, elementList.checkHasSetData());
        assertEquals(false, elementList.checkHasSetId());
        assertEquals(true, elementList.checkHasStandardData());
        assertEquals(false, elementList.checkHasInfo());
        // msgKey's elementList entry 1
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPID.toString(), elementEntry.name().toString());
        assertEquals("256", elementEntry.encodedData().toString());
        // msgKey's elementList entry 2        
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.APPNAME.toString(), elementEntry.name().toString());
        assertEquals("rsslConsumer", elementEntry.encodedData().toString());
        // msgKey's elementList entry 3
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.decode(dIter));
        assertEquals(DataTypes.ASCII_STRING, elementEntry.dataType());
        assertEquals(ElementNames.POSITION.toString(), elementEntry.name().toString());
        assertEquals("localhost", elementEntry.encodedData().toString());
        // msgKey's elementList END_OF_CONTAINER
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, elementEntry.decode(dIter));
        
        // decode payload (FieldList with four entries.
        FieldList fieldList = CodecFactory.createFieldList();
        FieldEntry fieldEntry = CodecFactory.createFieldEntry();
        Real real = CodecFactory.createReal();
        UInt uint = CodecFactory.createUInt();
        Buffer buffer = CodecFactory.createBuffer();
        assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(dIter, null));
        assertEquals(true, fieldList.checkHasInfo());
        assertEquals(true, fieldList.checkHasStandardData());
        assertEquals(false, fieldList.checkHasSetData());
        assertEquals(false, fieldList.checkHasSetId());
        assertEquals(2, fieldList.dictionaryId());
        assertEquals(3, fieldList.fieldListNum());
        // decode field entry 1 (fieldId 10 encoded as a blank REAL).
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(10, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.BLANK_DATA, real.decode(dIter));
        assertEquals(true, real.isBlank());
        // decode field entry 2 (fieldId 175 encoded as ASCII_STRING).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(175, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, buffer.decode(dIter));
        assertEquals("ABCDEFG", buffer.toString());
        // decode field entry 3 (fieldId 32 encoded as UINT).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(32, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, uint.decode(dIter));
        assertEquals(554433, uint.toLong());
        // decode field entry 4 (fieldId 111 emcpded as REAL).
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.SUCCESS, fieldEntry.decode(dIter));
        assertEquals(111, fieldEntry.fieldId());
        assertEquals(DataTypes.UNKNOWN, fieldEntry.dataType());
        assertEquals(CodecReturnCodes.SUCCESS, real.decode(dIter));
        assertEquals(RealHints.EXPONENT_4, real.hint());
        assertEquals(867564, real.toLong());
        // decode end of container
        fieldEntry.clear();
        assertEquals(CodecReturnCodes.END_OF_CONTAINER, fieldEntry.decode(dIter));
        // payload decoding complete. 
    }

    /**
     * Decode a field dictionary with boundary values and verify contents.
     */
    @Test
    public void decodeFieldDictionaryTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        DataDictionary decodedDictionary = CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryBoundary", error));
        
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode field dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
        
        /* decode field dictionary */
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        Int dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.FIELD_DEFINITIONS, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        
        // verify tags
        assertEquals(0, decodedDictionary.infoDictionaryId());
        assertArrayEquals("4.10.11".getBytes(), convertToByteArray(decodedDictionary.infoFieldVersion().data(), decodedDictionary.infoFieldVersion().length(), decodedDictionary.infoFieldVersion().position()));
        
        // verify contents
        // FID 1
        DictionaryEntry entry = decodedDictionary.entry(1);
        assertNotNull(entry);
        assertArrayEquals("PROD_PERM".getBytes(), convertToByteArray(entry.acronym().data(), entry.acronym().length(), entry.acronym().position()));
        assertArrayEquals("PERMISSION".getBytes(), convertToByteArray(entry.ddeAcronym().data(), entry.ddeAcronym().length(), entry.ddeAcronym().position()));
        assertEquals(1, entry.fid());
        assertEquals(0, entry.rippleToField());
        assertEquals(MfFieldTypes.INTEGER, entry.fieldType());
        assertEquals(5, entry.length());
        assertEquals(0, entry.enumLength());
        assertEquals(DataTypes.UINT, entry.rwfType());
        assertEquals(2, entry.rwfLength());

        // FID 32767
        entry = decodedDictionary.entry(32767);
        assertNotNull(entry);
        assertArrayEquals("MAX_FID".getBytes(), convertToByteArray(entry.acronym().data(), entry.acronym().length(), entry.acronym().position()));
        assertArrayEquals("MAX_FID".getBytes(), convertToByteArray(entry.ddeAcronym().data(), entry.ddeAcronym().length(), entry.ddeAcronym().position()));
        assertEquals(32767, entry.fid());
        assertEquals(0, entry.rippleToField());
        assertEquals(MfFieldTypes.ENUMERATED, entry.fieldType());
        assertEquals(3, entry.length());
        assertEquals(3, entry.enumLength());
        assertEquals(DataTypes.ENUM, entry.rwfType());
        assertEquals(1, entry.rwfLength());

        // FID -32768
        entry = decodedDictionary.entry(-32768);
        assertNotNull(entry);
        assertArrayEquals("MIN_FID".getBytes(), convertToByteArray(entry.acronym().data(), entry.acronym().length(), entry.acronym().position()));
        assertArrayEquals("MIN_FID".getBytes(), convertToByteArray(entry.ddeAcronym().data(), entry.ddeAcronym().length(), entry.ddeAcronym().position()));
        assertEquals(-32768, entry.fid());
        assertEquals(0, entry.rippleToField());
        assertEquals(MfFieldTypes.ENUMERATED, entry.fieldType());
        assertEquals(3, entry.length());
        assertEquals(3, entry.enumLength());
        assertEquals(DataTypes.ENUM, entry.rwfType());
        assertEquals(1, entry.rwfLength());
        
        // FID 6
        entry = decodedDictionary.entry(6);
        assertNotNull(entry);
        assertArrayEquals("TRDPRC_1".getBytes(), convertToByteArray(entry.acronym().data(), entry.acronym().length(), entry.acronym().position()));
        assertArrayEquals("LAST".getBytes(), convertToByteArray(entry.ddeAcronym().data(), entry.ddeAcronym().length(), entry.ddeAcronym().position()));
        assertEquals(6, entry.fid());
        assertEquals(7, entry.rippleToField());
        assertEquals(MfFieldTypes.PRICE, entry.fieldType());
        assertEquals(17, entry.length());
        assertEquals(0, entry.enumLength());
        assertEquals(DataTypes.REAL, entry.rwfType());
        assertEquals(7, entry.rwfLength());
    }
    
    /**
     * Decode a enum type dictionary with boundary values and verify contents.
     */
    @Test
    public void decodeEnumTypeDictionaryTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        DataDictionary decodedDictionary = CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(1);
        
        // load enumType dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("src/test/resources/com/refinitiv/eta/data/Codec/enumtypeBoundary.def", error));
        
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Enum Type Dictionary Refresh");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFEnum");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(4);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode enum type dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        /* decode enum type dictionary */
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        Int dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.ENUM_TABLES, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));
        
        // verify tags
        assertEquals(0, decodedDictionary.infoDictionaryId());
        assertArrayEquals("4.10.11".getBytes(), convertToByteArray(decodedDictionary.infoEnumRTVersion().data(), decodedDictionary.infoEnumRTVersion().length(), decodedDictionary.infoEnumRTVersion().position()));
        assertArrayEquals("13.11".getBytes(), convertToByteArray(decodedDictionary.infoEnumDTVersion().data(), decodedDictionary.infoEnumDTVersion().length(), decodedDictionary.infoEnumDTVersion().position()));
        
        // verify contents
        EnumTypeTable[] enumTypeTable = decodedDictionary.enumTables();
        assertNotNull(enumTypeTable[0]);
        assertEquals(null, enumTypeTable[1]);
        EnumTypeTable enumTypeTableEntry = enumTypeTable[0];
        assertEquals(2, enumTypeTableEntry.maxValue());
        assertEquals(2, enumTypeTableEntry.fidReferenceCount());
        int[] fidRefs = enumTypeTableEntry.fidReferences();
        assertEquals(32767, fidRefs[0]);        
        assertEquals(-32768, fidRefs[1]);        
        EnumType[] enumTypes = enumTypeTableEntry.enumTypes();
        assertEquals(3, enumTypes.length);
        assertEquals(0, enumTypes[0].value());
        assertArrayEquals("   ".getBytes(), convertToByteArray(enumTypes[0].display().data(), enumTypes[0].display().length(), enumTypes[0].display().position()));
        assertEquals(1, enumTypes[1].value());
        assertArrayEquals("MAX".getBytes(), convertToByteArray(enumTypes[1].display().data(), enumTypes[1].display().length(), enumTypes[1].display().position()));
        assertEquals(2, enumTypes[2].value());
        assertArrayEquals("MIN".getBytes(), convertToByteArray(enumTypes[2].display().data(), enumTypes[2].display().length(), enumTypes[2].display().position()));
    }
    
    /**
     * Decode a field and enum type dictionary with boundary values and verify enum types.
     */
    @Test
    public void decodeVerifyEnumTypesTest()
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(10000));
        RefreshMsg msg = (RefreshMsg)CodecFactory.createMsg();
        DataDictionary dictionary = CodecFactory.createDataDictionary();
        DataDictionary decodedDictionary = CodecFactory.createDataDictionary();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        Buffer nameBuf = CodecFactory.createBuffer();
        Int dictionaryFid = CodecFactory.createInt();
        dictionaryFid.value(-32768);
        
        // load field dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary("src/test/resources/com/refinitiv/eta/data/Codec/RDMFieldDictionaryBoundary", error));
        
        /* set-up message */
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
    	msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Field Dictionary Refresh (starting fid " + dictionaryFid.toLong() + ")");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFFld");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(3);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode field dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeFieldDictionary(encodeIter, dictionaryFid, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));
        
        /* decode field dictionary */
        msg.clear();
        int len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        Int dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.FIELD_DEFINITIONS, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeFieldDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        // load enumType dictionary
        dictionary.clear();
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadEnumTypeDictionary("src/test/resources/com/refinitiv/eta/data/Codec/enumtypeBoundary.def", error));
        
        /* set-up message */
        msg.clear();
        msg.msgClass(MsgClasses.REFRESH);
    	msg.applyRefreshComplete();
        msg.domainType(DomainTypes.DICTIONARY);
        msg.containerType(DataTypes.SERIES);
		msg.state().streamState(StreamStates.OPEN);
		msg.state().dataState(DataStates.OK);
		msg.state().code(StateCodes.NONE);
		msg.state().text().data("Enum Type Dictionary Refresh");    	
    	msg.applyHasMsgKey();
    	msg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
    	msg.msgKey().applyHasName();
    	msg.msgKey().applyHasFilter();
    	msg.msgKey().applyHasServiceId();
    	msg.msgKey().serviceId(0);
    	
    	/* DictionaryName */
    	nameBuf.data("RWFEnum");
    	msg.msgKey().name(nameBuf);

    	/* StreamId */
    	msg.streamId(4);

        /* clear encode iterator */
        encodeIter.clear();
        
        /* set iterator buffer */
        buf.data().clear();
        encodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.ENCODE_CONTAINER, msg.encodeInit(encodeIter, 0));

        /* encode enum type dictionary */
        assertEquals(CodecReturnCodes.SUCCESS, dictionary.encodeEnumTypeDictionary(encodeIter, Dictionary.VerbosityValues.NORMAL, error));        

        assertEquals(CodecReturnCodes.SUCCESS, msg.encodeComplete(encodeIter, true));

        /* decode enum type dictionary */
        msg.clear();
        len = buf.length();
        buf.data(buf.data(), 0, len);
        decodeIter.clear();
        assertEquals(CodecReturnCodes.SUCCESS,
        		decodeIter.setBufferAndRWFVersion(buf, Codec.majorVersion(),
                                                  Codec.minorVersion()));
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(decodeIter));
        dictionaryType = CodecFactory.createInt();
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.extractDictionaryType(decodeIter, dictionaryType, error));
        assertEquals(Dictionary.Types.ENUM_TABLES, dictionaryType.toLong());
        assertEquals(CodecReturnCodes.SUCCESS, decodedDictionary.decodeEnumTypeDictionary(decodeIter, Dictionary.VerbosityValues.NORMAL, error));

        // verify enum types
        EnumType enumType = null;
        Enum tempEnum = CodecFactory.createEnum();

        // FID 1
        DictionaryEntry entry = decodedDictionary.entry(1);
        tempEnum.value(0);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertEquals(null, enumType); // should be null since fid 1 type is not enum 

        // FID 32767
        entry = decodedDictionary.entry(32767);
        tempEnum.value(0);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(0, enumType.value());
        assertArrayEquals("   ".getBytes(), convertToByteArray(enumType.display().data(), enumType.display().length(), enumType.display().position()));
        tempEnum.value(1);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(1, enumType.value());
        assertArrayEquals("MAX".getBytes(), convertToByteArray(enumType.display().data(), enumType.display().length(), enumType.display().position()));
        tempEnum.value(2);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(2, enumType.value());
        assertArrayEquals("MIN".getBytes(), convertToByteArray(enumType.display().data(), enumType.display().length(), enumType.display().position()));

        // FID -32768
        entry = decodedDictionary.entry(-32768);
        tempEnum.value(0);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(0, enumType.value());
        assertArrayEquals("   ".getBytes(), convertToByteArray(enumType.display().data(), enumType.display().length(), enumType.display().position()));
        tempEnum.value(1);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(1, enumType.value());
        assertArrayEquals("MAX".getBytes(), convertToByteArray(enumType.display().data(), enumType.display().length(), enumType.display().position()));
        tempEnum.value(2);
        enumType = decodedDictionary.entryEnumType(entry, tempEnum);
        assertNotNull(enumType);
        assertEquals(2, enumType.value());
        assertArrayEquals("MIN".getBytes(), convertToByteArray(enumType.display().data(), enumType.display().length(), enumType.display().position()));
    }
    
    // copy decoded data into byte[]
    private byte[] convertToByteArray(ByteBuffer bb, int len, int position)
    {
        byte[] ba = new byte[len];
        for (int i = 0; i < len; i++) {
        	ba[i] = bb.get(i + position);
		}
        return ba;
    }
    
}
