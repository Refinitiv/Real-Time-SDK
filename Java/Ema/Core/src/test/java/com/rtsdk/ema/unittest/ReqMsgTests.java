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
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.rdm.EmaRdm;
import com.rtsdk.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.rtsdk.eta.codec.*;

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

	    com.rtsdk.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.rtsdk.ema.access.ReqMsg reqMsg = EmaFactory.createReqMsg();
	    
		System.out.println("Begin EMA ReqMsg test after constructor");

		TestUtilities.checkResult(reqMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
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
		
		TestUtilities.checkResult(reqMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(reqMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA ReqMsg Set");
	    
	    reqMsg.domainType( com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
	    reqMsg.initialImage(true);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
	    reqMsg.interestAfterRefresh(true);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
	    reqMsg.streamId( 15 );
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
	    reqMsg.qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

	    reqMsg.name("ABCDEF");
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
	    reqMsg.nameType( com.rtsdk.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

	    reqMsg.serviceId(5);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

	    reqMsg.filter( 12 );
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	
	    reqMsg.id(21);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
	    reqMsg.priority(5, 7);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
	    reqMsg.qos(com.rtsdk.ema.access.ReqMsg.Timeliness.REALTIME, com.rtsdk.ema.access.ReqMsg.Rate.TICK_BY_TICK);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
	    
	    reqMsg.conflatedInUpdates(true);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
	    reqMsg.attrib(fl);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    
		
	    reqMsg.payload(fl);
		TestUtilities.checkResult("ReqMsg.toString() == toString() not supported", reqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));	    

		System.out.println("End EMA ReqMsg Set");
		System.out.println();

		System.out.println("Begin EMA ReqMsg Decoding");

		com.rtsdk.ema.access.ReqMsg emaReqMsg = JUnitTestConnect.createReqMsg();

		JUnitTestConnect.setRsslData(emaReqMsg, reqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);

		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("ReqMsg.toString() != toString() not supported", !(emaReqMsg.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));	 		
	
		TestUtilities.checkResult(emaReqMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaReqMsg.nameType() == com.rtsdk.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "ReqMsg.nameType()");

		TestUtilities.checkResult(emaReqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(emaReqMsg.name().compareTo("ABCDEF") == 0, "ReqMsg.name()");
		
		TestUtilities.checkResult(reqMsg.hasPriority(), "ReqMsg.hasPriority()");
		
		TestUtilities.checkResult(emaReqMsg.priorityClass() == 5, "ReqMsg.priorityClass()");
		
		TestUtilities.checkResult(emaReqMsg.priorityCount() == 7, "ReqMsg.priorityCount()");
		
		TestUtilities.checkResult(reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.qosRate() ==  com.rtsdk.ema.access.ReqMsg.Rate.TICK_BY_TICK, "ReqMsg.qosRate()");
		
		TestUtilities.checkResult(reqMsg.qosTimeliness() ==  com.rtsdk.ema.access.ReqMsg.Timeliness.REALTIME, "ReqMsg.qosTimeliness()");
		
		TestUtilities.checkResult(reqMsg.conflatedInUpdates() ==  true, "ReqMsg.conflatedInUpdates()");

		TestUtilities.checkResult(emaReqMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaReqMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg Decoding");
		System.out.println();

		reqMsg.clear();
		
		System.out.println("Begin EMA ReqMsg test after clear");

		TestUtilities.checkResult(reqMsg.domainType() == com.rtsdk.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
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
		
		TestUtilities.checkResult(reqMsg.attrib().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(reqMsg.payload().dataType() == com.rtsdk.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg test after clear");
		System.out.println();
	}
	
	public void testReqMsg_EncodeETARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for FieldList as attrib, extended header and payload, Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyReqMsg.attrib().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAFieldListAll(decCopyReqMsg.payload().fieldList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestReqMsg_EncodeETARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for ElementList as attrib, extended header and payload, Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyReqMsg.attrib().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     TestUtilities.EmaDecode_ETAElementListAll(decCopyReqMsg.payload().elementList(), TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithElementListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for FilterList as attrib, payload, Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.FILTER_LIST);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyReqMsg.attrib().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     TestUtilities.EmaDecode_ETAFilterListAll(decCopyReqMsg.payload().filterList(), TestUtilities.EncodingTypeFlags.MESSAGE_TYPES);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithFilterListTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for Series as attrib, payload, Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.SERIES);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyReqMsg.attrib().series(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyReqMsg.payload().series(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for Vector as attrib, payload, Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buffer = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(14000));
        
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.VECTOR);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyReqMsg.attrib().vector(), com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyReqMsg.payload().vector(), com.rtsdk.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for Map as attrib, payload, Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.MAP);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyReqMsg.attrib().map(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyReqMsg.payload().map(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for RefreshMsg as attrib, payload. Encode it to another ReqMsg.");

        // Create a ETA Buffer to encode into
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.rtsdk.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.rtsdk.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.rtsdk.eta.codec.DataTypes.MSG);
	
	     com.rtsdk.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.rtsdk.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyReqMsg.attrib().refreshMsg(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyReqMsg.payload().refreshMsg(), com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}

	public void testRequestMsg_cloneIsNotSupportedFromTheEncodeSide()
	{
		TestUtilities.printTestHead("testRequestMsg_cloneIsNotSupportedFromTheEncodeSide", "cloning is not supported on encode side");
		ReqMsg msg = EmaFactory.createReqMsg()
				.domainType(EmaRdm.MMT_MARKET_PRICE);

		try {
			ReqMsg cloneMessage = EmaFactory.createReqMsg(msg);
			TestUtilities.checkResult(false, "Clone not supported  - exception expected: ");
		} catch ( OmmException excp ) {
			TestUtilities.checkResult(true, "Clone not supported  - exception expected: " +  excp.getMessage() );
			TestUtilities.checkResult(excp.getMessage().startsWith("Failed to clone empty encoded buffer"), "Clone not supported - exception text validated");
		}
	}

	public void testReqMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testReqMsg_cloneMsgKeyWLScenario", "cloning for minimal ema req message");
		ReqMsg emaMsg = EmaFactory.createReqMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.rtsdk.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.rtsdk.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, true);
		ReqMsg emaClonedMsg = EmaFactory.createReqMsg(emaMsg);

		compareEmaReqMsgFields(emaMsg, emaClonedMsg, "check clone for minimal message");
		JUnitTestConnect.setRsslMsgKeyFlag(emaMsg, false);

		System.out.println("End EMA ReqMsg Clone msgKey");
		System.out.println();
	}

	public void testRequestMsg_clone()
	{
		TestUtilities.printTestHead("testRequestMsg_clone", "cloning for ema request message");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
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
		
	    System.out.println("Begin ETA RequestMsg Set");
		com.rtsdk.eta.codec.RequestMsg requestMsg = (com.rtsdk.eta.codec.RequestMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		requestMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.REQUEST);
		
		requestMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		requestMsg.streamId( 15 );

		requestMsg.applyHasPriority();
		requestMsg.priority().priorityClass(5);
		requestMsg.priority().count(7);
			
		requestMsg.applyHasQos();
		requestMsg.qos().timeliness(QosTimeliness.REALTIME);
		requestMsg.qos().rate(QosRates.TICK_BY_TICK);

		requestMsg.msgKey().applyHasServiceId();
		requestMsg.msgKey().serviceId(5);
		
		requestMsg.msgKey().applyHasName();
		requestMsg.msgKey().name().data("ABCDEF");
		
		requestMsg.msgKey().filter(12);
		
		requestMsg.msgKey().identifier(21);
		
		requestMsg.msgKey().applyHasAttrib();
		requestMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		requestMsg.msgKey().encodedAttrib(fieldListBuf);
		
		requestMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		requestMsg.encodedDataBody(fieldListBuf);

		setMoreFields(requestMsg);

		System.out.println("End ETA RequestMsg Set");
		System.out.println();

		System.out.println("Begin ETA RequestMsg Buffer Encoding");

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
		
		requestMsg.encode(encIter);
		
		System.out.println("End ETA RequestMsg Buffer Encoding");
		System.out.println();
		
		System.out.println("Begin EMA RequestMsg Clone");
		com.rtsdk.eta.codec.RequestMsg requestMsgDecode = (com.rtsdk.eta.codec.RequestMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();

		com.rtsdk.eta.codec.DecodeIterator decIter = com.rtsdk.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		requestMsgDecode.decode(decIter);

		com.rtsdk.ema.access.ReqMsg emaRequestMsg = JUnitTestConnect.createReqMsg();
				
		JUnitTestConnect.setRsslData(emaRequestMsg, requestMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.rtsdk.ema.access.ReqMsg emaRequestMsgClone = EmaFactory.createReqMsg(emaRequestMsg);
		
		compareEmaReqMsgFields(emaRequestMsg, emaRequestMsgClone, "Request clone message");

		String emaRequestMsgString = emaRequestMsg.toString();
		String emaRequestMsgCloneString = emaRequestMsgClone.toString();
		
		System.out.println("Cloned EMA RequestMsg:");
		System.out.println(emaRequestMsgClone);
		
		TestUtilities.checkResult(emaRequestMsgString.equals(emaRequestMsgCloneString), "emaRequestMsgString.equals(emaRequestMsgCloneString)");

		com.rtsdk.ema.access.ReqMsg emaRequestMsgClone2 = EmaFactory.createReqMsg(emaRequestMsgClone);
		compareEmaReqMsgFields(emaRequestMsg, emaRequestMsgClone2, "Request double-cloned message");
		String emaRequestMsgClone2String = emaRequestMsgClone2.toString();
		TestUtilities.checkResult(emaRequestMsgString.equals(emaRequestMsgClone2String), "double-cloned emaRequestMsgString.equals(emaRequestMsgClone2String)");

		System.out.println("End EMA RequestMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.rtsdk.eta.codec.RequestMsg requestMsg) {
		requestMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		requestMsg.extendedHeader(extendedHeader);

		requestMsg.applyPrivateStream();
	}

	public void testRequestMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testRequestMsg_cloneEdit", "clone and edit ema request message");
		
		com.rtsdk.eta.codec.Buffer fieldListBuf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		fieldListBuf.data(ByteBuffer.allocate(1024));

		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory.createDataDictionary();
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
		
	    System.out.println("Begin ETA RequestMsg Set");
		com.rtsdk.eta.codec.RequestMsg requestMsg = (com.rtsdk.eta.codec.RequestMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();
		requestMsg.msgClass(com.rtsdk.eta.codec.MsgClasses.REQUEST);
		
		requestMsg.domainType( com.rtsdk.eta.rdm.DomainTypes.MARKET_PRICE );
		
		requestMsg.streamId( 15 );

		requestMsg.applyHasPriority();
		requestMsg.priority().priorityClass(5);
		requestMsg.priority().count(7);
			
		requestMsg.applyHasQos();
		requestMsg.qos().timeliness(QosTimeliness.REALTIME);
		requestMsg.qos().rate(QosRates.TICK_BY_TICK);

		requestMsg.msgKey().applyHasServiceId();
		requestMsg.msgKey().serviceId(5);
		
		requestMsg.msgKey().applyHasName();
		requestMsg.msgKey().name().data("ABCDEF");
		
		requestMsg.msgKey().filter(12);
		
		requestMsg.msgKey().identifier(21);
		
		requestMsg.msgKey().applyHasAttrib();
		requestMsg.msgKey().attribContainerType( com.rtsdk.eta.codec.DataTypes.FIELD_LIST );
		requestMsg.msgKey().encodedAttrib(fieldListBuf);
		
		requestMsg.containerType(com.rtsdk.eta.codec.DataTypes.FIELD_LIST);
		requestMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA RequestMsg Set");
		System.out.println();

		System.out.println("Begin ETA RequestMsg Buffer Encoding");

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
		
		requestMsg.encode(encIter);
		
		System.out.println("End ETA RequestMsg Buffer Encoding");
		System.out.println();
		
		System.out.println("Begin EMA RequestMsg Clone");
		com.rtsdk.eta.codec.RequestMsg requestMsgDecode = (com.rtsdk.eta.codec.RequestMsg)com.rtsdk.eta.codec.CodecFactory.createMsg();

		com.rtsdk.eta.codec.DecodeIterator decIter = com.rtsdk.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		requestMsgDecode.decode(decIter);

		com.rtsdk.ema.access.ReqMsg emaRequestMsg = JUnitTestConnect.createReqMsg();
				
		JUnitTestConnect.setRsslData(emaRequestMsg, requestMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.rtsdk.ema.access.ReqMsg emaRequestMsgClone = EmaFactory.createReqMsg(emaRequestMsg);
		
		TestUtilities.checkResult(emaRequestMsgClone.domainType() == emaRequestMsg.domainType(), "Compare domainType");
		TestUtilities.checkResult(emaRequestMsgClone.streamId() == emaRequestMsg.streamId(), "Compare streamId");
		TestUtilities.checkResult(emaRequestMsgClone.hasPriority() == emaRequestMsg.hasMsgKey(), "Compare hasPriority");
		TestUtilities.checkResult(emaRequestMsgClone.priorityClass() == emaRequestMsg.priorityClass(), "Compare priorityClass");
		TestUtilities.checkResult(emaRequestMsgClone.priorityCount() == emaRequestMsg.priorityCount(), "Compare priorityCount");

		TestUtilities.checkResult(emaRequestMsgClone.hasQos() == emaRequestMsg.hasQos(), "Compare hasQos");
		TestUtilities.checkResult(emaRequestMsgClone.qosTimeliness() == emaRequestMsg.qosTimeliness(), "Compare qosTimeliness");
		TestUtilities.checkResult(emaRequestMsgClone.qosRate() == emaRequestMsg.qosRate(), "Compare qosRate");
		
		TestUtilities.checkResult(emaRequestMsgClone.hasMsgKey() == emaRequestMsg.hasMsgKey(), "Compare hasMsgKey");
		
		String emaRequestMsgString = emaRequestMsg.toString();
		String emaRequestMsgCloneString = emaRequestMsgClone.toString();
		
		System.out.println("Cloned EMA RequestMsg:");
		System.out.println(emaRequestMsgClone);
		
		TestUtilities.checkResult(emaRequestMsgString.equals(emaRequestMsgCloneString), "emaRequestMsgString.equals(emaRequestMsgCloneString)");
		
		emaRequestMsgClone.streamId(10);
		emaRequestMsgClone.payload().fieldList().add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		TestUtilities.checkResult(emaRequestMsgClone.streamId() != emaRequestMsg.streamId(), "Compare streamId");

		// Check emaRequestMsg for no FID 21
		Iterator<FieldEntry> iter = emaRequestMsg.payload().fieldList().iterator();
		FieldEntry fieldEntry;
		while (iter.hasNext())
		{
			fieldEntry = iter.next();
			TestUtilities.checkResult(fieldEntry.fieldId() != 21, "Check emaRequestMsg for no FID 21");
		}
		
		boolean foundFid = false;
		Iterator<FieldEntry> iterClone = emaRequestMsgClone.payload().fieldList().iterator();
		while (iterClone.hasNext())
		{
			fieldEntry = iterClone.next();
			if(foundFid = fieldEntry.fieldId() == 21)
				break;
		}
		
		TestUtilities.checkResult(foundFid, "Check emaRequestMsgClone for FID 21");
		
		emaRequestMsgString = emaRequestMsg.toString();
		emaRequestMsgCloneString = emaRequestMsgClone.toString();
		
		TestUtilities.checkResult(!emaRequestMsgString.equals(emaRequestMsgCloneString), "Check that emaRequestMsgString does not equal emaRequestMsgCloneString");
		
		
		System.out.println("End EMA RequestMsg Clone");
		System.out.println();
	}

	private void compareEmaReqMsgFields(com.rtsdk.ema.access.ReqMsg expected, com.rtsdk.ema.access.ReqMsg actual, String checkPrefix) {
		TestMsgUtilities.compareMsgFields(expected, actual, checkPrefix + " base message");
		checkPrefix = checkPrefix + " compare: ";

		TestUtilities.checkResult(expected.hasServiceName() == actual.hasServiceName(), checkPrefix + "hasServiceName");
		if(expected.hasServiceName())
			TestUtilities.checkResult(expected.serviceName().equals(actual.serviceName()), checkPrefix + "serviceId" + "exp=" +actual.serviceName() + " act="+expected.serviceName());

		TestUtilities.checkResult(expected.privateStream() == actual.privateStream(), checkPrefix + "privateStream");

		TestUtilities.checkResult(expected.hasQos() == actual.hasQos(), checkPrefix + "hasQos");
		if(expected.hasQos()) {
			TestUtilities.checkResult(expected.qosRate() == actual.qosRate(), checkPrefix + "qosRate");
			TestUtilities.checkResult(expected.qosTimeliness() == actual.qosTimeliness(), checkPrefix + "qosTimeliness");
		}

		TestUtilities.checkResult(expected.hasPriority() == actual.hasPriority(), checkPrefix + "hasPriority");
		if(expected.hasPriority()) {
			TestUtilities.checkResult(expected.priorityClass() == actual.priorityClass(), checkPrefix + "priorityClass");
			TestUtilities.checkResult(expected.priorityCount() == actual.priorityCount(), checkPrefix + "priorityCount");
		}
	}
}
