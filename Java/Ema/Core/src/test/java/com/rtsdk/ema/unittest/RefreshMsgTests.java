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

public class RefreshMsgTests extends TestCase
{
	public RefreshMsgTests(String name)
	{
		super(name);
	}
	
	public void testRefreshMsg_Decode()
	{
		TestUtilities.printTestHead("testRefreshMsg_Decode", "upa encoding ema decoding");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin UPA RefreshMsg Set");
		com.rtsdk.eta.codec.RefreshMsg refreshMsg = (com.rtsdk.eta.codec.RefreshMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.rtsdk.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.rtsdk.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.rtsdk.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.rtsdk.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.rtsdk.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin UPA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End UPA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		com.rtsdk.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.rtsdk.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().compareTo("refresh complete") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();
	}

	public void testRefreshMsg_toString()
	{
		TestUtilities.printTestHead("testRefreshMsg_toString", "upa encoding ema toString");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin UPA RefreshMsg Set");
		com.rtsdk.eta.codec.RefreshMsg refreshMsg = (com.rtsdk.eta.codec.RefreshMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.rtsdk.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.rtsdk.eta.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.rtsdk.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.rtsdk.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.rtsdk.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin UPA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End UPA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg toString");

		com.rtsdk.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaRefreshMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("RefreshMsg.toString() != toString() not supported", !(emaRefreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));			
		
		System.out.println("End EMA RefreshMsg toString");
		System.out.println();
	}
	
	public void testRefreshMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testRefreshMsg_EncodeDecode", "ema encoding ema decoding");
		
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

	    com.rtsdk.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.rtsdk.ema.access.RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
	    
		System.out.println("Begin EMA RefreshMsg test after constructor");

		TestUtilities.checkResult(refreshMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(!refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(!refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(!refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA RefreshMsg Set");
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
    	refreshMsg.domainType( com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.streamId( 15 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.partNum( 10 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.seqNum( 22 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

		refreshMsg.qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

		refreshMsg.name("ABCDEF");
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

		refreshMsg.serviceId(5);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.filter( 12 );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
	
		refreshMsg.id(21);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.attrib(fl);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
	
		refreshMsg.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Complete");
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
	    refreshMsg.clearCache(true);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		refreshMsg.doNotCache(true);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		refreshMsg.solicited(true);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.publisherId(30,  15);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		refreshMsg.payload(fl);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

		System.out.println("End EMA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		com.rtsdk.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();

		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.rtsdk.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().compareTo("Refresh Complete") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();

		refreshMsg.clear();
		
		System.out.println("Begin EMA RefreshMsg test after clear");

		TestUtilities.checkResult(refreshMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(!refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(!refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(!refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg test after clear");
		System.out.println();
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for FieldList as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyRefreshMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyRefreshMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for ElementList as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyRefreshMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyRefreshMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithFilterListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithFilterListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for FilterList as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.FILTER_LIST);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyRefreshMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyRefreshMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithFilterListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Series as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.SERIES);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyRefreshMsg.attrib().series(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyRefreshMsg.payload().series(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Vector as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.VECTOR);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyRefreshMsg.attrib().vector(), com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyRefreshMsg.payload().vector(), com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Map as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.MAP);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyRefreshMsg.attrib().map(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyRefreshMsg.payload().map(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithRefreshTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Map as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.rtsdk.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.rtsdk.eta.codec.DataTypes.MSG);
	
	     com.rtsdk.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.rtsdk.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyRefreshMsg.attrib().refreshMsg(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyRefreshMsg.payload().refreshMsg(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithRefreshTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
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


		/** @see com.rtsdk.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.rtsdk.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		RefreshMsg emaMsg2 = JUnitTestConnect.createRefreshMsg();
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
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
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin UPA RefreshMsg Set");
		com.rtsdk.eta.codec.RefreshMsg refreshMsg = (com.rtsdk.eta.codec.RefreshMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.rtsdk.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.rtsdk.eta.codec.QosRates.TICK_BY_TICK );
		
		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.rtsdk.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.rtsdk.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.rtsdk.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		setMoreFields(refreshMsg);

		System.out.println("End UPA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin UPA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End UPA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Clone");

		com.rtsdk.eta.codec.RefreshMsg refreshMsgDecode = (com.rtsdk.eta.codec.RefreshMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();

		com.rtsdk.eta.codec.DecodeIterator decIter = com.rtsdk.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		refreshMsgDecode.decode(decIter);

		com.rtsdk.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.rtsdk.ema.access.RefreshMsg emaRefreshMsgClone = EmaFactory.createRefreshMsg(emaRefreshMsg);

		compareEmaRefreshMsgFields(emaRefreshMsg, emaRefreshMsgClone, "Refresh cloned message");

		String emaRefreshMsgString = emaRefreshMsg.toString();
		String emaRefreshMsgCloneString = emaRefreshMsgClone.toString();
		
		System.out.println("Cloned EMA RefreshMsg:");
		System.out.println(emaRefreshMsgClone);
		
		TestUtilities.checkResult(emaRefreshMsgString.equals(emaRefreshMsgCloneString), "emaRefreshMsgString.equals(emaRefreshMsgCloneString)");


		com.rtsdk.ema.access.RefreshMsg emaRefreshMsgClone2 = EmaFactory.createRefreshMsg(emaRefreshMsgClone);
		compareEmaRefreshMsgFields(emaRefreshMsg, emaRefreshMsgClone2, "Refresh double-cloned message");
		String emaRefreshMsgClone2String = emaRefreshMsgClone2.toString();
		TestUtilities.checkResult(emaRefreshMsgString.equals(emaRefreshMsgClone2String), "double-cloned emaRefreshMsgString.equals(emaRefreshMsgClone2String)");

		System.out.println("End EMA RefreshMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.rtsdk.eta.codec.RefreshMsg refreshMsg) {
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
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		int retVal;
		System.out.println("Begin UPA FieldList Encoding");
		if ((retVal = TestUtilities.upa_EncodeFieldListAll(fieldListBuf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
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
		
	    System.out.println("Begin UPA RefreshMsg Set");
		com.rtsdk.eta.codec.RefreshMsg refreshMsg = (com.rtsdk.eta.codec.RefreshMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.rtsdk.eta.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.rtsdk.eta.codec.QosRates.TICK_BY_TICK );
		
		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.rtsdk.eta.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.rtsdk.eta.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.rtsdk.eta.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin UPA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End UPA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Clone");

		com.rtsdk.eta.codec.RefreshMsg refreshMsgDecode = (com.rtsdk.eta.codec.RefreshMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();

		com.rtsdk.eta.codec.DecodeIterator decIter = com.rtsdk.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		refreshMsgDecode.decode(decIter);

		com.rtsdk.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.rtsdk.ema.access.RefreshMsg emaRefreshMsgClone = EmaFactory.createRefreshMsg(emaRefreshMsg);

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

}
