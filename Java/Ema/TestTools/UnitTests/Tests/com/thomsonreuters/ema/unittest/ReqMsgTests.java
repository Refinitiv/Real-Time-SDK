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
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

import junit.framework.TestCase;

public class ReqMsgTests extends TestCase 
{
	public ReqMsgTests(String name)
	{
		super(name);
	}
	
	public void testReqMsg_EncodeDecode()
	{
		TestUtilities.printTestHead("testRequestMsg_EncodeDecode", "ema encoding ema decoding");

	    com.thomsonreuters.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.thomsonreuters.ema.access.ReqMsg reqMsg = EmaFactory.createReqMsg();
	    
		System.out.println("Begin EMA ReqMsg test after constructor");

		TestUtilities.checkResult(reqMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
		TestUtilities.checkResult(reqMsg.streamId() == 0, "ReqMsg.streamId()");
		
		TestUtilities.checkResult(!reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.hasMsgKey(), "ReqMsg.hasMsgKey()");

		TestUtilities.checkResult(!reqMsg.hasId(), "ReqMsg.hasId()");
		
		TestUtilities.checkResult(!reqMsg.hasFilter(), "ReqMsg.hasFilter()");
		
		TestUtilities.checkResult(!reqMsg.hasServiceId(), "ReqMsg.hasServiceId()");
		
		TestUtilities.checkResult(!reqMsg.hasServiceName(), "ReqMsg.hasServiceName()");
		
		TestUtilities.checkResult(!reqMsg.hasNameType(), "ReqMsg.hasNameType()");
		
		TestUtilities.checkResult(!reqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(!reqMsg.hasPriority(), "ReqMsg.hasPriority()");
		
		TestUtilities.checkResult(!reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA ReqMsg Set");
	    
	    reqMsg.domainType( com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
	    
	    reqMsg.initialImage(true);
	    
	    reqMsg.interestAfterRefresh(true);
		
	    reqMsg.streamId( 15 );
		
	    reqMsg.qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK);

	    reqMsg.name("ABCDEF");
		
	    reqMsg.nameType( com.thomsonreuters.upa.rdm.InstrumentNameTypes.RIC );

	    reqMsg.serviceId(5);
		
	    reqMsg.filter( 12 );
	
	    reqMsg.id(21);
	    
	    reqMsg.priority(5, 7);
	    
	    reqMsg.qos(com.thomsonreuters.ema.access.ReqMsg.Timeliness.REALTIME, com.thomsonreuters.ema.access.ReqMsg.Rate.TICK_BY_TICK);
	    
	    reqMsg.conflatedInUpdates(true);
		
	    reqMsg.attrib(fl);
		
	    reqMsg.payload(fl);

		System.out.println("End EMA ReqMsg Set");
		System.out.println();

		System.out.println("Begin EMA ReqMsg Decoding");

		com.thomsonreuters.ema.access.ReqMsg emaReqMsg = JUnitTestConnect.createReqMsg();

		JUnitTestConnect.setRsslData(emaReqMsg, reqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);

		TestUtilities.checkResult(emaReqMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
		TestUtilities.checkResult(emaReqMsg.initialImage() == true, "ReqMsg.initialImage()");
		
		TestUtilities.checkResult(emaReqMsg.interestAfterRefresh() == true, "ReqMsg.interestAfterRefresh()");
		
		TestUtilities.checkResult(emaReqMsg.streamId() == 15, "ReqMsg.streamId()");

		TestUtilities.checkResult(emaReqMsg.hasQos(), "ReqMsg.hasQos()");
	
		TestUtilities.checkResult(emaReqMsg.hasMsgKey(), "ReqMsg.hasMsgKey()");

		TestUtilities.checkResult(emaReqMsg.hasId(), "ReqMsg.hasId()");
		
		TestUtilities.checkResult(emaReqMsg.id() == 21, "ReqMsg.id()");

		TestUtilities.checkResult(emaReqMsg.hasFilter(), "ReqMsg.hasFilter()");
		
		TestUtilities.checkResult(emaReqMsg.filter() == 12 , "ReqMsg.hasFilter()");

		TestUtilities.checkResult(!emaReqMsg.hasServiceName(), "ReqMsg.hasServiceName()");
		
		TestUtilities.checkResult(emaReqMsg.serviceId() == 5 , "ReqMsg.serviceId()");

		TestUtilities.checkResult(emaReqMsg.hasNameType(), "ReqMsg.hasNameType()");
		
		TestUtilities.checkResult(emaReqMsg.nameType() == com.thomsonreuters.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "ReqMsg.nameType()");

		TestUtilities.checkResult(emaReqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(emaReqMsg.name().compareTo("ABCDEF") == 0, "ReqMsg.name()");
		
		TestUtilities.checkResult(reqMsg.hasPriority(), "ReqMsg.hasPriority()");
		
		TestUtilities.checkResult(emaReqMsg.priorityClass() == 5, "ReqMsg.priorityClass()");
		
		TestUtilities.checkResult(emaReqMsg.priorityCount() == 7, "ReqMsg.priorityCount()");
		
		TestUtilities.checkResult(reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.qosRate() ==  com.thomsonreuters.ema.access.ReqMsg.Rate.TICK_BY_TICK, "ReqMsg.qosRate()");
		
		TestUtilities.checkResult(reqMsg.qosTimeliness() ==  com.thomsonreuters.ema.access.ReqMsg.Timeliness.REALTIME, "ReqMsg.qosTimeliness()");
		
		TestUtilities.checkResult(reqMsg.conflatedInUpdates() ==  true, "ReqMsg.conflatedInUpdates()");

		TestUtilities.checkResult(emaReqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaReqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg Decoding");
		System.out.println();

		reqMsg.clear();
		
		System.out.println("Begin EMA ReqMsg test after clear");

		TestUtilities.checkResult(reqMsg.domainType() == com.thomsonreuters.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
		TestUtilities.checkResult(reqMsg.streamId() == 0, "ReqMsg.streamId()");
		
		TestUtilities.checkResult(!reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.hasMsgKey(), "ReqMsg.hasMsgKey()");

		TestUtilities.checkResult(!reqMsg.hasId(), "ReqMsg.hasId()");
		
		TestUtilities.checkResult(!reqMsg.hasFilter(), "ReqMsg.hasFilter()");
		
		TestUtilities.checkResult(!reqMsg.hasServiceId(), "ReqMsg.hasServiceId()");
		
		TestUtilities.checkResult(!reqMsg.hasServiceName(), "ReqMsg.hasServiceName()");
		
		TestUtilities.checkResult(!reqMsg.hasNameType(), "ReqMsg.hasNameType()");
		
		TestUtilities.checkResult(!reqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(!reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.attrib().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(reqMsg.payload().dataType() == com.thomsonreuters.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg test after clear");
		System.out.println();
	}
	
	public void testReqMsg_EncodeUPARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for FieldList as attrib, extended header and payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyReqMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAFieldListAll(decCopyReqMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestReqMsg_EncodeUPARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeUPAReqMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPAReqMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for ElementList as attrib, extended header and payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyReqMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_UPAElementListAll(decCopyReqMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestReqMsg_EncodeUPAReqMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeUPAReqMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPAReqMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for FilterList as attrib, payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyReqMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_UPAFilterListAll(decCopyReqMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestReqMsg_EncodeUPAReqMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeUPAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for Series as attrib, payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.SERIES);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyReqMsg.attrib().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPASeriesAll(decCopyReqMsg.payload().series(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeUPAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeUPAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for Vector as attrib, payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.VECTOR);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyReqMsg.attrib().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_UPAVectorAll(decCopyReqMsg.payload().vector(), com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeUPAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for Map as attrib, payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.MAP);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyReqMsg.attrib().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPAMapKeyUIntAll(decCopyReqMsg.payload().map(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeUPARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeUPAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeUPAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with UPA for RefreshMsg as attrib, payload. Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.upa_EncodeRequestMsgAll(encodeIter, com.thomsonreuters.upa.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.upa_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.thomsonreuters.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_UPARequestMsgAll(reqMsg, com.thomsonreuters.upa.codec.DataTypes.MSG);
	
	     com.thomsonreuters.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.thomsonreuters.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyReqMsg.attrib().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_UPARefreshMsgAll(decCopyReqMsg.payload().refreshMsg(), com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeUPAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
}
