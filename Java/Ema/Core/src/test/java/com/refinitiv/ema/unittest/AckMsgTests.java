///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import static org.junit.Assert.assertThrows;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.NakCodes;

import junit.framework.TestCase;

public class AckMsgTests extends TestCase
{
	private static com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
	
	public AckMsgTests(String name)
	{
		super(name);
	}
	
	private com.refinitiv.eta.codec.Buffer encodeETAAckMsg()
	{
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		if(dictionary.numberOfEntries() == 0)
			TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return null;
		}
		System.out.println("End ETA FieldList Encoding");
		System.out.println();

		fieldListBuf.data(fieldListBuf.data(),  0,  fieldListBuf.length());
		
	    System.out.println("Begin ETA AckMsg Set");
		com.refinitiv.eta.codec.AckMsg ackMsg = (com.refinitiv.eta.codec.AckMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		ackMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.ACK);
		
		ackMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		ackMsg.streamId( 15 );
		
		ackMsg.ackId(321);

		ackMsg.applyHasText();
		ackMsg.text().data("denied by source");
		
		ackMsg.applyHasSeqNum();
		ackMsg.seqNum( 22 );

		ackMsg.applyHasNakCode();
		ackMsg.nakCode(com.refinitiv.eta.codec.NakCodes.DENIED_BY_SRC);
		
		ackMsg.applyHasMsgKey();

		ackMsg.msgKey().applyHasName();
		ackMsg.msgKey().name().data( "ABCDEF" );
		
		ackMsg.msgKey().applyHasNameType();
		ackMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		ackMsg.msgKey().applyHasServiceId();
		ackMsg.msgKey().serviceId(5);
		
		ackMsg.msgKey().applyHasFilter();
		ackMsg.msgKey().filter( 12 );
	
		ackMsg.msgKey().applyHasIdentifier();
		ackMsg.msgKey().identifier(21);
		
		ackMsg.msgKey().applyHasAttrib();
		ackMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		ackMsg.msgKey().encodedAttrib(fieldListBuf);
	
		ackMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		ackMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA AckMsg Set");
		System.out.println();

		System.out.println("Begin ETA AckMsg Buffer Encoding");

		com.refinitiv.eta.codec.Buffer msgBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));
		
		com.refinitiv.eta.codec.EncodeIterator encIter = com.refinitiv.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return null;
		}
		
		ackMsg.encode(encIter);

	    System.out.println("End ETA AckMsg Buffer Encoding");
		System.out.println();
		
		return msgBuf;
	}
	
	private AckMsg encodeEMAAckMsg()
	{
		AckMsg emaAckMsg = EmaFactory.createAckMsg();
		
		emaAckMsg.streamId(15);
		emaAckMsg.ackId(321);
		emaAckMsg.text("access denied");
		emaAckMsg.seqNum(22);
		emaAckMsg.nackCode(NakCodes.ACCESS_DENIED);
		
		emaAckMsg.name("ABCDEF");
		emaAckMsg.nameType(com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC);
		emaAckMsg.serviceId(5);
		emaAckMsg.filter(12);
		emaAckMsg.id(21);
		emaAckMsg.serviceName("DIRECT_FEED");
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().intValue(1, 5));
		
		emaAckMsg.attrib(fieldList);
		emaAckMsg.payload(fieldList);
		
		return emaAckMsg;
	}
	
	public void testAckMsg_Decode()
	{
		TestUtilities.printTestHead("testAckMsg_Decode", "eta encoding ema decoding");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();

		System.out.println("Begin EMA AckMsg Decoding");

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);

		TestUtilities.checkResult(emaAckMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(emaAckMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(emaAckMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(emaAckMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaAckMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(emaAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(emaAckMsg.nackCode() == com.refinitiv.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

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
		
		TestUtilities.checkResult(emaAckMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(emaAckMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(emaAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(emaAckMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaAckMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg Decoding");
		System.out.println();
	}

	public void testAckMsg_toString()
	{
		TestUtilities.printTestHead("testAckMsg_toString", "eta encoding ema toString");
		
	    com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();

		System.out.println("Begin EMA AckMsg toString");

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		System.out.println(emaAckMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("AckMsg.toString() != toString()", !(emaAckMsg.toString().equals("\nString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));
		
		System.out.println("End EMA AckMsg toString");
		System.out.println();
	}
	
	public void testAckMsg_EncodeDecode()
	{
		String ackMsgString = "AckMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    ackId=\"123\"\n" +
				"    seqNum=\"22\"\n" +
				"    nackCode=\"DeniedBySource\"\n" +
				"    text=\"denied by source\"\n" +
				"    name=\"ABCDEF\"\n" +
				"    nameType=\"1\"\n" +
				"    serviceId=\"5\"\n" +
				"    filter=\"12\"\n" +
				"    id=\"21\"\n" +
				"    Attrib dataType=\"FieldList\"\n" +
				"        FieldList FieldListNum=\"65\" DictionaryId=\"1\"\n" +
				"            FieldEntry fid=\"1\" name=\"PROD_PERM\" dataType=\"UInt\" value=\"64\"\n" +
				"            FieldEntry fid=\"6\" name=\"TRDPRC_1\" dataType=\"Real\" value=\"0.11\"\n" +
				"            FieldEntry fid=\"-2\" name=\"INTEGER\" dataType=\"Int\" value=\"32\"\n" +
				"            FieldEntry fid=\"16\" name=\"TRADE_DATE\" dataType=\"Date\" value=\"07 NOV 1999\"\n" +
				"            FieldEntry fid=\"18\" name=\"TRDTIM_1\" dataType=\"Time\" value=\"02:03:04:005:000:000\"\n" +
				"            FieldEntry fid=\"-3\" name=\"TRADE_DATE\" dataType=\"DateTime\" value=\"07 NOV 1999 01:02:03:000:000:000\"\n" +
				"            FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"RealTime/TickByTick\"\n" +
				"            FieldEntry fid=\"-6\" name=\"MY_STATE\" dataType=\"State\" value=\"Open / Ok / None / 'Succeeded'\"\n" +
				"            FieldEntry fid=\"235\" name=\"PNAC\" dataType=\"Ascii\" value=\"ABCDEF\"\n" +
				"        FieldListEnd\n" +
				"    AttribEnd\n" +
				"    Payload dataType=\"FieldList\"\n" +
				"        FieldList FieldListNum=\"65\" DictionaryId=\"1\"\n" +
				"            FieldEntry fid=\"1\" name=\"PROD_PERM\" dataType=\"UInt\" value=\"64\"\n" +
				"            FieldEntry fid=\"6\" name=\"TRDPRC_1\" dataType=\"Real\" value=\"0.11\"\n" +
				"            FieldEntry fid=\"-2\" name=\"INTEGER\" dataType=\"Int\" value=\"32\"\n" +
				"            FieldEntry fid=\"16\" name=\"TRADE_DATE\" dataType=\"Date\" value=\"07 NOV 1999\"\n" +
				"            FieldEntry fid=\"18\" name=\"TRDTIM_1\" dataType=\"Time\" value=\"02:03:04:005:000:000\"\n" +
				"            FieldEntry fid=\"-3\" name=\"TRADE_DATE\" dataType=\"DateTime\" value=\"07 NOV 1999 01:02:03:000:000:000\"\n" +
				"            FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"RealTime/TickByTick\"\n" +
				"            FieldEntry fid=\"-6\" name=\"MY_STATE\" dataType=\"State\" value=\"Open / Ok / None / 'Succeeded'\"\n" +
				"            FieldEntry fid=\"235\" name=\"PNAC\" dataType=\"Ascii\" value=\"ABCDEF\"\n" +
				"        FieldListEnd\n" +
				"    PayloadEnd\n" +
				"AckMsgEnd\n";

		String ackMsgEmptyString = "AckMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    ackId=\"0\"\n" +
				"AckMsgEnd\n";

		TestUtilities.printTestHead("testAckMsg_EncodeDecode", "ema encoding ema decoding");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.AckMsg ackMsg = EmaFactory.createAckMsg();
		com.refinitiv.ema.access.AckMsg ackMsgEmpty = EmaFactory.createAckMsg();
	    
		System.out.println("Begin EMA AckMsg test after constructor");

		TestUtilities.checkResult(ackMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
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
		
		TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA AckMsg Set");
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	    
    	ackMsg.domainType( com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.streamId( 15 );
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.ackId(123);
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.seqNum( 22 );
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.nackCode(com.refinitiv.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE);
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.text("denied by source");
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.name("ABCDEF");
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.serviceId(5);
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.filter( 12 );
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.id(21);
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.attrib(fl);
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		ackMsg.payload(fl);
		TestUtilities.checkResult("AckMsg.toString() == toString()", ackMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		System.out.println("End EMA AckMsg Set");
		System.out.println();

		System.out.println("Begin EMA AckMsg Decoding");

		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("AckMsg.toString(dictionary) == toString(dictionary)", ackMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("AckMsg.toString(dictionary) == toString(dictionary)", ackMsgEmpty.toString(emaDataDictionary).equals(ackMsgEmptyString));

		TestUtilities.checkResult("AckMsg.toString(dictionary) == toString(dictionary)", ackMsg.toString(emaDataDictionary).equals(ackMsgString));

		com.refinitiv.ema.access.AckMsg ackMsgClone = EmaFactory.createAckMsg(ackMsg);
		ackMsgClone.clear();
		TestUtilities.checkResult("AckMsg.toString(dictionary) == toString(dictionary)", ackMsgClone.toString(emaDataDictionary).equals(ackMsgEmptyString));

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, ackMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaAckMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(emaAckMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(ackMsg.ackId() == 123, "AckMsg.ackId()");

		TestUtilities.checkResult(emaAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(emaAckMsg.nackCode() == com.refinitiv.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

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
		
		TestUtilities.checkResult(emaAckMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(emaAckMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(emaAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

		TestUtilities.checkResult(emaAckMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaAckMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg Decoding");
		System.out.println();

		ackMsg.clear();
		
		System.out.println("Begin EMA AckMsg test after clear");

		TestUtilities.checkResult(ackMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(ackMsg.streamId() == 0, "AckMsg.streamId()");

		TestUtilities.checkResult(!ackMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(!ackMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(!ackMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(!ackMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(!ackMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(!ackMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(ackMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(ackMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "AckMsg.payload().dataType()");

		System.out.println("End EMA AckMsg test after clear");
		System.out.println();
	}
	
	public void testAckMsg_EncodeETAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for FieldList as attrib, extended header and payload. Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
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
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
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
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(18192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
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
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyAckMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyAckMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for Vector as attrib, payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyAckMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyAckMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for Map as attrib, payload, Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyAckMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyAckMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}
	
	public void testAckMsg_EncodeETAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testAckMsg_EncodeETAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode", "Encode AckMsg with ETA for RefreshMsg as attrib, payload. Encode it to another AckMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(8192));
        
    	int majorVersion = Codec.majorVersion();  // This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
	
		// Create and clear iterator to prepare for encoding
		com.refinitiv.eta.codec.EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
		encodeIter.clear();
	
		// Associate buffer and iterator and set proper protocol version information on iterator.
		if ((retVal = encodeIter.setBufferAndRWFVersion(buffer, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with setBufferAndRWFVersion. "
							+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		if ( ( retVal = TestUtilities.eta_EncodeAckMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeAckMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode AckMsg with EMA.
	     com.refinitiv.ema.access.AckMsg ackMsg = JUnitTestConnect.createAckMsg();
	     JUnitTestConnect.setRsslData(ackMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAAckMsgAll(ackMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.AckMsg copyAckMsg = EmaFactory.createAckMsg();
	     
	     // Encode to another AckMsg
	     copyAckMsg.extendedHeader(ackMsg.extendedHeader());
	     copyAckMsg.attrib(ackMsg.attrib().data());
	     copyAckMsg.payload(ackMsg.payload().data());
	     
	     com.refinitiv.ema.access.AckMsg decCopyAckMsg = JUnitTestConnect.createAckMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyAckMsg, copyAckMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyAckMsg.extendedHeader().equals(ackMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyAckMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyAckMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestAckMsg_EncodeETAAckMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherAckMsg_EMADecode passed");
	}

	public void testAckMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testAckMsg_cloneMsgKeyWLScenario", "cloning for minimal ema ack message");
		AckMsg emaMsg = EmaFactory.createAckMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
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
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();

		System.out.println("Begin EMA AckMsg Clone");
		
		com.refinitiv.eta.codec.AckMsg ackMsgDecode = (com.refinitiv.eta.codec.AckMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
		ackMsgDecode.decode(decIter);

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, ackMsgDecode, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		com.refinitiv.ema.access.AckMsg emaAckMsgClone = EmaFactory.createAckMsg(emaAckMsg);

		compareEmaAckMsgFields(emaAckMsg, emaAckMsgClone, "Ack clone message");

		String emaAckMsgString = emaAckMsg.toString();
		String emaAckMsgCloneString = emaAckMsgClone.toString();
		
		System.out.println("Cloned EMA AckMsg:");
		System.out.println(emaAckMsgClone);
		
		TestUtilities.checkResult(emaAckMsgString.equals(emaAckMsgCloneString), "emaAckMsgString.equals(emaAckMsgCloneString)");

		com.refinitiv.ema.access.AckMsg emaAckMsgClone2 = EmaFactory.createAckMsg(emaAckMsgClone);
		compareEmaAckMsgFields(emaAckMsg, emaAckMsgClone2, "Ack double-cloned message");
		String emaAckMsgClone2String = emaAckMsgClone2.toString();
		TestUtilities.checkResult(emaAckMsgString.equals(emaAckMsgClone2String), "double-cloned emaAckMsgString.equals(emaAckMsgClone2String)");

		System.out.println("End EMA AckMsg Clone");
		System.out.println();
	}

	public void testAckMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testAckMsg_cloneEdit", "clone and edit ema ack message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();

		System.out.println("Begin EMA AckMsg Clone");
		
		com.refinitiv.eta.codec.AckMsg ackMsgDecode = (com.refinitiv.eta.codec.AckMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
		ackMsgDecode.decode(decIter);

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();
		
		JUnitTestConnect.setRsslData(emaAckMsg, ackMsgDecode, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		com.refinitiv.ema.access.AckMsg emaAckMsgClone = EmaFactory.createAckMsg(emaAckMsg);
		
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
	
	public void testAckMsg_InitCopyingMessageWithUnknownSize()
	{
		AckMsg copyAckMsg = EmaFactory.createAckMsg(-1);
		int capacity = JUnitTestConnect.getCopiedBufferCapacity(copyAckMsg);
		
		TestUtilities.checkResult(capacity == 0, "Checks capacity of the copied buffer");
		
		copyAckMsg.clear();
		
		capacity = JUnitTestConnect.getCopiedBufferCapacity(copyAckMsg);
		
		TestUtilities.checkResult(capacity == 0, "Checks capacity of the copied buffer after clearing");
	}
	
	public void testAckMsg_InitCopyingMessage()
	{
		AckMsg copyAckMsg = EmaFactory.createAckMsg(1024);
		int capacity = JUnitTestConnect.getCopiedBufferCapacity(copyAckMsg);
		
		TestUtilities.checkResult(capacity == 1024, "Checks capacity of copied buffer");
		copyAckMsg.clear();
		
		capacity = JUnitTestConnect.getCopiedBufferCapacity(copyAckMsg);
		TestUtilities.checkResult(capacity == 1024, "Checks capacity of the copied buffer after clearing AckMsg");
	}
	
	public void testAckMsg_copy_from_decodingAckMsg_and_copied_message()
	{
		TestUtilities.printTestHead("testAckMsg_copy_from_decodingAckMsg_and_copied_message", "eta encoding ema copying");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();
		
		int encodedBufferSize = msgBuf.length();
		
		System.out.println("Begin EMA AckMsg Decoding");

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		String expectedServiceName = "DIRECT_FEED";
		
		/* Set the service name as well */
		emaAckMsg.serviceName(expectedServiceName);
		
		System.out.println("Begin EMA AckMsg Copying");
		
		AckMsg copyAckMsg = EmaFactory.createAckMsg(encodedBufferSize);
		
		emaAckMsg.copy(copyAckMsg);
		
		AckMsg copyAckMsg2 = EmaFactory.createAckMsg(encodedBufferSize);
		
		copyAckMsg.copy(copyAckMsg2);
		
		ArrayList<AckMsg> list = new ArrayList<AckMsg>();
		
		list.add(copyAckMsg);
		list.add(copyAckMsg2);
		
		for(int i = 0; i < list.size(); i++)
		{
			AckMsg checkingAckMsg = list.get(i);
			
			TestUtilities.checkResult(JUnitTestConnect.getDataDictionary(checkingAckMsg) != null, "Checks the reference to DataDictionary after copying AckMsg");
			
			TestUtilities.checkResult(checkingAckMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
			
			TestUtilities.checkResult(checkingAckMsg.streamId() == 15, "AckMsg.streamId()");

			TestUtilities.checkResult(checkingAckMsg.ackId() == 321, "AckMsg.ackId()");
			
			TestUtilities.checkResult(checkingAckMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
			
			TestUtilities.checkResult(checkingAckMsg.seqNum() == 22, "AckMsg.seqNum()");

			TestUtilities.checkResult(checkingAckMsg.hasNackCode(), "AckMsg.hasNackCode()");
			
			TestUtilities.checkResult(checkingAckMsg.nackCode() == com.refinitiv.ema.access.AckMsg.NackCode.DENIED_BY_SOURCE, "AckMsg.nackCode()");

			TestUtilities.checkResult(checkingAckMsg.hasText(), "AckMsg.hasText()");
			
			TestUtilities.checkResult(checkingAckMsg.text().compareTo("denied by source") == 0, "AckMsg.text()");

			TestUtilities.checkResult(checkingAckMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

			TestUtilities.checkResult(checkingAckMsg.hasId(), "AckMsg.hasId()");
			
			TestUtilities.checkResult(checkingAckMsg.id() == 21, "AckMsg.id()");

			TestUtilities.checkResult(checkingAckMsg.hasFilter(), "AckMsg.hasFilter()");
			
			TestUtilities.checkResult(checkingAckMsg.filter() == 12 , "AckMsg.hasFilter()");

			TestUtilities.checkResult(checkingAckMsg.hasServiceId(), "AckMsg.hasServiceId()");
			
			TestUtilities.checkResult(checkingAckMsg.serviceId() == 5 , "AckMsg.serviceId()");
			
			TestUtilities.checkResult(checkingAckMsg.hasServiceName(), "AckMsg.hasServiceName()");
			
			TestUtilities.checkResult(checkingAckMsg.serviceName().equals(expectedServiceName) , "AckMsg.serviceName()");

			TestUtilities.checkResult(checkingAckMsg.hasNameType(), "AckMsg.hasNameType()");
			
			TestUtilities.checkResult(checkingAckMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

			TestUtilities.checkResult(checkingAckMsg.hasName(), "AckMsg.hasName()");
			
			TestUtilities.checkResult(checkingAckMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");

			TestUtilities.checkResult(checkingAckMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
			
			TestUtilities.checkResult(checkingAckMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
			
			System.out.println("Clears the copied message");
			checkingAckMsg.clear();
			
			int capacity = JUnitTestConnect.getCopiedBufferCapacity(checkingAckMsg);
			TestUtilities.checkResult(capacity == encodedBufferSize, "Checks capacity of the copied buffer after clearing AckMsg");
			TestUtilities.checkResult(JUnitTestConnect.getDataDictionary(checkingAckMsg) == null, "Checks the null reference to DataDictionary after clearing AckMsg");
		}
	}
	
	public void testAckMsg_copy_from_edited_copiedmessage()
	{
		TestUtilities.printTestHead("testAckMsg_copy_from_edited_copiedmessage", "eta encoding ema copying from edited copied message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();
		
		int encodedBufferSize = msgBuf.length();
		
		System.out.println("Begin EMA AckMsg Decoding");

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA AckMsg Copying");
		
		AckMsg copyAckMsg = EmaFactory.createAckMsg(encodedBufferSize);
		
		emaAckMsg.copy(copyAckMsg);
	
		TestUtilities.checkResult(copyAckMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		TestUtilities.checkResult(copyAckMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
		
		AckMsg copyAckMsg2 = EmaFactory.createAckMsg(encodedBufferSize);
		
		/* Edits some properties of the message */
		copyAckMsg.attrib(emaAckMsg);
		copyAckMsg.payload(emaAckMsg);
		
		copyAckMsg.copy(copyAckMsg2);
		
		TestUtilities.checkResult(copyAckMsg2.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG, "AckMsg.attrib().dataType()");
		
		AckMsg attribPayload = copyAckMsg2.attrib().ackMsg();
		
		TestUtilities.checkResult("emaAckMsg.toString().equals(attribPayload.toString())", emaAckMsg.toString().equals(attribPayload.toString()));
		
		TestUtilities.checkResult(copyAckMsg2.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG, "AckMsg.payload().dataType()");
		
		AckMsg payloadMsg = copyAckMsg2.payload().ackMsg();
		
		TestUtilities.checkResult("emaAckMsg.toString().equals(payloadMsg.toString())", emaAckMsg.toString().equals(payloadMsg.toString()));
	}
	
	public void testAckMsg_clone_from_edited_clonedmessage()
	{
		TestUtilities.printTestHead("testAckMsg_copy_from_edited_copiedmessage", "eta encoding ema cloning from edited cloned message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();
		
		System.out.println("Begin EMA AckMsg Decoding");

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA AckMsg Copying");
		
		AckMsg cloneAckMsg = EmaFactory.createAckMsg(emaAckMsg);
	
		TestUtilities.checkResult(cloneAckMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		TestUtilities.checkResult(cloneAckMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
		
		/* Edits some properties of the message */
		cloneAckMsg.attrib(emaAckMsg);
		cloneAckMsg.payload(emaAckMsg);
		
		AckMsg cloneAckMsg2 = EmaFactory.createAckMsg(cloneAckMsg);
		
		TestUtilities.checkResult(cloneAckMsg2.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG, "AckMsg.attrib().dataType()");
		
		AckMsg attribPayload = cloneAckMsg2.attrib().ackMsg();
		
		TestUtilities.checkResult("emaAckMsg.toString().equals(attribPayload.toString())", emaAckMsg.toString().equals(attribPayload.toString()));
		
		TestUtilities.checkResult(cloneAckMsg2.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.ACK_MSG, "AckMsg.payload().dataType()");
		
		AckMsg payloadMsg = cloneAckMsg2.payload().ackMsg();
		
		TestUtilities.checkResult("emaAckMsg.toString().equals(payloadMsg.toString())", emaAckMsg.toString().equals(payloadMsg.toString()));
	}
	
	public void testAckMsg_invalid_destination_AckMsg()
	{
		TestUtilities.printTestHead("testAckMsg_invalid_destination_AckMsg", "eta encoding and ema copying with invalid destination ack message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAAckMsg();
		
		System.out.println("Begin EMA AckMsg Decoding");

		com.refinitiv.ema.access.AckMsg emaAckMsg = JUnitTestConnect.createAckMsg();

		JUnitTestConnect.setRsslData(emaAckMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA AckMsg Copying with EMA encoding message");
		
		System.out.println("Create EMA AckMsg for encoding");
		AckMsg copyAckMsg = EmaFactory.createAckMsg(); 
		
		Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> emaAckMsg.copy(copyAckMsg));
		
		OmmInvalidUsageException OIU = (OmmInvalidUsageException)exception;
		
		TestUtilities.checkResult(OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT == OIU.errorCode(), "Checks OmmInvalidUsageException.errorCode()");
		
		TestUtilities.checkResult("The passed in destination message is used for encoding only on com.refinitiv.ema.access.AckMsgImpl.copy(AckMsg destAckMsg)".equals(exception.getMessage()), "Checks OmmInvalidUsageException.getMessage()");
	}
	
	public void testAckMsg_EmaEncode_Clone()
	{
		TestUtilities.printTestHead("testAckMsg_EmaEncode_Clone", "Ema encoding and ema cloning message");
		
		AckMsg emaAckMsg = encodeEMAAckMsg();
		
		AckMsg clonedMsg = EmaFactory.createAckMsg(emaAckMsg);
		
		TestUtilities.checkResult(clonedMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(clonedMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(clonedMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(clonedMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(clonedMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(clonedMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(clonedMsg.nackCode() == com.refinitiv.ema.access.AckMsg.NackCode.ACCESS_DENIED, "AckMsg.nackCode()");

		TestUtilities.checkResult(clonedMsg.hasText(), "AckMsg.hasText()");
		
		TestUtilities.checkResult(clonedMsg.text().compareTo("access denied") == 0, "AckMsg.text()");

		TestUtilities.checkResult(clonedMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(clonedMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(clonedMsg.id() == 21, "AckMsg.id()");

		TestUtilities.checkResult(clonedMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(clonedMsg.filter() == 12 , "AckMsg.hasFilter()");

		TestUtilities.checkResult(clonedMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(clonedMsg.serviceId() == 5 , "AckMsg.serviceId()");
		
		TestUtilities.checkResult(clonedMsg.hasServiceName(), "AckMsg.hasServiceName()");
		
		TestUtilities.checkResult(clonedMsg.serviceName().equals("DIRECT_FEED") , "AckMsg.serviceName()");

		TestUtilities.checkResult(clonedMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(clonedMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(clonedMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(clonedMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");
		
		TestUtilities.checkResult(clonedMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(clonedMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
	}
	
	public void testAckMsg_EmaEncode_Copy()
	{
		TestUtilities.printTestHead("testAckMsg_EmaEncode_Copy", "Ema encoding and ema copying message");
		
		AckMsg emaAckMsg = encodeEMAAckMsg();
		
		AckMsg copiedMsg = EmaFactory.createAckMsg(1024);
		
		emaAckMsg.copy(copiedMsg);
		
		TestUtilities.checkResult(copiedMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "AckMsg.domainType()");
		
		TestUtilities.checkResult(copiedMsg.streamId() == 15, "AckMsg.streamId()");

		TestUtilities.checkResult(copiedMsg.ackId() == 321, "AckMsg.ackId()");
		
		TestUtilities.checkResult(copiedMsg.hasSeqNum(), "AckMsg.hasSeqNum()");
		
		TestUtilities.checkResult(copiedMsg.seqNum() == 22, "AckMsg.seqNum()");

		TestUtilities.checkResult(copiedMsg.hasNackCode(), "AckMsg.hasNackCode()");
		
		TestUtilities.checkResult(copiedMsg.nackCode() == com.refinitiv.ema.access.AckMsg.NackCode.ACCESS_DENIED, "AckMsg.nackCode()");

		TestUtilities.checkResult(copiedMsg.hasText(), "AckMsg.hasText()");
		
		TestUtilities.checkResult(copiedMsg.text().compareTo("access denied") == 0, "AckMsg.text()");

		TestUtilities.checkResult(copiedMsg.hasMsgKey(), "AckMsg.hasMsgKey()");

		TestUtilities.checkResult(copiedMsg.hasId(), "AckMsg.hasId()");
		
		TestUtilities.checkResult(copiedMsg.id() == 21, "AckMsg.id()");

		TestUtilities.checkResult(copiedMsg.hasFilter(), "AckMsg.hasFilter()");
		
		TestUtilities.checkResult(copiedMsg.filter() == 12 , "AckMsg.hasFilter()");

		TestUtilities.checkResult(copiedMsg.hasServiceId(), "AckMsg.hasServiceId()");
		
		TestUtilities.checkResult(copiedMsg.serviceId() == 5 , "AckMsg.serviceId()");
		
		TestUtilities.checkResult(copiedMsg.hasServiceName(), "AckMsg.hasServiceName()");
		
		TestUtilities.checkResult(copiedMsg.serviceName().equals("DIRECT_FEED") , "AckMsg.serviceName()");

		TestUtilities.checkResult(copiedMsg.hasNameType(), "AckMsg.hasNameType()");
		
		TestUtilities.checkResult(copiedMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "AckMsg.nameType()");

		TestUtilities.checkResult(copiedMsg.hasName(), "AckMsg.hasName()");
		
		TestUtilities.checkResult(copiedMsg.name().compareTo("ABCDEF") == 0, "AckMsg.name()");
		
		TestUtilities.checkResult(copiedMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.attrib().dataType()");
		
		TestUtilities.checkResult(copiedMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "AckMsg.payload().dataType()");
	}


}
