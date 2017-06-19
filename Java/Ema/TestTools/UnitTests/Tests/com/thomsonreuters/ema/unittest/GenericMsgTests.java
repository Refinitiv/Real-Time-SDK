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
import com.thomsonreuters.upa.rdm.DomainTypes;

import junit.framework.TestCase;

public class GenericMsgTests extends TestCase
{
	public GenericMsgTests(String name)
	{
		super(name);
	}
	
	public void testGenericMsg_Decode()
	{
		TestUtilities.printTestHead("testGenericMsg_Decode", "upa encoding ema decoding");
		
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
		
	    System.out.println("Begin UPA GenericMsg Set");
		com.thomsonreuters.upa.codec.GenericMsg genericMsg = (com.thomsonreuters.upa.codec.GenericMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
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
		genericMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		genericMsg.msgKey().encodedAttrib(fieldListBuf);
	
		genericMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		genericMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA GenericMsg Set");
		System.out.println();

		System.out.println("Begin UPA GenericMsg Buffer Encoding");

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
		
		genericMsg.encode(encIter);

	    System.out.println("End UPA GenericMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA GenericMsg Decoding");

		com.thomsonreuters.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();
		
		JUnitTestConnect.setRsslData(emaGenericMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		TestUtilities.checkResult(emaGenericMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "GenericMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaGenericMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "GenericMsg.nameType()");

		TestUtilities.checkResult(emaGenericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(emaGenericMsg.name().compareTo("ABCDEF") == 0, "GenericMsg.name()");

		TestUtilities.checkResult(emaGenericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaGenericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg Decoding");
		System.out.println();
	}

	public void testGenericMsg_toString()
	{
		TestUtilities.printTestHead("testGenericMsg_toString", "upa encoding ema toString");
		
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
		
	    System.out.println("Begin UPA GenericMsg Set");
		com.thomsonreuters.upa.codec.GenericMsg genericMsg = (com.thomsonreuters.upa.codec.GenericMsg)com.thomsonreuters.upa.codec.CodecFactory.createMsg();
		genericMsg.msgClass(com.thomsonreuters.upa.codec.MsgClasses.GENERIC);
		
		genericMsg.domainType( com.thomsonreuters.upa.rdm.DomainTypes.MARKET_PRICE );
		
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
		genericMsg.msgKey().nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		genericMsg.msgKey().applyHasServiceId();
		genericMsg.msgKey().serviceId(5);
		
		genericMsg.msgKey().applyHasFilter();
		genericMsg.msgKey().filter( 12 );
	
		genericMsg.msgKey().applyHasIdentifier();
		genericMsg.msgKey().identifier(21);
		
		genericMsg.msgKey().applyHasAttrib();
		genericMsg.msgKey().attribContainerType( com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST );
		genericMsg.msgKey().encodedAttrib(fieldListBuf);
	
		genericMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
		genericMsg.encodedDataBody(fieldListBuf);

		System.out.println("End UPA GenericMsg Set");
		System.out.println();

		System.out.println("Begin UPA GenericMsg Buffer Encoding");

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
		
		genericMsg.encode(encIter);

	    System.out.println("End UPA GenericMsg Buffer Encoding");
		System.out.println();

		System.out.println("Begin EMA GenericMsg toString");

		com.thomsonreuters.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();
		
		JUnitTestConnect.setRsslData(emaGenericMsg, msgBuf, majorVersion, minorVersion, dictionary, null);

		System.out.println(emaGenericMsg);
		
		System.out.println("End EMA GenericMsg toString");
		System.out.println();
	}
	
	public void testGenericMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testGenericMsg_EncodeDecode", "ema encoding ema decoding");
		
		com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

	    com.thomsonreuters.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.thomsonreuters.ema.access.GenericMsg genericMsg = EmaFactory.createGenericMsg();
	    
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
		
		TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA GenericMsg Set");
	    
    	genericMsg.domainType( com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		
		genericMsg.streamId( 15 );
		
		genericMsg.partNum( 10 );
		
		genericMsg.seqNum( 22 );

		genericMsg.secondarySeqNum(123);
		
		genericMsg.complete(true);
		
		genericMsg.name("ABCDEF");
		
		genericMsg.nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

		genericMsg.serviceId(5);
		
		genericMsg.filter( 12 );
	
		genericMsg.id(21);
		
		genericMsg.attrib(fl);
	
		genericMsg.payload(fl);

		System.out.println("End EMA GenericMsg Set");
		System.out.println();

		System.out.println("Begin EMA GenericMsg Decoding");

		com.thomsonreuters.ema.access.GenericMsg emaGenericMsg = JUnitTestConnect.createGenericMsg();

		JUnitTestConnect.setRsslData(emaGenericMsg, genericMsg, 14, 0, dictionary, null);

		TestUtilities.checkResult(emaGenericMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "GenericMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaGenericMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "GenericMsg.nameType()");

		TestUtilities.checkResult(emaGenericMsg.hasName(), "GenericMsg.hasName()");
		
		TestUtilities.checkResult(emaGenericMsg.name().compareTo("ABCDEF") == 0, "GenericMsg.name()");

		TestUtilities.checkResult(emaGenericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaGenericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "GenericMsg.payload().dataType()");

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
		
		TestUtilities.checkResult(genericMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.attrib().dataType()");
		
		TestUtilities.checkResult(genericMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "GenericMsg.payload().dataType()");

		System.out.println("End EMA GenericMsg test after clear");
		System.out.println();
	}
	
	public void testGenericMsg_EncodeUPAGenericMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPAGenericMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for FieldList as attrib, extended header and payload. Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode Msg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyGenericMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyGenericMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPAGenericMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherUpdateMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeUPAGenericMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPAGenericMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for ElementList as attrib, extended header and payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyGenericMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyGenericMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPAGenericMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeUPAGenericMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPAGenericMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for FilterList as attrib, payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyGenericMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyGenericMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPAGenericMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeUPAGenericMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPAGenericMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for Series as attrib, payload, Encode it to another GenericMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.SERIES);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyGenericMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyGenericMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPAGenericMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeUPAGenericMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPAGenericMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for Vector as attrib, payload, Encode it to another GenericMsg.");

        // Create a UPA Buffer to encode into
        com.thomsonreuters.upa.codec.Buffer buffer = com.thomsonreuters.upa.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(10240));
        
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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyGenericMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyGenericMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPAGenericMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for Map as attrib, payload, Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.MAP);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyGenericMsg.attrib().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyGenericMsg.payload().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
	
	public void testGenericMsg_EncodeUPAGenericMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testGenericMsg_EncodeUPAGenericMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode", "Encode GenericMsg with UPA for RefreshMsg as attrib, payload. Encode it to another GenericMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeGenericMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeGenericMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode GenericMsg with EMA.
	     com.thomsonreuters.ema.access.GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
	     JUnitTestConnect.setRsslData(genericMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPAGenericMsgAll(genericMsg, com.thomsonreuters.upa.codec.DataTypes.MSG);
	
	     com.thomsonreuters.ema.access.GenericMsg copyGenericMsg = EmaFactory.createGenericMsg();
	     
	     // Encode to another GenericMsg
	     copyGenericMsg.extendedHeader(genericMsg.extendedHeader());
	     copyGenericMsg.permissionData(genericMsg.permissionData());
	     copyGenericMsg.attrib(genericMsg.attrib().data());
	     copyGenericMsg.payload(genericMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.GenericMsg decCopyGenericMsg = JUnitTestConnect.createGenericMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyGenericMsg, copyGenericMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyGenericMsg.extendedHeader().equals(genericMsg.extendedHeader()));
	     TestUtilities.checkResult(decCopyGenericMsg.permissionData().equals(genericMsg.permissionData()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyGenericMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyGenericMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestGenericMsg_EncodeUPAGenericMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherGenericMsg_EMADecode passed");
	}
}
