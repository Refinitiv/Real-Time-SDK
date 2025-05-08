///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.     
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

import junit.framework.TestCase;

public class RefreshMsgTests extends TestCase
{
	public RefreshMsgTests(String name)
	{
		super(name);
	}
	
	public void testRefreshMsg_Decode()
	{
		TestUtilities.printTestHead("testRefreshMsg_Decode", "eta encoding ema decoding");
		
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
		
	    System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End ETA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
		TestUtilities.checkResult(emaRefreshMsg.streamId() == 15, "RefreshMsg.streamId()");

		TestUtilities.checkResult(emaRefreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");
		
		TestUtilities.checkResult(emaRefreshMsg.partNum() == 10, "RefreshMsg.partNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaRefreshMsg.seqNum() == 22, "RefreshMsg.seqNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasQos(), "RefreshMsg.hasQos()");
		
		TestUtilities.checkResult(emaRefreshMsg.qos().timeliness() == OmmQos.Timeliness.REALTIME, "RefreshMsg.qos().timeliness()");
		
		TestUtilities.checkResult(emaRefreshMsg.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "RefreshMsg.qos().rate()");
		
		TestUtilities.checkResult(emaRefreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(emaRefreshMsg.hasId(), "RefreshMsg.hasId()");
		
		TestUtilities.checkResult(emaRefreshMsg.id() == 21, "RefreshMsg.id()");

		TestUtilities.checkResult(emaRefreshMsg.hasFilter(), "RefreshMsg.hasFilter()");
		
		TestUtilities.checkResult(emaRefreshMsg.filter() == 12 , "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(emaRefreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaRefreshMsg.serviceId() == 5 , "RefreshMsg.serviceId()");

		TestUtilities.checkResult(emaRefreshMsg.hasNameType(), "RefreshMsg.hasNameType()");
		
		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().compareTo("refresh complete") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();
	}

	public void testRefreshMsg_toString()
	{
		TestUtilities.printTestHead("testRefreshMsg_toString", "eta encoding ema toString");
		
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
		
	    System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End ETA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg toString");

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaRefreshMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("RefreshMsg.toString() != toString()", !(emaRefreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));
		
		System.out.println("End EMA RefreshMsg toString");
		System.out.println();
	}
	
	public void testRefreshMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testRefreshMsg_EncodeDecode", "ema encoding ema decoding");

		String refreshMsgString = "RefreshMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    solicited\n" +
				"    clearCache\n" +
				"    doNotCache\n" +
				"    qos=\"RealTime/TickByTick\"\n" +
				"    state=\"Open / Ok / None / 'Refresh Complete'\"\n" +
				"    itemGroup=\"00 00\"\n" +
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
				"RefreshMsgEnd\n";

		String refreshMsgEmptyString = "RefreshMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    state=\"Open / Ok / None / ''\"\n" +
				"    itemGroup=\"00 00\"\n" +
				"RefreshMsgEnd\n";

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
	    com.refinitiv.ema.access.RefreshMsg refreshMsgEmpty = EmaFactory.createRefreshMsg();

		System.out.println("Begin EMA RefreshMsg test after constructor");

		TestUtilities.checkResult(refreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
		TestUtilities.checkResult(refreshMsg.streamId() == 0, "RefreshMsg.streamId()");

		TestUtilities.checkResult(!refreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");
		
		TestUtilities.checkResult(!refreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!refreshMsg.hasQos(), "RefreshMsg.hasQos()");
		
		TestUtilities.checkResult(!refreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(!refreshMsg.hasId(), "RefreshMsg.hasId()");
		
		TestUtilities.checkResult(!refreshMsg.hasFilter(), "RefreshMsg.hasFilter()");
		
		TestUtilities.checkResult(!refreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");
		
		TestUtilities.checkResult(!refreshMsg.hasNameType(), "RefreshMsg.hasNameType()");
		
		TestUtilities.checkResult(!refreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(!refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(!refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(!refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA RefreshMsg Set");
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
    	refreshMsg.domainType( com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.streamId( 15 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.partNum( 10 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.seqNum( 22 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		refreshMsg.qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		refreshMsg.name("ABCDEF");
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		refreshMsg.serviceId(5);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.filter( 12 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		refreshMsg.id(21);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.attrib(fl);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		refreshMsg.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Complete");
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
	    refreshMsg.clearCache(true);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		refreshMsg.doNotCache(true);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		refreshMsg.solicited(true);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.publisherId(30,  15);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		refreshMsg.payload(fl);
		TestUtilities.checkResult("RefreshMsg.toString() == toString()", refreshMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		System.out.println("End EMA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("RefreshMsg.toString(dictionary) == toString(dictionary)", refreshMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("RefreshMsg.toString(dictionary) == toString(dictionary)", refreshMsgEmpty.toString(emaDataDictionary).equals(refreshMsgEmptyString));

		TestUtilities.checkResult("RefreshMsg.toString(dictionary) == toString(dictionary)", refreshMsg.toString(emaDataDictionary).equals(refreshMsgString));

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();

		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsg, 14, 0, dictionary, null);

		com.refinitiv.ema.access.RefreshMsg refreshMsgClone = EmaFactory.createRefreshMsg(refreshMsg);
		refreshMsgClone.clear();
		TestUtilities.checkResult("RefreshMsg.toString(dictionary) == toString(dictionary)", refreshMsgClone.toString(emaDataDictionary).equals(refreshMsgEmptyString));

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
		TestUtilities.checkResult(emaRefreshMsg.streamId() == 15, "RefreshMsg.streamId()");

		TestUtilities.checkResult(emaRefreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");
		
		TestUtilities.checkResult(emaRefreshMsg.partNum() == 10, "RefreshMsg.partNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaRefreshMsg.seqNum() == 22, "RefreshMsg.seqNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasQos(), "RefreshMsg.hasQos()");
		
		TestUtilities.checkResult(emaRefreshMsg.qos().timeliness() == OmmQos.Timeliness.REALTIME, "RefreshMsg.qos().timeliness()");
		
		TestUtilities.checkResult(emaRefreshMsg.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "RefreshMsg.qos().rate()");
		
		TestUtilities.checkResult(emaRefreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(emaRefreshMsg.hasId(), "RefreshMsg.hasId()");
		
		TestUtilities.checkResult(emaRefreshMsg.id() == 21, "RefreshMsg.id()");

		TestUtilities.checkResult(emaRefreshMsg.hasFilter(), "RefreshMsg.hasFilter()");
		
		TestUtilities.checkResult(emaRefreshMsg.filter() == 12 , "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(emaRefreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaRefreshMsg.serviceId() == 5 , "RefreshMsg.serviceId()");

		TestUtilities.checkResult(emaRefreshMsg.hasNameType(), "RefreshMsg.hasNameType()");
		
		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().compareTo("Refresh Complete") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();

		refreshMsg.clear();
		
		System.out.println("Begin EMA RefreshMsg test after clear");

		TestUtilities.checkResult(refreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
		TestUtilities.checkResult(refreshMsg.streamId() == 0, "RefreshMsg.streamId()");

		TestUtilities.checkResult(!refreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");
		
		TestUtilities.checkResult(!refreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!refreshMsg.hasQos(), "RefreshMsg.hasQos()");
		
		TestUtilities.checkResult(!refreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(!refreshMsg.hasId(), "RefreshMsg.hasId()");
		
		TestUtilities.checkResult(!refreshMsg.hasFilter(), "RefreshMsg.hasFilter()");
		
		TestUtilities.checkResult(!refreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");
		
		TestUtilities.checkResult(!refreshMsg.hasNameType(), "RefreshMsg.hasNameType()");
		
		TestUtilities.checkResult(!refreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(!refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(!refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(!refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg test after clear");
		System.out.println();
	}
	
	public void testRefreshMsg_EncodeETARefreshWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for FieldList as attrib, extended header and payload, Encode it to another RefreshMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyRefreshMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyRefreshMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeETARefreshWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another RefreshMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyRefreshMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyRefreshMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeETARefreshWithFilterListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithFilterListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for FilterList as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyRefreshMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyRefreshMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithFilterListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeETARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for Series as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyRefreshMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyRefreshMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeETARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for Vector as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyRefreshMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyRefreshMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeETARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for Map as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyRefreshMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyRefreshMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeETARefreshWithRefreshTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeETARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with ETA for Map as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRefreshMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.refinitiv.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARefreshMsgAll(refreshMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.refinitiv.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyRefreshMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyRefreshMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeETARefreshWithRefreshTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}

	public void testRefreshMsg_cloneIsNotSupportedFromTheEncodeSide()
	{
		TestUtilities.printTestHead("testRefreshMsg_cloneIsNotSupportedFromTheEncodeSide", "cloning is not supported on encode side");
		RefreshMsg msg = EmaFactory.createRefreshMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE);

		try {
			RefreshMsg cloneMessage = EmaFactory.createRefreshMsg(msg);
			TestUtilities.checkResult(false, "Clone not supported  - exception expected: ");
		} catch ( OmmException excp ) {
			TestUtilities.checkResult(true, "Clone not supported  - exception expected: " +  excp.getMessage() );
			TestUtilities.checkResult(excp.getMessage().startsWith("Failed to clone empty encoded buffer"), "Clone not supported - exception text validated");
		}
	}

	public void testRefreshMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testRefreshMsg_cloneMsgKeyWLScenario", "cloning for minimal ema refresh message");
		RefreshMsg emaMsg = EmaFactory.createRefreshMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample())
			// refresh message always has group id
			.itemGroup(ByteBuffer.wrap(new byte[] {1, 2, 3}));


		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		RefreshMsg emaMsg2 = JUnitTestConnect.createRefreshMsg();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		JUnitTestConnect.setRsslData(emaMsg2, JUnitTestConnect.getRsslData(emaMsg), majorVersion, minorVersion, dictionary, null);
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg2, true);
		RefreshMsg emaClonedMsg = EmaFactory.createRefreshMsg(emaMsg2);

		compareEmaRefreshMsgFields(emaMsg2, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg2, false);

		System.out.println("End EMA RefreshMsg Clone msgKey");
		System.out.println();
	}

	public void testRefreshMsg_clone()
	{
		TestUtilities.printTestHead("testRefreshMsg_clone", "cloning for ema refresh message");
		
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
		
	    System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );
		
		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		setMoreFields(refreshMsg);

		System.out.println("End ETA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End ETA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Clone");

		com.refinitiv.eta.codec.RefreshMsg refreshMsgDecode = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		refreshMsgDecode.decode(decIter);

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.RefreshMsg emaRefreshMsgClone = EmaFactory.createRefreshMsg(emaRefreshMsg);

		compareEmaRefreshMsgFields(emaRefreshMsg, emaRefreshMsgClone, "Refresh cloned message");

		String emaRefreshMsgString = emaRefreshMsg.toString();
		String emaRefreshMsgCloneString = emaRefreshMsgClone.toString();
		
		System.out.println("Cloned EMA RefreshMsg:");
		System.out.println(emaRefreshMsgClone);
		
		TestUtilities.checkResult(emaRefreshMsgString.equals(emaRefreshMsgCloneString), "emaRefreshMsgString.equals(emaRefreshMsgCloneString)");


		com.refinitiv.ema.access.RefreshMsg emaRefreshMsgClone2 = EmaFactory.createRefreshMsg(emaRefreshMsgClone);
		compareEmaRefreshMsgFields(emaRefreshMsg, emaRefreshMsgClone2, "Refresh double-cloned message");
		String emaRefreshMsgClone2String = emaRefreshMsgClone2.toString();
		TestUtilities.checkResult(emaRefreshMsgString.equals(emaRefreshMsgClone2String), "double-cloned emaRefreshMsgString.equals(emaRefreshMsgClone2String)");

		System.out.println("End EMA RefreshMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.refinitiv.eta.codec.RefreshMsg refreshMsg) {
		refreshMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		refreshMsg.extendedHeader(extendedHeader);

		Buffer itemGroup = CodecFactory.createBuffer();
		itemGroup.data(ByteBuffer.wrap(new byte[] {30, 40, 50, 60, 77, 77, 77, 77, 88}));
		refreshMsg.groupId(itemGroup);

		refreshMsg.applyHasPermData();
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data(ByteBuffer.wrap(new byte[]{50, 51, 52, 53}));
		refreshMsg.permData(permissionData);

		refreshMsg.applyPrivateStream();
	}

	public void testRefreshMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testRefreshMsg_cloneEdit", "clone and edit ema refresh message");
		
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
		
	    System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );
		
		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End ETA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Clone");

		com.refinitiv.eta.codec.RefreshMsg refreshMsgDecode = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		refreshMsgDecode.decode(decIter);

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.RefreshMsg emaRefreshMsgClone = EmaFactory.createRefreshMsg(emaRefreshMsg);

		compareEmaRefreshMsgFields(emaRefreshMsg, emaRefreshMsgClone, "Refresh cloned message");

		String emaRefreshMsgString = emaRefreshMsg.toString();
		String emaRefreshMsgCloneString = emaRefreshMsgClone.toString();
		
		System.out.println("Cloned EMA RefreshMsg:");
		System.out.println(emaRefreshMsgClone);
		
		TestUtilities.checkResult(emaRefreshMsgString.equals(emaRefreshMsgCloneString), "emaRefreshMsgString.equals(emaRefreshMsgCloneString)");
		
		emaRefreshMsgClone.streamId(10);
		emaRefreshMsgClone.payload().fieldList().add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		TestUtilities.checkResult(emaRefreshMsgClone.streamId() != emaRefreshMsg.streamId(), "Compare streamId");

		// Check emaRefreshMsg for no FID 21
		Iterator<FieldEntry> iter = emaRefreshMsg.payload().fieldList().iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			TestUtilities.checkResult(fieldEntry.fieldId() != 21, "Check emaRefreshMsg for no FID 21");
		}
		
		boolean foundFid = false;
		Iterator<FieldEntry> iterClone = emaRefreshMsgClone.payload().fieldList().iterator();
		while (iterClone.hasNext())
		{
			fieldEntry = iterClone.next();
			if(foundFid = fieldEntry.fieldId() == 21)
				break;
		}
		
		TestUtilities.checkResult(foundFid, "Check emaRefreshMsgClone for FID 21");
		
		emaRefreshMsgString = emaRefreshMsg.toString();
		emaRefreshMsgCloneString = emaRefreshMsgClone.toString();
		
		TestUtilities.checkResult(!emaRefreshMsgString.equals(emaRefreshMsgCloneString), "Check that emaRefreshMsgString does not equal emaRefreshMsgCloneString");
		
		
		System.out.println("End EMA RefreshMsg Clone");
		System.out.println();
	}

	private void compareEmaRefreshMsgFields(RefreshMsg expected, RefreshMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + " base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasServiceName() == actual.hasServiceName(), checkPrefix + "hasServiceName");
		TestUtilities.checkResult(expected.doNotCache() == actual.doNotCache(), checkPrefix + "doNotCache");
		if(expected.hasServiceName())
			TestUtilities.checkResult(expected.serviceName().equals(actual.serviceName()), checkPrefix + "serviceId" + "exp=" +actual.serviceName() + " act="+expected.serviceName());

			TestUtilities.checkResult(expected.state().code() == actual.state().code(), checkPrefix + "state.code");
			TestUtilities.checkResult(expected.state().statusCode() == actual.state().statusCode(), checkPrefix + "state.statusCode");
			TestUtilities.checkResult(expected.state().streamState() == actual.state().streamState(), checkPrefix + "state.streamState()");
			TestUtilities.checkResult(expected.state().statusText().equals(actual.state().statusText()), checkPrefix + "state.statusText");

		TestUtilities.checkResult(expected.hasPublisherId() == actual.hasPublisherId(), checkPrefix + "hasPublisherId");
		if(expected.hasPublisherId()) {
			TestUtilities.checkResult(expected.publisherIdUserId() == actual.publisherIdUserId(), checkPrefix + "publisherIdUserId");
			TestUtilities.checkResult(expected.publisherIdUserAddress() == actual.publisherIdUserAddress(), checkPrefix + "publisherIdUserAddress");
		}

		TestUtilities.checkResult(expected.clearCache() == actual.clearCache(), checkPrefix + "clearCache");
		TestUtilities.checkResult(expected.privateStream() == actual.privateStream(), checkPrefix + "privateStream");

		TestUtilities.checkResult(expected.itemGroup().equals(actual.itemGroup()), checkPrefix + "itemGroup");

		TestUtilities.checkResult(expected.hasPermissionData() == actual.hasPermissionData(), checkPrefix + "hasPermissionData");
		if(expected.hasPermissionData())
			TestUtilities.checkResult(expected.permissionData().equals(actual.permissionData()), checkPrefix + "permissionData");

		TestUtilities.checkResult(expected.solicited() == actual.solicited(), checkPrefix + "solicited");

		TestUtilities.checkResult(expected.hasQos() == actual.hasQos(), checkPrefix + "hasQos");
		if(expected.hasQos()) {
			TestUtilities.checkResult(expected.qos().rate() == actual.qos().rate(), checkPrefix + "qos.rate");
			TestUtilities.checkResult(expected.qos().timeliness() == actual.qos().timeliness(), checkPrefix + "qos.timeliness");
		}

		TestUtilities.checkResult(expected.hasSeqNum() == actual.hasSeqNum(), checkPrefix + "hasSeqNum");
		if(expected.hasSeqNum())
			TestUtilities.checkResult(expected.seqNum() == actual.seqNum(), checkPrefix + "seqNum");

		TestUtilities.checkResult(expected.hasPartNum() == actual.hasPartNum(), checkPrefix + "hasPartNum");
		if(expected.hasPartNum())
			TestUtilities.checkResult(expected.partNum() == actual.partNum(), checkPrefix + "partNum");

		TestUtilities.checkResult(expected.clearCache() == actual.clearCache(), checkPrefix + "clearCache");
	}

	public void testRefreshMsg_EncodeETA_XmlPayload_DecodeEMA()
	{
		final String XML_STRING = "<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";

		TestUtilities.printTestHead("testRefreshMsg_EncodeETA_XmlPayload_DecodeEMA", "Encode RefreshMsg with ETA for XML payload, decode it with EMA");

		com.refinitiv.eta.codec.Buffer dataBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		dataBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.Buffer xmlBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		xmlBuf.data(ByteBuffer.allocate(1024));
		xmlBuf.data(XML_STRING);

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA XML Encoding");
		if ((retVal = TestUtilities.eta_EncodeNonRWFData(dataBuf, xmlBuf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding Xml.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeNonRWFData.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA XML Encoding");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);

		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );

		refreshMsg.streamId( 15 );

		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );

		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );

		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);

		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );

		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);

		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");

		refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();

		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);

		refreshMsg.containerType(com.refinitiv.eta.codec.DataTypes.XML);
		refreshMsg.encodedDataBody(xmlBuf);

		System.out.println("End ETA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Buffer Encoding");

		com.refinitiv.eta.codec.Buffer msgBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));

		com.refinitiv.eta.codec.EncodeIterator encIter = com.refinitiv.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + " encountered with setBufferAndRWFVersion. "
					+ " Error Text: " + CodecReturnCodes.info(retVal));
			return;
		}

		refreshMsg.encode(encIter);

		System.out.println("End ETA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();

		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");

		TestUtilities.checkResult(emaRefreshMsg.streamId() == 15, "RefreshMsg.streamId()");

		TestUtilities.checkResult(emaRefreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");

		TestUtilities.checkResult(emaRefreshMsg.partNum() == 10, "RefreshMsg.partNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");

		TestUtilities.checkResult(emaRefreshMsg.seqNum() == 22, "RefreshMsg.seqNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasQos(), "RefreshMsg.hasQos()");

		TestUtilities.checkResult(emaRefreshMsg.qos().timeliness() == OmmQos.Timeliness.REALTIME, "RefreshMsg.qos().timeliness()");

		TestUtilities.checkResult(emaRefreshMsg.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "RefreshMsg.qos().rate()");

		TestUtilities.checkResult(emaRefreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(emaRefreshMsg.hasId(), "RefreshMsg.hasId()");

		TestUtilities.checkResult(emaRefreshMsg.id() == 21, "RefreshMsg.id()");

		TestUtilities.checkResult(emaRefreshMsg.hasFilter(), "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(emaRefreshMsg.filter() == 12 , "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(emaRefreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");

		TestUtilities.checkResult(emaRefreshMsg.serviceId() == 5 , "RefreshMsg.serviceId()");

		TestUtilities.checkResult(emaRefreshMsg.hasNameType(), "RefreshMsg.hasNameType()");

		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");

		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");

		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.XML, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.payload().xml().string().equals(XML_STRING), "RefreshMsg.payload().xml().string()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");

		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().equals("refresh complete"), "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();
	}

	public void testRefreshMsg_EncodeETA_JsonPayload_DecodeEMA()
	{
		final String JSON_STRING = "{\"consumerList\" : {\"consumer\" : {\"name\" : \"Consumer_1\"} } }";

		TestUtilities.printTestHead("testRefreshMsg_EncodeETA_JsonPayload_DecodeEMA", "Encode RefreshMsg with ETA for Json payload, decode it with EMA");

		com.refinitiv.eta.codec.Buffer dataBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		dataBuf.data(ByteBuffer.allocate(1024));

		com.refinitiv.eta.codec.Buffer jsonBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		jsonBuf.data(ByteBuffer.allocate(1024));
		jsonBuf.data(JSON_STRING);

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin ETA Json Encoding");
		if ((retVal = TestUtilities.eta_EncodeNonRWFData(dataBuf, jsonBuf)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding Json.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeNonRWFData.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA Json Encoding");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Set");
		com.refinitiv.eta.codec.RefreshMsg refreshMsg = (com.refinitiv.eta.codec.RefreshMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REFRESH);

		refreshMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );

		refreshMsg.streamId( 15 );

		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );

		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.refinitiv.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.refinitiv.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );

		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);

		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );

		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);

		refreshMsg.state().code( com.refinitiv.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.refinitiv.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.refinitiv.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");

		refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();

		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);

		refreshMsg.containerType(com.refinitiv.eta.codec.DataTypes.JSON);
		refreshMsg.encodedDataBody(jsonBuf);

		System.out.println("End ETA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin ETA RefreshMsg Buffer Encoding");

		com.refinitiv.eta.codec.Buffer msgBuf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		msgBuf.data(ByteBuffer.allocate(2048));

		com.refinitiv.eta.codec.EncodeIterator encIter = com.refinitiv.eta.codec.CodecFactory.createEncodeIterator();
		encIter.clear();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		if ((retVal = encIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal + " encountered with setBufferAndRWFVersion. "
					+ " Error Text: " + CodecReturnCodes.info(retVal));
			return;
		}

		refreshMsg.encode(encIter);

		System.out.println("End ETA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		com.refinitiv.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();

		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");

		TestUtilities.checkResult(emaRefreshMsg.streamId() == 15, "RefreshMsg.streamId()");

		TestUtilities.checkResult(emaRefreshMsg.hasPartNum(), "RefreshMsg.hasPartNum()");

		TestUtilities.checkResult(emaRefreshMsg.partNum() == 10, "RefreshMsg.partNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasSeqNum(), "RefreshMsg.hasSeqNum()");

		TestUtilities.checkResult(emaRefreshMsg.seqNum() == 22, "RefreshMsg.seqNum()");

		TestUtilities.checkResult(emaRefreshMsg.hasQos(), "RefreshMsg.hasQos()");

		TestUtilities.checkResult(emaRefreshMsg.qos().timeliness() == OmmQos.Timeliness.REALTIME, "RefreshMsg.qos().timeliness()");

		TestUtilities.checkResult(emaRefreshMsg.qos().rate() == OmmQos.Rate.TICK_BY_TICK, "RefreshMsg.qos().rate()");

		TestUtilities.checkResult(emaRefreshMsg.hasMsgKey(), "RefreshMsg.hasMsgKey()");

		TestUtilities.checkResult(emaRefreshMsg.hasId(), "RefreshMsg.hasId()");

		TestUtilities.checkResult(emaRefreshMsg.id() == 21, "RefreshMsg.id()");

		TestUtilities.checkResult(emaRefreshMsg.hasFilter(), "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(emaRefreshMsg.filter() == 12 , "RefreshMsg.hasFilter()");

		TestUtilities.checkResult(emaRefreshMsg.hasServiceId(), "RefreshMsg.hasServiceId()");

		TestUtilities.checkResult(emaRefreshMsg.serviceId() == 5 , "RefreshMsg.serviceId()");

		TestUtilities.checkResult(emaRefreshMsg.hasNameType(), "RefreshMsg.hasNameType()");

		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");

		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");

		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.JSON, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.payload().json().string().equals(JSON_STRING), "RefreshMsg.payload().json().string()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");

		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().equals("refresh complete"), "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();
	}
}
