///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.     
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

public class PostMsgTests extends TestCase
{
	public PostMsgTests(String name)
	{
		super(name);
	}
	
	public void testPostMsg_Decode()
	{
		TestUtilities.printTestHead("testPostMsg_Decode", "eta encoding ema decoding");
		
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
		
	    System.out.println("Begin ETA PostMsg Set");
		com.refinitiv.eta.codec.PostMsg postMsg = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.POST);
		
		postMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.refinitiv.eta.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA PostMsg Set");
		System.out.println();

		System.out.println("Begin ETA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End ETA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg Decoding");

		com.refinitiv.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
		
		JUnitTestConnect.setRsslData(emaPostMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaPostMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(emaPostMsg.streamId() == 15, "PostMsg.streamId()");

		TestUtilities.checkResult(emaPostMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(emaPostMsg.partNum() == 10, "PostMsg.partNum()");

		TestUtilities.checkResult(emaPostMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaPostMsg.seqNum() == 22, "PostMsg.seqNum()");

		TestUtilities.checkResult(emaPostMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(emaPostMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(emaPostMsg.id() == 21, "PostMsg.id()");

		TestUtilities.checkResult(emaPostMsg.hasPostUserRights(), "PostMsg.hasPostUserRights()");
		
		TestUtilities.checkResult(emaPostMsg.postUserRights() == PostMsg.PostUserRights.CREATE, "PostMsg.postUserRights()");
		
		TestUtilities.checkResult(emaPostMsg.hasPostId(), "PostMsg.hasPostId()");
		
		TestUtilities.checkResult(emaPostMsg.postId() == 223,  "PostMsg.postId()");
		
		TestUtilities.checkResult(emaPostMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(emaPostMsg.filter() == 12 , "PostMsg.hasFilter()");

		TestUtilities.checkResult(emaPostMsg.solicitAck(), "PostMsg.solicitAck()");
		
		TestUtilities.checkResult(emaPostMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaPostMsg.serviceId() == 5 , "PostMsg.serviceId()");

		TestUtilities.checkResult(emaPostMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(emaPostMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "PostMsg.nameType()");

		TestUtilities.checkResult(emaPostMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(emaPostMsg.name().compareTo("ABCDEF") == 0, "PostMsg.name()");

		TestUtilities.checkResult(emaPostMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaPostMsg.publisherIdUserAddress() == 15, "PostMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaPostMsg.publisherIdUserId() == 30, "PostMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaPostMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.payload().dataType()");

		System.out.println("End EMA PostMsg Decoding");
		System.out.println();
	}

	public void testPostMsg_toString()
	{
		TestUtilities.printTestHead("testPostMsg_toString", "eta encoding ema toString");
		
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
		
	    System.out.println("Begin ETA PostMsg Set");
		com.refinitiv.eta.codec.PostMsg postMsg = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.POST);
		
		postMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.refinitiv.eta.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA PostMsg Set");
		System.out.println();

		System.out.println("Begin ETA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End ETA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg toString");

		com.refinitiv.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
		
		JUnitTestConnect.setRsslData(emaPostMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaPostMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("PostMsg.toString() != toString()", !(emaPostMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));
		

		System.out.println("End EMA PostMsg toString");
		System.out.println();
	}
	
	public void testPostMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testPostMsg_EncodeDecode", "ema encoding ema decoding");

		String postMsgString = "PostMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    seqNum=\"22\"\n" +
				"    partNum=\"10\"\n" +
				"    publisherIdUserId=\"30\"\n" +
				"    publisherIdUserAddress=\"15\"\n" +
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
				"PostMsgEnd\n";

		String postMsgEmptyString = "PostMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"    publisherIdUserId=\"0\"\n" +
				"    publisherIdUserAddress=\"0\"\n" +
				"PostMsgEnd\n";

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.PostMsg postMsg = EmaFactory.createPostMsg();
	    com.refinitiv.ema.access.PostMsg postMsgEmpty = EmaFactory.createPostMsg();

		System.out.println("Begin EMA PostMsg test after constructor");

		TestUtilities.checkResult(postMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(postMsg.streamId() == 0, "PostMsg.streamId()");

		TestUtilities.checkResult(!postMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(!postMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!postMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(!postMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(!postMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(!postMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(!postMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(!postMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.payload().dataType()");

		System.out.println("Emd EMA PostMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA PostMsg Set");
	    
    	postMsg.domainType( com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.streamId( 15 );
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.partNum( 10 );
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.seqNum( 22 );
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		postMsg.name("ABCDEF");
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		postMsg.serviceId(5);
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.filter( 12 );
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		postMsg.id(21);
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.attrib(fl);
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
		postMsg.publisherId(30,  15);
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
		postMsg.payload(fl);
		TestUtilities.checkResult("PostMsg.toString() == toString()", postMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		System.out.println("End EMA PostMsg Set");
		System.out.println();

		System.out.println("Begin EMA PostMsg Decoding");

		DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("PostMsg.toString(dictionary) == toString(dictionary)", postMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("PostMsg.toString(dictionary) == toString(dictionary)", postMsgEmpty.toString(emaDataDictionary).equals(postMsgEmptyString));

		TestUtilities.checkResult("PostMsg.toString(dictionary) == toString(dictionary)", postMsg.toString(emaDataDictionary).equals(postMsgString));

		com.refinitiv.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();

		JUnitTestConnect.setRsslData(emaPostMsg, postMsg, 14, 0, dictionary, null);

		com.refinitiv.ema.access.PostMsg postMsgClone = EmaFactory.createPostMsg(postMsg);
		postMsgClone.clear();
		TestUtilities.checkResult("PostMsg.toString(dictionary) == toString(dictionary)", postMsgClone.toString(emaDataDictionary).equals(postMsgEmptyString));

		TestUtilities.checkResult(emaPostMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(emaPostMsg.streamId() == 15, "PostMsg.streamId()");

		TestUtilities.checkResult(emaPostMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(emaPostMsg.partNum() == 10, "PostMsg.partNum()");

		TestUtilities.checkResult(emaPostMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(emaPostMsg.seqNum() == 22, "PostMsg.seqNum()");

		TestUtilities.checkResult(emaPostMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(emaPostMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(emaPostMsg.id() == 21, "PostMsg.id()");

		TestUtilities.checkResult(emaPostMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(emaPostMsg.filter() == 12 , "PostMsg.hasFilter()");

		TestUtilities.checkResult(emaPostMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(emaPostMsg.serviceId() == 5 , "PostMsg.serviceId()");

		TestUtilities.checkResult(emaPostMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(emaPostMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "PostMsg.nameType()");

		TestUtilities.checkResult(emaPostMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(emaPostMsg.name().compareTo("ABCDEF") == 0, "PostMsg.name()");

		TestUtilities.checkResult(emaPostMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaPostMsg.publisherIdUserAddress() == 15, "PostMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaPostMsg.publisherIdUserId() == 30, "PostMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaPostMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.payload().dataType()");

		System.out.println("End EMA PostMsg Decoding");
		System.out.println();

		postMsg.clear();
		
		System.out.println("Begin EMA PostMsg test after clear");

		TestUtilities.checkResult(postMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(postMsg.streamId() == 0, "PostMsg.streamId()");

		TestUtilities.checkResult(!postMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(!postMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!postMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(!postMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(!postMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(!postMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(!postMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(!postMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(postMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(postMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.payload().dataType()");

		System.out.println("End EMA PostMsg test after clear");
		System.out.println();
	}
	
	public void testPostMsg_EncodeETAPostMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETAPostMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for FieldList as attrib, extended header and payload. Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode Msg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyPostMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyPostMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestPostMsg_EncodeETAPostMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeETAPostMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETAPostMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyPostMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyPostMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestPostMsg_EncodeETAPostMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeETAPostMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETAPostMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for FilterList as attrib, payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyPostMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyPostMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestPostMsg_EncodeETAPostMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeETAPostMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETAPostMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for Series as attrib, payload, Encode it to another PostMsg.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buffer = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14240));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyPostMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyPostMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeETAPostMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeETAPostMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETAPostMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for Vector as attrib, payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyPostMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyPostMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeETAPostMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for Map as attrib, payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyPostMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyPostMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeETAPostMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeETAPostMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with ETA for RefreshMsg as attrib, payload. Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodePostMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.refinitiv.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETAPostMsgAll(postMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.refinitiv.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyPostMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyPostMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeETAPostMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}

	public void testPostMsgMsg_cloneIsNotSupportedFromTheEncodeSide()
	{
		TestUtilities.printTestHead("testPostMsgMsg_cloneIsNotSupportedFromTheEncodeSide", "cloning is not supported on encode side");
		PostMsg msg = EmaFactory.createPostMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE);

		try {
			PostMsg cloneMessage = EmaFactory.createPostMsg(msg);
			TestUtilities.checkResult(false, "Clone not supported - exception expected: ");
		} catch ( OmmException excp ) {
			TestUtilities.checkResult(true, "Clone not supported  - exception expected: " +  excp.getMessage() );
			TestUtilities.checkResult(excp.getMessage().startsWith("Failed to clone empty encoded buffer"), "Clone not supported - exception text validated");
		}
	}

	public void testPostMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testPostMsg_cloneMsgKeyWLScenario", "cloning for minimal ema post message");
		PostMsg emaMsg = EmaFactory.createPostMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, true);
		PostMsg emaClonedMsg = EmaFactory.createPostMsg(emaMsg);

		compareEmaPostMsgFields(emaMsg, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, false);

		System.out.println("End EMA PostMsg Clone msgKey");
		System.out.println();
	}

	public void testPostMsg_clone()
	{
		TestUtilities.printTestHead("testPostMsg_clone", "cloning for ema post message");
		
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
		
	    System.out.println("Begin ETA PostMsg Set");
		com.refinitiv.eta.codec.PostMsg postMsg = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.POST);
		
		postMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.refinitiv.eta.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		setMoreFields(postMsg);

		System.out.println("End ETA PostMsg Set");
		System.out.println();

		System.out.println("Begin ETA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End ETA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg Clone");

		com.refinitiv.eta.codec.PostMsg postMsgDecode = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		postMsgDecode.decode(decIter);

		com.refinitiv.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
				
		JUnitTestConnect.setRsslData(emaPostMsg, postMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.PostMsg emaPostMsgClone = EmaFactory.createPostMsg(emaPostMsg);
		

		compareEmaPostMsgFields(emaPostMsg, emaPostMsgClone, "Post clone message");

		String emaPostMsgString = emaPostMsg.toString();
		String emaPostMsgCloneString = emaPostMsgClone.toString();
		
		System.out.println("Cloned EMA PostMsg:");
		System.out.println(emaPostMsgClone);
		
		TestUtilities.checkResult(emaPostMsgString.equals(emaPostMsgCloneString), "emaPostMsgString.equals(emaPostMsgCloneString)");

		com.refinitiv.ema.access.PostMsg emaPostMsgClone2 = EmaFactory.createPostMsg(emaPostMsgClone);
		compareEmaPostMsgFields(emaPostMsg, emaPostMsgClone2, "Post double-cloned message");
		String emaPostMsgClone2String = emaPostMsgClone2.toString();
		TestUtilities.checkResult(emaPostMsgString.equals(emaPostMsgClone2String), "double-cloned emaPostMsgString.equals(emaPostMsgClone2String)");


		System.out.println("End EMA PostMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.refinitiv.eta.codec.PostMsg postMsg) {
		postMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		postMsg.extendedHeader(extendedHeader);

		postMsg.applyHasPermData();
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data(ByteBuffer.wrap(new byte[]{50, 51, 52, 53}));
		postMsg.permData(permissionData);
	}

	public void testPostMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testPostMsg_cloneEdit", "clone and edit ema post message");
		
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
		
	    System.out.println("Begin ETA PostMsg Set");
		com.refinitiv.eta.codec.PostMsg postMsg = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.POST);
		
		postMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.refinitiv.eta.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA PostMsg Set");
		System.out.println();

		System.out.println("Begin ETA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End ETA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg Clone");

		com.refinitiv.eta.codec.PostMsg postMsgDecode = (com.refinitiv.eta.codec.PostMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		postMsgDecode.decode(decIter);

		com.refinitiv.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
				
		JUnitTestConnect.setRsslData(emaPostMsg, postMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.PostMsg emaPostMsgClone = EmaFactory.createPostMsg(emaPostMsg);
		
		TestUtilities.checkResult(emaPostMsgClone.domainType() == emaPostMsg.domainType(), "Compare domainType");
		TestUtilities.checkResult(emaPostMsgClone.streamId() == emaPostMsg.streamId(), "Compare streamId");
		TestUtilities.checkResult(emaPostMsgClone.hasPartNum() == emaPostMsg.hasPartNum(), "Compare hasPartNum");
		TestUtilities.checkResult(emaPostMsgClone.partNum() == emaPostMsg.partNum(), "Compare partNum");
		TestUtilities.checkResult(emaPostMsgClone.hasSeqNum() == emaPostMsg.hasSeqNum(), "Compare hasSeqNum");
		TestUtilities.checkResult(emaPostMsgClone.seqNum() == emaPostMsg.seqNum(), "Compare seqNum");
		TestUtilities.checkResult(emaPostMsgClone.hasPostId() == emaPostMsg.hasPostId(), "Compare hasPostId");
		TestUtilities.checkResult(emaPostMsgClone.postId() == emaPostMsg.postId(), "Compare postId");
		TestUtilities.checkResult(emaPostMsgClone.hasPostUserRights() == emaPostMsg.hasPostUserRights(), "Compare hasPostUserRights");
		TestUtilities.checkResult(emaPostMsgClone.postUserRights() == emaPostMsg.postUserRights(), "Compare postUserRights");
		TestUtilities.checkResult(emaPostMsgClone.hasMsgKey() == emaPostMsg.hasMsgKey(), "Compare hasMsgKey");

		String emaPostMsgString = emaPostMsg.toString();
		String emaPostMsgCloneString = emaPostMsgClone.toString();
		
		System.out.println("Cloned EMA PostMsg:");
		System.out.println(emaPostMsgClone);
		
		TestUtilities.checkResult(emaPostMsgString.equals(emaPostMsgCloneString), "emaPostMsgString.equals(emaPostMsgCloneString)");
		
		emaPostMsgClone.streamId(10);
		emaPostMsgClone.payload().fieldList().add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		TestUtilities.checkResult(emaPostMsgClone.streamId() != emaPostMsg.streamId(), "Compare streamId");

		// Check emaPostMsg for no FID 21
		Iterator<FieldEntry> iter = emaPostMsg.payload().fieldList().iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			TestUtilities.checkResult(fieldEntry.fieldId() != 21, "Check emaPostMsg for no FID 21");
		}
		
		boolean foundFid = false;
		Iterator<FieldEntry> iterClone = emaPostMsgClone.payload().fieldList().iterator();
		while (iterClone.hasNext())
		{
			fieldEntry = iterClone.next();
			if(foundFid = fieldEntry.fieldId() == 21)
				break;
		}
		
		TestUtilities.checkResult(foundFid, "Check emaPostMsgClone for FID 21");
		
		emaPostMsgString = emaPostMsg.toString();
		emaPostMsgCloneString = emaPostMsgClone.toString();
		
		TestUtilities.checkResult(!emaPostMsgString.equals(emaPostMsgCloneString), "Check that emaPostMsgString does not equal emaPostMsgCloneString");
		
		
		System.out.println("End EMA PostMsg Clone");
		System.out.println();
	}

	private void compareEmaPostMsgFields(PostMsg expected, PostMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + " base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasServiceName() == actual.hasServiceName(), checkPrefix + "hasServiceName");
		if(expected.hasServiceName())
			TestUtilities.checkResult(expected.serviceName().equals(actual.serviceName()), checkPrefix + "serviceId" + "exp=" +actual.serviceName() + " act="+expected.serviceName());

		TestUtilities.checkResult(expected.hasSeqNum() == actual.hasSeqNum(), checkPrefix + "hasSeqNum");
		if(expected.hasSeqNum())
			TestUtilities.checkResult(expected.seqNum() == actual.seqNum(), checkPrefix + "seqNum");

		TestUtilities.checkResult(expected.hasPartNum() == actual.hasPartNum(), checkPrefix + "hasPartNum");
		if(expected.hasPartNum())
			TestUtilities.checkResult(expected.partNum() == actual.partNum(), checkPrefix + "partNum");

		TestUtilities.checkResult(expected.publisherIdUserId() == actual.publisherIdUserId(), checkPrefix + "publisherIdUserId");
		TestUtilities.checkResult(expected.publisherIdUserAddress() == actual.publisherIdUserAddress(), checkPrefix + "publisherIdUserAddress");

		TestUtilities.checkResult(expected.hasPostUserRights() == actual.hasPostUserRights(), checkPrefix + "hasPostUserRights");
		if (expected.hasPostUserRights())
			TestUtilities.checkResult(expected.postUserRights() == actual.postUserRights(), checkPrefix + "postUserRights");

		TestUtilities.checkResult(expected.hasPostId() == actual.hasPostId(), checkPrefix + "hasPostId");
		if (expected.hasPostId())
			TestUtilities.checkResult(expected.postId() == actual.postId(), checkPrefix + "postId");

		TestUtilities.checkResult(expected.solicitAck() == actual.solicitAck(), checkPrefix + "solicitAck");

		TestUtilities.checkResult(expected.hasPermissionData() == actual.hasPermissionData(), checkPrefix + "hasPermissionData");
		if(expected.hasPermissionData())
			TestUtilities.checkResult(expected.permissionData().equals(actual.permissionData()), checkPrefix + "permissionData");
	}
}
