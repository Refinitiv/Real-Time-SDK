///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.util.Arrays;

import org.junit.Test;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.RmtesBuffer;

import junit.framework.TestCase;

public class RmtesUnitTest  extends TestCase
{
	char inPartialBuf1[] = { 0x1B, 0x5B, 0x30, 0x60, 0x31, 0x32 }; // replace
																	// with 1, 2
																	// (0x31,
																	// 0x32)
																	// starting
																	// index at
																	// 0
	char inPartialBuf2[] = { 0x1B, 0x5B, 0x39, 0x60, 0x20, 0x1B, 0x5B, 0x32, 0x62 }; // replace
																						// space(0x20)
																						// starting
																						// index
																						// at
																						// 9(0x39),
																						// also
																						// repeat(0x62)
																						// 2
																						// (0x32)
																						// time.

	private com.thomsonreuters.ema.access.RmtesBuffer inputRmtesBuf = EmaFactory.createRmtesBuffer();
	private ByteBuffer inputByteBuf = ByteBuffer.allocate(30);

	private byte[] inputByte = { 0x57, 0x61, 0x69, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x4C, 0x42,
			0x4D, 0x2E, 0x2E, 0x2E };
	private String targetString = "Waiting for LBM...";
	private CharBuffer targetCharBuf = CharBuffer.allocate(30);

	private RmtesBuffer inputRmtesBuf1 = EmaFactory.createRmtesBuffer();
	private ByteBuffer inputByteBuf1 = ByteBuffer.allocate(30);

	private byte[] inputByte1 = { 0x43, 0x4F, 0x46, 0x2F, 0x4E, 0x4A };
	private String targetString1 = "COF/NJ";
	private CharBuffer targetCharBuf1 = CharBuffer.allocate(30);

	@Test
	public void testRmtesBuffer_asUTF16()
	{
		TestUtilities.printTestHead("testRmtesBuffer_asUTF16", "test RmtesBuffer function call" );

		setupFirstBuffer();

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		CharBuffer outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf).asUTF16();

		assertEquals(outputCharBuffer.length(), 18);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		clearBuffer();
	}

	@Test
	public void testRmtesBuffer_toString()
	{
		TestUtilities.printTestHead("testRmtesBuffer_toString", "test RmtesBuffer function call" );

		setupFirstBuffer();

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		String outputString = outputRmtesBuf.apply(inputRmtesBuf).toString();

		assertEquals(outputString.length(), 18);
		assertEquals(outputString.compareTo(targetString), 0);

		// test recall again
		String outputString1 = outputRmtesBuf.toString();
		assertEquals(outputString1.length(), 18);
		assertEquals(outputString1.compareTo(targetString), 0);

		clearBuffer();
	}

	@Test
	public void testRmtesBuffer_toStringAsUTF16() throws CharacterCodingException
	{
		TestUtilities.printTestHead("testRmtesBuffer_toStringAsUTF16", "test RmtesBuffer function call" );

		setupFirstBuffer();

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		String outputString = outputRmtesBuf.apply(inputRmtesBuf).toString();

		assertEquals(18, outputString.length());
		assertEquals(outputString.compareTo(targetString), 0);
		
		System.out.println(outputString);

		System.out.println(targetString);
		
		CharBuffer outputCharBuffer = outputRmtesBuf.asUTF16();

		assertEquals(outputCharBuffer.length(), 18);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		clearBuffer();
	}
	
//	@Test
//	public void testRmtesBuffer_toStringAsUTF16() throws CharacterCodingException
//	{
//		TestUtilities.printTestHead("testRmtesBuffer_toStringAsUTF16", "test RmtesBuffer function call" );
//
//		setupFirstBuffer();
//
//		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
//		String outputString = outputRmtesBuf.apply(inputRmtesBuf).toString();
//
//		System.out.println(outputString.length());
//		byte[] temp = targetString.getBytes(Charset.forName("UTF-8"));
//		
//		System.out.println("targetString legth" + temp.length  );
//
//		byte[] temp1 = outputString.getBytes(Charset.forName("UTF-8"));
//		
//		System.out.println("outputString legth" + temp1.length  );
//		
//		  String a = "ab";
//	        byte[] utf16 = null;
//	        byte[] utf8 = null;
//	    try {
//	    	utf16 = a.getBytes("UTF-16BE"); //Java default UTF-16
//	        utf8 = a.getBytes("UTF-8");
//	        
//	        CharsetDecoder decode = Charset.forName("UTF-16BE").newDecoder();
//	        System.out.println("new test " + decode.decode(ByteBuffer.wrap(utf16)).toString());
//	        System.out.println("orig test " + a);
//	        
//	    } catch (UnsupportedEncodingException e) {
//	        // TODO Auto-generated catch block
//	        e.printStackTrace();
//	    }
//
//	    for (int i = 0 ; i < utf16.length ; i ++){
//	        System.out.println("utf16 = " + utf16[i]);
//	    }
//
//	    for (int i = 0 ; i < utf8.length ; i ++){
//	        System.out.println("utf8 = " + utf8[i]);
//	    }
//         
//         
////		assertEquals(36, outputString.length());
////		assertEquals(outputString.compareTo(targetString), 0);
////		
//		System.out.println(outputString);
//
//		System.out.println(targetString);
//		
//		CharBuffer outputCharBuffer = outputRmtesBuf.asUTF16();
//
//		assertEquals(outputCharBuffer.length(), 36);
//		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
//		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);
//
//		clearBuffer();
//	}

	@Test
	public void testRmtesBuffer_clear()
	{
		TestUtilities.printTestHead("testRmtesBuffer_clear", "test RmtesBuffer function call" );

		setupFirstBuffer();
		setupSecondBuffer();

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		CharBuffer outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf).asUTF16();

		assertEquals(outputCharBuffer.length(), 18);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		 ByteBuffer outputByteBuffer = outputRmtesBuf.clear().apply(inputRmtesBuf1).asUTF8();
		 outputRmtesBuf.asUTF8();
		
		 assertEquals(outputByteBuffer.limit() - outputByteBuffer.position(), 6);
		 assertTrue(Arrays.equals(outputByteBuffer.array(), inputByte1));
		 
		 outputByteBuffer = outputRmtesBuf.asUTF8();
			
		 assertEquals(outputByteBuffer.limit() - outputByteBuffer.position(), 6);
		 assertTrue(Arrays.equals(outputByteBuffer.array(), inputByte1));

		clearBuffer();
	}

	@Test
	public void testRmtesBuffer_selfApply()
	{
		TestUtilities.printTestHead("testRmtesBuffer_selfApply", "test RmtesBuffer function call" );

		setupFirstBuffer();

		CharBuffer outputCharBuffer1 = inputRmtesBuf.asUTF16();
		assertEquals(outputCharBuffer1.length(), 18);
		assertEquals(outputCharBuffer1.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer1.compareTo(targetCharBuf), 0);

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		CharBuffer outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf).asUTF16();

		assertEquals(outputCharBuffer.length(), 18);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		clearBuffer();
	}

	@Test
	public void testRmtesBuffer_selfApplyApplyAgain()
	{
		TestUtilities.printTestHead("testRmtesBuffer_selfApplyApplyAgain", "test RmtesBuffer function call" );

		setupFirstBuffer();

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		String outputString = outputRmtesBuf.apply(inputRmtesBuf).toString();

		assertEquals(outputString.length(), 18);
		assertEquals(outputString.compareTo(targetString), 0);

		CharBuffer outputCharBuffer = outputRmtesBuf.asUTF16();

		assertEquals(outputCharBuffer.length(), 18);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		clearBuffer();
	}

	@Test
	public void testRmtesBuffer_emptyBuffer()
	{
		TestUtilities.printTestHead("testRmtesBuffer_emptyBuffer", "test RmtesBuffer function call" );

		setupEmptyBuffer();

		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		try
		{
			outputRmtesBuf.apply(inputRmtesBuf).asUTF16();
		} catch (OmmException e)
		{
			assertTrue(true);
			return;
		}

		clearBuffer();
	}

	@Test
	public void testRmtesBuffer_partialUpdate()
	{
		TestUtilities.printTestHead("testRmtesBuffer_partialUpdate", "test RmtesBuffer function call" );

		String cacheStr = "abcdefghijkl";
		inputByteBuf.put(cacheStr.getBytes());
		inputByteBuf.flip();
		JUnitTestConnect.setRsslData(inputRmtesBuf, inputByteBuf);

		targetCharBuf.clear();
		targetCharBuf.put(cacheStr);
		targetCharBuf.flip();

		// case1
		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		CharBuffer outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf).asUTF16();
		assertEquals(outputCharBuffer.length(), 12);
		assertEquals(outputCharBuffer.toString().compareTo(cacheStr), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		// apply partial
		StringBuilder str = new StringBuilder();
		inputByteBuf1.put(str.append(inPartialBuf1).toString().getBytes());
		inputByteBuf1.flip();
		JUnitTestConnect.setRsslData(inputRmtesBuf1, inputByteBuf1);

		outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf1).asUTF16();

		String targetString = "12cdefghijkl";

		targetCharBuf.clear();
		targetCharBuf.put(targetString);
		targetCharBuf.flip();

		assertEquals(outputCharBuffer.length(), 12);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		clearBuffer();

		// case2
		JUnitTestConnect.setRsslData(inputRmtesBuf, inputByteBuf);
		outputRmtesBuf.clear();
		outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf).asUTF16();

		// apply partial
		str.setLength(0);
		inputByteBuf1.clear();
		inputByteBuf1.put(str.append(inPartialBuf2).toString().getBytes());
		inputByteBuf1.flip();
		JUnitTestConnect.setRsslData(inputRmtesBuf1, inputByteBuf1);

		outputCharBuffer = outputRmtesBuf.apply(inputRmtesBuf1).asUTF16();

		targetString = "abcdefghi   ";

		targetCharBuf.clear();
		targetCharBuf.put(targetString);
		targetCharBuf.flip();

		assertEquals(outputCharBuffer.length(), 12);
		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);

		clearBuffer();
	}
	
	@Test
	public void testRmtesBuffer_asUTF8Test()
	 {
		TestUtilities.printTestHead("testRmtesBuffer_asUTF8Test", "test RmtesBuffer function call" );
	
		 setupFirstBuffer();
		
		 RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		 ByteBuffer outputByteBuffer = outputRmtesBuf.apply(inputRmtesBuf).asUTF8();
		
		 assertEquals(outputByteBuffer.limit() - outputByteBuffer.position(), 18);
		 assertTrue(Arrays.equals(outputByteBuffer.array(), inputByte));
		
		 clearBuffer();
	 }

	private void setupFirstBuffer()
	{
		targetCharBuf.clear();
		targetCharBuf.put(targetString);
		targetCharBuf.flip();

		inputByteBuf.put(inputByte);
		inputByteBuf.flip();
		JUnitTestConnect.setRsslData(inputRmtesBuf, inputByteBuf);
	}

	private void setupSecondBuffer()
	{
		targetCharBuf1.clear();
		targetCharBuf1.put(targetString1);
		targetCharBuf1.flip();

		inputByteBuf1.put(inputByte1);
		inputByteBuf1.flip();
		JUnitTestConnect.setRsslData(inputRmtesBuf1, inputByteBuf1);
	}

	private void clearBuffer()
	{
		inputRmtesBuf.clear();
		inputRmtesBuf1.clear();
	}

	private void setupEmptyBuffer()
	{
		targetCharBuf.clear();
		targetCharBuf.put("");
		targetCharBuf.flip();

		inputRmtesBuf.clear();
		JUnitTestConnect.setRsslData(inputRmtesBuf, null);
	}

//	@Test
//	public void testRmtesBuffer_applyApplyAsUTF16WithOutputBufTest()
//	{
//		TestUtilities.printTestHead("testRmtesBuffer_applyApplyAsUTF16", "test RmtesBuffer function call" );
//
//		setupFirstBuffer();
//
//		RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
//
//		// CharBuffer outBuffer = CharBuffer.allocate(100);
//		// outputRmtesBuf.apply(inputRmtesBuf).asUTF16(outBuffer);
//		//
//		// assertEquals(outBuffer.length(), 18);
//		// assertEquals(outBuffer.toString().compareTo(targetString), 0);
//
//		CharBuffer outputCharBuffer = outputRmtesBuf.asUTF16();
//
//		assertEquals(outputCharBuffer.length(), 18);
//		assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
//		assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0);
//
//		clearBuffer();
//	}
	
	// @Test
		// public void asUTF16WithOutBufferTest()
		// {
		// System.out.println("********starting asUTF16WithOutBufferTest");
		//
		// setupFirstBuffer();
		//
		// RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		//
		// CharBuffer outBuffer = CharBuffer.allocate(20);
		// outputRmtesBuf.apply(inputRmtesBuf).asUTF16(outBuffer);
		//
		// assertEquals(outBuffer.length(), 18);
		// assertEquals(outBuffer.toString().compareTo(targetString), 0);
		// assertEquals(outBuffer.compareTo(targetCharBuf), 0 );
		//
		// clearBuffer();
		// System.out.println("********ending asUTF16WithOutBufferTest\n");
		// }

	// @Test
		// void backingBufferTest()
		// {
		// System.out.println("********starting backingBufferTest");
		//
		// backingBufferTest(true);
		// backingBufferTest(false);
		//
		// clearBuffer();
		//
		// System.out.println("********ending backingBufferTest\n");
		// }

		// @Test
		// void asUTF8WithOutBufferTest()
		// {
		// System.out.println("********starting asUTF8WithOutBufferTest");
		//
		// setupFirstBuffer();
		//
		// RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		//
		// CharBuffer outBuffer = CharBuffer.allocate(100);
		// outputRmtesBuf.apply(inputRmtesBuf).asUTF8(outBuffer);
		//
		// assertEquals(outBuffer.length(), 18);
		// assertEquals(outBuffer.toString().compareTo(targetString), 0);
		// assertEquals(outBuffer.compareTo(targetCharBuf), 0 );
		//
		// CharBuffer outputCharBuffer = outputRmtesBuf.asUTF8();
		//
		// assertEquals(outputCharBuffer.length(), 18);
		// assertEquals(outputCharBuffer.toString().compareTo(targetString), 0);
		// assertEquals(outputCharBuffer.compareTo(targetCharBuf), 0 );
		//
		// outputRmtesBuf.asUTF8(outBuffer);
		//
		// assertEquals(outBuffer.length(), 18);
		// assertEquals(outBuffer.toString().compareTo(targetString), 0);
		// assertEquals(outBuffer.compareTo(targetCharBuf), 0 );
		//
		//
		// CharBuffer outBuffer1 = CharBuffer.allocate(10);
		// try
		// {
		// outputRmtesBuf.apply(inputRmtesBuf).asUTF8(outBuffer1);
		// }
		// catch (OmmException e)
		// {
		// assertTrue(true);
		// return;
		// }
		//
		// clearBuffer();
		// System.out.println("********ending asUTF8WithOutBufferTest\n");
		// }

		// @Test
		// void asUTF8WithOutBufferTest1()
		// {
		// System.out.println("********starting asUTF8WithOutBufferTest1");
		//
		// setupFirstBuffer();
		// setupSecondBuffer();
		//
		// RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		// outputRmtesBuf.apply(inputRmtesBuf).asUTF8();
		//
		// CharBuffer outBuffer = CharBuffer.allocate(100);
		// outputRmtesBuf.asUTF8(outBuffer);
		//
		// assertEquals(outBuffer.length(), 18);
		// assertEquals(outBuffer.toString().compareTo(targetString), 0);
		// assertEquals(outBuffer.compareTo(targetCharBuf), 0 );
		//
		// outputRmtesBuf.apply(inputRmtesBuf1).asUTF8(outBuffer);
		//
		// assertEquals(outBuffer.length(), 6);
		// assertEquals(outBuffer.toString().compareTo(targetString1), 0);
		// assertEquals(outBuffer.compareTo(targetCharBuf1), 0 );
		//
		// clearBuffer();
		// System.out.println("********ending asUTF8WithOutBufferTest1\n");
		// }
		//

		// void backingBufferTest(boolean clear)
		// {
		// setupFirstBuffer();
		// setupSecondBuffer();
		//
		// RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
		// CharBuffer backingBuffer = CharBuffer.allocate(100);
		// outputRmtesBuf.backingBuffer(backingBuffer);
		//
		// String outputString = outputRmtesBuf.apply(inputRmtesBuf).toString();
		//
		// assertEquals(outputString.length(), 18);
		// assertEquals(outputString.compareTo(targetString), 0);
		//
		// if ( clear == true)
		// outputRmtesBuf.clear();
		//
		// CharBuffer outputCharBuffer =
		// outputRmtesBuf.apply(inputRmtesBuf1).asUTF8();
		//
		// assertEquals(outputCharBuffer.length(), 6);
		// assertEquals(outputCharBuffer.toString().compareTo(targetString1), 0);
		// assertEquals(outputCharBuffer.compareTo(targetCharBuf1), 0 );
		// }
	
	// @Test
	// void backingBufferAsUTF16WithOutBufferTest()
	// {
	// System.out.println("********starting
	// backingBufferAsUTF16WithOutBufferTest");
	//
	// setupFirstBuffer();
	// setupSecondBuffer();
	//
	// RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
	// CharBuffer backingBuffer = CharBuffer.allocate(10);
	// outputRmtesBuf.backingBuffer(backingBuffer);
	//
	// CharBuffer outBuffer = CharBuffer.allocate(20);
	// outputRmtesBuf.apply(inputRmtesBuf1).asUTF16(outBuffer);
	//
	// assertEquals(outBuffer.length(), 6);
	// assertEquals(outBuffer.toString().compareTo(targetString1), 0);
	// assertEquals(outBuffer.compareTo(targetCharBuf1), 0 );
	//
	// try
	// {
	// outputRmtesBuf.apply(inputRmtesBuf).asUTF16();
	// }
	// catch (OmmException e)
	// {
	// assertTrue(true);
	// return;
	// }
	//
	// clearBuffer();
	// System.out.println("********ending
	// backingBufferAsUTF16WithOutBufferTest\n");
	// }
	//

}
