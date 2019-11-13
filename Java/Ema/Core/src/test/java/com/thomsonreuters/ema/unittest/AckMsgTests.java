///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class AckMsgTests extends TestCase
{
	public AckMsgTests(String name)
	{
		super(name);
	}
	
	public void testAckMsg_Decode()
	{
		TestUtilities.printTestHead("testAckMsg_Decode", "upa encoding ema decoding");
		
		com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES )) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin UPA AckMsg Set");
		com.thomsonreuters.upa.codec.AckMsg ackMsg = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");

		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA AckMsg Set");
		System.out.println();

		System.out.println("Begin UPA AckMsg Buffer Encoding");

		com.thomsonreuters.upa.codec.Buffer msgBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.thomsonreuters.upa.codec.EncodeIterator encIter = com.thomsonreuters.upa.codec.CodecFactory.createEncodeIterator();
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

	    System.out.println("End UPA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg Decoding");

		com.thomsonreuters.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, majorVersion, minorVersion, TestUtilities.getDataDictionary(), null);

		TestUtilities.checkResult(emaAckMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(emaAckMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(emaAckMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(emaAckMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaAckMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(emaAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(emaAckMsg.nackCode() == com.thomsonreuters.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

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
		
		TestUtilities.checkResult(emaAckMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(emaAckMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(emaAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(emaAckMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaAckMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg Decoding");
		System.out.println();
	}

	public void testAckMsg_toString()
	{
		TestUtilities.printTestHead("testAckMsg_toString", "upa encoding ema toString");
		
		com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin UPA AckMsg Set");
		com.thomsonreuters.upa.codec.AckMsg ackMsg = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA AckMsg Set");
		System.out.println();

		System.out.println("Begin UPA AckMsg Buffer Encoding");

		com.thomsonreuters.upa.codec.Buffer msgBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.thomsonreuters.upa.codec.EncodeIterator encIter = com.thomsonreuters.upa.codec.CodecFactory.createEncodeIterator();
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

	    System.out.println("End UPA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg toString");

		com.thomsonreuters.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

	    com.thomsonreuters.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.thomsonreuters.ema.access.AckMsg ackMsg = EmaFactory.createAckMsg();
	    
		System.out.println("Begin EMA AckMsg test after constructor");

		TestUtilities.checkResult(ackMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
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
		
		TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA AckMsg Set");
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
    	ackMsg.domainType( com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.streamId( 15 );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.ackId(123);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.seqNum( 22 );
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.nackCode(com.thomsonreuters.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE);
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.text("denied by source");
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.name("ABCDEF");
		TestUtilities.checkResult("AckMsg.toString() == toString() not supported", ackMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		ackMsg.nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );
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

		com.thomsonreuters.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, ackMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaAckMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(emaAckMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(ackMsg.ackId() == 123, "AckMsg.ackId()");

		TestUtilities.checkResult(emaAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(emaAckMsg.nackCode() == com.thomsonreuters.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

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
		
		TestUtilities.checkResult(emaAckMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(emaAckMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(emaAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(emaAckMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaAckMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg Decoding");
		System.out.println();

		ackMsg.clear();
		
		System.out.println("Begin EMA AckMsg test after clear");

		TestUtilities.checkResult(ackMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(ackMsg.streamId() == 0, "AckMsg.streamId()");

		TestUtilities.checkResult(!ackMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(!ackMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(!ackMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(!ackMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(!ackMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(!ackMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(ackMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(ackMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg test after clear");
		System.out.println();
	}
	
	public void testAckMsg_EncodeUPAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for FieldList as attrib, extended header and payload. Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyAckMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyAckMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestAckMsg_EncodeUPAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeUPAAckMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPAAckMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for ElementList as attrib, extended header and payload, Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyAckMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyAckMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestAckMsg_EncodeUPAAckMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeUPAAckMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPAAckMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for FilterList as attrib, payload, Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyAckMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyAckMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestAckMsg_EncodeUPAAckMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeUPAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for Series as attrib, payload, Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.SERIES);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyAckMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyAckMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeUPAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeUPAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for Vector as attrib, payload, Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyAckMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyAckMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeUPAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for Map as attrib, payload, Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.MAP);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyAckMsg.attrib().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyAckMsg.payload().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeUPAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeUPAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with UPA for RefreshMsg as attrib, payload. Encode it to another AckMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.thomsonreuters.upa.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.upa_EncodeAckMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.thomsonreuters.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAAckMsgAll(ackMsg, com.thomsonreuters.upa.codec.DataTypes.MSG);
	
	     com.thomsonreuters.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyAckMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyAckMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeUPAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_clone()
	{
		TestUtilities.printTestHead("testAckMsg_clone", "cloning for ema ack message");
		
		com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin UPA AckMsg Set");
		com.thomsonreuters.upa.codec.AckMsg ackMsg = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA AckMsg Set");
		System.out.println();

		System.out.println("Begin UPA AckMsg Buffer Encoding");

		com.thomsonreuters.upa.codec.Buffer msgBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.thomsonreuters.upa.codec.EncodeIterator encIter = com.thomsonreuters.upa.codec.CodecFactory.createEncodeIterator();
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

	    System.out.println("End UPA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg Clone");
		
		com.thomsonreuters.upa.codec.AckMsg ackMsgDecode = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();

		com.thomsonreuters.upa.codec.DecodeIterator decIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		ackMsgDecode.decode(decIter);

		com.thomsonreuters.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, ackMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.thomsonreuters.ema.access.AckMsg emaAckMsgClone = EmaFactory.createAckMsg(emaAckMsg);
		
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
		System.out.println("End EMA AckMsg Clone");
		System.out.println();
	}
	
	public void testAckMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testAckMsg_cloneEdit", "clone and edit ema ack message");
		
		com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End UPA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin UPA AckMsg Set");
		com.thomsonreuters.upa.codec.AckMsg ackMsg = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.thomsonreuters.upa.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA AckMsg Set");
		System.out.println();

		System.out.println("Begin UPA AckMsg Buffer Encoding");

		com.thomsonreuters.upa.codec.Buffer msgBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.thomsonreuters.upa.codec.EncodeIterator encIter = com.thomsonreuters.upa.codec.CodecFactory.createEncodeIterator();
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

	    System.out.println("End UPA AckMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA AckMsg Clone");
		
		com.thomsonreuters.upa.codec.AckMsg ackMsgDecode = (com.thomsonreuters.upa.codec.AckMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();

		com.thomsonreuters.upa.codec.DecodeIterator decIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		ackMsgDecode.decode(decIter);

		com.thomsonreuters.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, ackMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.thomsonreuters.ema.access.AckMsg emaAckMsgClone = EmaFactory.createAckMsg(emaAckMsg);
		
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
}
