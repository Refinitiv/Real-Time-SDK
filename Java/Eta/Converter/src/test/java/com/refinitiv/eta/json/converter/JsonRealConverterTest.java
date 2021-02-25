package com.refinitiv.eta.json.converter;

import org.junit.Test;

import static com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
import static com.refinitiv.eta.json.converter.JsonConverterTestUtils.TEST_MSG_BUFFER_SIZE;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.lang.reflect.Proxy;
import java.nio.ByteBuffer;

import org.junit.Before;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.RealHints;
import com.refinitiv.eta.transport.TransportFactory;

public class JsonRealConverterTest {
	
	Buffer jsonBuffer;
    Msg resultMsg;
    JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    Buffer origMsgBuffer;
    EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
    DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    JsonConverterError convError = ConverterFactory.createJsonConverterError();
    JsonConverter converter;
    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();
    ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    int streamId = MsgClasses.POST;
    Buffer errorBuffer = CodecFactory.createBuffer();
    GetJsonErrorParams errorParams = ConverterFactory.createJsonErrorParams();
    Msg message = CodecFactory.createMsg();
    FieldList fieldList = CodecFactory.createFieldList();
    FieldEntry fieldEntry = CodecFactory.createFieldEntry();
    DataDictionary dictionary = CodecFactory.createDataDictionary();
    Real realValue = CodecFactory.createReal();
    
    @Before
    public void init() {
        jsonMsg.clear();
        final String dictionaryFileName1 = "src/test/resources/RDMFieldDictionary";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();
        dictionary.loadFieldDictionary(dictionaryFileName1, error);

        jsonBuffer = CodecFactory.createBuffer();
        jsonBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        resultMsg = CodecFactory.createMsg();
        origMsgBuffer = CodecFactory.createBuffer();
        origMsgBuffer.data(ByteBuffer.allocate(TEST_MSG_BUFFER_SIZE));
        decodeIter.clear();
        JsonConverter converter = ConverterFactory.createJsonConverterBuilder()
                .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, false)
                .setServiceConverter(new ServiceNameIdTestConverter())
                .setDictionary(dictionary)
                .build(convError);
        JsonConverterProxy proxy = new JsonConverterProxy(converter);
        this.converter = (JsonConverter) Proxy.newProxyInstance(JsonConverter.class.getClassLoader(),
                new Class[]{JsonConverter.class},
                proxy);
        getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        getJsonMsgOptions.streamId(streamId);
        parseJsonOptions.setProtocolType(JsonProtocol.JSON_JPT_JSON2);
        rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
        decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
    }
    
    @Test
    public void testValidLongDecimalValuesTest() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"PostID\":3,\"PermData\":\"AwO9ZWLA\","
        		+ "\"Fields\":{\"NETCHNG_1\":1151194421449.10009, \"ASK\":9223372036854775.808 , \"ACVOL_1\":92233.72036854775808, \"TRDPRC_1\":1151194421449.10009766 , \"HIGH_1\": 9223372036854775808.9223372036854775808}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        
        int ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError);

        assertEquals(SUCCESS, ret);
        
        decodeIter.clear();
        
        ret = decodeIter.setBufferAndRWFVersion(jsonMsg.rwfMsg().encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion());
        
        assertEquals(CodecReturnCodes.SUCCESS, ret);
        
        message.clear();
        
       ret =  message.decode(decodeIter);
       
       assertEquals(CodecReturnCodes.SUCCESS, ret);
        
       fieldList.clear();
       
       ret = fieldList.decode(decodeIter, null);
       
       assertEquals(CodecReturnCodes.SUCCESS, ret);
       
       // decode each field entry in list
       while ((ret = fieldEntry.decode(decodeIter)) != CodecReturnCodes.END_OF_CONTAINER)
       {
    	   DictionaryEntry dictEntry = dictionary.entry(fieldEntry.fieldId());
    	   String fieldName = dictEntry.acronym().toString();
    	   
    	   if(fieldName.equals("NETCHNG_1"))
    	   {
    		   realValue.clear();
    		   
    		   ret = realValue.decode(decodeIter);
    		   assertEquals(CodecReturnCodes.SUCCESS, ret);
    		   
    		   assertEquals(11511944214491L, realValue.toLong());
    		   assertEquals(RealHints.EXPONENT_1, realValue.hint());
    	   }
    	   else if(fieldName.equals("ASK"))
    	   {
    		   realValue.clear();
    		   
    		   ret = realValue.decode(decodeIter);
    		   assertEquals(CodecReturnCodes.SUCCESS, ret);
    		   
    		   assertEquals(9223372036854776L, realValue.toLong());
    		   assertEquals(RealHints.EXPONENT0, realValue.hint());
    	   }
    	   else if(fieldName.equals("ACVOL_1"))
    	   {
    		   realValue.clear();
    		   
    		   ret = realValue.decode(decodeIter);
    		   assertEquals(CodecReturnCodes.SUCCESS, ret);
    		   
    		   assertEquals(9223372036854776L, realValue.toLong());
    		   assertEquals(RealHints.EXPONENT_11, realValue.hint());
    	   }
    	   else if(fieldName.equals("TRDPRC_1"))
    	   {
    		   realValue.clear();
    		   
    		   ret = realValue.decode(decodeIter);
    		   assertEquals(CodecReturnCodes.SUCCESS, ret);
    		   
    		   assertEquals(11511944214491L, realValue.toLong());
    		   assertEquals(RealHints.EXPONENT_1, realValue.hint());
    	   }
    	   else if(fieldName.equals("HIGH_1"))
    	   {
    		   realValue.clear();
    		   
    		   ret = realValue.decode(decodeIter);
    		   assertEquals(CodecReturnCodes.SUCCESS, ret);
    		   
    		   assertEquals(9223372036854776L, realValue.toLong());
    		   assertEquals(RealHints.EXPONENT3, realValue.hint());
    	   }
       }
    }
    
    @Test
    public void testOutOfRangeDecimalValuesTest() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"PostID\":3,\"PermData\":\"AwO9ZWLA\","
        		+ "\"Fields\":{\"BID\":9223372036854775808}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        
        int ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError);
        
        assertEquals(CodecReturnCodes.FAILURE, ret);
        assertEquals("JSON Converter unexpected value: Failed to converter the JSON numeric '9223372036854775808' value to Real", convError.getText());
    }
    
    @Test
    public void testOutOfRangeDecimalValuesTest2() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"PostID\":3,\"PermData\":\"AwO9ZWLA\","
        		+ "\"Fields\":{\"ASK\":6.666666666666666}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        
        int ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError);
        
        assertEquals(CodecReturnCodes.FAILURE, ret);
        assertEquals("JSON Converter encode error: code=-22, 'ASK' encoding failure", convError.getText());
    }
    
    @Test
    public void testOutOfRangeDecimalValuesTest3() throws IOException {
        final String postMsg = "{\"ID\":2,\"Type\":\"Post\",\"Domain\":\"MarketPrice\",\"Key\":{\"Name\":\"TRI.N\",\"Service\":1},\"PostID\":3,\"PermData\":\"AwO9ZWLA\","
        		+ "\"Fields\":{\"ACVOL_1\":-6.666666666666666}}}";

        jsonBuffer.data(postMsg);
        assertEquals(SUCCESS, converter.parseJsonBuffer(jsonBuffer, parseJsonOptions, convError));
        
        int ret = converter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, convError);
        
        assertEquals(CodecReturnCodes.FAILURE, ret);
        assertEquals("JSON Converter encode error: code=-22, 'ACVOL_1' encoding failure", convError.getText());
    }
	
	
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
	 
	 @Test
	 public void maxPositiveDecimalValueRangeTest()
	 {
		 String inputValue = "922337203685477580.7";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.SUCCESS, ret);
	 }
	 
	 
	 @Test
	 public void OutOfRangePositiveDecimalValueRangeTest()
	 {
		 String inputValue = "922337203685477580.8";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
	 }
	 
	 @Test
	 public void OutOfRangePositiveDecimalValueRangeTest2()
	 {
		 String inputValue = "9223372036854775808.8";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
	 }
	 
	 @Test
	 public void maxNegativeDecimalValueRangeTest()
	 {
		 String inputValue = "-922337203685477580.7";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.SUCCESS, ret);
	 }
	 
	 @Test
	 public void maxNegativeExponentialValueRangeTest2()
	 {
		 String inputValue = "-922337203685477580.8E-4";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.SUCCESS, ret);
	 }
	 
	 @Test
	 public void OutOfRangeNegativeDecimalValueRangeTest()
	 {
		 String inputValue = "-922337203685477580.8";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
	 }
	 
	 @Test
	 public void OutOfRangeNegativeDecimalValueRangeTest2()
	 {
		 String inputValue = "-9223372036854775808.8";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.INVALID_ARGUMENT, ret);
	 }
	 
	 @Test
	 public void maxPositiveExponentialValueRangeTest()
	 {
		 String inputValue = "922337203685477580.7E-4";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.SUCCESS, ret);
	 }
	 
	 @Test
	 public void outOfValueRangePositiveTest()
	 {
		 String inputValue = "9223372036854775808.0E7";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.FAILURE, ret);
	 }
	 
	 @Test
	 public void outOfValueRangePositiveTest2()
	 {
		 String inputValue = "922337203685477580.8E7";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.FAILURE, ret);
	 }
	 
	 @Test
	 public void outOfValueRangeNegativeTest()
	 {
		 String inputValue = "-9223372036854775807.0E-4";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.FAILURE, ret);
	 }
	 
	 @Test
	 public void outOfValueRangeNegativeTest2()
	 {
		 String inputValue = "-922337203685477580.9E-4";
		 Real real = CodecFactory.createReal();
		 real.clear();
		 
		 int ret = JsonRealConverter.processReal(inputValue, real);
		 
		 assertEquals(CodecReturnCodes.FAILURE, ret);
	 }
}
