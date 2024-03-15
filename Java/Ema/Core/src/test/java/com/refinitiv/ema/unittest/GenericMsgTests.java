///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.rdm.DomainTypes;

import junit.framework.TestCase;

public class GenericMsgTests extends TestCase
{
	public GenericMsgTests(String name)
	{
		super(name);
	}
	
	public void testGenericMsg_Decode()
	{
		TestUtilities.printTestHead("testGenericMsg_Decode", "eta encoding ema decoding");
		
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin ETA GenericMsg Set");
		com.refinitiv.eta.codec.GenericMsg genericMsg = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		genericMsg.streamId( 15 );
		
		genericMsg.applyHasPartNum();
		genericMsg.partNum( 10 );
		
		genericMsg.applyHasSeqNum();
		genericMsg.seqNum( 22 );

		genericMsg.applyHasSecondarySeqNum();
		genericMsg.secondarySeqNum(123);
		
		genericMsg.applyMessageComplete();
		
		genericMsg.applyHasMsgKey();

		genericMsg.msgKey().applyHasName();
		genericMsg.msgKey().name().data( "ABCDEF" );
		
		genericMsg.msgKey().applyHasNameType();
		genericMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		genericMsg.msgKey().encodedAttrib(fieldListBuf);
	
		genericMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		genericMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA GenericMsg Set");
		System.out.println();

		System.out.println("Begin ETA GenericMsg Buffer Encoding");

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
			return;
		}
		
		genericMsg.encode(encIter);

	    System.out.println("End ETA GenericMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA GenericMsg Decoding");

		com.refinitiv.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();
		
		JUnitTestConnect.setRsslData(emaGenericMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaGenericMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "GenericMsg.domainType()");
		
		TestUtilities.checkResult(emaGenericMsg.streamId() == 15, "GenericMsg.streamId()");

		TestUtilities.checkResult(emaGenericMsg.hasPartNum(), "GenericMsg.hasPartNum()");
		
		TestUtilities.checkResult(emaGenericMsg.partNum() == 10, "GenericMsg.partNum()");

		TestUtilities.checkResult(emaGenericMsg.hasSeqNum(), "GenericMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaGenericMsg.seqNum() == 22, "GenericMsg.seqNum()");

		TestUtilities.checkResult(emaGenericMsg.hasSecondarySeqNum(), "GenericMsg.hasSecondarySeqNum()");
		
		TestUtilities.checkResult(emaGenericMsg.secondarySeqNum() == 123, "GenericMsg.secondarySeqNum()");

		TestUtilities.checkResult(emaGenericMsg.complete(), "GenericMsg.complete()");
		
		TestUtilities.checkResult(emaGenericMsg.hasMsgKey(), "GenericMsg.hasMsgKey()");

		TestUtilities.checkResult(emaGenericMsg.hasId(), "GenericMsg.hasId()");
		
		TestUtilities.checkResult(emaGenericMsg.id() == 21, "GenericMsg.id()");

		TestUtilities.checkResult(emaGenericMsg.hasFilter(), "GenericMsg.hasFilter()");
		
		TestUtilities.checkResult(emaGenericMsg.filter() == 12 , "GenericMsg.hasFilter()");

		TestUtilities.checkResult(emaGenericMsg.hasServiceId(), "GenericMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaGenericMsg.serviceId() == 5 , "GenericMsg.serviceId()");

		TestUtilities.checkResult(emaGenericMsg.hasNameType(), "GenericMsg.hasNameType()");
		
		TestUtilities.checkResult(emaGenericMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "GenericMsg.nameType()");

		TestUtilities.checkResult(emaGenericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(emaGenericMsg.name().compareTo("ABCDEF") == 0, "GenericMsg.name()");

		TestUtilities.checkResult(emaGenericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaGenericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg Decoding");
		System.out.println();
	}

	public void testGenericMsg_toString()
	{
		TestUtilities.printTestHead("testGenericMsg_toString", "eta encoding ema toString");
		
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin ETA GenericMsg Set");
		com.refinitiv.eta.codec.GenericMsg genericMsg = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		genericMsg.streamId( 15 );
		
		genericMsg.applyHasPartNum();
		genericMsg.partNum( 10 );
		
		genericMsg.applyHasSeqNum();
		genericMsg.seqNum( 22 );

		genericMsg.applyHasSecondarySeqNum();
		genericMsg.secondarySeqNum(123);

		genericMsg.applyMessageComplete();
		
		genericMsg.applyHasMsgKey();

		genericMsg.msgKey().applyHasName();
		genericMsg.msgKey().name().data( "ABCDEF" );
		
		genericMsg.msgKey().applyHasNameType();
		genericMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		genericMsg.msgKey().encodedAttrib(fieldListBuf);
	
		genericMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		genericMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA GenericMsg Set");
		System.out.println();

		System.out.println("Begin ETA GenericMsg Buffer Encoding");

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
			return;
		}
		
		genericMsg.encode(encIter);

	    System.out.println("End ETA GenericMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA GenericMsg toString");

		com.refinitiv.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();
		
		JUnitTestConnect.setRsslData(emaGenericMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaGenericMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("GenericMsg.toString() != toString()", !(emaGenericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));
		
		System.out.println("End EMA GenericMsg toString");
		System.out.println();
	}
	
	public void testGenericMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testGenericMsg_EncodeDecode", "ema encoding ema decoding");

		String genericMsgString = "GenericMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    MessageComplete\n" +
				"    seqNum=\"22\"\n" +
				"    secondarySeqNum=\"123\"\n" +
				"    partNum=\"10\"\n" +
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
				"GenericMsgEnd\n";

		String genericMsgEmptyString = "GenericMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"GenericMsgEnd\n";

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.GenericMsg genericMsg = EmaFactory.createGenericMsg();

		com.refinitiv.ema.access.GenericMsg genericMsgEmpty = EmaFactory.createGenericMsg();

		System.out.println("Begin EMA GenericMsg test after constructor");

		TestUtilities.checkResult(genericMsg.domainType() == DomainTypes.MARKET_PRICE, "GenericMsg.domainType()");
		
		TestUtilities.checkResult(genericMsg.streamId() == 0, "GenericMsg.streamId()");

		TestUtilities.checkResult(!genericMsg.hasPartNum(), "GenericMsg.hasPartNum()");
		
		TestUtilities.checkResult(!genericMsg.hasSeqNum(), "GenericMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!genericMsg.hasSecondarySeqNum(), "GenericMsg.hasSecondarySeqNum()");

		TestUtilities.checkResult(!genericMsg.complete(), "GenericMsg.complete()");

		TestUtilities.checkResult(!genericMsg.hasMsgKey(), "GenericMsg.hasMsgKey()");

		TestUtilities.checkResult(!genericMsg.hasId(), "GenericMsg.hasId()");
		
		TestUtilities.checkResult(!genericMsg.hasFilter(), "GenericMsg.hasFilter()");
		
		TestUtilities.checkResult(!genericMsg.hasServiceId(), "GenericMsg.hasServiceId()");
		
		TestUtilities.checkResult(!genericMsg.hasNameType(), "GenericMsg.hasNameType()");
		
		TestUtilities.checkResult(!genericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA GenericMsg Set");
	    
    	genericMsg.domainType( com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.streamId( 15 );
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.partNum( 10 );
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.seqNum( 22 );
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		genericMsg.secondarySeqNum(123);
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.complete(true);
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.name("ABCDEF");
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		genericMsg.serviceId(5);
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		genericMsg.filter( 12 );
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		genericMsg.id(21);
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		genericMsg.attrib(fl);
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		genericMsg.payload(fl);
		TestUtilities.checkResult("GenericMsg.toString() == toString()", genericMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		System.out.println("End EMA GenericMsg Set");
		System.out.println();

		System.out.println("Begin EMA GenericMsg Decoding");

		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("GenericMsg.toString(dictionary) == toString(dictionary)", genericMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("GenericMsg.toString(dictionary) == toString(dictionary)", genericMsgEmpty.toString(emaDataDictionary).equals(genericMsgEmptyString));

		TestUtilities.checkResult("GenericMsg.toString(dictionary) == toString(dictionary)", genericMsg.toString(emaDataDictionary).equals(genericMsgString));

		com.refinitiv.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();

		JUnitTestConnect.setRsslData(emaGenericMsg, genericMsg, 14, 0, dictionary, null);

		com.refinitiv.ema.access.GenericMsg genericMsgClone = EmaFactory.createGenericMsg(genericMsg);
		genericMsgClone.clear();
		TestUtilities.checkResult("GenericMsg.toString(dictionary) == toString(dictionary)", genericMsgClone.toString(emaDataDictionary).equals(genericMsgEmptyString));

		TestUtilities.checkResult(emaGenericMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "GenericMsg.domainType()");
		
		TestUtilities.checkResult(emaGenericMsg.streamId() == 15, "GenericMsg.streamId()");

		TestUtilities.checkResult(emaGenericMsg.hasPartNum(), "GenericMsg.hasPartNum()");
		
		TestUtilities.checkResult(emaGenericMsg.partNum() == 10, "GenericMsg.partNum()");

		TestUtilities.checkResult(emaGenericMsg.hasSeqNum(), "GenericMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaGenericMsg.seqNum() == 22, "GenericMsg.seqNum()");

		TestUtilities.checkResult(emaGenericMsg.hasSecondarySeqNum(), "GenericMsg.hasSecondarySeqNum()");
		
		TestUtilities.checkResult(emaGenericMsg.secondarySeqNum() == 123, "GenericMsg.secondarySeqNum()");

		TestUtilities.checkResult(emaGenericMsg.complete(), "GenericMsg.complete()");

		TestUtilities.checkResult(emaGenericMsg.hasMsgKey(), "GenericMsg.hasMsgKey()");

		TestUtilities.checkResult(emaGenericMsg.hasId(), "GenericMsg.hasId()");
		
		TestUtilities.checkResult(emaGenericMsg.id() == 21, "GenericMsg.id()");

		TestUtilities.checkResult(emaGenericMsg.hasFilter(), "GenericMsg.hasFilter()");
		
		TestUtilities.checkResult(emaGenericMsg.filter() == 12 , "GenericMsg.hasFilter()");

		TestUtilities.checkResult(emaGenericMsg.hasServiceId(), "GenericMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaGenericMsg.serviceId() == 5 , "GenericMsg.serviceId()");

		TestUtilities.checkResult(emaGenericMsg.hasNameType(), "GenericMsg.hasNameType()");
		
		TestUtilities.checkResult(emaGenericMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "GenericMsg.nameType()");

		TestUtilities.checkResult(emaGenericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(emaGenericMsg.name().compareTo("ABCDEF") == 0, "GenericMsg.name()");

		TestUtilities.checkResult(emaGenericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaGenericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg Decoding");
		System.out.println();

		genericMsg.clear();
		
		System.out.println("Begin EMA GenericMsg test after clear");

		TestUtilities.checkResult(genericMsg.domainType() == DomainTypes.MARKET_PRICE, "GenericMsg.domainType()");
		
		TestUtilities.checkResult(genericMsg.streamId() == 0, "GenericMsg.streamId()");

		TestUtilities.checkResult(!genericMsg.hasPartNum(), "GenericMsg.hasPartNum()");
		
		TestUtilities.checkResult(!genericMsg.hasSeqNum(), "GenericMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!genericMsg.hasSecondarySeqNum(), "GenericMsg.hasSecondarySeqNum()");

		TestUtilities.checkResult(!genericMsg.complete(), "GenericMsg.complete()");

		TestUtilities.checkResult(!genericMsg.hasMsgKey(), "GenericMsg.hasMsgKey()");

		TestUtilities.checkResult(!genericMsg.hasId(), "GenericMsg.hasId()");
		
		TestUtilities.checkResult(!genericMsg.hasFilter(), "GenericMsg.hasFilter()");
		
		TestUtilities.checkResult(!genericMsg.hasServiceId(), "GenericMsg.hasServiceId()");
		
		TestUtilities.checkResult(!genericMsg.hasNameType(), "GenericMsg.hasNameType()");
		
		TestUtilities.checkResult(!genericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(genericMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(genericMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg test after clear");
		System.out.println();
	}
	
	public void testGenericMsg_EncodeETAGenericMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETAGenericMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for FieldList as attrib, extended header and payload. Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode Msg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyGenericMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyGenericMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestGenericMsg_EncodeETAGenericMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeETAGenericMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETAGenericMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyGenericMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyGenericMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestGenericMsg_EncodeETAGenericMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeETAGenericMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETAGenericMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for FilterList as attrib, payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyGenericMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyGenericMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestGenericMsg_EncodeETAGenericMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeETAGenericMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETAGenericMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for Series as attrib, payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyGenericMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyGenericMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeETAGenericMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeETAGenericMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETAGenericMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for Vector as attrib, payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyGenericMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyGenericMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeETAGenericMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for Map as attrib, payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyGenericMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyGenericMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeETAGenericMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeETAGenericMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with ETA for RefreshMsg as attrib, payload. Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeGenericMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.refinitiv.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAGenericMsgAll(genericMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.refinitiv.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyGenericMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyGenericMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeETAGenericMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}

	public void testGenericMsgMsg_cloneIsNotSupportedFromTheEncodeSide()
	{
		TestUtilities.printTestHead("testGenericMsgMsg_cloneIsNotSupportedFromTheEncodeSide", "cloning is not supported on encode side");
		GenericMsg msg = EmaFactory.createGenericMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE);

		try {
			GenericMsg cloneMessage = EmaFactory.createGenericMsg(msg);
			TestUtilities.checkResult(false, "Clone not supported - exception expected: ");
		} catch ( OmmException excp ) {
			TestUtilities.checkResult(true, "Clone not supported  - exception expected: " +  excp.getMessage() );
			TestUtilities.checkResult(excp.getMessage().startsWith("Failed to clone empty encoded buffer"), "Clone not supported - exception text validated");
		}
	}

	public void testGenericMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testGenericMsg_cloneMsgKeyWLScenario", "cloning for minimal ema generic message");
		GenericMsg emaMsg = EmaFactory.createGenericMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, true);
		GenericMsg emaClonedMsg = EmaFactory.createGenericMsg(emaMsg);

		compareEmaGenericMsgFields(emaMsg, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, false);

		System.out.println("End EMA GenericMsg Clone msgKey");
		System.out.println();
	}

	public void testGenericMsg_clone()
	{
		TestUtilities.printTestHead("testGenericMsg_clone", "cloning for ema generic message");
		
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin ETA GenericMsg Set");
		com.refinitiv.eta.codec.GenericMsg genericMsg = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		genericMsg.streamId( 15 );
		
		genericMsg.applyHasPartNum();
		genericMsg.partNum( 10 );
		
		genericMsg.applyHasSeqNum();
		genericMsg.seqNum( 22 );

		genericMsg.applyHasSecondarySeqNum();
		genericMsg.secondarySeqNum(123);

		genericMsg.applyMessageComplete();
		
		genericMsg.applyHasMsgKey();

		genericMsg.msgKey().applyHasName();
		genericMsg.msgKey().name().data( "ABCDEF" );
		
		genericMsg.msgKey().applyHasNameType();
		genericMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		genericMsg.msgKey().encodedAttrib(fieldListBuf);
	
		genericMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		genericMsg.encodedDataBody(fieldListBuf);

		setMoreFields(genericMsg);

		System.out.println("End ETA GenericMsg Set");
		System.out.println();

		System.out.println("Begin ETA GenericMsg Buffer Encoding");

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
			return;
		}
		
		genericMsg.encode(encIter);

	    System.out.println("End ETA GenericMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA GenericMsg Clone");
		com.refinitiv.eta.codec.GenericMsg genericMsgDecode = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		genericMsgDecode.decode(decIter);

		com.refinitiv.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();
				
		JUnitTestConnect.setRsslData(emaGenericMsg, genericMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.GenericMsg emaGenericMsgClone = EmaFactory.createGenericMsg(emaGenericMsg);
		
		compareEmaGenericMsgFields(emaGenericMsg, emaGenericMsgClone, "Generic cloned message");
		String emaGenericMsgString = emaGenericMsg.toString();
		String emaGenericMsgCloneString = emaGenericMsgClone.toString();
		
		System.out.println("Cloned EMA GenericMsg:");
		System.out.println(emaGenericMsgClone);
		
		TestUtilities.checkResult(emaGenericMsgString.equals(emaGenericMsgCloneString), "emaGenericMsgString.equals(emaGenericMsgCloneString)");

		com.refinitiv.ema.access.GenericMsg emaGenericMsgClone2 = EmaFactory.createGenericMsg(emaGenericMsgClone);
		compareEmaGenericMsgFields(emaGenericMsg, emaGenericMsgClone2, "Generic double-cloned message");
		String emaGenericMsgClone2String = emaGenericMsgClone2.toString();
		TestUtilities.checkResult(emaGenericMsgString.equals(emaGenericMsgClone2String), "double-cloned check emaGenericMsgString.equals(emaGenericMsgClone2String)");

		System.out.println("End EMA GenericMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.refinitiv.eta.codec.GenericMsg genericMsg) {
		genericMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		genericMsg.extendedHeader(extendedHeader);

		genericMsg.applyHasPermData();
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data(ByteBuffer.wrap(new byte[]{50, 51, 52, 53}));
		genericMsg.permData(permissionData);

		genericMsg.applyMessageComplete();
	}

	public void testGenericMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testGenericMsg_cloneEdit", "clone and edit ema generic message");
		
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin ETA GenericMsg Set");
		com.refinitiv.eta.codec.GenericMsg genericMsg = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		genericMsg.streamId( 15 );
		
		genericMsg.applyHasPartNum();
		genericMsg.partNum( 10 );
		
		genericMsg.applyHasSeqNum();
		genericMsg.seqNum( 22 );

		genericMsg.applyHasSecondarySeqNum();
		genericMsg.secondarySeqNum(123);

		genericMsg.applyMessageComplete();
		
		genericMsg.applyHasMsgKey();

		genericMsg.msgKey().applyHasName();
		genericMsg.msgKey().name().data( "ABCDEF" );
		
		genericMsg.msgKey().applyHasNameType();
		genericMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		genericMsg.msgKey().encodedAttrib(fieldListBuf);
	
		genericMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		genericMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA GenericMsg Set");
		System.out.println();

		System.out.println("Begin ETA GenericMsg Buffer Encoding");

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
			return;
		}
		
		genericMsg.encode(encIter);

	    System.out.println("End ETA GenericMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA GenericMsg Clone");
		com.refinitiv.eta.codec.GenericMsg genericMsgDecode = (com.refinitiv.eta.codec.GenericMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		genericMsgDecode.decode(decIter);

		com.refinitiv.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();
				
		JUnitTestConnect.setRsslData(emaGenericMsg, genericMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.GenericMsg emaGenericMsgClone = EmaFactory.createGenericMsg(emaGenericMsg);
		
		TestUtilities.checkResult(emaGenericMsgClone.domainType() == emaGenericMsg.domainType(), "Compare domainType");
		TestUtilities.checkResult(emaGenericMsgClone.streamId() == emaGenericMsg.streamId(), "Compare streamId");
		TestUtilities.checkResult(emaGenericMsgClone.hasPartNum() == emaGenericMsg.hasPartNum(), "Compare hasPartNum");
		TestUtilities.checkResult(emaGenericMsgClone.partNum() == emaGenericMsg.partNum(), "Compare partNum");
		TestUtilities.checkResult(emaGenericMsgClone.hasSeqNum() == emaGenericMsg.hasSeqNum(), "Compare hasSeqNum");
		TestUtilities.checkResult(emaGenericMsgClone.seqNum() == emaGenericMsg.seqNum(), "Compare seqNum");
		TestUtilities.checkResult(emaGenericMsgClone.hasSecondarySeqNum() == emaGenericMsg.hasSecondarySeqNum(), "Compare hasSecondarySeqNum");
		TestUtilities.checkResult(emaGenericMsgClone.secondarySeqNum() == emaGenericMsg.secondarySeqNum(), "Compare secondarySeqNum");
		TestUtilities.checkResult(emaGenericMsgClone.complete() == emaGenericMsg.complete(), "Compare complete");
		TestUtilities.checkResult(emaGenericMsgClone.hasMsgKey() == emaGenericMsg.hasMsgKey(), "Compare hasMsgKey");
		
		String emaGenericMsgString = emaGenericMsg.toString();
		String emaGenericMsgCloneString = emaGenericMsgClone.toString();
		
		System.out.println("Cloned EMA GenericMsg:");
		System.out.println(emaGenericMsgClone);
		
		TestUtilities.checkResult(emaGenericMsgString.equals(emaGenericMsgCloneString), "emaGenericMsgString.equals(emaGenericMsgCloneString)");
		
		emaGenericMsgClone.streamId(10);
		emaGenericMsgClone.payload().fieldList().add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		TestUtilities.checkResult(emaGenericMsgClone.streamId() != emaGenericMsg.streamId(), "Compare streamId");

		// Check emaGenericMsg for no FID 21
		Iterator<FieldEntry> iter = emaGenericMsg.payload().fieldList().iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			TestUtilities.checkResult(fieldEntry.fieldId() != 21, "Check emaGenericMsg for no FID 21");
		}
		
		boolean foundFid = false;
		Iterator<FieldEntry> iterClone = emaGenericMsgClone.payload().fieldList().iterator();
		while (iterClone.hasNext())
		{
			fieldEntry = iterClone.next();
			if(foundFid = fieldEntry.fieldId() == 21)
				break;
		}
		
		TestUtilities.checkResult(foundFid, "Check emaGenericMsgClone for FID 21");
		
		emaGenericMsgString = emaGenericMsg.toString();
		emaGenericMsgCloneString = emaGenericMsgClone.toString();
		
		TestUtilities.checkResult(!emaGenericMsgString.equals(emaGenericMsgCloneString), "Check that emaGenericMsgString does not equal emaGenericMsgCloneString");

		ByteBuffer permData = ByteBuffer.wrap(new byte[]{16, 32, 48});
		emaGenericMsgClone.permissionData(permData);
		GenericMsg emaGenericMsgClone2 = EmaFactory.createGenericMsg(emaGenericMsgClone);
		TestUtilities.checkResult(permData.equals(emaGenericMsgClone2.permissionData()), "clone2 permdata should match clone alter");
		
		System.out.println("End EMA GenericMsg Clone");
		System.out.println();
	}
	private void compareEmaGenericMsgFields(GenericMsg expected, GenericMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + " base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasPermissionData() == actual.hasPermissionData(), checkPrefix + "hasPermissionData");
		if(expected.hasPermissionData())
			TestUtilities.checkResult(expected.permissionData().equals(actual.permissionData()), checkPrefix + "permissionData");

		TestUtilities.checkResult(expected.hasSeqNum() == actual.hasSeqNum(), checkPrefix + "hasSeqNum");
		if(expected.hasSeqNum())
			TestUtilities.checkResult(expected.seqNum() == actual.seqNum(), checkPrefix + "seqNum");

		TestUtilities.checkResult(expected.hasPartNum() == actual.hasPartNum(), checkPrefix + "hasPartNum");
		if(expected.hasPartNum())
			TestUtilities.checkResult(expected.partNum() == actual.partNum(), checkPrefix + "partNum");

		TestUtilities.checkResult(expected.hasSecondarySeqNum() == actual.hasSecondarySeqNum(), checkPrefix + "hasSecondarySeqNum");
		if(expected.hasSecondarySeqNum())
			TestUtilities.checkResult(expected.secondarySeqNum() == actual.secondarySeqNum(), checkPrefix + "secondarySeqNum");

		TestUtilities.checkResult(expected.complete() == actual.complete(), checkPrefix + "complete");
	}

}
