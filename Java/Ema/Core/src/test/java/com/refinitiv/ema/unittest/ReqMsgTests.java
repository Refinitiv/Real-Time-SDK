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
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.refinitiv.eta.codec.*;

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

		String reqMsgString = "ReqMsg\n" +
				"    streamId=\"15\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
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
				"\n" +
				"    PayloadEnd\n" +
				"ReqMsgEnd\n";

		String reqMsgEmptyString = "ReqMsg\n" +
				"    streamId=\"0\"\n" +
				"    domain=\"MarketPrice Domain\"\n" +
				"ReqMsgEnd\n";

	    com.refinitiv.ema.access.FieldList fl = EmaFactory.createFieldList();
	    
	    TestUtilities.EmaEncodeFieldListAll(fl);
	    
	    com.refinitiv.ema.access.ReqMsg reqMsg = EmaFactory.createReqMsg();
	    com.refinitiv.ema.access.ReqMsg reqMsgEmpty = EmaFactory.createReqMsg();

		System.out.println("Begin EMA ReqMsg test after constructor");

		TestUtilities.checkResult(reqMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
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
		
		TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg test after constructor");
		System.out.println();		
		
	    System.out.println("Begin EMA ReqMsg Set");
	    
	    reqMsg.domainType( com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE );
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	    
	    reqMsg.initialImage(true);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	    
	    reqMsg.interestAfterRefresh(true);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
	    reqMsg.streamId( 15 );
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
	    reqMsg.qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

	    reqMsg.name("ABCDEF");
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
	    reqMsg.nameType( com.refinitiv.eta.rdm.InstrumentNameTypes.RIC );
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

	    reqMsg.serviceId(5);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

	    reqMsg.filter( 12 );
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	
	    reqMsg.id(21);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	    
	    reqMsg.priority(5, 7);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	    
	    reqMsg.qos(com.refinitiv.ema.access.ReqMsg.Timeliness.REALTIME, com.refinitiv.ema.access.ReqMsg.Rate.TICK_BY_TICK);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
	    
	    reqMsg.conflatedInUpdates(true);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
	    reqMsg.attrib(fl);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
		
	    reqMsg.payload(fl);
		TestUtilities.checkResult("ReqMsg.toString() == toString()", reqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

		System.out.println("End EMA ReqMsg Set");
		System.out.println();

		System.out.println("Begin EMA ReqMsg Decoding");

		com.refinitiv.ema.rdm.DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

		TestUtilities.checkResult("ReqMsg.toString(dictionary) == toString(dictionary)", reqMsg.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

		emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
		emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

		TestUtilities.checkResult("ReqMsg.toString(dictionary) == toString(dictionary)", reqMsgEmpty.toString(emaDataDictionary).equals(reqMsgEmptyString));

		TestUtilities.checkResult("ReqMsg.toString(dictionary) == toString(dictionary)", reqMsg.toString(emaDataDictionary).equals(reqMsgString));

		com.refinitiv.ema.access.ReqMsg emaReqMsg = JUnitTestConnect.createReqMsg();

		JUnitTestConnect.setRsslData(emaReqMsg, reqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);

		com.refinitiv.ema.access.ReqMsg reqMsgClone = EmaFactory.createReqMsg(reqMsg);
		reqMsgClone.clear();
		TestUtilities.checkResult("ReqMsg.toString(dictionary) == toString(dictionary)", reqMsgClone.toString(emaDataDictionary).equals(reqMsgEmptyString));

		// check that we can still get the toString on encoded/decoded msg.
		TestUtilities.checkResult("ReqMsg.toString() != toString()", !(emaReqMsg.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));
	
		TestUtilities.checkResult(emaReqMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
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
		
		TestUtilities.checkResult(emaReqMsg.nameType() == com.refinitiv.ema.rdm.EmaRdm.INSTRUMENT_NAME_RIC , "ReqMsg.nameType()");

		TestUtilities.checkResult(emaReqMsg.hasName(), "ReqMsg.hasName()");
		
		TestUtilities.checkResult(emaReqMsg.name().compareTo("ABCDEF") == 0, "ReqMsg.name()");
		
		TestUtilities.checkResult(reqMsg.hasPriority(), "ReqMsg.hasPriority()");
		
		TestUtilities.checkResult(emaReqMsg.priorityClass() == 5, "ReqMsg.priorityClass()");
		
		TestUtilities.checkResult(emaReqMsg.priorityCount() == 7, "ReqMsg.priorityCount()");
		
		TestUtilities.checkResult(reqMsg.hasQos(), "ReqMsg.hasQos()");
		
		TestUtilities.checkResult(reqMsg.qosRate() ==  com.refinitiv.ema.access.ReqMsg.Rate.TICK_BY_TICK, "ReqMsg.qosRate()");
		
		TestUtilities.checkResult(reqMsg.qosTimeliness() ==  com.refinitiv.ema.access.ReqMsg.Timeliness.REALTIME, "ReqMsg.qosTimeliness()");
		
		TestUtilities.checkResult(reqMsg.conflatedInUpdates() ==  true, "ReqMsg.conflatedInUpdates()");

		TestUtilities.checkResult(emaReqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(emaReqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.FIELD_LIST, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg Decoding");
		System.out.println();

		reqMsg.clear();
		
		System.out.println("Begin EMA ReqMsg test after clear");

		TestUtilities.checkResult(reqMsg.domainType() == com.refinitiv.ema.rdm.EmaRdm.MMT_MARKET_PRICE, "ReqMsg.domainType()");
		
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
		
		TestUtilities.checkResult(reqMsg.attrib().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.attrib().dataType()");
		
		TestUtilities.checkResult(reqMsg.payload().dataType() == com.refinitiv.ema.access.DataType.DataTypes.NO_DATA, "ReqMsg.payload().dataType()");

		System.out.println("End EMA ReqMsg test after clear");
		System.out.println();
	}
	
	public void testReqMsg_EncodeETARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETARequestMsgWithFieldListTypeAsAttrib_ExtendedHeader_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for FieldList as attrib, extended header and payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FIELD_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.FILTER_LIST)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.FILTER_LIST);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.SERIES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.SERIES);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyReqMsg.attrib().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETASeriesAll(decCopyReqMsg.payload().series(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithSeriesTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for Vector as attrib, payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.VECTOR)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.VECTOR);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyReqMsg.attrib().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     TestUtilities.EmaDecode_ETAVectorAll(decCopyReqMsg.payload().vector(), com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithVectorTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for Map as attrib, payload, Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MAP)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.MAP);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyReqMsg.attrib().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETAMapKeyUIntAll(decCopyReqMsg.payload().map(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETARefreshWithMapTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}
	
	public void testReqMsg_EncodeETAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode()
	{ 
	    int retVal;
		 
		TestUtilities.printTestHead("testReqMsg_EncodeETAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode", "Encode ReqMsg with ETA for RefreshMsg as attrib, payload. Encode it to another ReqMsg.");

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
		
		if ( ( retVal = TestUtilities.eta_EncodeRequestMsgAll(encodeIter, com.refinitiv.eta.codec.DataTypes.MSG)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" +retVal + " encountered with TestUtilities.eta_EncodeRequestMsgAll(). "
					+ " Error Text: " + CodecReturnCodes.info(retVal)); 
			return;
		}
		
		 // Decode ReqMsg with EMA.
	     com.refinitiv.ema.access.ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
	     JUnitTestConnect.setRsslData(reqMsg, buffer, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	    TestUtilities.EmaDecode_ETARequestMsgAll(reqMsg, com.refinitiv.eta.codec.DataTypes.MSG);
	
	     com.refinitiv.ema.access.ReqMsg copyReqMsg = EmaFactory.createReqMsg();
	     
	     // Encode to another ReqMsg
	     copyReqMsg.extendedHeader(reqMsg.extendedHeader());
	     copyReqMsg.attrib(reqMsg.attrib().data());
	     copyReqMsg.payload(reqMsg.payload().data());
	     
	     com.refinitiv.ema.access.ReqMsg decCopyReqMsg = JUnitTestConnect.createReqMsg();
	     
	     JUnitTestConnect.setRsslData(decCopyReqMsg, copyReqMsg, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	     
	     // Check result
	     TestUtilities.checkResult(decCopyReqMsg.extendedHeader().equals(reqMsg.extendedHeader()));
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyReqMsg.attrib().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     TestUtilities.EmaDecode_ETARefreshMsgAll(decCopyReqMsg.payload().refreshMsg(), com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
	     
	     System.out.println("\ttestReqMsg_EncodeETAReqMsgWithRefreshTypeAsAttrib_Payload_EncodeEMA_ToAnotherReqMsg_EMADecode passed");
	}

	public void testReqMsg_cloneMsgKeyWLScenario()
	{
		TestUtilities.printTestHead("testReqMsg_cloneMsgKeyWLScenario", "cloning for minimal ema req message");
		ReqMsg emaMsg = EmaFactory.createReqMsg();
		emaMsg.payload(TestMsgUtilities.createFiledListBodySample());

		JUnitTestConnect.getRsslData(emaMsg);
		/** @see com.refinitiv.eta.valueadd.reactor.WlItemHandler#callbackUser(String, com.refinitiv.eta.codec.Msg, MsgBase, WlRequest, ReactorErrorInfo) */
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
		
	    System.out.println("Begin ETA RequestMsg Set");
		com.refinitiv.eta.codec.RequestMsg requestMsg = (com.refinitiv.eta.codec.RequestMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		requestMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REQUEST);
		
		requestMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
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
		requestMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		requestMsg.msgKey().encodedAttrib(fieldListBuf);
		
		requestMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		requestMsg.encodedDataBody(fieldListBuf);

		setMoreFields(requestMsg);

		System.out.println("End ETA RequestMsg Set");
		System.out.println();

		System.out.println("Begin ETA RequestMsg Buffer Encoding");

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
		
		requestMsg.encode(encIter);
		
		System.out.println("End ETA RequestMsg Buffer Encoding");
		System.out.println();
		
		System.out.println("Begin EMA RequestMsg Clone");
		com.refinitiv.eta.codec.RequestMsg requestMsgDecode = (com.refinitiv.eta.codec.RequestMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		requestMsgDecode.decode(decIter);

		com.refinitiv.ema.access.ReqMsg emaRequestMsg = JUnitTestConnect.createReqMsg();
				
		JUnitTestConnect.setRsslData(emaRequestMsg, requestMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.ReqMsg emaRequestMsgClone = EmaFactory.createReqMsg(emaRequestMsg);
		
		compareEmaReqMsgFields(emaRequestMsg, emaRequestMsgClone, "Request clone message");

		String emaRequestMsgString = emaRequestMsg.toString();
		String emaRequestMsgCloneString = emaRequestMsgClone.toString();
		
		System.out.println("Cloned EMA RequestMsg:");
		System.out.println(emaRequestMsgClone);
		
		TestUtilities.checkResult(emaRequestMsgString.equals(emaRequestMsgCloneString), "emaRequestMsgString.equals(emaRequestMsgCloneString)");

		com.refinitiv.ema.access.ReqMsg emaRequestMsgClone2 = EmaFactory.createReqMsg(emaRequestMsgClone);
		compareEmaReqMsgFields(emaRequestMsg, emaRequestMsgClone2, "Request double-cloned message");
		String emaRequestMsgClone2String = emaRequestMsgClone2.toString();
		TestUtilities.checkResult(emaRequestMsgString.equals(emaRequestMsgClone2String), "double-cloned emaRequestMsgString.equals(emaRequestMsgClone2String)");

		System.out.println("End EMA RequestMsg Clone");
		System.out.println();
	}

	private void setMoreFields(com.refinitiv.eta.codec.RequestMsg requestMsg) {
		requestMsg.applyHasExtendedHdr();
		Buffer extendedHeader = CodecFactory.createBuffer();
		extendedHeader.data(ByteBuffer.wrap(new byte[] {5, -6, 7, -8}));
		requestMsg.extendedHeader(extendedHeader);

		requestMsg.applyPrivateStream();
	}

	public void testRequestMsg_cloneEdit()
	{
		TestUtilities.printTestHead("testRequestMsg_cloneEdit", "clone and edit ema request message");
		
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
		
	    System.out.println("Begin ETA RequestMsg Set");
		com.refinitiv.eta.codec.RequestMsg requestMsg = (com.refinitiv.eta.codec.RequestMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();
		requestMsg.msgClass(com.refinitiv.eta.codec.MsgClasses.REQUEST);
		
		requestMsg.domainType( com.refinitiv.eta.rdm.DomainTypes.MARKET_PRICE );
		
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
		requestMsg.msgKey().attribContainerType( com.refinitiv.eta.codec.DataTypes.FIELD_LIST );
		requestMsg.msgKey().encodedAttrib(fieldListBuf);
		
		requestMsg.containerType(com.refinitiv.eta.codec.DataTypes.FIELD_LIST);
		requestMsg.encodedDataBody(fieldListBuf);

		System.out.println("End ETA RequestMsg Set");
		System.out.println();

		System.out.println("Begin ETA RequestMsg Buffer Encoding");

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
		
		requestMsg.encode(encIter);
		
		System.out.println("End ETA RequestMsg Buffer Encoding");
		System.out.println();
		
		System.out.println("Begin EMA RequestMsg Clone");
		com.refinitiv.eta.codec.RequestMsg requestMsgDecode = (com.refinitiv.eta.codec.RequestMsg)com.refinitiv.eta.codec.CodecFactory.createMsg();

		com.refinitiv.eta.codec.DecodeIterator decIter = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
		decIter.setBufferAndRWFVersion(msgBuf, majorVersion, minorVersion);
		requestMsgDecode.decode(decIter);

		com.refinitiv.ema.access.ReqMsg emaRequestMsg = JUnitTestConnect.createReqMsg();
				
		JUnitTestConnect.setRsslData(emaRequestMsg, requestMsgDecode, majorVersion, minorVersion, dictionary, null);
		
		com.refinitiv.ema.access.ReqMsg emaRequestMsgClone = EmaFactory.createReqMsg(emaRequestMsg);
		
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

	private void compareEmaReqMsgFields(com.refinitiv.ema.access.ReqMsg expected, com.refinitiv.ema.access.ReqMsg actual, String checkPrefix) {
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
