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
import com.thomsonreuters.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class UpdateMsgTests extends TestCase
{

	public UpdateMsgTests(String name)
	{
		super(name);
	}

	public void testUpdateMsg_ServiceName_and_ServiceId()
	{
		TestUtilities.printTestHead("testUpdateMsg_ServiceName_and_ServiceId", "setting both serviceName and serviceId");
		
		com.thomsonreuters.ema.access.UpdateMsg emaUpdateMsg = EmaFactory.createUpdateMsg();
		
		emaUpdateMsg.serviceName("TEST");
		
		try {
			emaUpdateMsg.serviceId(5);
			TestUtilities.checkResult( false, "UpdateMsg can't set serviceId when serviceName is set - exception expected" );				
		}
		catch(Exception e)
		{
			TestUtilities.checkResult( true, "UpdateMsg can't set serviceId when serviceName is set - exception expected" );
		}		

		TestUtilities.checkResult(emaUpdateMsg.hasServiceName(), "UpdateMsg.hasServiceName()");			
		TestUtilities.checkResult(emaUpdateMsg.serviceName().equals("TEST"), "UpdateMsg.serviceName()");		
		
		int majorVersion = Codec.majorVersion();
		int minorVersion = Codec.minorVersion();		
		com.thomsonreuters.ema.access.UpdateMsg emaUpdateMsgDec = JUnitTestConnect.createUpdateMsg();
		
		JUnitTestConnect.setRsslData(emaUpdateMsgDec, emaUpdateMsg, majorVersion, minorVersion, TestUtilities.getDataDictionary(), null);
		
		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("UpdateMsg.toString() != toString() not supported", !(emaUpdateMsgDec.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));	 		

		System.out.println("End EMA ServiceName and ServiceId");
		System.out.println();
	}		
		
	public void testUpdateMsg_Decode()
	{
		TestUtilities.printTestHead("testUpdateMsg_Decode", "upa encoding ema decoding");
		
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
		
	    System.out.println("Begin UPA UpdateMsg Set");
		com.thomsonreuters.upa.codec.UpdateMsg updateMsg = (com.thomsonreuters.upa.codec.UpdateMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		updateMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.UPDATE);
		
		updateMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		updateMsg.streamId( 15 );
		
		updateMsg.applyHasSeqNum();
		updateMsg.seqNum( 22 );

		updateMsg.applyHasMsgKey();

		updateMsg.msgKey().applyHasName();
		updateMsg.msgKey().name().data( "ABCDEF" );
		
		updateMsg.msgKey().applyHasNameType();
		updateMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		updateMsg.msgKey().applyHasServiceId();
		updateMsg.msgKey().serviceId(5);
		
		updateMsg.msgKey().applyHasFilter();
		updateMsg.msgKey().filter( 12 );
	
		updateMsg.msgKey().applyHasIdentifier();
		updateMsg.msgKey().identifier(21);
		
		updateMsg.msgKey().applyHasAttrib();
		updateMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		updateMsg.msgKey().encodedAttrib(fieldListBuf);
	
		updateMsg.applyDoNotCache();
		updateMsg.applyDoNotConflate();
		
		updateMsg.applyHasConfInfo();
		updateMsg.conflationCount(10);
		updateMsg.conflationTime(20);
		
		updateMsg.applyHasPostUserInfo();
		updateMsg.postUserInfo().userAddr(15);
		updateMsg.postUserInfo().userId(30);
		
		updateMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		updateMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA UpdateMsg Set");
		System.out.println();

		System.out.println("Begin UPA UpdateMsg Buffer Encoding");

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
		
		updateMsg.encode(encIter);

	    System.out.println("End UPA UpdateMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA UpdateMsg Decoding");

		com.thomsonreuters.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
		
		JUnitTestConnect.setRsslData(emaUpdateMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaUpdateMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaUpdateMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(emaUpdateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(emaUpdateMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");

		TestUtilities.checkResult(emaUpdateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaUpdateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(emaUpdateMsg.doNotConflate(), "UpdateMsg.doNotconflate()");

		TestUtilities.checkResult(!emaUpdateMsg.doNotRipple(), "UpdateMsg.solicited()");
		
		TestUtilities.checkResult(emaUpdateMsg.hasConflated(), "UpdateMsg.hasConflated()");
	
		TestUtilities.checkResult(emaUpdateMsg.conflatedCount() == 10, "UpdateMsg.conflatedCount()");

		TestUtilities.checkResult(emaUpdateMsg.conflatedTime() == 20, "UpdateMsg.conflatedTime()");

		TestUtilities.checkResult(emaUpdateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");

		TestUtilities.checkResult(emaUpdateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");

		System.out.println("End EMA UpdateMsg Decoding");
		System.out.println();
	}
	
	public void testUpdateMsg_toString()
	{
		TestUtilities.printTestHead("testUpdateMsg_toString", "upa encoding ema toString");
		
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
		
	    System.out.println("Begin UPA UpdateMsg Set");
		com.thomsonreuters.upa.codec.UpdateMsg updateMsg = (com.thomsonreuters.upa.codec.UpdateMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		updateMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.UPDATE);
		
		updateMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
		updateMsg.streamId( 15 );
		
		updateMsg.applyHasSeqNum();
		updateMsg.seqNum( 22 );

		updateMsg.applyHasMsgKey();

		updateMsg.msgKey().applyHasName();
		updateMsg.msgKey().name().data( "ABCDEF" );
		
		updateMsg.msgKey().applyHasNameType();
		updateMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		updateMsg.msgKey().applyHasServiceId();
		updateMsg.msgKey().serviceId(5);
		
		updateMsg.msgKey().applyHasFilter();
		updateMsg.msgKey().filter( 12 );
	
		updateMsg.msgKey().applyHasIdentifier();
		updateMsg.msgKey().identifier(21);
		
		updateMsg.msgKey().applyHasAttrib();
		updateMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		updateMsg.msgKey().encodedAttrib(fieldListBuf);
	
		updateMsg.applyDoNotCache();
		updateMsg.applyDoNotConflate();
		
		updateMsg.applyHasConfInfo();
		updateMsg.conflationCount(10);
		updateMsg.conflationTime(20);
		
		updateMsg.applyHasPostUserInfo();
		updateMsg.postUserInfo().userAddr(15);
		updateMsg.postUserInfo().userId(30);
		
		updateMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		updateMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA UpdateMsg Set");
		System.out.println();

		System.out.println("Begin UPA UpdateMsg Buffer Encoding");

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
		
		updateMsg.encode(encIter);

	    System.out.println("End UPA UpdateMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA UpdateMsg toString");

		com.thomsonreuters.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();
		
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
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

	    com.thomsonreuters.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.thomsonreuters.ema.access.UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
	    
		System.out.println("Begin EMA UpdateMsg test after constructor");

		TestUtilities.checkResult(updateMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(updateMsg.streamId() == 0, "UpdateMsg.streamId()");

		TestUtilities.checkResult(!updateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!updateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(!updateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(!updateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(!updateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(!updateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(!updateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!updateMsg.doNotConflate(), "UpdateMsg.doNotConflate()");

		TestUtilities.checkResult(!updateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(!updateMsg.doNotRipple(), "UpdateMsg.doNotRipple()");

		TestUtilities.checkResult(!updateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.payload().dataType()");

		System.out.println("Begin EMA UpdateMsg test after constructor");
		System.out.println();		
		
	    System.out.println("End EMA UpdateMsg Set");
	    
    	updateMsg.domainType( com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.streamId( 15 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.seqNum( 22 );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		updateMsg.name("ABCDEF");
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		updateMsg.nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		updateMsg.serviceId(5);
		TestUtilities.checkResult("UpdateMsg.toString() == toString() not supported", updateMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
		try {
			updateMsg.serviceName("TEST");
			TestUtilities.checkResult( false, "UpdateMsg can't set serviceName when serviceId is set - exception expected" );				
		}
		catch(Exception e)
		{
			TestUtilities.checkResult( true, "UpdateMsg can't set serviceName when serviceId is set - exception expected" );			
		}			
		
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

		com.thomsonreuters.ema.access.UpdateMsg emaUpdateMsg = JUnitTestConnect.createUpdateMsg();

		JUnitTestConnect.setRsslData(emaUpdateMsg, updateMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaUpdateMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaUpdateMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "UpdateMsg.nameType()");

		TestUtilities.checkResult(emaUpdateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(emaUpdateMsg.name().compareTo("ABCDEF") == 0, "UpdateMsg.name()");

		TestUtilities.checkResult(emaUpdateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaUpdateMsg.doNotConflate(), "UpdateMsg.doNotConflate()");

		TestUtilities.checkResult(emaUpdateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(emaUpdateMsg.doNotRipple(), "UpdateMsg.doNotRipple()");

		TestUtilities.checkResult(emaUpdateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserAddress() == 15, "UpdateMsg.publisherIdUserAddress()");

		TestUtilities.checkResult(emaUpdateMsg.publisherIdUserId() == 30, "UpdateMsg.publisherIdUserId()");
		
		TestUtilities.checkResult(emaUpdateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "UpdateMsg.payload().dataType()");

		System.out.println("End EMA UpdateMsg Decoding");
		System.out.println();

		updateMsg.clear();
		
		System.out.println("Begin EMA UpdateMsg test after clear");

		TestUtilities.checkResult(updateMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "UpdateMsg.domainType()");
		
		TestUtilities.checkResult(updateMsg.streamId() == 0, "UpdateMsg.streamId()");

		TestUtilities.checkResult(!updateMsg.hasSeqNum(), "UpdateMsg.hasSeqNum()");
		
		TestUtilities.checkResult(!updateMsg.hasMsgKey(), "UpdateMsg.hasMsgKey()");

		TestUtilities.checkResult(!updateMsg.hasId(), "UpdateMsg.hasId()");
		
		TestUtilities.checkResult(!updateMsg.hasFilter(), "UpdateMsg.hasFilter()");
		
		TestUtilities.checkResult(!updateMsg.hasServiceId(), "UpdateMsg.hasServiceId()");
		
		TestUtilities.checkResult(!updateMsg.hasNameType(), "UpdateMsg.hasNameType()");
		
		TestUtilities.checkResult(!updateMsg.hasName(), "UpdateMsg.hasName()");
		
		TestUtilities.checkResult(updateMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.attrib().dataType()");
		
		TestUtilities.checkResult(!updateMsg.doNotConflate(), "UpdateMsg.doNotConflate()");

		TestUtilities.checkResult(!updateMsg.doNotCache(), "UpdateMsg.doNotCache()");

		TestUtilities.checkResult(!updateMsg.doNotRipple(), "UpdateMsg.doNotRipple()");

		TestUtilities.checkResult(!updateMsg.hasPublisherId(), "UpdateMsg.hasPublisherId()");
		
		TestUtilities.checkResult(updateMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "UpdateMsg.payload().dataType()");

		System.out.println("End EMA UpdateMsg test after clear");
		System.out.println();
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for FieldList as attrib, extended header and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyUpdateMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyUpdateMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestUpdateMsg_EncodeUPAUpdateWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for ElementList as attrib, extended header and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyUpdateMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyUpdateMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestUpdateMsg_EncodeUPAUpdateWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for FilterList as attrib and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyUpdateMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyUpdateMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestUpdateMsg_EncodeUPAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for Series as attrib and payload, Encode it to another UpdateMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(12040));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.SERIES);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyUpdateMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyUpdateMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestUpdateMsg_EncodeUPAUpdateWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for Vector as attrib and payload, Encode it to another UpdateMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(12040));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyUpdateMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyUpdateMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\testUpdateMsg_EncodeUPAUpdateWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for Map as attrib and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.MAP);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyUpdateMsg.attrib().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyUpdateMsg.payload().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestUpdateMsg_EncodeUPAUpdateWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testUpdateMsg_EncodeUPAUpdateWithRefreshMsgTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testUpdateMsg_EncodeUPAUpdateWithRefreshMsgTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode", "Encode UpdateMsg with UPA for RefreshMsg as attrib and payload, Encode it to another UpdateMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeUpdateMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeUpdateMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode UpdateMsg with EMA.
	     com.thomsonreuters.ema.access.UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
	     JUnitTestConnect.setRsslData(updateMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAUpdateMsgAll(updateMsg, com.thomsonreuters.upa.codec.DataTypes.MSG);
	
	     com.thomsonreuters.ema.access.UpdateMsg copyUpdateMsg = EmaFactory.createUpdateMsg();
	     
	     // Encode to another UpdateMsg
	     copyUpdateMsg.extendedHeader(updateMsg.extendedHeader());
	     copyUpdateMsg.permissionData(updateMsg.permissionData());
	     copyUpdateMsg.attrib(updateMsg.attrib().data());
	     copyUpdateMsg.payload(updateMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.UpdateMsg decCopyUpdateMsg = JUnitTestConnect.createUpdateMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyUpdateMsg, copyUpdateMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyUpdateMsg.extendedHeader().equals(updateMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyUpdateMsg.permissionData().equals(updateMsg.permissionData()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyUpdateMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyUpdateMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestUpdateMsg_EncodeUPAUpdateWithRefreshMsgTypeAsAttrib_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	
}
