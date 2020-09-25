package com.refinitiv.ema.unittest;

import java.nio.ByteBuffer;

import org.junit.Test;

import com.refinitiv.ema.access.EmaUtility;

import junit.framework.TestCase;

public class EmaUtilityTests extends TestCase {
	
	public EmaUtilityTests(String name)
	{
		super(name);
	}

	@Test
	public void testAsRawHexString_fullBytes()
	{
		ByteBuffer buffer = ByteBuffer.allocate(32);

		for(int i = 1;i<=32;i++)

		{        buffer.put((byte)i); }
		buffer.rewind();
		String expectedOutput = "0102 0304 0506 0708 090A 0B0C 0D0E 0F10\n1112 1314 1516 1718 191A 1B1C 1D1E 1F20";
		String hexString = EmaUtility.asRawHexString(buffer);
		
		TestUtilities.checkResult(hexString.equals(expectedOutput), "EmaUtility.asHexString(ByteBuffer buffer) with full bytes");
	}
	
	@Test
	public void testAsRawHexString_partialBytes()
	{
		ByteBuffer buffer = ByteBuffer.allocate(34);

		for(int i = 1;i<=34;i++)

		{        buffer.put((byte)i); }
		buffer.rewind();
		String expectedOutput = "0102 0304 0506 0708 090A 0B0C 0D0E 0F10\n1112 1314 1516 1718 191A 1B1C 1D1E 1F20\n2122";
		String hexString = EmaUtility.asRawHexString(buffer);
		
		TestUtilities.checkResult(hexString.equals(expectedOutput), "EmaUtility.asHexString(ByteBuffer buffer) with partial bytes");
	}
	
	@Test
	public void testAsRawHexString_noBytes()
	{
		ByteBuffer buffer = ByteBuffer.allocate(0);

		buffer.rewind();
		String expectedOutput = "";
		String hexString = EmaUtility.asRawHexString(buffer);
		
		TestUtilities.checkResult(hexString.equals(expectedOutput), "EmaUtility.asHexString(ByteBuffer buffer) with no bytes");
	}
}
