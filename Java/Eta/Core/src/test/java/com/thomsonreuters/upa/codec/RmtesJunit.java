///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.codec;

import static org.junit.Assert.assertEquals;

import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.List;

import org.junit.Test;

import com.thomsonreuters.upa.codec.CharSet.RmtesWorkingSet;

public class RmtesJunit
{
	
	@Test
	public void RMTESTestToolUCS2()
	{
		Buffer inBuffer = CodecFactory.createBuffer();
        RmtesDecoder decoder = CodecFactory.createRmtesDecoder();
        RmtesCacheBuffer cacheBuffer = CodecFactory.createRmtesCacheBuffer(1000);
        RmtesBuffer rmtesBuffer = CodecFactory.createRmtesBuffer(1000);
        RmtesTestCase testCase;
        int retVal;
        
        for (int x = 0; x < allTestCases.size(); ++x)
        {
        	testCase = allTestCases.get(x);
        	
        	System.out.println("---Test: " + testCase.name() + " ---");
        	
        	byte[] byteArray = testCase.testCase();
         	 System.out.print("Input byteArray Byte for Byte data: ");
             for (int i = 0; i < byteArray.length; ++i)
             {
             	System.out.print(String.format("%02x", byteArray[i]));
             	if (i % 2 != 0)
             		System.out.print(" ");
             }
             System.out.println("");
        	if (testCase.name().contains("ASY1012"))
        	{
        		// Used for debugging location
        		byteArray = testCase.testCase();
        	}
        	
			  ByteBuffer byteBuffer = ByteBuffer.allocate(byteArray.length);
	          byteBuffer.put(byteArray);
	          
              byteBuffer.position(0);
              inBuffer.data(byteBuffer, byteBuffer.position(), byteArray.length);

              // Fully clear out buffer because these Junit tests do not account for previous partial update existences
              cacheBuffer.clear();
              for (int i = 0; i < cacheBuffer.allocatedLength(); ++i)
              {
            	  cacheBuffer.byteData().put((byte) 0);
              }
              cacheBuffer.clear();
              retVal = decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
            	if (retVal < CodecReturnCodes.SUCCESS)
              	{
              		System.out.println("Error in RMTESApplyToCache: " + CodecReturnCodes.toString(retVal));
              	}

        	// Fully clear out buffer because these Junit tests do not account for previous partial update existences
        	rmtesBuffer.clear();
              for (int i = 0; i < rmtesBuffer.allocatedLength(); ++i)
              {
            	rmtesBuffer.byteData().put((byte) 0);
              }
            rmtesBuffer.clear();
          	retVal = decoder.RMTESToUCS2(rmtesBuffer, cacheBuffer);
          	if (retVal < CodecReturnCodes.SUCCESS)
          	{
          		System.out.println("Error in RMTESToUCS2: " + CodecReturnCodes.toString(retVal));
          	}
          	 System.out.print("Input cacheBuffer Byte for Byte data: ");
              for (int i = 0; i < cacheBuffer.length(); ++i)
              {
              	System.out.print(String.format("%02x", cacheBuffer.byteData().get(i)));
              	if (i % 2 != 0)
              		System.out.print(" ");
              }
              System.out.println("");
              System.out.println("Output RmtesBuffer string:" + new String(rmtesBuffer.byteData().array(), Charset.forName("UTF-16")));
              System.out.print("RmtesBuffer Byte for Byte data:     " );
              for (int i = 0; i < rmtesBuffer.length(); ++i)
              {
              	System.out.print(String.format("%02x", rmtesBuffer.byteData().get(i)));
              	if (i % 2 != 0)
              		System.out.print(" ");
              }
              System.out.println("");
              
              // Validation
            byte[] endResult = testCase.endResult();
            
            System.out.print("Expected result Byte for Byte data: ");
              for (int i = 0; i < endResult.length; ++i)
              {
              	System.out.print(String.format("%02x", endResult[i]));
              	if (i % 2 != 0)
              		System.out.print(" ");
              }
              System.out.println("\n");
            for (int i = 0; i < endResult.length; ++i)
            {
            	if (endResult[i] != rmtesBuffer.byteData().get(i))
            	{
            		System.out.println("Assert error on above test");
            	}
            	assertEquals(endResult[i], rmtesBuffer.byteData().get(i));
            }
    	  }
	}
	
	public class RmtesTestCase 
	{
		byte[] _testCase;
		byte[] _endResult;
		String _name;
		
		RmtesTestCase(String name, byte[] testCase, byte[] endResult)
		{
			_name = name;
			_testCase = testCase;
			_endResult = endResult;
		}
		
		public byte[] testCase()
		{
			return _testCase;
		}
		
		public byte[] endResult()
		{
			return _endResult;
		}
		
		public String name()
		{
			return _name;
		}
	}
	

	// ----------------------------------- TEST CASES ----------------------------------

	RmtesTestCase STS23003a = new RmtesTestCase( "STS23003a", new byte[] {0x1b, 0x6f, 0x28, 0x41, 0x28, 0x7e, 0x74, 0x27}, new byte[] {(byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD});
	RmtesTestCase STS23003b = new RmtesTestCase( "STS23003b", new byte[]{0x1b, 0x6f, 0x29, 0x21, 0x2f, 0x7e, 0x75, 0x21}, new byte[] {(byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD});
	RmtesTestCase STS23003c = new RmtesTestCase( "STS23003c", new byte[]{0x0e, 0x20, 0x7f}, new byte[]{0x00, 0x20, (byte)0xFF, (byte) 0xFD});
	RmtesTestCase STS23003d = new RmtesTestCase( "STS23003d", new byte[]{0x1b, 0x7c, (byte) 0xa8, (byte) 0xc1, (byte) 0xf4, (byte) 0xa7}, new byte[] {(byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD});
	RmtesTestCase STS23003e = new RmtesTestCase( "STS23003e", new byte[]{0x1b, 0x7c, (byte) 0xa9, (byte) 0xa1, (byte) 0xaf, (byte) 0xfe, (byte) 0xfe, (byte) 0xa1},  new byte[] {(byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD});
	
	RmtesTestCase STS23004a = new RmtesTestCase("STS23004a", new byte[] {0x1b, 0x5b, 0x30, 0x60, 0x61, 0x1b, 0x5b, 0x32, 0x62, 0x1b, 0x5b, 0x33, 0x60, 0x61, 0x1b, 0x5b, 0x33, 0x62, 0x1b, 0x5b, 0x38, 0x60, 0x61, 0x1b, 0x5b, 0x34, 0x62}, new byte[] {0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61, 0x00, 0x61});
	RmtesTestCase STS23004b = new RmtesTestCase("STS23004b", new byte[] {0x1b, 0x5b, 0x35, 0x60, 0x62, 0x1b, 0x5b, 0x33, 0x62}, new byte[]{0x00, 0x62, 0x00, 0x62, 0x00, 0x62, 0x00, 0x62});
	RmtesTestCase STS23004c = new RmtesTestCase("STS23004c", new byte[] {0x1b, 0x5b, 0x34, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x1b, 0x5b, 0x35, 0x62}, new byte[] {0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47, 0x00, 0x47});
	RmtesTestCase STS23004d = new RmtesTestCase("STS23004d", new byte[] {0x1B, 0x5B, 0x31, 0x30, 0x60, 0x58, 0x59, 0x5A, 0x1B, 0x5B, 0x31, 0x30, 0x60, 0x58, 0x59, 0x5A, 0x1B, 0x5B, 0x33, 0x62, 0x1B, 0x5B, 0x33, 0x62}, new byte[]{0x00, 0x58, 0x00, 0x59, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a, 0x00, 0x5a});
	RmtesTestCase STS23004e = new RmtesTestCase("STS23004e", new byte[]{0x1B, 0x5B, 0x35, 0x35, 0x60, 0x58, 0x59, 0x5A}, new byte[] {0x00, 0x58, 0x00, 0x59, 0x00, 0x5a});
	
	RmtesTestCase STS23005a = new RmtesTestCase("STS23005a", new byte[] {0x1b, 0x5b, 0x30, 0x60, 0x41, 0x42, 0x43, 0x44, 0x1b, 0x5b, 0x38, 0x60, 0x45, 0x46, 0x47, 0x48}, new byte[]{0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48});
	RmtesTestCase STS23005b = new RmtesTestCase("STS23005b", new byte[] {0x1b, 0x5b, 0x33, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45}, new byte[] {0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45});
	RmtesTestCase STS23005c = new RmtesTestCase("STS23005c", new byte[] {0x1b, 0x5b, 0x34, 0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47}, new byte[] {0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47});
	RmtesTestCase STS23005d = new RmtesTestCase("STS23005d", new byte[] {0x1B, 0x5B, 0x38, 0x60, 0x58, 0x58, 0x1B, 0x5B, 0x38, 0x38, 0x60, 0x59, 0x59, 0x1B, 0x5B, 0x38, 0x38, 0x38, 0x60, 0x5A, 0x5A}, new byte[] {0x00, 0x58, 0x00, 0x58, 0x00, 0x59, 0x00, 0x59, 0x00, 0x5a, 0x00, 0x5a});
	RmtesTestCase STS23005e = new RmtesTestCase("STS23005e", new byte[]{0x1B, 0x5B, 0x35, 0x60, 0x58, 0x58, 0x1B, 0x5B, 0x35, 0x60, 0x59, 0x59, 0x1B, 0x5B, 0x35, 0x60, 0x5A, 0x5A}, new byte[] {0x00, 0x5a, 0x00, 0x5a});

	RmtesTestCase STS23089 = new RmtesTestCase("STS23089", new byte[] {0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1b, 0x5B, 0x36, 0x34, 0x60, 0x7a}, new byte[] {0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x30, 0x00, 0x7a});
	RmtesTestCase STS23090 = new RmtesTestCase("STS23090", new byte[] {0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1b, 0x5B, 0x61, 0x35, 0x60, 0x41}, new byte[] {}); // Invalid usage test, returns nothing
	RmtesTestCase STS23091 = new RmtesTestCase("STS23091", new byte[] {0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1B, 0x5B, 0x60, 0x41}, new byte[] {0x00, 0x41, 0x00, 0x25, 0x00, 0x30, 0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x30});
	RmtesTestCase STS23092 = new RmtesTestCase("STS23092", new byte[] {0x1B, 0x25, 0x30, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x30, 0x1B, 0x5B, 0x30, 0x60}, new byte[] {0x00, 0x41, 0x00, 0x42, 0x00, 0x43, 0x00, 0x44, 0x00, 0x45, 0x00, 0x46, 0x00, 0x47, 0x00, 0x48, 0x00, 0x49, 0x00, 0x30}); // Invalid data usage case. Curser moves, but no data

	RmtesTestCase ASY1001 = new RmtesTestCase("ASY1001 - Japanese Kanji", new byte[] {0x1B, 0x24, 0x2B, 0x34, 0x1B, 0x7C, (byte) 0xEA, (byte) 0xDE, (byte) 0xEA, (byte) 0xDF}, new byte[] {(byte) 0x88, (byte) 0xB0, (byte) 0x88, (byte) 0xBF}); // Japanese Kanji character test
	RmtesTestCase ASY1002 = new RmtesTestCase("ASY1002 - Chinese 1 & 2", new byte[] {0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, 0x1B, 0x24, 0x2B, 0x36, 0x1B, 0x7C, (byte) 0x46, (byte) 0x47, (byte) 0x62, (byte) 0x3D, (byte) 0xCE, (byte) 0xE4, (byte) 0xB2, (byte) 0xAC}, new byte[] {0x5C, 0x3C, (byte) 0x8D, (byte) 0x8A, (byte) 0x92, (byte) 0x94, (byte) 0x80, (byte) 0x39}); // Chinese 1 and 2 character set test
	RmtesTestCase ASY1003 = new RmtesTestCase("ASY1003 - Japenese Katakana and Japanese Latin", new byte[] {0x1B, 0x2A, 0x32, 0x1B, 0x2B, 0x33, 0x1B, 0x6E, 0x1B, 0x7C, (byte) 0x46, (byte) 0x47, (byte) 0xAF, (byte) 0xDE}, new byte[] {(byte) 0xFF, (byte) 0x86, (byte) 0xFF, (byte) 0x87, 0x00, 0x2F, 0x00, 0x5E}); // Japanese Katakana and Japanese Latin test
	RmtesTestCase ASY1004 = new RmtesTestCase("ASY1004 - UTF8 Encoded", new byte[] {0x1B, 0x25, 0x30, 0x34, (byte) 0xCF, (byte) 0xEF, 0x2F}, new byte[] {0x00, 0x34, 0x03, (byte) 0xEF, 0x00, 0x2F}); // Testing UTF8 sequence
	RmtesTestCase ASY1005 = new RmtesTestCase("ASY1005 - Partial repeat on Chinese that fails", new byte[] {0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, (byte) 0x46, (byte) 0x47, (byte) 0x62, (byte) 0x3D, 0x1B, 0x5B, 0x33, 0x62}, new byte[] {}); // Testing partial update repeat of a chinese character. Puts 0x3D at end 3 times, which do not decode correctly because that would ask for a character outside of scope. Fails RMTES.
	RmtesTestCase ASY1006 = new RmtesTestCase("ASY1006 - Partial repeat on Chinese that works", new byte[] {0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, (byte) 0x46, (byte) 0x47, (byte) 0x62, (byte) 0x3D, 0x1B, 0x5B, 0x34, 0x62}, new byte[] {0x5C, 0x3C, (byte) 0x8D, (byte) 0x8A, (byte) 0xFF, (byte) 0xFD, (byte) 0xFF, (byte) 0xFD}); // Testing partial update repeat of a chinese character. Puts 0x3D at end 4 times, which can be found in scope but is just an FFFD character, twice
	RmtesTestCase ASY1007 = new RmtesTestCase("ASY1007 - Multiple language/character sets", new byte[] {0x1B, 0x2A, 0x32, 0x1B, 0x2B, 0x33, 0x1B, 0x6E, 0x1B, 0x7C, (byte) 0x46, (byte) 0x47, (byte) 0xAF, (byte) 0xDE, 0x1B, 0x24, 0x2B, 0x34, 0x1B, 0x7C, (byte) 0xEA, (byte) 0xDE, (byte) 0xEA, (byte) 0xDF, 0x1B, 0x24, 0x2A, 0x47, 0x1B, 0x6E, 0x1B, 0x24, 0x2B, 0x36, 0x1B, 0x7C, (byte) 0x46, (byte) 0x47, (byte) 0x62, (byte) 0x3D, (byte) 0xCE, (byte) 0xE4, (byte) 0xB2, (byte) 0xAC}, new byte[] {}); // Testing changing out character sets in mid-sequence
	
	RmtesTestCase ASY1008 = new RmtesTestCase("ASY1008 - InitialContextWithShiftFn Test 1", new byte[] {0x1b, 0x6f, 0x21, 0x21, 0x28, 0x40, 0x26, 0x7e, 0x27, 0x21}, new byte[] {0x30, 0x00, 0x25, 0x42, (byte) 0xff, (byte) 0xfd, 0x04, 0x10});
	RmtesTestCase ASY1009 = new RmtesTestCase("ASY1009 - BUA2 Test 2", new byte[] {0x1b, 0x24, 0x2b, 0x34, 0x1b, 0x6f, 0x30, 0x21, 0x32, 0x2b, 0x3f, 0x37}, new byte[] {0x4e, (byte) 0x9c, (byte) 0x9e, (byte) 0xc4, 0x65, (byte) 0xb0});
	RmtesTestCase ASY1010 = new RmtesTestCase("ASY1010 - LockingAndSingleShift Test 6", new byte[] {0x1b, 0x24, 0x2a, 0x35, 0x1b, 0x7d, (byte) 0xa1, (byte) 0xa1, (byte) 0xa4, (byte) 0xa1, (byte) 0xa4, (byte) 0xa2, (byte) 0xa4, (byte) 0xfe, (byte) 0xa5, (byte) 0xa1, (byte) 0xa7, (byte) 0xa1}, new byte[] {});
	RmtesTestCase ASY1011 = new RmtesTestCase("ASY1011 - PartialNo62 missing", new byte[] {0x1b, 0x5b, 0x30, 0x60, 0x50, 0x1b, 0x5b, 0x32}, new byte[] {});
	RmtesTestCase ASY1012 = new RmtesTestCase("ASY1012 - PartialWithoutFullField Test 2", new byte[] {0x1b, 0x5b, 0x30, 0x60, 0x61}, new byte[] {0x00, 0x61});
	RmtesTestCase ASY1013 = new RmtesTestCase("ASY1013 - PartialWithoutFullField Test 3", new byte[] {0x1b, 0x5b, 0x30, 0x60, 0x62}, new byte[] {0x00, 0x62});
	RmtesTestCase ASY1014 = new RmtesTestCase("ASY1014 - MultiLang AR test", new byte[] {0x1b, 0x25, 0x30, (byte) 0xd8, (byte) 0xa1, (byte) 0xd8, (byte) 0xa2, (byte) 0xd9, (byte) 0x81, (byte) 0xda, (byte) 0xa4, (byte) 0xdb, (byte) 0xbc}, new byte[] {0x06, 0x21, 0x06, 0x22, 0x06, 0x41, 0x06, (byte) 0xa4, 0x06, (byte) 0xfc});
	RmtesTestCase ASY1015 = new RmtesTestCase("ASY1015 - MultiLang GC test", new byte[] {0x1b, 0x25, 0x30, (byte) 0xcd, (byte) 0xb4, (byte) 0xce, (byte) 0x86, (byte) 0xce, (byte) 0xa9, (byte) 0xcf, (byte) 0x94, (byte) 0xcf, (byte) 0xb6}, new byte[] {0x03, 0x74, 0x03, (byte) 0x86, 0x03, (byte) 0xa9, 0x03, (byte) 0xd4, 0x03, (byte) 0xf6});
	RmtesTestCase ASY1016 = new RmtesTestCase("ASY1016 - MultiLang CS test", new byte[] {(byte) 0x1b, (byte) 0x25, (byte) 0x30, (byte) 0xe2, (byte) 0x82, (byte) 0xa0, (byte) 0xe2, (byte) 0x82, (byte) 0xa1, (byte) 0xe2, (byte) 0x82, (byte) 0xa2, (byte) 0xe2, (byte) 0x82, (byte) 0xa3, (byte) 0xe2, (byte) 0x82, (byte) 0xa4, (byte) 0xe2, (byte) 0x82, (byte) 0xa5, (byte) 0xe2, (byte) 0x82, (byte) 0xa6, (byte) 0xe2, (byte) 0x82, (byte) 0xa7, (byte) 0xe2, (byte) 0x82, (byte) 0xa8, (byte) 0xe2, (byte) 0x82, (byte) 0xa9, (byte) 0xe2, (byte) 0x82, (byte) 0xaa, (byte) 0xe2, (byte) 0x82, (byte) 0xab, (byte) 0xe2, (byte) 0x82, (byte) 0xac, (byte) 0xe2, (byte) 0x82, (byte) 0xad, (byte) 0xe2, (byte) 0x82, (byte) 0xae, (byte) 0xe2, (byte) 0x82, (byte) 0xaf, (byte) 0xe2, (byte) 0x82, (byte) 0xb0, (byte) 0xe2, (byte) 0x82, (byte) 0xb1}, new byte[] {(byte) 0x20, (byte) 0xa0, (byte) 0x20, (byte) 0xa1, (byte) 0x20, (byte) 0xa2, (byte) 0x20, (byte) 0xa3, (byte) 0x20, (byte) 0xa4, (byte) 0x20, (byte) 0xa5, (byte) 0x20, (byte) 0xa6, (byte) 0x20, (byte) 0xa7, (byte) 0x20, (byte) 0xa8, (byte) 0x20, (byte) 0xa9, (byte) 0x20, (byte) 0xaa, (byte) 0x20, (byte) 0xab, (byte) 0x20, (byte) 0xac, (byte) 0x20, (byte) 0xad, (byte) 0x20, (byte) 0xae, (byte) 0x20, (byte) 0xaf, (byte) 0x20, (byte) 0xb0, (byte) 0x20, (byte) 0xb1});
	RmtesTestCase ASY1017 = new RmtesTestCase("ASY1017 - DataFollowedByPartial", new byte[] {0x1b, 0x25, 0x30, (byte) 0xe1, (byte) 0x9c, (byte) 0x80, (byte) 0xe1, (byte) 0x9c, (byte) 0x88, 0x1b, 0x5b, 0x30, 0x60, 0x1b, 0x25, 0x30, 0x24, 0x35}, new byte[] {0x00, 0x24, 0x00, 0x35, 0x00, 0x00, 0x17, 0x08});
	
	RmtesTestCase ASY1018 = new RmtesTestCase("ASY1018 - Changing GL/GR mid-message, expecting zeroed out GL/GR", new byte[] {0x1B, 0x6F, 0x21, 0x7D, 0x25, 0x6D, 0x25, 0x73, 0x25, 0x49, 0x25, 0x73, 0x47, 0x72, 0x45, 0x7C, 0x40, 0x68, 0x4A, 0x2A, 0x41, 0x6A, 0x3E, 0x6C, 0x0F, 0x28, 0x35, 0x1B, 0x6F, 0x39, 0x66, 0x4C, 0x73, 0x44, 0x6A, 0x21, 0x22, 0x0F, 0x24, 0x2F, 0x1B, 0x7D, (byte) 0xC4, (byte) 0xDD, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x28, (byte) 0xDB, (byte) 0xB2, (byte) 0xC0, (byte) 0xB0, 0x45, 0x53, 0x1B, 0x6F, 0x3B, 0x7E, 0x3B, 0x76, 0x0F, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x46, 0x52, 0x39, 0x39, 0x20, 0x30, 0x36, 0x3A, 0x30, 0x39}, new byte[] {0x25, (byte) 0xce, 0x30, (byte) 0xed, 0x30, (byte) 0xf3, 0x30, (byte) 0xc9, 0x30, (byte) 0xf3, 0x76, 0x7d, 0x7c, (byte) 0xd6, 0x51, 0x48, 0x72, 0x69, 0x76, (byte) 0xf8, 0x58, 0x34, 0x00, 0x28, 0x00, 0x35, 0x53, (byte) 0xf7, 0x7d, 0x04, 0x5b, (byte) 0x9a, 0x30, 0x01, 0x00, 0x24, 0x00, 0x2f, (byte) 0xff, (byte) 0x84, (byte) 0xff, (byte) 0x9d, 0x00, 0x29, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x28, (byte) 0xff, (byte) 0x9b, (byte) 0xff, 0x72, (byte) 0xff, (byte) 0x80, (byte) 0xff, 0x70, 0x00, 0x45, 0x00, 0x53, 0x66, 0x42, 0x4e, (byte) 0x8b, 0x00, 0x29, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x46, 0x00, 0x52, 0x00, 0x39, 0x00, 0x39, 0x00, 0x20, 0x00, 0x30, 0x00, 0x36, 0x00, 0x3a, 0x00, 0x30, 0x00, 0x39});
	
	List<RmtesTestCase> allTestCases = Arrays.asList(STS23003a, STS23003b, STS23003c, STS23003d, STS23003e, STS23004a, STS23004b, STS23004c, STS23004d, STS23004e,
			STS23005a, STS23005b, STS23005c, STS23005d, STS23005e, STS23089, STS23090, STS23091, STS23092, ASY1001, ASY1002, ASY1003, ASY1004, ASY1005, ASY1006, 
			ASY1007, ASY1008, ASY1009, ASY1010, ASY1011, ASY1012, ASY1013, ASY1014, ASY1015, ASY1016, ASY1017, ASY1018);
	
	/**
	 * 		------ The following is a list of character sequence possibilities when switching and building character set tables as well as partial updates ------
	 * 
	 * This is used for creating new tests, or understanding the tests used in RMTES.
	 * 
	 * The codes can be followed by tab breaks.
	 * 
		RMTES Codes
		
		1B = Escape
			21 = ESC_21
				40 = Reuters Ctrl 1 to CL
			22 = ESC_22
				30 = Reuters Ctrl 2 to CR
			24 = ESC_24
				28 = ESC_24_28
					47 = G0 = Chinese1
					48 = G0 = Chinese2
				29 = ESC_24_29
					47 = G1 = Chinese1
					48 = G1 = Chinese2
				2A = ESC_24_2A
					47 = G2 = Chinese1
					35 = G2 = Chinese1
					48 = G2 = Chinese2
				2B = ESC_24_2B
					47 = G3 = Chinese1
					48 = G3 = Chinese2
					36 = G3 = Chinese2
					34 = G3 = JapaneseKanji
			25 = ESC_25
				30 = UTF_ENC
			26 = ESC_26
				40 = ESC_26_40
					1B = ESC_26_40_ESC
						24 = ESC_26_40_ESC_24
							42 = G0 = JapaneseKanji
							29 = ESC_26_40_ESC_24_29
								42 = G1 = JapaneseKanji
							2A = ESC_26_40_ESC_24_2A
								42 = G2 = JapaneseKanji
							2B = ESC_26_40_ESC_24_2B
								42 = G3 = JapaneseKanji
			28 = ESC_28
				42 = G0 = Reuters Basic 1
				49 = G0 = JapaneseKatakana
				4A = G0 = JapaneseLatin
			29 = ESC_29
				31 = G1 = Reuters Basic 2
				42 = G1 = Reuters Basic 1
				49 = G1 = JapaneseKatakana
				4A = G1 = JapaneseLatin
			2A = ESC_2A
				32 = G2 = JapaneseKatakana
			2B = ESC_2B
				33 = G3 = JapaneseLatin
			5B = LBRKT
			60 = Partial update move cursor
			62 = Partial update repeat char at cursor
			6E = GL = G2
			6F = GL = G3
			7E = GR = G1
			7D = GR = G2
			7C = GR = G3

	 */
	
	
	
	@Test
	public void RMTEStoUCS2Conversion()
	{
		Buffer inBuffer = CodecFactory.createBuffer();
        RmtesDecoder decoder = CodecFactory.createRmtesDecoder();
        RmtesCacheBuffer cacheBuffer = CodecFactory.createRmtesCacheBuffer(1000);
        RmtesBuffer rmtesBuffer = CodecFactory.createRmtesBuffer(1000);
        
        String stringText1 = "This is a test";

        inBuffer.data(stringText1);

        decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
        decoder.RMTESToUCS2(rmtesBuffer, cacheBuffer);

        assertEquals(true, stringText1.regionMatches(0, new String(rmtesBuffer.byteData().array(), Charset.forName("UTF-16")), 0, stringText1.length()));
        
        String stringText2 = "The quick brown fox jumps over the lazy dog1234567890,./;l'[]";

        inBuffer.data(stringText2);

        decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
        decoder.RMTESToUCS2(rmtesBuffer, cacheBuffer);

        assertEquals(true, stringText2.regionMatches(0, new String(rmtesBuffer.byteData().array(), Charset.forName("UTF-16")), 0, stringText2.length()));
	}
	
    @Test
    public void partialUpdateTest()
    {
        Buffer inBuffer = CodecFactory.createBuffer();
        RmtesDecoder decoder = CodecFactory.createRmtesDecoder();
        RmtesCacheBuffer cacheBuffer = CodecFactory.createRmtesCacheBuffer(1000);
        char[] charBuf1 = new char[100];
        String charBuf1String = new String(charBuf1);
        char[] charBuf2 = new char[100];
        String charBuf2String = new String(charBuf2);
        char[] outBuf = new char[100];
        char inBuf1[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 0x1B, 0x5B, '3', 0x60, 'j',
                'k', 'l' };
        String inBuf1String = new String(inBuf1);

        inBuffer.data(charBuf1String);
        cacheBuffer.data(cacheBuffer.byteData().put(charBuf2String.getBytes()));
        cacheBuffer.length(0);
        cacheBuffer.allocatedLength(100);

        assertEquals(false, decoder.hasPartialRMTESUpdate(inBuffer));
        decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
        inBuffer.data(cacheBuffer.byteData());
        assertEquals(true, inBuffer.toString().regionMatches(0, cacheBuffer.byteData().asCharBuffer().toString(), 0, cacheBuffer.length()));

        inBuffer.data(inBuf1String);
        cacheBuffer.data(cacheBuffer.byteData().put(charBuf2String.getBytes()));
        cacheBuffer.length(0);
        cacheBuffer.allocatedLength(100);

        outBuf = "abcjklhgi".toCharArray();

        assertEquals(true, decoder.hasPartialRMTESUpdate(inBuffer));
        assertEquals(true, new String(outBuf).regionMatches(0, new String(cacheBuffer.byteData()
                .toString()), 0, cacheBuffer.length()));

        char inBuf2[] = { 'a', 'b', 'c', 'd', 0x1B, 0x5B, '3', 0x62, 'e', 'f', 'g' };
        String inBuf2String = new String(inBuf2);
        cacheBuffer.clear();

        inBuffer.data(inBuf2String);

        cacheBuffer.data(cacheBuffer.byteData().put(charBuf2String.getBytes()));
        cacheBuffer.length(0);
        cacheBuffer.allocatedLength(100);

        outBuf = "abcddddefg".toCharArray();

        assertEquals(true, decoder.hasPartialRMTESUpdate(inBuffer));
        assertEquals(true, new String(outBuf).regionMatches(0, new String(cacheBuffer.byteData()
                .toString()), 0, cacheBuffer.length()));

        char inBuf3[] = { 0x1B, 0x5B, '1', '0', 0x60, 'm', 'n', 'o' };
        String inBuf3String = new String(inBuf3);
        cacheBuffer.clear();

        inBuffer.data(inBuf3String);

        cacheBuffer.data(cacheBuffer.byteData().put("abcdefghijklm".toString().getBytes()));
        cacheBuffer.length(cacheBuffer.byteData().position());
        cacheBuffer.allocatedLength(100);
        outBuf = "abcdefghijmno".toCharArray();

        assertEquals(true, decoder.hasPartialRMTESUpdate(inBuffer));

        decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
        inBuffer.data(new String(cacheBuffer.byteData().toString()));
        cacheBuffer.byteData().position(0);
        assertEquals(true, new String(outBuf).regionMatches(0, new String(cacheBuffer.byteData().array(), Charset.forName("UTF-8")), 0, cacheBuffer.length()));

        inBuffer.data("abcdefgh");

        cacheBuffer.clear();
        cacheBuffer.data(cacheBuffer.byteData().put("abcdefghijkl".toString().getBytes()));
        cacheBuffer.length(12);
        cacheBuffer.allocatedLength(100);
        outBuf = "abcdefgh".toCharArray();

        assertEquals(false, decoder.hasPartialRMTESUpdate(inBuffer));	// No update present

        decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
        inBuffer.data(new String(cacheBuffer.byteData().toString()));
        assertEquals(8, cacheBuffer.length());
        cacheBuffer.byteData().position(0);
        assertEquals(true, new String(outBuf).regionMatches(0, new String(cacheBuffer.byteData().array(), Charset.forName("UTF-8")), 0, cacheBuffer.length()));

        char inBuf4[] = { 'a', 'b', 'c', 0x1B, 0x5B, '2', 0x60, 'd', 'e', 0x1B, 0x5B, '3', 0x62 };
        String inBuf4String = new String(inBuf4);
        inBuffer.data(inBuf4String);

        cacheBuffer.data(cacheBuffer.byteData().put(charBuf2String.getBytes()));
        cacheBuffer.length(0);
        cacheBuffer.allocatedLength(100);
        outBuf = "abdeeee".toCharArray();

        assertEquals(true, decoder.hasPartialRMTESUpdate(inBuffer));

        decoder.RMTESApplyToCache(inBuffer, cacheBuffer);
        inBuffer.data(new String(cacheBuffer.byteData().toString()));
        assertEquals(7, cacheBuffer.length());
        cacheBuffer.byteData().position(0);
        assertEquals(true, new String(outBuf).regionMatches(0, new String(cacheBuffer.byteData().array(), Charset.forName("UTF-8")), 0, cacheBuffer.length()));

    }

    public enum LRCode
    {
        NOLR(0), G0ToGL(1), G1ToGL(2), G2ToGL(3), G3ToGL(4), G1ToGR(5), G2ToGR(6), G3ToGR(7);

        private final int value;

        private LRCode(int value)
        {
            this.value = value;
        }

        public int getValue()
        {
            return value;
        }

    };

    public class controlTest
    {
        byte[] testString;
        int strLen;
        int expectedRet;
        RmtesWorkingSet endSet;
        LRCode endLR;
        int returnValue;

        controlTest(byte[] _testString, int _strLen, int _expectedRet, RmtesWorkingSet _endSet,
                LRCode _endLR, int _returnValue)
        {
            testString = _testString;
            strLen = _strLen;
            expectedRet = _expectedRet;
            endSet = _endSet;
            endLR = _endLR;
            returnValue = _returnValue;
        }
    }

    RmtesWorkingSet standardSet = new RmtesWorkingSet();

    RmtesWorkingSet C1G0 = new RmtesWorkingSet();
    RmtesWorkingSet C2G0 = new RmtesWorkingSet();
    RmtesWorkingSet C1G1 = new RmtesWorkingSet();
    RmtesWorkingSet C2G1 = new RmtesWorkingSet();
    RmtesWorkingSet C1G2 = new RmtesWorkingSet();
    RmtesWorkingSet C2G2 = new RmtesWorkingSet();
    RmtesWorkingSet C1G3 = new RmtesWorkingSet();
    RmtesWorkingSet C2G3 = new RmtesWorkingSet();
    RmtesWorkingSet JLG3 = new RmtesWorkingSet();
    RmtesWorkingSet JKG2 = new RmtesWorkingSet();
    RmtesWorkingSet R2G1 = new RmtesWorkingSet();
    RmtesWorkingSet R1G1 = new RmtesWorkingSet();
    RmtesWorkingSet JKG1 = new RmtesWorkingSet();
    RmtesWorkingSet JLG1 = new RmtesWorkingSet();
    RmtesWorkingSet R1G0 = new RmtesWorkingSet();
    RmtesWorkingSet JKG0 = new RmtesWorkingSet();
    RmtesWorkingSet JLG0 = new RmtesWorkingSet();
    RmtesWorkingSet K1G0 = new RmtesWorkingSet();
    RmtesWorkingSet K1G1 = new RmtesWorkingSet();
    RmtesWorkingSet K1G2 = new RmtesWorkingSet();
    RmtesWorkingSet K1G3 = new RmtesWorkingSet();

    void workingSetSetup()
    {
        CharSet characterSet = new CharSet();
        characterSet.initWorkingSet(standardSet);

        characterSet.initWorkingSet(C1G0);
        C1G0.G0 = characterSet._rsslChinese1;

        characterSet.initWorkingSet(C2G0);
        C2G0.G0 = characterSet._rsslChinese2;

        characterSet.initWorkingSet(C1G1);
        C1G1.G1 = characterSet._rsslChinese1;

        characterSet.initWorkingSet(C2G1);
        C2G1.G1 = characterSet._rsslChinese2;

        characterSet.initWorkingSet(C1G2);
        C1G2.G2 = characterSet._rsslChinese1;

        characterSet.initWorkingSet(C2G2);
        C2G2.G2 = characterSet._rsslChinese2;

        characterSet.initWorkingSet(C1G3);
        C1G3.G3 = characterSet._rsslChinese1;

        characterSet.initWorkingSet(C2G3);
        C2G3.G3 = characterSet._rsslChinese2;

        characterSet.initWorkingSet(JLG3);
        JLG3.G3 = characterSet._rsslJapaneseLatin;

        characterSet.initWorkingSet(JKG2);
        JKG2.G2 = characterSet._rsslJapaneseKatakana;

        characterSet.initWorkingSet(R2G1);
        R2G1.G1 = characterSet._rsslReuterBasic2;

        characterSet.initWorkingSet(R1G1);
        R1G1.G1 = characterSet._rsslReuterBasic1;

        characterSet.initWorkingSet(JKG1);
        JKG1.G1 = characterSet._rsslJapaneseKatakana;

        characterSet.initWorkingSet(JLG1);
        JLG1.G1 = characterSet._rsslJapaneseLatin;

        characterSet.initWorkingSet(R1G0);
        R1G0.G0 = characterSet._rsslReuterBasic1;

        characterSet.initWorkingSet(JLG0);
        JLG0.G0 = characterSet._rsslJapaneseLatin;

        characterSet.initWorkingSet(K1G0);
        K1G0.G0 = characterSet._rsslJapaneseKanji;

        characterSet.initWorkingSet(K1G1);
        K1G1.G1 = characterSet._rsslJapaneseKanji;

        characterSet.initWorkingSet(K1G2);
        K1G2.G2 = characterSet._rsslJapaneseKanji;

        characterSet.initWorkingSet(K1G3);
        K1G3.G3 = characterSet._rsslJapaneseKanji;

    }

    byte nullControlChar[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte C1G0ControlChar[] = { 0x1B, 0x24, 0x28, 0x47, 0x00, 0x00, 0x00, 0x00 };
    byte C2G0ControlChar[] = { 0x1B, 0x24, 0x28, 0x48, 0x00, 0x00, 0x00, 0x00 };
    byte C1G1ControlChar[] = { 0x1B, 0x24, 0x29, 0x47, 0x00, 0x00, 0x00, 0x00 };
    byte C2G1ControlChar[] = { 0x1B, 0x24, 0x29, 0x48, 0x00, 0x00, 0x00, 0x00 };
    byte C1G247ControlChar[] = { 0x1B, 0x24, 0x2A, 0x47, 0x00, 0x00, 0x00, 0x00 };
    byte C1G235ControlChar[] = { 0x1B, 0x24, 0x2A, 0x35, 0x00, 0x00, 0x00, 0x00 };
    byte C2G2ControlChar[] = { 0x1B, 0x24, 0x2A, 0x48, 0x00, 0x00, 0x00, 0x00 };
    byte C1G3ControlChar[] = { 0x1B, 0x24, 0x2B, 0x47, 0x00, 0x00, 0x00, 0x00 };
    byte C2G348ControlChar[] = { 0x1B, 0x24, 0x2B, 0x48, 0x00, 0x00, 0x00, 0x00 };
    byte C2G336ControlChar[] = { 0x1B, 0x24, 0x2B, 0x36, 0x00, 0x00, 0x00, 0x00 };
    byte K1G3ControlChar[] = { 0x1B, 0x24, 0x2B, 0x34, 0x00, 0x00, 0x00, 0x00 };
    byte JLG3ControlChar[] = { 0x1B, 0x2B, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte JKG2ControlChar[] = { 0x1B, 0x2A, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte R2G1ControlChar[] = { 0x1B, 0x29, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte R1G1ControlChar[] = { 0x1B, 0x29, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte JKG1ControlChar[] = { 0x1B, 0x29, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte JLG1ControlChar[] = { 0x1B, 0x29, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte UTFControlChar[] = { 0x1B, 0x25, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte K1G0LongControlChar[] = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x42, 0x00, 0x00 };
    byte K1G1LongControlChar[] = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x29, 0x42, 0x00 };
    byte K1G2LongControlChar[] = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x2A, 0x42, 0x00 };
    byte K1G3LongControlChar[] = { 0x1B, 0x26, 0x40, 0x1B, 0x24, 0x2B, 0x42, 0x00 };
    byte G1GRControlChar[] = { 0x1B, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte G2GLControlChar[] = { 0x1B, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte G2GRControlChar[] = { 0x1B, 0x7D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte G3GLControlChar[] = { 0x1B, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte G3GRControlChar[] = { 0x1B, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte G0GLControlChar[] = { 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte G1GLControlChar[] = { 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    byte EndCharControlChar[] = { 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    /* Fail cases */
    byte FailureControlChar[] = { 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    controlTest controlTestArray[] = {
            new controlTest(nullControlChar, 1, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.NOLR, 1),
            new controlTest(C1G0ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G0, LRCode.NOLR, 4),
            new controlTest(C2G0ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G0, LRCode.NOLR, 4),
            new controlTest(C1G1ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G1, LRCode.NOLR, 4),
            new controlTest(C2G1ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G1, LRCode.NOLR, 4),
            new controlTest(C1G247ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G2, LRCode.NOLR, 4),
            new controlTest(C1G235ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G2, LRCode.NOLR, 4),
            new controlTest(C2G2ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G2, LRCode.NOLR, 4),
            new controlTest(C1G3ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C1G3, LRCode.NOLR, 4),
            new controlTest(C2G348ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G3, LRCode.NOLR, 4),
            new controlTest(C2G336ControlChar, 4, ESCReturnCode.ESC_SUCCESS, C2G3, LRCode.NOLR, 4),
            new controlTest(K1G3ControlChar, 4, ESCReturnCode.ESC_SUCCESS, K1G3, LRCode.NOLR, 4),
            new controlTest(JLG3ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JLG3, LRCode.NOLR, 3),
            new controlTest(JKG2ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JKG2, LRCode.NOLR, 3),
            new controlTest(R2G1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, R2G1, LRCode.NOLR, 3),
            new controlTest(R1G1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, R1G1, LRCode.NOLR, 3),
            new controlTest(JKG1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JKG1, LRCode.NOLR, 3),
            new controlTest(JLG1ControlChar, 3, ESCReturnCode.ESC_SUCCESS, JLG1, LRCode.NOLR, 3),
            new controlTest(UTFControlChar, 3, ESCReturnCode.UTF_ENC, standardSet, LRCode.NOLR, 3),
            new controlTest(K1G0LongControlChar, 6, ESCReturnCode.ESC_SUCCESS, K1G0, LRCode.NOLR, 6),
            new controlTest(K1G1LongControlChar, 7, ESCReturnCode.ESC_SUCCESS, K1G1, LRCode.NOLR, 7),
            new controlTest(K1G2LongControlChar, 7, ESCReturnCode.ESC_SUCCESS, K1G2, LRCode.NOLR, 7),
            new controlTest(K1G3LongControlChar, 7, ESCReturnCode.ESC_SUCCESS, K1G3, LRCode.NOLR, 7),
            new controlTest(G1GRControlChar, 2, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G1ToGR, 2),
            new controlTest(G2GLControlChar, 2, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G2ToGL, 2),
            new controlTest(G2GRControlChar, 2, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G2ToGR, 2),
            new controlTest(G3GLControlChar, 2, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G3ToGL, 2),
            new controlTest(G3GRControlChar, 2, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G3ToGR, 2),
            new controlTest(G0GLControlChar, 1, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G0ToGL, 1),
            new controlTest(G1GLControlChar, 1, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.G1ToGL, 1), // 30
            new controlTest(EndCharControlChar, 1, ESCReturnCode.END_CHAR, standardSet,
                    LRCode.NOLR, 0),
            new controlTest(FailureControlChar, 1, ESCReturnCode.ESC_SUCCESS, standardSet,
                    LRCode.NOLR, 0), };

    @Test
    public void controlParseTest()
    {
        RmtesWorkingSet testSet = new RmtesWorkingSet();
        controlTest curTest;
        int testCount;
        int retCode = ESCReturnCode.ESC_ERROR;
        int ret;
        ByteBuffer start = ByteBuffer.allocate(1000);
        CharSet characterSet = new CharSet();
        RmtesDecoderImpl decoder = new RmtesDecoderImpl();

        RmtesInfo tempInfo = new RmtesInfo();

        System.out.println("Control Parse Tests:\n\n");

        workingSetSetup();

        for (testCount = 0; testCount < controlTestArray.length; testCount++)
        {
            System.out.println("Testcount = " + testCount);
            characterSet.initWorkingSet(testSet);
            curTest = controlTestArray[testCount];
            start.clear();
            start.put(curTest.testString);

            tempInfo = decoder.controlParse(start, 0, curTest.strLen, testSet);
            ret = tempInfo.getValue();
            testSet = tempInfo.getSet();
            retCode = tempInfo.getRetCode();

            assertEquals(curTest.expectedRet, retCode);
            System.out.println("retCode = " + retCode);
            assertEquals(curTest.returnValue, ret);
            System.out.println("ret = " + ret);

            assertEquals(true, curTest.endSet.G0.equals(testSet.G0));
            System.out.println("testSet.G0 = " + testSet.G0 + ", curTest.endSet.G0 = "
                    + curTest.endSet.G0);
            assertEquals(true, curTest.endSet.G1.equals(testSet.G1));
            System.out.println("testSet.G1 = " + testSet.G1 + ", curTest.endSet.G1 = "
                    + curTest.endSet.G1);
            assertEquals(true, curTest.endSet.G2.equals(testSet.G2));
            assertEquals(true, curTest.endSet.G3.equals(testSet.G3));

            switch (curTest.endLR)
            {
                case NOLR:
                    break;
                case G0ToGL:
                    assertEquals(true, testSet.GL.equals(testSet.G0));
                    break;
                case G1ToGL:
                    assertEquals(true, testSet.GL.equals(testSet.G1));
                    break;
                case G2ToGL:
                    assertEquals(true, testSet.GL.equals(testSet.G2));
                    break;
                case G3ToGL:
                    assertEquals(true, testSet.GL.equals(testSet.G3));
                    break;
                case G1ToGR:
                    assertEquals(true, testSet.GR.equals(testSet.G1));
                    break;
                case G2ToGR:
                    assertEquals(true, testSet.GR.equals(testSet.G2));
                    break;
                case G3ToGR:
                    assertEquals(true, testSet.GR.equals(testSet.G3));
                    break;
                default:
                    break;
            }
        }
    }
    
    @Test
    public void constructorTest()
    {
    	RmtesBuffer rmtesBuffer;
    	RmtesCacheBuffer rmtesCacheBuffer;
    	ByteBuffer byteBuffer = ByteBuffer.allocate(100);
    	byte[] byteArray = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    	byteBuffer.put(byteArray);
    	rmtesBuffer = CodecFactory.createRmtesBuffer(10, byteBuffer, 100);
    	rmtesCacheBuffer = CodecFactory.createRmtesCacheBuffer(10, byteBuffer, 100);
    	assertEquals(10, rmtesBuffer.length());
    	assertEquals(100, rmtesBuffer.allocatedLength());
    	assertEquals(byteBuffer, rmtesBuffer.byteData());
    	
    	assertEquals(10, rmtesCacheBuffer.length());
    	assertEquals(100, rmtesCacheBuffer.allocatedLength());
    	assertEquals(byteBuffer, rmtesCacheBuffer.byteData());
    	
    	rmtesBuffer = CodecFactory.createRmtesBuffer(100);
    	rmtesCacheBuffer = CodecFactory.createRmtesCacheBuffer(100);
    	rmtesBuffer.data(byteBuffer);
    	rmtesBuffer.length(byteBuffer.position());
    	rmtesCacheBuffer.data(byteBuffer);
    	rmtesCacheBuffer.length(byteBuffer.position());
    	assertEquals(10, rmtesBuffer.length());
    	assertEquals(100, rmtesBuffer.allocatedLength());
    	assertEquals(byteBuffer, rmtesBuffer.byteData());
    	
    	assertEquals(10, rmtesCacheBuffer.length());
    	assertEquals(100, rmtesCacheBuffer.allocatedLength());
    	assertEquals(byteBuffer, rmtesCacheBuffer.byteData());
    }

}
