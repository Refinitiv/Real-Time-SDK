package com.refinitiv.eta.json.converter;

import org.junit.Test;
import static org.junit.Assert.assertEquals;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.Real;

public class JsonRealConverterTest {
	
	
	 @Test
	 public void positiveExponentialTest_with_1_decimalpoint()
	 {
		 String inputValue = "1.0E4";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("10000.0", real.toString()); 
	 }
	 
	 @Test
	 public void positiveExponentialTest_with_2_decimalpoint()
	 {
		 String inputValue = "21.55E5";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("2155000.0", real.toString()); 
	 }
	 
	 @Test
	 public void positiveExponentialTest_with_3_decimalpoint()
	 {
		 String inputValue = "5968.367E3";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("5968367.0", real.toString()); 
	 }
	 
	 @Test
	 public void positiveExponentialTest_with_4_decimalpoint()
	 {
		 String inputValue = "25.5678E2";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("2556.78", real.toString()); 
	 }
	 
	 @Test
	 public void negativeExponentialTest_with_1_decimalpoint()
	 {
		 String inputValue = "1.0E-4";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals(inputValue, real.toString()); 
	 }
	 
	 @Test
	 public void negativeExponentialTest_with_2_decimalpoint()
	 {
		 String inputValue = "21.55E-5";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("2.155E-4", real.toString()); 
	 }
	 
	 @Test
	 public void negativeExponentialTest_with_3_decimalpoint()
	 {
		 String inputValue = "5968.367E-3";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("5.968367", real.toString()); 
	 }
	 
	 @Test
	 public void negativeExponentialTest_with_4_decimalpoint()
	 {
		 String inputValue = "25.5678E-2";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("0.255678", real.toString()); 
	 }
	 
	 @Test
	 public void negativezerovalue_positiveExponentialTest_with_4_decimalpoint()
	 {
		 String inputValue = "-0.5678E2";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("-56.78", real.toString()); 
	 }
	 
	 @Test
	 public void positivezerovalue_positiveExponentialTest_with_4_decimalpoint()
	 {
		 String inputValue = "0.5678E2";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("56.78", real.toString()); 
	 }
	 
	 @Test
	 public void negativevalue_positiveExponentialTest_with_4_decimalpoint()
	 {
		 String inputValue = "-25.5678E2";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("-2556.78", real.toString()); 
	 }
	 
	 @Test
	 public void negativevalue_negativeExponentialTest_with_2_decimalpoint()
	 {
		 String inputValue = "-21.55E-5";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("-2.155E-4", real.toString());
	 }
	 
	 
	 @Test
	 public void negativevalue_negativeExponentialTest_with_4_decimalpoint()
	 {
		 String inputValue = "-25.5678E-2";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("-0.255678", real.toString()); 
	 }
	 
	 @Test
	 public void minimum_positiveExponential()
	 {
		 String inputValue = "1.0E0";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals("1.0", real.toString()); 
	 }
	 
	 @Test
	 public void maximum_positiveExponential()
	 {
		 String inputValue = "1.0E8";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals(inputValue, real.toString()); 
	 }
	 
	 @Test
	 public void maximum_negativeExponential()
	 {
		 String inputValue = "1.0E-13";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.SUCCESS, JsonRealConverter.processReal(inputValue, real));
		 assertEquals(inputValue, real.toString()); 
	 }
	 
	 @Test
	 public void invalid_outofrange_positiveExponential()
	 {
		 String inputValue = "1.0E9";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.FAILURE, JsonRealConverter.processReal(inputValue, real));
	 }
	 
	 @Test
	 public void invalid_outofrange_negativeExponential()
	 {
		 String inputValue = "1.0E-14";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.FAILURE, JsonRealConverter.processReal(inputValue, real));
	 }
	 
	 @Test
	 public void invalid_value()
	 {
		 String inputValue = "ABC.0E-14";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.FAILURE, JsonRealConverter.processReal(inputValue, real));
	 }
	 
	 @Test
	 public void invalid_decimalvalue()
	 {
		 String inputValue = "123.ABDE-14";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.FAILURE, JsonRealConverter.processReal(inputValue, real));
	 }
	 
	 @Test
	 public void invalid_positiveExponentialValue()
	 {
		 String inputValue = "123.0ECD";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.FAILURE, JsonRealConverter.processReal(inputValue, real));
	 }
	 
	 @Test
	 public void invalid_negativeExponentialValue()
	 {
		 String inputValue = "123.0E-A";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 assertEquals(CodecReturnCodes.FAILURE, JsonRealConverter.processReal(inputValue, real));
	 }
}
