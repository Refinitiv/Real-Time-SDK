///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.codec;

import java.math.BigDecimal;
import java.nio.ByteBuffer;

import org.junit.Test;

import static org.junit.Assert.*;

public class RealJunit 
{
	private void testRealToString(double val, int hint)
	{
		Real real = CodecFactory.createReal();
		real.value(val, hint);
		String s;
		
		if( RealHints.EXPONENT_14 <= hint && hint <= RealHints.EXPONENT_1)
		{
			s = String.format(RealImpl.decimalStringFormat[hint],val);
		}
		else
		{
			s = String.valueOf(val);
		}
		
		assertTrue(s.equals(real.toString()));
	}
	
	private void testRealToDouble(String s)
	{
		Real real = CodecFactory.createReal();
		real.value(s);
		Double  tst = CodecFactory.createDouble();
		tst.value(real.toDouble());
		assertTrue(real.toDouble() == tst.toDouble());
	}
	
	private void testDoubleToReal(double val, int hint)
	{
		Double  dbl = CodecFactory.createDouble();
		dbl.value(val);
		Real real = CodecFactory.createReal();
		real = dbl.toReal(hint);
		assertTrue(val == real.toDouble());
	}
	
	private void testDoubleToRealFraction(double val, int hint)
	{
		Real real = CodecFactory.createReal();
		real.value(val, hint);
		double tst = real.toDouble();
		assertTrue(val == tst);
	}
	
	private void testDoubleToRealDecimal(double val, int hint)
	{
		Real real = CodecFactory.createReal();
		real.value(val, hint);
		double tst = real.toDouble();
		assertTrue(val == tst);		
	}

	private void testRealToBigDecimal(String val)
	{
		Real real = CodecFactory.createReal();
		real.value(val);
		BigDecimal tst = new BigDecimal(real.toBigDecimal().toString());
		assertEquals(tst, real.toBigDecimal());
	}

	private void testRealToBigDecimalWhenEmptyValue() {
		Real real = CodecFactory.createReal();
		real.value("");
		BigDecimal tst = real.toBigDecimal();
		assertNull(tst);
	}

	private void testRealToBigDecimalWhenBadValue(double val, int hint) {
		Real real = CodecFactory.createReal();
		real.value(val, hint);
		BigDecimal tst = real.toBigDecimal();
		assertNull(tst);
	}
	
	private void testDoubleRealCompare(double dblVal, long val, int hint)
	{
		Real real = CodecFactory.createReal();
		real.value(dblVal, hint);
		assertTrue(val == real.toLong());		
		assertEquals(hint, real.hint());
	}
	
	private void testRealField()
	{
		FieldEntry entry = CodecFactory.createFieldEntry();
		FieldList fieldList = CodecFactory.createFieldList();
		Real real = CodecFactory.createReal();
		Real decReal = CodecFactory.createReal();
		Real encReal = CodecFactory.createReal();
		Real outReal = CodecFactory.createReal();
		EncodeIterator encIter = CodecFactory.createEncodeIterator();
		DecodeIterator decIter = CodecFactory.createDecodeIterator();
		LocalFieldSetDefDb localFieldSetDefDb = CodecFactory.createLocalFieldSetDefDb();
		localFieldSetDefDb.encode(encIter);
		localFieldSetDefDb.decode(decIter);
		int majorVersion = Codec.majorVersion();  //This should be initialized to the MAJOR version of RWF being encoded
		int minorVersion = Codec.minorVersion();  // This should be initialized to the MINOR version of RWF being encoded
		Buffer _buffer = CodecFactory.createBuffer();
		_buffer.data(ByteBuffer.allocate(50));
		
		fieldList.clear();
		fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);
		encIter.clear();
		encIter.setBufferAndRWFVersion(_buffer, majorVersion, minorVersion);
		
		/* init */
		entry.clear();
		assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldListInit(encIter, fieldList, localFieldSetDefDb, 0));
		entry.fieldId(1);
		entry.dataType(DataTypes.REAL);
		
		/* 32-bit 64-bit real */
		real.blank();
		real.value(65535, RealHints.EXPONENT_2);
		assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldEntry(encIter,  entry,  real));
		
		/* 64-bit real */
		entry.clear();
		entry.fieldId(2);
		entry.dataType(DataTypes.REAL);
		real.blank();
		real.value(68719476735L, RealHints.EXPONENT_2);
		
		assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldEntry(encIter,  entry, real));
		
		entry.clear();
		entry.fieldId(3);
		entry.dataType(DataTypes.REAL);
		
		/* 32-bit 64-bit real */
		encReal.blank();
		encReal.value(65535, RealHints.EXPONENT_2);
		
		assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldEntry(encIter, entry, encReal));
		
		/* finish encoding */
		assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldListComplete(encIter, true));
		
		// Setup Container
		fieldList.clear();
		entry.clear();
		decIter.clear();
		decIter.setBufferAndRWFVersion(_buffer, majorVersion, minorVersion);
		
		// Begin container decoding
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeFieldList(decIter, fieldList, localFieldSetDefDb));
		
		assertEquals(fieldList.flags(), FieldListFlags.HAS_STANDARD_DATA);
		
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeFieldEntry(decIter, entry));
		
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeReal(decIter, decReal));
		
		Boolean correctDecReal = false;
		if (!decReal.isBlank() && decReal.hint() == RealHints.EXPONENT_2 && decReal.toLong() == 65535)
			correctDecReal = true;
		assertTrue(correctDecReal);
		
		entry.clear();
		
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeFieldEntry(decIter, entry));
		
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeReal(decIter, outReal));
		
		correctDecReal = false;
		if (!outReal.isBlank() && outReal.hint() == RealHints.EXPONENT_2 && outReal.toLong() == 68719476735L)
			correctDecReal = true;
		assertTrue(correctDecReal);
		
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeFieldEntry(decIter, entry));
		
		assertEquals(CodecReturnCodes.SUCCESS, Decoders.decodeReal(decIter, decReal));
		
		correctDecReal = false;
		if (!decReal.isBlank() && decReal.hint() == RealHints.EXPONENT_2 && decReal.toLong() == 65535)
			correctDecReal = true;
		assertTrue(correctDecReal);
		
	}
	
	@Test
	public void realMaximumAndMinimumValuesTest() 
	{
		Real real = CodecFactory.createReal();
		int ret  = real.value("9223372036854775807");
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertEquals(Long.MAX_VALUE, real.toLong());
		assertEquals(RealHints.EXPONENT0, real.hint());
		
		real.clear();
		ret  = real.value("922337203685477580.7");
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertEquals(Long.MAX_VALUE, real.toLong());
		assertEquals(RealHints.EXPONENT_1, real.hint());
		
		real.clear();
		ret  = real.value("-9223372036854775808");
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertEquals(Long.MIN_VALUE, real.toLong());
		assertEquals(RealHints.EXPONENT0, real.hint());
		
		real.clear();
		ret  = real.value("-922337203685477580.8");
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertEquals(Long.MIN_VALUE, real.toLong());
		assertEquals(RealHints.EXPONENT_1, real.hint());
		
		real.clear();
		ret  = real.value("-9223372036854775807");
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertEquals(Long.MIN_VALUE+1, real.toLong());
		assertEquals(RealHints.EXPONENT0, real.hint());
		
		real.clear();
		ret  = real.value("-922337203685477580.7");
		
		assertEquals(CodecReturnCodes.SUCCESS, ret);
		assertEquals(Long.MIN_VALUE+1, real.toLong());
		assertEquals(RealHints.EXPONENT_1, real.hint());
	}
	
	@Test
	public void realOverflowValuesTest() 
	{
		Real real = CodecFactory.createReal();
		int ret  = real.value("9223372036854775808");
		
		assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);

		real.clear();
		ret  = real.value("922337203685477580.8");
		
		assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
		
		real.clear();
		ret  = real.value("-9223372036854775809");
		
		assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
		
		real.clear();
		ret  = real.value("-922337203685477580.9");
		
		assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
	}
	
	@Test
	public void realTest() 
	{
		//Real Related Conversion test
		testRealToString((double)20/4, RealHints.FRACTION_4);
		testRealToString((double)100/256, RealHints.FRACTION_256);
		testRealToString((double)-20/128, RealHints.FRACTION_128);
		testRealToString((double)123456/64, RealHints.FRACTION_64);
		testRealToString(1.5, RealHints.FRACTION_2);
		testRealToString(-1.5, RealHints.FRACTION_2);
		testRealToString(0.5, RealHints.FRACTION_2);
		testRealToString(5, RealHints.FRACTION_2);
		testRealToString(1.5, RealHints.FRACTION_4);
		testRealToString(-1.5, RealHints.FRACTION_4);
		testRealToString(0.5, RealHints.FRACTION_4);
		testRealToString(12.25, RealHints.FRACTION_4);
		testRealToString(-1.75, RealHints.FRACTION_4);
		testRealToString(0.75, RealHints.FRACTION_4);
		testRealToString(5, RealHints.FRACTION_4);
		testRealToString(1.125, RealHints.FRACTION_8);
		testRealToString(1.0 + 1.0/256.0, RealHints.FRACTION_256);

		testRealToString(210, RealHints.EXPONENT1);
		testRealToString(2100, RealHints.EXPONENT2);
		testRealToString(21000, RealHints.EXPONENT3);
		testRealToString(210000, RealHints.EXPONENT4);
		testRealToString(2100000, RealHints.EXPONENT5);
		testRealToString(21000000, RealHints.EXPONENT6);
		testRealToString(210000000, RealHints.EXPONENT7);
		testRealToString(20, RealHints.EXPONENT_1);
		testRealToString(0, RealHints.EXPONENT_1);
		testRealToString(-20, RealHints.EXPONENT_1);
		testRealToString(123456, RealHints.EXPONENT_1); 
		testRealToString(1.5, RealHints.EXPONENT_1);
		testRealToString(-1.5, RealHints.EXPONENT_1);
		testRealToString(0.5, RealHints.EXPONENT_1);
		testRealToString(5, RealHints.EXPONENT_1);
		testRealToString(1.5, RealHints.EXPONENT_2);
		testRealToString(-1.5, RealHints.EXPONENT_2);
		testRealToString(0.5, RealHints.EXPONENT_2);
		testRealToString(12.25, RealHints.EXPONENT_2);
		testRealToString(-1.75, RealHints.EXPONENT_2);
		testRealToString(0.75, RealHints.EXPONENT_2);
		testRealToString(5, RealHints.EXPONENT_2);
		testRealToString(1.125, RealHints.EXPONENT_3);
		testRealToString(78989393.12558696, RealHints.EXPONENT_8);
		testRealToString(93931255869.8, RealHints.EXPONENT_7);
		testRealToString(78919.555, RealHints.EXPONENT_14);
		testRealToString(78919.55, RealHints.EXPONENT_13);
		testRealToString(210.0, RealHints.EXPONENT1);
		testRealToString(2100.00, RealHints.EXPONENT2);
		testRealToString(21000.000, RealHints.EXPONENT3);
		testRealToString(210000.0000, RealHints.EXPONENT4);
		testRealToString(2100000.00000, RealHints.EXPONENT5);
		testRealToString(21000000.000000, RealHints.EXPONENT6);
		testRealToString(210000000.0000000, RealHints.EXPONENT7);
		
		testRealToDouble("-12");
		testRealToDouble("-12.");
		testRealToDouble("-12.0");
		testRealToDouble("-12.1");
		testRealToDouble("-12.12");
		testRealToDouble("-12.123");
		testRealToDouble("-12.1234");
		testRealToDouble("-12.12345");
		testRealToDouble("-12.123456");
		testRealToDouble("-12.1234567");
		testRealToDouble("-12.12345678");
		testRealToDouble("-1.123456789"); 
		testRealToDouble("12");
		testRealToDouble("12.");
		testRealToDouble("12.0");
		testRealToDouble("12.1");
		testRealToDouble("12.12");
		testRealToDouble("12.123");
		testRealToDouble("12.1234");
		testRealToDouble("12.12345");
		testRealToDouble("12.123456");
		testRealToDouble("12.1234567");
		testRealToDouble("12.12345678");
		testRealToDouble("1.123456789");
		testRealToDouble("-12.123456789");			
		testRealToDouble("12.123456789");
		testRealToDouble("12.000000");
		testRealToDouble("0.000000");
		testRealToDouble("0.000001");
		testRealToDouble("-0.000001");
		testRealToDouble("-12.000000");
		testRealToDouble(".1");
		testRealToDouble("0");  
		testRealToDouble("12 1/2");
		testRealToDouble("12 1/4");
		testRealToDouble("12 2/8");
		testRealToDouble("12 2/16");
		testRealToDouble("12 2/32");
		testRealToDouble("12 2/64");
		testRealToDouble("12 2/128");
		testRealToDouble("12 2/256"); 
		testRealToDouble("-12 1/2");
		testRealToDouble("-12 1/4");
		testRealToDouble("-12 2/8");
		testRealToDouble("-12 2/16");
		testRealToDouble("-12 2/32");
		testRealToDouble("-12 2/64");
		testRealToDouble("-12 2/128");
		testRealToDouble("-12 2/256");
		testRealToDouble("-12 0/2");
		testRealToDouble("12.1234");

		testRealToBigDecimal("-12");
		testRealToBigDecimal("-12.");
		testRealToBigDecimal("-12.0");
		testRealToBigDecimal("-12.1");
		testRealToBigDecimal("-12.12");
		testRealToBigDecimal("-12.123");
		testRealToBigDecimal("-12.1234");
		testRealToBigDecimal("-12.12345");
		testRealToBigDecimal("-12.123456");
		testRealToBigDecimal("-12.1234567");
		testRealToBigDecimal("-12.12345678901234567890123");
		testRealToBigDecimal("-1.12345678901234567890123");
		testRealToBigDecimal("12");
		testRealToBigDecimal("12.");
		testRealToBigDecimal("12.0");
		testRealToBigDecimal("12.1");
		testRealToBigDecimal("12.12");
		testRealToBigDecimal("12.123");
		testRealToBigDecimal("12.1234");
		testRealToBigDecimal("12.12345");
		testRealToBigDecimal("12.123456");
		testRealToBigDecimal("12.1234567");
		testRealToBigDecimal("12.12345678901234567890123");
		testRealToBigDecimal("1.12345678901234567890123");
		testRealToBigDecimal("12.000000");
		testRealToBigDecimal("0.000000");
		testRealToBigDecimal("0.000001");
		testRealToBigDecimal("-0.000001");
		testRealToBigDecimal("-12.000000");
		testRealToBigDecimal(".1");
		testRealToBigDecimal("0");
		testRealToBigDecimal("12 1/2");
		testRealToBigDecimal("12 1/4");
		testRealToBigDecimal("12 2/8");
		testRealToBigDecimal("12 2/16");
		testRealToBigDecimal("12 2/32");
		testRealToBigDecimal("12 2/64");
		testRealToBigDecimal("12 2/128");
		testRealToBigDecimal("12 2/256");
		testRealToBigDecimal("12 2/512");
		testRealToBigDecimal("12 2/1024");
		testRealToBigDecimal("-12 1/2");
		testRealToBigDecimal("-12 1/4");
		testRealToBigDecimal("-12 2/8");
		testRealToBigDecimal("-12 2/16");
		testRealToBigDecimal("-12 2/32");
		testRealToBigDecimal("-12 2/64");
		testRealToBigDecimal("-12 2/128");
		testRealToBigDecimal("-12 2/256");
		testRealToBigDecimal("-12 2/512");
		testRealToBigDecimal("-12 2/1024");
		testRealToBigDecimal("-12 0/2");
		testRealToBigDecimal("12.1234");
		testRealToBigDecimalWhenEmptyValue();
		testRealToBigDecimalWhenBadValue(java.lang.Double.NaN, RealHints.EXPONENT1);
		testRealToBigDecimalWhenBadValue(java.lang.Double.POSITIVE_INFINITY, RealHints.EXPONENT2);
		testRealToBigDecimalWhenBadValue(java.lang.Double.NEGATIVE_INFINITY, RealHints.EXPONENT3);
		
		testDoubleToReal((double)20/4, RealHints.FRACTION_4);
		testDoubleToReal((double)100/256, RealHints.FRACTION_256);
		testDoubleToReal((double)-20/128, RealHints.FRACTION_128);
		testDoubleToReal((double)123456/64, RealHints.FRACTION_64);
		testDoubleToReal(1.5, RealHints.FRACTION_2);
		testDoubleToReal(-1.5, RealHints.FRACTION_2);
		testDoubleToReal(0.5, RealHints.FRACTION_2);
		testDoubleToReal(5, RealHints.FRACTION_2);
		testDoubleToReal(1.5, RealHints.FRACTION_4);
		testDoubleToReal(-1.5, RealHints.FRACTION_4);
		testDoubleToReal(0.5, RealHints.FRACTION_4);
		testDoubleToReal(12.25, RealHints.FRACTION_4);
		testDoubleToReal(-1.75, RealHints.FRACTION_4);
		testDoubleToReal(0.75, RealHints.FRACTION_4);
		testDoubleToReal(5, RealHints.FRACTION_4);
		testDoubleToReal(1.125, RealHints.FRACTION_8);
		testDoubleToReal(1.0 + 1.0/256.0, RealHints.FRACTION_256);

		testDoubleToReal(210, RealHints.EXPONENT1);
		testDoubleToReal(2100, RealHints.EXPONENT2);
		testDoubleToReal(21000, RealHints.EXPONENT3);
		testDoubleToReal(210000, RealHints.EXPONENT4);
		testDoubleToReal(2100000, RealHints.EXPONENT5);
		testDoubleToReal(21000000, RealHints.EXPONENT6);
		testDoubleToReal(210000000, RealHints.EXPONENT7);
		testDoubleToReal(20, RealHints.EXPONENT_1);
		testDoubleToReal(0, RealHints.EXPONENT1);
		testDoubleToReal(-20, RealHints.EXPONENT_1);
		testDoubleToReal(123456, RealHints.EXPONENT_1); 
		testDoubleToReal(1.5, RealHints.EXPONENT_1);
		testDoubleToReal(-1.5, RealHints.EXPONENT_1);
		testDoubleToReal(0.5, RealHints.EXPONENT_1);
		testDoubleToReal(5, RealHints.EXPONENT_1);
		testDoubleToReal(1.5, RealHints.EXPONENT_2);
		testDoubleToReal(-1.5, RealHints.EXPONENT_2);
		testDoubleToReal(0.5, RealHints.EXPONENT_2);
		testDoubleToReal(12.25, RealHints.EXPONENT_2);
		testDoubleToReal(-1.75, RealHints.EXPONENT_2);
		testDoubleToReal(0.75, RealHints.EXPONENT_2);
		testDoubleToReal(5, RealHints.EXPONENT_2);
		testDoubleToReal(1.125, RealHints.EXPONENT_3);

		testDoubleToRealFraction((double)20/4, RealHints.FRACTION_4);
		testDoubleToRealFraction((double)100/256, RealHints.FRACTION_256);
		testDoubleToRealFraction((double)-20/128, RealHints.FRACTION_128);
		testDoubleToRealFraction((double)123456/64, RealHints.FRACTION_64);
		testDoubleToRealFraction(1.5, RealHints.FRACTION_2);
		testDoubleToRealFraction(-1.5, RealHints.FRACTION_2);
		testDoubleToRealFraction(0.5, RealHints.FRACTION_2);
		testDoubleToRealFraction(5, RealHints.FRACTION_2);
		testDoubleToRealFraction(1.5, RealHints.FRACTION_4);
		testDoubleToRealFraction(-1.5, RealHints.FRACTION_4);
		testDoubleToRealFraction(0.5, RealHints.FRACTION_4);
		testDoubleToRealFraction(12.25, RealHints.FRACTION_4);
		testDoubleToRealFraction(-1.75, RealHints.FRACTION_4);
		testDoubleToRealFraction(0.75, RealHints.FRACTION_4);
		testDoubleToRealFraction(5, RealHints.FRACTION_4);
		testDoubleToRealFraction(1.125, RealHints.FRACTION_8);
		testDoubleToRealFraction(1.0 + 1.0/256.0, RealHints.FRACTION_256);

		testDoubleToRealDecimal(210, RealHints.EXPONENT1);
		testDoubleToRealDecimal(2100, RealHints.EXPONENT2);
		testDoubleToRealDecimal(21000, RealHints.EXPONENT3);
		testDoubleToRealDecimal(210000, RealHints.EXPONENT4);
		testDoubleToRealDecimal(2100000, RealHints.EXPONENT5);
		testDoubleToRealDecimal(21000000, RealHints.EXPONENT6);
		testDoubleToRealDecimal(210000000, RealHints.EXPONENT7);
		testDoubleToRealDecimal(20, RealHints.EXPONENT_1);
		testDoubleToRealDecimal(0, RealHints.EXPONENT1);
		testDoubleToRealDecimal(-20, RealHints.EXPONENT_1);
		testDoubleToRealDecimal(123456, RealHints.EXPONENT_1); 
		testDoubleToRealDecimal(1.5, RealHints.EXPONENT_1);
		testDoubleToRealDecimal(-1.5, RealHints.EXPONENT_1);
		testDoubleToRealDecimal(0.5, RealHints.EXPONENT_1);
		testDoubleToRealDecimal(5, RealHints.EXPONENT_1);
		testDoubleToRealDecimal(1.5, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(-1.5, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(0.5, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(12.25, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(-1.75, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(0.75, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(5, RealHints.EXPONENT_2);
		testDoubleToRealDecimal(1.125, RealHints.EXPONENT_3);

		testDoubleRealCompare(.35, 35, RealHints.EXPONENT_2);
		testDoubleRealCompare(100000000.1, 1000000001, RealHints.EXPONENT_1);
		testDoubleRealCompare(100000001.1, 1000000011, RealHints.EXPONENT_1);
		testDoubleRealCompare(900000000.1, 9000000001L, RealHints.EXPONENT_1);
		testDoubleRealCompare(100010000.1, 1000100001, RealHints.EXPONENT_1);
		testDoubleRealCompare(111111111.1, 1111111111, RealHints.EXPONENT_1);
		testDoubleRealCompare(11111111.1, 111111111, RealHints.EXPONENT_1);
		testDoubleRealCompare(1.1, 11, RealHints.EXPONENT_1);
		testDoubleRealCompare(111.1, 1111, RealHints.EXPONENT_1);
		testDoubleRealCompare(11.1, 111, RealHints.EXPONENT_1);
		testDoubleRealCompare(0.00001, 1, RealHints.EXPONENT_5);
		testDoubleRealCompare(100000000.2, 1000000002, RealHints.EXPONENT_1);
		testDoubleRealCompare(2000.2, 20002, RealHints.EXPONENT_1);
		testDoubleRealCompare(20000.2, 200002, RealHints.EXPONENT_1);
		testDoubleRealCompare(200000.2, 2000002, RealHints.EXPONENT_1);
		testDoubleRealCompare(2000000.2, 20000002, RealHints.EXPONENT_1);
		testDoubleRealCompare(20000000.2, 200000002, RealHints.EXPONENT_1);
		testDoubleRealCompare(200000000.2, 2000000002, RealHints.EXPONENT_1);
		testDoubleRealCompare(987.0123456789012,9870123456789012L, RealHints.EXPONENT_13);

		//extensive double/real test
		for(int i = 0; i < 13; i++)
		{
			for(int j = 0; j < 10000; j++)
			{
				switch(i)
				{
					case(12):
						testDoubleRealCompare((.000000000001)+(.000000000001)*j, (long) Math.floor(((.000000000001+.000000000001*j)*(1000000000000L)) + 0.5), RealHints.EXPONENT_12);	
						break;
					case(11):
						testDoubleRealCompare((.00000000001)+(.00000000001)*j, (long) Math.floor(((.00000000001+.00000000001*j)*(100000000000L)) + 0.5), RealHints.EXPONENT_11);
						break;
					case(10):
						testDoubleRealCompare((.0000000001)+(.0000000001)*j, (long) Math.floor(((.0000000001+.0000000001*j)*(10000000000L)) + 0.5), RealHints.EXPONENT_10);
						break;
					case(9):
						testDoubleRealCompare((.000000001)+(.000000001)*j, (long) Math.floor(((.000000001+.000000001*j)*(1000000000)) + 0.5), RealHints.EXPONENT_9);
						break;
					case(8):
						testDoubleRealCompare((.00000001)+(.00000001)*j, (long) Math.floor(((.00000001+.00000001*j)*(100000000)) + 0.5), RealHints.EXPONENT_8);
						break;
					case(7):
						testDoubleRealCompare((.0000001)+(.0000001)*j, (long) Math.floor(((.0000001+.0000001*j)*(10000000)) + 0.5), RealHints.EXPONENT_7);
						break;
					case(6):
						testDoubleRealCompare((.000001)+(.000001)*j, (long) Math.floor(((.000001+.000001*j)*(1000000)) + 0.5), RealHints.EXPONENT_6);
						break;
					case(5):
						testDoubleRealCompare((.00001)+(.00001)*j, (long) Math.floor(((.00001+.00001*j)*(100000)) + 0.5), RealHints.EXPONENT_5);
						break;
					case(4):
						testDoubleRealCompare((.0001)+(.0001)*j, (long) Math.floor(((.0001+.0001*j)*(10000) +0.5)), RealHints.EXPONENT_4);
						break;
					case(3):
						testDoubleRealCompare((.001)+(.001)*j, (long) Math.floor(((.001+.001*j)*(1000)) +0.5), RealHints.EXPONENT_3);
						break;
					case(2):
						testDoubleRealCompare((.01)+(.01)*j, (long) Math.floor(((.01+.01*j)*(100)) +0.5), RealHints.EXPONENT_2);
						break;
					case(1):
						testDoubleRealCompare((.1)+(.1)*j, (long) Math.floor(((.1+.1*j)*(10)) +0.5), RealHints.EXPONENT_1);
						break;
					case(0):
						testDoubleRealCompare(1+1*j, (long) Math.floor((1+1*j) + 0.5), RealHints.EXPONENT0);
						break;
					default:
						break;
				}
			}
		}

		testDoubleToRealDecimal(0.123456789, RealHints.EXPONENT_9);
		testDoubleToRealDecimal(1.23456789, RealHints.EXPONENT_9);		 
		testDoubleToRealDecimal(1.2, RealHints.EXPONENT_9);				
		
		testRealField();

		testDoubleToRealOutOfRange(1000000000., RealHints.EXPONENT_14);
		testDoubleToRealOutOfRange(-1000000000., RealHints.EXPONENT_14);
		testDoubleToRealOutOfRange(1000000000., RealHints.EXPONENT_13);
		testDoubleToRealOutOfRange(-1000000000., RealHints.EXPONENT_13);
		testDoubleToRealOutOfRange(1000000000., RealHints.EXPONENT_12);
		testDoubleToRealOutOfRange(-1000000000., RealHints.EXPONENT_12);
		testDoubleToRealOutOfRange(1000000000., RealHints.EXPONENT_11);
		testDoubleToRealOutOfRange(-1000000000., RealHints.EXPONENT_11);
		testDoubleToRealOutOfRange(1000000000., RealHints.EXPONENT_10);
		testDoubleToRealOutOfRange(-1000000000., RealHints.EXPONENT_10);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_9);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_9);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_8);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_8);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_7);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_7);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_6);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_6);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_5);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_5);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_4);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_4);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_3);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_3);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_2);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_2);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT_1);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT_1);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT0);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT0);
		testDoubleToRealOutOfRange(1e20, RealHints.EXPONENT1);
		testDoubleToRealOutOfRange(-1e20, RealHints.EXPONENT1);
		testDoubleToRealOutOfRange(1e30, RealHints.EXPONENT2);
		testDoubleToRealOutOfRange(-1e30, RealHints.EXPONENT2);
		testDoubleToRealOutOfRange(1e30, RealHints.EXPONENT3);
		testDoubleToRealOutOfRange(-1e30, RealHints.EXPONENT3);
		testDoubleToRealOutOfRange(1e30, RealHints.EXPONENT4);
		testDoubleToRealOutOfRange(-1e30, RealHints.EXPONENT4);
		testDoubleToRealOutOfRange(1e30, RealHints.EXPONENT5);
		testDoubleToRealOutOfRange(-1e30, RealHints.EXPONENT5);
		testDoubleToRealOutOfRange(1e30, RealHints.EXPONENT6);
		testDoubleToRealOutOfRange(-1e30, RealHints.EXPONENT6);
		testDoubleToRealOutOfRange(1e30, RealHints.EXPONENT7);
		testDoubleToRealOutOfRange(-1e30, RealHints.EXPONENT7);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_1);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_1);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_2);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_2);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_4);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_4);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_8);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_8);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_16);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_16);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_32);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_32);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_64);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_64);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_128);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_128);
		testDoubleToRealOutOfRange(1e20, RealHints.FRACTION_256);
		testDoubleToRealOutOfRange(-1e20, RealHints.FRACTION_256);

		testFloatToRealOutOfRange(1e20f, RealHints.EXPONENT0);
		testFloatToRealOutOfRange(-1e20f, RealHints.EXPONENT0);
		testFloatToRealOutOfRange(1000000.f, RealHints.EXPONENT_14);
		testFloatToRealOutOfRange(-1000000.f, RealHints.EXPONENT_14);
		testFloatToRealOutOfRange(1e30f, RealHints.EXPONENT7);
		testFloatToRealOutOfRange(-1e30f, RealHints.EXPONENT7);
		testFloatToRealOutOfRange(1e20f, RealHints.FRACTION_256);
		testFloatToRealOutOfRange(-1e20f, RealHints.FRACTION_256);

		testDoubleToRealMaximumMinimumValue(1e14, RealHints.EXPONENT_14);
		testDoubleToRealMaximumMinimumValue(1e13, RealHints.EXPONENT_13);
		testDoubleToRealMaximumMinimumValue(1e12, RealHints.EXPONENT_12);
		testDoubleToRealMaximumMinimumValue(1e11, RealHints.EXPONENT_11);
		testDoubleToRealMaximumMinimumValue(1e10, RealHints.EXPONENT_10);
		testDoubleToRealMaximumMinimumValue(1e9, RealHints.EXPONENT_9);
		testDoubleToRealMaximumMinimumValue(1e8, RealHints.EXPONENT_8);
		testDoubleToRealMaximumMinimumValue(1e7, RealHints.EXPONENT_7);
		testDoubleToRealMaximumMinimumValue(1e6, RealHints.EXPONENT_6);
		testDoubleToRealMaximumMinimumValue(1e5, RealHints.EXPONENT_5);
		testDoubleToRealMaximumMinimumValue(1e4, RealHints.EXPONENT_4);
		testDoubleToRealMaximumMinimumValue(1e3, RealHints.EXPONENT_3);
		testDoubleToRealMaximumMinimumValue(100., RealHints.EXPONENT_2);
		testDoubleToRealMaximumMinimumValue(10., RealHints.EXPONENT_1);
		testDoubleToRealMaximumMinimumValue(1., RealHints.EXPONENT0);
		testDoubleToRealMaximumMinimumValue(.1, RealHints.EXPONENT1);
		testDoubleToRealMaximumMinimumValue(.01, RealHints.EXPONENT2);
		testDoubleToRealMaximumMinimumValue(1e-3, RealHints.EXPONENT3);
		testDoubleToRealMaximumMinimumValue(1e-4, RealHints.EXPONENT4);
		testDoubleToRealMaximumMinimumValue(1e-5, RealHints.EXPONENT5);
		testDoubleToRealMaximumMinimumValue(1e-6, RealHints.EXPONENT6);
		testDoubleToRealMaximumMinimumValue(1e-7, RealHints.EXPONENT7);
		testDoubleToRealMaximumMinimumValue(1., RealHints.FRACTION_1);
		testDoubleToRealMaximumMinimumValue(2., RealHints.FRACTION_2);
		testDoubleToRealMaximumMinimumValue(4., RealHints.FRACTION_4);
		testDoubleToRealMaximumMinimumValue(8., RealHints.FRACTION_8);
		testDoubleToRealMaximumMinimumValue(16., RealHints.FRACTION_16);
		testDoubleToRealMaximumMinimumValue(32., RealHints.FRACTION_32);
		testDoubleToRealMaximumMinimumValue(64., RealHints.FRACTION_64);
		testDoubleToRealMaximumMinimumValue(128., RealHints.FRACTION_128);
		testDoubleToRealMaximumMinimumValue(256., RealHints.FRACTION_256);

		testFloatToRealMaximumMinimumValue(1e14f, RealHints.EXPONENT_14);
		testFloatToRealMaximumMinimumValue(1e13f, RealHints.EXPONENT_13);
		testFloatToRealMaximumMinimumValue(1e12f, RealHints.EXPONENT_12);
		testFloatToRealMaximumMinimumValue(1e11f, RealHints.EXPONENT_11);
		testFloatToRealMaximumMinimumValue(1e10f, RealHints.EXPONENT_10);
		testFloatToRealMaximumMinimumValue(1e9f, RealHints.EXPONENT_9);
		testFloatToRealMaximumMinimumValue(1e8f, RealHints.EXPONENT_8);
		testFloatToRealMaximumMinimumValue(1e7f, RealHints.EXPONENT_7);
		testFloatToRealMaximumMinimumValue(1e6f, RealHints.EXPONENT_6);
		testFloatToRealMaximumMinimumValue(1e5f, RealHints.EXPONENT_5);
		testFloatToRealMaximumMinimumValue(1e4f, RealHints.EXPONENT_4);
		testFloatToRealMaximumMinimumValue(1e3f, RealHints.EXPONENT_3);
		testFloatToRealMaximumMinimumValue(100.f, RealHints.EXPONENT_2);
		testFloatToRealMaximumMinimumValue(10.f, RealHints.EXPONENT_1);
		testFloatToRealMaximumMinimumValue(1.f, RealHints.EXPONENT0);
		testFloatToRealMaximumMinimumValue(.1f, RealHints.EXPONENT1);
		testFloatToRealMaximumMinimumValue(.01f, RealHints.EXPONENT2);
		testFloatToRealMaximumMinimumValue(1e-3f, RealHints.EXPONENT3);
		testFloatToRealMaximumMinimumValue(1e-4f, RealHints.EXPONENT4);
		testFloatToRealMaximumMinimumValue(1e-5f, RealHints.EXPONENT5);
		testFloatToRealMaximumMinimumValue(1e-6f, RealHints.EXPONENT6);
		testFloatToRealMaximumMinimumValue(1e-7f, RealHints.EXPONENT7);
		testFloatToRealMaximumMinimumValue(1.f, RealHints.FRACTION_1);
		testFloatToRealMaximumMinimumValue(2.f, RealHints.FRACTION_2);
		testFloatToRealMaximumMinimumValue(4.f, RealHints.FRACTION_4);
		testFloatToRealMaximumMinimumValue(8.f, RealHints.FRACTION_8);
		testFloatToRealMaximumMinimumValue(16.f, RealHints.FRACTION_16);
		testFloatToRealMaximumMinimumValue(32.f, RealHints.FRACTION_32);
		testFloatToRealMaximumMinimumValue(64.f, RealHints.FRACTION_64);
		testFloatToRealMaximumMinimumValue(128.f, RealHints.FRACTION_128);
		testFloatToRealMaximumMinimumValue(256.f, RealHints.FRACTION_256);
	}
	
	@Test
	public void realNaNTest() 
	{
		Real  testReal = CodecFactory.createReal();
		
		// test double with NaN
		testReal.value(java.lang.Double.NaN, 0);
		assertTrue(java.lang.Double.isNaN(testReal.toDouble()));
		assertTrue(testReal.hint() == RealHints.NOT_A_NUMBER);
		
		// test float with NaN
		testReal.value(java.lang.Float.NaN, 0);
		assertTrue(java.lang.Float.isNaN((float)testReal.toDouble()));
		assertTrue(testReal.hint() == RealHints.NOT_A_NUMBER);
	}
	
	@Test
	public void realTailingZerosTest() 
	{
		Real real = CodecFactory.createReal();
		real.clear();
		
		real.value("555.0");
		assertEquals("555.0", real.toString());
		
		real.value("-555.00");
		assertEquals("-555.00", real.toString());
		
		real.value("555.000");
		assertEquals("555.000", real.toString());
		
		real.value("-555.0000");
		assertEquals("-555.0000", real.toString());
		
		real.value("555.00000");
		assertEquals("555.00000", real.toString());
		
		real.value("-555.000000");
		assertEquals("-555.000000", real.toString());
		
		real.value("555.0000000");
		assertEquals("555.0000000", real.toString());
		
		real.value("-555.00000000");
		assertEquals("-555.00000000", real.toString());
		
		real.value("555.000000000");
		assertEquals("555.000000000", real.toString());
		
		real.value("-555.0000000000");
		assertEquals("-555.0000000000", real.toString());
		
		real.value("555.00000000000");
		assertEquals("555.00000000000", real.toString());
		
		real.value("-555.000000000000");
		assertEquals("-555.000000000000", real.toString());
		
		real.value("555.0000000000000");
		assertEquals("555.0000000000000", real.toString());
		
		real.value("-555.00000000000000");
		assertEquals("-555.00000000000000", real.toString());
	}

	private void testDoubleToRealOutOfRange(double val, int hint)
	{
		Real testReal = CodecFactory.createReal();
		assertEquals(testReal.value(val, hint), CodecReturnCodes.INVALID_ARGUMENT);
	}

	private void testFloatToRealOutOfRange(float val, int hint)
	{
		Real testReal = CodecFactory.createReal();
		assertEquals(testReal.value(val, hint), CodecReturnCodes.INVALID_ARGUMENT);
	}

	private void testDoubleToRealMaximumMinimumValue(double factor, int hint)
	{
		testDoubleToRealMaximumValue(factor, hint);
		testDoubleToRealMinimumValue(factor, hint);
	}

	private void testDoubleToRealMaximumValue(double factor, int hint)
	{
		Real testReal = CodecFactory.createReal();
		double val = Long.MAX_VALUE / factor;
		assertEquals(testReal.value(val, hint), CodecReturnCodes.SUCCESS);
		assertEquals(testReal.hint(), hint);

		// difference between MAX_VALUE and real value 0 .. 2048 = 2^11 - precision difference between long(64bit) and double(53bit)
		long diff = Long.MAX_VALUE - testReal.toLong();
		if (diff == 0) {
			assertEquals(testReal.toLong(), Long.MAX_VALUE);
		}
		else {
			assertTrue(diff <= 2048);
		}
		Real testNextReal = CodecFactory.createReal();
		val = java.lang.Math.nextAfter(val, 1e30);
		val = java.lang.Math.nextAfter(val, 1e30);
		assertEquals(testNextReal.value(val, hint), CodecReturnCodes.INVALID_ARGUMENT);
	}

	private void testDoubleToRealMinimumValue(double factor, int hint)
	{
		Real testReal = CodecFactory.createReal();
		double val = Long.MIN_VALUE / factor;
		assertEquals(testReal.value(val, hint), CodecReturnCodes.SUCCESS);
		assertEquals(testReal.hint(), hint);

		// difference between MIN_VALUE and real value 0 .. 2048 = 2^11 - precision difference between long(64bit) and double(53bit)
		long diff = testReal.toLong() - Long.MIN_VALUE;
		if (diff == 0) {
			assertEquals(testReal.toLong(), Long.MIN_VALUE);
		}
		else {
			assertTrue(diff <= 2048);
		}
		Real testNextReal = CodecFactory.createReal();
		val = java.lang.Math.nextAfter(val, -1e30);
		val = java.lang.Math.nextAfter(val, -1e30);
		assertEquals(testNextReal.value(val, hint), CodecReturnCodes.INVALID_ARGUMENT);
	}

	private void testFloatToRealMaximumMinimumValue(float factor, int hint)
	{
		testFloatToRealMaximumValue(factor, hint);
		testFloatToRealMinimumValue(factor, hint);
	}

	private void testFloatToRealMaximumValue(float factor, int hint)
	{
		Real testReal = CodecFactory.createReal();
		float val = Long.MAX_VALUE / factor;
		assertEquals(testReal.value(val, hint), CodecReturnCodes.SUCCESS);
		assertEquals(testReal.hint(), hint);

		// difference between MAX_VALUE and real value 0 .. 2^40 - precision difference between long(64bit) and float(24bit)
		long diff = Long.MAX_VALUE - testReal.toLong();
		if (diff == 0) {
			assertEquals(testReal.toLong(), Long.MAX_VALUE);
		}
		else {
			assertTrue(diff <= 1099511627776L);
		}

		Real testNextReal = CodecFactory.createReal();
		val = java.lang.Math.nextAfter(val, 1e30);
		val = java.lang.Math.nextAfter(val, 1e30);
		assertEquals(testNextReal.value(val, hint), CodecReturnCodes.INVALID_ARGUMENT);
	}

	private void testFloatToRealMinimumValue(float factor, int hint)
	{
		Real testReal = CodecFactory.createReal();
		float val = Long.MIN_VALUE / factor;
		assertEquals(testReal.value(val, hint), CodecReturnCodes.SUCCESS);
		assertEquals(testReal.hint(), hint);

		// difference between MIN_VALUE and real value 0 .. 2^40 - precision difference between long(64bit) and float(24bit)
		long diff = testReal.toLong() - Long.MIN_VALUE;
		if (diff == 0) {
			assertEquals(testReal.toLong(), Long.MIN_VALUE);
		}
		else {
			assertTrue(diff <= 1099511627776L);
		}

		Real testNextReal = CodecFactory.createReal();
		val = java.lang.Math.nextAfter(val, -1e30);
		val = java.lang.Math.nextAfter(val, -1e30);
		assertEquals(testNextReal.value(val, hint), CodecReturnCodes.INVALID_ARGUMENT);
	}

	@Test
	public void testDateDecodedAsRealNoCrash() {

		FieldEntry entry = CodecFactory.createFieldEntry();
		FieldList fieldList = CodecFactory.createFieldList();
		Real real = CodecFactory.createReal();
		Date date = CodecFactory.createDate();

		EncodeIterator encIter = CodecFactory.createEncodeIterator();
		DecodeIterator decIter = CodecFactory.createDecodeIterator();

		Buffer _buffer = CodecFactory.createBuffer();

		for (int year = 1900; year <= 2100; year++) {
			for (int month = 1; month <= 12; month++) {
				for (int day = 1; day <= 31; day++) {

					fieldList.clear();
					entry.clear();
					real.clear();
					date.clear();

					encIter.clear();
					decIter.clear();

					date.day(day);
					date.month(month);
					date.year(year);

					_buffer.clear();
					_buffer.data(ByteBuffer.allocate(100));

					fieldList.clear();
					fieldList.flags(FieldListFlags.HAS_STANDARD_DATA);
					encIter.clear();
					encIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());

					assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldListInit(encIter, fieldList, null, 0));
					entry.clear();
					entry.fieldId(6);
					entry.dataType(DataTypes.DATE);
					entry.encode(encIter, date);
					assertEquals(CodecReturnCodes.SUCCESS, Encoders.encodeFieldListComplete(encIter, true));

					fieldList.clear();
					entry.clear();
					decIter.clear();
					decIter.setBufferAndRWFVersion(_buffer, Codec.majorVersion(), Codec.minorVersion());
					assertEquals(CodecReturnCodes.SUCCESS, fieldList.decode(decIter, null));
					assertEquals(CodecReturnCodes.SUCCESS, entry.decode(decIter));

					boolean notCrashed = true;

					try {
						real.clear();
						int ret = real.decode(decIter);
						if (day == 31) {
							assertEquals(ret, CodecReturnCodes.INVALID_ARGUMENT);
						}
					} catch (Exception ex) {
						notCrashed = false;
					}

					assertEquals(notCrashed, true);
				}
			}
		}
	}

}
