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
import com.refinitiv.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class UpdateMsgTests extends TestCase
{
	private static com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();

	public UpdateMsgTests(String name)
	{
		super(name);
	}
	
	private com.refinitiv.eta.codec.Buffer encodeETAUpdateMsg()
	{
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		if(dictionary.numberOfEntries() == 0)
			TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin ETA UpdateMsg Set");
		com.refinitiv.eta.codec.UpdateMsg updateMsg = (com.refinitiv.eta.codec.UpdateMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		updateMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.UPDATE);
		
		updateMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		updateMsg.streamId( 15 );
		
		updateMsg.applyHasSeqNum();
		updateMsg.seqNum( 22 );

		updateMsg.applyHasMsgKey();

		updateMsg.msgKey().applyHasName();
		updateMsg.msgKey().name().data( "ABCDEF" );
		
		updateMsg.msgKey().applyHasNameType();
		updateMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		updateMsg.msgKey().applyHasServiceId();
		updateMsg.msgKey().serviceId(5);
		
		updateMsg.msgKey().applyHasFilter();
		updateMsg.msgKey().filter( 12 );
	
		updateMsg.msgKey().applyHasIdentifier();
		updateMsg.msgKey().identifier(21);
		
		updateMsg.msgKey().applyHasAttrib();
		updateMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		updateMsg.msgKey().encodedAttrib(fieldListBuf);
	
		updateMsg.applyDoNotCache();
		updateMsg.applyDoNotConflate();
		
		updateMsg.applyHasConfInfo();
		updateMsg.conflationCount(10);
		updateMsg.conflationTime(20);
		
		updateMsg.applyHasPostUserInfo();
		updateMsg.postUserInfo().userAddr(15);
		updateMsg.postUserInfo().userId(30);
		
		updateMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		updateMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA UpdateMsg Set");
		System.out.println();

		System.out.println("Begin ETA UpdateMsg Buffer Encoding");

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
		
		updateMsg.encode(encIter);

	    System.out.println("End ETA UpdateMsg Buffer Encoding");
		System.out.println();
		
		return msgBuf;
	}
	
	UpdateMsg encodeEMAUpdateMsg()
	{
		UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
		
		updateMsg.streamId(15);
		updateMsg.seqNum(22);
		
		updateMsg.name("ABCDEF");
		updateMsg.nameType(com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC);
		updateMsg.serviceId(5);
		updateMsg.filter(12);
		updateMsg.id(21);
		updateMsg.serviceName("DIRECT_FEED");
		
		
		updateMsg.doNotCache(true);
		updateMsg.doNotConflate(true);
		updateMsg.conflated(10, 20);
		
		updateMsg.publisherId(30, 15);
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().intValue(1, 5));
		
		updateMsg.attrib(fieldList);
		updateMsg.payload(fieldList);
		
		return updateMsg;
	}

	public void testUpdateMsg_Decode()
	{
		TestUtilities.printTestHead("testUpdateMsg_Decode", "eta encoding ema decoding");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();

		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
		
		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		TestUtilities.checkResult(emaUpdateMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(emaUpdateMsg.streamId() == 15, "UpdateMsg.streamId()");

		TestUtilities.checkResult(emaUpdateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaUpdateMsg.seqNum() == 22, "UpdateMsg.seqNum()");

		TestUtilities.checkResult(emaUpdateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(emaUpdateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(emaUpdateMsg.id() == 21, "UpdateMsg.id()");

		TestUtilities.checkResult(emaUpdateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(emaUpdateMsg.filter() == 12 , "UpdateMsg.hasFilter()");

		TestUtilities.checkResult(emaUpdateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaUpdateMsg.serviceId() == 5 , "UpdateMsg.serviceId()");

		TestUtilities.checkResult(emaUpdateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(emaUpdateMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(emaUpdateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(emaUpdateMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");

		TestUtilities.checkResult(emaUpdateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaUpdateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(emaUpdateMsg.doNotConflate(), "UpdateMsg.doNotconflate()");

		TestUtilities.checkResult(!emaUpdateMsg.doNotRipple(), "UpdateMsg.solicited()");
		
		TestUtilities.checkResult(emaUpdateMsg.hasConflated(), "UpdateMsg.hasConflated()");
	
		TestUtilities.checkResult(emaUpdateMsg.conflatedCount() == 10, "UpdateMsg.conflatedCount()");

		TestUtilities.checkResult(emaUpdateMsg.conflatedTime() == 20, "UpdateMsg.conflatedTime()");

		TestUtilities.checkResult(emaUpdateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");

		TestUtilities.checkResult(emaUpdateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");

		System.out.println("End EMA UpdateMsg Decoding");
		System.out.println();
	}
	
	public void testUpdateMsg_toString()
	{
		TestUtilities.printTestHead("testUpdateMsg_toString", "eta encoding ema toString");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();

		System.out.println("Begin EMA UpdateMsg toString");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
		
		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		System.out.println(emaUpdateMsg);

		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("UpdateMsg.toString() != toString()", !(emaUpdateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));

		System.out.println("End EMA UpdateMsg toString");
		System.out.println();
	}
	
	public void testUpdateMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testUpdateMsg_EncodeDecode", "ema encoding ema decoding");

		String updateMsgString = "UpdateMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    updateTypeNum=\"0\"\n" +
				"    seqNum=\"22\"\n" +
				"    doNotCache\n" +
				"    doNotRipple\n" +
				"    doNotConflate\n" +
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
				"UpdateMsgEnd\n";

		String updateMsgEmptyString = "UpdateMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    updateTypeNum=\"0\"\n" +
				"UpdateMsgEnd\n";

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
	    com.refinitiv.ema.access.UpdateMsg updateMsgEmpty = EmaFactory.createUpdateMsg();

		System.out.println("Begin EMA UpdateMsg test after constructor");

		TestUtilities.checkResult(updateMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(updateMsg.streamId() == 0, "UpdateMsg.streamId()");

		TestUtilities.checkResult(!updateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!updateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(!updateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(!updateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(!updateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(!updateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(!updateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!updateMsg.doNotConflate(), "UpdateMsg.doNotConflate()");

		TestUtilities.checkResult(!updateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(!updateMsg.doNotRipple(), "UpdateMsg.doNotRipple()");

		TestUtilities.checkResult(!updateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.payload().dataType()");

		System.out.println("Begin EMA UpdateMsg test after constructor");
		System.out.println();		
		
	    System.out.println("End EMA UpdateMsg Set");
	    
    	updateMsg.domainType( com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.streamId( 15 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.seqNum( 22 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		updateMsg.name("ABCDEF");
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		updateMsg.serviceId(5);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.filter( 12 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		updateMsg.id(21);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.attrib(fl);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
	    updateMsg.doNotCache(true);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		updateMsg.doNotConflate(true);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		updateMsg.doNotRipple(true);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.publisherId(30,  15);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		updateMsg.payload(fl);
		TestUtilities.checkResult("UpdateMsg.toString() == toString()", updateMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		System.out.println("End EMA UpdateMsg Set");
		System.out.println();

		System.out.println("Begin EMA UpdateMsg Decoding");

		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("UpdateMsg.toString(dictionary) == toString(dictionary)", updateMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("UpdateMsg.toString(dictionary) == toString(dictionary)", updateMsgEmpty.toString(emaDataDictionary).equals(updateMsgEmptyString));

		TestUtilities.checkResult("UpdateMsg.toString(dictionary) == toString(dictionary)", updateMsg.toString(emaDataDictionary).equals(updateMsgString));

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsg, 14, 0, dictionary, null);

		com.refinitiv.ema.access.UpdateMsg updateMsgClone = EmaFactory.createUpdateMsg(updateMsg);
		updateMsgClone.clear();
		TestUtilities.checkResult("UpdateMsg.toString(dictionary) == toString(dictionary)", updateMsgClone.toString(emaDataDictionary).equals(updateMsgEmptyString));

		TestUtilities.checkResult(emaUpdateMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(emaUpdateMsg.streamId() == 15, "UpdateMsg.streamId()");

		TestUtilities.checkResult(emaUpdateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaUpdateMsg.seqNum() == 22, "UpdateMsg.seqNum()");

		TestUtilities.checkResult(emaUpdateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(emaUpdateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(emaUpdateMsg.id() == 21, "UpdateMsg.id()");

		TestUtilities.checkResult(emaUpdateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(emaUpdateMsg.filter() == 12 , "UpdateMsg.hasFilter()");

		TestUtilities.checkResult(emaUpdateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaUpdateMsg.serviceId() == 5 , "UpdateMsg.serviceId()");

		TestUtilities.checkResult(emaUpdateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(emaUpdateMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(emaUpdateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(emaUpdateMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");

		TestUtilities.checkResult(emaUpdateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaUpdateMsg.doNotConflate(), "UpdateMsg.doNotConflate()");

		TestUtilities.checkResult(emaUpdateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(emaUpdateMsg.doNotRipple(), "UpdateMsg.doNotRipple()");

		TestUtilities.checkResult(emaUpdateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaUpdateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");

		System.out.println("End EMA UpdateMsg Decoding");
		System.out.println();

		updateMsg.clear();
		
		System.out.println("Begin EMA UpdateMsg test after clear");

		TestUtilities.checkResult(updateMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(updateMsg.streamId() == 0, "UpdateMsg.streamId()");

		TestUtilities.checkResult(!updateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!updateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(!updateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(!updateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(!updateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(!updateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(!updateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(updateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!updateMsg.doNotConflate(), "UpdateMsg.doNotConflate()");

		TestUtilities.checkResult(!updateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(!updateMsg.doNotRipple(), "UpdateMsg.doNotRipple()");

		TestUtilities.checkResult(!updateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(updateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.payload().dataType()");

		System.out.println("End EMA UpdateMsg test after clear");
		System.out.println();
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for FieldList as attrib, extended header and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     //com.refinitiv.ema.access.UpdateMsg emaUpdateMsgClone = EmaFactory.createUpdateMsg(decCopyUpdateMsg);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyUpdateMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyUpdateMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestUpdateMsg_EncodeETAUpdateWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyUpdateMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyUpdateMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestUpdateMsg_EncodeETAUpdateWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for FilterList as attrib and payload, Encode it to another UpdateMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(12288));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyUpdateMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyUpdateMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestUpdateMsg_EncodeETAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for Series as attrib and payload, Encode it to another UpdateMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(12040));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyUpdateMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyUpdateMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestUpdateMsg_EncodeETAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for Vector as attrib and payload, Encode it to another UpdateMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(12040));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyUpdateMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyUpdateMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\testUpdateMsg_EncodeETAUpdateWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for Map as attrib and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyUpdateMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyUpdateMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestUpdateMsg_EncodeETAUpdateWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeETAUpdateWithRefreshMsgTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeETAUpdateWithRefreshMsgTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with ETA for RefreshMsg as attrib and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeUpdateMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.refinitiv.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAUpdateMsgAll(updateMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.refinitiv.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyUpdateMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyUpdateMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);

	     System.out.println("\ttestUpdateMsg_EncodeETAUpdateWithRefreshMsgTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}

	public void testUpdateMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testUpdateMsg_cloneMsgKeyWLScenario", "cloning for minimal ema update message");
		UpdateMsg emaMsg = EmaFactory.createUpdateMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, true);
		UpdateMsg emaClonedMsg = EmaFactory.createUpdateMsg(emaMsg);

		compareEmaUpdateMsgFields(emaMsg, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, false);

		System.out.println("End EMA UpdateMsg Clone msgKey");
		System.out.println();
	}

		public void testUpdateMsg_clone()
	{
		TestUtilities.printTestHead("testUpdateMsg_clone", "cloning for ema update message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();
		
		System.out.println("Begin EMA UpdateMsg Clone");
		com.refinitiv.eta.codec.UpdateMsg updateMsgDecode = (com.refinitiv.eta.codec.UpdateMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
		updateMsgDecode.decode(decIter);

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
				
		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsgDecode, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		com.refinitiv.ema.access.UpdateMsg emaUpdateMsgClone = EmaFactory.createUpdateMsg(emaUpdateMsg);
		
		compareEmaUpdateMsgFields(emaUpdateMsg, emaUpdateMsgClone, "Update cloned message");

		String emaUpdateMsgString = emaUpdateMsg.toString();
		String emaUpdateMsgCloneString = emaUpdateMsgClone.toString();
		
		System.out.println("Cloned EMA UpdateMsg:");
		System.out.println(emaUpdateMsgClone);
		
		TestUtilities.checkResult(emaUpdateMsgString.equals(emaUpdateMsgCloneString), "emaUpdateMsgString.equals(emaUpdateMsgCloneString)");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsgClone2 = EmaFactory.createUpdateMsg(emaUpdateMsgClone);
		compareEmaUpdateMsgFields(emaUpdateMsg, emaUpdateMsgClone2, "Update double-cloned message");
		String emaUpdateMsgClone2String = emaUpdateMsgClone2.toString();
		TestUtilities.checkResult(emaUpdateMsgString.equals(emaUpdateMsgClone2String), "double-cloned emaUpdateMsgString.equals(emaUpdateMsgClone2String)");

		System.out.println("End EMA UpdateMsg Clone");
		System.out.println();
	}

	public void testUpdateMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testUpdateMsg_cloneEdit", "clone and edit ema update message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();
		
		System.out.println("Begin EMA UpdateMsg Clone");
		com.refinitiv.eta.codec.UpdateMsg updateMsgDecode = (com.refinitiv.eta.codec.UpdateMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
		updateMsgDecode.decode(decIter);

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
				
		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsgDecode, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		com.refinitiv.ema.access.UpdateMsg emaUpdateMsgClone = EmaFactory.createUpdateMsg(emaUpdateMsg);
		
		TestUtilities.checkResult(emaUpdateMsgClone.domainType() == emaUpdateMsg.domainType(), "Compare domainType");
		TestUtilities.checkResult(emaUpdateMsgClone.streamId() == emaUpdateMsg.streamId(), "Compare streamId");
		TestUtilities.checkResult(emaUpdateMsgClone.hasSeqNum() == emaUpdateMsg.hasSeqNum(), "Compare hasSeqNum");
		TestUtilities.checkResult(emaUpdateMsgClone.seqNum() == emaUpdateMsg.seqNum(), "Compare seqNum");
		TestUtilities.checkResult(emaUpdateMsgClone.hasMsgKey() == emaUpdateMsg.hasMsgKey(), "Compare hasMsgKey");
		TestUtilities.checkResult(emaUpdateMsgClone.doNotCache() == emaUpdateMsg.doNotCache(), "Compare doNotCache");
		TestUtilities.checkResult(emaUpdateMsgClone.doNotConflate() == emaUpdateMsg.doNotConflate(), "Compare doNotConflate");
		TestUtilities.checkResult(emaUpdateMsgClone.hasConflated() == emaUpdateMsg.hasConflated(), "Compare hasConfInfo");
		TestUtilities.checkResult(emaUpdateMsgClone.conflatedCount() == emaUpdateMsg.conflatedCount(), "Compare conflationCount");
		TestUtilities.checkResult(emaUpdateMsgClone.conflatedTime() == emaUpdateMsg.conflatedTime(), "Compare conflationTime");
		
		String emaUpdateMsgString = emaUpdateMsg.toString();
		String emaUpdateMsgCloneString = emaUpdateMsgClone.toString();
		
		System.out.println("Cloned EMA UpdateMsg:");
		System.out.println(emaUpdateMsgClone);
		
		TestUtilities.checkResult(emaUpdateMsgString.equals(emaUpdateMsgCloneString), "emaUpdateMsgString.equals(emaUpdateMsgCloneString)");
				
		emaUpdateMsgClone.streamId(10);
		emaUpdateMsgClone.payload().fieldList().add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		TestUtilities.checkResult(emaUpdateMsgClone.streamId() != emaUpdateMsg.streamId(), "Compare streamId");

		// Check emaUpdateMsg for no FID 21
		Iterator<FieldEntry> iter = emaUpdateMsg.payload().fieldList().iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			TestUtilities.checkResult(fieldEntry.fieldId() != 21, "Check emaUpdateMsg for no FID 21");
		}
		
		boolean foundFid = false;
		Iterator<FieldEntry> iterClone = emaUpdateMsgClone.payload().fieldList().iterator();
		while (iterClone.hasNext())
		{
			fieldEntry = iterClone.next();
			if(foundFid = fieldEntry.fieldId() == 21)
				break;
		}
		
		TestUtilities.checkResult(foundFid, "Check emaUpdateMsgClone for FID 21");
		
		emaUpdateMsgString = emaUpdateMsg.toString();
		emaUpdateMsgCloneString = emaUpdateMsgClone.toString();
		
		TestUtilities.checkResult(!emaUpdateMsgString.equals(emaUpdateMsgCloneString), "Check that emaUpdateMsgString does not equal emaUpdateMsgCloneString");
		
		System.out.println("End EMA UpdateMsg Clone");
		System.out.println();
	}

	private void compareEmaUpdateMsgFields(UpdateMsg expected, UpdateMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + "base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasConflated() == actual.hasConflated(), "hasConflated");
		if(expected.hasConflated()) {
			TestUtilities.checkResult(expected.conflatedCount() == actual.conflatedCount(), "conflatedCount");
			TestUtilities.checkResult(expected.conflatedTime() == actual.conflatedTime(), "conflatedTime");
		}

		TestUtilities.checkResult(expected.doNotCache() == actual.doNotCache(), "doNotCache");
		TestUtilities.checkResult(expected.doNotConflate() == actual.doNotConflate(), "doNotConflate");

		TestUtilities.checkResult(expected.hasServiceName() == actual.hasServiceName(), checkPrefix + "hasServiceName");
		if(expected.hasServiceName())
			TestUtilities.checkResult(expected.serviceName().equals(actual.serviceName()), checkPrefix + "serviceId" + "exp=" +actual.serviceName() + " act="+expected.serviceName());

		TestUtilities.checkResult(expected.hasPublisherId() == actual.hasPublisherId(), checkPrefix + "hasPublisherId");
		if(expected.hasPublisherId()) {
			TestUtilities.checkResult(expected.publisherIdUserId() == actual.publisherIdUserId(), checkPrefix + "publisherIdUserId");
			TestUtilities.checkResult(expected.publisherIdUserAddress() == actual.publisherIdUserAddress(), checkPrefix + "publisherIdUserAddress");
		}

		TestUtilities.checkResult(expected.hasPermissionData() == actual.hasPermissionData(), checkPrefix + "hasPermissionData");
		if(expected.hasPermissionData())
			TestUtilities.checkResult(expected.permissionData().equals(actual.permissionData()), checkPrefix + "permissionData");

		TestUtilities.checkResult(expected.hasSeqNum() == actual.hasSeqNum(), checkPrefix + "hasSeqNum");
		if(expected.hasSeqNum())
			TestUtilities.checkResult(expected.seqNum() == actual.seqNum(), checkPrefix + "seqNum");
	}
	
	public void testUpdateMsg_InitCopyingMessageWithUnknownSize()
	{
		TestUtilities.printTestHead("testUpdateMsg_InitCopyingMessageWithUnknownSize", "Init copying message with unknown size");
		
		UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg(-1);
		int capacity = JUnitTestConnect.getCopiedBufferCapacity(copyUpdateMsg);
		
		TestUtilities.checkResult(capacity == 0, "Checks capacity of the copied buffer");
		
		copyUpdateMsg.clear();
		
		capacity = JUnitTestConnect.getCopiedBufferCapacity(copyUpdateMsg);
		
		TestUtilities.checkResult(capacity == 0, "Checks capacity of the copied buffer after clearing");
	}
	
	public void testUpdateMsg_InitCopyingMessage()
	{
		TestUtilities.printTestHead("testUpdateMsg_InitCopyingMessage", "Init copying message with a specific size");
		
		UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg(1024);
		int capacity = JUnitTestConnect.getCopiedBufferCapacity(copyUpdateMsg);
		
		TestUtilities.checkResult(capacity == 1024, "Checks capacity of copied buffer");
		copyUpdateMsg.clear();
		
		capacity = JUnitTestConnect.getCopiedBufferCapacity(copyUpdateMsg);
		TestUtilities.checkResult(capacity == 1024, "Checks capacity of the copied buffer after clearing UpdateMsg");
	}
	
	public void testUpdateMsg_copy_from_decodingUpdateMsg_and_copied_message()
	{
		TestUtilities.printTestHead("testUpdateMsg_copy_from_decodingUpdateMsg_and_copied_message", "eta encoding ema copying");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();
		
		int encodedBufferSize = msgBuf.length();
		
		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		String expectedServiceName = "DIRECT_FEED";
		
		/* Set the service name as well */
		emaUpdateMsg.serviceName(expectedServiceName);
		
		System.out.println("Begin EMA RefreshMsg Copying");
		
		UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg(encodedBufferSize);
		
		emaUpdateMsg.copy(copyUpdateMsg);
		
		UpdateMsg copyUpdateMsg2 = EmaFactory.createUpdateMsg(encodedBufferSize);
		
		copyUpdateMsg.copy(copyUpdateMsg2);
		
		ArrayList<UpdateMsg> list = new ArrayList<UpdateMsg>();
		
		list.add(copyUpdateMsg);
		list.add(copyUpdateMsg2);
		
		for(int i = 0; i < list.size(); i++)
		{
			UpdateMsg checkingUpdateMsg = list.get(i);
			
			TestUtilities.checkResult(JUnitTestConnect.getDataDictionary(checkingUpdateMsg) != null, "Checks the reference to DataDictionary after copying UpdateMsg");
			
			TestUtilities.checkResult(checkingUpdateMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
			
			TestUtilities.checkResult(checkingUpdateMsg.streamId() == 15, "UpdateMsg.streamId()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
			
			TestUtilities.checkResult(checkingUpdateMsg.seqNum() == 22, "UpdateMsg.seqNum()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasId(), "UpdateMsg.hasId()");
			
			TestUtilities.checkResult(checkingUpdateMsg.id() == 21, "UpdateMsg.id()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasFilter(), "UpdateMsg.hasFilter()");
			
			TestUtilities.checkResult(checkingUpdateMsg.filter() == 12 , "UpdateMsg.hasFilter()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
			
			TestUtilities.checkResult(checkingUpdateMsg.serviceId() == 5 , "UpdateMsg.serviceId()");
			
			TestUtilities.checkResult(checkingUpdateMsg.hasServiceName(), "UpdateMsg.hasServiceName()");
			
			TestUtilities.checkResult(checkingUpdateMsg.serviceName().equals(expectedServiceName) , "UpdateMsg.serviceName()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasNameType(), "UpdateMsg.hasNameType()");
			
			TestUtilities.checkResult(checkingUpdateMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasName(), "UpdateMsg.hasName()");
			
			TestUtilities.checkResult(checkingUpdateMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");
	
			TestUtilities.checkResult(checkingUpdateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
			
			TestUtilities.checkResult(checkingUpdateMsg.doNotCache(), "UpdateMsg.doNotCache()");
	
			TestUtilities.checkResult(checkingUpdateMsg.doNotConflate(), "UpdateMsg.doNotconflate()");
	
			TestUtilities.checkResult(!checkingUpdateMsg.doNotRipple(), "UpdateMsg.solicited()");
			
			TestUtilities.checkResult(checkingUpdateMsg.hasConflated(), "UpdateMsg.hasConflated()");
		
			TestUtilities.checkResult(checkingUpdateMsg.conflatedCount() == 10, "UpdateMsg.conflatedCount()");
	
			TestUtilities.checkResult(checkingUpdateMsg.conflatedTime() == 20, "UpdateMsg.conflatedTime()");
	
			TestUtilities.checkResult(checkingUpdateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
			
			TestUtilities.checkResult(checkingUpdateMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");
	
			TestUtilities.checkResult(checkingUpdateMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");
	
			TestUtilities.checkResult(checkingUpdateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
	
			System.out.println("End EMA UpdateMsg Decoding");
			System.out.println();
			checkingUpdateMsg.clear();
			
			int capacity = JUnitTestConnect.getCopiedBufferCapacity(checkingUpdateMsg);
			TestUtilities.checkResult(capacity == encodedBufferSize, "Checks capacity of the copied buffer after clearing UpdateMsg");
			TestUtilities.checkResult(JUnitTestConnect.getDataDictionary(checkingUpdateMsg) == null, "Checks the null reference to DataDictionary after clearing UpdateMsg");
		}
	}
	
	public void testUpdateMsg_copy_from_edited_copiedmessage()
	{
		TestUtilities.printTestHead("testUpdateMsg_copy_from_edited_copiedmessage", "eta encoding ema copying from edited copied message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();
		
		int encodedBufferSize = msgBuf.length();
		
		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA UpdateMsg Copying");
		
		UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg(encodedBufferSize);
		
		emaUpdateMsg.copy(copyUpdateMsg);
	
		TestUtilities.checkResult(copyUpdateMsg.hasPermissionData() == false, "Check that the permission data is not set");
		TestUtilities.checkResult(copyUpdateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		TestUtilities.checkResult(copyUpdateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
		
		UpdateMsg copyUpdateMsg2 = EmaFactory.createUpdateMsg(encodedBufferSize);
		
		ByteBuffer permData = ByteBuffer.wrap(new byte[]{16, 32, 48});
		
		/* Edits some properties of the message */
		copyUpdateMsg.permissionData(permData);
		copyUpdateMsg.attrib(emaUpdateMsg);
		copyUpdateMsg.payload(emaUpdateMsg);
		
		copyUpdateMsg.copy(copyUpdateMsg2);
		
		TestUtilities.checkResult(copyUpdateMsg2.hasPermissionData(), "Check to ensure that the permission is set");
		TestUtilities.checkResult(permData.equals(copyUpdateMsg2.permissionData()), "copyUpdateMsg2 permdata should match copy alter");
		TestUtilities.checkResult(copyUpdateMsg2.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG, "UpdateMsg.attrib().dataType()");
		
		UpdateMsg attribPayload = copyUpdateMsg2.attrib().updateMsg();
		
		TestUtilities.checkResult("emaUpdateMsg.toString().equals(attribPayload.toString())", emaUpdateMsg.toString().equals(attribPayload.toString()));
		
		TestUtilities.checkResult(copyUpdateMsg2.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG, "UpdateMsg.payload().dataType()");
		
		UpdateMsg payloadMsg = copyUpdateMsg2.payload().updateMsg();
		
		TestUtilities.checkResult("emaUpdateMsg.toString().equals(payloadMsg.toString())", emaUpdateMsg.toString().equals(payloadMsg.toString()));
	}
	
	public void testUpdateMsg_clone_from_edited_clonedmessage()
	{
		TestUtilities.printTestHead("testUpdateMsg_copy_from_edited_copiedmessage", "eta encoding ema cloning from edited cloned message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();
		
		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA UpdateMsg Copying");
		
		UpdateMsg cloneUpdateMsg = EmaFactory.createUpdateMsg(emaUpdateMsg);
	
		TestUtilities.checkResult(cloneUpdateMsg.hasPermissionData() == false, "Check that the permission data is not set");
		TestUtilities.checkResult(cloneUpdateMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		TestUtilities.checkResult(cloneUpdateMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
		
		ByteBuffer permData = ByteBuffer.wrap(new byte[]{16, 32, 48});
		
		/* Edits some properties of the message */
		cloneUpdateMsg.permissionData(permData);
		cloneUpdateMsg.attrib(emaUpdateMsg);
		cloneUpdateMsg.payload(emaUpdateMsg);
		
		UpdateMsg cloneUpdateMsg2 = EmaFactory.createUpdateMsg(cloneUpdateMsg);
		
		TestUtilities.checkResult(cloneUpdateMsg2.hasPermissionData(), "Check to ensure that the permission is set");
		TestUtilities.checkResult(permData.equals(cloneUpdateMsg2.permissionData()), "copyUpdateMsg2 permdata should match copy alter");
		TestUtilities.checkResult(cloneUpdateMsg2.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG, "UpdateMsg.attrib().dataType()");
		
		UpdateMsg attribPayload = cloneUpdateMsg2.attrib().updateMsg();
		
		TestUtilities.checkResult("emaUpdateMsg.toString().equals(attribPayload.toString())", emaUpdateMsg.toString().equals(attribPayload.toString()));
		
		TestUtilities.checkResult(cloneUpdateMsg2.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.UPDATE_MSG, "UpdateMsg.payload().dataType()");
		
		UpdateMsg payloadMsg = cloneUpdateMsg2.payload().updateMsg();
		
		TestUtilities.checkResult("emaUpdateMsg.toString().equals(payloadMsg.toString())", emaUpdateMsg.toString().equals(payloadMsg.toString()));
	}
	
	public void testUpdateMsg_invalid_destination_UpdateMsg()
	{
		TestUtilities.printTestHead("testUpdateMsg_invalid_destination_UpdateMsg", "eta encoding and ema copying with invalid destination refresh message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAUpdateMsg();
		
		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA UpdateMsg Copying with EMA encoding message");
		
		System.out.println("Create EMA UpdateMsg for encoding");
		UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg(); 
		
		Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> emaUpdateMsg.copy(copyUpdateMsg));
		
		OmmInvalidUsageException OIU = (OmmInvalidUsageException)exception;
		
		TestUtilities.checkResult(OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT == OIU.errorCode(), "Checks OmmInvalidUsageException.errorCode()");
		
		TestUtilities.checkResult("The passed in destination message is used for encoding only on com.refinitiv.ema.access.UpdateMsgImpl.copy(UpdateMsg destUpdateMsg)".equals(exception.getMessage()), "Checks OmmInvalidUsageException.getMessage()");
	}
	
	public void testUpdateMsg_EmaEncode_Clone()
	{
		TestUtilities.printTestHead("testUpdateMsg_EmaEncode_Clone", "Ema encoding and ema cloning message");
		
		UpdateMsg updateMsg = encodeEMAUpdateMsg();
		
		UpdateMsg clonedMsg = EmaFactory.createUpdateMsg(updateMsg);
		
		TestUtilities.checkResult(clonedMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(clonedMsg.streamId() == 15, "UpdateMsg.streamId()");

		TestUtilities.checkResult(clonedMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(clonedMsg.seqNum() == 22, "UpdateMsg.seqNum()");

		TestUtilities.checkResult(clonedMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(clonedMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(clonedMsg.id() == 21, "UpdateMsg.id()");

		TestUtilities.checkResult(clonedMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(clonedMsg.filter() == 12 , "UpdateMsg.hasFilter()");

		TestUtilities.checkResult(clonedMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(clonedMsg.serviceId() == 5 , "UpdateMsg.serviceId()");
		
		TestUtilities.checkResult(clonedMsg.hasServiceName(), "UpdateMsg.hasServiceName()");
		
		TestUtilities.checkResult(clonedMsg.serviceName().equals("DIRECT_FEED") , "UpdateMsg.serviceName()");

		TestUtilities.checkResult(clonedMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(clonedMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(clonedMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(clonedMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");

		TestUtilities.checkResult(clonedMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(clonedMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(clonedMsg.doNotConflate(), "UpdateMsg.doNotconflate()");

		TestUtilities.checkResult(!clonedMsg.doNotRipple(), "UpdateMsg.solicited()");
		
		TestUtilities.checkResult(clonedMsg.hasConflated(), "UpdateMsg.hasConflated()");
	
		TestUtilities.checkResult(clonedMsg.conflatedCount() == 10, "UpdateMsg.conflatedCount()");

		TestUtilities.checkResult(clonedMsg.conflatedTime() == 20, "UpdateMsg.conflatedTime()");

		TestUtilities.checkResult(clonedMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(clonedMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(clonedMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");

		TestUtilities.checkResult(clonedMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
	}
	
	public void testUpdateMsg_EmaEncode_Copy()
	{
		TestUtilities.printTestHead("testUpdateMsg_EmaEncode_Copy", "Ema encoding and ema copying message");
		
		UpdateMsg updateMsg = encodeEMAUpdateMsg();
		
		UpdateMsg copiedMsg = EmaFactory.createUpdateMsg(1024);
		
		updateMsg.copy(copiedMsg);
		
		TestUtilities.checkResult(copiedMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(copiedMsg.streamId() == 15, "UpdateMsg.streamId()");

		TestUtilities.checkResult(copiedMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(copiedMsg.seqNum() == 22, "UpdateMsg.seqNum()");

		TestUtilities.checkResult(copiedMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(copiedMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(copiedMsg.id() == 21, "UpdateMsg.id()");

		TestUtilities.checkResult(copiedMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(copiedMsg.filter() == 12 , "UpdateMsg.hasFilter()");

		TestUtilities.checkResult(copiedMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(copiedMsg.serviceId() == 5 , "UpdateMsg.serviceId()");
		
		TestUtilities.checkResult(copiedMsg.hasServiceName(), "UpdateMsg.hasServiceName()");
		
		TestUtilities.checkResult(copiedMsg.serviceName().equals("DIRECT_FEED") , "UpdateMsg.serviceName()");

		TestUtilities.checkResult(copiedMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(copiedMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(copiedMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(copiedMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");

		TestUtilities.checkResult(copiedMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(copiedMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(copiedMsg.doNotConflate(), "UpdateMsg.doNotconflate()");

		TestUtilities.checkResult(!copiedMsg.doNotRipple(), "UpdateMsg.solicited()");
		
		TestUtilities.checkResult(copiedMsg.hasConflated(), "UpdateMsg.hasConflated()");
	
		TestUtilities.checkResult(copiedMsg.conflatedCount() == 10, "UpdateMsg.conflatedCount()");

		TestUtilities.checkResult(copiedMsg.conflatedTime() == 20, "UpdateMsg.conflatedTime()");

		TestUtilities.checkResult(copiedMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(copiedMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(copiedMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");

		TestUtilities.checkResult(copiedMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");
	}
	
}
