///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.unittest;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Iterator;

import com.rtsdk.eta.codec.Buffer;
import com.rtsdk.eta.codec.Codec;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.ema.access.Data;
import com.rtsdk.ema.access.DataType;
import com.rtsdk.ema.access.DataType.DataTypes;
import com.rtsdk.ema.access.ElementEntry;
import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldEntry;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.JUnitTestConnect;
import com.rtsdk.ema.access.Map;
import com.rtsdk.ema.access.MapEntry;
import com.rtsdk.ema.access.OmmArray;
import com.rtsdk.ema.access.OmmArrayEntry;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmOpaque;
import com.rtsdk.ema.access.OmmQos;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.OmmXml;
import com.rtsdk.ema.access.Series;
import com.rtsdk.ema.rdm.*;
import com.rtsdk.ema.unittest.TestUtilities.EncodingTypeFlags;

import junit.framework.TestCase;

public class ElementListTests extends TestCase
{

	public ElementListTests(String name)
	{
		super(name);
	}

	public void testElementList_ReEncodeEMAWithBigBuffer()
	{
		TestUtilities.printTestHead("testElementList_ReEncodeEMAWithBigBuffer", "test reencode in ema when return code is BUFFER_TOO_SMALL");

		// load dictionary
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

           ElementList batch = EmaFactory.createElementList();
           OmmArray array = EmaFactory.createOmmArray();
            
           for (int i = 0; i < 500; i++)     {
           	array.add(EmaFactory.createOmmArrayEntry().ascii("TEST" + i + ".BK"));
            }
     
          batch.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_BATCH_ITEM_LIST, array));
		
		// Decode ElementList and array 
          com.rtsdk.ema.access.ElementList elDec = JUnitTestConnect.createElementList();
  		JUnitTestConnect.setRsslData(elDec, batch, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

  		System.out.println(elDec);
        
		Iterator<ElementEntry> iter = elDec.iterator();
		TestUtilities.checkResult("ElementList with all datat types - size of collection = 20 ", elDec.size() == 1);
	
		TestUtilities.checkResult("ElementList with all data types- first hasNext()" , iter.hasNext());
		ElementEntry ee1 = iter.next();
		TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals(EmaRdm.ENAME_BATCH_ITEM_LIST) );
		
		TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ARRAY", ee1.loadType() == DataTypes.ARRAY );
		TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ARRAY", ee1.load().dataType()== DataTypes.ARRAY );
		TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee1.code() ==Data.DataCode.NO_CODE);
		OmmArray ar2 = ee1.array();
		TestUtilities.checkResult("OmmArray.size == 500", ar2.size() == 500);

  		System.out.println("\ntestElementList_ReEncodeEMAWithBigBuffer passed");
	}

	public void testElementList_EncodeUPA_decodeEMA_DecodeAll()
	{
		TestUtilities.printTestHead("testElementListt_EncodeUPA_decodeEMA_DecodeAll", "Encode ElementList with UPA and Decode ElementList with EMA");

		// Create a UPA Buffer to encode into
		com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
		buf.data(ByteBuffer.allocate(1024));

		// load dictionary
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		// Encode ElementList with UPA.
		System.out.println("Begin UPA ElementList Encoding");
		int retVal;
		if ((retVal = TestUtilities.upa_EncodeElementListAll(buf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
		{
			System.out.println("Error encoding element list.");
			System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
					+ ") encountered with TestUtilities.upa_EncodeElementListAll.  " + "Error Text: "
					+ CodecReturnCodes.info(retVal));
			return;
		}

		System.out.println("End UPA ElementList Encoding");
		System.out.println();

		// Decode ElementList with EMA.
		com.rtsdk.ema.access.ElementList el = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(el, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

		TestUtilities.EmaDecode_UPAElementListAll(el,TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);

		System.out.println("\ntestElementList_EncodeUPA_decodeEMA_DecodeAll passed");
	}

    public void testElementList_EncodeUPA_DecodeEMA_EncodeEMA_DecodeUPA()
    {
        TestUtilities.printTestHead("testElementList_EncodeUPA_DecodeEMA_EncodeEMA_DecodeUPA", "Encode ElementList with UPA, Decode ElementList with EMA, Encode ElementList with EMA, Decode ElementList with UPA");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(4048));

        // load dictionary
        com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                .createDataDictionary();
        TestUtilities.upa_encodeDictionaryMsg(dictionary);

        // Encode ElementList with UPA.
        int retVal;
        if ((retVal = TestUtilities.upa_EncodeElementListAll(buf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding field list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeElementListAll.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode ElementList with EMA.
        com.rtsdk.ema.access.ElementList el = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(el, buf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

        TestUtilities.EmaDecode_UPAElementListAll(el, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);
        
        // Copy decoded entries into a different ElementList with EMA
        com.rtsdk.ema.access.ElementList elCopy = EmaFactory.createElementList();
        
        elCopy.info(el.infoElementListNum());
        
        Iterator<ElementEntry> iterator = el.iterator();
        while (iterator.hasNext())
        {
            elCopy.add(iterator.next());
        }
        
        // decode field list copy
        ElementList elDecCopy = JUnitTestConnect.createElementList();
        
        elDecCopy.info(elCopy.infoElementListNum());
        
        JUnitTestConnect.setRsslData(elDecCopy, elCopy, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);
        
        // compare with original
        assertEquals(elDecCopy.size(), el.size());
        
        TestUtilities.EmaDecode_UPAElementListAll(elDecCopy, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES);

        System.out.println("\ntestElementList_EncodeUPA_DecodeEMA_EncodeEMA_DecodeEMA passed");
    }
    
    public void testElementList_EncodeUPAELWithPrimitiveTypes_EncodeEMA_ToAnotherElementList_EMADecode() 
    {
        TestUtilities.printTestHead("testElementList_EncodeUPAELWithPrimitiveTypes_EncodeEMA_ToAnotherElementList_EMADecode", "Encode ElementList with UPA for primitive types, Encode it to another ElementList.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(9216));

        // Encode ElementList with UPA.
        int retVal;
        if ((retVal = TestUtilities.upa_EncodeElementListAll(buf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding element list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeElementListAll.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode ElementList with EMA.
        com.rtsdk.ema.access.ElementList el = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(el, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different ElementList with EMA
        com.rtsdk.ema.access.ElementList elCopy = EmaFactory.createElementList();
        
        elCopy.info(el.infoElementListNum());
        
        Iterator<ElementEntry> iterator = el.iterator();
        while (iterator.hasNext())
        {
            elCopy.add(iterator.next());
        }
        
        // compare with original
        assertEquals(elCopy.size(), el.size());
        
        TestUtilities.EmaDecode_UPAElementListAll(elCopy, EncodingTypeFlags.PRIMITIVE_TYPES);

        System.out.println("\ntestElementList_EncodeUPAELWithPrimitiveTypes_EncodeEMA_ToAnotherElementList_EMADecode passed");
    }
    
    public void testElementList_EncodeUPAELWithContainerTypes_EncodeEMA_ToAnotherElementList_EMADecode() 
    {
        TestUtilities.printTestHead("testElementList_EncodeUPAELWithContainerTypes_EncodeEMA_ToAnotherFieldList_EMADecode", "Encode ElementList with UPA for container types, Encode it to another ElementList.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(14240));

        // Encode ElementList with UPA.
        int retVal;
        if ((retVal = TestUtilities.upa_EncodeElementListAll(buf, TestUtilities.EncodingTypeFlags.CONTAINER_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding element list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeElementListAll.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode ElementList with EMA.
        com.rtsdk.ema.access.ElementList el = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(el, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different ElementList with EMA
        com.rtsdk.ema.access.ElementList elCopy = EmaFactory.createElementList();
        
        elCopy.info(el.infoElementListNum());
        
        Iterator<ElementEntry> iterator = el.iterator();
        while (iterator.hasNext())
        {
            elCopy.add(iterator.next());
        }
        
        // compare with original
        assertEquals(elCopy.size(), el.size());
        
        TestUtilities.EmaDecode_UPAElementListAll(elCopy, EncodingTypeFlags.CONTAINER_TYPES);

        System.out.println("\ntestElementList_EncodeUPAELWithContainerTypes_EncodeEMA_ToAnotherFieldList_EMADecode passed");
    }
    
    public void testElementList_EncodeUPAELWithMsgTypes_EncodeEMA_ToAnotherElementList_EMADecode() 
    {
        TestUtilities.printTestHead("testElementList_EncodeUPAELWithMsgType_EncodeEMA_ToAnotherElementList_EMADecod", "Encode ElementList with UPA for msg type, Encode it to another ElementList.");

        // Create a UPA Buffer to encode into
        com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(8048));

        // Encode ElementList with UPA.
        int retVal;
        if ((retVal = TestUtilities.upa_EncodeElementListAll(buf, TestUtilities.EncodingTypeFlags.MESSAGE_TYPES)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Error encoding element list.");
            System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
                    + ") encountered with TestUtilities.upa_EncodeElementListAll.  " + "Error Text: "
                    + CodecReturnCodes.info(retVal));
            return;
        }

        // Decode ElementList with EMA.
        com.rtsdk.ema.access.ElementList el = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(el, buf, Codec.majorVersion(), Codec.minorVersion(), TestUtilities.getDataDictionary(), null);
        
        // Copy decoded entries into a different ElementList with EMA
        com.rtsdk.ema.access.ElementList elCopy = EmaFactory.createElementList();
        
        elCopy.info(el.infoElementListNum());
        
        Iterator<ElementEntry> iterator = el.iterator();
        while (iterator.hasNext())
        {
            elCopy.add(iterator.next());
        }
        
        // compare with original
        assertEquals(elCopy.size(), el.size());
        
        TestUtilities.EmaDecode_UPAElementListAll(elCopy, EncodingTypeFlags.MESSAGE_TYPES);

        System.out.println("\ntestElementList_EncodeUPAELWithMsgType_EncodeEMA_ToAnotherElementList_EMADecode passed");
    }
    
	public void testElementList_EncodeEMA_EmptyEncode()
	{
		TestUtilities.printTestHead("testElementList_EncodeEMA_EmptyEncode", "Encode empty ElementList with EMA and Decode ElementList with EMA");
		
		ElementList elEnc = EmaFactory.createElementList();
        ElementList elDec = JUnitTestConnect.createElementList();
        
        JUnitTestConnect.setRsslData(elDec, elEnc, Codec.majorVersion(), Codec.minorVersion(), null, null);

        System.out.println(elDec);

        TestUtilities.checkResult("Empty ElementList - hasInfo()" , !elDec.hasInfo());

        Iterator<ElementEntry> iter = elDec.iterator();
        TestUtilities.checkResult("Empty ElementList types - hasNext()", !iter.hasNext());
	}

	public void testElementList_EncodeEMA_DecodeEMA_DecodeAll()
	{
		TestUtilities.printTestHead("testElementList_EncodeEMA_DecodeEMA_DecodeAll", "Encode ElementList with EMA and Decode ElementList with EMA");

		//EMA Encode ElementList
		ElementList elEnc = EmaFactory.createElementList();
		elEnc.info( 555 );
		
		try {
			//EMA Encoding

			//first entry
			ElementEntry ee = EmaFactory.createElementEntry().uintValue( "MY_UINT", 64 );
			TestUtilities.checkResult("ElementEntry.toString() == toString() not supported", ee.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			elEnc.add(ee);
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//second entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT2", 64));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//third entry
			elEnc.add(EmaFactory.createElementEntry().real( "MY_REAL", 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//fourth entry
			elEnc.add(EmaFactory.createElementEntry().intValue( "MY_INT", 32));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//fifth entry
			elEnc.add(EmaFactory.createElementEntry().date( "MY_DATE", 1999, 11, 7));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//sixth entry
			elEnc.add(EmaFactory.createElementEntry().time( "MY_TIME", 02, 03, 04, 005));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//seventh entry
			elEnc.add(EmaFactory.createElementEntry().dateTime( "MY_DATETIME", 1999, 11, 7, 01, 02, 03, 000));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//eightth entry
			elEnc.add(EmaFactory.createElementEntry().qos( "MY_QOS", 5656, 2345 ));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//ninth entry
			elEnc.add(EmaFactory.createElementEntry().state("MY_STATE", OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Succeeded"));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			
			
			//tenth entry
			elEnc.add(EmaFactory.createElementEntry().ascii( "MY_ASCII", "ABCDEF" ));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//eleventh entry
			elEnc.add(EmaFactory.createElementEntry().rmtes( "MY_RMTES", ByteBuffer.wrap("ABCDEF".getBytes())));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//twelfth entry
			elEnc.add(EmaFactory.createElementEntry().enumValue( "MY_ENUM", 29));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//thirteenth entry
			elEnc.add(EmaFactory.createElementEntry().floatValue( "MY_FLOAT", 11.11f));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//fourteenth entry
			elEnc.add(EmaFactory.createElementEntry().doubleValue( "MY_DOUBLE", 22.22f));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//fifteenth entry (blank real)
			elEnc.add(EmaFactory.createElementEntry().codeReal("REAL_CODE"));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//sixteenth entry
			elEnc.add(EmaFactory.createElementEntry().buffer( "MY_BUFFER", ByteBuffer.wrap("ABCDEFGH".getBytes())));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//seventeenth entry
			elEnc.add(EmaFactory.createElementEntry().utf8( "MY_UTF8",ByteBuffer.wrap("KLMNOPQR".getBytes())));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//eighteenth entry
			OmmArray ar1 = EmaFactory.createOmmArray();
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(123));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(234));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(345));
			elEnc.add(EmaFactory.createElementEntry().array( "ARRAY", ar1));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			

			//nineteenth entry
			OmmOpaque opaque1 = EmaFactory.createOmmOpaque();
			opaque1.buffer(ByteBuffer.wrap("OPQRST".getBytes()));
			elEnc.add(EmaFactory.createElementEntry().opaque( "MY_OPAQUE", opaque1));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			
					
			//twentyth entry
			elEnc.add(EmaFactory.createElementEntry().qos( "MY_QOS", 756565, 1232365));
			TestUtilities.checkResult("ElementList.toString() == toString() not supported", elEnc.toString().equals("\nDecoding of just encoded object in the same application is not supported\n"));			
			
			ElementList elDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elDec, elEnc, Codec.majorVersion(), Codec.minorVersion(), null, null);
			// check that we can still get the toString on encoded/decoded container.
			TestUtilities.checkResult("ElementList.toString() != toString() not supported", !(elDec.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));			

			System.out.println(elDec);
            
			TestUtilities.checkResult("ElementList with all data types - hasInfo()" , elDec.hasInfo());
			TestUtilities.checkResult("ElementList with all data types - infoElementListNum()", elDec.infoElementListNum() == 555 );

			Iterator<ElementEntry> iter = elDec.iterator();
			TestUtilities.checkResult("ElementList with all datat types - size of collection = 20 ", elDec.size() == 20);
			TestUtilities.checkResult("ElementList with all datat types - collection isEmpty is false ", !elDec.isEmpty());

			TestUtilities.checkResult("ElementList with all data types- first hasNext()" , iter.hasNext());
			ElementEntry ee1 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "MY_UINT") );
			// check that we can still get the toString on encoded/decoded entry.
			TestUtilities.checkResult("ElementEntry.toString() != toString() not supported", !(ee1.toString().equals("\nDecoding of just encoded object in the same application is not supported\n")));
			
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee1.loadType() == DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee1.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 64 );
			

			TestUtilities.checkResult("ElementList with all data types - second hasNext()", iter.hasNext() );
			ElementEntry ee2 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals( "MY_UINT2") );
			
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee2.loadType() == DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee2.uintValue() == 64 );
			
			TestUtilities.checkResult("ElementList with all data types - third entry", iter.hasNext());
			ElementEntry ee3 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee3.name().equals("MY_REAL"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.real().mantissa()", ee3.real().mantissa() == 11 );
			TestUtilities.checkResult("ElementEntry.real().magnitudeType()", ee3.real().magnitudeType() == 12 );


			System.out.println("ElementList get iterator again");
			iter = elDec.iterator();
			{
				TestUtilities.checkResult("ElementList with all data types - hasInfo()" , elDec.hasInfo() );
				TestUtilities.checkResult("ElementList with all data types- infoElementListNum()", elDec.infoElementListNum() == 555 );

				TestUtilities.checkResult("ElementList with all data types - first entry again", iter.hasNext());
				ee1 = iter.next();
				TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "MY_UINT"));
				TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT",  ee1.loadType() == DataTypes.UINT);
				TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE",  ee1.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 64 );


				TestUtilities.checkResult("ElementList with all data types - first entry again", iter.hasNext());
				ee2 = iter.next();
				TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals( "MY_UINT2"));
				TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT",  ee2.loadType() == DataTypes.UINT);
				TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE",  ee2.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("ElementEntry.uintValue()", ee2.uintValue() == 64 );

				TestUtilities.checkResult("ElementList with all data types - first entry again", iter.hasNext());
				ee3 = iter.next();
				TestUtilities.checkResult("ElementEntry.name()",ee3.name().equals( "MY_REAL"));
				TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
				TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee3.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("ElementEntry.real().mantissa()", ee3.real().mantissa() == 11 );
				TestUtilities.checkResult("ElementEntry.real().magnitudeType()", ee3.real().magnitudeType() == 12 );
			}
			
			TestUtilities.checkResult("ElementList with all data types - 4th entry", iter.hasNext());
			ElementEntry ee4 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee4.name().equals( "MY_INT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.INT", ee4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", ee4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.intValue()" ,  ee4.intValue() == 32);

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext());
			ElementEntry ee5 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee5.name().equals( "MY_DATE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATE", ee5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", ee5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("ElementEntry.date().day()", ee5.date().day() == 7 );
			TestUtilities.checkResult("ElementEntry.date().month()()", ee5.date().month()== 11 );
			TestUtilities.checkResult("ElementEntry.date().year()", ee5.date().year() == 1999 );

			TestUtilities.checkResult("ElementList with all data types - 6th entry", iter.hasNext());
			ElementEntry ee6 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee6.name().equals( "MY_TIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.TIME", ee6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", ee6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.time().hour()", ee6.time().hour() == 02 );
			TestUtilities.checkResult("ElementEntry.time().minute()", ee6.time().minute() == 03 );
			TestUtilities.checkResult("ElementEntry.time().second()", ee6.time().second() == 04 );
			TestUtilities.checkResult("ElementEntry.time().millisecond()", ee6.time().millisecond() == 005 );

			TestUtilities.checkResult("ElementList with all data types - 7th entry", iter.hasNext());
			ElementEntry ee7 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee7.name().equals( "MY_DATETIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATETIME", ee7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", ee7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.dateTime().day()", ee7.dateTime().day() == 7 );
			TestUtilities.checkResult("ElementEntry.dateTime().month()()",  ee7.dateTime().month()== 11 );
			TestUtilities.checkResult("ElementEntry.dateTime().year()", ee7.dateTime().year() == 1999 );
			TestUtilities.checkResult("ElementEntry.dateTime().hour()", ee7.dateTime().hour() == 01 );
			TestUtilities.checkResult("ElementEntry.dateTime().minute()", ee7.dateTime().minute() == 02 );
			TestUtilities.checkResult("ElementEntry.dateTime().second()", ee7.dateTime().second() == 03 );
			TestUtilities.checkResult("ElementEntry.dateTime().millisecond()", ee7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("ElementList with all data types - 8th entry", iter.hasNext());
			ElementEntry ee8 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee8.name().equals( "MY_QOS"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.QOS", ee8.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.QOS", ee8.load().dataType()== DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.qos().timeliness()", ee8.qos().timeliness() == 5656 );
			TestUtilities.checkResult("ElementEntry.qos().rate()", ee8.qos().rate() == 2345 );
			TestUtilities.checkResult("ElementEntry.qos().timelinessAsString()",  ee8.qos().timelinessAsString().equals("Timeliness: 5656" ));
			TestUtilities.checkResult("ElementEntry.qos().rateAsString()",  ee8.qos().rateAsString().equals("Rate: 2345" ));
			TestUtilities.checkResult("ElementEntry.qos().toString()" , ee8.qos().toString().equals("Timeliness: 5656/Rate: 2345"));

			TestUtilities.checkResult("ElementList with all data types - 9th entry", iter.hasNext());
			ElementEntry ee9 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee9.name().equals( "MY_STATE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.STATE", ee9.loadType() == DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.STATE", ee9.load().dataType()== DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee9.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.state().StreamState()", ee9.state().streamState() == OmmState.StreamState.OPEN );
			TestUtilities.checkResult("ElementEntry.state().dataState()", ee9.state().dataState() ==OmmState.DataState.OK );
			TestUtilities.checkResult("ElementEntry.state().statusCode()", ee9.state().statusCode() == OmmState.StatusCode.NONE );
			TestUtilities.checkResult("ElementEntry.state().statusText()", ee9.state().statusText().equals( "Succeeded" ));
			TestUtilities.checkResult("ElementEntry.state().toString()", ee9.state().toString().equals("Open / Ok / None / 'Succeeded'" ));

			TestUtilities.checkResult("ElementList with all data types - 10th entry", iter.hasNext());
			ElementEntry ee10 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee10.name().equals( "MY_ASCII"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ASCII", ee10.loadType() == DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ASCII", ee10.load().dataType()== DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee10.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.ascii()", ee10.ascii().ascii().equals("ABCDEF") );

			TestUtilities.checkResult("ElementList with all data types -11th entry", iter.hasNext());
			TestUtilities.checkResult("ElementList with all data types - ", iter.hasNext() );
			ElementEntry ee11= iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee11.name().equals( "MY_RMTES"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.RMTES", ee11.loadType() == DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.RMTES", ee11.load().dataType()== DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee11.code() ==Data.DataCode.NO_CODE);

			TestUtilities.checkResult("ElementList with all data types - 12th entry ", iter.hasNext() );
			ElementEntry ee12 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee12.name().equals( "MY_ENUM"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ENUM", ee12.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ENUM", ee12.load().dataType()== DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee12.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.Enum()", ee12.enumValue() == 29 );

			TestUtilities.checkResult("ElementList with all data types - 13th entry", iter.hasNext() );
			ElementEntry ee13 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee13.name().equals( "MY_FLOAT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.FLOAT", ee13.loadType() == DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.FLOAT", ee13.load().dataType()== DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee13.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.floatValue()", ee13.floatValue() == 11.11f );

			TestUtilities.checkResult("ElementList with all data types - 14th entry ", iter.hasNext() );
			ElementEntry ee14 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee14.name().equals( "MY_DOUBLE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DOUBLE", ee14.loadType() == DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DOUBLE", ee14.load().dataType()== DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee14.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.floatValue()", ee14.doubleValue() == 22.22f );

			TestUtilities.checkResult("ElementList with all data types - 15th entry", iter.hasNext() );
			ElementEntry ee15 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee15.name().equals( "REAL_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee15.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee15.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee15.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 16th entry", iter.hasNext() );
			ElementEntry ee16 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee16.name().equals( "MY_BUFFER"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.BUFFER", ee16.loadType() == DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.BUFFER", ee16.load().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee16.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.buffer()",  Arrays.equals(ee16.buffer().buffer().array(), new String("ABCDEFGH").getBytes()));

			TestUtilities.checkResult("ElementList with all data types - 17th entry", iter.hasNext() );
			ElementEntry ee17 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee17.name().equals( "MY_UTF8"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UTF8", ee17.loadType() == DataTypes.UTF8 );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UTF8", ee17.load().dataType()== DataTypes.UTF8 );
			TestUtilities.checkResult( ee17.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.utf8()", Arrays.equals(ee17.utf8().buffer().array() , new String("KLMNOPQR").getBytes()));

			TestUtilities.checkResult("ElementList with all data types - 18th entry", iter.hasNext() );
			ElementEntry ee18 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee18.name().equals( "ARRAY"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ARRAY", ee18.loadType() == DataTypes.ARRAY );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ARRAY", ee18.load().dataType()== DataTypes.ARRAY );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee18.code() ==Data.DataCode.NO_CODE);
			OmmArray ar2 = ee18.array();
			Iterator<OmmArrayEntry> arrayIter = ar2.iterator();
				TestUtilities.checkResult("OmmArray within elementlist - first hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae1 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae1.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae1.intValue() == 123 );
				TestUtilities.checkResult("OmmArray within elementlist - second hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae2 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae2.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae2.intValue() == 234 );
				TestUtilities.checkResult("OmmArray within elementlist - third hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae3 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae3.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae3.intValue() == 345 );
				TestUtilities.checkResult("OmmArray within elementlist - final hasNext()",  !arrayIter.hasNext() );
				
            TestUtilities.checkResult("ElementList with all data types - 19th entry", iter.hasNext() );
            ElementEntry ee19 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()",ee19.name().equals( "MY_OPAQUE"));
			TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.OpaqueEnum", ee19.loadType() == DataTypes.OPAQUE );
            TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee19.code() ==Data.DataCode.NO_CODE);
            OmmOpaque opaque2 = ee19.opaque();
			TestUtilities.checkResult( Arrays.equals(opaque2.buffer().array(), new String("OPQRST").getBytes()), "ElementEntry.opaque().buffer()" );

			TestUtilities.checkResult("ElementList with all data types - 20th entry", iter.hasNext() );
			ElementEntry ee20 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee20.name().equals( "MY_QOS"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.QOS", ee20.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.QOS", ee20.load().dataType()== DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee20.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.time().timeliness()", ee20.qos().timeliness() == OmmQos.Timeliness.INEXACT_DELAYED );
			TestUtilities.checkResult("ElementEntry.time().rate()", ee20.qos().rate() == OmmQos.Rate.JUST_IN_TIME_CONFLATED);
			TestUtilities.checkResult("ElementEntry.qos().timelinessAsString()", ee20.qos().timelinessAsString().equals("InexactDelayed" ));
			TestUtilities.checkResult("ElementEntry.qos().rateAsString()", ee20.qos().rateAsString().equals("JustInTimeConflated" ));
			TestUtilities.checkResult("ElementEntry.qos().toString()", ee20.qos().toString().equals("InexactDelayed/JustInTimeConflated"));

			TestUtilities.checkResult("ElementList with all data types - final hasNext()", !(iter.hasNext()));

			TestUtilities.checkResult("ElementList with all data types - exception not expected", true );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( "ElementList with all data types - exception not expected" , false);
			System.out.println(excp);
		}


		elEnc.clear();
		elEnc.info( 777 );

		try {
			//EMA Encoding
			
			//first entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 64));
			
			//second entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT2", 64));
			//third  entry
			elEnc.add(EmaFactory.createElementEntry().codeUInt("UINT_CODE"));

			//fourth entry
			elEnc.add(EmaFactory.createElementEntry().real( "MY_REAL", 11, OmmReal.MagnitudeType.EXPONENT_NEG_2));
			//fifth  entry
			elEnc.add(EmaFactory.createElementEntry().codeReal("REAL_CODE"));

			//sixth entry
			elEnc.add(EmaFactory.createElementEntry().intValue( "MY_INT", 32));
			//seventh  entry
			elEnc.add(EmaFactory.createElementEntry().codeInt("INT_CODE"));

			//eighth entry
			elEnc.add(EmaFactory.createElementEntry().date( "MY_DATE", 1999, 11, 7));
			//ninth  entry
			elEnc.add(EmaFactory.createElementEntry().codeDate("DATE_CODE"));

			//tenth entry
			elEnc.add(EmaFactory.createElementEntry().time( "MY_TIME", 02, 03, 04, 005));
			//eleventh  entry
			elEnc.add(EmaFactory.createElementEntry().codeTime("TIME_CODE"));

			//twelfth entry
			elEnc.add(EmaFactory.createElementEntry().dateTime( "MY_DATETIME", 1999, 11, 7, 01, 02, 03, 000));
			//thirteenth entry
			elEnc.add(EmaFactory.createElementEntry().codeDateTime("DATETIME_CODE"));

			//fourteenth entry
			elEnc.add(EmaFactory.createElementEntry().qos( "MY_QOS", OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK));
			//fifteenth  entry
			elEnc.add(EmaFactory.createElementEntry().codeQos("QOS_CODE"));

			//sixteenth entry
			elEnc.add(EmaFactory.createElementEntry().state("MY_STATE", OmmState.StreamState.OPEN,OmmState.DataState.OK, OmmState.StatusCode.NONE, "Succeeded"));
			//seventeenth  entry
			elEnc.add(EmaFactory.createElementEntry().codeState("STATE_CODE"));

			//eighteenth entry
			elEnc.add(EmaFactory.createElementEntry().ascii( "MY_ASCII", "ABCDEF"));
			//nineteenth  entry
			elEnc.add(EmaFactory.createElementEntry().codeAscii("ASCII_CODE"));

			//twentieth entry
			elEnc.add(EmaFactory.createElementEntry().rmtes( "MY_RMTES", ByteBuffer.wrap("ABCDEF".getBytes())));
			//21st  entry
			elEnc.add(EmaFactory.createElementEntry().codeRmtes( "RMTES_CODE"));

			//22nd entry
			elEnc.add(EmaFactory.createElementEntry().enumValue( "MY_ENUM", 29));
			//23rd  entry
			elEnc.add(EmaFactory.createElementEntry().codeEnum( "ENUM_CODE"));

			//24th entry
			elEnc.add(EmaFactory.createElementEntry().floatValue( "MY_FLOAT", 11.11f));
			//25th  entry
			elEnc.add(EmaFactory.createElementEntry().codeFloat( "FLOAT_CODE"));

			//26th entry
			elEnc.add(EmaFactory.createElementEntry().doubleValue( "MY_DOUBLE", 22.22f));
			//27th entry
			elEnc.add(EmaFactory.createElementEntry().codeDouble( "DOUBLE_CODE"));

			//28th entry (blank real)
			elEnc.add(EmaFactory.createElementEntry().codeReal( "REAL_CODE"));

			//29th entry
			elEnc.add(EmaFactory.createElementEntry().buffer( "MY_BUFFER",ByteBuffer.wrap("ABCDEFGH".getBytes())));
			//30th  entry
			elEnc.add(EmaFactory.createElementEntry().codeBuffer( "BUFFER_CODE"));

			//31st entry
			elEnc.add(EmaFactory.createElementEntry().utf8( "MY_UTF8",ByteBuffer.wrap("KLMNOPQR".getBytes())));
			//32nd  entry
			elEnc.add(EmaFactory.createElementEntry().codeUtf8( "UTF8_CODE"));

			//33rd entry
			OmmArray ar1 = EmaFactory.createOmmArray();
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(123));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(234));
			ar1.add(EmaFactory.createOmmArrayEntry().intValue(345));
			elEnc.add(EmaFactory.createElementEntry().array( "ARRAY", ar1));

			//34th entry
			OmmXml xml = EmaFactory.createOmmXml();
			xml.string("OPQRST");
			elEnc.add(EmaFactory.createElementEntry().xml( "MY_XML" , xml));
			

			//Decoding
			ElementList elDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elDec, elEnc, Codec.majorVersion(), Codec.minorVersion(), null, null);

			System.out.println(elDec);

			TestUtilities.checkResult("ElementList with all data types - hasInfo()" , elDec.hasInfo());
			TestUtilities.checkResult("ElementList with all data types - infoElementListNum()" ,  elDec.infoElementListNum() == 777);

			Iterator<ElementEntry> iter = elDec.iterator();
			
			TestUtilities.checkResult("ElementList with all data types - second entry", iter.hasNext() );
			ElementEntry ee1 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "MY_UINT") );
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee1.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee1.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 64 );
			
			TestUtilities.checkResult("ElementList with all data types - second entry", iter.hasNext() );
			ElementEntry ee2 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals( "MY_UINT2") );
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee2.uintValue() == 64 );

			TestUtilities.checkResult("ElementList with all data types - third entry", iter.hasNext() );
			ElementEntry ee2b = iter.next();
			TestUtilities.checkResult( ee2b.name().equals( "UINT_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT",  ee2b.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee2b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 4th entry", iter.hasNext() );
			ElementEntry ee3 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee3.name().equals( "MY_REAL"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.real().mantissa()", ee3.real().mantissa() == 11 );
			TestUtilities.checkResult("ElementEntry.real().magnitudeType()", ee3.real().magnitudeType() == 12 );
			
			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee3b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee3b.name().equals( "REAL_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee3b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee4 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee4.name().equals( "MY_INT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.INT", ee4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", ee4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult( ee4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.intValue()" ,  ee4.intValue() == 32);

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee4b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee4b.name().equals( "INT_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.INT", ee4b.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee4b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee5 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee5.name().equals( "MY_DATE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATE", ee5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", ee5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("ElementEntry.date().day()", ee5.date().day() == 7 );
		TestUtilities.checkResult("ElementEntry.date().month()()", ee5.date().month()== 11 );
			TestUtilities.checkResult("ElementEntry.date().year()", ee5.date().year() == 1999 );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee5b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee5b.name().equals( "DATE_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATE", ee5b.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee5b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee6 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee6.name().equals( "MY_TIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.TIME", ee6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", ee6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.time().hour()", ee6.time().hour() == 02 );
			TestUtilities.checkResult("ElementEntry.time().minute()", ee6.time().minute() == 03 );
			TestUtilities.checkResult("ElementEntry.time().second()", ee6.time().second() == 04 );
			TestUtilities.checkResult("ElementEntry.time().millisecond()", ee6.time().millisecond() == 005 );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee6b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee6b.name().equals( "TIME_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.TIME", ee6b.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee6b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee7 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee7.name().equals( "MY_DATETIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATETIME", ee7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", ee7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.dateTime().day()", ee7.dateTime().day() == 7 );
			TestUtilities.checkResult("ElementEntry.dateTime().month()()",  ee7.dateTime().month()== 11 );
			TestUtilities.checkResult("ElementEntry.dateTime().year()", ee7.dateTime().year() == 1999 );
			TestUtilities.checkResult("ElementEntry.dateTime().hour()", ee7.dateTime().hour() == 01 );
			TestUtilities.checkResult("ElementEntry.dateTime().minute()", ee7.dateTime().minute() == 02 );
			TestUtilities.checkResult("ElementEntry.dateTime().second()", ee7.dateTime().second() == 03 );
			TestUtilities.checkResult("ElementEntry.dateTime().millisecond()", ee7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee7b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee7b.name().equals( "DATETIME_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATETIME", ee7b.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee7b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee8 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee8.name().equals( "MY_QOS"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.QOS", ee8.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.QOS", ee8.load().dataType()== DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.time().timeliness()", ee8.qos().timeliness() == OmmQos.Timeliness.REALTIME );
			TestUtilities.checkResult("ElementEntry.time().rate()", ee8.qos().rate() == OmmQos.Rate.TICK_BY_TICK );
			TestUtilities.checkResult("ElementEntry.time().timelinessAsString()", ee8.qos().timelinessAsString().equals("RealTime" ));
			TestUtilities.checkResult("ElementEntry.time().rateAsString()", ee8.qos().rateAsString().equals("TickByTick" ));
			TestUtilities.checkResult("ElementEntry.time().toString()", ee8.qos().toString().equals("RealTime/TickByTick" ));

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee8ba = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee8ba.name().equals( "QOS_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.QOS", ee8ba.loadType() == DataTypes.QOS );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee8ba.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee9 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee9.name().equals( "MY_STATE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.STATE", ee9.loadType() == DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.STATE", ee9.load().dataType()== DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee9.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.state().StreamState()", ee9.state().streamState() == OmmState.StreamState.OPEN );
			TestUtilities.checkResult("ElementEntry.state().dataState()", ee9.state().dataState() ==OmmState.DataState.OK );
			TestUtilities.checkResult("ElementEntry.state().statusCode()", ee9.state().statusCode() == OmmState.StatusCode.NONE );
			TestUtilities.checkResult("ElementEntry.state().statusText()", ee9.state().statusText().equals( "Succeeded" ));
			TestUtilities.checkResult("ElementEntry.state().toString()", ee9.state().toString().equals("Open / Ok / None / 'Succeeded'") );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee9b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee9b.name().equals( "STATE_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.STATE", ee9b.loadType() == DataTypes.STATE );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee9b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee10 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee10.name().equals( "MY_ASCII"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ASCII", ee10.loadType() == DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ASCII", ee10.load().dataType()== DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee10.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.ascii()", ee10.ascii().ascii().equals( "ABCDEF" ));

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee10b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee10b.name().equals( "ASCII_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ASCII", ee10b.loadType() == DataTypes.ASCII );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee10b.code() == Data.DataCode.BLANK );
			
			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee11= iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee11.name().equals( "MY_RMTES"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.RMTES", ee11.loadType() == DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.RMTES", ee11.load().dataType()== DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee11.code() ==Data.DataCode.NO_CODE);

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee11b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee11b.name().equals( "RMTES_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.RMTES", ee11b.loadType() == DataTypes.RMTES );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee11b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee12 = iter.next();
			TestUtilities.checkResult( ee12.name().equals( "MY_ENUM"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ENUM", ee12.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ENUM", ee12.load().dataType()== DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee12.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.Enum()", ee12.enumValue() == 29 );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee12b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee12b.name().equals( "ENUM_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ENUM", ee12b.loadType() == DataTypes.ENUM );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee12b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee13 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee13.name().equals( "MY_FLOAT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.FLOAT", ee13.loadType() == DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.FLOAT", ee13.load().dataType()== DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee13.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.floatValue()", ee13.floatValue() == 11.11f );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee13b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee13b.name().equals( "FLOAT_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.FLOAT", ee13b.loadType() == DataTypes.FLOAT );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee13b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee14 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee14.name().equals( "MY_DOUBLE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DOUBLE", ee14.loadType() == DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DOUBLE", ee14.load().dataType()== DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee14.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.floatValue()", ee14.doubleValue() == 22.22f );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee14b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee14b.name().equals( "DOUBLE_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DOUBLE", ee14b.loadType() == DataTypes.DOUBLE );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee14b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee15 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee15.name().equals( "REAL_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee15.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee15.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee15.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee16 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee16.name().equals( "MY_BUFFER"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.BUFFER", ee16.loadType() == DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.BUFFER", ee16.load().dataType()== DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee16.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.buffer()", Arrays.equals(ee16.buffer().buffer().array() , new String("ABCDEFGH").getBytes()) );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee16b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee16b.name().equals( "BUFFER_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.BUFFER", ee16b.loadType() == DataTypes.BUFFER );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee16b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee17 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee17.name().equals( "MY_UTF8"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UTF8", ee17.loadType() == DataTypes.UTF8 );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UTF8", ee17.load().dataType()== DataTypes.UTF8 );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee17.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.utf8()",  Arrays.equals(ee17.utf8().buffer().array() , new String("KLMNOPQR").getBytes() ));

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee17b = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee17b.name().equals( "UTF8_CODE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UTF8", ee17b.loadType() == DataTypes.UTF8 );
			TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee17b.code() == Data.DataCode.BLANK );

			TestUtilities.checkResult("ElementList with all data types - 5th entry", iter.hasNext() );
			ElementEntry ee18 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",  ee18.name().equals( "ARRAY"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ARRAY", ee18.loadType() == DataTypes.ARRAY );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ARRAY", ee18.load().dataType()== DataTypes.ARRAY );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee18.code() ==Data.DataCode.NO_CODE);
			OmmArray ar2 = ee18.array();
			Iterator<OmmArrayEntry> arrayIter = ar2.iterator();
				TestUtilities.checkResult("OmmArray within elementlist - first hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae1 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae1.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae1.intValue() == 123 );
				TestUtilities.checkResult("OmmArray within elementlist - second hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae2 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae2.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae2.intValue() == 234 );
				TestUtilities.checkResult("OmmArray within elementlist - third hasNext()",  arrayIter.hasNext() );
					OmmArrayEntry ae3 = arrayIter.next();
					TestUtilities.checkResult("OmmArrayEntry.loadType() == DataTypes.INT", ae3.loadType() == DataTypes.INT );
					TestUtilities.checkResult("OmmArrayEntry.intValue()", ae3.intValue() == 345 );
				TestUtilities.checkResult("OmmArray within elementlist - final hasNext()",  !arrayIter.hasNext() );
				
			TestUtilities.checkResult("ElementList with all data types - 34th entry", iter.hasNext() );
			ElementEntry ee19 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee19.name().equals( "MY_XML"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.XML", ee19.loadType() == DataTypes.XML);
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee19.code() ==Data.DataCode.NO_CODE);
			OmmXml xml2 = ee19.xml();
			TestUtilities.checkResult( xml2.string().equals("OPQRST"), "ElementEntry.xml().string()" );

			TestUtilities.checkResult("ElementList after clear() - final hasNext()",  !(iter.hasNext()) );

			TestUtilities.checkResult("ElementList with all data types - exception not expected" , true );
		}
		catch ( OmmException excp )
		{
			TestUtilities.checkResult( "Encode ElementList after clear() - exception not expected" , false);
			System.out.println(excp);
		}
		
		
		
		
		
		//Decode ElementList with EMA

	}

	public void testElementList_EncodeEMA_DecodeEMA_ContainsElementList_EncodeDecodeAll()
	{
		TestUtilities.printTestHead("testElementList_EncodeEMA_DecodeEMA_ContainsElementList_EncodeDecodeAll", "");

		// load dictionary
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);

		ElementList elEnc = EmaFactory.createElementList();
		elEnc.info( 9999 );

		try { 
			//EMA Encoding
			// encoding order:  UINT, UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

			//first entry 
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 64 ));

			//second entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT2", 64 ));

			//third entry
			elEnc.add(EmaFactory.createElementEntry().real( "MY_REAL", 11, OmmReal.MagnitudeType.EXPONENT_NEG_2 ));

			//fourth entry
			elEnc.add(EmaFactory.createElementEntry().intValue( "MY_INT", 32 ));

			//fifth entry
			elEnc.add(EmaFactory.createElementEntry().date( "MY_DATE", 1999, 11, 7 ));

			//sixth entry
			elEnc.add(EmaFactory.createElementEntry().time( "MY_TIME", 02, 03, 04, 005 ));

			//seventh entry
			elEnc.add(EmaFactory.createElementEntry().dateTime( "MY_DATETIME", 1999, 11, 7, 01, 02, 03, 000 ));

			//eightth entry (nested ElementList)
			ElementList elEnc1= EmaFactory.createElementList();
			elEnc1.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 641));
			elEnc.add(EmaFactory.createElementEntry().elementList( "EE_UINT", elEnc1 ));

			//ninth entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 642 ));
			

			//Now do EMA decoding of ElementList
			ElementList elDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elDec, elEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(elDec);

			TestUtilities.checkResult("ElementList with primitives and ElementList - hasInfo()" , elDec.hasInfo() );
			TestUtilities.checkResult("ElementList with primitives and ElementList - infoElementListNum()" ,  elDec.infoElementListNum() == 9999);

			Iterator<ElementEntry> iter = elDec.iterator();

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee1 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "MY_UINT") );
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee1.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee1.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 64 );

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee2 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals( "MY_UINT2") );
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee2.uintValue() == 64 );

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee3 = iter.next();
			TestUtilities.checkResult( ee3.name().equals( "MY_REAL"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.real().mantissa()", ee3.real().mantissa() == 11 );
			TestUtilities.checkResult("ElementEntry.real().magnitudeType()", ee3.real().magnitudeType() == 12 );

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee4 = iter.next();
			TestUtilities.checkResult( ee4.name().equals( "MY_INT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.INT", ee4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", ee4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.intValue()" ,  ee4.intValue() == 32);

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee5 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee5.name().equals( "MY_DATE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATE", ee5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", ee5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("ElementEntry.date().day()", ee5.date().day() == 7 );
		TestUtilities.checkResult("ElementEntry.date().month()()", ee5.date().month()== 11 );
			TestUtilities.checkResult("ElementEntry.date().year()", ee5.date().year() == 1999 );

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee6 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee6.name().equals( "MY_TIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.TIME", ee6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", ee6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.time().hour()", ee6.time().hour() == 02 );
			TestUtilities.checkResult("ElementEntry.time().minute()", ee6.time().minute() == 03 );
			TestUtilities.checkResult("ElementEntry.time().second()", ee6.time().second() == 04 );
			TestUtilities.checkResult("ElementEntry.time().millisecond()", ee6.time().millisecond() == 005 );

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee7 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee7.name().equals( "MY_DATETIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATETIME", ee7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", ee7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.dateTime().day()", ee7.dateTime().day() == 7 );
			TestUtilities.checkResult("ElementEntry.dateTime().month()()",  ee7.dateTime().month()== 11 );
			TestUtilities.checkResult("ElementEntry.dateTime().year()", ee7.dateTime().year() == 1999 );
			TestUtilities.checkResult("ElementEntry.dateTime().hour()", ee7.dateTime().hour() == 01 );
			TestUtilities.checkResult("ElementEntry.dateTime().minute()", ee7.dateTime().minute() == 02 );
			TestUtilities.checkResult("ElementEntry.dateTime().second()", ee7.dateTime().second() == 03 );
			TestUtilities.checkResult("ElementEntry.dateTime().millisecond()", ee7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee8 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee8.name().equals( "EE_UINT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ELEMENT_LIST", ee8.loadType() == DataTypes.ELEMENT_LIST );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ELEMENT_LIST", ee8.load().dataType()== DataTypes.ELEMENT_LIST );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.code() == DataTypes.ELEMENT_LIST", ee8.load().dataType()== DataTypes.ELEMENT_LIST );
			{
				ElementList nestedEl = ee8.elementList();

				TestUtilities.checkResult("ElementEntry ElementList within elementlist - hasInfo()",  !nestedEl.hasInfo() );

				Iterator<ElementEntry> nestedIter = nestedEl.iterator();
				TestUtilities.checkResult("ElementEntry ElementList within elementlist - first elementlist hasNext()" , nestedIter.hasNext());
				ElementEntry nee1 = nestedIter.next();
				TestUtilities.checkResult( nee1.name().equals( "MY_UINT"));
				TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", nee1.loadType() == DataTypes.UINT);
				TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", nee1.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("ElementEntry.uintValue()", nee1.uintValue() == 641 );

				TestUtilities.checkResult("ElementEntry ElementList within elementlist - second elementlist hasNext()", !nestedIter.hasNext());
			}

			TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
			ElementEntry ee9 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()",ee9.name().equals( "MY_UINT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee9.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee9.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee9.uintValue() == 642 );


			TestUtilities.checkResult("ElementList with primitives and ElementList - tenth hasNext()", !iter.hasNext());

			TestUtilities.checkResult("ElementList with primitives and ElementList - exception not expected", true);
			
			dictionary = null;

		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "ElementList with primitives and ElementList - exception not expected" , false);
			System.out.println(excp);
		}
	}
	
    public void testElementListList_EncodeEMA_DecodeEMA_ContainsFieldList_EncodeDecodeAll()
	{
		
		TestUtilities.printTestHead("testElementList_EncodeEMA_DecodeEMA_DecodeAll", "Encode ElementList with EMA and Decode ElementList with EMA");

		// load dictionary
		com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
				.createDataDictionary();
		TestUtilities.upa_encodeDictionaryMsg(dictionary);
		
		ElementList elEnc= EmaFactory.createElementList();
		elEnc.info( 65 );

		try { 
			//EMA Encoding
			// encoding order:  UINT, UINT, REAL, INT, DATE, TIME, DATETIME, FieldList, UINT

			//first entry 
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 64 ));

			//second entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT2", 64 ));

			//third entry
			elEnc.add(EmaFactory.createElementEntry().real( "MY_REAL", 11, OmmReal.MagnitudeType.EXPONENT_NEG_2 ));

			//fourth entry
			elEnc.add(EmaFactory.createElementEntry().intValue( "MY_INT", 32 ));

			//fifth entry
			elEnc.add(EmaFactory.createElementEntry().date( "MY_DATE", 1999, 11, 7 ));

			//sixth entry
			elEnc.add(EmaFactory.createElementEntry().time( "MY_TIME", 02, 03, 04, 005 ));

			//seventh entry
			elEnc.add(EmaFactory.createElementEntry().dateTime( "MY_DATETIME", 1999, 11, 7, 01, 02, 03, 000 ));

			//eightth entry (nested FieldList)
			FieldList elEnc1= EmaFactory.createFieldList();
			elEnc1.add(EmaFactory.createFieldEntry().uintValue( 1, 641));
			elEnc.add(EmaFactory.createElementEntry().fieldList( "MY_FIELD_LIST", elEnc1));

			//ninth entry
			elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 642));
			

			//Now do EMA decoding of ElementList
			ElementList elDec = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(elDec, elEnc, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

			System.out.println(elDec);

			TestUtilities.checkResult("ElementList with primitives and FieldList - hasInfo()" , elDec.hasInfo() );
			TestUtilities.checkResult("ElementList with primitives and FieldList - infoElementListNum()" ,  elDec.infoElementListNum() == 65);

			Iterator<ElementEntry> iter = elDec.iterator();
			
			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee1 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "MY_UINT") );
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee1.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee1.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 64 );

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee2 = iter.next();
			TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals( "MY_UINT2") );
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee2.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee2.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee2.uintValue() == 64 );

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee3 = iter.next();
			TestUtilities.checkResult( ee3.name().equals( "MY_REAL"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee3.load().dataType()== DataTypes.REAL );
			TestUtilities.checkResult("ElementEntry.real().mantissa()", ee3.real().mantissa() == 11 );
			TestUtilities.checkResult("ElementEntry.real().magnitudeType()", ee3.real().magnitudeType() == 12 );

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee4 = iter.next();
			TestUtilities.checkResult( ee4.name().equals( "MY_INT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.INT", ee4.loadType() == DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", ee4.load().dataType()== DataTypes.INT );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee4.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.intValue()" ,  ee4.intValue() == 32);

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee5 = iter.next();
			TestUtilities.checkResult( ee5.name().equals( "MY_DATE"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATE", ee5.loadType() == DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", ee5.load().dataType()== DataTypes.DATE );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee5.code() ==Data.DataCode.NO_CODE);
					TestUtilities.checkResult("ElementEntry.date().day()", ee5.date().day() == 7 );
		TestUtilities.checkResult("ElementEntry.date().month()()", ee5.date().month()== 11 );
			TestUtilities.checkResult("ElementEntry.date().year()", ee5.date().year() == 1999 );

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee6 = iter.next();
			TestUtilities.checkResult( ee6.name().equals( "MY_TIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.TIME", ee6.loadType() == DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", ee6.load().dataType()== DataTypes.TIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee6.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.time().hour()", ee6.time().hour() == 02 );
			TestUtilities.checkResult("ElementEntry.time().minute()", ee6.time().minute() == 03 );
			TestUtilities.checkResult("ElementEntry.time().second()", ee6.time().second() == 04 );
			TestUtilities.checkResult("ElementEntry.time().millisecond()", ee6.time().millisecond() == 005 );

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee7 = iter.next();
			TestUtilities.checkResult( ee7.name().equals( "MY_DATETIME"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATETIME", ee7.loadType() == DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", ee7.load().dataType()== DataTypes.DATETIME );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee7.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.dateTime().day()", ee7.dateTime().day() == 7 );
			TestUtilities.checkResult("ElementEntry.dateTime().month()()",  ee7.dateTime().month()== 11 );
			TestUtilities.checkResult("ElementEntry.dateTime().year()", ee7.dateTime().year() == 1999 );
			TestUtilities.checkResult("ElementEntry.dateTime().hour()", ee7.dateTime().hour() == 01 );
			TestUtilities.checkResult("ElementEntry.dateTime().minute()", ee7.dateTime().minute() == 02 );
			TestUtilities.checkResult("ElementEntry.dateTime().second()", ee7.dateTime().second() == 03 );
			TestUtilities.checkResult("ElementEntry.dateTime().millisecond()", ee7.dateTime().millisecond() == 000 );

			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee8 = iter.next();
			TestUtilities.checkResult( ee8.name().equals( "MY_FIELD_LIST"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.ELEMENT_LIST", ee8.loadType() == DataTypes.FIELD_LIST );
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.ELEMENT_LIST", ee8.load().dataType()== DataTypes.FIELD_LIST );
			TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee8.code() ==Data.DataCode.NO_CODE);
			TestUtilities.checkResult("ElementEntry.code() == DataTypes.ELEMENT_LIST", ee8.load().dataType()== DataTypes.FIELD_LIST );
            {
				FieldList nestedFl = ee8.fieldList();

				TestUtilities.checkResult("FieldEntry FieldList within elementlist - hasInfo()",  !nestedFl.hasInfo() );

				Iterator<FieldEntry> nestedIter = nestedFl.iterator();
				TestUtilities.checkResult("FieldEntry FieldList within elementlist - first elementlist hasNext()" , nestedIter.hasNext());
				FieldEntry nfe1 = nestedIter.next();
				TestUtilities.checkResult("FieldEntry.name()",nfe1.fieldId() == 1 );
				TestUtilities.checkResult( nfe1.name().equals( "PROD_PERM"));
				TestUtilities.checkResult("FieldEntry.loadType() == DataTypes.UINT", nfe1.loadType() == DataTypes.UINT);
				TestUtilities.checkResult("FieldEntry.code() ==Data.DataCode.NO_CODE", nfe1.code() ==Data.DataCode.NO_CODE);
				TestUtilities.checkResult("FieldEntry.uintValue()", nfe1.uintValue() == 641 );

				TestUtilities.checkResult("FieldEntry FieldList within elementlist - second elementlist hasNext()", !nestedIter.hasNext());
			}


			TestUtilities.checkResult("ElementList with primitives and FieldList - first entry", iter.hasNext());
			ElementEntry ee9 = iter.next();
			TestUtilities.checkResult( ee9.name().equals( "MY_UINT"));
			TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee9.loadType() == DataTypes.UINT);
			TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee9.load().dataType()== DataTypes.UINT );
			TestUtilities.checkResult("ElementEntry.uintValue()", ee9.uintValue() == 642 );


			TestUtilities.checkResult("ElementList with primitives and FieldList - tenth hasNext()", !iter.hasNext());

			TestUtilities.checkResult("ElementList with primitives and FieldList - exception not expected", true);
			
			dictionary = null;

		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "ElementList with primitives and FieldList - exception not expected" , false);
			System.out.println(excp);
		}

	}
	
	public void testElementList_EncodeEMA_DecodeEMA_ContainsMap_EncodeDecodeAll()
    {
        TestUtilities.printTestHead("testElementList_EncodeEMA_DecodeEMA_DecodeAll", "Encode ElementList with EMA and Decode ElementList with EMA");

        ElementList elEnc = EmaFactory.createElementList();
        elEnc.info( 65 );

        try { 
            //EMA Encoding
            // encoding order:  UINT, UINT, REAL, INT, DATE, TIME, DATETIME, Map, UINT

            //first entry 
            elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 64 ));

            //second entry
            elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT2", 64 ));

            //third entry
            elEnc.add(EmaFactory.createElementEntry().real( "MY_REAL", 11, OmmReal.MagnitudeType.EXPONENT_NEG_2 ));

            //fourth entry
            elEnc.add(EmaFactory.createElementEntry().intValue( "MY_INT", 32 ));

            //fifth entry
            elEnc.add(EmaFactory.createElementEntry().date( "MY_DATE", 1999, 11, 7 ));

            //sixth entry
            elEnc.add(EmaFactory.createElementEntry().time( "MY_TIME", 02, 03, 04, 005 ));

            //seventh entry
            elEnc.add(EmaFactory.createElementEntry().dateTime( "MY_DATETIME", 1999, 11, 7, 01, 02, 03, 000 ));

            //eightth entry (nested Map)
            Map mapEnc1 = EmaFactory.createMap();
            TestUtilities.EmaEncodeMapAllWithElementList( mapEnc1);
            elEnc.add(EmaFactory.createElementEntry().map( "MY_MAP", mapEnc1));

            //ninth entry
            elEnc.add(EmaFactory.createElementEntry().uintValue( "MY_UINT", 642));


            //Now do EMA decoding of ElementList
            ElementList flDec = JUnitTestConnect.createElementList();
            JUnitTestConnect.setRsslData(flDec, elEnc, Codec.majorVersion(), Codec.minorVersion(), null, null);

            System.out.println(flDec);

            Iterator<ElementEntry> iter = flDec.iterator();

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee1 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals( "MY_UINT") );
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee1.loadType() == DataTypes.UINT);
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee1.load().dataType()== DataTypes.UINT );
            TestUtilities.checkResult("ElementEntry.uintValue()", ee1.uintValue() == 64 );

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee2 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals( "MY_UINT2") );
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee2.loadType() == DataTypes.UINT);
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee2.load().dataType()== DataTypes.UINT );
            TestUtilities.checkResult("ElementEntry.uintValue()", ee2.uintValue() == 64 );

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee3 = iter.next();
            TestUtilities.checkResult( ee3.name().equals( "MY_REAL"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee3.loadType() == DataTypes.REAL );
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.REAL", ee3.load().dataType()== DataTypes.REAL );
            TestUtilities.checkResult("ElementEntry.real().mantissa()", ee3.real().mantissa() == 11 );
            TestUtilities.checkResult("ElementEntry.real().magnitudeType()", ee3.real().magnitudeType() == 12 );

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee4 = iter.next();
            TestUtilities.checkResult( ee4.name().equals( "MY_INT"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.INT", ee4.loadType() == DataTypes.INT );
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.INT", ee4.load().dataType()== DataTypes.INT );
            TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee4.code() ==Data.DataCode.NO_CODE);
            TestUtilities.checkResult("ElementEntry.intValue()" ,  ee4.intValue() == 32);

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee5 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()",ee5.name().equals( "MY_DATE"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATE", ee5.loadType() == DataTypes.DATE );
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATE", ee5.load().dataType()== DataTypes.DATE );
            TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee5.code() ==Data.DataCode.NO_CODE);
            TestUtilities.checkResult("ElementEntry.date().day()", ee5.date().day() == 7 );
            TestUtilities.checkResult("ElementEntry.date().month()()", ee5.date().month()== 11 );
            TestUtilities.checkResult("ElementEntry.date().year()", ee5.date().year() == 1999 );

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee6 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()",ee6.name().equals( "MY_TIME"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.TIME", ee6.loadType() == DataTypes.TIME );
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.TIME", ee6.load().dataType()== DataTypes.TIME );
            TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee6.code() ==Data.DataCode.NO_CODE);
            TestUtilities.checkResult("ElementEntry.time().hour()", ee6.time().hour() == 02 );
            TestUtilities.checkResult("ElementEntry.time().minute()", ee6.time().minute() == 03 );
            TestUtilities.checkResult("ElementEntry.time().second()", ee6.time().second() == 04 );
            TestUtilities.checkResult("ElementEntry.time().millisecond()", ee6.time().millisecond() == 005 );

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee7 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()",ee7.name().equals( "MY_DATETIME"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.DATETIME", ee7.loadType() == DataTypes.DATETIME );
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.DATETIME", ee7.load().dataType()== DataTypes.DATETIME );
            TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee7.code() ==Data.DataCode.NO_CODE);
            TestUtilities.checkResult("ElementEntry.dateTime().day()", ee7.dateTime().day() == 7 );
            TestUtilities.checkResult("ElementEntry.dateTime().month()()",  ee7.dateTime().month()== 11 );
            TestUtilities.checkResult("ElementEntry.dateTime().year()", ee7.dateTime().year() == 1999 );
            TestUtilities.checkResult("ElementEntry.dateTime().hour()", ee7.dateTime().hour() == 01 );
            TestUtilities.checkResult("ElementEntry.dateTime().minute()", ee7.dateTime().minute() == 02 );
            TestUtilities.checkResult("ElementEntry.dateTime().second()", ee7.dateTime().second() == 03 );
            TestUtilities.checkResult("ElementEntry.dateTime().millisecond()", ee7.dateTime().millisecond() == 000 );

            TestUtilities.checkResult("ElementList with primitives and Map - eighth entry", iter.hasNext());
            ElementEntry ee8 = iter.next();
            TestUtilities.checkResult( ee8.name().equals( "MY_MAP"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.MAP", ee8.loadType() == DataTypes.MAP );
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.MAP", ee8.load().dataType()== DataTypes.MAP );
            TestUtilities.checkResult("ElementEntry.code() ==Data.DataCode.NO_CODE", ee8.code() ==Data.DataCode.NO_CODE);
            TestUtilities.checkResult("ElementEntry.code() == DataTypes.MAP", ee8.load().dataType()== DataTypes.MAP );
            {
                Map map = ee8.map();
                Iterator<MapEntry> mapIter = map.iterator();
                TestUtilities.checkResult("ElementEntry Map within elementList - hasKeyElementId()",  map.hasKeyFieldId() );
                TestUtilities.checkResult("ElementEntry Map within map - keyElementId()" ,  map.keyFieldId() == 3426);

                TestUtilities.checkResult("ElementEntry Map within elementList - first map hasNext()", mapIter.hasNext());
                MapEntry me1a = mapIter.next();
                TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me1a.key().dataType()== DataTypes.BUFFER );
                TestUtilities.checkResult("MapEntry.key().buffer()", Arrays.equals( me1a.key().buffer().buffer().array() , new String("ABCD").getBytes()));
                TestUtilities.checkResult("MapEntry.action() == MapEntry.Action.DELETE", me1a.action() == MapEntry.MapAction.DELETE );
                TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.NO_DATA", me1a.load().dataType()== DataTypes.NO_DATA );

                TestUtilities.checkResult("ElementEntry Map within elementList - second map hasNext()", mapIter.hasNext());
                MapEntry me2a = mapIter.next();
                TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me2a.key().dataType()== DataTypes.BUFFER );

                //me2a.key().buffer() is empty
                //..

                TestUtilities.checkResult("MapEntry.action() == MapEntry.MapAction.ADD", me2a.action() == MapEntry.MapAction.ADD );
                TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.ELEMENT_LIST", me2a.load().dataType()== DataTypes.ELEMENT_LIST );
                TestUtilities.EmaDecodeElementListAll( me2a.elementList() );

                TestUtilities.checkResult("ElementEntry Map within elementList - third map hasNext()", mapIter.hasNext());
                MapEntry me3a = mapIter.next();
                TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me3a.key().dataType()== DataTypes.BUFFER );
                TestUtilities.checkResult("MapEntry.key().buffer()",  Arrays.equals(me3a.key().buffer().buffer().array() , new String("EFGHI").getBytes()));
                TestUtilities.checkResult("MapEntry.action() == MapEntry.MapAction.ADD", me3a.action() == MapEntry.MapAction.ADD );
                TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.ELEMENT_LIST", me3a.load().dataType()== DataTypes.ELEMENT_LIST );
                TestUtilities.EmaDecodeElementListAll( me3a.elementList() );

                TestUtilities.checkResult("ElementEntry Map within elementList - fourth map hasNext()" , mapIter.hasNext());
                MapEntry me4a = mapIter.next();
                TestUtilities.checkResult("MapEntry.key().dataType()== DataTypes.BUFFER", me4a.key().dataType()== DataTypes.BUFFER );
                TestUtilities.checkResult("MapEntry.key().buffer()" ,Arrays.equals( me4a.key().buffer().buffer().array() , new String("JKLMNOP").getBytes()));
                TestUtilities.checkResult("MapEntry.action() == MapEntry.MapAction.UPDATE", me4a.action() == MapEntry.MapAction.UPDATE );
                TestUtilities.checkResult("MapEntry.load().dataType()== DataTypes.ELEMENT_LIST", me4a.load().dataType()== DataTypes.ELEMENT_LIST );
                TestUtilities.EmaDecodeElementListAll( me4a.elementList() );

                TestUtilities.checkResult("ElementEntry Map within elementList - fifth map hasNext()", !mapIter.hasNext());
            }

            TestUtilities.checkResult("ElementList with primitives and ElementList - first entry", iter.hasNext());
            ElementEntry ee9 = iter.next();
            TestUtilities.checkResult("ElementEntry.name()",ee9.name().equals( "MY_UINT"));
            TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT", ee9.loadType() == DataTypes.UINT);
            TestUtilities.checkResult("ElementEntry.load().dataType()== DataTypes.UINT", ee9.load().dataType()== DataTypes.UINT );
            TestUtilities.checkResult("ElementEntry.uintValue()", ee9.uintValue() == 642 );

            TestUtilities.checkResult("ElementList with primitives and Map - tenth hasNext()", !iter.hasNext());

            TestUtilities.checkResult("ElementList with primitives and Map - exception not expected", true);
        } catch ( OmmException excp  ) {
            TestUtilities.checkResult( "ElementList with primitives and Map - exception not expected" , false);
            System.out.println(excp);
        }

    }

    public void testElementList_DecodeErrorElementList()
    {
        TestUtilities.printTestHead("testElementList_DecodeErrorElementList", "");

        try { 

            Buffer rsslBuf = CodecFactory.createBuffer();
            rsslBuf.data(ByteBuffer.allocate(1000));

            TestUtilities.upa_EncodeErrorElementList( rsslBuf );

            com.rtsdk.eta.codec.DataDictionary dictionary = com.rtsdk.eta.codec.CodecFactory
                .createDataDictionary();
            TestUtilities.upa_encodeDictionaryMsg(dictionary);

            ElementList el = JUnitTestConnect.createElementList();
            JUnitTestConnect.setRsslData(el, rsslBuf, Codec.majorVersion(), Codec.minorVersion(), dictionary, null);

            Iterator<ElementEntry> iter = el.iterator();
            // first entry found and correct
            {
                TestUtilities.checkResult("ElementList.hasNext() second",  iter.hasNext());

                ElementEntry ee1 = iter.next();

                TestUtilities.checkResult("ElementEntry.name()", ee1.name().equals("MY_UINT"));

                TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT",ee1.loadType() == DataTypes.UINT);

                TestUtilities.checkResult("ElementEntry.uintValue() == 64", ee1.uintValue() == 64 );

                System.out.println();
            }

            // second entry found and correct
            {
                TestUtilities.checkResult("ElementList.hasNext() second",  iter.hasNext());

                ElementEntry ee2 = iter.next();

                TestUtilities.checkResult("ElementEntry.name()", ee2.name().equals("MY_UINT2"));

                TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT",ee2.loadType() == DataTypes.UINT);

                TestUtilities.checkResult("ElementEntry.uintValue() == 64", ee2.uintValue() == 64 );

                System.out.println();
            }

            // third entry found but has bad data
            {
                TestUtilities.checkResult("ElementList.hasNext() third", iter.hasNext());

                ElementEntry ee3 = iter.next();

                TestUtilities.checkResult("ElementEntry.name()", ee3.name().equals("MY_REAL"));

                TestUtilities.checkResult( ee3.loadType() == DataTypes.REAL);

                TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee3.code() == Data.DataCode.BLANK );
                
                System.out.println();
            }

            
            // fourth entry found and correct
            {
                TestUtilities.checkResult("ElementList.hasNext() fifth", iter.hasNext() );

                ElementEntry ee4 = iter.next();

                TestUtilities.checkResult("ElementEntry.name()", ee4.name().equals("MY_UINT3"));

                TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.UINT",  ee4.loadType() == DataTypes.UINT );
                
                TestUtilities.checkResult("ElementEntry.uintValue() == 64", ee4.uintValue() == 64 );

                System.out.println();
 
            }
            
            // fifth entry found but has bad data 
            {
                TestUtilities.checkResult("ElementList.hasNext() fourth", iter.hasNext() );

                ElementEntry ee5 = iter.next();

                TestUtilities.checkResult("ElementEntry.name()", ee5.name().equals("MY_REAL2"));

                TestUtilities.checkResult("ElementEntry.code() == Data.DataCode.BLANK", ee5.code() == Data.DataCode.BLANK );

                System.out.println();
            }

            // sixth entry found and correct data type
            {
                TestUtilities.checkResult("ElementList.hasNext() sixth", iter.hasNext() );

                ElementEntry ee6 = iter.next();

                TestUtilities.checkResult("ElementEntry.name()", ee6.name().equals("MY_REAL3"));

                TestUtilities.checkResult("ElementEntry.loadType() == DataTypes.REAL",  ee6.loadType() == DataTypes.REAL );

                TestUtilities.checkResult("ElementEntry.real()" ,  ee6.real().magnitudeType() ==  12 && ee6.real().mantissa() == 11);

                System.out.println();
            }

            TestUtilities.checkResult("ElementList.hasNext() seventh", !iter.hasNext() );

            TestUtilities.checkResult("Error ElementList decoding - exception not expected", true);

            dictionary = null;

        }
        catch ( OmmException excp  )
        {
            TestUtilities.checkResult( "Error ElementList decoding - exception not expected" , false);
            System.out.println(excp);
        }
    }

	public void testElementList_ElementListDecode_toString()
	{
		TestUtilities.printTestHead("testElementList_ElementListDecode_toString", "Display an encoded ElementList by calling toString()");

		try { 
	
			// Create a UPA Buffer to encode into
			com.rtsdk.eta.codec.Buffer buf = com.rtsdk.eta.codec.CodecFactory.createBuffer();
			buf.data(ByteBuffer.allocate(1024));
	
			// Encode ElementList with UPA.
			int retVal;
			System.out.println("Begin UPA ElementList Encoding");
			if ((retVal = TestUtilities.upa_EncodeElementListAll(buf, TestUtilities.EncodingTypeFlags.PRIMITIVE_TYPES)) < CodecReturnCodes.SUCCESS)
			{
				System.out.println("Error encoding element list.");
				System.out.println("Error " + CodecReturnCodes.toString(retVal) + "(" + retVal
						+ ") encountered with TestUtilities.upa_EncodeElementListAll.  " + "Error Text: "
						+ CodecReturnCodes.info(retVal));
				return;
			}
			System.out.println("End UPA ElementList Encoding");
			System.out.println();
			
			com.rtsdk.ema.access.ElementList el = JUnitTestConnect.createElementList();
			JUnitTestConnect.setRsslData(el, buf, Codec.majorVersion(), Codec.minorVersion(), null, null);
	
			System.out.println(el);
	
			buf = null;
			TestUtilities.checkResult("Elementlist toString Decode - exception not expected", true);
			
		} catch ( OmmException excp  ) {
			TestUtilities.checkResult( "Elementlist toString Decode - exception not expected",false);
			System.out.println(excp);
		}
	}

	public void testElementList_InvalidDate()
	{
		TestUtilities.printTestHead("testElementList_InvalidDate", "encode invalid Time");
		ElementEntry elementEntry = EmaFactory.createElementEntry();
		
		try {
			elementEntry.time( "MY_TIME", 10,11,22,1000,900,900 );
			TestUtilities.checkResult( "ElementList set invalid date - exception expected",false);
		}
		catch (OmmException excp ) {
			TestUtilities.checkResult("ElementList set invalid date - exception expected : "+ excp.getMessage(), true );
		}
	}
	
	public void testElementListEntryWithNoPayload_Encode_Decode()
	{
		TestUtilities.printTestHead("testElementListEntryWithNoPayload_Encode_Decode","Encode multiple Element entry with no payload");
		
		try
		{		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().noData("Element1"));
		elementList.add(EmaFactory.createElementEntry().noData("Element2"));
		
		ElementList elementListDec = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		Iterator<ElementEntry> elementListIt = elementListDec.iterator();
		
		ElementEntry elementEntry = elementListIt.next();
		TestUtilities.checkResult( elementEntry.name().equals("Element1"), "Check the name of the first entry");
		TestUtilities.checkResult( elementEntry.loadType() == DataType.DataTypes.NO_DATA, "Check the load type of the first entry");
		
		elementEntry = elementListIt.next();
		TestUtilities.checkResult( elementEntry.name().equals("Element2"), "Check the name of the second entry");
		TestUtilities.checkResult( elementEntry.loadType() == DataType.DataTypes.NO_DATA, "Check the load type of the second entry");
		
		TestUtilities.checkResult( elementListIt.hasNext() == false , "Check to make sure there is no more entry");
		
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to Encode multiple ElementList entry with no payload - exception not expected with text : " +  excp.getMessage()  );
			return;
		}	
	}
	
	public void testElementListClear_Encode_Decode()
	{
		TestUtilities.printTestHead("testElementListClear_Encode_Decode","Test Clear ElementList before encoding");
		
		com.rtsdk.eta.codec.DataDictionary dataDictionary = TestUtilities.getDataDictionary();
		
		try
		{
		FieldList fieldList = EmaFactory.createFieldList();
		fieldList.add(EmaFactory.createFieldEntry().uintValue(1, 3056));
		fieldList.add(EmaFactory.createFieldEntry().enumValue(15, 840));
		
		Series series = EmaFactory.createSeries();
		series.add(EmaFactory.createSeriesEntry().noData());
		
		ElementList elementList = EmaFactory.createElementList();
		elementList.add(EmaFactory.createElementEntry().series("element1", series));
		
		ElementList elementListDec = JUnitTestConnect.createElementList();
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), dataDictionary, null);
		
		TestUtilities.checkResult( elementListDec.iterator().next().loadType() == DataType.DataTypes.SERIES, "Check data type of entry before calling the clear method" );
		
		elementList.clear();
		
		TestUtilities.checkResult( elementList.isEmpty() , "Check whether the ElementList is empty after calling the clear method" );
		
		elementList.add(EmaFactory.createElementEntry().noData("element2"));
		elementList.add(EmaFactory.createElementEntry().fieldList("element3", fieldList));
		
		elementListDec.clear();
		
		JUnitTestConnect.setRsslData(elementListDec, elementList, Codec.majorVersion(), Codec.minorVersion(), dataDictionary, null);
		
		Iterator<ElementEntry> elementListIt = elementListDec.iterator();
		
		ElementEntry elementEntry = elementListIt.next();
		
		TestUtilities.checkResult( elementEntry.name().equals("element2"), "Check the key value of the first entry" );
		TestUtilities.checkResult( elementEntry.loadType() == DataType.DataTypes.NO_DATA, "Check the load type of the first entry" );
		
		elementEntry = elementListIt.next();
		TestUtilities.checkResult( elementEntry.name().equals("element3"), "Check the key value of the second entry" );
		TestUtilities.checkResult( elementEntry.loadType() == DataType.DataTypes.FIELD_LIST, "Check the load type of the second entry" );
		
		FieldList fieldListDec = elementEntry.fieldList();
		
		Iterator<FieldEntry> fieldIt = fieldListDec.iterator();
		
		FieldEntry fieldEntry = fieldIt.next();
		
		TestUtilities.checkResult( fieldEntry.fieldId() == 1, "Check the field ID of the first field entry" );
		TestUtilities.checkResult( fieldEntry.uintValue() == 3056, "Check the value of the first field entry" );
		
		fieldEntry = fieldIt.next();
		
		TestUtilities.checkResult( fieldEntry.fieldId() == 15, "Check the field ID of the second field entry" );
		TestUtilities.checkResult( fieldEntry.enumValue() == 840, "Check the value of the second field entry" );
		
		TestUtilities.checkResult( fieldIt.hasNext() == false, "Check whether this is an entry from FieldList");

		TestUtilities.checkResult( elementListIt.hasNext() == false, "Check whether there is another Element entry" );
		}
		catch( OmmException excp)
		{
			TestUtilities.checkResult( false, "Fails to encode after calling the clear method - exception not expected with text : " +  excp.getMessage()  );
			return;
		}		
	}
}
