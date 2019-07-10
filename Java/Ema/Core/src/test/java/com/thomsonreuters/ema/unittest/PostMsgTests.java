///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015, 2019. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class PostMsgTests extends TestCase
{
	public PostMsgTests(String name)
	{
		super(name);
	}
	
	public void testPostMsg_Decode()
	{
		TestUtilities.printTestHead("testPostMsg_Decode", "upa encoding ema decoding");
		
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
		
	    System.out.println("Begin UPA PostMsg Set");
		com.thomsonreuters.upa.codec.PostMsg postMsg = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.POST);
		
		postMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.thomsonreuters.upa.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA PostMsg Set");
		System.out.println();

		System.out.println("Begin UPA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End UPA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg Decoding");

		com.thomsonreuters.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
		
		JUnitTestConnect.setRsslData(emaPostMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaPostMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaPostMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "PostMsg.nameType()");

		TestUtilities.checkResult(emaPostMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(emaPostMsg.name().compareTo("ABCDEF") == 0, "PostMsg.name()");

		TestUtilities.checkResult(emaPostMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaPostMsg.publisherIdUserAddress() == 15, "PostMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaPostMsg.publisherIdUserId() == 30, "PostMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaPostMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.payload().dataType()");

		System.out.println("End EMA PostMsg Decoding");
		System.out.println();
	}

	public void testPostMsg_toString()
	{
		TestUtilities.printTestHead("testPostMsg_toString", "upa encoding ema toString");
		
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
		
	    System.out.println("Begin UPA PostMsg Set");
		com.thomsonreuters.upa.codec.PostMsg postMsg = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.POST);
		
		postMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.thomsonreuters.upa.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA PostMsg Set");
		System.out.println();

		System.out.println("Begin UPA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End UPA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg toString");

		com.thomsonreuters.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
		
		JUnitTestConnect.setRsslData(emaPostMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaPostMsg);
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("PostMsg.toString() != toString() not supported", !(emaPostMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));	 		
		

		System.out.println("End EMA PostMsg toString");
		System.out.println();
	}
	
	public void testPostMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testPostMsg_EncodeDecode", "ema encoding ema decoding");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

	    com.thomsonreuters.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.thomsonreuters.ema.access.PostMsg postMsg = EmaFactory.createPostMsg();
	    
		System.out.println("Begin EMA PostMsg test after constructor");

		TestUtilities.checkResult(postMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(postMsg.streamId() == 0, "PostMsg.streamId()");

		TestUtilities.checkResult(!postMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(!postMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!postMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(!postMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(!postMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(!postMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(!postMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(!postMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.payload().dataType()");

		System.out.println("Emd EMA PostMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA PostMsg Set");
	    
    	postMsg.domainType( com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		postMsg.streamId( 15 );
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		postMsg.partNum( 10 );
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		postMsg.seqNum( 22 );
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		postMsg.name("ABCDEF");
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		postMsg.nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		postMsg.serviceId(5);
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    			
		
		postMsg.filter( 12 );
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	
		postMsg.id(21);
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		postMsg.attrib(fl);
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	
		postMsg.publisherId(30,  15);
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		postMsg.payload(fl);
		TestUtilities.checkResult("PostMsg.toString() == toString() not supported", postMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		System.out.println("End EMA PostMsg Set");
		System.out.println();

		System.out.println("Begin EMA PostMsg Decoding");

		com.thomsonreuters.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();

		JUnitTestConnect.setRsslData(emaPostMsg, postMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaPostMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaPostMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "PostMsg.nameType()");

		TestUtilities.checkResult(emaPostMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(emaPostMsg.name().compareTo("ABCDEF") == 0, "PostMsg.name()");

		TestUtilities.checkResult(emaPostMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaPostMsg.publisherIdUserAddress() == 15, "PostMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaPostMsg.publisherIdUserId() == 30, "PostMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaPostMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "PostMsg.payload().dataType()");

		System.out.println("End EMA PostMsg Decoding");
		System.out.println();

		postMsg.clear();
		
		System.out.println("Begin EMA PostMsg test after clear");

		TestUtilities.checkResult(postMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "PostMsg.domainType()");
		
		TestUtilities.checkResult(postMsg.streamId() == 0, "PostMsg.streamId()");

		TestUtilities.checkResult(!postMsg.hasPartNum(), "PostMsg.hasPartNum()");
		
		TestUtilities.checkResult(!postMsg.hasSeqNum(), "PostMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!postMsg.hasMsgKey(), "PostMsg.hasMsgKey()");

		TestUtilities.checkResult(!postMsg.hasId(), "PostMsg.hasId()");
		
		TestUtilities.checkResult(!postMsg.hasFilter(), "PostMsg.hasFilter()");
		
		TestUtilities.checkResult(!postMsg.hasServiceId(), "PostMsg.hasServiceId()");
		
		TestUtilities.checkResult(!postMsg.hasNameType(), "PostMsg.hasNameType()");
		
		TestUtilities.checkResult(!postMsg.hasName(), "PostMsg.hasName()");
		
		TestUtilities.checkResult(postMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.attrib().dataType()");
		
		TestUtilities.checkResult(postMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "PostMsg.payload().dataType()");

		System.out.println("End EMA PostMsg test after clear");
		System.out.println();
	}
	
	public void testPostMsg_EncodeUPAPostMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPAPostMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for FieldList as attrib, extended header and payload. Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode Msg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyPostMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyPostMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestPostMsg_EncodeUPAPostMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeUPAPostMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPAPostMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for ElementList as attrib, extended header and payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyPostMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyPostMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestPostMsg_EncodeUPAPostMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeUPAPostMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPAPostMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for FilterList as attrib, payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyPostMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyPostMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestPostMsg_EncodeUPAPostMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeUPAPostMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPAPostMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for Series as attrib, payload, Encode it to another PostMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14240));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.SERIES);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyPostMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyPostMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeUPAPostMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeUPAPostMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPAPostMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for Vector as attrib, payload, Encode it to another PostMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14000));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyPostMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyPostMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeUPAPostMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for Map as attrib, payload, Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.MAP);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyPostMsg.attrib().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyPostMsg.payload().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_EncodeUPAPostMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testPostMsg_EncodeUPAPostMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode", "Encode PostMsg with UPA for RefreshMsg as attrib, payload. Encode it to another PostMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodePostMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodePostMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode PostMsg with EMA.
	     com.thomsonreuters.ema.access.PostMsg postMsg = JUnitTestConnect.createPostMsg();
	     JUnitTestConnect.setRsslData(postMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAPostMsgAll(postMsg, com.thomsonreuters.upa.codec.DataTypes.MSG);
	
	     com.thomsonreuters.ema.access.PostMsg copyPostMsg = EmaFactory.createPostMsg();
	     
	     // Encode to another PostMsg
	     copyPostMsg.extendedHeader(postMsg.extendedHeader());
	     copyPostMsg.permissionData(postMsg.permissionData());
	     copyPostMsg.attrib(postMsg.attrib().data());
	     copyPostMsg.payload(postMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.PostMsg decCopyPostMsg = JUnitTestConnect.createPostMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyPostMsg, copyPostMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyPostMsg.extendedHeader().equals(postMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyPostMsg.permissionData().equals(postMsg.permissionData()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyPostMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyPostMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestPostMsg_EncodeUPAPostMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherPostMsg_EMADecode passed");
	}
	
	public void testPostMsg_clone()
	{
		TestUtilities.printTestHead("testPostMsg_clone", "cloning for ema post message");
		
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
		
	    System.out.println("Begin UPA PostMsg Set");
		com.thomsonreuters.upa.codec.PostMsg postMsg = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.POST);
		
		postMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.thomsonreuters.upa.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA PostMsg Set");
		System.out.println();

		System.out.println("Begin UPA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End UPA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg Clone");

		com.thomsonreuters.upa.codec.PostMsg postMsgDecode = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();

		com.thomsonreuters.upa.codec.DecodeIterator decIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		postMsgDecode.decode(decIter);

		com.thomsonreuters.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
				
		JUnitTestConnect.setRsslData(emaPostMsg, postMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.thomsonreuters.ema.access.PostMsg emaPostMsgClone = EmaFactory.createPostMsg(emaPostMsg);
		
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
		
		System.out.println("End EMA PostMsg Clone");
		System.out.println();
	}
	
	public void testPostMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testPostMsg_cloneEdit", "clone and edit ema post message");
		
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
		
	    System.out.println("Begin UPA PostMsg Set");
		com.thomsonreuters.upa.codec.PostMsg postMsg = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		postMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.POST);
		
		postMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		postMsg.streamId( 15 );
		
		postMsg.applyHasPartNum();
		postMsg.partNum( 10 );
		
		postMsg.applyHasSeqNum();
		postMsg.seqNum( 22 );

		postMsg.applyHasPostId();
		postMsg.postId(223);
		
		postMsg.applyHasPostUserRights();
		postMsg.postUserRights(com.thomsonreuters.upa.codec.PostUserRights.CREATE);
		
		postMsg.applyAck();
		
		postMsg.applyHasMsgKey();

		postMsg.msgKey().applyHasName();
		postMsg.msgKey().name().data( "ABCDEF" );
		
		postMsg.msgKey().applyHasNameType();
		postMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		postMsg.msgKey().applyHasServiceId();
		postMsg.msgKey().serviceId(5);
		
		postMsg.msgKey().applyHasFilter();
		postMsg.msgKey().filter( 12 );
	
		postMsg.msgKey().applyHasIdentifier();
		postMsg.msgKey().identifier(21);
		
		postMsg.msgKey().applyHasAttrib();
		postMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		postMsg.msgKey().encodedAttrib(fieldListBuf);
	
		postMsg.postUserInfo().userAddr(15);
		postMsg.postUserInfo().userId(30);
		
		postMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		postMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA PostMsg Set");
		System.out.println();

		System.out.println("Begin UPA PostMsg Buffer Encoding");

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
		
		postMsg.encode(encIter);

	    System.out.println("End UPA PostMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA PostMsg Clone");

		com.thomsonreuters.upa.codec.PostMsg postMsgDecode = (com.thomsonreuters.upa.codec.PostMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();

		com.thomsonreuters.upa.codec.DecodeIterator decIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		postMsgDecode.decode(decIter);

		com.thomsonreuters.ema.access.PostMsg emaPostMsg = JUnitTestConnect.createPostMsg();
				
		JUnitTestConnect.setRsslData(emaPostMsg, postMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.thomsonreuters.ema.access.PostMsg emaPostMsgClone = EmaFactory.createPostMsg(emaPostMsg);
		
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
}
