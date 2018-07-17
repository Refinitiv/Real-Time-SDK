///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class RefreshMsgTests extends TestCase
{
	public RefreshMsgTests(String name)
	{
		super(name);
	}
	
	public void testRefreshMsg_ServiceName_and_ServiceId()
	{
		TestUtilities.printTestHead("testRefreshMsg_ServiceName_and_ServiceId", "setting both serviceName and serviceId");
		
		com.thomsonreuters.ema.access.RefreshMsg emaRefreshMsg = EmaFactory.createRefreshMsg();
		
		emaRefreshMsg.serviceName("TEST");
		
		try {
			emaRefreshMsg.serviceId(5);
			TestUtilities.checkResult( false, "RefreshMsg can't set serviceId when serviceName is set - exception expected" );				
		}
		catch(Exception e)
		{
			TestUtilities.checkResult( true, "RefreshMsg can't set serviceId when serviceName is set - exception expected" );
		}		

		TestUtilities.checkResult(emaRefreshMsg.hasServiceName(), "RefreshMsg.hasServiceName()");			
		TestUtilities.checkResult(emaRefreshMsg.serviceName().equals("TEST"), "RefreshMsg.serviceName()");		
		
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();		
		com.thomsonreuters.ema.access.RefreshMsg emaRefreshMsgDec = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsgDec, emaRefreshMsg, majorVersion, minorVersion, TestUtilities.getDataDictionary(), null);
		
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("RefreshMsg.toString() != toString() not supported", !(emaRefreshMsgDec.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));	 		

		System.out.println("End EMA ServiceName and ServiceId");
		System.out.println();
	}		
	
	public void testRefreshMsg_Decode()
	{
		TestUtilities.printTestHead("testRefreshMsg_Decode", "upa encoding ema decoding");
		
		com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
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
		com.thomsonreuters.upa.codec.RefreshMsg refreshMsg = (com.thomsonreuters.upa.codec.RefreshMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.thomsonreuters.upa.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.thomsonreuters.upa.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.thomsonreuters.upa.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin UPA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End UPA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg Decoding");

		com.thomsonreuters.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
		JUnitTestConnect.setRsslData(emaRefreshMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");

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
		
		com.thomsonreuters.upa.codec.Buffer fieldListBuf = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
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
		com.thomsonreuters.upa.codec.RefreshMsg refreshMsg = (com.thomsonreuters.upa.codec.RefreshMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		refreshMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.REFRESH);
		
		refreshMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		refreshMsg.streamId( 15 );
		
		refreshMsg.applyHasPartNum();
		refreshMsg.partNum( 10 );
		
		refreshMsg.applyHasSeqNum();
		refreshMsg.seqNum( 22 );

		refreshMsg.applyHasQos();
		refreshMsg.qos().timeliness( com.thomsonreuters.upa.codec.QosTimeliness.REALTIME );
		refreshMsg.qos().rate( com.thomsonreuters.upa.codec.QosRates.TICK_BY_TICK );

		refreshMsg.applyHasMsgKey();

		refreshMsg.msgKey().applyHasName();
		refreshMsg.msgKey().name().data( "ABCDEF" );
		
		refreshMsg.msgKey().applyHasNameType();
		refreshMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		refreshMsg.msgKey().applyHasServiceId();
		refreshMsg.msgKey().serviceId(5);
		
		refreshMsg.msgKey().applyHasFilter();
		refreshMsg.msgKey().filter( 12 );
	
		refreshMsg.msgKey().applyHasIdentifier();
		refreshMsg.msgKey().identifier(21);
		
		refreshMsg.msgKey().applyHasAttrib();
		refreshMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		refreshMsg.msgKey().encodedAttrib(fieldListBuf);
	
		refreshMsg.state().code( com.thomsonreuters.upa.codec.StateCodes.NONE);
		refreshMsg.state().streamState(com.thomsonreuters.upa.codec.StreamStates.OPEN);
		refreshMsg.state().dataState(com.thomsonreuters.upa.codec.DataStates.OK);
		refreshMsg.state().text().data("refresh complete");
		
	    refreshMsg.applyClearCache();
		refreshMsg.applyDoNotCache();
		refreshMsg.applySolicited();
		
		refreshMsg.applyHasPostUserInfo();
		refreshMsg.postUserInfo().userAddr(15);
		refreshMsg.postUserInfo().userId(30);
		
		refreshMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		refreshMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA RefreshMsg Set");
		System.out.println();

		System.out.println("Begin UPA RefreshMsg Buffer Encoding");

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
		
		refreshMsg.encode(encIter);

	    System.out.println("End UPA RefreshMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA RefreshMsg toString");

		com.thomsonreuters.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();
		
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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

	    com.thomsonreuters.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.thomsonreuters.ema.access.RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
	    
		System.out.println("Begin EMA RefreshMsg test after constructor");

		TestUtilities.checkResult(refreshMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(!refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(!refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(!refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(refreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(refreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(refreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(refreshMsg.state().statusText().compareTo("") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA RefreshMsg Set");
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
    	refreshMsg.domainType( com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
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
		
		refreshMsg.nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));

		refreshMsg.serviceId(5);
		TestUtilities.checkResult("RefreshMsg.toString() == toString() not supported", refreshMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));
		
		try {
			refreshMsg.serviceName("TEST");
			TestUtilities.checkResult( false, "RefreshMsg can't set serviceName when serviceId is set - exception expected" );				
		}
		catch(Exception e)
		{
			TestUtilities.checkResult( true, "RefreshMsg can't set serviceName when serviceId is set - exception expected" );			
		}
		
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

		com.thomsonreuters.ema.access.RefreshMsg emaRefreshMsg = JUnitTestConnect.createRefreshMsg();

		JUnitTestConnect.setRsslData(emaRefreshMsg, refreshMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaRefreshMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaRefreshMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "RefreshMsg.nameType()");

		TestUtilities.checkResult(emaRefreshMsg.hasName(), "RefreshMsg.hasName()");
		
		TestUtilities.checkResult(emaRefreshMsg.name().compareTo("ABCDEF") == 0, "RefreshMsg.name()");

		TestUtilities.checkResult(emaRefreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaRefreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(emaRefreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(emaRefreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(emaRefreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserAddress() == 15, "RefreshMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaRefreshMsg.publisherIdUserId() == 30, "RefreshMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaRefreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "RefreshMsg.payload().dataType()");

		TestUtilities.checkResult(emaRefreshMsg.state().code() == OmmState.StatusCode.NONE, "RefreshMsg.state().code()");
		
		TestUtilities.checkResult(emaRefreshMsg.state().streamState() == OmmState.StreamState.OPEN, "RefreshMsg.state().streamState()");

		TestUtilities.checkResult(emaRefreshMsg.state().dataState() == OmmState.DataState.OK, "RefreshMsg.state().dataState()");

		TestUtilities.checkResult(emaRefreshMsg.state().statusText().compareTo("Refresh Complete") == 0, "RefreshMsg.state().statusText()");

		System.out.println("End EMA RefreshMsg Decoding");
		System.out.println();

		refreshMsg.clear();
		
		System.out.println("Begin EMA RefreshMsg test after clear");

		TestUtilities.checkResult(refreshMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "RefreshMsg.domainType()");
		
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
		
		TestUtilities.checkResult(refreshMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!refreshMsg.clearCache(), "RefreshMsg.clearCache()");

		TestUtilities.checkResult(!refreshMsg.doNotCache(), "RefreshMsg.doNotCache()");

		TestUtilities.checkResult(!refreshMsg.solicited(), "RefreshMsg.solicited()");

		TestUtilities.checkResult(!refreshMsg.hasPublisherId(), "RefreshMsg.hasPublisherId()");
		
		TestUtilities.checkResult(refreshMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "RefreshMsg.payload().dataType()");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
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
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
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
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.SERIES);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyRefreshMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyRefreshMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithSeriesTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Vector as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyRefreshMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyRefreshMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithVectorTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Map as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.MAP);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyRefreshMsg.attrib().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyRefreshMsg.payload().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	public void testRefreshMsg_EncodeUPARefreshWithRefreshTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testRefreshMsg_EncodeUPARefreshWithMapTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode", "Encode RefreshMsg with UPA for Map as attrib, extended header and payload, Encode it to another RefreshMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(20000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeRefreshMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRefreshMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode RefreshMsg with EMA.
	     com.thomsonreuters.ema.access.RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
	     JUnitTestConnect.setRsslData(refreshMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARefreshMsgAll(refreshMsg, com.thomsonreuters.upa.codec.DataTypes.MSG);
	
	     com.thomsonreuters.ema.access.RefreshMsg copyRefreshMsg = EmaFactory.createRefreshMsg();
	     
	     // Encode to another RefreshMsg
	     copyRefreshMsg.extendedHeader(refreshMsg.extendedHeader());
	     copyRefreshMsg.permissionData(refreshMsg.permissionData());
	     copyRefreshMsg.attrib(refreshMsg.attrib().data());
	     copyRefreshMsg.payload(refreshMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.RefreshMsg decCopyRefreshMsg = JUnitTestConnect.createRefreshMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyRefreshMsg, copyRefreshMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyRefreshMsg.extendedHeader().equals(refreshMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyRefreshMsg.permissionData().equals(refreshMsg.permissionData()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyRefreshMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyRefreshMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestRefreshMsg_EncodeUPARefreshWithRefreshTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherRefreshMsg_EMADecode passed");
	}
	
	
}
