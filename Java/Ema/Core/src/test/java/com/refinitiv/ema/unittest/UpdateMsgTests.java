///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Iterator;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class UpdateMsgTests extends TestCase
{

	public UpdateMsgTests(String name)
	{
		super(name);
	}

	public void testUpdateMsg_Decode()
	{
		TestUtilities.printTestHead("testUpdateMsg_Decode", "eta encoding ema decoding");
		
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
			return;
		}
		
		updateMsg.encode(encIter);

	    System.out.println("End ETA UpdateMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
		
		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

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
			return;
		}
		
		updateMsg.encode(encIter);

	    System.out.println("End ETA UpdateMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA UpdateMsg toString");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
		
		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaUpdateMsg);

		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("UpdateMsg.toString() != toString() not supported", !(emaUpdateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));	 		

		System.out.println("End EMA UpdateMsg toString");
		System.out.println();
	}
	
	public void testUpdateMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testUpdateMsg_EncodeDecode", "ema encoding ema decoding");
		
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
	    
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
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.streamId( 15 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.seqNum( 22 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		updateMsg.name("ABCDEF");
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		updateMsg.serviceId(5);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.filter( 12 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	
		updateMsg.id(21);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.attrib(fl);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	
	    updateMsg.doNotCache(true);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		updateMsg.doNotConflate(true);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		updateMsg.doNotRipple(true);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.publisherId(30,  15);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.payload(fl);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		System.out.println("End EMA UpdateMsg Set");
		System.out.println();

		System.out.println("Begin EMA UpdateMsg Decoding");

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsg, 14, 0, dictionary, null);

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

		setMoreFields(updateMsg);

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
			return;
		}
		
		updateMsg.encode(encIter);
		
		System.out.println("End ETA UpdateMsg Buffer Encoding");
		System.out.println();
		
		System.out.println("Begin EMA UpdateMsg Clone");
		com.refinitiv.eta.codec.UpdateMsg updateMsgDecode = (com.refinitiv.eta.codec.UpdateMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		updateMsgDecode.decode(decIter);

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
				
		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsgDecode, majorVersion, minorVersion, dictionary, null);
		
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

	private void setMoreFields(com.refinitiv.eta.codec.UpdateMsg updateMsg) {
		updateMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		updateMsg.extendedHeader(extendedHeader);

		updateMsg.applyHasPermData();
		Buffer permissionData = CodecFactory.createBuffer();
		permissionData.data(ByteBuffer.wrap(new byte[]{50, 51, 52, 53}));
		updateMsg.permData(permissionData);
	}

	public void testUpdateMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testUpdateMsg_cloneEdit", "clone and edit ema update message");
		
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
			return;
		}
		
		updateMsg.encode(encIter);
		
		System.out.println("End ETA UpdateMsg Buffer Encoding");
		System.out.println();
		
		System.out.println("Begin EMA UpdateMsg Clone");
		com.refinitiv.eta.codec.UpdateMsg updateMsgDecode = (com.refinitiv.eta.codec.UpdateMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		updateMsgDecode.decode(decIter);

		com.refinitiv.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
				
		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsgDecode, majorVersion, minorVersion, dictionary, null);
		
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
}
