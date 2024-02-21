///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.unittest;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;

import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.unittest.TestUtilities.EncodingTypeFlags;
import com.refinitiv.ema.access.ElementEntry;
import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldEntry;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.JUnitTestConnect;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmArrayEntry;
import com.refinitiv.ema.access.OmmError;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmOpaque;
import com.refinitiv.ema.access.OmmQos;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.OmmXml;
import com.refinitiv.ema.unittest.TestUtilities;

import junit.framework.TestCase;

public class FieldListTests extends TestCase
{

	public FieldListTests(String name)
	{
		super(name);
	}

	public void testFieldList_EncodeETA_DecodeEMA_DecodeAll()
	{
		TestUtilities.printTestHead("testFieldList_EncodeETA_DecodeEMA_DecodeAll", "Encode FieldList with ETA and Decode FieldList with EMA");

		// Create a ETA Buffer to encode into
		com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		// Encode FieldList with ETA.
		int retVal;
		System.out.println("Begin ETA FieldList Encoding");
		if ((retVal = TestUtilities.eta_EncodeFieldListAll(buf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding field list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}
		System.out.println("End ETA FieldList Encoding");
		System.out.println();

		// Decode FieldList with EMA.
		com.refinitiv.ema.access.FieldList fl = JUnitTestConnect.createFieldList();
		JUnitTestConnect.setRsslData(fl, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		TestUtilities.EmaDecode_ETAFieldListAll(fl, EncodingTypeFlags.PRIMITIVE_TYPES);

		System.out.println("\ntestFieldList_EncodeETA_decodeEMA_DecodeAll passed");
	}
	

    public void testFieldList_EncodeETA_DecodeEMA_EncodeEMA_DecodeETA()
    {
        TestUtilities.printTestHead("testFieldList_EncodeETA_DecodeEMA_EncodeEMA_DecodeETA", "Encode FieldList with ETA, Decode FieldList with EMA, Encode FieldList with EMA, Decode FieldList with ETA");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));

        // load dictionary
        com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
                .createDataDictionary();
        TestUtilities.eta_encodeDictionaryMsg(dictionary);

        // Encode FieldList with ETA.
        int retVal;
        if ((retVal = TestUtilities.eta_EncodeFieldListAll(buf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FieldList with EMA.
        com.refinitiv.ema.access.FieldList fl = JUnitTestConnect.createFieldList();
        JUnitTestConnect.setRsslData(fl, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        TestUtilities.EmaDecode_ETAFieldListAll(fl, EncodingTypeFlags.PRIMITIVE_TYPES);
        
        // Copy decoded entries into a different FieldList with EMA
        com.refinitiv.ema.access.FieldList flCopy = EmaFactory.createFieldList();
        flCopy.info(fl.infoDictionaryId(), fl.infoFieldListNum());
        Iterator<FieldEntry> iterator = fl.iterator();
        while (iterator.hasNext())
        {
            flCopy.add(iterator.next());
        }
        
        // decode field list copy
        FieldList flDecCopy = JUnitTestConnect.createFieldList();
        
        flDecCopy.info(flCopy.infoDictionaryId(), flCopy.infoFieldListNum());
        
        JUnitTestConnect.setRsslData(flDecCopy, flCopy, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        // compare with original
        assertEquals(flDecCopy.size(), fl.size());
        
        TestUtilities.EmaDecode_ETAFieldListAll(flDecCopy, EncodingTypeFlags.PRIMITIVE_TYPES);

        System.out.println("\ntestFieldList_EncodeETA_DecodeEMA_EncodeEMA_DecodeEMA passed");
    }
    
    public void testFieldList_EncodeETAFLWithPrimitiveTypes_EncodeEMA_ToAnotherFieldList_EMADecode() 
    {
        TestUtilities.printTestHead("testFieldList_EncodeETAFLWithPrimitiveTypes_EncodeEMA_ToAnotherFieldList_EMADecode", "Encode FieldList with ETA for primitive types, Encode it to another FieldList.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(15000));

        // Encode FieldList with ETA.
        int retVal;
        if ((retVal = TestUtilities.eta_EncodeFieldListAll(buf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeFieldListAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FieldList with EMA.
        com.refinitiv.ema.access.FieldList fl = JUnitTestConnect.createFieldList();
        JUnitTestConnect.setRsslData(fl, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETAFieldListAll(fl, EncodingTypeFlags.PRIMITIVE_TYPES);
        
        // Copy decoded entries into a different FieldList with EMA     
        com.refinitiv.ema.access.FieldList flCopy = EmaFactory.createFieldList();
        
        flCopy.info(fl.infoDictionaryId(), fl.infoFieldListNum());
 
        Iterator<FieldEntry> iterator = fl.iterator();
        while (iterator.hasNext())
        {
            flCopy.add(iterator.next());
        }
        
        assertEquals(flCopy.size(), fl.size());
        
        com.refinitiv.ema.access.FieldList flDecCopy = JUnitTestConnect.createFieldList();
        
        JUnitTestConnect.setRsslData(flDecCopy, flCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETAFieldListAll(flDecCopy, EncodingTypeFlags.PRIMITIVE_TYPES);

        System.out.println("\testFieldList_EncodeETAFLWithPrimitiveTypes_EncodeEMA_ToAnotherFieldList_EMADecode passed");
    }
    
    public void testFieldList_EncodeETAFLWithContainerTypes_EncodeEMA_ToAnotherFieldList_EMADecode() 
    {
        TestUtilities.printTestHead("testFieldList_EncodeETAFLWithContainerTypes_EncodeEMA_ToAnotherFieldList_EMADecode", "Encode FieldList with ETA for container types, Encode it to another FieldList.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(15000));

        // Encode FieldList with ETA.
        int retVal;
        if ((retVal = TestUtilities.eta_EncodeFieldListAll(buf, TestUtilities.EncodingTypeFlags.CONTAINER_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeFieldListAll for container types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FieldList with EMA.
        com.refinitiv.ema.access.FieldList fl = JUnitTestConnect.createFieldList();
        JUnitTestConnect.setRsslData(fl, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETAFieldListAll(fl, EncodingTypeFlags.CONTAINER_TYPES);
        
        // Copy decoded entries into a different FieldList with EMA     
        com.refinitiv.ema.access.FieldList flCopy = EmaFactory.createFieldList();
        
        flCopy.info(fl.infoDictionaryId(), fl.infoFieldListNum());
 
        Iterator<FieldEntry> iterator = fl.iterator();
        while (iterator.hasNext())
        {
            flCopy.add(iterator.next());
        }
        
        assertEquals(flCopy.size(), fl.size());
        
        com.refinitiv.ema.access.FieldList flDecCopy = JUnitTestConnect.createFieldList();
        
        JUnitTestConnect.setRsslData(flDecCopy, flCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETAFieldListAll(flDecCopy, EncodingTypeFlags.CONTAINER_TYPES);

        System.out.println("\ntestFieldList_EncodeETAFLWithContainerTypes_EncodeEMA_ToAnotherFieldList_EMADecode passed");
    }
    
    public void testFieldList_EncodeETAFLWithMsgTypes_EncodeEMA_ToAnotherFieldList_EMADecode() 
    {
        TestUtilities.printTestHead("testFieldList_EncodeETAFLWithMsgTypes_EncodeEMA_ToAnotherFieldList_EMADecode", "Encode FieldList with ETA for message types, Encode it to another FieldList.");

        // Create a ETA Buffer to encode into
        com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(8192));

        // Encode FieldList with ETA.
        int retVal;
        if ((retVal = TestUtilities.eta_EncodeFieldListAll(buf, TestUtilities.EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.eta_EncodeFieldListAll for message types.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode FieldList with EMA.
        com.refinitiv.ema.access.FieldList fl = JUnitTestConnect.createFieldList();
        JUnitTestConnect.setRsslData(fl, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different FieldList with EMA     
        com.refinitiv.ema.access.FieldList flCopy = EmaFactory.createFieldList();
        
        flCopy.info(fl.infoDictionaryId(), fl.infoFieldListNum());
 
        Iterator<FieldEntry> iterator = fl.iterator();
        while (iterator.hasNext())
        {
            flCopy.add(iterator.next());
        }
        
        assertEquals(flCopy.size(), fl.size());
        
        com.refinitiv.ema.access.FieldList flDecCopy = JUnitTestConnect.createFieldList();
        
        JUnitTestConnect.setRsslData(flDecCopy, flCopy, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        TestUtilities.EmaDecode_ETAFieldListAll(flDecCopy, EncodingTypeFlags.MESSAGE_TYPES);

        System.out.println("\ntestFieldList_EncodeETAFLWithMsgTypes_EncodeEMA_ToAnotherFieldList_EMADecode passed");
    }
    
	public void testFieldList_EncodeEMA_EmptyEncode()
	{
		TestUtilities.printTestHead("testFieldList_EncodeEMA_EmptyEncode", "Encode empty fieldlist on map entry, should not get exception");
		
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.info(1, 1);
		MapEntry mapEntry = EmaFactory.createMapEntry();
		
		try {
		mapEntry.keyAscii(TestUtilities.KEY_ASCII, MapEntry.MapAction.ADD, fieldList);
		TestUtilities.checkResult( "EMA Error: empty FieldList - should not get expected exception", true);
        }
        catch(OmmException excp)
        {
        	TestUtilities.checkResult( "Exception not expected (empty FieldList - exception) : " + excp.getMessage(),  false);
        }
	}
	
	public void testFieldList_EncodeEMA_DecodeEMA_DecodeAll() throws UnsupportedEncodingException
	{
		TestUtilities.printTestHead("testFieldList_EncodeEMA_DecodeEMA_DecodeAll", "Encode FieldList with EMA and Decode FieldList with EMA");

		String fieldListString = "FieldList FieldListNum=\"65\" DictionaryId=\"0\"\n" +
				"    FieldEntry fid=\"-100\" name=\"\" dataType=\"Error\"\n" +
				"        OmmError\n" +
				"              ErrorCode=\"FieldIdNotFound\"\n" +
				"        OmmErrorEnd\n" +
				"    FieldEntryEnd\n" +
				"    FieldEntry fid=\"1\" name=\"PROD_PERM\" dataType=\"UInt\" value=\"64\"\n" +
				"    FieldEntry fid=\"6\" name=\"TRDPRC_1\" dataType=\"Real\" value=\"0.11\"\n" +
				"    FieldEntry fid=\"-2\" name=\"INTEGER\" dataType=\"Int\" value=\"32\"\n" +
				"    FieldEntry fid=\"16\" name=\"TRADE_DATE\" dataType=\"Date\" value=\"07 NOV 1999\"\n" +
				"    FieldEntry fid=\"18\" name=\"TRDTIM_1\" dataType=\"Time\" value=\"02:03:04:005:000:000\"\n" +
				"    FieldEntry fid=\"-3\" name=\"TRADE_DATE\" dataType=\"DateTime\" value=\"07 NOV 1999 01:02:03:000:000:000\"\n" +
				"    FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"Timeliness: 5656/Rate: 2345\"\n" +
				"    FieldEntry fid=\"-6\" name=\"MY_STATE\" dataType=\"State\" value=\"Open / Ok / None / 'Succeeded'\"\n" +
				"    FieldEntry fid=\"715\" name=\"STORY_ID\" dataType=\"Ascii\" value=\"ABCDEF\"\n" +
				"    FieldEntry fid=\"28\" name=\"NEWS\" dataType=\"Rmtes\" value=\"ABCDEF\"\n" +
				"    FieldEntry fid=\"4\" name=\"RDN_EXCHID\" dataType=\"Enum\" value=\"29\"\n" +
				"    FieldEntry fid=\"-9\" name=\"MY_FLOAT\" dataType=\"Float\" value=\"11.11\"\n" +
				"    FieldEntry fid=\"-10\" name=\"MY_DOUBLE\" dataType=\"Double\" value=\"22.219999313354492\"\n" +
				"    FieldEntry fid=\"7\" name=\"TRDPRC_2\" dataType=\"Real\" value=\"(blank data)\"\n" +
				"    FieldEntry fid=\"-11\" name=\"MY_BUFFER\" dataType=\"Buffer\"\n" +
				"4142 4344 4546 4748                            ABCDEFGH\n" +
				"    FieldEntry fid=\"-12\" name=\"MY_UTF8\" dataType=\"Utf8\" value=\"KLMNOPQR\"\n" +
				"    FieldEntry fid=\"-16\" name=\"MY_ARRAY\" dataType=\"OmmArray\"\n" +
				"        OmmArray with entries of dataType=\"Int\"\n" +
				"            value=\"\"123\"\n" +
				"            value=\"\"234\"\n" +
				"            value=\"\"345\"\n" +
				"        OmmArrayEnd\n" +
				"    FieldEntryEnd\n" +
				"    FieldEntry fid=\"-17\" name=\"MY_OPAQUE\" dataType=\"Opaque\" value=\"Opaque\n" +
				"\n" +
				"4F50 5152 5354                                  OPQRST\n" +
				"OpaqueEnd\n" +
				"\"\n" +
				"    FieldEntry fid=\"-5\" name=\"MY_QOS\" dataType=\"Qos\" value=\"InexactDelayed/JustInTimeConflated\"\n" +
				"FieldListEnd\n";

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		//EMA Encode FieldList
		FieldList flEnc = EmaFactory.createFieldList();
		FieldList flEmpty = EmaFactory.createFieldList();
		flEnc.info( dictionary.infoDictionaryId(), 65 );
		
		try {
			//EMA Encoding

			//first entry (fid not found case)
			FieldEntry fe = EmaFactory.createFieldEntry().uintValue( -100, 64 );
			TestUtilities.checkResult("FieldEntry.toString() == toString()", fe.toString().equals("\nEntity is not encoded yet. Complete encoding to use this method.\n"));
			
			flEnc.add(fe);
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//second entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//third entry
			flEnc.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//fourth entry
			flEnc.add(EmaFactory.createFieldEntry().intValue( -2, 32));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//fifth entry
			flEnc.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//sixth entry
			flEnc.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//seventh entry
			flEnc.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//eightth entry
			flEnc.add(EmaFactory.createFieldEntry().qos( TestUtilities.MY_QOS, 5656, 2345 ));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//ninth entry
			flEnc.add(EmaFactory.createFieldEntry().state(TestUtilities.MY_STATE, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Succeeded"));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//tenth entry
			flEnc.add(EmaFactory.createFieldEntry().ascii( 715, "ABCDEF" ));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//eleventh entry
			flEnc.add(EmaFactory.createFieldEntry().rmtes( 28,ByteBuffer.wrap("ABCDEF".getBytes())));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//twelfth entry
			flEnc.add(EmaFactory.createFieldEntry().enumValue( 4, 29));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//thirteenth entry
			flEnc.add(EmaFactory.createFieldEntry().floatValue( -9, 11.11f));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//fourteenth entry
			flEnc.add(EmaFactory.createFieldEntry().doubleValue( -10, 22.22f));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//fifteenth entry (blank real)
			flEnc.add(EmaFactory.createFieldEntry().codeReal( 7));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//sixteenth entry
			flEnc.add(EmaFactory.createFieldEntry().buffer( -11, ByteBuffer.wrap("ABCDEFGH".getBytes())));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//seventeenth entry
			flEnc.add(EmaFactory.createFieldEntry().utf8( -12,ByteBuffer.wrap("KLMNOPQR".getBytes())));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			//eighteenth entry
			OmmArray ar1 = EmaFactory.createOmmArray();
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(123));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(234));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(345));
			flEnc.add(EmaFactory.createFieldEntry().array( -16, ar1));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
			

			//nineteenth entry
			OmmOpaque opaque1 = EmaFactory.createOmmOpaque();
			opaque1.buffer(ByteBuffer.wrap("OPQRST".getBytes()));
			flEnc.add(EmaFactory.createFieldEntry().opaque( -17 , opaque1));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));
					
			//twentyth entry
			flEnc.add(EmaFactory.createFieldEntry().qos( TestUtilities.MY_QOS, 756565, 1232365));
			TestUtilities.checkResult("FieldList.toString() == toString()", flEnc.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n"));

			DataDictionary emaDataDictionary = EmaFactory.createDataDictionary();

			TestUtilities.checkResult("FieldList.toString(dictionary) == toString(dictionary)", flEnc.toString(emaDataDictionary).equals("\nDictionary is not loaded.\n"));

			emaDataDictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
			emaDataDictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

			TestUtilities.checkResult("FieldList.toString(dictionary) == toString(dictionary)", flEnc.toString(emaDataDictionary).equals(fieldListString));

			TestUtilities.checkResult("FieldList.toString(dictionary) == toString(dictionary)", flEmpty.toString(emaDataDictionary).equals("FieldList\nFieldListEnd\n"));

			flEmpty.add(EmaFactory.createFieldEntry().opaque( -17 , opaque1));
			flEmpty.clear();
			TestUtilities.checkResult("FieldList.toString(dictionary) == toString(dictionary)", flEmpty.toString(emaDataDictionary).equals("FieldList\nFieldListEnd\n"));

			FieldList flDec = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
			// check that we can still get the toString on encoded/decoded container.
			TestUtilities.checkResult("FieldList.toString() != toString()", !(flDec.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));

			System.out.println(flDec);

			TestUtilities.checkResult("FieldList with all data types - hasInfo()" , flDec.hasInfo());
			TestUtilities.checkResult("FieldList with all data types - infoDictionaryId()",  flDec.infoDictionaryId() == dictionary.infoDictionaryId() );
			TestUtilities.checkResult("FieldList with all data types - infoFieldListNum()", flDec.infoFieldListNum() == 65 );

			Iterator<FieldEntry> iter = flDec.iterator();
			TestUtilities.checkResult("FieldList with all datat types - size of collection = 20 ", flDec.size() == 20);
			TestUtilities.checkResult("FieldList with all datat types - collection isEmpty is false ", !flDec.isEmpty());

			TestUtilities.checkResult("FieldList with all data types- first hasNext()" , iter.hasNext());
			FieldEntry fe1 = iter.next();
			// check that we can still get the toString on encoded/decoded entry.
			TestUtilities.checkResult("FieldEntry.toString() != toString()", !(fe1.toString().equals("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n")));
			
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
			TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" , fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND);
			try {
				fe1.intValue();
				TestUtilities.checkResult( "FieldList with all data types - exception expected", false);
			}
			catch ( OmmException excp )
			{
				TestUtilities.checkResult("FieldList with all data types - exception expected: " +  excp.getMessage() , true );
			}

			TestUtilities.checkResult("FieldList with all data types - second hasNext()", iter.hasNext() );
			FieldEntry fe2 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
			
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );
			
			TestUtilities.checkResult("FieldList with all data types - third entry", iter.hasNext());
			FieldEntry fe3 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe3.fieldId() == 6 );
			TestUtilities.checkResult("FieldEntry.name()",fe3.name().equals("TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
			TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );


			System.out.println("FieldList get iterator again");
			iter = flDec.iterator();
			{
				TestUtilities.checkResult("FieldList with all data types - hasInfo()" , flDec.hasInfo() );
				TestUtilities.checkResult("FieldList with all data types- infoDictionaryId()", flDec.infoDictionaryId() == dictionary.infoDictionaryId() );
				TestUtilities.checkResult("FieldList with all data types- infoFieldListNum()", flDec.infoFieldListNum() == 65 );

				TestUtilities.checkResult("FieldList with all data types - first entry again", iter.hasNext());
				fe1 = iter.next();
				TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
				TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe1.code() == Data.DataCode.NO_CODE);
				TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" ,  fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND);

				try {
					fe1.uintValue();
					TestUtilities.checkResult( "FieldList with all data types - exception expected : ", false);
				}
				catch ( OmmException excp )
				{
					TestUtilities.checkResult("FieldList with all data types - exception expected: " +  excp.getMessage() , true);
				}

				TestUtilities.checkResult("FieldList with all data types - first entry again", iter.hasNext());
				fe2 = iter.next();
				TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
				TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM"));
				TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT",  fe2.loadType() == DataTypes.UINT);
				TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE",  fe2.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );

				TestUtilities.checkResult("FieldList with all data types - first entry again", iter.hasNext());
				fe3 = iter.next();
				TestUtilities.checkResult("FieldEntry.fieldId()", fe3.fieldId() == 6 );
				TestUtilities.checkResult("FieldEntry.name()",fe3.name().equals( "TRDPRC_1"));
				TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
				TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe3.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
				TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );
			}
			
			TestUtilities.checkResult("FieldList with all data types - 4th entry", iter.hasNext());
			FieldEntry fe4 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe4.fieldId() == -2 );
			TestUtilities.checkResult("FieldEntry.name()",fe4.name().equals( "INTEGER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", fe4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.intValue()" ,  fe4.intValue() == 32);

			TestUtilities.checkResult("FieldList with all data types - 5th entry", iter.hasNext());
			FieldEntry fe5 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe5.fieldId() == 16 );
			TestUtilities.checkResult("FieldEntry.name()",fe5.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", fe5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("FieldEntry.date().day()", fe5.date().day() == 7 );
			TestUtilities.checkResult("FieldEntry.date().month()()", fe5.date().month()== 11 );
			TestUtilities.checkResult("FieldEntry.date().year()", fe5.date().year() == 1999 );

			TestUtilities.checkResult("FieldList with all data types - 6th entry", iter.hasNext());
			FieldEntry fe6 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",   fe6.fieldId() == 18 );
			TestUtilities.checkResult("FieldEntry.name()",fe6.name().equals( "TRDTIM_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().hour()", fe6.time().hour() == 02 );
			TestUtilities.checkResult("FieldEntry.time().minute()", fe6.time().minute() == 03 );
			TestUtilities.checkResult("FieldEntry.time().second()", fe6.time().second() == 04 );
			TestUtilities.checkResult("FieldEntry.time().millisecond()", fe6.time().millisecond() == 005 );

			TestUtilities.checkResult("FieldList with all data types - 7th entry", iter.hasNext());
			FieldEntry fe7 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",   fe7.fieldId() == -3 );
			TestUtilities.checkResult("FieldEntry.name()",fe7.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", fe7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.dateTime().day()", fe7.dateTime().day() == 7 );
			TestUtilities.checkResult("FieldEntry.dateTime().month()()",  fe7.dateTime().month()== 11 );
			TestUtilities.checkResult("FieldEntry.dateTime().year()", fe7.dateTime().year() == 1999 );
			TestUtilities.checkResult("FieldEntry.dateTime().hour()", fe7.dateTime().hour() == 01 );
			TestUtilities.checkResult("FieldEntry.dateTime().minute()", fe7.dateTime().minute() == 02 );
			TestUtilities.checkResult("FieldEntry.dateTime().second()", fe7.dateTime().second() == 03 );
			TestUtilities.checkResult("FieldEntry.dateTime().millisecond()", fe7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("FieldList with all data types - 8th entry", iter.hasNext());
			FieldEntry fe8 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",   fe8.fieldId() == TestUtilities.MY_QOS );
			TestUtilities.checkResult("FieldEntry.name()",fe8.name().equals( "MY_QOS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.QOS", fe8.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("FieldEntry.load().dataType()== DataTypes.QOS", fe8.load().dataType()== DataTypes.QOS );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.qos().timeliness()", fe8.qos().timeliness() == 5656 );
			TestUtilities.checkResult("FieldEntry.qos().rate()", fe8.qos().rate() == 2345 );
			TestUtilities.checkResult("FieldEntry.qos().timelinessAsString()",  fe8.qos().timelinessAsString().equals("Timeliness: 5656" ));
			TestUtilities.checkResult("FieldEntry.qos().rateAsString()",  fe8.qos().rateAsString().equals("Rate: 2345" ));
			TestUtilities.checkResult("FieldEntry.qos().toString()" , fe8.qos().toString().equals("Timeliness: 5656/Rate: 2345"));

			TestUtilities.checkResult("FieldList with all data types - 9th entry", iter.hasNext());
			FieldEntry fe9 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",   fe9.fieldId() == TestUtilities.MY_STATE );
			TestUtilities.checkResult("FieldEntry.name()",fe9.name().equals( "MY_STATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.STATE", fe9.loadType() == DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.STATE", fe9.load().dataType()== DataTypes.STATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe9.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.state().StreamState()", fe9.state().streamState() == OmmState.StreamState.OPEN );
			TestUtilities.checkResult("FieldEntry.state().dataState()", fe9.state().dataState() ==OmmState.DataState.OK );
			TestUtilities.checkResult("FieldEntry.state().statusCode()", fe9.state().statusCode() == OmmState.StatusCode.NONE );
			TestUtilities.checkResult("FieldEntry.state().statusText()", fe9.state().statusText().equals( "Succeeded" ));
			TestUtilities.checkResult("FieldEntry.state().toString()", fe9.state().toString().equals("Open / Ok / None / 'Succeeded'" ));

			TestUtilities.checkResult("FieldList with all data types - 10th entry", iter.hasNext());
			FieldEntry fe10 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe10.fieldId() == 715 );
			TestUtilities.checkResult("FieldEntry.name()",fe10.name().equals( "STORY_ID"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ASCII", fe10.loadType() == DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ASCII", fe10.load().dataType()== DataTypes.ASCII );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe10.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.ascii()", fe10.ascii().ascii().equals("ABCDEF") );

			TestUtilities.checkResult("FieldList with all data types -11th entry", iter.hasNext());
			TestUtilities.checkResult("FieldList with all data types - ", iter.hasNext() );
			FieldEntry fe11= iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe11.fieldId() == 28 );
			TestUtilities.checkResult("FieldEntry.name()",fe11.name().equals( "NEWS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.RMTES", fe11.loadType() == DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.RMTES", fe11.load().dataType()== DataTypes.RMTES );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe11.code() ==Data.DataCode.NO_CODE);
			//TODO fix me
			//TestUtilities.checkResult( Arrays.equals(fe11.rmtes(), new String("ABCDEF", 6), "FieldEntry.rmtes()" );

			TestUtilities.checkResult("FieldList with all data types - 12th entry ", iter.hasNext() );
			FieldEntry fe12 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe12.fieldId() == 4 );
			TestUtilities.checkResult("FieldEntry.name()",fe12.name().equals( "RDN_EXCHID"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ENUM", fe12.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ENUM", fe12.load().dataType()== DataTypes.ENUM );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe12.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.Enum()", fe12.enumValue() == 29 );

			TestUtilities.checkResult("FieldList with all data types - 13th entry", iter.hasNext() );
			FieldEntry fe13 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe13.fieldId() == -9 );
			TestUtilities.checkResult("FieldEntry.name()",fe13.name().equals( "MY_FLOAT"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.FLOAT", fe13.loadType() == DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.FLOAT", fe13.load().dataType()== DataTypes.FLOAT );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe13.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.floatValue()", fe13.floatValue() == 11.11f );

			TestUtilities.checkResult("FieldList with all data types - 14th entry ", iter.hasNext() );
			FieldEntry fe14 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",   fe14.fieldId() == -10 );
			TestUtilities.checkResult("FieldEntry.name()",fe14.name().equals( "MY_DOUBLE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DOUBLE", fe14.loadType() == DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DOUBLE", fe14.load().dataType()== DataTypes.DOUBLE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe14.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.floatValue()", fe14.doubleValue() == 22.22f );

			TestUtilities.checkResult("FieldList with all data types - 15th entry", iter.hasNext() );
			FieldEntry fe15 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe15.fieldId() == 7 );
			TestUtilities.checkResult("FieldEntry.name()",fe15.name().equals( "TRDPRC_2"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe15.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe15.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe15.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 16th entry", iter.hasNext() );
			FieldEntry fe16 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",   fe16.fieldId() == -11 );
			TestUtilities.checkResult("FieldEntry.name()",fe16.name().equals( "MY_BUFFER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.BUFFER", fe16.loadType() == DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.BUFFER", fe16.load().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe16.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.buffer()",  Arrays.equals(fe16.buffer().buffer().array(), new String("ABCDEFGH").getBytes()));

			TestUtilities.checkResult("FieldList with all data types - 17th entry", iter.hasNext() );
			FieldEntry fe17 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe17.fieldId() == -12 );
			TestUtilities.checkResult("FieldEntry.name()",fe17.name().equals( "MY_UTF8"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UTF8", fe17.loadType() == DataTypes.UTF8 );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UTF8", fe17.load().dataType()== DataTypes.UTF8 );
			TestUtilities.checkResult( fe17.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.utf8()", Arrays.equals(fe17.utf8().buffer().array() , new String("KLMNOPQR").getBytes()));

			TestUtilities.checkResult("FieldList with all data types - 18th entry", iter.hasNext() );
			FieldEntry fe18 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" , fe18.fieldId() == -16 );
			TestUtilities.checkResult("FieldEntry.name()",fe18.name().equals( "MY_ARRAY"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ARRAY", fe18.loadType() == DataTypes.ARRAY );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ARRAY", fe18.load().dataType()== DataTypes.ARRAY );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe18.code() ==Data.DataCode.NO_CODE);
			OmmArray ar2 = fe18.array();
			Iterator<OmmArrayEntry> arrayIter = ar2.iterator();
				TestUtilities.checkResult("OmmArray within fieldlist - first hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae1 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae1.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae1.intValue() == 123 );
				TestUtilities.checkResult("OmmArray within fieldlist - second hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae2 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae2.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae2.intValue() == 234 );
				TestUtilities.checkResult("OmmArray within fieldlist - third hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae3 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae3.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae3.intValue() == 345 );
				TestUtilities.checkResult("OmmArray within fieldlist - final hasNext()",  !arrayIter.hasNext() );
				
			TestUtilities.checkResult("FieldList with all data types - 19th entry", iter.hasNext() );
			FieldEntry fe19 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" ,  fe19.fieldId() == -17 );
			TestUtilities.checkResult("FieldEntry.name()",fe19.name().equals( "MY_OPAQUE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.OpaqueEnum", fe19.loadType() == DataTypes.OPAQUE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe19.code() ==Data.DataCode.NO_CODE);
			OmmOpaque opaque2 = fe19.opaque();
			TestUtilities.checkResult( Arrays.equals(opaque2.buffer().array(), new String("OPQRST").getBytes()), "FieldEntry.opaque().buffer()" );

			TestUtilities.checkResult("FieldList with all data types - 20th entry", iter.hasNext() );
			FieldEntry fe20 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" , fe20.fieldId() == TestUtilities.MY_QOS );
			TestUtilities.checkResult("FieldEntry.name()",fe20.name().equals( "MY_QOS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.QOS", fe20.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("FieldEntry.load().dataType()== DataTypes.QOS", fe20.load().dataType()== DataTypes.QOS );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe20.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().timeliness()", fe20.qos().timeliness() == OmmQos.Timeliness.INEXACT_DELAYED );
			TestUtilities.checkResult("FieldEntry.time().rate()", fe20.qos().rate() == OmmQos.Rate.JUST_IN_TIME_CONFLATED);
			TestUtilities.checkResult("FieldEntry.qos().timelinessAsString()", fe20.qos().timelinessAsString().equals("InexactDelayed" ));
			TestUtilities.checkResult("FieldEntry.qos().rateAsString()", fe20.qos().rateAsString().equals("JustInTimeConflated" ));
			TestUtilities.checkResult("FieldEntry.qos().toString()", fe20.qos().toString().equals("InexactDelayed/JustInTimeConflated"));

			TestUtilities.checkResult("FieldList with all data types - final hasNext()", !(iter.hasNext()));

			TestUtilities.checkResult("FieldList with all data types - exception not expected", true );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( "FieldList with all data types - exception not expected" , false);
			System.out.println(excp);
		}


		flEnc.clear();
		flEnc.info( dictionary.infoDictionaryId()+1, 66 );

		try {
			//EMA Encoding
			
			//first entry (fid not found case)
			flEnc.add(EmaFactory.createFieldEntry().uintValue( -100, 64));
			
			//second entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64));
			//third  entry
			flEnc.add(EmaFactory.createFieldEntry().codeUInt( 1));

			//fourth entry
			flEnc.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			//fifth  entry
			flEnc.add(EmaFactory.createFieldEntry().codeReal( 6));

			//sixth entry
			flEnc.add(EmaFactory.createFieldEntry().intValue( -2, 32));
			//seventh  entry
			flEnc.add(EmaFactory.createFieldEntry().codeInt( -2));

			//eighth entry
			flEnc.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7));
			//ninth  entry
			flEnc.add(EmaFactory.createFieldEntry().codeDate( 16));

			//tenth entry
			flEnc.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005));
			//eleventh  entry
			flEnc.add(EmaFactory.createFieldEntry().codeTime( 18));

			//twelfth entry
			flEnc.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000));
			//thirteenth entry
			flEnc.add(EmaFactory.createFieldEntry().codeDateTime( -3));

			//fourteenth entry
			flEnc.add(EmaFactory.createFieldEntry().qos( TestUtilities.MY_QOS, OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
			//fifteenth  entry
			flEnc.add(EmaFactory.createFieldEntry().codeQos( TestUtilities.MY_QOS));

			//sixteenth entry
			flEnc.add(EmaFactory.createFieldEntry().state(TestUtilities.MY_STATE, OmmState.StreamState.OPEN,OmmState.DataState.OK, OmmState.StatusCode.NONE, "Succeeded"));
			//seventeenth  entry
			flEnc.add(EmaFactory.createFieldEntry().codeState(TestUtilities.MY_STATE));

			//eighteenth entry
			flEnc.add(EmaFactory.createFieldEntry().ascii( 715, "ABCDEF"));
			//nineteenth  entry
			flEnc.add(EmaFactory.createFieldEntry().codeAscii( 715));

			//twentieth entry
			flEnc.add(EmaFactory.createFieldEntry().rmtes( 28, ByteBuffer.wrap("ABCDEF".getBytes())));
			//21st  entry
			flEnc.add(EmaFactory.createFieldEntry().codeRmtes( 28));

			//22nd entry
			flEnc.add(EmaFactory.createFieldEntry().enumValue( 4, 29));
			//23rd  entry
			flEnc.add(EmaFactory.createFieldEntry().codeEnum( 4));

			//24th entry
			flEnc.add(EmaFactory.createFieldEntry().floatValue( -9, 11.11f));
			//25th  entry
			flEnc.add(EmaFactory.createFieldEntry().codeFloat( -9));

			//26th entry
			flEnc.add(EmaFactory.createFieldEntry().doubleValue( -10, 22.22f));
			//27th entry
			flEnc.add(EmaFactory.createFieldEntry().codeDouble( -10));

			//28th entry (blank real)
			flEnc.add(EmaFactory.createFieldEntry().codeReal( 7));

			//29th entry
			flEnc.add(EmaFactory.createFieldEntry().buffer( -11,ByteBuffer.wrap("ABCDEFGH".getBytes())));
			//30th  entry
			flEnc.add(EmaFactory.createFieldEntry().codeBuffer( -11));

			//31st entry
			flEnc.add(EmaFactory.createFieldEntry().utf8( -12,ByteBuffer.wrap("KLMNOPQR".getBytes())));
			//32nd  entry
			flEnc.add(EmaFactory.createFieldEntry().codeUtf8( -12));

			//33rd entry
			OmmArray ar1 = EmaFactory.createOmmArray();
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(123));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(234));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(345));
			flEnc.add(EmaFactory.createFieldEntry().array( -16, ar1));

			//34th entry
			OmmXml xml = EmaFactory.createOmmXml();
			xml.string("OPQRST");
			flEnc.add(EmaFactory.createFieldEntry().xml( -1 , xml));

			//Decoding
			FieldList flDec = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(flDec);

			TestUtilities.checkResult("FieldList with all data types - hasInfo()" , flDec.hasInfo());
			TestUtilities.checkResult("FieldList with all data types - infoDictionaryId()",  flDec.infoDictionaryId() == dictionary.infoDictionaryId()+1 );
			TestUtilities.checkResult("FieldList with all data types - infoFieldListNum()" ,  flDec.infoFieldListNum() == 66);

			Iterator<FieldEntry> iter = flDec.iterator();
			
			TestUtilities.checkResult("FieldList with all data types - first entry", iter.hasNext() );
			FieldEntry fe1 = iter.next();
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
			TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" ,  fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND);

			TestUtilities.checkResult("FieldList with all data types - second entry", iter.hasNext() );
			FieldEntry fe2 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );

			TestUtilities.checkResult("FieldList with all data types - third entry", iter.hasNext() );
			FieldEntry fe2b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" , fe2b.fieldId() == 1 );
			TestUtilities.checkResult( fe2b.name().equals( "PROD_PERM"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT",  fe2b.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe2b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 4th entry", iter.hasNext() );
			FieldEntry fe3 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" ,  fe3.fieldId() == 6 );
			TestUtilities.checkResult("FieldEntry.name()",fe3.name().equals( "TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
			TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );
			
			TestUtilities.checkResult("FieldList with all data types - 5th entry", iter.hasNext() );
			FieldEntry fe3b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" ,  fe3b.fieldId() == 6 );
			TestUtilities.checkResult("FieldEntry.name()",fe3b.name().equals( "TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe3b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 6th entry", iter.hasNext() );
			FieldEntry fe4 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" , fe4.fieldId() == -2 );
			TestUtilities.checkResult("FieldEntry.name()",fe4.name().equals( "INTEGER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", fe4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult( fe4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.intValue()" ,  fe4.intValue() == 32);

			TestUtilities.checkResult("FieldList with all data types - 7th entry", iter.hasNext() );
			FieldEntry fe4b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" ,  fe4b.fieldId() == -2 );
			TestUtilities.checkResult("FieldEntry.name()",fe4b.name().equals( "INTEGER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4b.loadType() == DataTypes.INT );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe4b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 8th entry", iter.hasNext() );
			FieldEntry fe5 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" ,  fe5.fieldId() == 16 );
			TestUtilities.checkResult("FieldEntry.name()",fe5.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", fe5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("FieldEntry.date().day()", fe5.date().day() == 7 );
		TestUtilities.checkResult("FieldEntry.date().month()()", fe5.date().month()== 11 );
			TestUtilities.checkResult("FieldEntry.date().year()", fe5.date().year() == 1999 );

			TestUtilities.checkResult("FieldList with all data types - 9th entry", iter.hasNext() );
			FieldEntry fe5b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" , fe5b.fieldId() == 16 );
			TestUtilities.checkResult("FieldEntry.name()",fe5b.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5b.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe5b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 10th entry", iter.hasNext() );
			FieldEntry fe6 = iter.next();
			TestUtilities.checkResult( fe6.fieldId() == 18 );
			TestUtilities.checkResult("FieldEntry.name()",fe6.name().equals( "TRDTIM_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().hour()", fe6.time().hour() == 02 );
			TestUtilities.checkResult("FieldEntry.time().minute()", fe6.time().minute() == 03 );
			TestUtilities.checkResult("FieldEntry.time().second()", fe6.time().second() == 04 );
			TestUtilities.checkResult("FieldEntry.time().millisecond()", fe6.time().millisecond() == 005 );

			TestUtilities.checkResult("FieldList with all data types - 11th entry", iter.hasNext() );
			FieldEntry fe6b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe6b.fieldId() == 18 );
			TestUtilities.checkResult("FieldEntry.name()",fe6b.name().equals( "TRDTIM_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6b.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe6b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 12th entry", iter.hasNext() );
			FieldEntry fe7 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe7.fieldId() == -3 );
			TestUtilities.checkResult("FieldEntry.name()",fe7.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", fe7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.dateTime().day()", fe7.dateTime().day() == 7 );
			TestUtilities.checkResult("FieldEntry.dateTime().month()()",  fe7.dateTime().month()== 11 );
			TestUtilities.checkResult("FieldEntry.dateTime().year()", fe7.dateTime().year() == 1999 );
			TestUtilities.checkResult("FieldEntry.dateTime().hour()", fe7.dateTime().hour() == 01 );
			TestUtilities.checkResult("FieldEntry.dateTime().minute()", fe7.dateTime().minute() == 02 );
			TestUtilities.checkResult("FieldEntry.dateTime().second()", fe7.dateTime().second() == 03 );
			TestUtilities.checkResult("FieldEntry.dateTime().millisecond()", fe7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("FieldList with all data types - 13th entry", iter.hasNext() );
			FieldEntry fe7b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe7b.fieldId() == -3 );
			TestUtilities.checkResult("FieldEntry.name()",fe7b.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7b.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe7b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 14th entry", iter.hasNext() );
			FieldEntry fe8 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe8.fieldId() == TestUtilities.MY_QOS );
			TestUtilities.checkResult("FieldEntry.name()",fe8.name().equals( "MY_QOS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.QOS", fe8.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.QOS", fe8.load().dataType()== DataTypes.QOS );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().timeliness()", fe8.qos().timeliness() == OmmQos.Timeliness.REALTIME );
			TestUtilities.checkResult("FieldEntry.time().rate()", fe8.qos().rate() == OmmQos.Rate.TICK_BY_TICK );
			TestUtilities.checkResult("FieldEntry.time().timelinessAsString()", fe8.qos().timelinessAsString().equals("RealTime" ));
			TestUtilities.checkResult("FieldEntry.time().rateAsString()", fe8.qos().rateAsString().equals("TickByTick" ));
			TestUtilities.checkResult("FieldEntry.time().toString()", fe8.qos().toString().equals("RealTime/TickByTick" ));

			TestUtilities.checkResult("FieldList with all data types - 15th entry", iter.hasNext() );
			FieldEntry fe8ba = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe8ba.fieldId() == TestUtilities.MY_QOS );
			TestUtilities.checkResult("FieldEntry.name()",fe8ba.name().equals( "MY_QOS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.QOS", fe8ba.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe8ba.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 16th entry", iter.hasNext() );
			FieldEntry fe9 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe9.fieldId() == TestUtilities.MY_STATE );
			TestUtilities.checkResult("FieldEntry.name()",fe9.name().equals( "MY_STATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.STATE", fe9.loadType() == DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.STATE", fe9.load().dataType()== DataTypes.STATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe9.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.state().StreamState()", fe9.state().streamState() == OmmState.StreamState.OPEN );
			TestUtilities.checkResult("FieldEntry.state().dataState()", fe9.state().dataState() ==OmmState.DataState.OK );
			TestUtilities.checkResult("FieldEntry.state().statusCode()", fe9.state().statusCode() == OmmState.StatusCode.NONE );
			TestUtilities.checkResult("FieldEntry.state().statusText()", fe9.state().statusText().equals( "Succeeded" ));
			TestUtilities.checkResult("FieldEntry.state().toString()", fe9.state().toString().equals("Open / Ok / None / 'Succeeded'") );

			TestUtilities.checkResult("FieldList with all data types - 17th entry", iter.hasNext() );
			FieldEntry fe9b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe9b.fieldId() == TestUtilities.MY_STATE );
			TestUtilities.checkResult("FieldEntry.name()",fe9b.name().equals( "MY_STATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.STATE", fe9b.loadType() == DataTypes.STATE );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe9b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 18th entry", iter.hasNext() );
			FieldEntry fe10 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe10.fieldId() == 715 );
			TestUtilities.checkResult("FieldEntry.name()",fe10.name().equals( "STORY_ID"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ASCII", fe10.loadType() == DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ASCII", fe10.load().dataType()== DataTypes.ASCII );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe10.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.ascii()", fe10.ascii().ascii().equals( "ABCDEF" ));

			TestUtilities.checkResult("FieldList with all data types - 19th entry", iter.hasNext() );
			FieldEntry fe10b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe10b.fieldId() == 715 );
			TestUtilities.checkResult("FieldEntry.name()",fe10b.name().equals( "STORY_ID"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ASCII", fe10b.loadType() == DataTypes.ASCII );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe10b.code() == Data.DataCode.BLANK );
			
			TestUtilities.checkResult("FieldList with all data types - 20th entry", iter.hasNext() );
			FieldEntry fe11= iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe11.fieldId() == 28 );
			TestUtilities.checkResult("FieldEntry.name()",fe11.name().equals( "NEWS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.RMTES", fe11.loadType() == DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.RMTES", fe11.load().dataType()== DataTypes.RMTES );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe11.code() ==Data.DataCode.NO_CODE);
		//TODO
			//	TestUtilities.checkResult( Arrays.equals(fe11.rmtes().AsUTF8() , new String("ABCDEF", 6), "FieldEntry.rmtes()" );

			TestUtilities.checkResult("FieldList with all data types - 21th entry", iter.hasNext() );
			FieldEntry fe11b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe11b.fieldId() == 28 );
			TestUtilities.checkResult("FieldEntry.name()",fe11b.name().equals( "NEWS"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.RMTES", fe11b.loadType() == DataTypes.RMTES );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe11b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 22th entry", iter.hasNext() );
			FieldEntry fe12 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe12.fieldId() == 4 );
			TestUtilities.checkResult( fe12.name().equals( "RDN_EXCHID"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ENUM", fe12.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ENUM", fe12.load().dataType()== DataTypes.ENUM );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe12.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.Enum()", fe12.enumValue() == 29 );

			TestUtilities.checkResult("FieldList with all data types - 23th entry", iter.hasNext() );
			FieldEntry fe12b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe12b.fieldId() == 4 );
			TestUtilities.checkResult("FieldEntry.name()",fe12b.name().equals( "RDN_EXCHID"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ENUM", fe12b.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe12b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 24th entry", iter.hasNext() );
			FieldEntry fe13 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe13.fieldId() == -9 );
			TestUtilities.checkResult("FieldEntry.name()",fe13.name().equals( "MY_FLOAT"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.FLOAT", fe13.loadType() == DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.FLOAT", fe13.load().dataType()== DataTypes.FLOAT );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe13.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.floatValue()", fe13.floatValue() == 11.11f );

			TestUtilities.checkResult("FieldList with all data types - 25th entry", iter.hasNext() );
			FieldEntry fe13b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe13b.fieldId() == -9 );
			TestUtilities.checkResult("FieldEntry.name()",fe13b.name().equals( "MY_FLOAT"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.FLOAT", fe13b.loadType() == DataTypes.FLOAT );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe13b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 26th entry", iter.hasNext() );
			FieldEntry fe14 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe14.fieldId() == -10 );
			TestUtilities.checkResult("FieldEntry.name()",fe14.name().equals( "MY_DOUBLE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DOUBLE", fe14.loadType() == DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DOUBLE", fe14.load().dataType()== DataTypes.DOUBLE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe14.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.floatValue()", fe14.doubleValue() == 22.22f );

			TestUtilities.checkResult("FieldList with all data types - 27th entry", iter.hasNext() );
			FieldEntry fe14b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe14b.fieldId() == -10 );
			TestUtilities.checkResult("FieldEntry.name()",fe14b.name().equals( "MY_DOUBLE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DOUBLE", fe14b.loadType() == DataTypes.DOUBLE );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe14b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 28th entry", iter.hasNext() );
			FieldEntry fe15 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe15.fieldId() == 7 );
			TestUtilities.checkResult("FieldEntry.name()",fe15.name().equals( "TRDPRC_2"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe15.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe15.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe15.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 29th entry", iter.hasNext() );
			FieldEntry fe16 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe16.fieldId() == -11 );
			TestUtilities.checkResult("FieldEntry.name()",fe16.name().equals( "MY_BUFFER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.BUFFER", fe16.loadType() == DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.BUFFER", fe16.load().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe16.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.buffer()", Arrays.equals(fe16.buffer().buffer().array() , new String("ABCDEFGH").getBytes()) );

			TestUtilities.checkResult("FieldList with all data types - 30th entry", iter.hasNext() );
			FieldEntry fe16b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe16b.fieldId() == -11 );
			TestUtilities.checkResult("FieldEntry.name()",fe16b.name().equals( "MY_BUFFER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.BUFFER", fe16b.loadType() == DataTypes.BUFFER );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe16b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 31th entry", iter.hasNext() );
			FieldEntry fe17 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe17.fieldId() == -12 );
			TestUtilities.checkResult("FieldEntry.name()",fe17.name().equals( "MY_UTF8"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UTF8", fe17.loadType() == DataTypes.UTF8 );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UTF8", fe17.load().dataType()== DataTypes.UTF8 );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe17.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.utf8()",  Arrays.equals(fe17.utf8().buffer().array() , new String("KLMNOPQR").getBytes() ));

			TestUtilities.checkResult("FieldList with all data types - 32th entry", iter.hasNext() );
			FieldEntry fe17b = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe17b.fieldId() == -12 );
			TestUtilities.checkResult("FieldEntry.name()",fe17b.name().equals( "MY_UTF8"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UTF8", fe17b.loadType() == DataTypes.UTF8 );
			TestUtilities.checkResult("FieldEntry.code() == Data.DataCode.BLANK", fe17b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("FieldList with all data types - 33th entry", iter.hasNext() );
			FieldEntry fe18 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe18.fieldId() == -16 );
			TestUtilities.checkResult("FieldEntry.name()",  fe18.name().equals( "MY_ARRAY"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ARRAY", fe18.loadType() == DataTypes.ARRAY );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ARRAY", fe18.load().dataType()== DataTypes.ARRAY );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe18.code() ==Data.DataCode.NO_CODE);
			OmmArray ar2 = fe18.array();
			Iterator<OmmArrayEntry> arrayIter = ar2.iterator();
				TestUtilities.checkResult("OmmArray within fieldlist - first hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae1 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae1.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae1.intValue() == 123 );
				TestUtilities.checkResult("OmmArray within fieldlist - second hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae2 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae2.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae2.intValue() == 234 );
				TestUtilities.checkResult("OmmArray within fieldlist - third hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae3 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae3.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae3.intValue() == 345 );
				TestUtilities.checkResult("OmmArray within fieldlist - final hasNext()",  !arrayIter.hasNext() );
				
			TestUtilities.checkResult("FieldList with all data types - 34th entry", iter.hasNext() );
			FieldEntry fe19 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()" ,  fe19.fieldId() == -1 );
			TestUtilities.checkResult("FieldEntry.name()",fe19.name().equals( "XML"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.XML", fe19.loadType() == DataTypes.XML);
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe19.code() ==Data.DataCode.NO_CODE);
			OmmXml xml2 = fe19.xml();
			TestUtilities.checkResult( xml2.string().equals("OPQRST"), "FieldEntry.xml().string()" );
				
			TestUtilities.checkResult("FieldList after clear() - final hasNext()",  !(iter.hasNext()) );

			TestUtilities.checkResult("FieldList with all data types - exception not expected" , true );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( "Encode FieldList after clear() - exception not expected" , false);
			System.out.println(excp);
		}
	}
	
	public void testFieldList_EncodeEMA_DecodeEMA_ContainsFieldList_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testFieldList_EncodeEMA_DecodeEMA_ContainsFieldList_EncodeDecodeAll", "");

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);

		FieldList flEnc = EmaFactory.createFieldList();
		flEnc.info( dictionary.infoDictionaryId(), 65 );

		try { 
			//EMA Encoding
			// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, FieldList, UINT

			//first entry (fid not found case)
			flEnc.add(EmaFactory.createFieldEntry().uintValue( -100, 64 ));

			//second entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64 ));

			//third entry
			flEnc.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2 ));

			//fourth entry
			flEnc.add(EmaFactory.createFieldEntry().intValue( -2, 32 ));

			//fifth entry
			flEnc.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7 ));

			//sixth entry
			flEnc.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005 ));

			//seventh entry
			flEnc.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000 ));

			//eightth entry (nested FieldList)
			FieldList flEnc1= EmaFactory.createFieldList();
			flEnc1.add(EmaFactory.createFieldEntry().uintValue( 1, 641));
			flEnc.add(EmaFactory.createFieldEntry().fieldList( -13, flEnc1 ));

			//ninth entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 642 ));
			

			//Now do EMA decoding of FieldList
			FieldList flDec = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(flDec);

			TestUtilities.checkResult("FieldList with primitives and FieldList - hasInfo()" , flDec.hasInfo() );
			TestUtilities.checkResult("FieldList with primitives and FieldList - infoDictionaryId()",  flDec.infoDictionaryId() == dictionary.infoDictionaryId());
			TestUtilities.checkResult("FieldList with primitives and FieldList - infoFieldListNum()" ,  flDec.infoFieldListNum() == 65);

			Iterator<FieldEntry> iter = flDec.iterator();
			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe1 = iter.next();
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
			TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" , fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND);

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe2 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe3 = iter.next();
			TestUtilities.checkResult( fe3.fieldId() == 6 );
			TestUtilities.checkResult( fe3.name().equals( "TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
			TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe4 = iter.next();
			TestUtilities.checkResult( fe4.fieldId() == -2 );
			TestUtilities.checkResult( fe4.name().equals( "INTEGER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", fe4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.intValue()" ,  fe4.intValue() == 32);

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe5 = iter.next();
			TestUtilities.checkResult( fe5.fieldId() == 16 );
			TestUtilities.checkResult("FieldEntry.name()",fe5.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", fe5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("FieldEntry.date().day()", fe5.date().day() == 7 );
		TestUtilities.checkResult("FieldEntry.date().month()()", fe5.date().month()== 11 );
			TestUtilities.checkResult("FieldEntry.date().year()", fe5.date().year() == 1999 );

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe6 = iter.next();
			TestUtilities.checkResult( fe6.fieldId() == 18 );
			TestUtilities.checkResult("FieldEntry.name()",fe6.name().equals( "TRDTIM_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().hour()", fe6.time().hour() == 02 );
			TestUtilities.checkResult("FieldEntry.time().minute()", fe6.time().minute() == 03 );
			TestUtilities.checkResult("FieldEntry.time().second()", fe6.time().second() == 04 );
			TestUtilities.checkResult("FieldEntry.time().millisecond()", fe6.time().millisecond() == 005 );

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe7 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe7.fieldId() == -3 );
			TestUtilities.checkResult("FieldEntry.name()",fe7.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", fe7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.dateTime().day()", fe7.dateTime().day() == 7 );
			TestUtilities.checkResult("FieldEntry.dateTime().month()()",  fe7.dateTime().month()== 11 );
			TestUtilities.checkResult("FieldEntry.dateTime().year()", fe7.dateTime().year() == 1999 );
			TestUtilities.checkResult("FieldEntry.dateTime().hour()", fe7.dateTime().hour() == 01 );
			TestUtilities.checkResult("FieldEntry.dateTime().minute()", fe7.dateTime().minute() == 02 );
			TestUtilities.checkResult("FieldEntry.dateTime().second()", fe7.dateTime().second() == 03 );
			TestUtilities.checkResult("FieldEntry.dateTime().millisecond()", fe7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe8 = iter.next();
			TestUtilities.checkResult( fe8.fieldId() == -13 );
			TestUtilities.checkResult("FieldEntry.name()",fe8.name().equals( "MY_FIELDLIST"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.FIELD_LIST", fe8.loadType() == DataTypes.FIELD_LIST );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.FIELD_LIST", fe8.load().dataType()== DataTypes.FIELD_LIST );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.code() == DataTypes.FIELD_LIST", fe8.load().dataType()== DataTypes.FIELD_LIST );
			{
				FieldList nestedFl = fe8.fieldList();

				TestUtilities.checkResult("FieldEntry FieldList within fieldlist - hasInfo()",  !nestedFl.hasInfo() );

				Iterator<FieldEntry> nestedIter = nestedFl.iterator();
				TestUtilities.checkResult("FieldEntry FieldList within fieldlist - first fieldlist hasNext()" , nestedIter.hasNext());
				FieldEntry nfe1 = nestedIter.next();
				TestUtilities.checkResult("FieldEntry.name()",nfe1.fieldId() == 1 );
				TestUtilities.checkResult( nfe1.name().equals( "PROD_PERM"));
				TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", nfe1.loadType() == DataTypes.UINT);
				TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", nfe1.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("FieldEntry.uintValue()", nfe1.uintValue() == 641 );

				TestUtilities.checkResult("FieldEntry FieldList within fieldlist - second fieldlist hasNext()", !nestedIter.hasNext());
			}

			TestUtilities.checkResult("FieldList with primitives and FieldList - first entry", iter.hasNext());
			FieldEntry fe9 = iter.next();
			TestUtilities.checkResult( fe9.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()",fe9.name().equals( "PROD_PERM"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe9.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe9.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe9.uintValue() == 642 );


			TestUtilities.checkResult("FieldList with primitives and FieldList - tenth hasNext()", !iter.hasNext());

			TestUtilities.checkResult("FieldList with primitives and FieldList - exception not expected", true);
			
			dictionary = null;

		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "FieldList with primitives and FieldList - exception not expected" , false);
			System.out.println(excp);
		}
	}
	
	public void testFieldList_EncodeEMA_DecodeEMA_ContainsElementList_EncodeDecodeAll()
	{
		
		TestUtilities.printTestHead("testFieldList_EncodeEMA_DecodeEMA_DecodeAll", "Encode FieldList with EMA and Decode FieldList with EMA");

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		FieldList flEnc= EmaFactory.createFieldList();
		flEnc.info( dictionary.infoDictionaryId(), 65 );

		try { 
			//EMA Encoding
			// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

			//first entry (fid not found case)
			flEnc.add(EmaFactory.createFieldEntry().uintValue( -100, 64));

			//second entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64));

			//third entry
			flEnc.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));

			//fourth entry
			flEnc.add(EmaFactory.createFieldEntry().intValue( -2, 32));

			//fifth entry
			flEnc.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7));

			//sixth entry
			flEnc.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005));

			//seventh entry
			flEnc.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000));

			//eightth entry (nested ElementList)
			ElementList elEnc= EmaFactory.createElementList();;
			elEnc.info( 5);
			elEnc.add(EmaFactory.createElementEntry().uintValue("Element - UInt", 641));
			flEnc.add(EmaFactory.createFieldEntry().elementList( -15, elEnc));

			//ninth entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 642));
			

			//Now do EMA decoding of FieldList
			FieldList flDec = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(flDec);

			TestUtilities.checkResult("FieldList with primitives and FieldList - hasInfo()" , flDec.hasInfo() );
			TestUtilities.checkResult("FieldList with primitives and FieldList - infoDictionaryId()",  flDec.infoDictionaryId() == dictionary.infoDictionaryId() );
			TestUtilities.checkResult("FieldList with primitives and FieldList - infoFieldListNum()" ,  flDec.infoFieldListNum() == 65);

			Iterator<FieldEntry> iter = flDec.iterator();
			
			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe1 = iter.next();
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
			TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" , fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe2 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe3 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe3.fieldId() == 6 );
			TestUtilities.checkResult( fe3.name().equals( "TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
			TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe4 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe4.fieldId() == -2 );
			TestUtilities.checkResult( fe4.name().equals( "INTEGER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", fe4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.intValue()" ,  fe4.intValue() == 32);

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe5 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe5.fieldId() == 16 );
			TestUtilities.checkResult( fe5.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", fe5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("FieldEntry.date().day()", fe5.date().day() == 7 );
		TestUtilities.checkResult("FieldEntry.date().month()()", fe5.date().month()== 11 );
			TestUtilities.checkResult("FieldEntry.date().year()", fe5.date().year() == 1999 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe6 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe6.fieldId() == 18 );
			TestUtilities.checkResult( fe6.name().equals( "TRDTIM_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().hour()", fe6.time().hour() == 02 );
			TestUtilities.checkResult("FieldEntry.time().minute()", fe6.time().minute() == 03 );
			TestUtilities.checkResult("FieldEntry.time().second()", fe6.time().second() == 04 );
			TestUtilities.checkResult("FieldEntry.time().millisecond()", fe6.time().millisecond() == 005 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe7 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe7.fieldId() == -3 );
			TestUtilities.checkResult( fe7.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", fe7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.dateTime().day()", fe7.dateTime().day() == 7 );
			TestUtilities.checkResult("FieldEntry.dateTime().month()()",  fe7.dateTime().month()== 11 );
			TestUtilities.checkResult("FieldEntry.dateTime().year()", fe7.dateTime().year() == 1999 );
			TestUtilities.checkResult("FieldEntry.dateTime().hour()", fe7.dateTime().hour() == 01 );
			TestUtilities.checkResult("FieldEntry.dateTime().minute()", fe7.dateTime().minute() == 02 );
			TestUtilities.checkResult("FieldEntry.dateTime().second()", fe7.dateTime().second() == 03 );
			TestUtilities.checkResult("FieldEntry.dateTime().millisecond()", fe7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe8 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe8.fieldId() == -15 );
			TestUtilities.checkResult( fe8.name().equals( "MY_ELEMENTLIST"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ELEMENT_LIST", fe8.loadType() == DataTypes.ELEMENT_LIST );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ELEMENT_LIST", fe8.load().dataType()== DataTypes.ELEMENT_LIST );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.code() == DataTypes.ELEMENT_LIST", fe8.load().dataType()== DataTypes.ELEMENT_LIST );
			{
				ElementList nestedEl = fe8.elementList();

				Iterator<ElementEntry> nestedIter = nestedEl.iterator();
				TestUtilities.checkResult("FieldEntry ElementList within fieldlist - hasInfo()",  nestedEl.hasInfo() );

				TestUtilities.checkResult("ElementEntry ElementList within fieldlist - first elementlist hasNext()", nestedIter.hasNext());
				ElementEntry ee1 = nestedIter.next();
				TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "Element - UInt" ));
				TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee1.loadType() == DataTypes.UINT );
				TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee1.code() ==Data.DataCode.NO_CODE );
				TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 641 );

				TestUtilities.checkResult("ElementEntry ElementList within fieldlist - second elementlist hasNext()", !nestedIter.hasNext());
			}

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe9 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe9.fieldId() == 1 );
			TestUtilities.checkResult( fe9.name().equals( "PROD_PERM"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe9.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe9.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe9.uintValue() == 642 );


			TestUtilities.checkResult("FieldList with primitives and ElementList - tenth hasNext()", !iter.hasNext());

			TestUtilities.checkResult("FieldList with primitives and ElementList - exception not expected", true);
			
			dictionary = null;

		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "FieldList with primitives and ElementList - exception not expected" , false);
			System.out.println(excp);
		}

	}
	
	public void testFieldList_EncodeEMA_DecodeEMA_Efficient_EncodeDecodeAll()
	{
		
		TestUtilities.printTestHead("testFieldList_EncodeEMA_DecodeEMA_Efficient_DecodeAll", "Encode FieldList with EMA and Decode FieldList with EMA using Efficient methods");

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		FieldList flEnc= EmaFactory.createFieldList();
		flEnc.info( dictionary.infoDictionaryId(), 65 );

		try { 
			//EMA Encoding
			// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

			//first entry (fid not found case)
			flEnc.add(EmaFactory.createFieldEntry().uintValue( -100, 64));

			//second entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64));

			//third entry
			flEnc.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));

			//fourth entry
			flEnc.add(EmaFactory.createFieldEntry().intValue( -2, 32));

			//fifth entry
			flEnc.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7));

			//sixth entry
			flEnc.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005));

			//seventh entry
			flEnc.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000));

			//Now do EMA decoding of FieldList
			FieldList flDec = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(flDec);

			TestUtilities.checkResult("FieldList with primitives and FieldList - hasInfo()" , flDec.hasInfo() );
			TestUtilities.checkResult("FieldList with primitives and FieldList - infoDictionaryId()",  flDec.infoDictionaryId() == dictionary.infoDictionaryId() );
			TestUtilities.checkResult("FieldList with primitives and FieldList - infoFieldListNum()" ,  flDec.infoFieldListNum() == 65);

			Iterator<FieldEntry> iter = flDec.iteratorByRef();
			
			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe1 = iter.next();
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
			TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" , fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe2 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe3 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe3.fieldId() == 6 );
			TestUtilities.checkResult( fe3.name().equals( "TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
			TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe4 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe4.fieldId() == -2 );
			TestUtilities.checkResult( fe4.name().equals( "INTEGER"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", fe4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.intValue()" ,  fe4.intValue() == 32);

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe5 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe5.fieldId() == 16 );
			TestUtilities.checkResult( fe5.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", fe5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("FieldEntry.date().day()", fe5.date().day() == 7 );
		TestUtilities.checkResult("FieldEntry.date().month()()", fe5.date().month()== 11 );
			TestUtilities.checkResult("FieldEntry.date().year()", fe5.date().year() == 1999 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe6 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe6.fieldId() == 18 );
			TestUtilities.checkResult( fe6.name().equals( "TRDTIM_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.time().hour()", fe6.time().hour() == 02 );
			TestUtilities.checkResult("FieldEntry.time().minute()", fe6.time().minute() == 03 );
			TestUtilities.checkResult("FieldEntry.time().second()", fe6.time().second() == 04 );
			TestUtilities.checkResult("FieldEntry.time().millisecond()", fe6.time().millisecond() == 005 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - first entry", iter.hasNext());
			FieldEntry fe7 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe7.fieldId() == -3 );
			TestUtilities.checkResult( fe7.name().equals( "TRADE_DATE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", fe7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.dateTime().day()", fe7.dateTime().day() == 7 );
			TestUtilities.checkResult("FieldEntry.dateTime().month()()",  fe7.dateTime().month()== 11 );
			TestUtilities.checkResult("FieldEntry.dateTime().year()", fe7.dateTime().year() == 1999 );
			TestUtilities.checkResult("FieldEntry.dateTime().hour()", fe7.dateTime().hour() == 01 );
			TestUtilities.checkResult("FieldEntry.dateTime().minute()", fe7.dateTime().minute() == 02 );
			TestUtilities.checkResult("FieldEntry.dateTime().second()", fe7.dateTime().second() == 03 );
			TestUtilities.checkResult("FieldEntry.dateTime().millisecond()", fe7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("FieldList with primitives and ElementList - eigth hasNext()", !iter.hasNext());

			TestUtilities.checkResult("FieldList with primitives and ElementList - exception not expected", true);
			
			dictionary = null;

		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "FieldList with primitives and ElementList - exception not expected" , false);
			System.out.println(excp);
		}

	}
	
	public void testFieldList_EncodeEMA_DecodeEMA_ContainsMap_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testFieldList_EncodeEMA_DecodeEMA_DecodeAll", "Encode FieldList with EMA and Decode FieldList with EMA");

		// load dictionary
		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
	FieldList flEnc = EmaFactory.createFieldList();
	flEnc.info( dictionary.infoDictionaryId(), 65 );

	try { 
		//EMA Encoding
		// encoding order:  ERROR, UINT, REAL, INT, DATE, TIME, DATETIME, Map, UINT

		//first entry (fid not found case)
		flEnc.add(EmaFactory.createFieldEntry().uintValue( -100, 64));
		
		

		//second entry
		flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64));

		//third entry
		flEnc.add(EmaFactory.createFieldEntry().real( 6, 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));

		//fourth entry
		flEnc.add(EmaFactory.createFieldEntry().intValue( -2, 32));

		//fifth entry
		flEnc.add(EmaFactory.createFieldEntry().date( 16, 1999, 11, 7));

		//sixth entry
		flEnc.add(EmaFactory.createFieldEntry().time( 18, 02, 03, 04, 005));

		//seventh entry
		flEnc.add(EmaFactory.createFieldEntry().dateTime( -3, 1999, 11, 7, 01, 02, 03, 000));

		//eightth entry (nested Map)
		Map mapEnc1 = EmaFactory.createMap();
		TestUtilities.EmaEncodeMapAllWithFieldList( mapEnc1);
		flEnc.add(EmaFactory.createFieldEntry().map( -14, mapEnc1));

		//ninth entry
		flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 642));
		

		//Now do EMA decoding of FieldList
		FieldList flDec = JUnitTestConnect.createFieldList();
		JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		System.out.println(flDec);

		Iterator<FieldEntry> iter = flDec.iterator();

		TestUtilities.checkResult("FieldList with primitives and Map - hasInfo()" , flDec.hasInfo() );
		TestUtilities.checkResult("FieldList with primitives and Map - infoDictionaryId()",  flDec.infoDictionaryId() == dictionary.infoDictionaryId() );
		TestUtilities.checkResult("FieldList with primitives and Map - infoFieldListNum()" ,  flDec.infoFieldListNum() == 65);

		TestUtilities.checkResult("FieldList with primitives and Map - first entry", iter.hasNext());
		FieldEntry fe1 = iter.next();
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );
		TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" , fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND);

		TestUtilities.checkResult("FieldList with primitives and Map - second entry", iter.hasNext());
		FieldEntry fe2 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
		TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT);
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
		TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );

		TestUtilities.checkResult("FieldList with primitives and Map - third entry", iter.hasNext());
		FieldEntry fe3 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()",  fe3.fieldId() == 6 );
		TestUtilities.checkResult( fe3.name().equals( "TRDPRC_1"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe3.loadType() == DataTypes.REAL );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe3.load().dataType()== DataTypes.REAL );
		TestUtilities.checkResult("FieldEntry.real().mantissa()", fe3.real().mantissa() == 11 );
		TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe3.real().magnitudeType() == 12 );

		TestUtilities.checkResult("FieldList with primitives and Map - fourth entry", iter.hasNext());
		FieldEntry fe4 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()", fe4.fieldId() == -2 );
		TestUtilities.checkResult( fe4.name().equals( "INTEGER"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.INT", fe4.loadType() == DataTypes.INT );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", fe4.load().dataType()== DataTypes.INT );
		TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe4.code() ==Data.DataCode.NO_CODE);
		TestUtilities.checkResult("FieldEntry.intValue()" ,  fe4.intValue() == 32);

		TestUtilities.checkResult("FieldList with primitives and Map - fifth entry", iter.hasNext());
		FieldEntry fe5 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()",  fe5.fieldId() == 16 );
		TestUtilities.checkResult("FieldEntry.name()", fe5.name().equals( "TRADE_DATE"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATE", fe5.loadType() == DataTypes.DATE );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", fe5.load().dataType()== DataTypes.DATE );
		TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe5.code() ==Data.DataCode.NO_CODE);
		TestUtilities.checkResult("FieldEntry.date().day()", fe5.date().day() == 7 );
	TestUtilities.checkResult("FieldEntry.date().month()()", fe5.date().month()== 11 );
		TestUtilities.checkResult("FieldEntry.date().year()", fe5.date().year() == 1999 );

		TestUtilities.checkResult("FieldList with primitives and Map - sixth entry", iter.hasNext());
		FieldEntry fe6 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()", fe6.fieldId() == 18 );
		TestUtilities.checkResult( fe6.name().equals( "TRDTIM_1"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME", fe6.loadType() == DataTypes.TIME );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe6.load().dataType()== DataTypes.TIME );
		TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe6.code() ==Data.DataCode.NO_CODE);
		TestUtilities.checkResult("FieldEntry.time().hour()", fe6.time().hour() == 02 );
		TestUtilities.checkResult("FieldEntry.time().minute()", fe6.time().minute() == 03 );
		TestUtilities.checkResult("FieldEntry.time().second()", fe6.time().second() == 04 );
		TestUtilities.checkResult("FieldEntry.time().millisecond()", fe6.time().millisecond() == 005 );

		TestUtilities.checkResult("FieldList with primitives and Map - seventh entry", iter.hasNext());
		FieldEntry fe7 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()", fe7.fieldId() == -3 );
		TestUtilities.checkResult("FieldEntry.name()",  fe7.name().equals( "TRADE_DATE"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.DATETIME", fe7.loadType() == DataTypes.DATETIME );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", fe7.load().dataType()== DataTypes.DATETIME );
		TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe7.code() ==Data.DataCode.NO_CODE);
		TestUtilities.checkResult("FieldEntry.dateTime().day()", fe7.dateTime().day() == 7 );
		TestUtilities.checkResult("FieldEntry.dateTime().month()()",  fe7.dateTime().month()== 11 );
		TestUtilities.checkResult("FieldEntry.dateTime().year()", fe7.dateTime().year() == 1999 );
		TestUtilities.checkResult("FieldEntry.dateTime().hour()", fe7.dateTime().hour() == 01 );
		TestUtilities.checkResult("FieldEntry.dateTime().minute()", fe7.dateTime().minute() == 02 );
		TestUtilities.checkResult("FieldEntry.dateTime().second()", fe7.dateTime().second() == 03 );
		TestUtilities.checkResult("FieldEntry.dateTime().millisecond()", fe7.dateTime().millisecond() == 000 );

		TestUtilities.checkResult("FieldList with primitives and Map - eighth entry", iter.hasNext());
		FieldEntry fe8 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()", fe8.fieldId() == -14 );
		TestUtilities.checkResult( fe8.name().equals( "MY_MAP"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.MAP", fe8.loadType() == DataTypes.MAP );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.MAP", fe8.load().dataType()== DataTypes.MAP );
		TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe8.code() ==Data.DataCode.NO_CODE);
		TestUtilities.checkResult("FieldEntry.code() == DataTypes.MAP", fe8.load().dataType()== DataTypes.MAP );
		{
			Map map = fe8.map();
			Iterator<MapEntry> mapIter = map.iterator();
			TestUtilities.checkResult("FieldEntry Map within fieldList - hasKeyFieldId()",  map.hasKeyFieldId() );
			TestUtilities.checkResult("FieldEntry Map within map - keyFieldId()" ,  map.keyFieldId() == 3426);

			TestUtilities.checkResult("FieldEntry Map within fieldList - first map hasNext()", mapIter.hasNext());
			MapEntry me1a = mapIter.next();
			TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me1a.key().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("MapEntry.key().buffer()", Arrays.equals( me1a.key().buffer().buffer().array() , new String("ABCD").getBytes()));
			TestUtilities.checkResult("MapEntry.action() == MapEntry.Action.DELETE", me1a.action() == MapEntry.MapAction.DELETE );
			TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.NO_DATA", me1a.load().dataType()== DataTypes.NO_DATA );

			TestUtilities.checkResult("FieldEntry Map within fieldList - second map hasNext()", mapIter.hasNext());
			MapEntry me2a = mapIter.next();
			TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me2a.key().dataType()== DataTypes.BUFFER );

			//me2a.key().buffer() is empty
			//..

			TestUtilities.checkResult("MapEntry.action() == MapEntry.MapAction.ADD", me2a.action() == MapEntry.MapAction.ADD );
			TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.FIELD_LIST", me2a.load().dataType()== DataTypes.FIELD_LIST );
			TestUtilities.EmaDecodeFieldListAll( me2a.fieldList() );

			TestUtilities.checkResult("FieldEntry Map within fieldList - third map hasNext()", mapIter.hasNext());
			MapEntry me3a = mapIter.next();
			TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me3a.key().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("MapEntry.key().buffer()",  Arrays.equals(me3a.key().buffer().buffer().array() , new String("EFGHI").getBytes()));
			TestUtilities.checkResult("MapEntry.action() == MapEntry.MapAction.ADD", me3a.action() == MapEntry.MapAction.ADD );
			TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.FIELD_LIST", me3a.load().dataType()== DataTypes.FIELD_LIST );
			TestUtilities.EmaDecodeFieldListAll( me3a.fieldList() );

			TestUtilities.checkResult("FieldEntry Map within fieldList - fourth map hasNext()" , mapIter.hasNext());
			MapEntry me4a = mapIter.next();
			TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me4a.key().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("MapEntry.key().buffer()" ,Arrays.equals( me4a.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()));
			TestUtilities.checkResult("MapEntry.action() == MapEntry.MapAction.UPDATE", me4a.action() == MapEntry.MapAction.UPDATE );
			TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.FIELD_LIST", me4a.load().dataType()== DataTypes.FIELD_LIST );
			TestUtilities.EmaDecodeFieldListAll( me4a.fieldList() );

			TestUtilities.checkResult("FieldEntry Map within fieldList - fifth map hasNext()", !mapIter.hasNext());
		}

		TestUtilities.checkResult("FieldList with primitives and Map - first entry", iter.hasNext());
		FieldEntry fe9 = iter.next();
		TestUtilities.checkResult("FieldEntry.fieldId()", fe9.fieldId() == 1 );
		TestUtilities.checkResult("FieldEntry.name()", fe9.name().equals( "PROD_PERM"));
		TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe9.loadType() == DataTypes.UINT);
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe9.load().dataType()== DataTypes.UINT );
		TestUtilities.checkResult("FieldEntry.uintValue()", fe9.uintValue() == 642 );

		TestUtilities.checkResult("FieldList with primitives and Map - tenth hasNext()", !iter.hasNext());

		TestUtilities.checkResult("FieldList with primitives and Map - exception not expected", true);
		
		dictionary = null;
	} catch ( OmmException excp  ) {
		TestUtilities.checkResult( "FieldList with primitives and Map - exception not expected" , false);
		System.out.println(excp);
	}

}
public void testFieldList_DecodeErrorFieldList()
{
	TestUtilities.printTestHead("testFieldList_DecodeErrorFieldList", "");
	
	try { 

		Buffer rsslBuf = CodecFactory.createBuffer();
		rsslBuf.data(ByteBuffer.allocate(1000));

		TestUtilities.eta_EncodeErrorFieldList( rsslBuf );

		com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.eta_encodeDictionaryMsg(dictionary);
		
		FieldList fl = JUnitTestConnect.createFieldList();
		JUnitTestConnect.setRsslData(fl, rsslBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		Iterator<FieldEntry> iter = fl.iterator();
		// first entry "fid not found"
		{
			TestUtilities.checkResult("FieldList.hasNext() first", iter.hasNext() );

			FieldEntry fe1 = iter.next();

			TestUtilities.checkResult("FieldEntry.fieldId() == -100", fe1.fieldId() == -100 );

			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR", fe1.loadType() == DataTypes.ERROR );

			TestUtilities.checkResult("FieldEntry.ErrorCode() == FieldIdNotFound" , fe1.error().errorCode() == OmmError.ErrorCode.FIELD_ID_NOT_FOUND);
			
			TestUtilities.checkResult("FieldEntry.hasEnumDisplay() == false", fe1.hasEnumDisplay() == false);
			
			try
			{
				fe1.enumDisplay();
				TestUtilities.checkResult(false, "Exception expected. Calling FieldEntry.enumDisplay() must throw exception as load type is Error");
			}
			catch(OmmException excp)
			{
				System.out.println(excp.getMessage());
				TestUtilities.checkResult(excp.getMessage().equals("Attempt to enumDisplay() while actual entry data type is Error")  , "Exception expected. "+
				excp.getMessage() );
			}

			TestUtilities.checkResult("FieldEntry.load().toString() = empty", !fe1.load().toString().isEmpty() );

			if ( !fe1.load().toString().isEmpty() ) System.out.println("Fid Not Found FieldEntry.load().toString() output: "  +  fe1.load().toString() );
		}

		// second entry fid found and correct
		{
			TestUtilities.checkResult("FieldList.hasNext() second",  iter.hasNext());

			FieldEntry fe2 = iter.next();

			TestUtilities.checkResult("FieldEntry.fieldId() == 1", fe2.fieldId() == 1 );

			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT",fe2.loadType() == DataTypes.UINT);

			TestUtilities.checkResult("FieldEntry.uintValue() == 64", fe2.uintValue() == 64 );
			
			System.out.println();
		}

		// third entry fid found but not corrct data type (longer)
		{
			TestUtilities.checkResult("FieldList.hasNext() third", iter.hasNext());

			FieldEntry fe3 = iter.next();

			TestUtilities.checkResult("FieldEntry.fieldId() == 1", fe3.fieldId() == 1 );

			TestUtilities.checkResult( fe3.loadType() == DataTypes.UINT);

			System.out.println("Fid Not Found FieldEntry.toString() output: "  +  fe3.toString()) ;

			if ( !fe3.load().toString().isEmpty() ) System.out.println("Fid Not Found FieldEntry.load().toString() output: "  +  fe3.load().toString()) ;
		}

		// fourth entry fid found and correct data type
		{
			TestUtilities.checkResult("FieldList.hasNext() fourth", iter.hasNext() );

			FieldEntry fe4 = iter.next();

			TestUtilities.checkResult("FieldEntry.fieldId() == 6", fe4.fieldId() == 6 );

			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe4.loadType() == DataTypes.REAL );

			TestUtilities.checkResult("FieldEntry.real()", (fe4.real().magnitudeType() ==  12  &&  fe4.real().mantissa() == 11 ));

			System.out.println();
		}

		// fifth entry fid found and not correct (shorter)
		{
			TestUtilities.checkResult("FieldList.hasNext() fifth", iter.hasNext() );

			FieldEntry fe5 = iter.next();

			TestUtilities.checkResult("FieldEntry.fieldId() == 6" , fe5.fieldId() == 6);

			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe5.loadType() == DataTypes.REAL );

			System.out.println("Fid Not Found FieldEntry.toString() output: "  +  fe5.toString()) ;

			if ( !fe5.load().toString().isEmpty() ) System.out.println("Fid Not Found FieldEntry.load().toString() output: "  +  fe5.load().toString());
		}

		// sixth entry fid found and correct data type
		{
			TestUtilities.checkResult("FieldList.hasNext() sixth", iter.hasNext() );

			FieldEntry fe4 = iter.next();

			TestUtilities.checkResult("FieldEntry.fieldId() == 6", fe4.fieldId() == 6 );

			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe4.loadType() == DataTypes.REAL );

			TestUtilities.checkResult("FieldEntry.real()" ,  fe4.real().magnitudeType() ==  12 && fe4.real().mantissa() == 11);
			
			System.out.println();
		}

		TestUtilities.checkResult("FieldList.hasNext() seventh", !iter.hasNext() );

		TestUtilities.checkResult("Error FieldList decoding - exception not expected", true);

		dictionary = null;
		
	}
	catch ( OmmException excp  )
	{
		TestUtilities.checkResult( "Error FieldList decoding - exception not expected" , false);
		System.out.println(excp);
	}
}

	public void testFieldList_FieldListDecode_toString()
	{
		TestUtilities.printTestHead("testFieldList_FieldListDecode_toString", "Display an encoded FieldList by calling toString()");

		try { 
	
			// Create a ETA Buffer to encode into
			com.refinitiv.eta.codec.Buffer buf = com.refinitiv.eta.codec.CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1024));
	
			// load dictionary
			com.refinitiv.eta.codec.DataDictionary dictionary = com.refinitiv.eta.codec.CodecFactory
					.createDataDictionary();
			TestUtilities.eta_encodeDictionaryMsg(dictionary);
	
			// Encode FieldList with ETA.
			int retVal;
			System.out.println("Begin ETA FieldList Encoding");
			if ((retVal = TestUtilities.eta_EncodeFieldListAll(buf, EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("Error encoding field list.");
				System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
						+ ") encountered with TestUtilities.eta_EncodeFieldListAll.  " + "Error Text: "
						+ CodecReturnCodes.info(retVal));
				return;
			}
			System.out.println("End ETA FieldList Encoding");
			System.out.println();
			
			com.refinitiv.ema.access.FieldList fl = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(fl, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
	
			System.out.println(fl);
	
			buf = null;
			TestUtilities.checkResult("Fieldlist toString Decode - exception not expected", true);
			dictionary = null;
	
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "Fieldlist toString Decode - exception not expected",false);
			System.out.println(excp);
		}
	}

	public void testFieldList_InvalidDate()
	{
		TestUtilities.printTestHead("testFieldList_InvalidDate", "encode invalid Time");
		FieldEntry fieldEntry = EmaFactory.createFieldEntry();
		
		try {
			fieldEntry.time( 267, 10,11,22,1000,900,900 );
			TestUtilities.checkResult( "FieldList set invalid date - exception expected",false);
		}
		catch (OmmException excp ) {
			TestUtilities.checkResult("FieldList set invalid date - exception expected : "+ excp.getMessage(), true );
		}
	}
	
	public void testFieldList_EncodeEMA_DecodeEMA_RippleTo_RippleToName()
	{
		TestUtilities.printTestHead("testFieldList_EncodeEMA_DecodeEMA_RippleTo_RippleToName", "Encode FieldList with EMA for ripple fields and Decode FieldList with EMA");
		
		FieldList flEnc = EmaFactory.createFieldList();
		flEnc.info( TestUtilities.getDataDictionary().infoDictionaryId(), 65 );	

		try {
			// Get ripple to fields from creating FieldEntry
			TestUtilities.checkResult("FieldEntry.rippleTo()",EmaFactory.createFieldEntry().rippleTo() == 0 );
			TestUtilities.checkResult("FieldEntry.rippleTo()",EmaFactory.createFieldEntry().rippleToName(5).equals(""));
			
			//EMA Encoding for ripple fields
			
			//first entry (fid not found case)
			flEnc.add(EmaFactory.createFieldEntry().real( 6, 5236, OmmReal.MagnitudeType.EXPONENT_NEG_2)); // TRDPRC_1
			
			//second entry
			flEnc.add(EmaFactory.createFieldEntry().uintValue( 1, 64)); // PROD_PERM
			
			//third entry
			flEnc.add(EmaFactory.createFieldEntry().time(286, 23, 59,60) ); // HIGH_TIME
			
			//fourth entry
			flEnc.add(EmaFactory.createFieldEntry().enumValue(270, 5)); // ACT_TP_1
			
			//Now do EMA decoding of FieldList with out dictionary
			{
				FieldList flDecNoDict = JUnitTestConnect.createFieldList();
				JUnitTestConnect.setRsslData(flDecNoDict, flEnc, Codec.majorVersion(), Codec.minorVersion(), null, null);
				Iterator<FieldEntry> iter = flDecNoDict.iterator();
				TestUtilities.checkResult("FieldList with primitives - first entry", iter.hasNext());
				FieldEntry fe1 = iter.next();
				TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ERROR",  fe1.loadType() == DataTypes.ERROR );
				TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ERROR", fe1.load().dataType()== DataTypes.ERROR );
				TestUtilities.checkResult("FieldEntry.rippleTo()", fe1.rippleTo() == 0 );
				TestUtilities.checkResult("FieldEntry.rippleToName()", fe1.rippleToName().equals(""));
			}
				
			//Now do EMA decoding of FieldList
			FieldList flDec = JUnitTestConnect.createFieldList();
			JUnitTestConnect.setRsslData(flDec, flEnc, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
	
			Iterator<FieldEntry> iter = flDec.iterator();
	
			TestUtilities.checkResult("FieldList with primitives - hasInfo()" , flDec.hasInfo() );
			TestUtilities.checkResult("FieldList with primitives - infoDictionaryId()",  flDec.infoDictionaryId() == TestUtilities.getDataDictionary().infoDictionaryId() );
			TestUtilities.checkResult("FieldList with primitives - infoFieldListNum()" ,  flDec.infoFieldListNum() == 65);
	
			TestUtilities.checkResult("FieldList with primitives - first entry", iter.hasNext());
			FieldEntry fe1 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe1.fieldId() == 6 );
			TestUtilities.checkResult( fe1.name().equals( "TRDPRC_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.REAL",  fe1.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", fe1.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("FieldEntry.real().mantissa()", fe1.real().mantissa() == 5236 );
			TestUtilities.checkResult("FieldEntry.real().magnitudeType()", fe1.real().magnitudeType() == OmmReal.MagnitudeType.EXPONENT_NEG_2 );
			TestUtilities.checkResult("FieldEntry.rippleTo()", fe1.rippleTo() == 7 );
			TestUtilities.checkResult("FieldEntry.rippleToName()", fe1.rippleToName().equals("TRDPRC_2"));
	
			TestUtilities.checkResult("FieldList with primitives - second entry", iter.hasNext());
			FieldEntry fe2 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe2.fieldId() == 1 );
			TestUtilities.checkResult("FieldEntry.name()", fe2.name().equals( "PROD_PERM") );
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", fe2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", fe2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("FieldEntry.uintValue()", fe2.uintValue() == 64 );
			TestUtilities.checkResult("FieldEntry.rippleTo()", fe2.rippleTo() == 0 );
			TestUtilities.checkResult("FieldEntry.rippleToName()", fe2.rippleToName().equals(""));
	
			TestUtilities.checkResult("FieldList with primitives - third entry", iter.hasNext());
			FieldEntry fe3 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()",  fe3.fieldId() == 286 );
			TestUtilities.checkResult( fe3.name().equals( "HIGH_TIME"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.TIME",  fe3.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", fe3.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("FieldEntry.time().hour()", fe3.time().hour() == 23 );
			TestUtilities.checkResult("FieldEntry.time().minute()", fe3.time().minute() == 59 );
			TestUtilities.checkResult("FieldEntry.time().second()", fe3.time().second() == 60 );
			TestUtilities.checkResult("FieldEntry.rippleTo()", fe3.rippleTo() == 3625 );
			TestUtilities.checkResult("FieldEntry.rippleToName()", fe3.rippleToName().equals("HIGH_TIME2"));
			
	
			TestUtilities.checkResult("FieldList with primitives - fourth entry", iter.hasNext());
			FieldEntry fe4 = iter.next();
			TestUtilities.checkResult("FieldEntry.fieldId()", fe4.fieldId() == 270 );
			TestUtilities.checkResult( fe4.name().equals( "ACT_TP_1"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.ENUM", fe4.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ENUM", fe4.load().dataType()== DataTypes.ENUM );
			TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", fe4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("FieldEntry.enumValue()" ,  fe4.enumValue() == 5);
			TestUtilities.checkResult("FieldEntry.rippleTo()", fe4.rippleTo() == 271 );
			TestUtilities.checkResult("FieldEntry.rippleToName()", fe4.rippleToName().equals("ACT_TP_2"));
			
			// Searching for others ripple fields
			TestUtilities.checkResult("FieldEntry.rippleTo(int)", fe4.rippleTo(23) == 24 ); // BID_1
			TestUtilities.checkResult("FieldEntry.rippleToName(int)", fe4.rippleToName(26).equals("ASK_2") ); // ASK_1
			TestUtilities.checkResult("FieldEntry.rippleTo(int)", fe4.rippleTo(18) == 0 );  // TRDTIM_1
			TestUtilities.checkResult("FieldEntry.rippleToName(int)", fe4.rippleToName(29).equals("") ); //NEWS_TIME
	
			TestUtilities.checkResult("FieldList with primitives - fifth hasNext()", !iter.hasNext());
	
			TestUtilities.checkResult("FieldList with primitives - exception not expected", true);
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "FieldList with primitives - exception not expected" , false);
			System.out.println(excp);
		}
	}
}
