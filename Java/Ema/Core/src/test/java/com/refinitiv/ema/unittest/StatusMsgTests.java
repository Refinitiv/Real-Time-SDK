/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.unittest;

import static org.junit.Assert.assertThrows;

import java.nio.ByteBuffer;
import java.util.ArrayList;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class StatusMsgTests extends TestCase
{
	private static com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
	
	public StatusMsgTests(String name)
	{
		super(name);
	}
	
	private com.refinitiv.eta.codec.Buffer encodeETAStatusMsg()
	{
		com.refinitiv.eta.codec.Buffer fieldListBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
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

		fieldListBuf.data(fieldListBuf.data(), 0, fieldListBuf.length());

		System.out.println("Begin ETA StatusMsg Set");
		com.refinitiv.eta.codec.StatusMsg statusMsg = (com.refinitiv.eta.codec.StatusMsg) com.refinitiv.eta.codec.CodecFactory
				.createMsg();
		statusMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.STATUS);

		statusMsg.domainType(com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE);

		statusMsg.streamId(15);

		statusMsg.applyHasMsgKey();

		statusMsg.msgKey().applyHasName();
		statusMsg.msgKey().name().data("ABCDEF");

		statusMsg.msgKey().applyHasNameType();
		statusMsg.msgKey().nameType(com.refinitiv.eta.rdm.InstrumentNameTypes.RIC);

		statusMsg.msgKey().applyHasServiceId();
		statusMsg.msgKey().serviceId(5);

		statusMsg.msgKey().applyHasFilter();
		statusMsg.msgKey().filter(12);

		statusMsg.msgKey().applyHasIdentifier();
		statusMsg.msgKey().identifier(21);

		statusMsg.msgKey().applyHasAttrib();
		statusMsg.msgKey().attribContainerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		statusMsg.msgKey().encodedAttrib(fieldListBuf);

		statusMsg.applyHasState();
		statusMsg.state().code(com.refinitiv.eta.codec.StateCodes.NONE);
		statusMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		statusMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		statusMsg.state().text().data("status complete");

		statusMsg.applyClearCache();

		statusMsg.applyHasPostUserInfo();
		statusMsg.postUserInfo().userAddr(15);
		statusMsg.postUserInfo().userId(30);

		statusMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		statusMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA StatusMsg Set");
		System.out.println();

		System.out.println("Begin ETA StatusMsg Buffer Encoding");

		com.refinitiv.eta.codec.Buffer msgBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));

		com.refinitiv.eta.codec.EncodeIterator encIter = com.refinitiv.eta.codec.CodecFactory
				.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ " encountered with setBufferAndRWFVersion. " + " Error Text: " + CodecReturnCodes.info(retVal));
			return null;
		}

		statusMsg.encode(encIter);

		System.out.println("End ETA StatusMsg Buffer Encoding");
		System.out.println();
		
		return msgBuf;
	}
	
	StatusMsg encodeEMAStatusMsg()
	{
		StatusMsg statusMsg = EmaFactory.createStatusMsg();
		
		statusMsg.streamId(15);
		
		statusMsg.name("ABCDEF");
		statusMsg.nameType(com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC);
		statusMsg.serviceId(5);
		statusMsg.filter(12);
		statusMsg.id(21);
		statusMsg.serviceName("DIRECT_FEED");
		
		statusMsg.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "status complete");
		
		statusMsg.clearCache(true);
		
		statusMsg.publisherId(30, 15);
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().intValue(1, 5));
		
		statusMsg.attrib(fieldList);
		statusMsg.payload(fieldList);
		
		return statusMsg;
	}
	
	public void testStatusMsg_Decode()
	{
		TestUtilities.printTestHead("testStatusMsg_Decode", "eta encoding ema decoding");

		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();

		System.out.println("Begin EMA StatusMsg Decoding");

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		TestUtilities.checkResult(emaStatusMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(emaStatusMsg.streamId() == 15, "StatusMsg.streamId()");

		TestUtilities.checkResult(emaStatusMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(emaStatusMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(emaStatusMsg.id() == 21, "StatusMsg.id()");

		TestUtilities.checkResult(emaStatusMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(emaStatusMsg.filter() == 12, "StatusMsg.hasFilter()");

		TestUtilities.checkResult(emaStatusMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(emaStatusMsg.serviceId() == 5, "StatusMsg.serviceId()");

		TestUtilities.checkResult(emaStatusMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(emaStatusMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
				"StatusMsg.nameType()");

		TestUtilities.checkResult(emaStatusMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(emaStatusMsg.name().compareTo("ABCDEF") == 0, "StatusMsg.name()");

		TestUtilities.checkResult(
				emaStatusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.attrib().dataType()");

		TestUtilities.checkResult(emaStatusMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(emaStatusMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(emaStatusMsg.publisherIdUserAddress() == 15, "StatusMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaStatusMsg.publisherIdUserId() == 30, "StatusMsg.publisherIdUserId()");

		TestUtilities.checkResult(
				emaStatusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.payload().dataType()");

		TestUtilities.checkResult(emaStatusMsg.hasState(), "StatusMsg.hasState()");

		TestUtilities.checkResult(emaStatusMsg.state().code() == OmmState.StatusCode.NONE, "StatusMsg.state().code()");

		TestUtilities.checkResult(emaStatusMsg.state().streamState() == OmmState.StreamState.OPEN,
				"StatusMsg.state().streamState()");

		TestUtilities.checkResult(emaStatusMsg.state().dataState() == OmmState.DataState.OK,
				"StatusMsg.state().dataState()");

		TestUtilities.checkResult(emaStatusMsg.state().statusText().compareTo("status complete") == 0,
				"StatusMsg.state().statusText()");

		System.out.println("End EMA StatusMsg Decoding");
		System.out.println();
	}

	public void testStatusMsg_toString()
	{
		TestUtilities.printTestHead("testStatusMsg_toString", "eta encoding ema toString");

		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();

		System.out.println("Begin EMA StatusMsg toString");

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		System.out.println(emaStatusMsg);

		System.out.println("End EMA StatusMsg toString");
		System.out.println();
	}

	public void testStatusMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testStatusMsg_EncodeDecode", "ema encoding ema decoding");

		String statusMsgString = "StatusMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    state=\"Open / Ok / None / 'Status Complete'\"\n" +
				"    clearCache\n" +
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
				"\n" +
				"    PayloadEnd\n" +
				"StatusMsgEnd\n";

		String statusMsgStringEmpty = "StatusMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"StatusMsgEnd\n";

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();

		TestUtilities.EmaEncodeFieldListAll(fl);

		com.refinitiv.ema.access.StatusMsg statusMsg = EmaFactory.createStatusMsg();
		com.refinitiv.ema.access.StatusMsg statusMsgEmpty = EmaFactory.createStatusMsg();

		System.out.println("Begin EMA StatusMsg test after constructor");

		TestUtilities.checkResult(statusMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(statusMsg.streamId() == 0, "StatusMsg.streamId()");

		TestUtilities.checkResult(!statusMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(!statusMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(!statusMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(!statusMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(!statusMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(!statusMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(
				statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA,
				"StatusMsg.attrib().dataType()");

		TestUtilities.checkResult(!statusMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(!statusMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(
				statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA,
				"StatusMsg.payload().dataType()");

		TestUtilities.checkResult(!statusMsg.hasState(), "StatusMsg.hasState()");

		System.out.println("Begin EMA StatusMsg test after constructor");
		System.out.println();

		System.out.println("End EMA StatusMsg Set");

		statusMsg.domainType(com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.streamId(15);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.name("ABCDEF");
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.nameType(com.refinitiv.eta.rdm.InstrumentNameTypes.RIC);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.serviceId(5);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.filter(12);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.id(21);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.attrib(fl);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Status Complete");
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.clearCache(true);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.publisherId(30, 15);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		statusMsg.payload(fl);
		TestUtilities.checkResult("StatusMsg.toString() == toString()", statusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		System.out.println("End EMA StatusMsg Set");
		System.out.println();

		System.out.println("Begin EMA StatusMsg Decoding");

		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("StatusMsg.toString(dictionary) == toString(dictionary)", statusMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("StatusMsg.toString(dictionary) == toString(dictionary)", statusMsgEmpty.toString(emaDataDictionary).equals(statusMsgStringEmpty));

		TestUtilities.checkResult("StatusMsg.toString(dictionary) == toString(dictionary)", statusMsg.toString(emaDataDictionary).equals(statusMsgString));

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, statusMsg, 14, 0, dictionary, null);

		com.refinitiv.ema.access.StatusMsg statusMsgClone = EmaFactory.createStatusMsg(statusMsg);
		statusMsgClone.clear();
		TestUtilities.checkResult("StatusMsg.toString(dictionary) == toString(dictionary)", statusMsgClone.toString(emaDataDictionary).equals(statusMsgStringEmpty));

		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("StatusMsg.toString() != toString()", !(emaStatusMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));

		TestUtilities.checkResult(emaStatusMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(emaStatusMsg.streamId() == 15, "StatusMsg.streamId()");

		TestUtilities.checkResult(emaStatusMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(emaStatusMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(emaStatusMsg.id() == 21, "StatusMsg.id()");

		TestUtilities.checkResult(emaStatusMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(emaStatusMsg.filter() == 12, "StatusMsg.hasFilter()");

		TestUtilities.checkResult(emaStatusMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(emaStatusMsg.serviceId() == 5, "StatusMsg.serviceId()");

		TestUtilities.checkResult(emaStatusMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(emaStatusMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
				"StatusMsg.nameType()");

		TestUtilities.checkResult(emaStatusMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(emaStatusMsg.name().compareTo("ABCDEF") == 0, "StatusMsg.name()");

		TestUtilities.checkResult(
				emaStatusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.attrib().dataType()");

		TestUtilities.checkResult(emaStatusMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(emaStatusMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(emaStatusMsg.publisherIdUserAddress() == 15, "StatusMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaStatusMsg.publisherIdUserId() == 30, "StatusMsg.publisherIdUserId()");

		TestUtilities.checkResult(
				emaStatusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.payload().dataType()");

		TestUtilities.checkResult(statusMsg.hasState(), "StatusMsg.hasState()");

		TestUtilities.checkResult(emaStatusMsg.state().code() == OmmState.StatusCode.NONE, "StatusMsg.state().code()");

		TestUtilities.checkResult(emaStatusMsg.state().streamState() == OmmState.StreamState.OPEN,
				"StatusMsg.state().streamState()");

		TestUtilities.checkResult(emaStatusMsg.state().dataState() == OmmState.DataState.OK,
				"StatusMsg.state().dataState()");

		TestUtilities.checkResult(emaStatusMsg.state().statusText().compareTo("Status Complete") == 0,
				"StatusMsg.state().statusText()");

		System.out.println("End EMA StatusMsg Decoding");
		System.out.println();

		statusMsg.clear();

		System.out.println("Begin EMA StatusMsg test after clear");

		TestUtilities.checkResult(statusMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(statusMsg.streamId() == 0, "StatusMsg.streamId()");

		TestUtilities.checkResult(!statusMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(!statusMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(!statusMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(!statusMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(!statusMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(!statusMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(
				statusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA,
				"StatusMsg.attrib().dataType()");

		TestUtilities.checkResult(!statusMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(!statusMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(
				statusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA,
				"StatusMsg.payload().dataType()");

		TestUtilities.checkResult(!statusMsg.hasState(), "StatusMsg.hasState()");

		System.out.println("End EMA StatusMsg test after clear");
		System.out.println();
	}
	
	public void testStatusMsg_EncodeETAStatusMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETAStatusMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode StatusMsg with ETA for FieldList as attrib, extended header and payload, Encode it to another StatusMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyStatusMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyStatusMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestStatusMsg_EncodeETAStatusMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testStatusMsg_EncodeETAStatusMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETAStatusMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode", "Encode StatusMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another StatusMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyStatusMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyStatusMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestStatusMsg_EncodeETAStatusMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode passed");
	}
	
	public void testStatusMsg_EncodeETAStatusMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETAStatusMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode", "Encode StatusMsg with ETA for FilterList as attrib, payload, Encode it to another StatusMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyStatusMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyStatusMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestStatusMsg_EncodeETAStatusMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode passed");
	}
	
	public void testStatusMsg_EncodeETAStatusMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETAStatusMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode", "Encode StatusMsg with ETA for Series as attrib, payload, Encode it to another StatusMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyStatusMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyStatusMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestStatusMsg_EncodeETAStatusMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode passed");
	}
	
	public void testStatusMsg_EncodeETAStatusMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETAStatusMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode", "Encode StatusMsg with ETA for Vector as attrib, payload, Encode it to another StatusMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyStatusMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyStatusMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestStatusMsg_EncodeETAStatusMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode passed");
	}
	
	public void testStatusMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode", "Encode StatusMsg with ETA for Map as attrib, payload, Encode it to another StatusMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyStatusMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyStatusMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestStatusMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode passed");
	}
	
	public void testStatusMsg_EncodeETAStatusMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testStatusMsg_EncodeETAStatusMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode", "Encode StatusMsg with ETA for RefreshMsg as attrib, payload. Encode it to another StatusMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeStatusMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeStatusMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode StatusMsg with EMA.
	     com.refinitiv.ema.access.StatusMsg statusMsg = JUnitTestConnect.createStatusMsg();
	     JUnitTestConnect.setRsslData(statusMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAStatusMsgAll(statusMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.StatusMsg copyStatusMsg = EmaFactory.createStatusMsg();
	     
	     // Encode to another StatusMsg
	     copyStatusMsg.extendedHeader(statusMsg.extendedHeader());
	     copyStatusMsg.permissionData(statusMsg.permissionData());
	     copyStatusMsg.attrib(statusMsg.attrib().data());
	     copyStatusMsg.payload(statusMsg.payload().data());
	     
	     com.refinitiv.ema.access.StatusMsg decCopyStatusMsg = JUnitTestConnect.createStatusMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyStatusMsg, copyStatusMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyStatusMsg.extendedHeader().equals(statusMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyStatusMsg.permissionData().equals(statusMsg.permissionData()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyStatusMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyStatusMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestStatusMsg_EncodeETAStatusMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherStatusMsg_EMADecode passed");
	}

	public void testStatusMsg_cloneForEncodeSide()
	{
		TestUtilities.printTestHead("testStatusMsg_cloneIsNotSupportedFromTheEncodeSide", "cloning is not supported on encode side");
		StatusMsg msg = EmaFactory.createStatusMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE)
				.name("Some status")
				.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted")
				;

		try {
			StatusMsg cloneMessage = EmaFactory.createStatusMsg(msg);
			TestUtilities.checkResult(true, "Clone not supported  - exception IS NOT expected: ");
			compareEmaStatusMsgFields(msg, cloneMessage, "Status message with no payload clone");
		} catch ( OmmException excp ) {
			TestUtilities.checkResult(false, "Clone not supported  - exception IS NOT expected: " +  excp.getMessage() );
		}
	}

	public void testStatusMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testStatusMsg_cloneMsgKeyWLScenario", "cloning for minimal ema status message");
		StatusMsg emaMsg = EmaFactory.createStatusMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, true);
		StatusMsg emaClonedMsg = EmaFactory.createStatusMsg(emaMsg);

		compareEmaStatusMsgFields(emaMsg, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, false);

		System.out.println("End EMA StatusMsg Clone msgKey");
		System.out.println();
	}

	public void testStatusMsg_clone()
	{
		TestUtilities.printTestHead("testStatusMsg_clone", "cloning for ema status message");

		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();

		System.out.println("Begin EMA StatusMsg clone");

		com.refinitiv.eta.codec.StatusMsg statusMsgDecode = (com.refinitiv.eta.codec.StatusMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
		statusMsgDecode.decode(decIter);

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();
				
		JUnitTestConnect.setRsslData(emaStatusMsg, statusMsgDecode, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		com.refinitiv.ema.access.StatusMsg emaStatusMsgClone = EmaFactory.createStatusMsg(emaStatusMsg);
		
		compareEmaStatusMsgFields(emaStatusMsg, emaStatusMsgClone, "Status cloned message");
		String emaStatusMsgString = emaStatusMsg.toString();
		String emaStatusMsgCloneString = emaStatusMsgClone.toString();
		
		System.out.println("Cloned EMA StatusMsg:");
		System.out.println(emaStatusMsgClone);
		
		TestUtilities.checkResult(emaStatusMsgString.equals(emaStatusMsgCloneString), "emaStatusMsgString.equals(emaStatusMsgCloneString)");

		com.refinitiv.ema.access.StatusMsg emaStatusMsgClone2 = EmaFactory.createStatusMsg(emaStatusMsgClone);
		compareEmaStatusMsgFields(emaStatusMsg, emaStatusMsgClone2, "Status double-cloned message");
		String emaStatusMsgClone2String = emaStatusMsgClone2.toString();
		TestUtilities.checkResult(emaStatusMsgString.equals(emaStatusMsgClone2String), "double-cloned emaStatusMsgString.equals(emaStatusMsgClone2String)");

		System.out.println("End EMA StatusMsg clone");
		System.out.println();
	}

	public void testStatusMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testStatusMsg_clone", "cloning for ema status message");

		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();
		System.out.println("Begin EMA StatusMsg clone");

		com.refinitiv.eta.codec.StatusMsg statusMsgDecode = (com.refinitiv.eta.codec.StatusMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, Codec.majorVersion(), Codec.minorVersion());
		statusMsgDecode.decode(decIter);

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, statusMsgDecode, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		com.refinitiv.ema.access.StatusMsg emaStatusMsgClone = EmaFactory.createStatusMsg(emaStatusMsg);

		compareEmaStatusMsgFields(emaStatusMsg, emaStatusMsgClone, "Status cloned message");

		String emaStatusMsgString = emaStatusMsg.toString();
		String emaStatusMsgCloneString = emaStatusMsgClone.toString();
		
		System.out.println("Cloned EMA StatusMsg:");
		System.out.println(emaStatusMsgClone);
		
		TestUtilities.checkResult(emaStatusMsgString.equals(emaStatusMsgCloneString), "emaStatusMsgString.equals(emaStatusMsgCloneString)");
		
		System.out.println("End EMA StatusMsg clone");
		System.out.println();
	}

	private void compareEmaStatusMsgFields(StatusMsg expected, StatusMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + " base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasServiceName() == actual.hasServiceName(), checkPrefix + "hasServiceName");
		if(expected.hasServiceName())
			TestUtilities.checkResult(expected.serviceName().equals(actual.serviceName()), checkPrefix + "serviceId" + "exp=" +actual.serviceName() + " act="+expected.serviceName());

		TestUtilities.checkResult(expected.hasState() == actual.hasState(), checkPrefix + "hasState");
		if(expected.hasState()) {
			TestUtilities.checkResult(expected.state().code() == actual.state().code(), checkPrefix + "state.code");
			TestUtilities.checkResult(expected.state().statusCode() == actual.state().statusCode(), checkPrefix + "state.statusCode");
			TestUtilities.checkResult(expected.state().streamState() == actual.state().streamState(), checkPrefix + "state.streamState()");
			TestUtilities.checkResult(expected.state().statusText().equals(actual.state().statusText()), checkPrefix + "state.statusText");
		}

		TestUtilities.checkResult(expected.hasPublisherId() == actual.hasPublisherId(), checkPrefix + "hasPublisherId");
		if(expected.hasPublisherId()) {
			TestUtilities.checkResult(expected.publisherIdUserId() == actual.publisherIdUserId(), checkPrefix + "publisherIdUserId");
			TestUtilities.checkResult(expected.publisherIdUserAddress() == actual.publisherIdUserAddress(), checkPrefix + "publisherIdUserAddress");
		}

		TestUtilities.checkResult(expected.clearCache() == actual.clearCache(), checkPrefix + "clearCache");
		TestUtilities.checkResult(expected.privateStream() == actual.privateStream(), checkPrefix + "privateStream");

		TestUtilities.checkResult(expected.hasItemGroup() == actual.hasItemGroup(), checkPrefix + "hasItemGroup");
		if(expected.hasItemGroup())
			TestUtilities.checkResult(expected.itemGroup().equals(actual.itemGroup()), checkPrefix + "itemGroup");

		TestUtilities.checkResult(expected.hasPermissionData() == actual.hasPermissionData(), checkPrefix + "hasPermissionData");
		if(expected.hasPermissionData())
			TestUtilities.checkResult(expected.permissionData().equals(actual.permissionData()), checkPrefix + "permissionData");
	}
	
	public void testStatusMsg_InitCopyingMessageWithUnknownSize()
	{
		StatusMsg copyStatusMsg = EmaFactory.createStatusMsg(-1);
		int capacity = JUnitTestConnect.getCopiedBufferCapacity(copyStatusMsg);
		
		TestUtilities.checkResult(capacity == 0, "Checks capacity of the copied buffer");
		
		copyStatusMsg.clear();
		
		capacity = JUnitTestConnect.getCopiedBufferCapacity(copyStatusMsg);
		
		TestUtilities.checkResult(capacity == 0, "Checks capacity of the copied buffer after clearing");
	}
	
	public void testStatusMsg_InitCopyingMessage()
	{
		StatusMsg copyStatusMsg = EmaFactory.createStatusMsg(1024);
		int capacity = JUnitTestConnect.getCopiedBufferCapacity(copyStatusMsg);
		
		TestUtilities.checkResult(capacity == 1024, "Checks capacity of copied buffer");
		copyStatusMsg.clear();
		
		capacity = JUnitTestConnect.getCopiedBufferCapacity(copyStatusMsg);
		TestUtilities.checkResult(capacity == 1024, "Checks capacity of the copied buffer after clearing StatusMsg");
	}
	
	public void testStatusMsg_copy_from_EncodingEMAStatusMsg()
	{
		TestUtilities.printTestHead("testStatusMsg_copy_from_emptyStatusMsg", "ema copying from empty ema message");
		
		StatusMsg emaStatusMsg = EmaFactory.createStatusMsg();
		
		emaStatusMsg.name("LSEG.O");
		emaStatusMsg.streamId(5);
		emaStatusMsg.serviceId(1);
		emaStatusMsg.state(OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Service is not available");
		
		StatusMsg copyStatusMsg = EmaFactory.createStatusMsg(-1);
		
		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();
		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());
		
		emaStatusMsg.copy(copyStatusMsg);
		
		TestUtilities.checkResult("emaStatusMsg.toString().equals(copyStatusMsg.toString())", emaStatusMsg.toString(emaDataDictionary).equals(copyStatusMsg.toString()));
		TestUtilities.checkResult("emaStatusMsg.toString(emaDataDictionary).equals(copyStatusMsg.toString(emaDataDictionary))", emaStatusMsg.toString(emaDataDictionary).equals(copyStatusMsg.toString(emaDataDictionary)));
	}
	
	public void testStatusMsg_copy_from_decodingStatusMsg_and_copied_message()
	{
		TestUtilities.printTestHead("testStatusMsg_copy_from_decodingStatusMsg_and_copied_message", "eta encoding ema copying");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();
		
		int encodedBufferSize = msgBuf.length();
		
		System.out.println("Begin EMA StatusMsg Decoding");

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		String expectedServiceName = "DIRECT_FEED";
		
		/* Set the service name as well */
		emaStatusMsg.serviceName(expectedServiceName);
		
		System.out.println("Begin EMA StatusMsg Copying");
		
		StatusMsg copyStatusMsg = EmaFactory.createStatusMsg(encodedBufferSize);
		
		emaStatusMsg.copy(copyStatusMsg);
		
		StatusMsg copyStatusMsg2 = EmaFactory.createStatusMsg(encodedBufferSize);
		
		copyStatusMsg.copy(copyStatusMsg2);
		
		ArrayList<StatusMsg> list = new ArrayList<StatusMsg>();
		
		list.add(copyStatusMsg);
		list.add(copyStatusMsg2);
		
		for(int i = 0; i < list.size(); i++)
		{
			StatusMsg checkingStatusMsg = list.get(i);
			
			TestUtilities.checkResult(JUnitTestConnect.getDataDictionary(checkingStatusMsg) != null, "Checks the reference to DataDictionary after copying StatusMsg");
			
			TestUtilities.checkResult(checkingStatusMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
					"StatusMsg.domainType()");

			TestUtilities.checkResult(checkingStatusMsg.streamId() == 15, "StatusMsg.streamId()");

			TestUtilities.checkResult(checkingStatusMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

			TestUtilities.checkResult(checkingStatusMsg.hasId(), "StatusMsg.hasId()");

			TestUtilities.checkResult(checkingStatusMsg.id() == 21, "StatusMsg.id()");

			TestUtilities.checkResult(checkingStatusMsg.hasFilter(), "StatusMsg.hasFilter()");

			TestUtilities.checkResult(checkingStatusMsg.filter() == 12, "StatusMsg.hasFilter()");

			TestUtilities.checkResult(checkingStatusMsg.hasServiceId(), "StatusMsg.hasServiceId()");

			TestUtilities.checkResult(checkingStatusMsg.serviceId() == 5, "StatusMsg.serviceId()");
			
			TestUtilities.checkResult(checkingStatusMsg.hasServiceName(), "StatusMsg.hasServiceName()");
			
			TestUtilities.checkResult(checkingStatusMsg.serviceName().equals(expectedServiceName) , "StatusMsg.serviceName()");


			TestUtilities.checkResult(checkingStatusMsg.hasNameType(), "StatusMsg.hasNameType()");

			TestUtilities.checkResult(checkingStatusMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
					"StatusMsg.nameType()");

			TestUtilities.checkResult(checkingStatusMsg.hasName(), "StatusMsg.hasName()");

			TestUtilities.checkResult(checkingStatusMsg.name().compareTo("ABCDEF") == 0, "StatusMsg.name()");

			TestUtilities.checkResult(
					checkingStatusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
					"StatusMsg.attrib().dataType()");

			TestUtilities.checkResult(checkingStatusMsg.clearCache(), "StatusMsg.clearCache()");

			TestUtilities.checkResult(checkingStatusMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

			TestUtilities.checkResult(checkingStatusMsg.publisherIdUserAddress() == 15, "StatusMsg.publisherIdUserAddress()");

			TestUtilities.checkResult(checkingStatusMsg.publisherIdUserId() == 30, "StatusMsg.publisherIdUserId()");

			TestUtilities.checkResult(
					checkingStatusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
					"StatusMsg.payload().dataType()");

			TestUtilities.checkResult(checkingStatusMsg.hasState(), "StatusMsg.hasState()");

			TestUtilities.checkResult(checkingStatusMsg.state().code() == OmmState.StatusCode.NONE, "StatusMsg.state().code()");

			TestUtilities.checkResult(checkingStatusMsg.state().streamState() == OmmState.StreamState.OPEN,
					"StatusMsg.state().streamState()");

			TestUtilities.checkResult(checkingStatusMsg.state().dataState() == OmmState.DataState.OK,
					"StatusMsg.state().dataState()");

			TestUtilities.checkResult(checkingStatusMsg.state().statusText().compareTo("status complete") == 0,
					"StatusMsg.state().statusText()");
			
			System.out.println("Clears the copied message");
			checkingStatusMsg.clear();
			
			int capacity = JUnitTestConnect.getCopiedBufferCapacity(checkingStatusMsg);
			TestUtilities.checkResult(capacity == encodedBufferSize, "Checks capacity of the copied buffer after clearing StatusMsg");
			TestUtilities.checkResult(JUnitTestConnect.getDataDictionary(checkingStatusMsg) == null, "Checks the null reference to DataDictionary after clearing StatusMsg");
		}
	}
	
	public void testStatusMsg_copy_from_edited_copiedmessage()
	{
		TestUtilities.printTestHead("testStatusMsg_copy_from_edited_copiedmessage", "eta encoding ema copying from edited copied message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();
		
		int encodedBufferSize = msgBuf.length();
		
		System.out.println("Begin EMA StatusMsg Decoding");

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA StatusMsg Copying");
		
		StatusMsg copyStatusMsg = EmaFactory.createStatusMsg(encodedBufferSize);
		
		emaStatusMsg.copy(copyStatusMsg);
	
		TestUtilities.checkResult(copyStatusMsg.hasPermissionData() == false, "Check that the permission data is not set");
		TestUtilities.checkResult(copyStatusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.attrib().dataType()");
		TestUtilities.checkResult(copyStatusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.payload().dataType()");
		
		StatusMsg copyStatusMsg2 = EmaFactory.createStatusMsg(encodedBufferSize);
		
		ByteBuffer permData = ByteBuffer.wrap(new byte[]{16, 32, 48});
		
		/* Edits some properties of the message */
		copyStatusMsg.permissionData(permData);
		copyStatusMsg.attrib(emaStatusMsg);
		copyStatusMsg.payload(emaStatusMsg);
		
		copyStatusMsg.copy(copyStatusMsg2);
		
		TestUtilities.checkResult(copyStatusMsg2.hasPermissionData(), "Check to ensure that the permission is set");
		TestUtilities.checkResult(permData.equals(copyStatusMsg2.permissionData()), "copyStatusMsg2 permdata should match copy alter");
		TestUtilities.checkResult(copyStatusMsg2.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG, "StatusMsg.attrib().dataType()");
		
		StatusMsg attribPayload = copyStatusMsg2.attrib().statusMsg();
		
		TestUtilities.checkResult("emaStatusMsg.toString().equals(attribPayload.toString())", emaStatusMsg.toString().equals(attribPayload.toString()));
		
		TestUtilities.checkResult(copyStatusMsg2.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG, "StatusMsg.payload().dataType()");
		
		StatusMsg payloadMsg = copyStatusMsg2.payload().statusMsg();
		
		TestUtilities.checkResult("emaStatusMsg.toString().equals(payloadMsg.toString())", emaStatusMsg.toString().equals(payloadMsg.toString()));
	}
	
	public void testStatusMsg_clone_from_edited_clonedmessage()
	{
		TestUtilities.printTestHead("testStatusMsg_copy_from_edited_copiedmessage", "eta encoding ema cloning from edited cloned message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();
		
		System.out.println("Begin EMA StatusMsg Decoding");

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA StatusMsg Copying");
		
		StatusMsg cloneStatusMsg = EmaFactory.createStatusMsg(emaStatusMsg);
	
		TestUtilities.checkResult(cloneStatusMsg.hasPermissionData() == false, "Check that the permission data is not set");
		TestUtilities.checkResult(cloneStatusMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.attrib().dataType()");
		TestUtilities.checkResult(cloneStatusMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "StatusMsg.payload().dataType()");
		
		ByteBuffer permData = ByteBuffer.wrap(new byte[]{16, 32, 48});
		
		/* Edits some properties of the message */
		cloneStatusMsg.permissionData(permData);
		cloneStatusMsg.attrib(emaStatusMsg);
		cloneStatusMsg.payload(emaStatusMsg);
		
		StatusMsg cloneStatusMsg2 = EmaFactory.createStatusMsg(cloneStatusMsg);
		
		TestUtilities.checkResult(cloneStatusMsg2.hasPermissionData(), "Check to ensure that the permission is set");
		TestUtilities.checkResult(permData.equals(cloneStatusMsg2.permissionData()), "copyStatusMsg2 permdata should match copy alter");
		TestUtilities.checkResult(cloneStatusMsg2.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG, "StatusMsg.attrib().dataType()");
		
		StatusMsg attribPayload = cloneStatusMsg2.attrib().statusMsg();
		
		TestUtilities.checkResult("emaStatusMsg.toString().equals(attribPayload.toString())", emaStatusMsg.toString().equals(attribPayload.toString()));
		
		TestUtilities.checkResult(cloneStatusMsg2.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.STATUS_MSG, "StatusMsg.payload().dataType()");
		
		StatusMsg payloadMsg = cloneStatusMsg2.payload().statusMsg();
		
		TestUtilities.checkResult("emaStatusMsg.toString().equals(payloadMsg.toString())", emaStatusMsg.toString().equals(payloadMsg.toString()));
	}
	
	public void testStatusMsg_invalid_destination_StatusMsg()
	{
		TestUtilities.printTestHead("testStatusMsg_invalid_destination_StatusMsg", "eta encoding and ema copying with invalid destination refresh message");
		
		com.refinitiv.eta.codec.Buffer msgBuf = encodeETAStatusMsg();
		
		System.out.println("Begin EMA StatusMsg Decoding");

		com.refinitiv.ema.access.StatusMsg emaStatusMsg = JUnitTestConnect.createStatusMsg();

		JUnitTestConnect.setRsslData(emaStatusMsg, msgBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
		
		System.out.println("Begin EMA StatusMsg Copying with EMA encoding message");
		
		System.out.println("Create EMA StatusMsg for encoding");
		StatusMsg copyStatusMsg = EmaFactory.createStatusMsg(); 
		
		Exception exception = assertThrows(OmmInvalidUsageException.class,  () -> emaStatusMsg.copy(copyStatusMsg));
		
		OmmInvalidUsageException OIU = (OmmInvalidUsageException)exception;
		
		TestUtilities.checkResult(OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT == OIU.errorCode(), "Checks OmmInvalidUsageException.errorCode()");
		
		TestUtilities.checkResult("The passed in destination message is used for encoding only on com.refinitiv.ema.access.StatusMsgImpl.copy(StatusMsg destStatusMsg)".equals(exception.getMessage()), "Checks OmmInvalidUsageException.getMessage()");
	}
	
	public void testStatusMsg_EmaEncode_Clone()
	{
		TestUtilities.printTestHead("testStatusMsg_EmaEncode_Clone", "Ema encoding and ema cloning message");
		
		StatusMsg statusMsg = encodeEMAStatusMsg();
		
		StatusMsg clonedMsg = EmaFactory.createStatusMsg(statusMsg);
		
		TestUtilities.checkResult(clonedMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(clonedMsg.streamId() == 15, "StatusMsg.streamId()");

		TestUtilities.checkResult(clonedMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(clonedMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(clonedMsg.id() == 21, "StatusMsg.id()");

		TestUtilities.checkResult(clonedMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(clonedMsg.filter() == 12, "StatusMsg.hasFilter()");

		TestUtilities.checkResult(clonedMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(clonedMsg.serviceId() == 5, "StatusMsg.serviceId()");
		
		TestUtilities.checkResult(clonedMsg.hasServiceName(), "StatusMsg.hasServiceName()");
		
		TestUtilities.checkResult(clonedMsg.serviceName().equals("DIRECT_FEED") , "StatusMsg.serviceName()");

		TestUtilities.checkResult(clonedMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(clonedMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
				"StatusMsg.nameType()");

		TestUtilities.checkResult(clonedMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(clonedMsg.name().compareTo("ABCDEF") == 0, "StatusMsg.name()");

		TestUtilities.checkResult(
				clonedMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.attrib().dataType()");

		TestUtilities.checkResult(clonedMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(clonedMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(clonedMsg.publisherIdUserAddress() == 15, "StatusMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(clonedMsg.publisherIdUserId() == 30, "StatusMsg.publisherIdUserId()");

		TestUtilities.checkResult(
				clonedMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.payload().dataType()");

		TestUtilities.checkResult(clonedMsg.hasState(), "StatusMsg.hasState()");

		TestUtilities.checkResult(clonedMsg.state().code() == OmmState.StatusCode.NONE, "StatusMsg.state().code()");

		TestUtilities.checkResult(clonedMsg.state().streamState() == OmmState.StreamState.OPEN,
				"StatusMsg.state().streamState()");

		TestUtilities.checkResult(clonedMsg.state().dataState() == OmmState.DataState.OK,
				"StatusMsg.state().dataState()");

		TestUtilities.checkResult(clonedMsg.state().statusText().compareTo("status complete") == 0,
				"StatusMsg.state().statusText()");
	}
	
	public void testStatusMsg_EmaEncode_Copy()
	{
		TestUtilities.printTestHead("testStatusMsg_EmaEncode_Copy", "Ema encoding and ema copying message");
		
		StatusMsg statusMsg = encodeEMAStatusMsg();
		
		StatusMsg copiedMsg = EmaFactory.createStatusMsg(1024);
		
		statusMsg.copy(copiedMsg);
		
		TestUtilities.checkResult(copiedMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE,
				"StatusMsg.domainType()");

		TestUtilities.checkResult(copiedMsg.streamId() == 15, "StatusMsg.streamId()");

		TestUtilities.checkResult(copiedMsg.hasMsgKey(), "StatusMsg.hasMsgKey()");

		TestUtilities.checkResult(copiedMsg.hasId(), "StatusMsg.hasId()");

		TestUtilities.checkResult(copiedMsg.id() == 21, "StatusMsg.id()");

		TestUtilities.checkResult(copiedMsg.hasFilter(), "StatusMsg.hasFilter()");

		TestUtilities.checkResult(copiedMsg.filter() == 12, "StatusMsg.hasFilter()");

		TestUtilities.checkResult(copiedMsg.hasServiceId(), "StatusMsg.hasServiceId()");

		TestUtilities.checkResult(copiedMsg.serviceId() == 5, "StatusMsg.serviceId()");
		
		TestUtilities.checkResult(copiedMsg.hasServiceName(), "StatusMsg.hasServiceName()");
		
		TestUtilities.checkResult(copiedMsg.serviceName().equals("DIRECT_FEED") , "StatusMsg.serviceName()");

		TestUtilities.checkResult(copiedMsg.hasNameType(), "StatusMsg.hasNameType()");

		TestUtilities.checkResult(copiedMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC,
				"StatusMsg.nameType()");

		TestUtilities.checkResult(copiedMsg.hasName(), "StatusMsg.hasName()");

		TestUtilities.checkResult(copiedMsg.name().compareTo("ABCDEF") == 0, "StatusMsg.name()");

		TestUtilities.checkResult(
				copiedMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.attrib().dataType()");

		TestUtilities.checkResult(copiedMsg.clearCache(), "StatusMsg.clearCache()");

		TestUtilities.checkResult(copiedMsg.hasPublisherId(), "StatusMsg.hasPublisherId()");

		TestUtilities.checkResult(copiedMsg.publisherIdUserAddress() == 15, "StatusMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(copiedMsg.publisherIdUserId() == 30, "StatusMsg.publisherIdUserId()");

		TestUtilities.checkResult(
				copiedMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST,
				"StatusMsg.payload().dataType()");

		TestUtilities.checkResult(copiedMsg.hasState(), "StatusMsg.hasState()");

		TestUtilities.checkResult(copiedMsg.state().code() == OmmState.StatusCode.NONE, "StatusMsg.state().code()");

		TestUtilities.checkResult(copiedMsg.state().streamState() == OmmState.StreamState.OPEN,
				"StatusMsg.state().streamState()");

		TestUtilities.checkResult(copiedMsg.state().dataState() == OmmState.DataState.OK,
				"StatusMsg.state().dataState()");

		TestUtilities.checkResult(copiedMsg.state().statusText().compareTo("status complete") == 0,
				"StatusMsg.state().statusText()");
	}
	
}
