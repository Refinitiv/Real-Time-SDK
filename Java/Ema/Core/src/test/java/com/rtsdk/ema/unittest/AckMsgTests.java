///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.rtsdk.ema.access.*;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class AckMsgTests extends TestCase
{
	public AckMsgTests(String name)
	{
		super(name);
	}
	
	public void testAckMsg_Decode()
	{
		TestUtilities.printTestHead("testAckMsg_Decode", "eta encoding ema decoding");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin ETA AckMsg Set");
		com.rtsdk.eta.codec.AckMsg ackMsg = (com.rtsdk.eta.codec.AckMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");

		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA AckMsg Set");
		System.out.println();

		System.out.println("Begin ETA AckMsg Buffer Encoding");

		com.rtsdk.eta.codec.Buffer msgBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.rtsdk.eta.codec.EncodeIterator encIter = com.rtsdk.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		ackMsg.encode(encIter);

	    System.out.println("End ETA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg Decoding");

		com.rtsdk.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, majorVersion, minorVersion, TestUtilities.getDataDictionary(), null);

		TestUtilities.checkResult(emaAckMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(emaAckMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(emaAckMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(emaAckMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaAckMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(emaAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(emaAckMsg.nackCode() == com.rtsdk.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

		TestUtilities.checkResult(emaAckMsg.hasText(), "AckMsg.hasText()");
		
		TestUtilities.checkResult(emaAckMsg.text().compareTo("denied by source") == 0, "AckMsg.text()");

		TestUtilities.checkResult(emaAckMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(emaAckMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(emaAckMsg.id() == 21, "AckMsg.id()");

		TestUtilities.checkResult(emaAckMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(emaAckMsg.filter() == 12 , "AckMsg.hasFilter()");

		TestUtilities.checkResult(emaAckMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaAckMsg.serviceId() == 5 , "AckMsg.serviceId()");

		TestUtilities.checkResult(emaAckMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(emaAckMsg.nameType() == com.rtsdk.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(emaAckMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(emaAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(emaAckMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaAckMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg Decoding");
		System.out.println();
	}

	public void testAckMsg_toString()
	{
		TestUtilities.printTestHead("testAckMsg_toString", "eta encoding ema toString");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin ETA AckMsg Set");
		com.rtsdk.eta.codec.AckMsg ackMsg = (com.rtsdk.eta.codec.AckMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA AckMsg Set");
		System.out.println();

		System.out.println("Begin ETA AckMsg Buffer Encoding");

		com.rtsdk.eta.codec.Buffer msgBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.rtsdk.eta.codec.EncodeIterator encIter = com.rtsdk.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		ackMsg.encode(encIter);

	    System.out.println("End ETA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg toString");

		com.rtsdk.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaAckMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("AckMsg.toString() != toString() not supported", !(ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));	 		
		
		System.out.println("End EMA AckMsg toString");
		System.out.println();
	}
	
	public void testAckMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testAckMsg_EncodeDecode", "ema encoding ema decoding");
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.rtsdk.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.rtsdk.ema.access.AckMsg ackMsg = EmaFactory.createAckMsg();
	    
		System.out.println("Begin EMA AckMsg test after constructor");

		TestUtilities.checkResult(ackMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(ackMsg.streamId() == 0, "AckMsg.streamId()");

		TestUtilities.checkResult(ackMsg.ackId() == 0, "AckMsg.ackId()");

		TestUtilities.checkResult(!ackMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!ackMsg.hasNackCode(), "AckMsg.hasNackCode()");;
		
		TestUtilities.checkResult(!ackMsg.hasText(), "AckMsg.hasText()");

		TestUtilities.checkResult(!ackMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(!ackMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(!ackMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(!ackMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(!ackMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(!ackMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(ackMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(ackMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA AckMsg Set");
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
    	ackMsg.domainType( com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.streamId( 15 );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.ackId(123);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.seqNum( 22 );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.nackCode(com.rtsdk.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.text("denied by source");
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.name("ABCDEF");
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.serviceId(5);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.filter( 12 );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.id(21);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.attrib(fl);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.payload(fl);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		System.out.println("End EMA AckMsg Set");
		System.out.println();

		System.out.println("Begin EMA AckMsg Decoding");

		com.rtsdk.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, ackMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaAckMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(emaAckMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(ackMsg.ackId() == 123, "AckMsg.ackId()");

		TestUtilities.checkResult(emaAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(emaAckMsg.nackCode() == com.rtsdk.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

		TestUtilities.checkResult(emaAckMsg.hasText(), "AckMsg.hasText()");
		
		TestUtilities.checkResult(emaAckMsg.text().compareTo("denied by source") == 0, "AckMsg.text()");
		
		TestUtilities.checkResult(emaAckMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaAckMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(emaAckMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(emaAckMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(emaAckMsg.id() == 21, "AckMsg.id()");

		TestUtilities.checkResult(emaAckMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(emaAckMsg.filter() == 12 , "AckMsg.hasFilter()");

		TestUtilities.checkResult(emaAckMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaAckMsg.serviceId() == 5 , "AckMsg.serviceId()");

		TestUtilities.checkResult(emaAckMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(emaAckMsg.nameType() == com.rtsdk.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(emaAckMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(emaAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(emaAckMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaAckMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg Decoding");
		System.out.println();

		ackMsg.clear();
		
		System.out.println("Begin EMA AckMsg test after clear");

		TestUtilities.checkResult(ackMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(ackMsg.streamId() == 0, "AckMsg.streamId()");

		TestUtilities.checkResult(!ackMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(!ackMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(!ackMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(!ackMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(!ackMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(!ackMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(ackMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(ackMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg test after clear");
		System.out.println();
	}
	
	public void testAckMsg_EncodeETAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for FieldList as attrib, extended header and payload. Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyAckMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyAckMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyAckMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyAckMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for FilterList as attrib, payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.FILTER_LIST);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyAckMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyAckMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for Series as attrib, payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.SERIES);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyAckMsg.attrib().series(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyAckMsg.payload().series(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for Vector as attrib, payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.VECTOR);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyAckMsg.attrib().vector(), com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyAckMsg.payload().vector(), com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for Map as attrib, payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.MAP);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyAckMsg.attrib().map(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyAckMsg.payload().map(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for RefreshMsg as attrib, payload. Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.rtsdk.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.rtsdk.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.rtsdk.eta.codec.DataTypes.MSG);
	
	     com.rtsdk.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.rtsdk.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyAckMsg.attrib().refreshMsg(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyAckMsg.payload().refreshMsg(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}

	public void testAckMsg_cloneIsNotSupportedFromTheEncodeSide()
	{
		TestUtilities.printTestHead("testAckMsg_cloneIsNotSupportedFromTheEncodeSide", "cloning is not supported on encode side");
		AckMsg msg = EmaFactory.createAckMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE);

		try {
			AckMsg cloneMessage = EmaFactory.createAckMsg(msg);
			TestUtilities.checkResult(false, "Clone not supported  - exception expected: ");
		} catch ( OmmException excp ) {
			TestUtilities.checkResult(true, "Clone not supported  - exception expected: " +  excp.getMessage() );
			TestUtilities.checkResult(excp.getMessage().startsWith("Failed to clone empty encoded buffer"), "Clone not supported - exception text validated");
		}
	}

	public void testAckMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testAckMsg_cloneMsgKeyWLScenario", "cloning for minimal ema ack message");
		AckMsg emaMsg = EmaFactory.createAckMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.rtsdk.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.rtsdk.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, true);
		AckMsg emaClonedMsg = EmaFactory.createAckMsg(emaMsg);

		compareEmaAckMsgFields(emaMsg, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, false);

		System.out.println("End EMA AckMsg Clone msgKey");
		System.out.println();
	}

	public void testAckMsg_clone()
	{
		TestUtilities.printTestHead("testAckMsg_clone", "cloning for ema ack message");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin ETA AckMsg Set");
		com.rtsdk.eta.codec.AckMsg ackMsg = (com.rtsdk.eta.codec.AckMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		setMoreFields(ackMsg);

		System.out.println("End ETA AckMsg Set");
		System.out.println();

		System.out.println("Begin ETA AckMsg Buffer Encoding");

		com.rtsdk.eta.codec.Buffer msgBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.rtsdk.eta.codec.EncodeIterator encIter = com.rtsdk.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		ackMsg.encode(encIter);

	    System.out.println("End ETA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg Clone");
		
		com.rtsdk.eta.codec.AckMsg ackMsgDecode = (com.rtsdk.eta.codec.AckMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();

		com.rtsdk.eta.codec.DecodeIterator decIter = com.rtsdk.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		ackMsgDecode.decode(decIter);

		com.rtsdk.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, ackMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.rtsdk.ema.access.AckMsg emaAckMsgClone = EmaFactory.createAckMsg(emaAckMsg);

		compareEmaAckMsgFields(emaAckMsg, emaAckMsgClone, "Ack clone message");

		String emaAckMsgString = emaAckMsg.toString();
		String emaAckMsgCloneString = emaAckMsgClone.toString();
		
		System.out.println("Cloned EMA AckMsg:");
		System.out.println(emaAckMsgClone);
		
		TestUtilities.checkResult(emaAckMsgString.equals(emaAckMsgCloneString), "emaAckMsgString.equals(emaAckMsgCloneString)");

		com.rtsdk.ema.access.AckMsg emaAckMsgClone2 = EmaFactory.createAckMsg(emaAckMsgClone);
		compareEmaAckMsgFields(emaAckMsg, emaAckMsgClone2, "Ack double-cloned message");
		String emaAckMsgClone2String = emaAckMsgClone2.toString();
		TestUtilities.checkResult(emaAckMsgString.equals(emaAckMsgClone2String), "double-cloned emaAckMsgString.equals(emaAckMsgClone2String)");

		System.out.println("End EMA AckMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.rtsdk.eta.codec.AckMsg ackMsg) {
		ackMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		ackMsg.extendedHeader(extendedHeader);

		ackMsg.applyPrivateStream();
	}

	public void testAckMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testAckMsg_cloneEdit", "clone and edit ema ack message");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin ETA AckMsg Set");
		com.rtsdk.eta.codec.AckMsg ackMsg = (com.rtsdk.eta.codec.AckMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.rtsdk.eta.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA AckMsg Set");
		System.out.println();

		System.out.println("Begin ETA AckMsg Buffer Encoding");

		com.rtsdk.eta.codec.Buffer msgBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.rtsdk.eta.codec.EncodeIterator encIter = com.rtsdk.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		ackMsg.encode(encIter);

	    System.out.println("End ETA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg Clone");
		
		com.rtsdk.eta.codec.AckMsg ackMsgDecode = (com.rtsdk.eta.codec.AckMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();

		com.rtsdk.eta.codec.DecodeIterator decIter = com.rtsdk.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		ackMsgDecode.decode(decIter);

		com.rtsdk.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, ackMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.rtsdk.ema.access.AckMsg emaAckMsgClone = EmaFactory.createAckMsg(emaAckMsg);
		
		TestUtilities.checkResult(emaAckMsgClone.domainType() == emaAckMsg.domainType(), "Compare domainType");
		TestUtilities.checkResult(emaAckMsgClone.streamId() == emaAckMsg.streamId(), "Compare streamId");
		TestUtilities.checkResult(emaAckMsgClone.ackId() == emaAckMsg.ackId(), "Compare ackId");
		TestUtilities.checkResult(emaAckMsgClone.hasSeqNum() == emaAckMsg.hasSeqNum(), "Compare hasSeqNum");
		TestUtilities.checkResult(emaAckMsgClone.seqNum() == emaAckMsg.seqNum(), "Compare seqNum");
		TestUtilities.checkResult(emaAckMsgClone.hasNackCode() == emaAckMsg.hasNackCode(), "Compare hasNackCode");
		TestUtilities.checkResult(emaAckMsgClone.nackCode() == emaAckMsg.nackCode(), "Compare nackCode");
		TestUtilities.checkResult(emaAckMsgClone.hasMsgKey() == emaAckMsg.hasMsgKey(), "Compare hasMsgKey");
		
		String emaAckMsgString = emaAckMsg.toString();
		String emaAckMsgCloneString = emaAckMsgClone.toString();
		
		System.out.println("Cloned EMA AckMsg:");
		System.out.println(emaAckMsgClone);
		
		TestUtilities.checkResult(emaAckMsgString.equals(emaAckMsgCloneString), "emaAckMsgString.equals(emaAckMsgCloneString)");
		
		emaAckMsgClone.streamId(10);
		emaAckMsgClone.payload().fieldList().add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		TestUtilities.checkResult(emaAckMsgClone.streamId() != emaAckMsg.streamId(), "Compare streamId");

		// Check emaAckMsg for no FID 21
		Iterator<FieldEntry> iter = emaAckMsg.payload().fieldList().iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			TestUtilities.checkResult(fieldEntry.fieldId() != 21, "Check emaAckMsg for no FID 21");
		}
		
		boolean foundFid = false;
		Iterator<FieldEntry> iterClone = emaAckMsgClone.payload().fieldList().iterator();
		while (iterClone.hasNext())
		{
			fieldEntry = iterClone.next();
			if(foundFid = fieldEntry.fieldId() == 21)
				break;
		}
		
		TestUtilities.checkResult(foundFid, "Check emaAckMsgClone for FID 21");
		
		emaAckMsgString = emaAckMsg.toString();
		emaAckMsgCloneString = emaAckMsgClone.toString();
		
		TestUtilities.checkResult(!emaAckMsgString.equals(emaAckMsgCloneString), "Check that emaAckMsgString does not equal emaAckMsgCloneString");
		
		System.out.println("End EMA AckMsg Clone");
		System.out.println();
	}

	private void compareEmaAckMsgFields(AckMsg expected, AckMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + " base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasServiceName() == actual.hasServiceName(), checkPrefix + "hasServiceName");
		if(expected.hasServiceName())
			TestUtilities.checkResult(expected.serviceName().equals(actual.serviceName()), checkPrefix + "serviceId" + "exp=" +actual.serviceName() + " act="+expected.serviceName());

		TestUtilities.checkResult(expected.privateStream() == actual.privateStream(), checkPrefix + "privateStream");

		TestUtilities.checkResult(expected.hasExtendedHeader() == actual.hasExtendedHeader(), checkPrefix + "hasExtendedHeader");
		if(expected.hasExtendedHeader())
			TestUtilities.checkResult(expected.extendedHeader().equals(actual.extendedHeader()), checkPrefix + "extendedHeader");

		TestUtilities.checkResult(expected.hasSeqNum() == actual.hasSeqNum(), checkPrefix + "hasSeqNum");
		if(expected.hasSeqNum())
			TestUtilities.checkResult(expected.seqNum() == actual.seqNum(), checkPrefix + "seqNum");

		TestUtilities.checkResult(expected.ackId() == actual.ackId(), checkPrefix + "ackId");

		TestUtilities.checkResult(expected.hasNackCode() == actual.hasNackCode(), checkPrefix + "hasNackCode");
		if(expected.hasNackCode())
			TestUtilities.checkResult(expected.nackCode() == actual.nackCode(), checkPrefix + "nackCode");

		TestUtilities.checkResult(expected.hasText() == actual.hasText(), checkPrefix + "hasText");
		if(expected.hasText())
			TestUtilities.checkResult(expected.text().equals(actual.text()), checkPrefix + "text");
	}

}
